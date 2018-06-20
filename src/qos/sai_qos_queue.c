/************************************************************************
* LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file  sai_qos_queue.c
*
* @brief This file contains function definitions for SAI QOS queue
*        initilization and SAI queue functionality API implementation.
*
*************************************************************************/

#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_api_utils.h"
#include "sai_qos_mem.h"
#include "sai_switch_utils.h"
#include "sai_common_infra.h"
#include "sai_qos_buffer_util.h"

#include "saistatus.h"

#include "std_assert.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#define SAI_QOS_QUEUE_DFLT_ATTR_COUNT 5

static void sai_qos_queue_node_init (dn_sai_qos_queue_t *p_queue_node)
{
    p_queue_node->port_id = SAI_NULL_OBJECT_ID;
    p_queue_node->queue_type = SAI_QUEUE_TYPE_ALL;
    p_queue_node->queue_index = 0xFF;
    p_queue_node->parent_sched_group_id = SAI_NULL_OBJECT_ID;
    p_queue_node->child_offset = SAI_QOS_CHILD_INDEX_INVALID;
    p_queue_node->wred_id = SAI_NULL_OBJECT_ID;
    p_queue_node->scheduler_id = SAI_NULL_OBJECT_ID;
    p_queue_node->buffer_profile_id = SAI_NULL_OBJECT_ID;

    return;
}

sai_status_t sai_qos_queue_remove_configs(dn_sai_qos_queue_t *p_queue_node)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_status_t rev_sai_rc;
    sai_attribute_t set_attr;
    bool wred_removed = false;
    bool buffer_profile_removed = false;
    sai_object_id_t buffer_profile_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t wred_id = SAI_NULL_OBJECT_ID;


    STD_ASSERT (p_queue_node != NULL);

    buffer_profile_id = p_queue_node->buffer_profile_id;
    wred_id = p_queue_node->wred_id;

    do {
        if(p_queue_node->buffer_profile_id != SAI_NULL_OBJECT_ID) {
            sai_qos_wred_link_update_cache(p_queue_node->key.queue_id);

            sai_rc = sai_qos_obj_update_buffer_profile(p_queue_node->key.queue_id, SAI_NULL_OBJECT_ID);

            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_QUEUE_LOG_ERR ("Error: Unable to remove buffer profile from queue:0x%"PRIx64"",
                        p_queue_node->key.queue_id);
                break;
            }
            p_queue_node->buffer_profile_id = SAI_NULL_OBJECT_ID;
            buffer_profile_removed = true;
        }
        if(p_queue_node->wred_id != SAI_NULL_OBJECT_ID) {
            sai_rc = sai_qos_wred_link_set(p_queue_node->key.queue_id,
                    SAI_NULL_OBJECT_ID, DN_SAI_QOS_WRED_LINK_QUEUE);

            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_QUEUE_LOG_ERR ("Error: Unable to remove WRED profile from queue:0x%"PRIx64"",
                        p_queue_node->key.queue_id);
                break;
            }
            p_queue_node->wred_id = SAI_NULL_OBJECT_ID;
            wred_removed = true;
        }

        if((p_queue_node->scheduler_id != SAI_NULL_OBJECT_ID) &&
           (p_queue_node->scheduler_id != sai_qos_default_sched_id_get())) {
            set_attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
            set_attr.value.oid = SAI_NULL_OBJECT_ID;

            sai_rc = sai_qos_queue_scheduler_set(p_queue_node, &set_attr);

            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_QUEUE_LOG_ERR ("Error: Unable to remove scheduler profile from queue:0x%"PRIx64"",
                        p_queue_node->key.queue_id);
                break;
            }
        }
    } while (0);


    if(sai_rc != SAI_STATUS_SUCCESS) {
        if(buffer_profile_removed) {
            rev_sai_rc =  sai_qos_obj_update_buffer_profile (p_queue_node->key.queue_id,
                                                             buffer_profile_id);
            if(rev_sai_rc != SAI_STATUS_SUCCESS) {
                SAI_QUEUE_LOG_ERR ("Unable to revert buffer profile on queue 0x%"PRIx64" Error:%d",
                                    p_queue_node->key.queue_id, rev_sai_rc);
            } else {
                p_queue_node->buffer_profile_id = buffer_profile_id;
                sai_qos_wred_link_apply_cache(p_queue_node->key.queue_id);
            }
        }
        if(wred_removed) {
            rev_sai_rc = sai_qos_wred_link_set(p_queue_node->key.queue_id,
                    wred_id, DN_SAI_QOS_WRED_LINK_QUEUE);
            if(rev_sai_rc != SAI_STATUS_SUCCESS) {
                SAI_QUEUE_LOG_ERR ("Unable to revert wred profile on queue 0x%"PRIx64" Error:%d",
                                    p_queue_node->key.queue_id, rev_sai_rc);
            } else {
                p_queue_node->wred_id = wred_id;
            }
        }
    }
    return sai_rc;

}

