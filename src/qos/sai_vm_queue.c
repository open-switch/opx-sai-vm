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
 * @file sai_vm_queue.c
 *
 * @brief This file contains the queue initialization, default settings of
 *        for queue objects and SAI API implementation for managing the
 *        SAI queue object in VM Environment.
 */

#include "sai_npu_qos.h"

#include "saistatus.h"
#include "saitypes.h"
#include "sai_vm_qos.h"
#include "std_assert.h"
#include "std_bit_masks.h"
#include "sai_qos_util.h"

#include <inttypes.h>

/**
 * Vendor attribute array for queue containing the attribute
 * values and properties for create,set and get functionality.
 */
static const dn_sai_attribute_entry_t sai_qos_queue_attr[] =  {
    /*            ID                         MC     VC     VS    VG    IMP    SUP */
    { SAI_QUEUE_ATTR_TYPE,                  true,  true,  false, true, true,  true },
    { SAI_QUEUE_ATTR_PORT,                  true,  true,  false, true, true,  true },
    { SAI_QUEUE_ATTR_INDEX,                 true,  true,  false, true, true,  true },
    { SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE, false, true,  true,  true, true,  true },
    { SAI_QUEUE_ATTR_WRED_PROFILE_ID,       false, true,  true,  true, true,  true },
    { SAI_QUEUE_ATTR_BUFFER_PROFILE_ID,     false, true,  true,  true, true,  true },
    { SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID,  false, true,  true,  true, true,  true },
};

/**
 * Function returning the attribute array for queue and
 * count of the total number of attributes.
 */
static void sai_vm_queue_attr_table_get (const dn_sai_attribute_entry_t **p_vendor,
                                         uint_t *p_max_attr_count)
{
    *p_vendor = &sai_qos_queue_attr[0];

    *p_max_attr_count = sizeof(sai_qos_queue_attr)/sizeof(sai_qos_queue_attr[0]);

    return;
}

