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
* @file  sai_qos_sched_group.c
*
* @brief This file contains function definitions for SAI QOS default
*        scheduler groups initilization and hiearchy creation and SAI
*        scheduler group functionality API implementation.
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
#include "std_bit_masks.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#define SAI_QOS_SCHED_GROUP_DFLT_ATTR_COUNT 5

static sai_status_t sai_qos_sched_group_node_init (dn_sai_qos_sched_group_t *p_sg_node)
{
    std_dll_init (&p_sg_node->hqos_info.child_sched_group_dll_head);

    std_dll_init (&p_sg_node->hqos_info.child_queue_dll_head);

    p_sg_node->parent_id = SAI_NULL_OBJECT_ID;

    p_sg_node->child_offset = SAI_QOS_CHILD_INDEX_INVALID;

    p_sg_node->max_childs = sai_switch_max_hierarchy_node_childs_get();

    p_sg_node->hqos_info.child_index_bitmap =
        std_bitmap_create_array (p_sg_node->max_childs);

    if (p_sg_node->hqos_info.child_index_bitmap == NULL) {
        SAI_SCHED_GRP_LOG_ERR ("Failed to create Hierarchy child index bitmap"
                               "array of size %d.", p_sg_node->max_childs);

        return SAI_STATUS_NO_MEMORY;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_sched_group_remove_configs (dn_sai_qos_sched_group_t *p_sg_node)
{
    sai_status_t sai_rc;
    sai_status_t rev_sai_rc;
    sai_attribute_t set_attr;
    sai_object_id_t sched_id = SAI_NULL_OBJECT_ID;

    STD_ASSERT (p_sg_node != NULL);

    if (p_sg_node->hierarchy_level == 0)
    {
        return SAI_STATUS_SUCCESS;
    }

    sched_id = p_sg_node->scheduler_id;

    if((sched_id != sai_qos_default_sched_id_get()) && (sched_id != SAI_NULL_OBJECT_ID)) {

        set_attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
        set_attr.value.oid = SAI_NULL_OBJECT_ID;

        sai_rc = sai_qos_sched_group_scheduler_set (p_sg_node, &set_attr);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Error: Unable to remove scheduler profile from sg:0x%"PRIx64"",
                    p_sg_node->key.sched_group_id);
            set_attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
            set_attr.value.oid = sched_id;
            rev_sai_rc = sai_qos_sched_group_scheduler_set (p_sg_node, &set_attr);
            if(rev_sai_rc != SAI_STATUS_SUCCESS) {
                SAI_SCHED_GRP_LOG_ERR ("Error unable to revert scheduler profile on sg:0x%"PRIx64"",
                        p_sg_node->key.sched_group_id);
            }
            return sai_rc;
        }
    }
    return SAI_STATUS_SUCCESS;

}

static void sai_qos_sched_group_free_resources (dn_sai_qos_sched_group_t *p_sg_node,
                                                bool is_sg_set_in_npu,
                                                bool is_sg_set_in_port_list,
                                                bool is_sg_add_to_parent)
{
    sai_object_id_t    sg_obj_id = SAI_NULL_OBJECT_ID;

    if (p_sg_node == NULL) {
        return;
    }

    sg_obj_id = p_sg_node->key.sched_group_id;
    if (is_sg_add_to_parent) {
        sai_sched_group_npu_api_get ()->sched_group_detach_from_parent(
                                              sg_obj_id);

        sai_qos_sched_group_and_child_nodes_update (p_sg_node->parent_id,
                                                    sg_obj_id, false);
    }

    /* Remove Scheduler group from NPU, if it was already applied created.*/
    if (is_sg_set_in_npu) {
        sai_sched_group_npu_api_get()->sched_group_remove (p_sg_node);
    }

    /* Delete Scheduler group node from the PORT's Scheduler group list,
     * if it was already added. */
    if (is_sg_set_in_port_list) {
        sai_qos_port_sched_group_list_update (p_sg_node, false);
    }

    std_bitmaparray_free_data(p_sg_node->hqos_info.child_index_bitmap);

    sai_qos_sched_group_node_free (p_sg_node);

}

