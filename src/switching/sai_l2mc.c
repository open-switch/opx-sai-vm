/*
 * Copyright (c) 2018 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

/**
* @file sai_l2mc.c
*
* @brief This file contains implementation of SAI L2MC APIs.
*************************************************************************/

#include <stdlib.h>
#include <inttypes.h>
#include "std_assert.h"
#include "sail2mc.h"
#include "sail2mcgroup.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saiport.h"
#include "sai_modules_init.h"
#include "sai_l2mc_common.h"
#include "sai_l2mc_api.h"
#include "sai_mcast_common.h"
#include "sai_mcast_api.h"
#include "sai_npu_port.h"
#include "sai_npu_l2mc.h"
#include "sai_port_utils.h"
#include "sai_l3_util.h"
#include "sai_port_common.h"
#include "sai_port_main.h"
#include "sai_switch_utils.h"
#include "sai_gen_utils.h"
#include "sai_oid_utils.h"
#include "sai_common_infra.h"

static void sai_l2mc_mcast_lock()
{
    sai_l2mc_lock();
    sai_mcast_lock();
}

static void sai_l2mc_mcast_unlock()
{
    sai_mcast_unlock();
    sai_l2mc_unlock();
}

static inline void sai_l2mc_entry_log_trace (dn_sai_mcast_entry_node_t *mcast_entry_node,
                                            char *p_info_str)
{
    char addr_str [SAI_FIB_MAX_BUFSZ];
    char addr_str1 [SAI_FIB_MAX_BUFSZ];

    SAI_L2MC_LOG_TRACE ("%s, VRF: 0x%"PRIx64", Group: %s, Vlan: 0x%"PRIx64", "
                         "Source: %s, Packet-action: %s",
                         p_info_str, mcast_entry_node->mcast_key.vrf_id, sai_ip_addr_to_str
                         (&mcast_entry_node->mcast_key.grp_addr, addr_str, SAI_FIB_MAX_BUFSZ),
                         mcast_entry_node->mcast_key.bv_id, sai_ip_addr_to_str
                         (&mcast_entry_node->mcast_key.src_addr, addr_str1, SAI_FIB_MAX_BUFSZ),
                         sai_packet_action_str (mcast_entry_node->action));
}

static inline void sai_l2mc_entry_log_error (dn_sai_mcast_entry_node_t *mcast_entry_node,
                                            char *p_error_str)
{
    char addr_str [SAI_FIB_MAX_BUFSZ];
    char addr_str1 [SAI_FIB_MAX_BUFSZ];

    SAI_L2MC_LOG_ERR ("%s, VRF: 0x%"PRIx64", Group: %s, BV_ID: 0x%"PRIx64", "
                         "Source: %s, Packet-action: %s",
                         p_error_str, mcast_entry_node->mcast_key.vrf_id, sai_ip_addr_to_str
                         (&mcast_entry_node->mcast_key.grp_addr, addr_str, SAI_FIB_MAX_BUFSZ),
                         mcast_entry_node->mcast_key.bv_id, sai_ip_addr_to_str
                         (&mcast_entry_node->mcast_key.src_addr, addr_str1, SAI_FIB_MAX_BUFSZ),
                         sai_packet_action_str (mcast_entry_node->action));
}

static void sai_l2mc_entry_node_init( dn_sai_mcast_entry_node_t *mcast_entry_node,
        const sai_l2mc_entry_t *l2mc_entry)
{
    memset(mcast_entry_node, 0, sizeof(dn_sai_mcast_entry_node_t));
    mcast_entry_node->mcast_key.bv_id = l2mc_entry->bv_id;
    sai_fib_ip_addr_copy(&mcast_entry_node->mcast_key.grp_addr, &l2mc_entry->destination);
    sai_fib_ip_addr_copy(&mcast_entry_node->mcast_key.src_addr, &l2mc_entry->source);

    mcast_entry_node->entry_type = (l2mc_entry->type == SAI_L2MC_ENTRY_TYPE_SG)?
        SAI_MCAST_ENTRY_TYPE_SG : SAI_MCAST_ENTRY_TYPE_XG;
    mcast_entry_node->owner = SAI_MCAST_OWNER_L2MC;
    mcast_entry_node->switch_id = l2mc_entry->switch_id;
}

