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
#include <string.h>
#include "std_assert.h"
#include "sail2mc.h"
#include "sail2mcgroup.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saiport.h"
#include "sai_modules_init.h"
#include "sai_l2mc_common.h"
#include "sai_l2mc_api.h"
#include "sai_mcast_api.h"
#include "sai_npu_port.h"
#include "sai_npu_l2mc.h"
#include "sai_switch_utils.h"
#include "sai_gen_utils.h"
#include "sai_oid_utils.h"
#include "sai_stp_api.h"
#include "sai_common_infra.h"
#include "sai_lag_api.h"
#include "sai_map_utl.h"
#include "sai_bridge_api.h"
#include "sai_bridge_main.h"
#include "sai_port_utils.h"

/*LAG lock needs to be taken since LAG can be modified independently of bridge port*/
static void sai_l2mc_take_module_lock(sai_object_id_t bridge_port_id)
{
    sai_bridge_lock();

    if(sai_is_bridge_port_obj_lag(bridge_port_id)) {
        sai_lag_lock();
    }
}

static void sai_l2mc_give_module_lock(sai_object_id_t bridge_port_id)
{
    if(sai_is_bridge_port_obj_lag(bridge_port_id)) {
        sai_lag_unlock();
    }
    sai_bridge_unlock();
}
sai_status_t sai_l2mc_handle_lag_change_notification(sai_object_id_t bridge_port_id,
        const sai_bridge_port_notif_t *data)
{
    uint_t                     l2mc_member_cnt = 0;
    uint_t                     member_idx = 0;
    sai_object_id_t            l2mc_member_id =SAI_NULL_OBJECT_ID;
    sai_status_t               sai_rc = SAI_STATUS_FAILURE;
    dn_sai_l2mc_member_node_t *l2mc_member_node = NULL;


    sai_l2mc_lock();
    sai_l2mc_take_module_lock(bridge_port_id);
    do {
        sai_rc = sai_bridge_port_to_l2mc_member_count_get(bridge_port_id, &l2mc_member_cnt);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_L2MC_LOG_ERR("Error %d in obtaining the l2mc member count for bridge port 0x%"PRIx64"",
                             sai_rc, bridge_port_id);
            break;
        }

        if(l2mc_member_cnt == 0) {
            SAI_L2MC_LOG_TRACE("No L2MC members associated to bridge port 0x%"PRIx64"",
                               bridge_port_id);
            sai_rc =  SAI_STATUS_SUCCESS;
            break;
        }

        for(member_idx = 0; member_idx < l2mc_member_cnt; member_idx++) {
            sai_rc = sai_bridge_port_get_l2mc_member_at_index(bridge_port_id, member_idx,
                                                              &l2mc_member_id);
            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_L2MC_LOG_ERR("Error %d in obtaining the l2mc member[%d] for bridge port "
                                 "0x%"PRIx64"", sai_rc, member_idx, bridge_port_id);
                break;
            }

            l2mc_member_node = sai_find_l2mc_member_node(l2mc_member_id);
            if(l2mc_member_node == NULL) {
                SAI_L2MC_LOG_ERR("Error - Unable to find info for l2mc member 0x%"PRIx64"",
                                 l2mc_member_id);
                sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
                break;
            }
            sai_rc = sai_l2mc_npu_api_get()->
                l2mc_member_lag_notif_handler(l2mc_member_node, data->lag_id,
                                              data->lag_port_mod_list->count,
                                              data->lag_port_mod_list->list,
                                              data->lag_add_port);
            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_L2MC_LOG_ERR("Error %d in npu update of lag handler for l2mc member "
                                 "0x%"PRIx64"", sai_rc, l2mc_member_id);
                break;
            }
        }

    } while(0);

    sai_l2mc_give_module_lock(bridge_port_id);
    sai_l2mc_unlock();
    return sai_rc;
}