static sai_status_t sai_qos_sched_group_node_insert_to_tree (
                                      dn_sai_qos_sched_group_t *p_sg_node)
{
    rbtree_handle   sg_tree = NULL;
    t_std_error     err_rc = STD_ERR_OK;

    STD_ASSERT (p_sg_node != NULL);

    sg_tree = sai_qos_access_global_config()->scheduler_group_tree;

    STD_ASSERT (sg_tree != NULL);

    err_rc = std_rbtree_insert (sg_tree, p_sg_node);

    if (STD_IS_ERR(err_rc)) {
        SAI_SCHED_GRP_LOG_ERR ("Failed to insert Scheduler group node for "
                               " SGID 0x%"PRIx64" into Scheduler group Tree",
                               p_sg_node->key.sched_group_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static void sai_qos_sched_group_node_remove_from_tree (
                                         dn_sai_qos_sched_group_t *p_sg_node)
{
    rbtree_handle   sg_tree = NULL;

    STD_ASSERT (p_sg_node != NULL);

    sg_tree = sai_qos_access_global_config()->scheduler_group_tree;

    STD_ASSERT (sg_tree != NULL);

    std_rbtree_remove (sg_tree, p_sg_node);

    return;
}

static void sai_qos_sched_group_attr_set (dn_sai_qos_sched_group_t *p_sg_node,
                                          uint_t attr_count,
                                          const sai_attribute_t *p_attr_list,
                                          dn_sai_operations_t op_type)
{
    const sai_attribute_t  *p_attr = NULL;
    uint_t                 list_index = 0;

    if ((op_type != SAI_OP_CREATE) && (op_type != SAI_OP_SET))
        return;

    STD_ASSERT(p_sg_node != NULL);
    STD_ASSERT(p_attr_list != NULL);

    SAI_SCHED_GRP_LOG_TRACE ("Set attributes for Scheduler group, "
                             "attribute count %d op_type %d.", attr_count,
                             op_type);

    for (list_index = 0, p_attr = p_attr_list;
         list_index < attr_count; ++list_index, ++p_attr) {

        switch (p_attr->id)
        {
            case SAI_SCHEDULER_GROUP_ATTR_PORT_ID:
                p_sg_node->port_id = p_attr->value.oid;
                break;

            case SAI_SCHEDULER_GROUP_ATTR_LEVEL:
                p_sg_node->hierarchy_level = p_attr->value.u32;
                break;

            case SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID:
                p_sg_node->scheduler_id = p_attr->value.oid;
                break;

            case SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS:
                p_sg_node->max_childs = p_attr->value.u32;
                break;

            case SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE:
                p_sg_node->parent_id = p_attr->value.oid;
                break;

            default:
                SAI_SCHED_GRP_LOG_TRACE ("Attribute id: %d - read-only attribute.",
                                         p_attr->id);
                break;
        }
    }
    return;
}

static sai_status_t sai_qos_sched_group_attributes_validate (
                                     uint_t attr_count,
                                     const sai_attribute_t *attr_list,
                                     dn_sai_operations_t op_type)
{
    sai_status_t                    sai_rc = SAI_STATUS_SUCCESS;
    uint_t                          max_vendor_attr_count = 0;
    const dn_sai_attribute_entry_t  *p_vendor_attr = NULL;
    uint_t                          list_index = 0;
    const sai_attribute_t           *p_attr = NULL;
    sai_object_type_t               parent_object_type = 0;

    SAI_SCHED_GRP_LOG_TRACE ("Parsing attributes for Scheduler group, "
                             "attribute count %d op_type %d.", attr_count,
                             op_type);

    do {
        if (attr_count == 0)
            return SAI_STATUS_INVALID_PARAMETER;

        sai_sched_group_npu_api_get()->attribute_table_get(&p_vendor_attr,
                                                       &max_vendor_attr_count);

        STD_ASSERT(p_vendor_attr != NULL);
        STD_ASSERT(max_vendor_attr_count > 0);

        sai_rc = sai_attribute_validate (attr_count, attr_list, p_vendor_attr,
                                         op_type, max_vendor_attr_count);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Attribute validation failed for %d "
                                   "operation", op_type);
            break;
        }

        if (op_type == SAI_OP_GET)
            break;

        for (list_index = 0, p_attr = attr_list;
             (list_index < attr_count) && (p_attr != NULL);
             list_index++, p_attr++) {

            switch (p_attr->id)
            {
                case SAI_SCHEDULER_GROUP_ATTR_PORT_ID :
                    if ((p_attr->value.oid == SAI_NULL_OBJECT_ID)
                          || (sai_qos_port_node_get (p_attr->value.oid)
                                               == NULL)) {
                        SAI_SCHED_GRP_LOG_ERR ("SAI_SCHEDULER_GROUP_ATTR_PORT_ID : 0x%"PRIx64""
                             "is invalid.", p_attr->value.oid);
                        return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                       list_index);
                    }
                    break;

                case SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID :
                    if ((p_attr->value.oid != SAI_NULL_OBJECT_ID)
                          && (sai_qos_scheduler_node_get
                                               (p_attr->value.oid) == NULL)) {
                        SAI_SCHED_GRP_LOG_ERR ("SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID : 0x%"PRIx64""
                             "is invalid.", p_attr->value.oid);
                        return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                       list_index);
                    }
                    break;

                case SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE :
                    if (p_attr->value.oid != SAI_NULL_OBJECT_ID) {
                        parent_object_type = sai_uoid_obj_type_get
                                                 (p_attr->value.oid);

                        if ((parent_object_type != SAI_OBJECT_TYPE_SCHEDULER_GROUP)
                              && (parent_object_type != SAI_OBJECT_TYPE_PORT)) {
                            SAI_SCHED_GRP_LOG_ERR ("SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE : 0x%"PRIx64""
                             "is invalid.", p_attr->value.oid);
                            return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                           list_index);
                        }

                        if ((parent_object_type == SAI_OBJECT_TYPE_SCHEDULER_GROUP)
                                    && (sai_qos_sched_group_node_get
                                                         (p_attr->value.oid) == NULL)) {
                            SAI_SCHED_GRP_LOG_ERR ("SAI_OBJECT_TYPE_SCHEDULER_GROUP  : 0x%"PRIx64""
                             "is invalid.", p_attr->value.oid);
                            return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                           list_index);
                        }

                        if ((parent_object_type == SAI_OBJECT_TYPE_PORT)
                                   && (sai_qos_port_node_get (p_attr->value.oid) == NULL)) {
                            SAI_SCHED_GRP_LOG_ERR ("SAI_OBJECT_TYPE_PORT  : 0x%"PRIx64""
                             "is invalid.", p_attr->value.oid);
                            return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                           list_index);
                        }
                    }
                    break;

                default:
                    break;
            }
        }
    } while (0);

    return sai_rc;
}