static void sai_qos_queue_free_resources (dn_sai_qos_queue_t *p_queue_node,
                                          bool is_queue_set_in_npu,
                                          bool is_queue_set_in_port_list,
                                          bool is_queue_attach_to_parent)
{
    if (p_queue_node == NULL) {
        return;
    }

    if (is_queue_attach_to_parent)
    {
        sai_queue_npu_api_get()->queue_detach_from_parent(p_queue_node->key.queue_id);

        sai_qos_sched_group_and_child_nodes_update
                                (p_queue_node->parent_sched_group_id,
                                 p_queue_node->key.queue_id, false);
    }
    /* Remove Queue from NPU, if it was already applied  created. */
    if (is_queue_set_in_npu) {
        sai_queue_npu_api_get()->queue_remove (p_queue_node);
    }

    /* Delete Queue node from the PORT's Queue list, if it was already added. */
    if (is_queue_set_in_port_list) {
        sai_qos_port_queue_list_update (p_queue_node, false);
    }

    sai_qos_queue_node_free (p_queue_node);

    return;
}

static sai_status_t sai_qos_queue_node_insert_to_tree (
                                     dn_sai_qos_queue_t *p_queue_node)
{
    rbtree_handle   queue_tree = NULL;
    t_std_error     err_rc = STD_ERR_OK;

    STD_ASSERT (p_queue_node != NULL);

    queue_tree = sai_qos_access_global_config()->queue_tree;

    STD_ASSERT (queue_tree != NULL);

    err_rc = std_rbtree_insert (queue_tree, p_queue_node);

    if (STD_IS_ERR(err_rc)) {
        SAI_QUEUE_LOG_ERR ("Failed to insert Queue node for QID 0x%"PRIx64" "
                           "into Queue Tree", p_queue_node->key.queue_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static void sai_qos_queue_node_remove_from_tree (dn_sai_qos_queue_t *p_queue_node)
{
    rbtree_handle   queue_tree = NULL;

    STD_ASSERT (p_queue_node != NULL);

    queue_tree = sai_qos_access_global_config()->queue_tree;

    STD_ASSERT (queue_tree != NULL);

    std_rbtree_remove (queue_tree, p_queue_node);

    return;
}

static sai_status_t sai_qos_queue_modify_parent (
                                        sai_object_id_t queue_id,
                                        dn_sai_qos_queue_t *p_queue_node,
                                        sai_object_id_t new_parent_id)
{
    sai_status_t                sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t             old_parent_id = SAI_NULL_OBJECT_ID;

    old_parent_id = p_queue_node->parent_sched_group_id;

    SAI_QUEUE_LOG_TRACE ("Queue modify parent. Port ID 0x%"PRIx64", "
                         "queue id 0x%"PRIx64" parent sg 0x%"PRIx64"",
                         p_queue_node->port_id,
                         queue_id, new_parent_id);

    if (new_parent_id != SAI_NULL_OBJECT_ID) {
        sai_rc = sai_queue_npu_api_get()->queue_modify_parent
                                                (queue_id, new_parent_id);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" modify parent from "
                               "0x%"PRIx64" to 0x%"PRIx64" failed in NPU.", queue_id,
                               old_parent_id, new_parent_id);
            return sai_rc;
        }
    } else {
        sai_rc = sai_queue_npu_api_get()->queue_remove (p_queue_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" remove failed in NPU.",
                               queue_id);
            return sai_rc;
        }
    }

    /* Update parent HQos information */
    sai_rc = sai_qos_sched_group_and_child_nodes_update (old_parent_id,
                                                         queue_id, false);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_QUEUE_LOG_ERR ("Failed to update the child and parent hierarchy "
                               "information with old parent details.");
        return sai_rc;
    }

    if (new_parent_id != SAI_NULL_OBJECT_ID) {
        sai_rc = sai_qos_sched_group_and_child_nodes_update (new_parent_id,
                                                             queue_id, true);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Failed to update the child and parent hierarchy "
                                   "information with new parent details.");
        }
    }
    return sai_rc;
}

static void sai_qos_queue_attr_set (dn_sai_qos_queue_t *p_queue_node,
                                    uint_t attr_count,
                                    const sai_attribute_t *p_attr_list,
                                    dn_sai_operations_t op_type)
{
    const sai_attribute_t  *p_attr = NULL;
    uint_t                 list_index = 0;

    STD_ASSERT(p_queue_node != NULL);
    STD_ASSERT(p_attr_list != NULL);

    SAI_QUEUE_LOG_TRACE ("Set attributes for Queue, attribute count %d "
                         "op_type %d.", attr_count, op_type);

    for (list_index = 0, p_attr = p_attr_list;
         list_index < attr_count; ++list_index, ++p_attr) {

        switch (p_attr->id)
        {
            case SAI_QUEUE_ATTR_TYPE:
                p_queue_node->queue_type = p_attr->value.s32;
                break;

            case SAI_QUEUE_ATTR_PORT:
                p_queue_node->port_id = p_attr->value.oid;
                break;

            case SAI_QUEUE_ATTR_INDEX:
                p_queue_node->queue_index = p_attr->value.u8;
                break;

            case SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE:
                p_queue_node->parent_sched_group_id = p_attr->value.oid;
                break;

            case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
                p_queue_node->scheduler_id = p_attr->value.oid;
                break;

            case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
                p_queue_node->wred_id = p_attr->value.oid;
                break;

            case SAI_QUEUE_ATTR_BUFFER_PROFILE_ID:
                p_queue_node->buffer_profile_id = p_attr->value.oid;
                break;

            default:
                SAI_QUEUE_LOG_TRACE ("Attribute id: %d - read-only attribute.",
                                     p_attr->id);
                break;
        }
    }
}

