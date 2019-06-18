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
 * @file sai_vm_rif.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI Router Interface object in VM environment.
 */

#include "sai_vm_defs.h"
#include "sai_routing_db_api.h"
#include "sai_vm_l3_util.h"
#include "sai_oid_utils.h"
#include "sai_l3_api.h"
#include "sai_l3_common.h"
#include "sai_l3_util.h"
#include "sairouterinterface.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_mac_utils.h"
#include "std_bit_masks.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include <inttypes.h>

static  dn_sai_id_gen_info_t sai_vm_rif_gen_info = {0};

static void sai_vm_rif_log_trace (sai_fib_router_interface_t *p_rif_node,
                                  char *p_info_str)
{
    char   p_buf [SAI_VM_MAX_BUFSZ];

    SAI_RIF_LOG_TRACE ("%s, RIF ID: %ld (VRF: 0x%"PRIx64", %s %d), "
                       "V4 admin state: %s, V6 admin state: %s, MTU: %d, "
                       "IP Options packet action: %d (%s), MAC: %s.",
                       p_info_str, sai_uoid_npu_obj_id_get
                       (p_rif_node->rif_id), p_rif_node->vrf_id,
                       sai_fib_rif_type_to_str (p_rif_node->type),
                       sai_fib_rif_attachment_id_get (p_rif_node),
                       (p_rif_node->v4_admin_state)? "ON" : "OFF",
                       (p_rif_node->v6_admin_state)? "ON" : "OFF",
                       p_rif_node->mtu, p_rif_node->ip_options_pkt_action,
                       sai_packet_action_str
                       (p_rif_node->ip_options_pkt_action),
                       std_mac_to_string
                       ((const hal_mac_addr_t *)&p_rif_node->src_mac, p_buf,
                        SAI_VM_MAX_BUFSZ));
}

static bool sai_vm_router_interface_in_use(uint64_t id)
{
    uint64_t temp_id = id + sai_vm_bridge_rif_id_start_get();
    sai_object_id_t rif_id =
        sai_uoid_create(SAI_OBJECT_TYPE_ROUTER_INTERFACE, temp_id);

    return sai_fib_is_rif_created(rif_id);
}

static sai_npu_object_id_t sai_vm_router_interface_id_generate(void)
{
    sai_vm_rif_gen_info.mask = (SAI_VM_MAX_BRIDGE_RIFS - 1);
    sai_vm_rif_gen_info.is_id_in_use = sai_vm_router_interface_in_use;

    if(SAI_STATUS_SUCCESS ==
       dn_sai_get_next_free_id(&sai_vm_rif_gen_info)) {

        return (sai_vm_rif_gen_info.cur_id + sai_vm_bridge_rif_id_start_get());
    }

    return SAI_VM_INVALID_RIF_ID;
}

static sai_npu_object_id_t sai_vm_rif_node_get_rif_id (
sai_fib_router_interface_t *p_rif)
{
    sai_object_id_t     obj_id = 0;
    sai_npu_object_id_t attach_id = 0;
    sai_npu_object_id_t rif_id = 0;

    STD_ASSERT (p_rif != NULL);

    obj_id = p_rif->attachment.port_id;

    if (p_rif->type == SAI_ROUTER_INTERFACE_TYPE_VLAN) {
        rif_id = sai_vm_vlan_rif_id_get (p_rif->attachment.vlan_id);
    } else if (p_rif->type == SAI_ROUTER_INTERFACE_TYPE_PORT) {
        attach_id = sai_uoid_npu_obj_id_get (obj_id);

        if (sai_is_obj_id_lag (obj_id)) {
            rif_id =  sai_vm_lag_rif_id_get (attach_id);
        } else if (sai_is_obj_id_port (obj_id)) {
            rif_id = sai_vm_port_rif_id_get (attach_id);
        }
    } else if (p_rif->type == SAI_ROUTER_INTERFACE_TYPE_BRIDGE) {
        rif_id = sai_vm_router_interface_id_generate();
    }

    return rif_id;
}

