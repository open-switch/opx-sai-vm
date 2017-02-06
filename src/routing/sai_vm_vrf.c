/*
 * Copyright (c) 2016 Dell Inc.
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
 * @file sai_vm_vrf.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI Virtual Router object in VM environment.
 */

#include "sai_vm_defs.h"
#include "sai_switch_utils.h"
#include "sai_routing_db_api.h"
#include "sai_switch_db_api.h"
#include "sai_oid_utils.h"
#include "sai_l3_api.h"
#include "sai_l3_common.h"
#include "sai_l3_util.h"
#include "sai_oid_utils.h"
#include "sairouter.h"
#include "saitypes.h"
#include "saistatus.h"
#include "ds_common_types.h"
#include "std_bit_masks.h"
#include "std_mac_utils.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include <inttypes.h>
#include <string.h>

static inline void sai_vm_vrf_log_trace (sai_fib_vrf_t *p_vrf_node,
                                         char *p_info_str)
{
    char p_buf [SAI_FIB_MAX_BUFSZ];

    SAI_ROUTER_LOG_TRACE ("%s, VR Obj Id: 0x%"PRIx64", VRF ID: %d, V4 admin "
                          "state: %s, V6 admin state: %s, IP Options packet"
                          " action: %d (%s), MAC: %s, RIF count: %d.",
                          p_info_str, p_vrf_node->vrf_id,
                          sai_uoid_npu_obj_id_get (p_vrf_node->vrf_id),
                          (p_vrf_node->v4_admin_state)? "ON" : "OFF",
                          (p_vrf_node->v6_admin_state)? "ON" : "OFF",
                          p_vrf_node->ip_options_pkt_action,
                          sai_packet_action_str
                          (p_vrf_node->ip_options_pkt_action),
                          std_mac_to_string
                          ((const hal_mac_addr_t *)&p_vrf_node->src_mac,
                           p_buf, SAI_FIB_MAX_BUFSZ), p_vrf_node->num_rif);
}