static sai_status_t sai_qos_queue_attributes_validate (uint_t attr_count,
                                        const sai_attribute_t *attr_list,
                                        dn_sai_operations_t op_type)
{
    sai_status_t                    sai_rc = SAI_STATUS_SUCCESS;
    uint_t                          max_vendor_attr_count = 0;
    const dn_sai_attribute_entry_t  *p_vendor_attr = NULL;
    const sai_attribute_t           *p_attr = NULL;
    uint_t                          list_index = 0;

    SAI_QUEUE_LOG_TRACE ("Parsing attributes for queue, attribute count %d "
                         "op_type %d.", attr_count, op_type);

    if (attr_count == 0)
        return SAI_STATUS_INVALID_PARAMETER;

    sai_queue_npu_api_get()->attribute_table_get(&p_vendor_attr,
                                                 &max_vendor_attr_count);

    STD_ASSERT(p_vendor_attr != NULL);
    STD_ASSERT(max_vendor_attr_count > 0);

    sai_rc = sai_attribute_validate (attr_count, attr_list, p_vendor_attr,
                                     op_type, max_vendor_attr_count);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_QUEUE_LOG_ERR ("Attribute validation failed for %d "
                           "operation", op_type);
        return sai_rc;
    }

    if (op_type == SAI_OP_GET)
        return sai_rc;

    for (list_index = 0, p_attr = attr_list;
         (list_index < attr_count) && (p_attr != NULL);
         list_index++, p_attr++) {

         switch (p_attr->id) {
             case SAI_QUEUE_ATTR_PORT:
                 if ((p_attr->value.oid == SAI_NULL_OBJECT_ID)
                          || (sai_qos_port_node_get (p_attr->value.oid)
                                               == NULL)) {
                        return SAI_STATUS_INVALID_OBJECT_ID;

                 }
                 break;

             case SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE:
                 if ((p_attr->value.oid != SAI_NULL_OBJECT_ID)
                            && (sai_qos_sched_group_node_get
                                              (p_attr->value.oid) == NULL)) {
                     return SAI_STATUS_INVALID_OBJECT_ID;

                 }
                 break;

             case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
                 if ((p_attr->value.oid != SAI_NULL_OBJECT_ID)
                           && (sai_qos_scheduler_node_get
                                                (p_attr->value.oid) == NULL)) {
                     return SAI_STATUS_INVALID_OBJECT_ID;

                 }
                 break;

             case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
                 if ((p_attr->value.oid != SAI_NULL_OBJECT_ID)
                           && (sai_qos_wred_node_get
                                            (p_attr->value.oid) == NULL)) {
                     return SAI_STATUS_INVALID_OBJECT_ID;

                 }
                 break;

             case SAI_QUEUE_ATTR_BUFFER_PROFILE_ID:
                 if ((p_attr->value.oid != SAI_NULL_OBJECT_ID)
                           && (sai_qos_buffer_profile_node_get
                                             (p_attr->value.oid) == NULL)) {

                     return SAI_STATUS_INVALID_OBJECT_ID;

                 }
                 break;

             default:
                 break;
         }
    }

    return sai_rc;
}

/* Attribute validation happen before this function */
static bool sai_qos_queue_is_duplicate_set (dn_sai_qos_queue_t  *p_queue_node,
                                            const sai_attribute_t *p_attr)
{
    STD_ASSERT(p_queue_node != NULL);
    STD_ASSERT(p_attr != NULL);

    SAI_QUEUE_LOG_TRACE ("Verify duplicate set attributes value, ID: %d.",
                         p_attr->id);

    switch (p_attr->id)
    {
        case SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE:
            if (p_queue_node->parent_sched_group_id == p_attr->value.oid)
                return true;
            break;

        case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
            if (p_queue_node->scheduler_id == p_attr->value.oid)
                return true;
            break;

        case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
            if (p_queue_node->wred_id == p_attr->value.oid)
                return true;
            break;

        case SAI_QUEUE_ATTR_BUFFER_PROFILE_ID:
            if (p_queue_node->buffer_profile_id == p_attr->value.oid)
                return true;
            break;

        default:
            SAI_QUEUE_LOG_TRACE ("Attribute id: %d - read-only attribute.",
                                 p_attr->id);
            break;
    }

    return false;
}

