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
 * @file sai_vm_sched_group.c
 *
 * @brief This file contains the scheduler group initialization and
 * function implementation for managing the SAI scheduler group object in
 * in VM environment.
 */

#include "sai_common_utils.h"
#include "sai_npu_qos.h"
#include "sai_vm_qos.h"

#include "saischedulergroup.h"
#include "saistatus.h"
#include "saitypes.h"
#include <inttypes.h>
#include "std_assert.h"
#include "sai_qos_util.h"
#include "sai_qos_common.h"


/**
 * Vendor attribute array for scheduler group containing the attribute
 * values and properties for create,set and get functionality.
 */
static const dn_sai_attribute_entry_t sai_qos_sched_group_attr[] =  {
    /*            ID                         MC     VC     VS    VG    IMP    SUP */
    { SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT, false, false, false, true, true, true },
    { SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST,  false, false, false, true, true, true },
    { SAI_SCHEDULER_GROUP_ATTR_PORT_ID,     true,  true,  false, true, true, true },
    { SAI_SCHEDULER_GROUP_ATTR_LEVEL,       true,  true,  false, true, true, true},
    { SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS,  true,  true,  false, true, true, true},
    { SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID, false,  false, true, true, true, true},
};

/**
 * Function returning the attribute array for scheduler groups and
 * count of the total number of attributes.
 */
static void sai_vm_sched_group_attr_table_get (const dn_sai_attribute_entry_t **p_vendor,
                                         uint_t *p_max_attr_count)
{
    *p_vendor = &sai_qos_sched_group_attr[0];

    *p_max_attr_count = sizeof(sai_qos_sched_group_attr)/sizeof(dn_sai_attribute_entry_t);

    return;
}