/* Attribute validation should happen before this function */
static sai_status_t sai_qos_sched_group_port_attribute_get (
                              uint_t attr_count, const sai_attribute_t *p_attr_list,
                              sai_object_id_t *port_id)
{
    const sai_attribute_t *p_attr = NULL;
    uint_t                 list_idx = 0;

    STD_ASSERT(p_attr_list != NULL);

    for (list_idx = 0, p_attr = p_attr_list;
         list_idx < attr_count; ++list_idx, ++p_attr) {

        if (p_attr->id == SAI_SCHEDULER_GROUP_ATTR_PORT_ID) {
            *port_id = p_attr->value.oid;
            return SAI_STATUS_SUCCESS;
        }
    }

    return SAI_STATUS_FAILURE;
}

/* Attribute validation should happen before this function */
static sai_status_t sai_qos_port_hierarchy_update_on_first_application_sched_group (
                                                            sai_object_id_t port_id)
{
    sai_status_t       sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_port_t *p_qos_port_node = NULL;

    /* Qos Init Not done, No Need to do any cleanup */
    if (! sai_qos_is_init_complete ())
        return sai_rc;

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    /* Delete existing hierarchy if first hqos sched group create from app */
    if (! p_qos_port_node->is_app_hqos_init) {

        sai_rc = sai_qos_port_hierarchy_deinit (port_id);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("SAI QOS port 0x%"PRIx64" hierarchy de-init "
                             "failed.", port_id);
            return sai_rc;
        }
        p_qos_port_node->is_app_hqos_init = true;
    }

    return sai_rc;
}

static sai_status_t sai_qos_port_application_sched_group_handle (uint_t attr_count,
                                                 const sai_attribute_t *attr_list)

{
    sai_status_t       sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t    port_id = SAI_NULL_OBJECT_ID;
    bool               is_app_hqos_init_started = false;

    sai_rc = sai_qos_sched_group_port_attribute_get (attr_count, attr_list,
                                                     &port_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Port is not part of the Input parameters for "
                               "Scheduler group creation.");

        return sai_rc;
    }

    sai_rc = sai_qos_port_hierarchy_init_from_application_status_get (port_id,
                                                            &is_app_hqos_init_started);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Port hierarchy init from application status get failed.");

        return sai_rc;
    }

    if (! is_app_hqos_init_started) {
        sai_rc = sai_qos_port_hierarchy_update_on_first_application_sched_group (port_id);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Default hierarchy remove failed on creation of "
                                   "first app scheduler group creation.");

            return sai_rc;
        }
    }

    return sai_rc;
}