static sai_status_t sai_qos_queue_create_internal(sai_object_id_t *queue_id,
                                         sai_object_id_t switch_id,
                                         uint32_t attr_count,
                                         const sai_attribute_t *attr_list)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_queue_t  *p_queue_node = NULL;
    bool                is_queue_set_in_npu = false;
    bool                is_queue_set_in_port_list = false;
    sai_attribute_t     attr;
    bool                attach = false;
    sai_object_id_t     buffer_profile_id = SAI_NULL_OBJECT_ID;


    sai_rc = sai_qos_queue_attributes_validate (attr_count, attr_list,
                                                    SAI_OP_CREATE);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_QUEUE_LOG_ERR ("Input parameters validation failed for "
                           "Queue create");

        return sai_rc;
    }

    do {
        p_queue_node = sai_qos_queue_node_alloc ();

        if (NULL == p_queue_node) {
            SAI_QUEUE_LOG_ERR ("Queue node memory allocation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;

            break;
        }

        sai_qos_queue_node_init (p_queue_node);

        sai_qos_queue_attr_set (p_queue_node, attr_count, attr_list, SAI_OP_CREATE);

        sai_rc = sai_queue_npu_api_get()->queue_create (p_queue_node,
                                                        queue_id);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Queue creation failed in NPU.");
            break;
        }

        p_queue_node->key.queue_id = *queue_id;

        SAI_QUEUE_LOG_TRACE ("Queue Created in NPU.");

        is_queue_set_in_npu = true;

        /* Add the Queue node to PORT's Queue list */
        sai_rc = sai_qos_port_queue_list_update (p_queue_node, true);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Failed to add Queue node to the "
                               "Queue List in PORT node");
            break;
        }

        is_queue_set_in_port_list = true;

        sai_rc = sai_qos_queue_node_insert_to_tree (p_queue_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Queue insertion to tree failed.");
            break;
        }

        if (p_queue_node->parent_sched_group_id != SAI_NULL_OBJECT_ID) {
            sai_rc = sai_queue_npu_api_get()->queue_attach_to_parent(*queue_id,
                                                p_queue_node->parent_sched_group_id);
            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_QUEUE_LOG_ERR ("Queue attach to parent SG 0x%"PRIx64" failed.",
                               p_queue_node->parent_sched_group_id);
                break;
            }

            sai_rc = sai_qos_sched_group_and_child_nodes_update
                                              (p_queue_node->parent_sched_group_id,
                                               *queue_id, true);
            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_QUEUE_LOG_ERR ("Failed to update the child and parent hierarchy "
                                   "information.");
                (void) sai_queue_npu_api_get()->queue_detach_from_parent(*queue_id);
                break;
            }
        }

        attach = true;

        attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
        attr.value.oid = p_queue_node->scheduler_id;

        p_queue_node->scheduler_id = SAI_NULL_OBJECT_ID;
        sai_rc = sai_qos_queue_scheduler_set(p_queue_node, &attr);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR("Scheduler set failed for qid0x%"PRIx64"",
                                   *queue_id);
            break;
        }

        if(p_queue_node->wred_id != SAI_NULL_OBJECT_ID) {
            if(p_queue_node->queue_type != SAI_QUEUE_TYPE_UNICAST){
                SAI_QUEUE_LOG_ERR("Queue 0x%"PRIx64" is not of type unicast to apply WRED.", *queue_id);
                sai_rc = SAI_STATUS_INVALID_ATTR_VALUE_0;
                break;
            }

            sai_rc = sai_qos_wred_link_set(*queue_id, p_queue_node->wred_id, DN_SAI_QOS_WRED_LINK_QUEUE);

            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_QUEUE_LOG_ERR ("Failed to apply WRED profile 0x%"PRIx64" to Queue 0x%"PRIx64".",
                        p_queue_node->wred_id, *queue_id);
                break;
            }
        }

        if(p_queue_node->buffer_profile_id != SAI_NULL_OBJECT_ID) {
            buffer_profile_id = p_queue_node->buffer_profile_id;
            p_queue_node->buffer_profile_id = SAI_NULL_OBJECT_ID;

            sai_rc = sai_qos_obj_update_buffer_profile(*queue_id, buffer_profile_id);
            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_QUEUE_LOG_ERR ("Failed to apply buffer profile 0x%"PRIx64" to queue:0x%"PRIx64"",
                        buffer_profile_id, *queue_id);
                break;
            }

            p_queue_node->buffer_profile_id = buffer_profile_id;
            sai_qos_wred_link_apply_cache(*queue_id);
        }

    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_QUEUE_LOG_INFO ("Queue Obj Id: 0x%"PRIx64" created.", *queue_id);
    } else {
        SAI_QUEUE_LOG_ERR ("Failed to create Queue.");

        sai_qos_queue_remove_configs (p_queue_node);

        sai_qos_queue_free_resources (p_queue_node, is_queue_set_in_npu,
                                      is_queue_set_in_port_list, attach);
    }

    return sai_rc;
}

