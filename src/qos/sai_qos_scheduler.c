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
* @file  sai_qos_scheduler.c
*
* @brief This file contains function definitions for SAI QOS Scheduler
*        functionality API implementation.
*
*************************************************************************/

#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_api_utils.h"
#include "sai_qos_mem.h"
#include "sai_switch_utils.h"
#include "sai_common_utils.h"
#include "sai_gen_utils.h"
#include "sai_common_infra.h"

#include "saistatus.h"
#include "saitypes.h"

#include "std_assert.h"

#include <string.h>
#include <inttypes.h>

static void sai_qos_scheduler_node_init (dn_sai_qos_scheduler_t *p_sched_node)
{
    p_sched_node->sched_algo = SAI_SCHEDULING_TYPE_WRR;
    p_sched_node->weight = SAI_QOS_SCHEDULER_DEFAULT_WEIGHT;
    p_sched_node->shape_type = SAI_QOS_SCHEDULER_DEFAULT_SHAPE_TYPE;
    std_dll_init (&p_sched_node->port_dll_head);
    std_dll_init (&p_sched_node->queue_dll_head);
    std_dll_init (&p_sched_node->sched_group_dll_head);
}

static bool sai_qos_scheduler_is_in_use (dn_sai_qos_scheduler_t *p_sched_node)
{
    STD_ASSERT (p_sched_node != NULL);

    /* Verify Scheduler is associted to queue.*/
    if (sai_qos_scheduler_get_first_queue (p_sched_node) != NULL) {
        SAI_SCHED_LOG_ERR ("Scheduler in use, Scheduler associated to queue.");
        return true;
    }

    /* Verify Scheduler is associted to Scheudler group.*/
    if (sai_qos_scheduler_get_first_sched_group (p_sched_node) != NULL) {
        SAI_SCHED_LOG_ERR ("Scheduler in use, Scheduler associated to SG.");
        return true;
    }

    /* Verify Scheduler is associted to port.*/
    if (sai_qos_scheduler_get_first_port (p_sched_node) != NULL) {
        SAI_SCHED_LOG_ERR ("Scheduler in use, Scheduler associated to port.");
        return true;
    }

    return false;
}

static void sai_qos_scheduler_free_resources (dn_sai_qos_scheduler_t *p_sched_node,
                                              bool is_sched_set_in_npu)
{
    if (p_sched_node == NULL) {
        return;
    }

    /* Remove Scheduler from NPU, if it was already applied created.*/
    if (is_sched_set_in_npu) {
        sai_scheduler_npu_api_get()->scheduler_remove (p_sched_node);
    }

    sai_qos_scheduler_node_free (p_sched_node);
}

