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

static sai_status_t sai_vm_nh_group_resolved_nh_paths_fill (
sai_fib_nh_group_t *p_nh_group, sai_object_id_t *p_nh_obj_id_arr,
uint_t arr_size, uint_t *p_resolved_count)
{
    sai_fib_wt_link_node_t *p_nh_link_node = NULL;
    sai_fib_wt_link_node_t *p_next_nh_link_node = NULL;
    sai_fib_nh_t           *p_nh_node = NULL;
    uint_t                  path_count = 0;
    uint_t                  weight = 0;

    STD_ASSERT (p_nh_group != NULL);
    STD_ASSERT (p_nh_obj_id_arr != NULL);
    STD_ASSERT (p_resolved_count != NULL);

    SAI_NH_GROUP_LOG_TRACE ("Getting resolved forwarding NH paths for "
                            "Group ID: 0x%"PRIx64".",
                            p_nh_group->key.group_id);

    for (p_nh_link_node = sai_fib_get_first_nh_from_nh_group (p_nh_group);
         p_nh_link_node != NULL; p_nh_link_node = p_next_nh_link_node) {
        p_next_nh_link_node =
            sai_fib_get_next_nh_from_nh_group (p_nh_group, p_nh_link_node);

        p_nh_node =
            sai_fib_get_nh_from_dll_link_node (&p_nh_link_node->link_node);

        STD_ASSERT (p_nh_node != NULL);

        /*
         * Skip the next-hops that are not neighbors.
         * Also skip the next-hops that are neighbors but does not have
         * FORWARD/LOG packet action.
         */
        if ((!sai_fib_is_owner_neighbor (p_nh_node)) ||
            ((p_nh_node->packet_action != SAI_PACKET_ACTION_FORWARD) &&
            (p_nh_node->packet_action != SAI_PACKET_ACTION_LOG))) {
            SAI_NH_GROUP_LOG_TRACE ("NH ID: 0x%"PRIx64" is not a forwarding"
                                    "path. Moving to next NH ID.",
                                    p_nh_node->next_hop_id);
            continue;
        }

        for (weight = 0; weight < p_nh_link_node->weight; weight++) {
            SAI_NH_GROUP_LOG_TRACE ("NH ID: 0x%"PRIx64", weight: %d.",
                                    p_nh_node->next_hop_id,
                                    p_nh_link_node->weight);

            if (path_count == arr_size) {
                SAI_NH_GROUP_LOG_ERR ("Failed to fill NH list. Input NH list"
                                      "size: %d is less than the resolved "
                                      "NH count in NH Group.", arr_size);

                return SAI_STATUS_FAILURE;
            }

            p_nh_obj_id_arr [path_count] = p_nh_node->next_hop_id;
            path_count++;
        }
    }

    *p_resolved_count = path_count;

    SAI_NH_GROUP_LOG_TRACE ("Resolved NH Count: %d.", *p_resolved_count);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_fib_nh_list_fill (sai_object_list_t *p_nh_list,
                                             sai_object_id_t *p_nh_obj_id_arr,
                                             uint_t nh_count)
{
    uint_t             index = 0;

    if (p_nh_list->count < nh_count) {
        /* Less memory space in nh list buffer */
        SAI_NH_GROUP_LOG_ERR ("Failed to fill NH list, Input NH list buffer"
                              " NH count: %d is less than the resolved NH "
                              "count: %d in NH Group.", p_nh_list->count,
                              nh_count);

        /* Fill the actual nh count in input list */
        p_nh_list->count = nh_count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    memcpy (p_nh_list->list, p_nh_obj_id_arr,
            (sizeof (sai_object_id_t) * nh_count));

    p_nh_list->count = nh_count;

    SAI_NH_GROUP_LOG_TRACE ("Filled NH LIST, count: %d.", p_nh_list->count);

    for (index = 0; index < p_nh_list->count; index++) {
        SAI_NH_GROUP_LOG_TRACE ("NH Id [%d]: 0x%"PRIx64".",
                                index, p_nh_list->list [index]);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_next_hop_group_create (sai_fib_nh_group_t *p_group,
                                                   uint_t next_hop_count,
                                                   sai_fib_nh_t *ap_next_hop [],
                                                   sai_npu_object_id_t *p_group_id)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    int             nh_grp_id = 0;
    sai_object_id_t nh_grp_obj_id = 0;

    STD_ASSERT (p_group != NULL);
    STD_ASSERT (p_group_id != NULL);
    STD_ASSERT (ap_next_hop != NULL);

    SAI_NH_GROUP_LOG_TRACE ("NH Group creation. Type: %s, NH Count: %d.",
                            sai_fib_nh_group_type_str (p_group->type),
                            next_hop_count);

    nh_grp_id = std_find_first_bit (sai_vm_access_nh_grp_bitmap (),
                                    SAI_VM_NH_GRP_TABLE_SIZE, 0);

    if (nh_grp_id < 0) {
        SAI_NH_GROUP_LOG_ERR ("NH Group Table is full.");

        return SAI_STATUS_TABLE_FULL;
    }

    /* Insert Next Hop Group record to DB. */
    nh_grp_obj_id = sai_uoid_create (SAI_OBJECT_TYPE_NEXT_HOP_GROUP,
                                     (sai_npu_object_id_t) nh_grp_id);

    sai_rc = sai_nh_group_create_db_entry (nh_grp_obj_id, p_group->type,
                                           next_hop_count, ap_next_hop);

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

    SAI_NH_GROUP_LOG_TRACE ("NH Group Deletion, ID: %d.", nh_grp_id);

    /* Remove Next Hop Group record from DB. */
    sai_rc = sai_nh_group_delete_db_entry (p_group->key.group_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_NH_GROUP_LOG_ERR ("Error removing entry from DB for NH GRP ID: "
                              "%d, OBJ ID: 0x%"PRIx64".", nh_grp_id,
                              p_group->key.group_id);

        return SAI_STATUS_FAILURE;
    }

    STD_BIT_ARRAY_SET (sai_vm_access_nh_grp_bitmap (), nh_grp_id);

    SAI_NH_GROUP_LOG_TRACE ("NH Group ID %d moved to free pool.", nh_grp_id);

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

static sai_status_t sai_npu_next_hop_group_attribute_get (
sai_fib_nh_group_t *p_nh_group, uint_t attr_count, sai_attribute_t *p_attr_list)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t *p_nh_obj_id_arr;
    sai_attribute_t *p_attr = NULL;
    uint_t           attr_index = 0;
    uint_t           fwd_nh_count = 0;

    STD_ASSERT (p_nh_group != NULL);
    STD_ASSERT (p_attr_list != NULL);

    SAI_NH_GROUP_LOG_TRACE ("NH Group ID %d attribute get request, "
                            "attr_count: %d.", p_nh_group->key.group_id,
                            attr_count);

    p_nh_obj_id_arr = (sai_object_id_t *) calloc (sai_fib_max_ecmp_paths_get(),
                                                  sizeof (sai_object_id_t));

    if (p_nh_obj_id_arr == NULL) {
        SAI_NH_GROUP_LOG_ERR ("Failed to allocate memory for NH Id array.");

        return SAI_STATUS_NO_MEMORY;
    }

    sai_rc = sai_vm_nh_group_resolved_nh_paths_fill (p_nh_group, p_nh_obj_id_arr,
                                                     sai_fib_max_ecmp_paths_get(),
                                                     &fwd_nh_count);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_NH_GROUP_LOG_ERR ("Failed to get the forwarding NH paths for "
                              "NH Group: %d.", p_nh_group->key.group_id);

        free (p_nh_obj_id_arr);

        return sai_rc;
    }

    for (attr_index = 0; attr_index < attr_count; attr_index++)
    {
        p_attr = &p_attr_list [attr_index];

        switch (p_attr->id) {
            case SAI_NEXT_HOP_GROUP_ATTR_TYPE:
                p_attr->value.s32 = p_nh_group->type;

                SAI_NH_GROUP_LOG_TRACE ("Index: %d, NH Group type: %s.",
                                        attr_index,
                                        sai_fib_nh_group_type_str
                                        (p_attr->value.s32));
                break;

            case SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT:
                /*
                 * Returns the number of resolved NH paths that are forwarding
                 * in the NH Group.
                 */
                p_attr->value.u32 = fwd_nh_count;

                SAI_NH_GROUP_LOG_TRACE ("Index: %d, NH forwarding path "
                                        "count: %d.", attr_index,
                                        p_attr->value.u32);
                break;

            case SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST:
                /*
                 * Returns the list of resolved NH paths that are forwarding
                 * in the NH Group.
                 */
                sai_rc = sai_vm_fib_nh_list_fill (&p_attr->value.objlist,
                                                  p_nh_obj_id_arr, fwd_nh_count);
                break;

            default:
                sai_rc = SAI_STATUS_INVALID_ATTRIBUTE_0;

                SAI_NH_GROUP_LOG_ERR ("Index: %d, Invalid attribute Id: %d.",
                                      attr_index, p_attr->id);
                break;
        }

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_NH_GROUP_LOG_ERR ("Failure in filling NH Group attr list "
                                  "at index: %d.", attr_index);

            free (p_nh_obj_id_arr);

            return (sai_fib_attr_status_code_get (sai_rc, attr_index));
        }
    }

    SAI_NH_GROUP_LOG_TRACE ("Next Hop Group ID: %d attribute get "
                            "successful.", p_nh_group->key.group_id);

    free (p_nh_obj_id_arr);

    return SAI_STATUS_SUCCESS;
}

static sai_npu_nh_group_api_t sai_vm_nh_group_api_table = {
    sai_npu_next_hop_group_create,
    sai_npu_next_hop_group_remove,
    sai_npu_next_hop_add_to_group,
    sai_npu_next_hop_remove_from_group,
    sai_npu_next_hop_group_attribute_get
};

sai_npu_nh_group_api_t* sai_vm_nh_group_api_query (void)
{
    return &sai_vm_nh_group_api_table;
}