static sai_status_t sai_qos_queue_create (sai_object_id_t *queue_id,
                                          sai_object_id_t switch_id,
                                          uint32_t attr_count,
                                          const sai_attribute_t *attr_list)
{
    sai_status_t         sai_rc = SAI_STATUS_SUCCESS;
    sai_qos_lock ();
    sai_rc =  sai_qos_queue_create_internal(queue_id, switch_id,
                                          attr_count,attr_list);
    sai_qos_unlock();
    return sai_rc;
}

static sai_status_t sai_qos_queue_remove_internal (sai_object_id_t queue_id)
{
    sai_status_t         sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_queue_t  *p_queue_node = NULL;

    SAI_QUEUE_LOG_TRACE ("Queue remove QID 0x%"PRIx64".", queue_id);

    if (! sai_is_obj_id_queue (queue_id)) {
        SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" is not a valid queue obj id.",
                           queue_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    do {
        p_queue_node = sai_qos_queue_node_get (queue_id);

        if (NULL == p_queue_node) {
            SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" does not exist in tree.",
                               queue_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }

        SAI_QUEUE_LOG_TRACE ("Queue remove. Port ID 0x%"PRIx64", "
                             "Q ID 0x%"PRIx64", Q Type: %s.", queue_id,
                             p_queue_node->port_id,
                             sai_qos_queue_type_to_str(p_queue_node->queue_type));

        /*TODO: Need to verify the presence of this queue in the TC to QUEUE mapping
         *      for this current queue and throw in use error if it is present
         */

        sai_rc = sai_qos_queue_remove_configs (p_queue_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" configs can't be deleted", queue_id);
            break;
        }

        sai_rc = sai_queue_npu_api_get()->queue_detach_from_parent(queue_id);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" failed to detach from parent SG 0x%"PRIx64"",
                               queue_id, p_queue_node->parent_sched_group_id);
            break;
        }

        sai_rc = sai_qos_sched_group_and_child_nodes_update
                                (p_queue_node->parent_sched_group_id,
                                 queue_id, false);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" failed to remove from parent SG 0x%"PRIx64" child list",
                               queue_id, p_queue_node->parent_sched_group_id);
            break;
        }

        sai_rc = sai_queue_npu_api_get()->queue_remove (p_queue_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" deletion failed in NPU.",
                                queue_id);

            break;
        }

        /* Delete from the PORT's queue list */
        sai_qos_port_queue_list_update (p_queue_node, false);

        sai_qos_queue_node_remove_from_tree (p_queue_node);

        sai_qos_queue_node_free (p_queue_node);

    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_QUEUE_LOG_INFO ("Queue 0x%"PRIx64" removed.", queue_id);
    } else {
        SAI_QUEUE_LOG_ERR ("Failed to remove Queue 0x%"PRIx64".", queue_id);
    }

    return sai_rc;
}

static sai_status_t sai_qos_queue_remove (sai_object_id_t queue_id)
{
    sai_status_t         sai_rc = SAI_STATUS_SUCCESS;
    sai_qos_lock ();
    sai_rc = sai_qos_queue_remove_internal (queue_id);
    sai_qos_unlock();
    return sai_rc;
}

static sai_status_t sai_qos_queue_attribute_set (sai_object_id_t queue_id,
                                                 const sai_attribute_t *p_attr)
{
    sai_status_t                sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_queue_t          *p_queue_node = NULL;
    uint_t                       attr_count = 1;

    STD_ASSERT (p_attr != NULL);

    SAI_QUEUE_LOG_TRACE ("Setting Attribute ID %d on Queue 0x%"PRIx64".",
                         p_attr->id, queue_id);

    if (! sai_is_obj_id_queue (queue_id)) {
        SAI_QUEUE_LOG_ERR ("%"PRIu64" is not a valid Queue obj id.",
                           queue_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_qos_lock ();

    do {
        p_queue_node = sai_qos_queue_node_get (queue_id);

        if (NULL == p_queue_node) {
            SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" does not exist in tree.",
                               queue_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }

        sai_rc = sai_qos_queue_attributes_validate (attr_count, p_attr,
                                                    SAI_OP_SET);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Input parameters validation failed for "
                               "Queue attributes set.");

            break;
        }

        if (sai_qos_queue_is_duplicate_set (p_queue_node, p_attr)) {
            SAI_QUEUE_LOG_TRACE ("Duplicate set value for Attribute ID %d.",
                                 p_attr->id);
            break;
        }

        switch (p_attr->id)
        {

            case SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE:
                sai_rc = sai_qos_queue_modify_parent (queue_id,
                                                      p_queue_node,
                                                      p_attr->value.oid);
                break;

            case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
                sai_rc = sai_qos_queue_scheduler_set(p_queue_node, p_attr);

                break;

            case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
                if(p_queue_node->queue_type != SAI_QUEUE_TYPE_UNICAST){
                    SAI_QUEUE_LOG_ERR("Queue 0x%"PRIx64" is not of type unicast to apply WRED.", queue_id);
                    sai_rc = SAI_STATUS_INVALID_ATTR_VALUE_0;
                } else {
                    sai_rc = sai_qos_wred_link_set(queue_id, p_attr->value.oid, DN_SAI_QOS_WRED_LINK_QUEUE);
                }
                break;

            case SAI_QUEUE_ATTR_BUFFER_PROFILE_ID:
                sai_qos_wred_link_update_cache(queue_id);

                sai_rc = sai_qos_obj_update_buffer_profile(queue_id, p_attr->value.oid);
                if(SAI_STATUS_SUCCESS == sai_rc) {
                    p_queue_node->buffer_profile_id = p_attr->value.oid;
                }

                sai_qos_wred_link_apply_cache(queue_id);
                break;
            default:
                sai_rc = sai_queue_npu_api_get()->queue_attribute_set(p_queue_node,
                                                                      attr_count,
                                                                      p_attr);
                break;
        }

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Failed to set queue Attribute ID: %d "
                               "in NPU, Error: %d.", p_attr->id, sai_rc);
            break;
        }

        sai_qos_queue_attr_set (p_queue_node, attr_count, p_attr, SAI_OP_SET);

    } while (0);

    sai_qos_unlock ();

    return sai_rc;
}