static sai_status_t sai_vm_queue_create (dn_sai_qos_queue_t *p_queue_node,
                                   sai_object_id_t *p_queue_oid)
{
    uint_t free_id = 0;

    STD_ASSERT (p_queue_node != NULL);
    STD_ASSERT (p_queue_oid != NULL);

    SAI_QUEUE_LOG_TRACE("NPU Queue create");

    free_id = std_find_first_bit (sai_vm_qos_queue_bitmap_get (),
                                  SAI_VM_QOS_MAX_SUPPORTED_QUEUES, 0);

    if (free_id > SAI_VM_QOS_MAX_SUPPORTED_QUEUES) {
        SAI_QUEUE_LOG_TRACE("Queue max limit exceeded.");
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    STD_BIT_ARRAY_CLR (sai_vm_qos_queue_bitmap_get(), free_id);

    *p_queue_oid = sai_uoid_create (SAI_OBJECT_TYPE_QUEUE,
                                   (sai_npu_object_id_t) free_id);

    SAI_QUEUE_LOG_TRACE ("Queue creation successful in NPU. "
                         "Queue oid 0x%"PRIx64".",*p_queue_oid);

    return SAI_STATUS_SUCCESS;

}

static sai_status_t sai_vm_queue_remove (dn_sai_qos_queue_t *p_queue_node)
{
    STD_ASSERT (p_queue_node != NULL);

    uint_t  queue_id = sai_uoid_npu_obj_id_get(p_queue_node->key.queue_id);

    STD_BIT_ARRAY_SET (sai_vm_qos_queue_bitmap_get(), queue_id);

    SAI_QUEUE_LOG_TRACE ("Queue delete successful in NPU. "
                         "Queue oid 0x%"PRIx64".",
                         p_queue_node->key.queue_id);

    return SAI_STATUS_SUCCESS;

}

static sai_status_t sai_vm_queue_attach_to_parent (
                             sai_object_id_t queue_id,
                             sai_object_id_t parent_id)
{
    sai_status_t       sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_queue_t  *p_queue_node = NULL;
    dn_sai_qos_sched_group_t  *p_parent_node = NULL;
    uint_t             child_index = 0;

    p_queue_node = sai_qos_queue_node_get (queue_id);

    if (p_queue_node == NULL) {
        SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" does not exist in tree.",
                           queue_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    p_parent_node = sai_qos_sched_group_node_get (parent_id);
    if (p_parent_node == NULL) {
        SAI_QUEUE_LOG_ERR ("Parent scheduler group 0x%"PRIx64""
                           "does not exist in tree.", parent_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_rc = sai_qos_queue_validate_child_parent(
                                                 p_queue_node, p_parent_node);
    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        SAI_QUEUE_LOG_ERR ("Parent scheduler group 0x%"PRIx64""
                           "and child queue 0x%"PRIx64" validation failed.",
                           parent_id, queue_id);
        return sai_rc;
    }

    sai_rc = sai_qos_sched_group_first_free_child_index_get (parent_id,
                                                     &child_index);
    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_QUEUE_LOG_ERR ("Failed to get free child index "
                           "for SG oid 0x%"PRIx64"", parent_id);

        return sai_rc;
    }

    sai_rc = sai_qos_child_index_update (queue_id, child_index);
    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_QUEUE_LOG_ERR ("Failed to update child index in node.");
    }

    return sai_rc;
}

static sai_status_t sai_vm_queue_detach_from_parent (
                             sai_object_id_t queue_id)
{
    sai_status_t              sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t           parent_id = SAI_NULL_OBJECT_ID;
    uint_t                    child_index = 0;
    dn_sai_qos_queue_t        *p_queue_node = NULL;
    dn_sai_qos_sched_group_t  *p_parent_node = NULL;

    do {
        p_queue_node = sai_qos_queue_node_get (queue_id);

        if(p_queue_node == NULL){
            SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" "
                               "does not exist in tree.",
                               queue_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        parent_id = p_queue_node->parent_sched_group_id;
        p_parent_node = sai_qos_sched_group_node_get (parent_id);

        if(p_parent_node == NULL){
            SAI_QUEUE_LOG_ERR ("Scheduler group 0x%"PRIx64" "
                               "does not exist in tree.",
                               parent_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_rc = sai_qos_child_index_get (queue_id, &child_index);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Failed to get child index "
                               "of SG oid 0x%"PRIx64" "
                               "and child oid 0x%"PRIx64"",
                               parent_id, queue_id);
            break;
        }

        sai_rc = sai_qos_child_index_update (queue_id, SAI_QOS_CHILD_INDEX_INVALID);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Failed to update child index %d in node 0x%"PRIx64".",
                               SAI_QOS_CHILD_INDEX_INVALID, queue_id);
            break;
        }

        sai_rc = sai_qos_sched_group_child_index_free (parent_id,
                                                       child_index);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Failed to free child index %d in node 0x%"PRIx64".",
                               child_index, queue_id);
        }

    } while (0);

    return sai_rc;
}

static sai_status_t sai_vm_queue_modify_parent (
                             sai_object_id_t queue_id,
                             sai_object_id_t new_parent_id)
{
    sai_status_t              sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_queue_t        *p_queue_node = NULL;
    dn_sai_qos_sched_group_t  *p_new_parent_node = NULL;
    dn_sai_qos_sched_group_t  *p_old_parent_node = NULL;
    sai_object_id_t           old_parent_id = SAI_NULL_OBJECT_ID;
    uint_t                    child_index = 0;

    do {

        p_queue_node = sai_qos_queue_node_get (queue_id);

        if(p_queue_node == NULL){
            SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" does not exist in tree.",
                               queue_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        p_new_parent_node = sai_qos_sched_group_node_get (new_parent_id);
        if (p_new_parent_node == NULL) {
            SAI_QUEUE_LOG_ERR ("New parent scheduler group 0x%"PRIx64""
                               "does not exist in tree.", new_parent_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_rc = sai_qos_queue_validate_child_parent(
                                                     p_queue_node, p_new_parent_node);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Parent scheduler group 0x%"PRIx64""
                               "and child queue 0x%"PRIx64" validation failed.",
                               new_parent_id, queue_id);
            break;
        }

        old_parent_id = p_queue_node->parent_sched_group_id;
        p_old_parent_node = sai_qos_sched_group_node_get (old_parent_id);
        if (p_old_parent_node == NULL) {
            SAI_QUEUE_LOG_ERR ("Old parent scheduler group 0x%"PRIx64""
                               "does not exist in tree.", old_parent_id);
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

        sai_rc = sai_vm_queue_detach_from_parent(queue_id);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Unable to detach child from old parent");
            break;
        }

        sai_rc = sai_qos_sched_group_first_free_child_index_get (new_parent_id,
                                                     &child_index);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Failed to get free child index "
                               "for SG oid 0x%"PRIx64"", new_parent_id);

            break;
        }

        sai_rc = sai_qos_child_index_update (queue_id, child_index);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Failed to update child index in node.");

        }
    } while (0);

    return sai_rc;
}

