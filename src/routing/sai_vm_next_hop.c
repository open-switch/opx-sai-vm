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
 * @file sai_vm_next_hop.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI Next-hop object in VM environment.
 */

#include "sai_vm_defs.h"
#include "sai_routing_db_api.h"
#include "sai_oid_utils.h"
#include "sai_l3_api.h"
#include "sai_l3_util.h"
#include "sai_l3_common.h"
#include "sainexthop.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_bit_masks.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include <inttypes.h>

static sai_status_t sai_npu_next_hop_create (sai_fib_nh_t *p_next_hop,
                                      sai_npu_object_id_t *p_next_hop_id)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    int             nh_id = 0;
    sai_object_id_t nh_obj_id = 0;

    STD_ASSERT (p_next_hop != NULL);
    STD_ASSERT (p_next_hop_id != NULL);

    SAI_NEXTHOP_LOG_TRACE ("Next Hop creation.");

    nh_id = std_find_first_bit (sai_vm_access_nh_bitmap (),
                                SAI_VM_NH_TABLE_SIZE, 0);

    if (nh_id < 0) {
        SAI_NEXTHOP_LOG_ERR ("Next Hop Table is full.");

        return SAI_STATUS_TABLE_FULL;
    }

    /* Insert Next Hop record to DB. */
    nh_obj_id =
        sai_uoid_create (SAI_OBJECT_TYPE_NEXT_HOP, (sai_npu_object_id_t) nh_id);

    sai_rc = sai_nexthop_create_db_entry (nh_obj_id, p_next_hop);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_NEXTHOP_LOG_ERR ("Error inserting entry to DB for NH ID %d, "
                             "OBJ ID: 0x%"PRIx64".", nh_id, nh_obj_id);

        return SAI_STATUS_FAILURE;
    }

    *p_next_hop_id = (sai_npu_object_id_t) nh_id;

    STD_BIT_ARRAY_CLR (sai_vm_access_nh_bitmap (), nh_id);

    SAI_NEXTHOP_LOG_TRACE ("Next Hop creation sucessful. NH Id: 0x%"PRIx64".",
                           (*p_next_hop_id));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_next_hop_remove (sai_fib_nh_t *p_next_hop)
{
    sai_status_t         sai_rc = SAI_STATUS_SUCCESS;
    sai_npu_object_id_t  nh_id = 0;

    STD_ASSERT (p_next_hop != NULL);

    nh_id = sai_uoid_npu_obj_id_get (p_next_hop->next_hop_id);

    SAI_NEXTHOP_LOG_TRACE ("Next Hop Deletion, Next Hop ID: %d.", nh_id);

    /* Remove Next Hop record from DB. */
    sai_rc = sai_nexthop_delete_db_entry (p_next_hop->next_hop_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_NEXTHOP_LOG_ERR ("Error removing entry from DB for NH ID %d, "
                             "OBJ ID: 0x%"PRIx64".", nh_id,
                             p_next_hop->next_hop_id);

        return SAI_STATUS_FAILURE;
    }

    STD_BIT_ARRAY_SET (sai_vm_access_nh_bitmap (), nh_id);

    SAI_NEXTHOP_LOG_TRACE ("Next Hop ID %d moved to free pool.", nh_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_next_hop_attribute_get (sai_fib_nh_t *p_next_hop,
                                                    uint_t attr_count,
                                                    sai_attribute_t *p_attr_list)
{
    sai_status_t      status = SAI_STATUS_SUCCESS;
    sai_attribute_t  *p_attr = NULL;
    uint_t            attr_index = 0;
    char              ip_addr_str [SAI_VM_MAX_BUFSZ];

    STD_ASSERT (p_next_hop != NULL);
    STD_ASSERT (p_attr_list != NULL);

    SAI_NEXTHOP_LOG_TRACE ("Next Hop attribute get request for NH ID: %d, "
                           "attr_count: %d.", p_next_hop->next_hop_id,
                           attr_count);

    for (attr_index = 0; attr_index < attr_count; attr_index++)
    {
        p_attr = &p_attr_list [attr_index];

        switch (p_attr->id) {
            case SAI_NEXT_HOP_ATTR_TYPE:
                p_attr->value.s32 = p_next_hop->key.nh_type;

                SAI_NEXTHOP_LOG_TRACE ("Index: %d, NH type: %s.", attr_index,
                                       sai_fib_next_hop_type_str
                                       (p_attr->value.s32));

                break;

            case SAI_NEXT_HOP_ATTR_IP:
                memset (&p_attr->value.ipaddr, 0, sizeof (sai_ip_address_t));

                sai_fib_ip_addr_copy (&p_attr->value.ipaddr,
                                      sai_fib_next_hop_ip_addr (p_next_hop));

                sai_ip_addr_to_str (&p_attr->value.ipaddr, ip_addr_str,
                                    SAI_VM_MAX_BUFSZ);

                SAI_NEXTHOP_LOG_TRACE ("Index: %d, Next Hop IP: %s.",
                                       attr_index, ip_addr_str);

                break;

            case SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID:
                p_attr->value.oid = p_next_hop->key.rif_id;

                SAI_NEXTHOP_LOG_TRACE ("Index: %d, NH RIF Id: 0x%"PRIx64".",
                                       attr_index, p_attr->value.oid);

                break;

            default:
                status = SAI_STATUS_INVALID_ATTRIBUTE_0;

                SAI_NEXTHOP_LOG_ERR ("Index: %d, Invalid attribute Id: %d.",
                                     attr_index, p_attr->id);

                break;
        }

        if (status != SAI_STATUS_SUCCESS) {
            SAI_NEXTHOP_LOG_ERR ("Failure in filling Next Hop attr list at "
                                 "index: %d for Next Hop Id: %d.",
                                 attr_index, p_next_hop->next_hop_id);

            return (sai_fib_attr_status_code_get (status, attr_index));
        }
    }

    SAI_NEXTHOP_LOG_TRACE ("Next Hop attribute get for NH ID: %d is "
                           "successful.", p_next_hop->next_hop_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_tunnel_encap_nh_route_resolve (sai_fib_nh_t *p_encap_nh,
                                                    sai_fib_route_t *p_route)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_tunnel_encap_nh_neighbor_resolve (sai_fib_nh_t *p_encap_nh,
                                                       sai_fib_nh_t *p_neighbor)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_tunnel_encap_nh_neighbor_attr_set (sai_fib_nh_t *p_encap_nh,
                                                        sai_fib_nh_t *p_neighbor,
                                                        sai_fib_nh_t *p_neighbor_info,
                                                        uint_t attr_flags)
{
    return SAI_STATUS_SUCCESS;
}

static sai_npu_nexthop_api_t sai_vm_nh_api_table = {
    sai_npu_next_hop_create,
    sai_npu_next_hop_remove,
    sai_npu_next_hop_attribute_get,
    sai_npu_tunnel_encap_nh_route_resolve,
    sai_npu_tunnel_encap_nh_neighbor_resolve,
    sai_npu_tunnel_encap_nh_neighbor_attr_set
};

sai_npu_nexthop_api_t* sai_vm_nexthop_api_query (void)
{
    return &sai_vm_nh_api_table;
}