static sai_status_t sai_qos_queue_attribute_get (sai_object_id_t queue_id,
                                                 uint32_t attr_count,
                                                 sai_attribute_t *p_attr_list)
{
    sai_status_t                sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_queue_t          *p_queue_node = NULL;

    STD_ASSERT (p_attr_list != NULL);

    SAI_QUEUE_LOG_TRACE ("Getting Attributes for queue 0x%"PRIx64", "
                         "attr_count %d.", queue_id, attr_count);

    if (! sai_is_obj_id_queue (queue_id)) {
        SAI_QUEUE_LOG_ERR ("%"PRIu64" is not a valid Queue obj id.",
                           queue_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_qos_lock ();

    do {
        p_queue_node = sai_qos_queue_node_get (queue_id);

        if (NULL == p_queue_node) {
            SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" does not exist in tree.", queue_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }

        sai_rc = sai_qos_queue_attributes_validate (attr_count, p_attr_list,
                                                    SAI_OP_GET);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Input parameters validation failed for "
                               "Queue attributes get.");

            break;
        }

        sai_rc = sai_queue_npu_api_get()->queue_attribute_get (p_queue_node,
                                                               attr_count,
                                                               p_attr_list);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Failed to get Queue Attributes from "
                               "NPU, Error: %d.", sai_rc);

            break;
        }
    } while (0);

    sai_qos_unlock ();

    return sai_rc;
}

sai_status_t sai_qos_port_queue_create(sai_object_id_t port_id,
                                       sai_queue_type_t queue_type,
                                       uint8_t queue_index,
                                       sai_object_id_t parent_sg_id,
                                       sai_object_id_t *queue_id)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    uint_t           attr_count = 0;
    sai_attribute_t  attr_list[SAI_QOS_QUEUE_DFLT_ATTR_COUNT] = {{0}};

    STD_ASSERT(queue_id != NULL);

    SAI_QUEUE_LOG_TRACE ("Queue Creation for port 0x%"PRIx64", queue_type %s."
                         "queue index %u parent SG 0x%"PRIx64"",
                         port_id, sai_qos_queue_type_to_str (queue_type),
                         queue_index, parent_sg_id);

    attr_count = 0;

    attr_list[attr_count].id = SAI_QUEUE_ATTR_PORT;
    attr_list[attr_count].value.oid = port_id;
    attr_count ++;

    attr_list[attr_count].id = SAI_QUEUE_ATTR_TYPE;
    attr_list[attr_count].value.s32 = queue_type;
    attr_count ++;

    attr_list[attr_count].id = SAI_QUEUE_ATTR_INDEX;
    attr_list[attr_count].value.u8 = queue_index;
    attr_count ++;

    attr_list[attr_count].id = SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE;
    attr_list[attr_count].value.oid = parent_sg_id;
    attr_count ++;

    attr_list[attr_count].id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
    attr_list[attr_count].value.oid = sai_qos_default_sched_id_get();
    attr_count ++;

    sai_rc = sai_qos_queue_create_internal(queue_id, SAI_DEFAULT_SWITCH_ID,
                                  attr_count, &attr_list[0]);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_QUEUE_LOG_ERR ("Failed to create queue for port 0x%"PRIx64""
                           "queue_type %s and queue index %u.",
                           port_id, sai_qos_queue_type_to_str (queue_type),
                           queue_index);
        return sai_rc;
    }

    SAI_QUEUE_LOG_TRACE ("Successfully created queue for port 0x%"PRIx64""
                         "queue_type %s and queue index %u.",
                         port_id, sai_qos_queue_type_to_str (queue_type),
                         queue_index);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_port_queue_remove(sai_object_id_t queue_id)
{
    return sai_qos_queue_remove_internal(queue_id);
}