/* Attribute validation happen before this function */
static bool sai_qos_sched_group_is_duplicate_set (dn_sai_qos_sched_group_t *p_sg_node,
                                                  const sai_attribute_t *p_attr)
{
    bool is_duplicate = false;

    STD_ASSERT(p_sg_node != NULL);
    STD_ASSERT(p_attr != NULL);

    SAI_SCHED_GRP_LOG_TRACE ("Verify duplicate set attributes value, ID: %d.",
                             p_attr->id);

    switch (p_attr->id)
    {
        case SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE:
            if (p_sg_node->parent_id == p_attr->value.oid)
                is_duplicate = true;
            break;

        case SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID:
            if (p_sg_node->scheduler_id == p_attr->value.oid)
                is_duplicate = true;
            break;

        default:
            SAI_SCHED_GRP_LOG_TRACE ("Attribute id: %d - read-only attribute.",
                                     p_attr->id);
            break;
    }

    return is_duplicate;
}


static sai_status_t sai_qos_sched_group_create_internal (
                          sai_object_id_t *sg_obj_id,
                          sai_object_id_t switch_id,
                          uint32_t attr_count,
                          const sai_attribute_t *attr_list)
{
    sai_status_t               sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_sched_group_t  *p_sg_node = NULL;
    sai_object_id_t            sg_oid = SAI_NULL_OBJECT_ID;
    bool                       is_sg_set_in_npu = false;
    bool                       is_sg_set_in_port_list = false;
    bool                       is_sg_add_to_parent = false;
    sai_attribute_t attr;

    SAI_SCHED_GRP_LOG_TRACE ("Scheduler group Creation, attr_count: %d.",
                             attr_count);

    STD_ASSERT (sg_obj_id != NULL);
    STD_ASSERT (attr_list != NULL);

    sai_rc = sai_qos_sched_group_attributes_validate (attr_count, attr_list,
                                                      SAI_OP_CREATE);
    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Input parameters validation failed for "
                               "Scheduler group creation.");
        return sai_rc;
    }

    sai_rc = sai_qos_port_application_sched_group_handle (attr_count, attr_list);
    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Failed to handle the port first Scheduler "
                               "group creation.");
        return sai_rc;
    }

    do {
        p_sg_node = sai_qos_sched_grp_node_alloc ();

        if (NULL == p_sg_node) {
            SAI_SCHED_GRP_LOG_ERR ("Scheduler group node memory allocation failed.");
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        sai_rc = sai_qos_sched_group_node_init (p_sg_node);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Scheduler group node init failed.");
            break;
        }

        sai_qos_sched_group_attr_set (p_sg_node, attr_count, attr_list,
                                      SAI_OP_CREATE);

        sai_rc = sai_sched_group_npu_api_get()->sched_group_create (p_sg_node,
                                                                    &sg_oid);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Scheduler group creation failed in NPU.");
            break;
        }

        SAI_SCHED_GRP_LOG_TRACE ("Scheduler group Created in NPU.");
        is_sg_set_in_npu = true;

        *sg_obj_id = sg_oid;
        p_sg_node->key.sched_group_id = sg_oid;

        /* Add the SG node to PORT's SG list */
        sai_rc = sai_qos_port_sched_group_list_update (p_sg_node, true);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Failed to add Scheduler group node to the "
                                   "scheduler group List in PORT node");
            break;
        }

        is_sg_set_in_port_list = true;

        sai_rc = sai_qos_sched_group_node_insert_to_tree (p_sg_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Scheduler group insertion to tree failed.");
            break;
        }

        if ((p_sg_node->hierarchy_level == 0)
                 && (sai_qos_is_hierarchy_qos_supported())) {
             break;
        } else {
            sai_rc = sai_sched_group_npu_api_get()->sched_group_attach_to_parent
                                                    (sg_oid,
                                                     p_sg_node->parent_id);

            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_SCHED_GRP_LOG_ERR ("Failed to attach child SG 0x%"PRIx64""
                                       "to parent SG 0x%"PRIx64" "
                                       "for port 0x%"PRIx64" level %d.",
                                       sg_oid, p_sg_node->parent_id,
                                       p_sg_node->port_id, p_sg_node->hierarchy_level);
                break;
            }
            is_sg_add_to_parent = true;

            sai_rc = sai_qos_sched_group_and_child_nodes_update (p_sg_node->parent_id,
                                                             sg_oid, true);
            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_SCHED_GRP_LOG_ERR ("Failed to update the child and parent hierarchy "
                                       "information.");
                break;
            }

            attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
            attr.value.oid = p_sg_node->scheduler_id;
            p_sg_node->scheduler_id = SAI_NULL_OBJECT_ID;
            sai_rc = sai_qos_sched_group_scheduler_set(p_sg_node, &attr);
            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_SCHED_GRP_LOG_ERR ("Error: Unable to set scheduler profile: 0x%"PRIx64""
                                       " for sg:0x%"PRIx64"",
                                       p_sg_node->scheduler_id,
                                       p_sg_node->key.sched_group_id);
            }
        }

    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_INFO ("Scheduler group Obj Id 0x%"PRIx64" created.",
                                 sg_oid);
    } else {
        SAI_SCHED_GRP_LOG_ERR ("Failed to create Scheduler group.");
        sai_qos_sched_group_free_resources (p_sg_node, is_sg_set_in_npu,
                                            is_sg_set_in_port_list,
                                            is_sg_add_to_parent);
        *sg_obj_id = SAI_NULL_OBJECT_ID;
    }

    return sai_rc;
}