static sai_status_t sai_vm_fib_rif_port_get_fill (
sai_fib_router_interface_t *p_rif_node, sai_attribute_t *p_attr)
{
    if (p_rif_node->type == SAI_ROUTER_INTERFACE_TYPE_PORT) {
        p_attr->value.oid = p_rif_node->attachment.port_id;
    } else {
        SAI_RIF_LOG_ERR ("RIF Obj Id: 0x%"PRIx64" is not a port, Type: %s.",
                         p_rif_node->rif_id,
                         sai_fib_rif_type_to_str (p_rif_node->type));

        return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_fib_rif_vlan_get_fill (
sai_fib_router_interface_t *p_rif_node, sai_attribute_t *p_attr)
{
    if (p_rif_node->type == SAI_ROUTER_INTERFACE_TYPE_VLAN) {
        p_attr->value.u16 = p_rif_node->attachment.vlan_id;
    } else {
        SAI_RIF_LOG_ERR ("RIF Obj Id: 0x%"PRIx64" is not a VLAN, Type: %s.",
                         p_rif_node->rif_id,
                         sai_fib_rif_type_to_str (p_rif_node->type));

        return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_rif_attr_validate (sai_router_interface_type_t type,
                                        const sai_attribute_t *p_attr)
{
    STD_ASSERT (p_attr != NULL);

    SAI_RIF_LOG_TRACE ("validating RIF attribute ID: %d for type: %s.",
                       p_attr->id, sai_fib_rif_type_to_str (type));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_router_interface_create (
sai_fib_router_interface_t *p_rif_node, sai_npu_object_id_t *p_rif_id)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    sai_npu_object_id_t rif_id = 0;
    sai_object_id_t     rif_obj_id = 0;

    sai_vm_rif_log_trace (p_rif_node, "Router Interface creation.");

    STD_ASSERT (p_rif_node != NULL);
    STD_ASSERT (p_rif_id != NULL);

    rif_id = sai_vm_rif_node_get_rif_id (p_rif_node);
    if(rif_id == SAI_VM_INVALID_RIF_ID) {

        SAI_RIF_LOG_ERR ("Router interface table full");
        return SAI_STATUS_TABLE_FULL;
    }

    /* Check if RIF exists already. */
    rif_obj_id = sai_uoid_create (SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_id);

    if (sai_fib_router_interface_node_get (rif_obj_id)) {
        SAI_RIF_LOG_TRACE ("RIF node exists already with RIF ID: %ld, "
                           "RIF Object ID: 0x%"PRIx64".", rif_id, rif_obj_id);

        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    *p_rif_id = rif_id;

    /* Insert RIF record to DB. */
    sai_rc = sai_routerintf_create_db_entry (rif_obj_id, p_rif_node);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_RIF_LOG_ERR ("Error inserting entry to DB for RIF ID %ld, "
                         "OBJ ID: 0x%"PRIx64".", rif_id, rif_obj_id);

        return SAI_STATUS_FAILURE;
    }

    SAI_RIF_LOG_TRACE ("RIF Creation success, RIF ID: %ld.", *p_rif_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_router_interface_remove (sai_fib_router_interface_t *p_rif)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    sai_npu_object_id_t rif_id = 0;

    STD_ASSERT (p_rif != NULL);

    rif_id = sai_uoid_npu_obj_id_get (p_rif->rif_id);

    SAI_RIF_LOG_TRACE ("RIF Deletion, RIF ID: %ld.", rif_id);

    /* Remove RIF record from DB. */
    sai_rc = sai_routerintf_delete_db_entry (p_rif->rif_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_RIF_LOG_ERR ("Error removing entry from DB for RIF ID %ld, "
                         "OBJ ID: 0x%"PRIx64".", rif_id, p_rif->rif_id);

        return SAI_STATUS_FAILURE;
    }

    SAI_RIF_LOG_TRACE ("RIF ID: %ld deletion success.", rif_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_router_interface_attribute_set (
sai_fib_router_interface_t *p_rif, uint_t attr_flags)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    sai_npu_object_id_t rif_id = 0;

    STD_ASSERT (p_rif != NULL);

    SAI_RIF_LOG_TRACE ("RIF attribute set, attr_flags: 0x%x", attr_flags);

    sai_vm_rif_log_trace (p_rif, "RIF info on attribute set");

    /* Update RIF record in DB with attribute set info. */
    rif_id = sai_uoid_npu_obj_id_get (p_rif->rif_id);

    sai_rc = sai_routerintf_set_db_entry (p_rif, attr_flags);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_RIF_LOG_ERR ("Error updating entry to DB for RIF ID %ld, "
                         "OBJ ID: 0x%"PRIx64".", rif_id, p_rif->rif_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_router_interface_attribute_get (
sai_fib_router_interface_t *p_rif, uint_t attr_cnt, sai_attribute_t *attr_list)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    uint_t           list_idx = 0;
    sai_attribute_t *p_attr = NULL;

    SAI_RIF_LOG_TRACE ("NPU RIF attribute get, attr_count: %d", attr_cnt);

    STD_ASSERT (p_rif != NULL);
    STD_ASSERT (attr_list != NULL);

    sai_vm_rif_log_trace (p_rif, "RIF info on NPU attribute get");

    for (list_idx = 0; list_idx < attr_cnt; list_idx++) {
        p_attr = &attr_list [list_idx];

        SAI_RIF_LOG_TRACE ("Getting attr_list [%d], Attribute id: %d.",
                           list_idx, p_attr->id);

        switch (p_attr->id) {
            case SAI_ROUTER_INTERFACE_ATTR_PORT_ID:
                sai_rc = sai_vm_fib_rif_port_get_fill (p_rif, p_attr);
                break;

            case SAI_ROUTER_INTERFACE_ATTR_VLAN_ID:
                sai_rc = sai_vm_fib_rif_vlan_get_fill (p_rif, p_attr);
                break;

            case SAI_ROUTER_INTERFACE_ATTR_TYPE:
                p_attr->value.s32 = p_rif->type;
                break;

            case SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID:
                p_attr->value.oid =
                    sai_uoid_create (SAI_OBJECT_TYPE_VIRTUAL_ROUTER,
                                     (sai_npu_object_id_t)p_rif->vrf_id);

                break;

            case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
                p_attr->value.booldata = p_rif->v4_admin_state;
                break;

            case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE:
                p_attr->value.booldata = p_rif->v6_admin_state;
                break;

            case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
                memcpy (p_attr->value.mac, p_rif->src_mac, sizeof (sai_mac_t));
                break;

            case SAI_ROUTER_INTERFACE_ATTR_MTU:
                if ((sai_npu_rif_attr_validate (p_rif->type, p_attr)) !=
                    SAI_STATUS_SUCCESS) {
                    SAI_RIF_LOG_ERR ("MTU attribute is not supported for "
                                     "RIF type: %d (%s).", p_rif->type,
                                     sai_fib_rif_type_to_str (p_rif->type));

                    sai_rc = SAI_STATUS_ATTR_NOT_SUPPORTED_0;
                } else {
                    p_attr->value.u32 = p_rif->mtu;
                }

                break;

            case SAI_ROUTER_INTERFACE_ATTR_INGRESS_ACL:
            case SAI_ROUTER_INTERFACE_ATTR_EGRESS_ACL:
            case SAI_ROUTER_INTERFACE_ATTR_V4_MCAST_ENABLE:
            case SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE:
                sai_rc = SAI_STATUS_ATTR_NOT_IMPLEMENTED_0;
                break;

            default:
                SAI_RIF_LOG_ERR ("Attribute id [%d]: %d is not known.",
                                 list_idx, p_attr->id);

                sai_rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
        }

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_ERR ("Failed to get RIF attr_list [%d], ID: %d, "
                             "Error: %d.", list_idx, p_attr->id, sai_rc);

            return (sai_fib_attr_status_code_get (sai_rc, list_idx));
        }
    }

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_RIF_LOG_ERR ("Failed to get RIF attributes.");
    } else {
        SAI_RIF_LOG_TRACE ("NPU RIF attribute get success.");
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_rif_lag_update (sai_fib_router_interface_t *p_rif_node,
                                            const sai_object_list_t *p_port_list,
                                            bool is_add)
{
    return SAI_STATUS_SUCCESS;
}

static sai_npu_rif_api_t sai_vm_rif_api_table = {
    sai_npu_router_interface_create,
    sai_npu_router_interface_remove,
    sai_npu_router_interface_attribute_set,
    sai_npu_router_interface_attribute_get,
    sai_npu_rif_attr_validate,
    sai_npu_rif_lag_update
};

sai_npu_rif_api_t* sai_vm_rif_api_query (void)
{
    return &sai_vm_rif_api_table;
}