sai_status_t sai_qos_port_queue_all_init (sai_object_id_t port_id)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    uint_t          queue = 0;
    uint_t          max_queues = 0;
    sai_object_id_t queue_id = SAI_NULL_OBJECT_ID;

    SAI_QUEUE_LOG_TRACE ("Port Queue All Init.");

    if (sai_qos_queue_is_seperate_ucast_or_mcast_supported (port_id)) {

        max_queues = sai_switch_max_uc_queues_per_port_get (port_id);

        for (queue = 0; queue < max_queues; queue++) {

            sai_rc = sai_qos_port_queue_create (port_id, SAI_QUEUE_TYPE_UNICAST, queue,
                                           SAI_NULL_OBJECT_ID, &queue_id);

            if (sai_rc != SAI_STATUS_SUCCESS) {

                SAI_QUEUE_LOG_ERR ("Unicast Queue create failed "
                                   "for port 0x%"PRIx64", Queue %d sai_rc=%d.",
                                   port_id, queue, sai_rc);
                return sai_rc;
            }
        }

        SAI_QUEUE_LOG_TRACE ("Port Unicast Queues Init success.");
        max_queues = sai_switch_max_mc_queues_per_port_get (port_id);

        for (queue = 0; queue < max_queues; queue++) {

            sai_rc = sai_qos_port_queue_create (port_id, SAI_QUEUE_TYPE_MULTICAST, queue,
                                           SAI_NULL_OBJECT_ID, &queue_id);

            if (sai_rc != SAI_STATUS_SUCCESS) {

                SAI_QUEUE_LOG_ERR ("Multicast Queue create failed "
                                   "for port 0x%"PRIx64", Queue %d sai_rc=%d.",
                                   port_id, queue, sai_rc);
                return sai_rc;
            }
        }
    } else {

        max_queues = sai_switch_max_queues_per_port_get (port_id);

        for (queue = 0; queue < max_queues; queue++) {

            sai_rc = sai_qos_port_queue_create (port_id, SAI_QUEUE_TYPE_ALL, queue,
                                           SAI_NULL_OBJECT_ID, &queue_id);

            if (sai_rc != SAI_STATUS_SUCCESS) {

                SAI_QUEUE_LOG_ERR ("Queue create failed "
                                   "for Port 0x%"PRIx64", Queue %d sai_rc=%d.",
                                   port_id, queue, sai_rc);
                return sai_rc;
            }
        }
    }

    SAI_QUEUE_LOG_INFO ("Port 0x%"PRIx64", All Queue Init complete.", port_id);

    return sai_rc;
}

sai_status_t sai_qos_port_queue_all_deinit (sai_object_id_t port_id)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_queue_t *p_queue_node = NULL;
    dn_sai_qos_queue_t *p_next_queue_node = NULL;
    dn_sai_qos_port_t  *p_qos_port_node = NULL;
    sai_object_id_t    queue_id = SAI_NULL_OBJECT_ID;

    SAI_QUEUE_LOG_TRACE ("Port 0x%"PRIx64" Queue All De-Init.", port_id);

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {

        SAI_QUEUE_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                         port_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for (p_queue_node = sai_qos_port_get_first_queue (p_qos_port_node);
         p_queue_node != NULL; p_queue_node = p_next_queue_node) {

        p_next_queue_node = sai_qos_port_get_next_queue (p_qos_port_node,
                                                         p_queue_node);
        queue_id = p_queue_node->key.queue_id;

        sai_rc = sai_qos_queue_remove_internal (queue_id);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Queue remove failed "
                               "for port 0x%"PRIx64", Queue 0x%"PRIx64".",
                               port_id, queue_id);
            return sai_rc;
        }
    }

    SAI_QUEUE_LOG_INFO ("Port 0x%"PRIx64", All Queue De-Init success.", port_id);
    return sai_rc;
}