static sai_status_t sai_l2mc_bridge_port_notif_handler(sai_object_id_t bridge_port_id,
                                                       const sai_bridge_port_notif_t *data)
{
    if(data == NULL) {
        SAI_L2MC_LOG_ERR("NULL data in bridge port 0x%"PRIx64" callback",bridge_port_id);
    }

    if(data->event == SAI_BRIDGE_PORT_EVENT_LAG_MODIFY) {
        return sai_l2mc_handle_lag_change_notification(bridge_port_id, data);
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_l2mc_init(void)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    uint_t type_bmp = (SAI_BRIDGE_PORT_TYPE_TO_BMP(SAI_BRIDGE_PORT_TYPE_PORT) |
                      (SAI_BRIDGE_PORT_TYPE_TO_BMP(SAI_BRIDGE_PORT_TYPE_SUB_PORT)));

    sai_rc = sai_l2mc_tree_init();
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_L2MC_LOG_ERR("L2mc Tree init failed");
    }

    sai_rc = sai_l2mc_npu_api_get()->l2mc_init();
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_L2MC_LOG_ERR("L2mc NPU init failed");
    }

    sai_rc = sai_mcast_init();
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_L2MC_LOG_ERR("MCAST NPU init failed");
    }
    sai_rc = sai_bridge_port_event_cb_register(SAI_MODULE_L2MC, type_bmp,
                                          sai_l2mc_bridge_port_notif_handler);
    return sai_rc;
}

static sai_status_t sai_l2mc_create_group(
        sai_object_id_t *l2mc_obj_id,
        sai_object_id_t switch_id,
        uint32_t attr_count,
        const sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_l2mc_group_node_t l2mc_group_node;

    if (attr_count > 0) {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&l2mc_group_node, 0, sizeof(l2mc_group_node));
    SAI_L2MC_LOG_TRACE("Creating L2MC Group ");
    sai_l2mc_lock();
    do {

        sai_rc = sai_l2mc_npu_api_get()->l2mc_group_create(&l2mc_group_node);
        if((sai_rc != SAI_STATUS_SUCCESS)) {
            break;
        }

        *l2mc_obj_id = l2mc_group_node.l2mc_group_id;
        if((sai_rc = sai_add_l2mc_group_node(&l2mc_group_node))
                != SAI_STATUS_SUCCESS) {
            SAI_L2MC_LOG_ERR("Unable to add L2mc Group id 0x%"PRIx64"",
                             l2mc_group_node.l2mc_group_id);
            sai_l2mc_npu_api_get()->l2mc_group_delete(&l2mc_group_node);
        }
        SAI_L2MC_LOG_TRACE("Created L2MC Group id 0x%"PRIx64"",
                           l2mc_group_node.l2mc_group_id);


    }while(0);

    sai_l2mc_unlock();

    return sai_rc;
}

static sai_status_t sai_l2mc_set_group_attribute(
        sai_object_id_t l2mc_group_id,
        const sai_attribute_t *attr)
{
    /* No set attribute present for l2mc group */
    return SAI_STATUS_INVALID_ATTRIBUTE_0;
}