static sai_status_t sai_npu_vr_create (sai_fib_vrf_t *p_vrf,
                                sai_npu_object_id_t *p_vr_id)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    int             vrf_id = 0;
    sai_object_id_t vrf_obj_id = 0;

    SAI_ROUTER_LOG_TRACE ("NPU VRF Creation API.");

    STD_ASSERT (p_vrf != NULL);
    STD_ASSERT (p_vr_id != NULL);

    vrf_id =
        std_find_first_bit (sai_vm_access_vrf_bitmap (), SAI_VM_MAX_VRF, 0);

    if ((vrf_id < 0) || (!(sai_fib_is_vrf_id_valid (vrf_id)))) {
        SAI_ROUTER_LOG_ERR ("VRF ID: %d. Valid VRF range is <0 - %d>, "
                            "Free VRF ID not available.", vrf_id,
                            ((sai_fib_max_virtual_routers_get ()) - 1));

        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    /* Insert VRF record to DB. */
    vrf_obj_id = sai_uoid_create (SAI_OBJECT_TYPE_VIRTUAL_ROUTER,
                                 (sai_npu_object_id_t) vrf_id);

    sai_rc = sai_router_create_db_entry (vrf_obj_id, p_vrf);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ROUTER_LOG_ERR ("Error inserting entry to DB for VRF ID %d, "
                            "OBJ ID: 0x%"PRIx64".", vrf_id, vrf_obj_id);

        return SAI_STATUS_FAILURE;
    }

    *p_vr_id = (sai_npu_object_id_t) vrf_id;

    STD_BIT_ARRAY_CLR (sai_vm_access_vrf_bitmap (), vrf_id);

    SAI_ROUTER_LOG_TRACE ("VRF Creation success, VRF ID: %d.", *p_vr_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_vr_remove (sai_fib_vrf_t *p_vrf)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    uint_t          vrf_id = 0;

    STD_ASSERT (p_vrf != NULL);

    vrf_id = (uint_t) sai_uoid_npu_obj_id_get (p_vrf->vrf_id);

    SAI_ROUTER_LOG_TRACE ("VRF Deletion, VRF ID: %d.", vrf_id);

    /* Remove VRF record from DB. */
    sai_rc = sai_router_delete_db_entry (p_vrf->vrf_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ROUTER_LOG_ERR ("Error removing entry from DB for VRF ID %d, "
                            "OBJ ID: 0x%"PRIx64".", vrf_id, p_vrf->vrf_id);

        return SAI_STATUS_FAILURE;
    }

    STD_BIT_ARRAY_SET (sai_vm_access_vrf_bitmap (), vrf_id);

    SAI_ROUTER_LOG_TRACE ("VRF ID %d moved to free pool.", vrf_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_vr_attribute_set (sai_fib_vrf_t *p_vrf, uint_t attr_flags)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    sai_npu_object_id_t vrf_id = 0;

    STD_ASSERT (p_vrf != NULL);

    vrf_id = sai_uoid_npu_obj_id_get (p_vrf->vrf_id);

    SAI_ROUTER_LOG_TRACE ("VRF attribute set, attr_flags: 0x%x", attr_flags);

    /* Update VRF record in DB with attribute set info. */
    sai_rc = sai_router_set_db_entry (p_vrf, attr_flags);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ROUTER_LOG_ERR ("Error updating entry to DB for VRF ID %d, "
                            "OBJ ID: 0x%"PRIx64".", vrf_id, p_vrf->vrf_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_vr_attribute_get (sai_fib_vrf_t *p_vrf_node,
                                       uint_t attr_count,
                                       sai_attribute_t *attr_list)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    uint_t           list_idx = 0;
    sai_attribute_t *p_attr = NULL;
    char             p_buf [SAI_VM_MAX_BUFSZ];

    STD_ASSERT (p_vrf_node != NULL);
    STD_ASSERT (attr_list != NULL);

    SAI_ROUTER_LOG_TRACE ("VRF attribute get, attr_count: %d", attr_count);

    sai_vm_vrf_log_trace (p_vrf_node, "VRF info on NPU attribute get");

    for (list_idx = 0; list_idx < attr_count; list_idx++) {
        p_attr = &attr_list [list_idx];

        SAI_ROUTER_LOG_TRACE ("Getting attr_list [%d], Attribute id: %d.",
                              list_idx, p_attr->id);

        switch (p_attr->id) {
            case SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS:
                memcpy (&p_attr->value.mac, p_vrf_node->src_mac,
                        sizeof (sai_mac_t));

                SAI_ROUTER_LOG_TRACE ("VRF MAC: %s.", std_mac_to_string
                                      ((const hal_mac_addr_t *)
                                       &p_attr->value.mac, p_buf,
                                       SAI_VM_MAX_BUFSZ));

                break;

            case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE:
                p_attr->value.booldata = p_vrf_node->v4_admin_state;

                SAI_ROUTER_LOG_TRACE ("VRF V4 Admin state: %s.",
                                      (p_attr->value.booldata)? "ON" :
                                      "OFF");

                break;

            case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE:
                p_attr->value.booldata = p_vrf_node->v6_admin_state;

                SAI_ROUTER_LOG_TRACE ("VRF V6 Admin state: %s.",
                                      (p_attr->value.booldata)? "ON" :
                                      "OFF");

                break;

            case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_ACTION:
                 p_attr->value.s32 = p_vrf_node->ttl0_1_pkt_action;

                 SAI_ROUTER_LOG_TRACE ("TTL1 Violation pkt action: %d "
                                       "(%s).", p_attr->value.s32,
                                       sai_packet_action_str
                                       (p_attr->value.s32));

                break;

            case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_IP_OPTIONS:
                p_attr->value.s32 = p_vrf_node->ip_options_pkt_action;

                SAI_ROUTER_LOG_TRACE ("IP Options pkt action: %d (%s).",
                                      p_attr->value.s32,
                                      sai_packet_action_str
                                      (p_attr->value.s32));

                break;

            default:
                SAI_ROUTER_LOG_ERR ("Attribute id [%d]: %d is not a known "
                                    "attribute for VRF functionality.",
                                    list_idx, p_attr->id);

                sai_rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
        }

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ROUTER_LOG_ERR ("Failed to get VRF attr_list [%d], "
                                "Attribute Id: %d, Error: %d.",
                                list_idx, p_attr->id, sai_rc);

            return (sai_fib_attr_status_code_get (sai_rc, list_idx));
        }
    }

    return sai_rc;
}

static sai_status_t sai_npu_fib_init (void)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_fib_router_mac_set (const sai_mac_t *p_router_mac)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;
    char            p_buf [SAI_VM_MAX_BUFSZ];

    STD_ASSERT (p_router_mac != NULL);

    SAI_RIF_LOG_TRACE ("Setting Router MAC as %s",
                       std_mac_to_string
                       (p_router_mac, p_buf, SAI_VM_MAX_BUFSZ));

    if (sai_fib_is_router_configured ()) {
        SAI_RIF_LOG_ERR ("Router Configuration exists already.");

        return SAI_STATUS_FAILURE;
    }

    /* Update the Switch DB entry with this attribute info. */
    attr.id = SAI_SWITCH_ATTR_SRC_MAC_ADDRESS;
    memcpy (&attr.value.mac, p_router_mac, sizeof (sai_mac_t));

    sai_rc = sai_switch_attribute_set_db_entry (sai_vm_switch_id_get(), &attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_TRACE ("Failed to update the switch attribute %d in "
                              "DB table.", attr.id);
    }

    return sai_rc;
}

static sai_status_t sai_npu_vr_attr_validate (const sai_attribute_t *p_attr)
{
    STD_ASSERT (p_attr != NULL);

    SAI_RIF_LOG_TRACE ("validating VR attribute ID: %d.",
                       p_attr->id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_ecmp_max_paths_set (uint_t max_paths)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    SAI_SWITCH_LOG_TRACE ("ECMP Max Paths set to %d.", max_paths);

    /* Update the Switch DB entry with this attribute info. */
    attr.id = SAI_SWITCH_ATTR_ECMP_MEMBERS;
    attr.value.u32 = max_paths;

    sai_rc = sai_switch_attribute_set_db_entry (sai_vm_switch_id_get(), &attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_TRACE ("Failed to update the switch attribute %d in "
                              "DB table.", attr.id);
    }

    return sai_rc;
}

static sai_npu_router_api_t sai_vm_router_api_table = {
    sai_npu_vr_create,
    sai_npu_vr_remove,
    sai_npu_vr_attribute_set,
    sai_npu_vr_attribute_get,
    sai_npu_vr_attr_validate,
    sai_npu_fib_init,
    sai_npu_fib_router_mac_set,
    sai_npu_ecmp_max_paths_set
};

sai_npu_router_api_t* sai_vm_router_api_query (void)
{
    return &sai_vm_router_api_table;
}

