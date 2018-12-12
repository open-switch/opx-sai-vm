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
 * @file sai_vm_next_hop_group.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI Next-hop Group object in VM environment.
 */

#include "sai_vm_defs.h"
#include "sai_routing_db_api.h"
#include "sai_oid_utils.h"
#include "sai_l3_api.h"
#include "sai_l3_util.h"
#include "sai_l3_common.h"
#include "sainexthopgroup.h"
#include "saiswitch.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_bit_masks.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include <inttypes.h>
#include <stdlib.h>

static sai_status_t sai_npu_next_hop_group_create (sai_fib_nh_group_t *p_group,
                                                   sai_npu_object_id_t *p_group_id)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    int             nh_grp_id = 0;
    sai_object_id_t nh_grp_obj_id = 0;

    STD_ASSERT (p_group != NULL);
    STD_ASSERT (p_group_id != NULL);

    SAI_NH_GROUP_LOG_TRACE ("NH Group creation. Type: %s.",
                            sai_fib_nh_group_type_str (p_group->type));

    nh_grp_id = std_find_first_bit (sai_vm_access_nh_grp_bitmap (),
                                    SAI_VM_NH_GRP_TABLE_SIZE, 0);

    if (nh_grp_id < 0) {
        SAI_NH_GROUP_LOG_ERR ("NH Group Table is full.");

        return SAI_STATUS_TABLE_FULL;
    }

    /* Insert Next Hop Group record to DB. */
    nh_grp_obj_id = sai_uoid_create (SAI_OBJECT_TYPE_NEXT_HOP_GROUP,
                                     (sai_npu_object_id_t) nh_grp_id);

    sai_rc = sai_nh_group_create_db_entry (nh_grp_obj_id, p_group->type);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_NH_GROUP_LOG_ERR ("Error inserting entry to DB for NH Group ID:"
                              " %d, OBJ ID: 0x%"PRIx64".", nh_grp_id,
                              nh_grp_obj_id);

        return SAI_STATUS_FAILURE;
    }

    *p_group_id = (sai_npu_object_id_t) nh_grp_id;

    STD_BIT_ARRAY_CLR (sai_vm_access_nh_grp_bitmap (), nh_grp_id);

    SAI_NH_GROUP_LOG_TRACE ("NH Group creation sucessful. NH Group Id: "
                            "0x%"PRIx64".", (*p_group_id));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_next_hop_group_remove (sai_fib_nh_group_t *p_group)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    sai_npu_object_id_t nh_grp_id = 0;

    STD_ASSERT (p_group != NULL);

    nh_grp_id = (uint_t) sai_uoid_npu_obj_id_get (p_group->key.group_id);

    SAI_NH_GROUP_LOG_TRACE ("NH Group Deletion, ID: 0x%"PRIx64".", nh_grp_id);

    /* Remove Next Hop Group record from DB. */
    sai_rc = sai_nh_group_delete_db_entry (p_group->key.group_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_NH_GROUP_LOG_ERR ("Error removing entry from DB for NH GRP ID: "
                              "0x%"PRIx64", OBJ ID: 0x%"PRIx64".", nh_grp_id,
                              p_group->key.group_id);

        return SAI_STATUS_FAILURE;
    }

    STD_BIT_ARRAY_SET (sai_vm_access_nh_grp_bitmap (), nh_grp_id);

    SAI_NH_GROUP_LOG_TRACE ("NH Group ID 0x%"PRIx64" moved to free pool.", nh_grp_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_next_hop_add_to_group (sai_fib_nh_group_t *p_group,
                                                   uint_t next_hop_count,
                                                   sai_fib_nh_t *ap_next_hop [])
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT (p_group != NULL);
    STD_ASSERT (ap_next_hop != NULL);

    SAI_NH_GROUP_LOG_TRACE ("Add NH to group 0x%"PRIx64", NH Count: %d.",
                            p_group->key.group_id, next_hop_count);

    /*
     * Update the Next Hop Group DB and Group NH List DB records with newly
     * added Next Hops and the next-hop count.
     */
    sai_rc = sai_nh_group_add_nh_list_to_db_entry (p_group->key.group_id,
                                                   next_hop_count, ap_next_hop,
                                                   p_group->nh_count);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_NH_GROUP_LOG_TRACE ("Failed to update DB entry for NH group ID:"
                                " 0x%"PRIx64".", p_group->key.group_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_next_hop_remove_from_group (sai_fib_nh_group_t *p_group,
                                                        uint_t next_hop_count,
                                                        sai_fib_nh_t *ap_next_hop [])
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT (p_group != NULL);
    STD_ASSERT (ap_next_hop != NULL);

    SAI_NH_GROUP_LOG_TRACE ("Remove NH from group 0x%"PRIx64", NH Count: %d.",
                            p_group->key.group_id, next_hop_count);

    /*
     * Update the Next Hop Group DB and Group NH List DB records with deleted
     * Next Hops and the next-hop count.
     */
    sai_rc =
        sai_nh_group_delete_nh_list_from_db_entry (p_group->key.group_id,
                                                   next_hop_count, ap_next_hop,
                                                   (p_group->nh_count -
                                                    next_hop_count));

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_NH_GROUP_LOG_TRACE ("Failed to update DB entry for NH group ID:"
                                " 0x%"PRIx64".", p_group->key.group_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_npu_nh_group_api_t sai_vm_nh_group_api_table = {
    sai_npu_next_hop_group_create,
    sai_npu_next_hop_group_remove,
    sai_npu_next_hop_add_to_group,
    sai_npu_next_hop_remove_from_group,
};

sai_npu_nh_group_api_t* sai_vm_nh_group_api_query (void)
{
    return &sai_vm_nh_group_api_table;
}