static sai_status_t sai_l2mc_get_group_attribute(
        sai_object_id_t l2mc_group_id,
        uint32_t attr_count,
        sai_attribute_t *attr_list)
{
    dn_sai_l2mc_group_node_t *l2mc_group_node = NULL;
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    unsigned int attr_idx = 0;

    if(!sai_is_obj_id_l2mc_group(l2mc_group_id)) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    STD_ASSERT(attr_list != NULL);
    if(attr_count == 0) {
        SAI_L2MC_LOG_ERR("Invalid attribute count 0 for L2MC group:0x%"PRIx64"", l2mc_group_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_l2mc_lock();
    if((l2mc_group_node = sai_find_l2mc_group_node(l2mc_group_id)) == NULL) {
        sai_l2mc_unlock();
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        switch(attr_list[attr_idx].id)
        {
            case SAI_L2MC_GROUP_ATTR_L2MC_OUTPUT_COUNT:
                SAI_L2MC_LOG_TRACE("Getting member count for L2MC id 0x%"PRIx64"", l2mc_group_id);
                attr_list[attr_idx].value.u32 = l2mc_group_node->port_count;
                sai_rc = SAI_STATUS_SUCCESS;
                break;
            case SAI_L2MC_GROUP_ATTR_L2MC_MEMBER_LIST:
                SAI_L2MC_LOG_TRACE("Getting member list for L2MC id 0x%"PRIx64"", l2mc_group_id);
                sai_rc = sai_l2mc_port_list_get(l2mc_group_node, &attr_list[attr_idx].value.objlist);
                break;
            default:
                SAI_L2MC_LOG_TRACE("Invalid attr : %d for L2mc Group id 0x%"PRIx64"",
                                   attr_list[attr_idx].id, l2mc_group_id);
                break;
        }
        if(sai_rc != SAI_STATUS_SUCCESS){
            sai_rc = sai_get_indexed_ret_val(sai_rc, attr_idx);
            break;
        }
    }
    sai_l2mc_unlock();
    return sai_rc;
}

static sai_status_t sai_l2mc_remove_group(sai_object_id_t l2mc_group_id)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_l2mc_group_node_t *l2mc_group_node = NULL;

    SAI_L2MC_LOG_TRACE("Removing L2MC Group 0x%"PRIx64"",l2mc_group_id);
    sai_l2mc_lock();
    do {
        if((l2mc_group_node = sai_find_l2mc_group_node(l2mc_group_id)) == NULL) {
            sai_rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        if((l2mc_group_node->port_count > 0) ||
           (l2mc_group_node->bridge_id != SAI_NULL_OBJECT_ID)) {
            sai_rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        if((sai_rc = sai_l2mc_npu_api_get()->l2mc_group_delete(l2mc_group_node))
                != SAI_STATUS_SUCCESS) {
            break;
        }
        sai_rc = sai_remove_l2mc_group_node(l2mc_group_node);
        SAI_L2MC_LOG_TRACE("Removed L2MC Group ");
    } while(0);

    sai_l2mc_unlock();
    return sai_rc;
}

static sai_status_t sai_l2mc_get_bridge_port_from_attr(uint32_t attr_count,
                                                       const sai_attribute_t *attr_list,
                                                       sai_object_id_t *bridge_port_id)
{
    uint_t attr_idx;

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        if(attr_list[attr_idx].id == SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID) {
            *bridge_port_id = attr_list[attr_idx].value.oid;
            return SAI_STATUS_SUCCESS;
        }
    }
    return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
}

static sai_status_t sai_l2mc_create_group_member(sai_object_id_t *l2mc_member_id,
        sai_object_id_t switch_id,
        uint32_t attr_count,
        const sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    bool l2mc_grp_attr_present = false;
    bool port_id_attr_present = false;
    dn_sai_l2mc_member_node_t l2mc_member_node;
    dn_sai_l2mc_group_node_t *l2mc_group_node;
    uint32_t attr_idx = 0;
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;

    STD_ASSERT (l2mc_member_id != NULL);
    STD_ASSERT (attr_list != NULL);

    *l2mc_member_id = SAI_INVALID_L2MC_MEMBER_ID;

    if (attr_count == 0) {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    } else {
        STD_ASSERT ((attr_list != NULL));
    }

    sai_rc = sai_l2mc_get_bridge_port_from_attr(attr_count, attr_list, &bridge_port_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    memset(&l2mc_member_node, 0, sizeof(l2mc_member_node));
    sai_l2mc_lock();
    sai_l2mc_take_module_lock(bridge_port_id);
    l2mc_member_node.switch_id = switch_id;

    do {
        for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {
            switch (attr_list [attr_idx].id) {
                case SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID:
                    if(!sai_is_obj_id_l2mc_group(attr_list[attr_idx].value.oid)) {
                        sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
                        break;
                    }
                    l2mc_member_node.l2mc_group_id = attr_list[attr_idx].value.oid;
                    l2mc_grp_attr_present = true;
                    break;
                case SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID:
                    l2mc_member_node.bridge_port_id = attr_list[attr_idx].value.oid;
                    port_id_attr_present = true;
                    break;

                case SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_ENDPOINT_IP:
                    break;

                default:
                    sai_rc = sai_get_indexed_ret_val(SAI_STATUS_UNKNOWN_ATTRIBUTE_0, attr_idx);
            }
            if(sai_rc != SAI_STATUS_SUCCESS) {
                break;
            }
        }

        if(sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }
        if(!(l2mc_grp_attr_present) || !(port_id_attr_present)) {
            sai_rc = SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
            break;
        }

        if((l2mc_group_node = sai_find_l2mc_group_node(l2mc_member_node.l2mc_group_id)) == NULL) {
            sai_rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        if(sai_find_l2mc_member_node_from_port(l2mc_group_node, l2mc_member_node.bridge_port_id)) {
            sai_rc = SAI_STATUS_ITEM_ALREADY_EXISTS;
            break;
        }

        if(!sai_l2mc_is_valid_member_bridge_id(l2mc_group_node,
                                               l2mc_member_node.bridge_port_id)) {
            SAI_L2MC_LOG_ERR("All members must belong to same bridge");
            sai_rc = SAI_STATUS_INVALID_PARAMETER;
            break;
        }
        if((sai_rc = sai_l2mc_npu_api_get()->l2mc_member_create(&l2mc_member_node))
                != SAI_STATUS_SUCCESS) {
            break;
        }

        *l2mc_member_id = l2mc_member_node.l2mc_member_id;
        if((sai_rc = sai_add_l2mc_member_node(l2mc_member_node))
                != SAI_STATUS_SUCCESS) {
            SAI_L2MC_LOG_ERR("Unable to add L2MC member 0x%"PRIx64" to Group:0x%"PRIx64" cache",
                             l2mc_member_node.l2mc_member_id, l2mc_member_node.l2mc_group_id);
            sai_l2mc_npu_api_get()->l2mc_member_remove(&l2mc_member_node);
            break;
        }
        sai_bridge_port_to_l2mc_member_map_insert(l2mc_member_node.bridge_port_id, *l2mc_member_id);

        SAI_L2MC_LOG_TRACE("Added port 0x%"PRIx64" on L2mc Group:0x%"PRIx64"",
                           l2mc_member_node.bridge_port_id, l2mc_member_node.l2mc_group_id);
    } while(0);

    sai_l2mc_give_module_lock(bridge_port_id);
    sai_l2mc_unlock();
    return sai_rc;
}

static sai_status_t sai_l2mc_set_group_member_attribute(sai_object_id_t l2mc_member_id,
        const sai_attribute_t *attr)
{
    /* There is no set attribute for the group member */
    return SAI_STATUS_INVALID_ATTRIBUTE_0;
}

static sai_status_t sai_l2mc_get_group_member_attribute(sai_object_id_t l2mc_member_id,
        const uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_l2mc_member_node_t *l2mc_member_node = NULL;
    uint32_t attr_idx = 0;

    STD_ASSERT(attr_list != NULL);
    if (attr_count == 0) {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    } else if (attr_count > 0) {
        STD_ASSERT ((attr_list != NULL));
    }

    if(!sai_is_obj_id_l2mc_member(l2mc_member_id)) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_l2mc_lock();
    if((l2mc_member_node = sai_find_l2mc_member_node(l2mc_member_id))
            == NULL) {
        sai_rc = SAI_STATUS_ITEM_NOT_FOUND;
    } else {
        for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {
            switch (attr_list [attr_idx].id) {
                case SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID:
                    attr_list[attr_idx].value.oid = l2mc_member_node->l2mc_group_id;
                    break;
                case SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID:
                    attr_list[attr_idx].value.oid = l2mc_member_node->bridge_port_id;
                    break;
                default:
                    sai_rc = (SAI_STATUS_UNKNOWN_ATTRIBUTE_0+attr_idx);
                    break;
            }
            if(SAI_STATUS_SUCCESS != sai_rc) {
                break;
            }
        }
    }

    sai_l2mc_unlock();
    return sai_rc;
}

static sai_status_t sai_l2mc_remove_group_member(sai_object_id_t l2mc_member_id)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_l2mc_member_node_t *l2mc_member_node = NULL;
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;

    if(!sai_is_obj_id_l2mc_member(l2mc_member_id)) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_l2mc_lock();
    if((l2mc_member_node = sai_find_l2mc_member_node(l2mc_member_id)) == NULL) {
        sai_l2mc_unlock();
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    bridge_port_id = l2mc_member_node->bridge_port_id;
    sai_l2mc_take_module_lock(bridge_port_id);
    do {
        if((sai_rc = sai_l2mc_npu_api_get()->l2mc_member_remove(l2mc_member_node))
                != SAI_STATUS_SUCCESS) {
            break;
        }
        sai_bridge_port_to_l2mc_member_map_remove(l2mc_member_node->bridge_port_id, l2mc_member_id);

        sai_rc = sai_remove_l2mc_member_node(l2mc_member_node);
    } while(0);

    sai_l2mc_give_module_lock(bridge_port_id);
    sai_l2mc_unlock();
    return sai_rc;
}

static sai_l2mc_group_api_t sai_l2mc_group_method_table =
{
    sai_l2mc_create_group,
    sai_l2mc_remove_group,
    sai_l2mc_set_group_attribute,
    sai_l2mc_get_group_attribute,
    sai_l2mc_create_group_member,
    sai_l2mc_remove_group_member,
    sai_l2mc_set_group_member_attribute,
    sai_l2mc_get_group_member_attribute,
};

sai_l2mc_group_api_t  *sai_l2mc_group_api_query (void)
{
    return (&sai_l2mc_group_method_table);
}