static sai_status_t sai_vm_root_sched_group_init (sai_object_id_t port_id)
{
     /* TODO */
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_sched_group_create (dn_sai_qos_sched_group_t *p_sg_node,
                                   sai_object_id_t *p_sg_oid)
{
    uint_t free_id = 0;

    STD_ASSERT (p_sg_node != NULL);
    STD_ASSERT (p_sg_oid != NULL);

    SAI_SCHED_LOG_TRACE("NPU Scheduler Group create");

    free_id = std_find_first_bit (sai_vm_qos_scheduler_group_bitmap_get (),
                                   SAI_VM_QOS_MAX_SCHEDULER_GROUPS, 0);

    if (free_id > SAI_VM_QOS_MAX_SCHEDULER_GROUPS) {
        SAI_SCHED_LOG_TRACE("Scheduler group max limit exceeded.");
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    STD_BIT_ARRAY_CLR (sai_vm_qos_scheduler_group_bitmap_get(), free_id);

    *p_sg_oid = sai_uoid_create (SAI_OBJECT_TYPE_SCHEDULER_GROUP,
                                    (sai_npu_object_id_t) free_id);

    SAI_SCHED_LOG_TRACE ("Scheduler creation successful in NPU. "
                         "Scheduler oid 0x%"PRIx64".",*p_sg_oid);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_sched_group_remove (dn_sai_qos_sched_group_t *p_sg_node)
{
    STD_ASSERT (p_sg_node != NULL);

    uint_t  sg_id = sai_uoid_npu_obj_id_get(p_sg_node->key.sched_group_id);

    STD_BIT_ARRAY_SET (sai_vm_qos_scheduler_group_bitmap_get(), sg_id);

    SAI_SCHED_LOG_TRACE ("Scheduler group delete successful in NPU. "
                         "Scheduler group oid 0x%"PRIx64".",
                         p_sg_node->key.sched_group_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_sched_group_attribute_set (dn_sai_qos_sched_group_t *p_sg_node,
                                                       uint_t attr_count,
                                                       const sai_attribute_t *p_attr_list)
{
    /* TODO */
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_sched_group_attribute_get (dn_sai_qos_sched_group_t *p_sg_node,
                                              uint_t attr_count,
                                              sai_attribute_t *p_attr_list)
{
    sai_status_t            sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t         *p_attr = NULL;
    size_t                  list_index = 0;

    STD_ASSERT (p_sg_node != NULL);
    STD_ASSERT (p_attr_list != NULL);

    SAI_SCHED_GRP_LOG_TRACE ("NPU Scheduler group attribute get, attr_count: "
                             "%d", attr_count);

    for (list_index = 0, p_attr = p_attr_list;
         list_index < attr_count; ++list_index, ++p_attr) {

        SAI_SCHED_GRP_LOG_TRACE("NPU Scheduler group attribute set, attr_id %d",
                                p_attr->id);

        switch (p_attr->id)
        {
            case SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT:
                p_attr->value.u32 = p_sg_node->hqos_info.child_count;
                break;

            case SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST:
                sai_rc = sai_qos_sched_group_child_id_list_get (p_sg_node,
                                                     &p_attr->value.objlist);
                break;

            case SAI_SCHEDULER_GROUP_ATTR_PORT_ID:
                p_attr->value.oid = p_sg_node->port_id;
                break;

            case SAI_SCHEDULER_GROUP_ATTR_LEVEL:
                p_attr->value.u32 = p_sg_node->hierarchy_level;
                break;

            case SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS:
                p_attr->value.u32 = p_sg_node->max_childs;

            case SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID:
                p_attr->value.oid = p_sg_node->scheduler_id;
                break;

            default:
                SAI_SCHED_GRP_LOG_ERR ("Unknown attribute id: %d.", p_attr->id);
                sai_rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }

        if (sai_rc != SAI_STATUS_SUCCESS) {

            SAI_SCHED_GRP_LOG_ERR ("Failure in get scheduler group attr list at "
                                   "index: %d.", list_index);

            return sai_rc;
        }
    }

    return sai_rc;
}


static sai_status_t sai_vm_add_child_object_to_group (sai_object_id_t sg_id,
                                                       uint_t child_count,
                                                       const sai_object_id_t *p_child_objects,
                                                       uint_t *added_child_count)
{
    sai_status_t       sai_rc = SAI_STATUS_SUCCESS;
    size_t             child_idx = 0;
    sai_object_id_t    child_id = SAI_NULL_OBJECT_ID;
    uint_t             child_npu_index = 0;
    dn_sai_qos_sched_group_t  *p_sg_node = NULL;

    STD_ASSERT (p_child_objects != NULL);

    SAI_SCHED_GRP_LOG_TRACE ("SAI Add childs to SG 0x%"PRIx64", child_count %d.",
                             sg_id, child_count);

    p_sg_node = sai_qos_sched_group_node_get (sg_id);

    if(p_sg_node == NULL){
        SAI_SCHED_GRP_LOG_ERR ("Scheduler group 0x%"PRIx64" "
                               "does not exist in tree.",
                               sg_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if(child_count > p_sg_node->max_childs){
        SAI_SCHED_GRP_LOG_ERR ("Child count %d exceeds max_child_count %d"
                               "of parent sgid 0x%"PRIx64"", child_count,
                               p_sg_node->max_childs, sg_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    for (child_idx = 0; child_idx < child_count; child_idx++)
    {
        child_id = p_child_objects[child_idx];

        sai_rc = sai_qos_sched_group_first_free_child_index_get (sg_id,
                                                     &child_npu_index);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Failed to get free child index "
                                   "for SG oid 0x%"PRIx64"", sg_id);

            break;
        }

        sai_rc = sai_qos_child_index_update (child_id, child_npu_index);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Failed to update child index in node.");

            break;
        }
    }

    *added_child_count = child_idx;

    return sai_rc;
}


static sai_status_t sai_vm_remove_child_object_from_group (sai_object_id_t sg_id,
                                                      uint_t child_count,
                                                      const sai_object_id_t *p_child_objects,
                                                      uint_t *removed_child_count)
{
    sai_status_t       sai_rc = SAI_STATUS_SUCCESS;
    size_t             child_idx = 0;
    sai_object_id_t    child_id = SAI_NULL_OBJECT_ID;
    uint_t             child_npu_index = 0;

    STD_ASSERT (p_child_objects != NULL);

    SAI_SCHED_GRP_LOG_TRACE ("SAI Remove childs from SG 0x%"PRIx64", "
                             "child_count %d.", sg_id, child_count);

    for (child_idx = 0; child_idx < child_count; child_idx++)
    {
        child_id = p_child_objects[child_idx];

        sai_rc = sai_qos_child_index_get (child_id, &child_npu_index);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Failed to get child index "
                                   "of SG oid 0x%"PRIx64" "
                                   "and child oid 0x%"PRIx64"", sg_id,
                                   child_id);
            break;
        }

        sai_rc = sai_qos_child_index_update (child_id, SAI_QOS_CHILD_INDEX_INVALID);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Failed to update child index in node.");

            break;
        }

        sai_rc = sai_qos_sched_group_child_index_free (sg_id, child_npu_index);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Failed to update child index in node.");

            break;
        }
    }

    *removed_child_count = child_idx;
    return sai_rc;
}

static sai_npu_sched_group_api_t sai_vm_sched_group_api_table = {
    sai_vm_root_sched_group_init,
    sai_vm_sched_group_create,
    sai_vm_sched_group_remove,
    sai_vm_sched_group_attribute_set,
    sai_vm_sched_group_attribute_get,
    sai_vm_add_child_object_to_group,
    sai_vm_remove_child_object_from_group,
    sai_vm_sched_group_attr_table_get,
};

sai_npu_sched_group_api_t* sai_vm_sched_group_api_query (void)
{
    return &sai_vm_sched_group_api_table;
}