sai_status_t sai_qos_first_free_queue_get (sai_object_id_t port_id,
                                           sai_queue_type_t queue_type,
                                           sai_object_id_t *p_queue_id)
{
    dn_sai_qos_port_t        *p_qos_port_node = NULL;
    dn_sai_qos_queue_t       *p_queue_node = NULL;

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {

        SAI_QUEUE_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                               port_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for (p_queue_node = sai_qos_port_get_first_queue (p_qos_port_node);
         p_queue_node != NULL; p_queue_node =
         sai_qos_port_get_next_queue (p_qos_port_node, p_queue_node)) {

        if (p_queue_node->queue_type != queue_type)
            continue;

        if (SAI_NULL_OBJECT_ID == p_queue_node->parent_sched_group_id) {
            *p_queue_id = p_queue_node->key.queue_id;
             return SAI_STATUS_SUCCESS;
        }
    }
    return SAI_STATUS_INSUFFICIENT_RESOURCES;
}

sai_status_t sai_qos_queue_id_list_get (sai_object_id_t port_id,
                                        uint_t queue_id_list_count,
                                        sai_object_id_t *p_queue_id_list)
{
    dn_sai_qos_port_t        *p_qos_port_node = NULL;
    dn_sai_qos_queue_t       *p_queue_node = NULL;
    int                      queue_count = 0;

    if (0 == queue_id_list_count)
        return SAI_STATUS_SUCCESS;

    STD_ASSERT (p_queue_id_list != NULL);

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {
        SAI_QUEUE_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                           port_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for (p_queue_node = sai_qos_port_get_first_queue (p_qos_port_node);
         ((p_queue_node != NULL) && (queue_count < queue_id_list_count)); p_queue_node =
         sai_qos_port_get_next_queue (p_qos_port_node, p_queue_node)) {

        p_queue_id_list[queue_count] = p_queue_node->key.queue_id;
        queue_count++;
    }

    if (queue_id_list_count != queue_count) {
        SAI_QUEUE_LOG_ERR ("Required queues not exits in "
                           "port 0x%"PRIx64", "
                           "req count %d, queues present %d.",
                           port_id, queue_id_list_count, queue_count);
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_qos_queue_stats_get (sai_object_id_t queue_id,
                                             uint32_t number_of_counters,
                                             const sai_queue_stat_t *counter_ids,
                                             uint64_t* counters)
{

    sai_status_t                sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_queue_t          *p_queue_node = NULL;

    if (counter_ids == NULL) {
        SAI_QUEUE_LOG_ERR("Invalid parameter counter_ids is NULL");
        STD_ASSERT(0);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (counters == NULL) {
        SAI_QUEUE_LOG_ERR("Invalid parameter counters is NULL");
        STD_ASSERT(0);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (number_of_counters == 0) {
        SAI_QUEUE_LOG_ERR("Invalid parameter number_of_counters is zero");
        STD_ASSERT(0);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    SAI_QUEUE_LOG_TRACE ("Getting stats for queue 0x%"PRIx64"", queue_id);

    if (! sai_is_obj_id_queue (queue_id)) {
        SAI_QUEUE_LOG_ERR ("0x%"PRIx64" is not a valid Queue obj id.",
                           queue_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_qos_lock ();

    p_queue_node = sai_qos_queue_node_get (queue_id);

    if (NULL == p_queue_node) {
        SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" does not exist in tree.", queue_id);
        sai_qos_unlock ();
        return SAI_STATUS_INVALID_OBJECT_ID;

    }

    sai_rc = sai_queue_npu_api_get()->queue_stats_get (p_queue_node, counter_ids,
            number_of_counters, counters);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_QUEUE_LOG_ERR ("Failed to get Queue stats NPU, Error: %d.", sai_rc);
    }

    sai_qos_unlock ();

    return sai_rc;
}

static sai_status_t sai_qos_queue_stats_clear (sai_object_id_t queue_id,
                                               uint32_t number_of_counters,
                                               const sai_queue_stat_t *counter_ids)
{

    sai_status_t                sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_queue_t          *p_queue_node = NULL;

    if (counter_ids == NULL) {
        SAI_QUEUE_LOG_ERR("Invalid parameter counter_ids is NULL");
        STD_ASSERT(0);
        return SAI_STATUS_INVALID_PARAMETER;
    }


    if (number_of_counters == 0) {
        SAI_QUEUE_LOG_ERR("Invalid parameter number_of_counters is zero");
        STD_ASSERT(0);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    SAI_QUEUE_LOG_TRACE ("Clearing stats for queue 0x%"PRIx64"", queue_id);

    if (! sai_is_obj_id_queue (queue_id)) {
        SAI_QUEUE_LOG_ERR ("0x%"PRIx64" is not a valid Queue obj id.",
                           queue_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_qos_lock ();

    p_queue_node = sai_qos_queue_node_get (queue_id);

    if (NULL == p_queue_node) {
        SAI_QUEUE_LOG_ERR ("Queue 0x%"PRIx64" does not exist in tree.", queue_id);
        sai_qos_unlock ();
        return SAI_STATUS_INVALID_OBJECT_ID;

    }

    sai_rc = sai_queue_npu_api_get()->queue_stats_clear (p_queue_node, counter_ids,
                                                         number_of_counters);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_QUEUE_LOG_ERR ("Failed to clear Queue stats NPU, Error: %d.", sai_rc);
    }

    sai_qos_unlock ();

    return sai_rc;
}

static sai_queue_api_t sai_qos_queue_method_table = {
    sai_qos_queue_create,
    sai_qos_queue_remove,
    sai_qos_queue_attribute_set,
    sai_qos_queue_attribute_get,
    sai_qos_queue_stats_get,
    sai_qos_queue_stats_clear
};

sai_queue_api_t *sai_qos_queue_api_query (void)
{
    return (&sai_qos_queue_method_table);
}