static sai_status_t sai_l2mc_entry_input_params_validate_fill(
        dn_sai_mcast_entry_node_t *mcast_entry_node, uint_t attr_count,
        const sai_attribute_t *attr_list)
{
    unsigned int attr_idx = 0;
    bool pkt_action_present = false;
    bool l2mc_grp_attr_present = false;

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        switch(attr_list[attr_idx].id) {
            case SAI_L2MC_ENTRY_ATTR_PACKET_ACTION:
                if(attr_list[attr_idx].value.s32 == SAI_PACKET_ACTION_FORWARD) {
                    pkt_action_present = true;
                    mcast_entry_node->action = SAI_PACKET_ACTION_FORWARD;
                } else {
                    sai_l2mc_entry_log_error(mcast_entry_node, "Invalid pkt action attribute");
                    return SAI_STATUS_INVALID_PARAMETER;
                }
                break;
            case SAI_L2MC_ENTRY_ATTR_OUTPUT_GROUP_ID:
                if(!sai_is_obj_id_l2mc_group(attr_list[attr_idx].value.oid)) {
                    return SAI_STATUS_INVALID_OBJECT_ID;
                }
                if(!sai_find_l2mc_group_node(attr_list[attr_idx].value.oid)) {
                    return SAI_STATUS_ITEM_NOT_FOUND;
                }
                l2mc_grp_attr_present = true;
                mcast_entry_node->mcast_group_id = attr_list[attr_idx].value.oid;
                break;
            default:
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
        }
    }
    if(!(pkt_action_present) || !(l2mc_grp_attr_present)) {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_l2mc_create_entry(
        const sai_l2mc_entry_t *l2mc_entry,
        uint32_t attr_count,
        const sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_mcast_entry_node_t mcast_entry_node;

    STD_ASSERT (l2mc_entry != NULL);
    STD_ASSERT (attr_list != NULL);

    if (attr_count == 0) {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    } else {
        STD_ASSERT ((attr_list != NULL));
    }

    sai_l2mc_entry_node_init(&mcast_entry_node, l2mc_entry);

    sai_rc = sai_l2mc_entry_input_params_validate_fill(&mcast_entry_node, attr_count, attr_list);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_L2MC_LOG_ERR ("Input paramters validation failed for MCAST entry.");
        return sai_rc;
    }

    SAI_L2MC_LOG_TRACE("Creating L2MC Entry ");
    sai_l2mc_mcast_lock();
    do {

        sai_rc = sai_mcast_npu_api_get()->mcast_entry_create(&mcast_entry_node);
        if((sai_rc != SAI_STATUS_SUCCESS)) {
            break;
        }

        if((sai_rc = sai_insert_mcast_entry_node(&mcast_entry_node))
                != SAI_STATUS_SUCCESS) {
            sai_l2mc_entry_log_error(&mcast_entry_node, "Unable to add L2MC entry");
            sai_mcast_npu_api_get()->mcast_entry_remove(&mcast_entry_node);
        }

    }while(0);

    sai_l2mc_mcast_unlock();

    if(sai_rc == SAI_STATUS_SUCCESS){
        sai_l2mc_entry_log_trace(&mcast_entry_node, "Created L2MC entry");
    }

    return sai_rc;
}

static sai_status_t sai_l2mc_set_entry_attribute(
        const sai_l2mc_entry_t *l2mc_entry,
        const sai_attribute_t *attr)
{
    /* Currently l2mc entry set is not supported */
    return SAI_STATUS_NOT_IMPLEMENTED;
}

static sai_status_t sai_l2mc_get_entry_attribute(
        const sai_l2mc_entry_t *l2mc_entry,
        uint32_t attr_count,
        sai_attribute_t *attr_list)
{
    /* Currently l2mc entry get is not supported */
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_l2mc_remove_entry(const sai_l2mc_entry_t *l2mc_entry)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_mcast_entry_node_t *mcast_entry_node = NULL;
    dn_sai_mcast_entry_node_t tmp_node;

    SAI_L2MC_LOG_TRACE("Remove L2MC Entry");
    STD_ASSERT(l2mc_entry != NULL);

    sai_l2mc_entry_node_init(&tmp_node, l2mc_entry);
    sai_l2mc_mcast_lock();
    do {
        if((mcast_entry_node = sai_find_mcast_entry(&tmp_node)) == NULL) {
            sai_rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        if((sai_rc = sai_mcast_npu_api_get()->mcast_entry_remove(mcast_entry_node))
                != SAI_STATUS_SUCCESS) {
            break;
        }
        sai_remove_mcast_entry_node(mcast_entry_node);
    } while(0);

    sai_l2mc_mcast_unlock();
    return sai_rc;
}

static sai_l2mc_api_t sai_l2mc_method_table =
{
    sai_l2mc_create_entry,
    sai_l2mc_remove_entry,
    sai_l2mc_set_entry_attribute,
    sai_l2mc_get_entry_attribute,
};

sai_l2mc_api_t  *sai_l2mc_entry_api_query (void)
{
    return (&sai_l2mc_method_table);
}
