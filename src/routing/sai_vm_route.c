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
 * @file sai_vm_route.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI Route object in VM environment.
 */

#include "sai_vm_defs.h"
#include "sai_routing_db_api.h"
#include "sai_l3_api.h"
#include "sai_l3_util.h"
#include "sai_l3_common.h"
#include "sairoute.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_radix.h"
#include "std_bit_masks.h"
#include "std_struct_utils.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include <inttypes.h>

static inline void sai_vm_route_log_trace (sai_fib_route_t *p_route,
                                           const char *p_trace_str)
{
    char addr_str [SAI_VM_MAX_BUFSZ];

    STD_ASSERT (p_route != NULL);

    SAI_ROUTE_LOG_TRACE ("%s VRF: 0x%"PRIx64", Prefix: %s/%d, NH type: %d, "
                         "NH Id: 0x%"PRIx64", Packet action: %s.",
                         p_trace_str, p_route->vrf_id, sai_ip_addr_to_str (
                         &p_route->key.prefix, addr_str, sizeof (addr_str)),
                         p_route->prefix_len,
                         sai_fib_route_nh_type_to_str (p_route->nh_type),
                         sai_fib_route_node_nh_id_get (p_route),
                         sai_packet_action_str (p_route->packet_action));
}

static sai_status_t sai_npu_route_create (sai_fib_route_t *p_route_in)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_fib_route_t *p_route_node = NULL;
    sai_fib_vrf_t   *p_vrf_node = NULL;
    uint_t           key_len = 0;
    uint_t           attr_flag = 0;

    STD_ASSERT (p_route_in != NULL);

    sai_vm_route_log_trace (p_route_in, "Route create.");

    /*
     * Check if Route node with the same IP Prefix already exists in SAI DB.
     */
    p_vrf_node = sai_fib_vrf_node_get (p_route_in->vrf_id);

    STD_ASSERT (p_vrf_node != NULL);

    key_len = (STD_STR_SIZE_OF(sai_ip_address_t, addr_family) * BITS_PER_BYTE)
        + p_route_in->prefix_len;

    p_route_node =
        (sai_fib_route_t *) std_radix_getexact (p_vrf_node->sai_route_tree,
                                                (uint8_t *)&p_route_in->key,
                                                key_len);

    if (!p_route_node) {
        /* Insert Route record to DB. */
        sai_rc = sai_route_create_db_entry (p_route_in);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ROUTE_LOG_ERR ("Error inserting Route entry in DB.");

            return SAI_STATUS_FAILURE;
        }
    } else {
        /* Update existing Route record in DB with attribute set info. */
        if (p_route_node->packet_action != p_route_in->packet_action) {
            attr_flag = SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION;
        } else if (p_route_node->trap_priority != p_route_in->trap_priority) {
            attr_flag = SAI_ROUTE_ENTRY_ATTR_TRAP_PRIORITY;
        } else {
            attr_flag = SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID;
        }

        sai_rc = sai_route_set_db_entry (p_route_in, attr_flag);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ROUTE_LOG_ERR ("Error %d on updating Route entry in DB.",
                               sai_rc);

            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_route_remove (sai_fib_route_t *p_route)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT (p_route != NULL);

    sai_vm_route_log_trace (p_route, "Route Remove.");

        /* Remove Route record from DB. */
    sai_rc = sai_route_delete_db_entry (p_route);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ROUTE_LOG_ERR ("Error %d on removing Route entry from DB.",
                           sai_rc);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_route_attribute_set (sai_fib_route_t *p_route_in,
                                                 uint_t attr_count,
                                                 const sai_attribute_t *p_attr_list)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    uint_t           attr_index = 0;
    char             addr_str [SAI_VM_MAX_BUFSZ];
    uint_t           attr_flag = 0;

    STD_ASSERT (p_route_in != NULL);
    STD_ASSERT (p_attr_list != NULL);

    SAI_ROUTE_LOG_TRACE ("Entering Route attribute set request, attr_count: "
                         "%d, VRF: 0x%"PRIx64", Prefix: %s/%d.", attr_count,
                         p_route_in->vrf_id, sai_ip_addr_to_str
                         (&p_route_in->key.prefix, addr_str,
                         SAI_VM_MAX_BUFSZ), p_route_in->prefix_len);

    for (attr_index = 0; attr_index < attr_count; attr_index++)
    {
        attr_flag |= p_attr_list [attr_index].id;
    }

    /* Update existing Route record in DB with attribute set info. */
    sai_rc = sai_route_set_db_entry (p_route_in, attr_flag);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ROUTE_LOG_ERR ("Error %d on updating Route entry in DB.",
                           sai_rc);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_route_attribute_get (sai_fib_route_t *p_route,
                                                 uint_t attr_count,
                                                 sai_attribute_t *p_attr_list)
{
    sai_status_t     status = SAI_STATUS_SUCCESS;
    uint_t           attr_index = 0;
    sai_attribute_t *p_attr = NULL;
    char             addr_str [SAI_VM_MAX_BUFSZ];

    STD_ASSERT (p_route != NULL);
    STD_ASSERT (p_attr_list != NULL);

    SAI_ROUTE_LOG_TRACE ("Entering Route attribute get request, attr_count: "
                         "%d, VRF: 0x%"PRIx64", Prefix: %s/%d.", attr_count,
                         p_route->vrf_id, sai_ip_addr_to_str
                         (&p_route->key.prefix, addr_str,
                         SAI_VM_MAX_BUFSZ), p_route->prefix_len);

    for (attr_index = 0; attr_index < attr_count; attr_index++)
    {
        p_attr = &p_attr_list [attr_index];

        switch (p_attr->id) {
            case SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID:
                p_attr->value.oid = sai_fib_route_node_nh_id_get (p_route);

                SAI_ROUTE_LOG_TRACE ("List index: %d, NH OBJ ID: 0x%"PRIx64".",
                                     attr_index, p_attr->value.oid);

                break;

            case SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION:
                p_attr->value.s32 = p_route->packet_action;

                SAI_ROUTE_LOG_TRACE ("List index: %d, Pkt action: %d (%s).",
                                     attr_index, p_attr->value.s32,
                                     sai_packet_action_str
                                     (p_attr->value.s32));

                break;

            case SAI_ROUTE_ENTRY_ATTR_TRAP_PRIORITY:
                p_attr->value.u8 = p_route->trap_priority;

                SAI_ROUTE_LOG_TRACE ("List index: %d, Trap Priority: %d.",
                                     attr_index, p_attr->value.u8);
                break;

            default:
                status = SAI_STATUS_INVALID_ATTRIBUTE_0;

                SAI_ROUTE_LOG_ERR ("List idx: %d, Invalid attribute Id: %d.",
                                   attr_index, p_attr->id);
                break;
        }

        if (status != SAI_STATUS_SUCCESS) {
            SAI_ROUTE_LOG_ERR ("Failure in filling Route attr list at "
                               "index: %d.", attr_index);

            return (sai_fib_attr_status_code_get (status, attr_index));
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_npu_route_api_t sai_vm_route_api_table = {
    sai_npu_route_create,
    sai_npu_route_remove,
    sai_npu_route_attribute_set,
    sai_npu_route_attribute_get,
};

sai_npu_route_api_t* sai_vm_route_api_query (void)
{
    return &sai_vm_route_api_table;
}

