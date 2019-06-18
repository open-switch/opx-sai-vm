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
 * @file sai_vm_neighbor.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI Neighbor object in VM environment.
 */

#include "sai_vm_defs.h"
#include "sai_vm_l2_util.h"
#include "sai_routing_db_api.h"
#include "sai_l3_api.h"
#include "sai_l3_util.h"
#include "sai_l3_common.h"
#include "sai_npu_fdb.h"
#include "saineighbor.h"
#include "saitypes.h"
#include "saistatus.h"
#include "ds_common_types.h"
#include "std_mac_utils.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include "sai_vlan_api.h"
#include "sai_bridge_api.h"
#include <inttypes.h>

static inline void sai_vm_neighbor_log_trace (sai_fib_nh_t *p_next_hop,
                                              const char *p_trace_str)
{
    char mac_str [SAI_VM_MAX_BUFSZ];
    char addr_str [SAI_VM_MAX_BUFSZ];

    SAI_NEIGHBOR_LOG_TRACE ("%s IP: %s, RIF: 0x%"PRIx64", MAC: %s, Pkt action:"
                            " %s.", p_trace_str, sai_ip_addr_to_str
                            (sai_fib_next_hop_ip_addr (p_next_hop), addr_str,
                             sizeof (addr_str)), p_next_hop->key.rif_id,
                            std_mac_to_string
                            ((const hal_mac_addr_t *)&p_next_hop->mac_addr,
                             mac_str, SAI_VM_MAX_BUFSZ),
                            sai_packet_action_str
                            (p_next_hop->packet_action));
}

static inline void sai_vm_neighbor_log_error (sai_fib_nh_t *p_next_hop,
                                              const char *p_error_str)
{
    char mac_str [SAI_VM_MAX_BUFSZ];
    char addr_str [SAI_VM_MAX_BUFSZ];

    SAI_NEIGHBOR_LOG_ERR ("%s IP: %s, RIF: 0x%"PRIx64", MAC: %s, Pkt action:"
                          " %s.", p_error_str, sai_ip_addr_to_str
                          (sai_fib_next_hop_ip_addr (p_next_hop), addr_str,
                           sizeof (addr_str)), p_next_hop->key.rif_id,
                          std_mac_to_string
                          ((const hal_mac_addr_t *)&p_next_hop->mac_addr,
                           mac_str, SAI_VM_MAX_BUFSZ),
                          sai_packet_action_str
                          (p_next_hop->packet_action));
}