static sai_status_t sai_qos_sched_group_remove_internal (sai_object_id_t sg_id)
{
    sai_status_t              sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_sched_group_t  *p_sg_node = NULL;
    sai_object_id_t           parent_id = SAI_NULL_OBJECT_ID;

    SAI_SCHED_GRP_LOG_TRACE ("Scheduler group remove ID 0x%"PRIx64".", sg_id);

    if (! sai_is_obj_id_scheduler_group (sg_id)) {
        SAI_SCHED_GRP_LOG_ERR ("0x%"PRIx64" is not a valid scheduler group obj id.",
                               sg_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    do {
        p_sg_node = sai_qos_sched_group_node_get (sg_id);

        if (NULL == p_sg_node) {
            SAI_SCHED_GRP_LOG_ERR ("Scheduler group 0x%"PRIx64" "
                                   "does not exist in tree.",
                                   sg_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }

        if (p_sg_node->hqos_info.child_count  > 0) {
            SAI_SCHED_GRP_LOG_ERR ("Scheduler group 0x%"PRIx64" configs can't be deleted, \
                                   child count :%d", sg_id, p_sg_node->hqos_info.child_count);
            sai_rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        parent_id = p_sg_node->parent_id;

        SAI_SCHED_GRP_LOG_TRACE ("Scheduler group remove. Port ID 0x%"PRIx64", "
                                 "Level %d SGID 0x%"PRIx64"", p_sg_node->port_id,
                                 p_sg_node->hierarchy_level, sg_id);

        sai_rc = sai_qos_sched_group_remove_configs (p_sg_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Scheduler group 0x%"PRIx64" configs can't be deleted, ", sg_id);
            break;
        }

        if (p_sg_node->hierarchy_level != 0)
        {
            sai_rc = sai_sched_group_npu_api_get()->sched_group_detach_from_parent(sg_id);
            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_SCHED_GRP_LOG_ERR ("Scheduler group 0x%"PRIx64" detach from parent "
                                       "0x%"PRIx64" failed in NPU.", sg_id,
                                       p_sg_node->parent_id);
                break;
            }

            /* Update parent HQos information */
            sai_rc = sai_qos_sched_group_and_child_nodes_update (parent_id,
                                                                 sg_id, false);
            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_SCHED_GRP_LOG_ERR ("Failed to update the child and parent hierarchy "
                                       "information.");
                break;
            }
        }

        sai_rc = sai_sched_group_npu_api_get()->sched_group_remove (p_sg_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Scheduler group 0x%"PRIx64" deletion "
                                   "failed in NPU.", sg_id);

            break;
        }

        /* Delete from the PORT's scheduler group list */
        sai_qos_port_sched_group_list_update (p_sg_node, false);

        sai_qos_sched_group_node_remove_from_tree (p_sg_node);

        sai_qos_sched_group_node_free (p_sg_node);

    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_INFO ("Scheduler group 0x%"PRIx64" removed.", sg_id);
    } else {
        SAI_SCHED_GRP_LOG_ERR ("Failed to remove Scheduler group 0x%"PRIx64".",
                               sg_id);
    }

    return sai_rc;
}

static sai_status_t sai_qos_sched_group_modify_parent (
                                        sai_object_id_t sg_id,
                                        dn_sai_qos_sched_group_t *p_sg_node,
                                        sai_object_id_t new_parent_id)
{
    sai_status_t                sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t             old_parent_id = SAI_NULL_OBJECT_ID;

    STD_ASSERT(p_sg_node != NULL);
    old_parent_id = p_sg_node->parent_id;

    SAI_SCHED_GRP_LOG_TRACE ("Scheduler group modify parent. Port ID 0x%"PRIx64", "
                             "Level %d SGID 0x%"PRIx64"", p_sg_node->port_id,
                             p_sg_node->hierarchy_level, sg_id);

    sai_rc = sai_sched_group_npu_api_get()->sched_group_modify_parent(sg_id, new_parent_id);
    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Scheduler group 0x%"PRIx64" modify parent from "
                               "0x%"PRIx64" to 0x%"PRIx64" failed in NPU.", sg_id,
                               old_parent_id, new_parent_id);
        return sai_rc;
    }

    /* Update parent HQos information */
    sai_rc = sai_qos_sched_group_and_child_nodes_update (old_parent_id,
                                                         sg_id, false);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Failed to update the child and parent hierarchy "
                               "information with old parent details.");
        return sai_rc;
    }

    sai_rc = sai_qos_sched_group_and_child_nodes_update (new_parent_id,
                                                         sg_id, true);
    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Failed to update the child and parent hierarchy "
                               "information with new parent details.");
    }
    return sai_rc;
}