static sai_status_t sai_vm_queue_attribute_set (
                             dn_sai_qos_queue_t *p_queue_node,
                             uint_t attr_count,
                             const sai_attribute_t *p_attr_list)
{
    /* TODO */
    return SAI_STATUS_NOT_IMPLEMENTED;
}

static sai_status_t sai_vm_queue_attribute_get (dn_sai_qos_queue_t *p_queue_node,
                                                uint_t attr_count,
                                                sai_attribute_t *p_attr_list)
{
    sai_status_t            sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t         *p_attr = NULL;
    uint_t                  list_index = 0;

    STD_ASSERT (p_queue_node != NULL);
    STD_ASSERT (p_attr_list != NULL);

    SAI_QUEUE_LOG_TRACE ("NPU Queue attribute get, attr_count: "
                         "%d", attr_count);

    for (list_index = 0, p_attr = p_attr_list;
         list_index < attr_count; ++list_index, ++p_attr) {

        SAI_QUEUE_LOG_TRACE("NPU Queue attribute set, attr_id %d",
                            p_attr->id);

        switch (p_attr->id)
        {
            case SAI_QUEUE_ATTR_TYPE:
                p_attr->value.s32 = p_queue_node->queue_type;
                break;

            case SAI_QUEUE_ATTR_INDEX:
                p_attr->value.u8 = p_queue_node->queue_index;
                break;

            case SAI_QUEUE_ATTR_PORT:
                p_attr->value.oid = p_queue_node->port_id;
                break;

            case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
                p_attr->value.oid = p_queue_node->scheduler_id;
                break;

            case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
                p_attr->value.oid = p_queue_node->wred_id;
                break;

            case SAI_QUEUE_ATTR_BUFFER_PROFILE_ID:
                p_attr->value.oid = p_queue_node->buffer_profile_id;
                break;

            case SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE:
                p_attr->value.oid = p_queue_node->parent_sched_group_id;
                break;

            default:
                SAI_QUEUE_LOG_ERR ("Unknown attribute id: %d.",
                                   p_attr->id);
                /* Attribute validation done in common, Not expected this case */
                sai_rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }

        if (sai_rc != SAI_STATUS_SUCCESS) {

            SAI_QUEUE_LOG_ERR ("Failure in get queue attr list at "
                               "index: %d.", list_index);

            return sai_rc;
        }
    }

    return sai_rc;
}

static sai_status_t sai_vm_queue_stats_get (dn_sai_qos_queue_t *p_queue_node,
                                            const sai_queue_stat_t *counter_ids,
                                            uint_t number_of_counters, uint64_t* counters)
{
    /* TODO */
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_queue_stats_clear (dn_sai_qos_queue_t *p_queue_node,
                                            const sai_queue_stat_t *counter_ids,
                                            uint_t number_of_counters)
{
    /* TODO */
    return SAI_STATUS_SUCCESS;
}
static sai_npu_queue_api_t sai_vm_queue_api_table = {
    sai_vm_queue_create,
    sai_vm_queue_remove,
    sai_vm_queue_attach_to_parent,
    sai_vm_queue_detach_from_parent,
    sai_vm_queue_modify_parent,
    sai_vm_queue_attribute_set,
    sai_vm_queue_attribute_get,
    sai_vm_queue_attr_table_get,
    sai_vm_queue_stats_get,
    sai_vm_queue_stats_clear,
};

sai_npu_queue_api_t* sai_vm_queue_api_query (void)
{
    return &sai_vm_queue_api_table;
}