static sai_status_t sai_npu_neighbor_create (sai_fib_nh_t *p_next_hop)
{
    sai_status_t                status = SAI_STATUS_SUCCESS;
    sai_fib_router_interface_t *p_rif = NULL;
    sai_object_id_t             rif_obj_id = 0;

    STD_ASSERT (p_next_hop != NULL);

    sai_vm_neighbor_log_trace (p_next_hop, "Creating Neighbor");

    rif_obj_id =  p_next_hop->key.rif_id;

    p_rif = sai_fib_router_interface_node_get (rif_obj_id);

    if ((!p_rif)) {
        SAI_NEIGHBOR_LOG_ERR ("RIF node not found for RIF Id: 0x%"PRIx64".",
                              rif_obj_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    /* Insert Neighbor record to DB. */
    status = sai_neighbor_create_db_entry (p_next_hop);

    if (status != SAI_STATUS_SUCCESS) {
        sai_vm_neighbor_log_error (p_next_hop,
                                   "Error inserting Neighbor entry in DB.");

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_neighbor_remove (sai_fib_nh_t *p_next_hop)
{
    sai_status_t status = SAI_STATUS_SUCCESS;

    STD_ASSERT (p_next_hop != NULL);

    sai_vm_neighbor_log_trace (p_next_hop, "Removing Neighbor");

    /* Remove Neighbor record from DB. */
    status = sai_neighbor_delete_db_entry (p_next_hop);

    if (status != SAI_STATUS_SUCCESS) {
        sai_vm_neighbor_log_error (p_next_hop,
                                   "Error removing Neighbor entry from DB.");

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_neighbor_attribute_set (sai_fib_nh_t *p_next_hop,
                                                    uint_t attr_flags)
{
    sai_status_t status = SAI_STATUS_SUCCESS;

    STD_ASSERT (p_next_hop != NULL);

    sai_vm_neighbor_log_trace (p_next_hop, "Neighbor attribute set request");

    /* Update existing Neighbor record in DB with attribute set info. */
    status = sai_neighbor_set_db_entry (p_next_hop, attr_flags);

    if (status != SAI_STATUS_SUCCESS) {
        sai_vm_neighbor_log_error (p_next_hop,
                                   "Error setting Neighbor attributes in DB.");

        return SAI_STATUS_FAILURE;
    }

    SAI_NEIGHBOR_LOG_TRACE ("Neighbor attribute set success, attr_flags: "
                            "0x%x.", attr_flags);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_neighbor_attribute_get (sai_fib_nh_t *p_neighbor,
                                                    uint_t attr_count,
                                                    sai_attribute_t *p_attr_list)
{
    sai_status_t     status = SAI_STATUS_SUCCESS;
    uint_t           attr_index = 0;
    sai_attribute_t *p_attr = NULL;
    char             mac_str [SAI_VM_MAX_BUFSZ];

    STD_ASSERT (p_neighbor != NULL);
    STD_ASSERT (p_attr_list != NULL);

    SAI_NEIGHBOR_LOG_TRACE ("Neighbor attribute get request, attr_count: %d",
                            attr_count);

    for (attr_index = 0; attr_index < attr_count; attr_index++) {
        p_attr = &p_attr_list [attr_index];

        switch (p_attr->id) {
            case SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS:
                memcpy (p_attr->value.mac, p_neighbor->mac_addr,
                        sizeof (sai_mac_t));

                std_mac_to_string ((const hal_mac_addr_t *) &p_attr->value.mac,
                                   mac_str, SAI_VM_MAX_BUFSZ);

                SAI_NEIGHBOR_LOG_TRACE ("Index: %d, Neighbor Dest MAC: %s.",
                                        attr_index, mac_str);

                break;

            case SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION:
                p_attr->value.s32 = p_neighbor->packet_action;

                SAI_NEIGHBOR_LOG_TRACE ("Index: %d, Pkt action: %d (%s).",
                                        attr_index, p_attr->value.s32,
                                        sai_packet_action_str
                                        (p_attr->value.s32));

                break;

            case SAI_NEIGHBOR_ENTRY_ATTR_NO_HOST_ROUTE:
                p_attr->value.booldata = p_neighbor->no_host_route;

                SAI_NEIGHBOR_LOG_TRACE ("Index: %d, Neighbor No Host Route: %d."
                                        , attr_index, p_attr->value.booldata);
                break;
            case SAI_NEIGHBOR_ENTRY_ATTR_USER_TRAP_ID:
                status = SAI_STATUS_NOT_SUPPORTED;
                SAI_NEIGHBOR_LOG_ERR ("Unsupported attribute   Id: %d.",p_attr->id);
                break;
            default:
                status = SAI_STATUS_INVALID_ATTRIBUTE_0;

                SAI_NEIGHBOR_LOG_ERR ("Index: %d, Invalid attribute Id: %d.",
                                      attr_index, p_attr->id);

                break;
        }

        if (status != SAI_STATUS_SUCCESS) {
            SAI_NEIGHBOR_LOG_ERR ("Failure in filling Neighbor attr list at"
                                  " index: %d.", attr_index);

            return (sai_fib_attr_status_code_get (status, attr_index));
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_npu_neighbor_api_t sai_vm_neighbor_api_table = {
    sai_npu_neighbor_create,
    sai_npu_neighbor_remove,
    sai_npu_neighbor_attribute_set,
    sai_npu_neighbor_attribute_get
};

sai_npu_neighbor_api_t* sai_vm_neighbor_api_query (void)
{
    return &sai_vm_neighbor_api_table;
}