static sai_status_t sai_qos_sched_group_attribute_set (sai_object_id_t sg_id,
                                                       const sai_attribute_t *p_attr)
{
    sai_status_t                sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_sched_group_t    *p_sg_node = NULL;
    uint_t                      attr_count = 1;

    STD_ASSERT (p_attr != NULL);

    SAI_SCHED_GRP_LOG_TRACE ("Setting Attribute ID %d on Scheduler group 0x%"PRIx64".",
                             p_attr->id, sg_id);

    if (! sai_is_obj_id_scheduler_group (sg_id)) {
        SAI_SCHED_GRP_LOG_ERR ("0x%"PRIx64" is not a valid Scheduler group obj id.",
                               sg_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_qos_lock ();

    do {
        p_sg_node = sai_qos_sched_group_node_get (sg_id);

        if (NULL == p_sg_node) {
            SAI_SCHED_GRP_LOG_ERR ("Scheduler group 0x%"PRIx64" "
                                   "does not exist in tree.", sg_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }

        sai_rc = sai_qos_sched_group_attributes_validate (attr_count, p_attr,
                                                          SAI_OP_SET);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Input parameters validation failed for "
                                   "Scheduler group attributes set.");

            break;
        }

        if (sai_qos_sched_group_is_duplicate_set (p_sg_node, p_attr)) {
            SAI_SCHED_GRP_LOG_TRACE ("Duplicate set value for Attribute ID %d.",
                                     p_attr->id);
            break;
        }

        switch (p_attr->id)
        {
            case SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE:
                sai_rc = sai_qos_sched_group_modify_parent(sg_id, p_sg_node,
                                                           p_attr->value.oid);
                if (sai_rc != SAI_STATUS_SUCCESS) {
                    SAI_SCHED_GRP_LOG_ERR ("Error: Unable to set new parent: 0x%"PRIx64""
                                           " for sg:0x%"PRIx64"",
                                           p_attr->value.oid,
                                           p_sg_node->key.sched_group_id);
                }

                break;

            case SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID:
                sai_rc = sai_qos_sched_group_scheduler_set(p_sg_node, p_attr);
                if (sai_rc != SAI_STATUS_SUCCESS) {
                    SAI_SCHED_GRP_LOG_ERR ("Error: Unable to set scheduler profile: 0x%"PRIx64""
                                           " for sg:0x%"PRIx64"",
                                           p_attr->value.oid,
                                           p_sg_node->key.sched_group_id);
                }
                break;

            default:
                break;
        }

        if (sai_rc == SAI_STATUS_SUCCESS)
            sai_qos_sched_group_attr_set (p_sg_node, attr_count, p_attr, SAI_OP_SET);

    } while (0);

    sai_qos_unlock ();

    return sai_rc;
}

static sai_status_t sai_qos_sched_group_attribute_get (sai_object_id_t sg_id,
                                                       uint32_t attr_count,
                                                       sai_attribute_t *p_attr_list)
{
    sai_status_t                sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_sched_group_t    *p_sg_node = NULL;

    STD_ASSERT (p_attr_list != NULL);

    SAI_SCHED_GRP_LOG_TRACE ("Getting Attributes for scheduler group 0x%"PRIx64", "
                             "attr_count %d.", sg_id, attr_count);

    if (! sai_is_obj_id_scheduler_group (sg_id)) {
        SAI_SCHED_GRP_LOG_ERR ("%"PRIx64" is not a valid Scheduler group obj id.",
                               sg_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_qos_lock ();

    do {
        p_sg_node = sai_qos_sched_group_node_get (sg_id);

        if (NULL == p_sg_node) {
            SAI_SCHED_GRP_LOG_ERR ("Scheduler group 0x%"PRIx64" "
                                   "does not exist in tree.", sg_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }


        sai_rc = sai_qos_sched_group_attributes_validate (attr_count, p_attr_list,
                                                          SAI_OP_GET);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Input parameters validation failed for "
                                   "Scheduler group attributes get.");

            break;
        }

        sai_rc = sai_sched_group_npu_api_get()->sched_group_attribute_get (p_sg_node,
                                                                           attr_count,
                                                                           p_attr_list);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Failed to get Scheduler group Attributes from "
                                   "NPU, Error: %d.", sai_rc);

            break;
        }
    } while (0);

    sai_qos_unlock ();

    return sai_rc;
}