static sai_status_t sai_qos_scheduler_node_insert_to_tree (
                                      dn_sai_qos_scheduler_t *p_sched_node)
{
    rbtree_handle   scheduler_tree = NULL;
    t_std_error     err_rc = STD_ERR_OK;

    STD_ASSERT (p_sched_node != NULL);

    scheduler_tree = sai_qos_access_global_config()->scheduler_tree;

    STD_ASSERT (scheduler_tree != NULL);

    err_rc = std_rbtree_insert (scheduler_tree, p_sched_node);

    if (STD_IS_ERR(err_rc)) {
        SAI_SCHED_LOG_ERR ("Failed to insert Scheduler node for ID 0x%"PRIx64" "
                           "into Scheduler Tree", p_sched_node->key.scheduler_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static void sai_qos_scheduler_node_remove_from_tree (
                                         dn_sai_qos_scheduler_t *p_sched_node)
{
    rbtree_handle   scheduler_tree = NULL;

    STD_ASSERT (p_sched_node != NULL);

    scheduler_tree = sai_qos_access_global_config()->scheduler_tree;

    STD_ASSERT (scheduler_tree != NULL);

    std_rbtree_remove (scheduler_tree, p_sched_node);
}

static void sai_qos_scheduler_attr_set (dn_sai_qos_scheduler_t *p_sched_node,
                                        uint_t attr_count,
                                        const sai_attribute_t *p_attr_list)
{
    const sai_attribute_t  *p_attr = NULL;
    uint_t                 list_index = 0;

    STD_ASSERT(p_sched_node != NULL);

    SAI_SCHED_LOG_TRACE ("Set attributes for Scheduler group, "
                         "attribute count %d.", attr_count);

    for (list_index = 0, p_attr = p_attr_list; (list_index < attr_count);
         ++list_index, ++p_attr) {

        switch (p_attr->id)
        {
            case SAI_SCHEDULER_ATTR_SCHEDULING_TYPE:
                p_sched_node->sched_algo = p_attr->value.s32;
                break;

            case SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT:
                p_sched_node->weight = p_attr->value.u8;
                break;

            case SAI_SCHEDULER_ATTR_METER_TYPE:
                p_sched_node->shape_type = p_attr->value.s32;
                break;

            case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE:
                p_sched_node->min_bandwidth_rate = p_attr->value.u64;
                break;

            case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE:
                p_sched_node->min_bandwidth_burst = p_attr->value.u64;
                break;

            case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE:
                p_sched_node->max_bandwidth_rate = p_attr->value.u64;
                break;

            case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE:
                p_sched_node->max_bandwidth_burst = p_attr->value.u64;
                break;

            default:
                SAI_SCHED_LOG_TRACE ("Attribute id: %d - read-only attribute.",
                                     p_attr->id);
                break;
        }
    }
}

static sai_status_t sai_qos_scheduler_attributes_validate (
                                     uint_t attr_count,
                                     const sai_attribute_t *p_attr_list,
                                     dn_sai_operations_t op_type)
{
    sai_status_t                    sai_rc = SAI_STATUS_SUCCESS;
    uint_t                          max_vendor_attr_count = 0;
    const dn_sai_attribute_entry_t *p_vendor_attr = NULL;

    SAI_SCHED_LOG_TRACE ("Parsing attributes for Scheduler group, "
                         "attribute count %d op_type %d.", attr_count,
                         op_type);

    if ((op_type != SAI_OP_CREATE) && (attr_count == 0)) {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count > 0) && (p_attr_list == NULL)) {
        STD_ASSERT(p_attr_list != NULL);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_scheduler_npu_api_get()->attribute_table_get(&p_vendor_attr,
                                                     &max_vendor_attr_count);

    STD_ASSERT(p_vendor_attr != NULL);
    STD_ASSERT(max_vendor_attr_count > 0);

    sai_rc = sai_attribute_validate (attr_count, p_attr_list, p_vendor_attr,
                                     op_type, max_vendor_attr_count);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_LOG_ERR ("Attribute validation failed for %d "
                           "operation", op_type);
    }

    return sai_rc;
}

/* Attribute validation happen before this function */
static bool sai_qos_scheduler_is_duplicate_set (dn_sai_qos_scheduler_t *p_sched_node,
                                                const sai_attribute_t *p_attr)
{
    STD_ASSERT(p_sched_node != NULL);
    STD_ASSERT(p_attr != NULL);

    SAI_SCHED_LOG_TRACE ("Verify duplicate set attributes value, ID: %d.",
                          p_attr->id);

    switch (p_attr->id)
    {
        case SAI_SCHEDULER_ATTR_SCHEDULING_TYPE:
            if (p_sched_node->sched_algo == p_attr->value.s32)
                return true;
            break;

        case SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT:
            if (p_sched_node->weight == p_attr->value.u8)
                return true;
            break;

        case SAI_SCHEDULER_ATTR_METER_TYPE:
            if (p_sched_node->shape_type == p_attr->value.s32)
                return true;
            break;

        case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE:
            if (p_sched_node->min_bandwidth_rate == p_attr->value.u64)
                return true;
            break;

        case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE:
            if (p_sched_node->min_bandwidth_burst == p_attr->value.u64)
                return true;
            break;

        case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE:
            if (p_sched_node->max_bandwidth_rate == p_attr->value.u64)
                return true;
            break;

        case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE:
            if (p_sched_node->max_bandwidth_burst == p_attr->value.u64)
                return true;
            break;
        default:
            SAI_SCHED_LOG_TRACE ("Attribute id: %d - read-only attribute.",
                                 p_attr->id);
            break;
    }

    return false;
}


static sai_status_t sai_qos_scheduler_create_internal (sai_object_id_t *sched_id,
                                              sai_object_id_t switch_id,
                                              uint32_t attr_count,
                                              const sai_attribute_t *attr_list)
{
    sai_status_t               sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_scheduler_t     *p_sched_node = NULL;
    bool                       is_sched_set_in_npu = false;

    SAI_SCHED_LOG_TRACE ("Scheduler Creation, attr_count: %d.", attr_count);

    STD_ASSERT (sched_id != NULL);

    sai_rc = sai_qos_scheduler_attributes_validate (attr_count, attr_list,
                                                    SAI_OP_CREATE);
    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_LOG_ERR ("Input parameters validation failed for "
                           "Scheduler creation.");
        return sai_rc;
    }

    do {
        p_sched_node = sai_qos_scheduler_node_alloc ();

        if (NULL == p_sched_node) {
            SAI_SCHED_LOG_ERR ("Scheduler node memory allocation failed.");
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        sai_qos_scheduler_node_init (p_sched_node);

        sai_qos_scheduler_attr_set (p_sched_node, attr_count, attr_list);

        sai_rc = sai_scheduler_npu_api_get()->scheduler_create (p_sched_node,
                                         &p_sched_node->key.scheduler_id);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR ("Scheduler creation failed in NPU.");
            break;
        }

        SAI_SCHED_LOG_TRACE ("Scheduler Created in NPU.");
        is_sched_set_in_npu = true;

        *sched_id = p_sched_node->key.scheduler_id;

        sai_rc = sai_qos_scheduler_node_insert_to_tree (p_sched_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR ("Scheduler insertion to tree failed.");
            break;
        }
    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_SCHED_LOG_INFO ("Scheduler Obj Id 0x%"PRIx64" created.", *sched_id);
    } else {
        SAI_SCHED_LOG_ERR ("Failed to create Scheduler.");
        sai_qos_scheduler_free_resources (p_sched_node, is_sched_set_in_npu);
    }

    return sai_rc;
}

static sai_status_t sai_qos_scheduler_create (sai_object_id_t *sched_id,
                                              sai_object_id_t switch_id,
                                              uint32_t attr_count,
                                              const sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    sai_qos_lock();

    sai_rc = sai_qos_scheduler_create_internal(sched_id, switch_id, attr_count,
                                               attr_list);

    sai_qos_unlock();

    return sai_rc;
}


static sai_status_t sai_qos_scheduler_remove_internal (sai_object_id_t sched_id)
{
    sai_status_t            sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_scheduler_t  *p_sched_node = NULL;

    SAI_SCHED_LOG_TRACE ("Scheduler remove ID 0x%"PRIx64".", sched_id);

    if (! sai_is_obj_id_scheduler (sched_id)) {
        SAI_SCHED_LOG_ERR ("0x%"PRIx64" is not a valid scheduler obj id.",
                           sched_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    do {
        p_sched_node = sai_qos_scheduler_node_get (sched_id);

        if (NULL == p_sched_node) {
            SAI_SCHED_LOG_ERR ("Scheduler 0x%"PRIx64" does not exist in tree.",
                                sched_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if (sai_qos_scheduler_is_in_use (p_sched_node)) {
            SAI_SCHED_LOG_ERR ("Scheduler 0x%"PRIx64" can't be deleted, "
                               "It is in use.", sched_id);
            sai_rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        sai_rc = sai_scheduler_npu_api_get()->scheduler_remove (p_sched_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR ("Scheduler 0x%"PRIx64" deletion failed in NPU.",
                               sched_id);
            break;
        }

        sai_qos_scheduler_node_remove_from_tree (p_sched_node);

        sai_qos_scheduler_node_free (p_sched_node);

    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_SCHED_LOG_INFO ("Scheduler 0x%"PRIx64" removed.", sched_id);
    } else {
        SAI_SCHED_LOG_ERR ("Failed to remove Scheduler 0x%"PRIx64".", sched_id);
    }

    return sai_rc;
}

static sai_status_t sai_qos_scheduler_remove (sai_object_id_t sched_id)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    sai_qos_lock();

    sai_rc = sai_qos_scheduler_remove_internal (sched_id);

    sai_qos_unlock();

    return sai_rc;
}

static sai_status_t sai_qos_scheduler_attribute_set (sai_object_id_t sched_id,
                                                     const sai_attribute_t *p_attr)
{
    sai_status_t               sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_scheduler_t    *p_sched_node = NULL;
    dn_sai_qos_scheduler_t     new_sched_node;
    const uint_t               attr_count = 1;

    STD_ASSERT (p_attr != NULL);

    SAI_SCHED_LOG_TRACE ("Setting Attribute ID %d on Scheduler 0x%"PRIx64".",
                          p_attr->id, sched_id);

    if (! sai_is_obj_id_scheduler (sched_id)) {
        SAI_SCHED_LOG_ERR ("%"PRIx64" is not a valid Scheduler obj id.",
                           sched_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_qos_lock ();

    do {
        p_sched_node = sai_qos_scheduler_node_get (sched_id);

        if (NULL == p_sched_node) {
            SAI_SCHED_LOG_ERR ("Scheduler 0x%"PRIx64" does not exist in tree.",
                               sched_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_rc = sai_qos_scheduler_attributes_validate (attr_count, p_attr,
                                                        SAI_OP_SET);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR ("Input parameters validation failed for "
                               "Scheduler attributes set.");
            break;
        }

        if (sai_qos_scheduler_is_duplicate_set (p_sched_node, p_attr)) {
            SAI_SCHED_LOG_TRACE ("Duplicate set value for Attribute ID %d.",
                                 p_attr->id);
            break;
        }

        sai_rc = sai_scheduler_npu_api_get()->scheduler_attribute_set(p_sched_node,
                                                                      attr_count,
                                                                      p_attr);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR ("Failed to set Scheduler Attribute ID: %d "
                               "in NPU, Error: %d.", p_attr->id, sai_rc);
            break;
        }

        if (! sai_scheduler_npu_api_get()->scheduler_is_hw_object()) {
            memcpy(&new_sched_node, p_sched_node, sizeof (dn_sai_qos_scheduler_t));
            sai_qos_scheduler_attr_set (&new_sched_node, attr_count, p_attr);
            sai_rc = sai_qos_scheduler_reapply (p_sched_node, &new_sched_node);
        }
        sai_qos_scheduler_attr_set (p_sched_node, attr_count, p_attr);
    } while (0);

    sai_qos_unlock ();

    return sai_rc;
}

static sai_status_t sai_qos_scheduler_attribute_get (sai_object_id_t sched_id,
                                                     uint32_t attr_count,
                                                     sai_attribute_t *p_attr_list)
{
    sai_status_t               sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_scheduler_t    *p_sched_node = NULL;

    STD_ASSERT (p_attr_list != NULL);

    SAI_SCHED_LOG_TRACE ("Getting Attributes for scheduler 0x%"PRIx64", "
                         "attr_count %d.", sched_id, attr_count);

    if (! sai_is_obj_id_scheduler (sched_id)) {
        SAI_SCHED_LOG_ERR ("%"PRIx64" is not a valid Scheduler obj id.",
                           sched_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_qos_lock ();

    do {
        p_sched_node = sai_qos_scheduler_node_get (sched_id);

        if (NULL == p_sched_node) {
            SAI_SCHED_LOG_ERR ("Scheduler 0x%"PRIx64" does not exist in tree.",
                               sched_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_rc = sai_qos_scheduler_attributes_validate (attr_count, p_attr_list,
                                                        SAI_OP_GET);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR ("Input parameters validation failed for "
                               "Scheduler attributes get.");
            break;
        }

        sai_rc = sai_scheduler_npu_api_get()->scheduler_attribute_get (p_sched_node,
                                                                       attr_count,
                                                                       p_attr_list);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR ("Failed to get Scheduler Attributes from "
                               "NPU, Error: %d.", sai_rc);
            break;
        }
    } while (0);

    sai_qos_unlock ();

    return sai_rc;
}

static sai_status_t sai_qos_scheduler_queue_list_update (dn_sai_qos_queue_t *p_queue_node,
                                                  sai_object_id_t  prev_sched_id,
                                                  sai_object_id_t  new_sched_id)
{
    dn_sai_qos_scheduler_t  *p_sched_node = NULL;
    dn_sai_qos_scheduler_t  *p_prev_sched_node = NULL;

    STD_ASSERT (p_queue_node != NULL);

    SAI_SCHED_LOG_TRACE ("Update scheduler queue list, QID 0x%"PRIx64", "
                         "Prev Scheduler 0x%"PRIx64" and New Scheduler "
                         "ID 0x%"PRIx64".", p_queue_node->key.queue_id,
                         prev_sched_id, new_sched_id);

    if ((prev_sched_id != SAI_NULL_OBJECT_ID) &&
        (prev_sched_id != sai_qos_default_sched_id_get())) {
        p_prev_sched_node= sai_qos_scheduler_node_get (prev_sched_id);

        if (NULL == p_prev_sched_node) {
            SAI_SCHED_LOG_ERR ("Previous Scheduler 0x%"PRIx64" does not exist in tree.",
                                prev_sched_id);

            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        std_dll_remove (&p_prev_sched_node->queue_dll_head,
                        &p_queue_node->scheduler_dll_glue);
    }

    if ((new_sched_id != SAI_NULL_OBJECT_ID) &&
        (new_sched_id != sai_qos_default_sched_id_get())) {

        p_sched_node = sai_qos_scheduler_node_get (new_sched_id);

        if (NULL == p_sched_node) {
            SAI_SCHED_LOG_ERR ("Scheduler 0x%"PRIx64" does not exist in tree.",
                                new_sched_id);

            return SAI_STATUS_INVALID_OBJECT_ID;
        }

        std_dll_insertatback (&p_sched_node->queue_dll_head,
                              &p_queue_node->scheduler_dll_glue);
    }

    p_queue_node->scheduler_id = new_sched_id;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_qos_scheduler_sched_group_list_update (
                             dn_sai_qos_sched_group_t *p_sg_node,
                             sai_object_id_t  prev_sched_id,
                             sai_object_id_t  new_sched_id)
{
    dn_sai_qos_scheduler_t  *p_sched_node = NULL;
    dn_sai_qos_scheduler_t  *p_prev_sched_node = NULL;

    STD_ASSERT (p_sg_node != NULL);

    SAI_SCHED_LOG_TRACE ("Update scheduler SG list, SGID 0x%"PRIx64", "
                         "Prev Scheduler 0x%"PRIx64" and New Scheduler "
                         "ID 0x%"PRIx64".", p_sg_node->key.sched_group_id,
                         prev_sched_id, new_sched_id);

    /* Replace case, Remove old and update new if exits */
    if ((prev_sched_id != SAI_NULL_OBJECT_ID) &&
        (prev_sched_id != sai_qos_default_sched_id_get())) {
        p_prev_sched_node= sai_qos_scheduler_node_get (prev_sched_id);

        if (NULL == p_prev_sched_node) {
            SAI_SCHED_LOG_ERR ("Previous Scheduler 0x%"PRIx64" does not exist in tree.",
                                prev_sched_id);

            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        std_dll_remove (&p_prev_sched_node->sched_group_dll_head,
                        &p_sg_node->scheduler_dll_glue);
    }


    if ((new_sched_id != SAI_NULL_OBJECT_ID) &&
        (new_sched_id != sai_qos_default_sched_id_get())){

        p_sched_node = sai_qos_scheduler_node_get (new_sched_id);

        if (NULL == p_sched_node) {
            SAI_SCHED_LOG_ERR ("Scheduler 0x%"PRIx64" does not exist in tree.",
                                new_sched_id);

            return SAI_STATUS_INVALID_OBJECT_ID;
        }

        std_dll_insertatback (&p_sched_node->sched_group_dll_head,
                              &p_sg_node->scheduler_dll_glue);
    }

    p_sg_node->scheduler_id = new_sched_id;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_qos_scheduler_port_list_update (dn_sai_qos_port_t  *p_qos_port_node,
                                                 sai_object_id_t  prev_sched_id,
                                                 sai_object_id_t  new_sched_id)
{
    dn_sai_qos_scheduler_t  *p_sched_node = NULL;
    dn_sai_qos_scheduler_t  *p_prev_sched_node = NULL;

    STD_ASSERT (p_qos_port_node != NULL);

    SAI_SCHED_LOG_TRACE ("Update scheduler port list, Port 0x%"PRIx64", "
                         "Prev Scheduler 0x%"PRIx64" and New Scheduler "
                         "ID 0x%"PRIx64".", p_qos_port_node->port_id,
                         prev_sched_id, new_sched_id);

    if ((prev_sched_id != SAI_NULL_OBJECT_ID) &&
        (prev_sched_id != sai_qos_default_sched_id_get())) {
        p_prev_sched_node= sai_qos_scheduler_node_get (prev_sched_id);

        if (NULL == p_prev_sched_node) {
            SAI_SCHED_LOG_ERR ("Previous Scheduler 0x%"PRIx64" does not exist in tree.",
                                prev_sched_id);

            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        std_dll_remove (&p_prev_sched_node->port_dll_head,
                        &p_qos_port_node->scheduler_dll_glue);
    }

    if ((new_sched_id != SAI_NULL_OBJECT_ID) &&
        (new_sched_id != sai_qos_default_sched_id_get())) {

        p_sched_node = sai_qos_scheduler_node_get (new_sched_id);

        if (NULL == p_sched_node) {
            SAI_SCHED_LOG_ERR ("Scheduler 0x%"PRIx64" does not exist in tree.",
                                new_sched_id);

            return SAI_STATUS_INVALID_OBJECT_ID;
        }

        std_dll_insertatback (&p_sched_node->port_dll_head,
                              &p_qos_port_node->scheduler_dll_glue);
    }
    p_qos_port_node->scheduler_id = new_sched_id;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_queue_scheduler_set (dn_sai_qos_queue_t *p_queue_node,
                                         const sai_attribute_t *p_attr)
{
    sai_status_t              sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t           sched_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t           old_sched_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t           queue_id = SAI_NULL_OBJECT_ID;
    dn_sai_qos_scheduler_t    *p_sched_node = NULL;
    dn_sai_qos_scheduler_t    *p_old_sched_node = NULL;

    STD_ASSERT(p_attr != NULL);
    STD_ASSERT(p_queue_node != NULL);

    sched_id = p_attr->value.oid;
    queue_id =  p_queue_node->key.queue_id;
    old_sched_id = p_queue_node->scheduler_id;

    SAI_SCHED_LOG_TRACE ("New Scheduler 0x%"PRIx64" Old Scheduler 0x%"PRIx64" set on QID 0x%"PRIx64"",
                         sched_id, old_sched_id, queue_id);

    if(sched_id == SAI_NULL_OBJECT_ID) {
        sched_id = sai_qos_default_sched_id_get();
    }

    if (old_sched_id == sched_id) {
        SAI_SCHED_LOG_TRACE ("Duplicate Update, Old Scheduler 0x%"PRIx64" "
                             "Scheduler 0x%"PRIx64" set on QID 0x%"PRIx64"",
                             old_sched_id, sched_id, queue_id);
        return SAI_STATUS_SUCCESS;
    }

    SAI_SCHED_LOG_TRACE ("Scheduler 0x%"PRIx64" set on QID 0x%"PRIx64"",
                         sched_id, queue_id);

    p_sched_node = sai_qos_scheduler_node_get (sched_id);

    if (NULL == p_sched_node) {
        SAI_SCHED_LOG_ERR ("Scheduler 0x%"PRIx64" does not exist in tree.",
                sched_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (old_sched_id != SAI_NULL_OBJECT_ID) {
        p_old_sched_node = sai_qos_scheduler_node_get (old_sched_id);

        if (NULL == p_old_sched_node) {
            SAI_SCHED_LOG_ERR ("Scheduler 0x%"PRIx64" does not exist in tree.",
                               old_sched_id);
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }

    sai_rc = sai_scheduler_npu_api_get()->scheduler_set (queue_id, p_old_sched_node,
                                                         p_sched_node);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_LOG_ERR("Scheduler set for queue id 0x%"PRIx64" failed with err %d",
                          queue_id, sai_rc);
        return sai_rc;
    }

    /* Queue Create with scheduler profile is not supported. In that case
     *  prev_id =
        ((op_type == SAI_OP_CREATE) ? SAI_NULL_OBJECT_ID : p_queue_node->scheduler_id); */

    sai_rc = sai_qos_scheduler_queue_list_update (p_queue_node,
                                                  p_queue_node->scheduler_id,
                                                  sched_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_LOG_ERR ("Failed to update queue in scheduler node.");
        STD_ASSERT(sai_rc != SAI_STATUS_SUCCESS);
        return sai_rc;
    }

    SAI_SCHED_LOG_TRACE ("Scheduler 0x%"PRIx64" set success on queue 0x%"PRIx64"",
                         sched_id, queue_id);

    return sai_rc;
}

sai_status_t sai_qos_sched_group_scheduler_set (dn_sai_qos_sched_group_t *p_sg_node,
                                         const sai_attribute_t *p_attr)
{
    sai_status_t              sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t           sched_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t           old_sched_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t           sg_id = SAI_NULL_OBJECT_ID;
    dn_sai_qos_scheduler_t    *p_sched_node = NULL;
    dn_sai_qos_scheduler_t    *p_old_sched_node = NULL;

    STD_ASSERT(p_attr != NULL);
    STD_ASSERT(p_sg_node != NULL);

    sched_id = p_attr->value.oid;
    sg_id =  p_sg_node->key.sched_group_id;
    old_sched_id = p_sg_node->scheduler_id;
    SAI_SCHED_LOG_TRACE ("New Scheduler 0x%"PRIx64" Old Scheduler 0x%"PRIx64" set on SGID 0x%"PRIx64"",
                         sched_id, old_sched_id, sg_id);

    if(sched_id == SAI_NULL_OBJECT_ID) {
        sched_id = sai_qos_default_sched_id_get();
    }

    if (old_sched_id == sched_id) {
        SAI_SCHED_LOG_TRACE ("Duplicate Update, Old Scheduler 0x%"PRIx64" "
                             "Scheduler 0x%"PRIx64" set on SGID 0x%"PRIx64"",
                             old_sched_id, sched_id, sg_id);
        return SAI_STATUS_SUCCESS;
    }

    SAI_SCHED_LOG_TRACE ("Scheduler 0x%"PRIx64" set on SGID 0x%"PRIx64"",
                          sched_id, sg_id);

    p_sched_node = sai_qos_scheduler_node_get (sched_id);

    if (NULL == p_sched_node) {
        SAI_SCHED_LOG_ERR ("Scheduler 0x%"PRIx64" does not exist in tree.",
                sched_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (old_sched_id != SAI_NULL_OBJECT_ID) {
        p_old_sched_node = sai_qos_scheduler_node_get (old_sched_id);

        if (NULL == p_old_sched_node) {
            SAI_SCHED_LOG_ERR ("Scheduler 0x%"PRIx64" does not exist in tree.",
                               old_sched_id);
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }

    sai_rc = sai_scheduler_npu_api_get()->scheduler_set (sg_id, p_old_sched_node, p_sched_node);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_LOG_ERR("Scheduler set for SGID 0x%"PRIx64" failed with err %d",
                          sg_id, sai_rc);
        return sai_rc;
    }

    /*  SG Create with scheduler profile is not supported. In that case
     *  prev_id =
        ((op_type == SAI_OP_CREATE) ? SAI_NULL_OBJECT_ID : p_sg_node->scheduler_id); */
    sai_rc = sai_qos_scheduler_sched_group_list_update (p_sg_node,
                                                        old_sched_id,
                                                        sched_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_LOG_ERR ("Failed to update SG in scheduler node.");
        STD_ASSERT(sai_rc != SAI_STATUS_SUCCESS);
        return sai_rc;
    }

    SAI_SCHED_LOG_TRACE ("Scheduler 0x%"PRIx64" set success on SG 0x%"PRIx64"",
                         sched_id, sg_id);

    return sai_rc;
}

sai_status_t sai_qos_port_scheduler_set (sai_object_id_t port_id,
                                         const sai_attribute_t *p_attr)
{
    sai_status_t               sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_port_t          *p_qos_port_node = NULL;
    sai_object_id_t             sched_id = SAI_NULL_OBJECT_ID;
    dn_sai_qos_scheduler_t     *p_sched_node = NULL;

    STD_ASSERT(p_attr != NULL);

    sched_id = p_attr->value.oid;

    if(sched_id == SAI_NULL_OBJECT_ID) {
        sched_id = sai_qos_default_sched_id_get();
    }


    SAI_SCHED_LOG_TRACE ("Scheduler 0x%"PRIx64" set on port id 0x%"PRIx64"",
                         sched_id, port_id);
    sai_qos_lock();

    do {

        p_qos_port_node = sai_qos_port_node_get (port_id);
        if (p_qos_port_node == NULL){
            SAI_SCHED_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                               port_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if (p_qos_port_node->scheduler_id == sched_id) {
            SAI_SCHED_LOG_TRACE("Scheduler id already exits on port");
            sai_rc = SAI_STATUS_SUCCESS;
            break;
        }

        p_sched_node = sai_qos_scheduler_node_get (sched_id);

        if (NULL == p_sched_node) {
            SAI_SCHED_LOG_ERR ("Scheduler 0x%"PRIx64" does not exist in tree.",
                    sched_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_rc = sai_scheduler_npu_api_get()->scheduler_set (port_id, NULL, p_sched_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR("Scheduler set for port 0x%"PRIx64" failed with err %d",
                              port_id, sai_rc);
            break;
        }

        /*  Port Create with scheduler profile is not supported. In that case
         *  prev_id =
            ((op_type == SAI_OP_CREATE) ? SAI_NULL_OBJECT_ID : p_qos_port_node->scheduler_id); */
        sai_rc = sai_qos_scheduler_port_list_update (p_qos_port_node,
                                                     p_qos_port_node->scheduler_id,
                                                     sched_id);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR ("Failed to update port in scheduler node.");
            break;
        }

        p_qos_port_node->scheduler_id = sched_id;
        SAI_SCHED_LOG_TRACE ("Qos Scheduler 0x%"PRIx64" set success on port 0x%"PRIx64"",
                             sched_id, port_id);
   } while(0);

   sai_qos_unlock();

   return sai_rc;
}

sai_status_t sai_qos_create_default_scheduler(sai_object_id_t *default_sched_id)
{
    STD_ASSERT(default_sched_id != NULL);

    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    uint32_t attr_count = 0;
    sai_attribute_t attr[2];

    attr[attr_count].id = SAI_SCHEDULER_ATTR_SCHEDULING_TYPE;
    attr[attr_count].value.s32 = SAI_SCHEDULING_TYPE_WRR;

    attr_count ++;

    attr[attr_count].id = SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT;
    attr[attr_count].value.u8 = 1;

    attr_count ++;

    sai_rc = sai_qos_scheduler_create_internal (default_sched_id, sai_switch_id_get(),
                                      attr_count, &attr[0]);

    return sai_rc;
}

sai_status_t sai_qos_remove_default_scheduler()
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t default_sched_id = SAI_NULL_OBJECT_ID;

    default_sched_id = sai_qos_default_sched_id_get ();

    if (default_sched_id != SAI_NULL_OBJECT_ID) {

        sai_rc = sai_qos_scheduler_remove_internal (default_sched_id);

        if (sai_rc == SAI_STATUS_SUCCESS)
            sai_qos_default_sched_id_set(SAI_NULL_OBJECT_ID);

        return sai_rc;
    }

    return SAI_STATUS_SUCCESS;
}


static sai_scheduler_api_t sai_qos_scheduler_method_table = {
    sai_qos_scheduler_create,
    sai_qos_scheduler_remove,
    sai_qos_scheduler_attribute_set,
    sai_qos_scheduler_attribute_get,
};

sai_scheduler_api_t *sai_qos_scheduler_api_query (void)
{
    return (&sai_qos_scheduler_method_table);
}