uint_t sai_qos_dlft_sched_groups_per_level_get (sai_object_id_t port_id,
                                                uint_t level)
{
    if (level == sai_switch_leaf_hierarchy_level_get ()) {
        if (sai_qos_queue_is_seperate_ucast_and_mcast_supported (port_id))
            return sai_switch_max_uc_queues_per_port_get(port_id);
    }
    return SAI_QOS_DLFT_SCHED_GROUPS_PER_LEVEL;
}
sai_status_t sai_qos_indexed_sched_group_id_get (sai_object_id_t port_id,
                                                 uint_t level, uint_t child_idx,
                                                 sai_object_id_t *sg_grp_id)
{
    dn_sai_qos_sched_group_t *p_sg_node = NULL;
    dn_sai_qos_port_t        *p_qos_port_node = NULL;
    uint_t                    sg_count = 0;

    SAI_SCHED_GRP_LOG_TRACE ("Port 0x%"PRIx64" level %d scheduler group id list "
                             "get, Index of schduler group %d",
                             port_id, level, child_idx);


    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {
        SAI_SCHED_GRP_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                               port_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    p_sg_node = sai_qos_port_get_first_sched_group (p_qos_port_node, level);

    while(p_sg_node != NULL)
    {
        if(sg_count == child_idx){
            *sg_grp_id = p_sg_node->key.sched_group_id;
            return SAI_STATUS_SUCCESS;
        }
        sg_count ++;
        p_sg_node = sai_qos_port_get_next_sched_group (p_qos_port_node, p_sg_node);
    }

    SAI_SCHED_GRP_LOG_ERR ("Required scheduler group not exits in "
                           "port 0x%"PRIx64" and level %d, "
                           "childidx %d.",
                           port_id, level, child_idx);
    return SAI_STATUS_FAILURE;
}

sai_status_t sai_qos_sched_group_id_list_per_level_get (sai_object_id_t port_id,
                                                        uint_t level,
                                                        uint_t sg_id_list_count,
                                                        sai_object_id_t *p_sg_id_list)
{
    dn_sai_qos_sched_group_t *p_sg_node = NULL;
    dn_sai_qos_port_t        *p_qos_port_node = NULL;
    uint_t                    sg_count = 0;

    SAI_SCHED_GRP_LOG_TRACE ("Port 0x%"PRIx64" level %d scheduler group id list "
                             "get, Req number of scheduler groups %d",
                             port_id, level, sg_id_list_count);

    if (0 == sg_id_list_count)
        return SAI_STATUS_SUCCESS;

    STD_ASSERT (p_sg_id_list != NULL);

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {
        SAI_SCHED_GRP_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                         port_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for (p_sg_node =
             sai_qos_port_get_first_sched_group (p_qos_port_node, level);
             ((p_sg_node != NULL) && (sg_count < sg_id_list_count)); p_sg_node =
             sai_qos_port_get_next_sched_group (p_qos_port_node, p_sg_node)) {
        p_sg_id_list [sg_count] = p_sg_node->key.sched_group_id;
        sg_count++;
    }

    if (sg_id_list_count != sg_count) {
        SAI_SCHED_GRP_LOG_ERR ("Required scheduler group not exits in "
                               "port 0x%"PRIx64" and level %d, "
                               "req count %d sg present %d.",
                               port_id, level, sg_id_list_count, sg_count);
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_port_sched_group_create (sai_object_id_t port_id,
                                                 sai_object_id_t parent_id,
                                                 uint_t level,
                                                 uint_t max_childs,
                                                 sai_object_id_t *sg_id)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    uint_t           attr_count = 0;
    sai_attribute_t  attr_list[SAI_QOS_SCHED_GROUP_DFLT_ATTR_COUNT] = {{0}};

    SAI_SCHED_GRP_LOG_TRACE ("Port 0x%"PRIx64" Scheduler Group Init and create.",
                             port_id);

    attr_count = 0;

    attr_list[attr_count].id = SAI_SCHEDULER_GROUP_ATTR_PORT_ID;
    attr_list[attr_count].value.oid = port_id;
    attr_count ++;

    attr_list[attr_count].id = SAI_SCHEDULER_GROUP_ATTR_LEVEL;
    attr_list[attr_count].value.u32 = level;
    attr_count ++;

    attr_list[attr_count].id = SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS;
    attr_list[attr_count].value.u32 = max_childs;
    attr_count ++;

    attr_list[attr_count].id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
    attr_list[attr_count].value.oid = sai_qos_default_sched_id_get();
    attr_count ++;

    attr_list[attr_count].id = SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE;
    attr_list[attr_count].value.oid = parent_id;
    attr_count ++;

    sai_rc = sai_qos_sched_group_create_internal (sg_id, SAI_DEFAULT_SWITCH_ID,
                                         attr_count, &attr_list[0]);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Failed to create scheduler "
                               "groups for port 0x%"PRIx64" level %d.",
                               port_id, level);
        return sai_rc;
    }

    SAI_SCHED_GRP_LOG_TRACE ("Created Scheduler Group 0x%"PRIx64" for "
                             "port 0x%"PRIx64" level %d",
                             *sg_id, port_id, level);

    return sai_rc;
}

sai_status_t sai_qos_port_sched_group_remove(sai_object_id_t sg_id)
{
    return sai_qos_sched_group_remove_internal(sg_id);
}

sai_status_t sai_qos_port_sched_groups_deinit (sai_object_id_t port_id)
{
    sai_status_t              sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_sched_group_t *p_sg_node = NULL;
    dn_sai_qos_sched_group_t *p_next_sg_node = NULL;
    dn_sai_qos_port_t        *p_qos_port_node = NULL;
    uint_t                    level = 0;

    SAI_SCHED_GRP_LOG_TRACE ("Port 0x%"PRIx64" Scheduler groups De-Init.",
                             port_id);

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {

        SAI_SCHED_GRP_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                         port_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for (level = 0; level < sai_switch_max_hierarchy_levels_get (); level++)
    {
        for (p_sg_node =
             sai_qos_port_get_first_sched_group (p_qos_port_node, level);
             p_sg_node != NULL; p_sg_node = p_next_sg_node) {

            p_next_sg_node = sai_qos_port_get_next_sched_group (p_qos_port_node,
                                                                     p_sg_node);
            sai_rc = sai_qos_sched_group_remove_internal (p_sg_node->key.sched_group_id);

            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_SCHED_GRP_LOG_ERR ("Scheduler group remove failed "
                                       "for port 0x%"PRIx64", Scheduler group "
                                       "0x%"PRIx64".", port_id,
                                       p_sg_node->key.sched_group_id);
                return sai_rc;
            }
        }
    }

    SAI_SCHED_GRP_LOG_INFO ("Port 0x%"PRIx64" Scheduler groups De-Init success.",
                             port_id);
    return sai_rc;
}

static sai_status_t sai_qos_sched_group_create_api (
                          sai_object_id_t *sg_obj_id,
                          sai_object_id_t switch_id,
                          uint32_t attr_count,
                          const sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    if (sai_qos_is_fixed_hierarchy_qos () == true) {
        return SAI_STATUS_NOT_SUPPORTED;
    }

    sai_qos_lock();
    sai_rc = sai_qos_sched_group_create_internal(sg_obj_id, switch_id, attr_count, attr_list);
    sai_qos_unlock();
    return sai_rc;
}

static sai_status_t sai_qos_sched_group_remove_api (sai_object_id_t sg_id)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    if (sai_qos_is_fixed_hierarchy_qos () == true) {
        return SAI_STATUS_NOT_SUPPORTED;
    }

    sai_qos_lock();
    sai_rc = sai_qos_sched_group_remove_internal(sg_id);
    sai_qos_unlock();
    return sai_rc;
}

static sai_scheduler_group_api_t sai_qos_sched_group_method_table = {
    sai_qos_sched_group_create_api,
    sai_qos_sched_group_remove_api,
    sai_qos_sched_group_attribute_set,
    sai_qos_sched_group_attribute_get,
};

sai_scheduler_group_api_t *sai_qos_sched_group_api_query (void)
{
    return (&sai_qos_sched_group_method_table);
}

