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
 * * @file sai_acl_range.c
 * *
 * * @brief This file contains functions for SAI ACL range operations.
 * *
 * *
 * *************************************************************************/
#include "sai_acl_type_defs.h"
#include "sai_acl_npu_api.h"
#include "sai_common_acl.h"
#include "sai_common_utils.h"
#include "sai_acl_utils.h"

#include "saitypes.h"
#include "saiacl.h"
#include "saistatus.h"

#include "std_type_defs.h"
#include "std_rbtree.h"
#include "std_llist.h"
#include "std_assert.h"
#include "sai_oid_utils.h"
#include "sai_common_infra.h"

#include <stdlib.h>
#include <inttypes.h>

static dn_sai_id_gen_info_t acl_range_info;

bool sai_is_acl_range_id_in_use(uint64_t obj_id)
{
    acl_node_pt acl_node = NULL;

    acl_node = sai_acl_get_acl_node();

    sai_object_id_t acl_range_id = sai_uoid_create(SAI_OBJECT_TYPE_ACL_RANGE,obj_id);

    if(sai_acl_range_find(acl_node->sai_acl_range_tree,
                                acl_range_id) != NULL) {
        return true;
    } else {
        return false;
    }
}

static sai_object_id_t sai_acl_range_id_create(void)
{
    if(SAI_STATUS_SUCCESS ==
       dn_sai_get_next_free_id(&acl_range_info)) {
        return (sai_uoid_create(SAI_OBJECT_TYPE_ACL_RANGE,
                                acl_range_info.cur_id));
    }
    return SAI_NULL_OBJECT_ID;
}

void sai_acl_range_init(void)
{
    acl_range_info.cur_id = 0;
    acl_range_info.is_wrappped = false;
    acl_range_info.mask = SAI_UOID_NPU_OBJ_ID_MASK;
    acl_range_info.is_id_in_use = sai_is_acl_range_id_in_use;
}

static sai_status_t sai_acl_range_attr_set(sai_acl_range_t *p_range_node,
                                      const sai_attribute_t *p_attr)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT (p_range_node != NULL);
    STD_ASSERT (p_attr != NULL);

    switch(p_attr->id) {

        case SAI_ACL_RANGE_ATTR_TYPE:
            p_range_node->range_type = p_attr->value.s32;
            break;

        case SAI_ACL_RANGE_ATTR_LIMIT:
            p_range_node->range_limit.min = p_attr->value.s32range.min;
            p_range_node->range_limit.max = p_attr->value.s32range.max;
            break;

        default:
            SAI_ACL_LOG_ERR ("Attribute id: %d is not a known attribute "
                              "for saiacl range.", p_attr->id);
            sai_rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            break;
    }

    return sai_rc;
}

static sai_status_t sai_acl_range_parse_update_attributes(sai_acl_range_t *p_range_node,
                                                     uint32_t attr_count,
                                                     const sai_attribute_t *attr_list,
                                                     dn_sai_operations_t op_type)
{
    sai_status_t               sai_rc = SAI_STATUS_SUCCESS;
    uint_t                     list_idx = 0;
    uint_t                     max_vendor_attr_count = 0;
    const dn_sai_attribute_entry_t   *p_vendor_attr = NULL;

    STD_ASSERT(p_range_node != NULL);

    sai_acl_npu_api_get()->attribute_table_get(&p_vendor_attr, &max_vendor_attr_count);

    STD_ASSERT(p_vendor_attr != NULL);
    STD_ASSERT(max_vendor_attr_count != 0);

    sai_rc = sai_attribute_validate(attr_count,attr_list, p_vendor_attr,
                                    op_type, max_vendor_attr_count);

    if(sai_rc != SAI_STATUS_SUCCESS){
        SAI_ACL_LOG_ERR("Attribute validation failed for %d operation",
                            op_type);
        return sai_rc;
    }

    if(op_type != SAI_OP_GET){
        for (list_idx = 0; list_idx < attr_count; list_idx++)
        {
            sai_rc = sai_acl_range_attr_set(p_range_node, &attr_list[list_idx]);
            if(sai_rc != SAI_STATUS_SUCCESS) {
                sai_rc = sai_get_indexed_ret_val(sai_rc, list_idx);
                return sai_rc;
            }
        }
    }

    return sai_rc;
}

sai_status_t sai_create_acl_range(sai_object_id_t *acl_range_id,
                                  sai_object_id_t switch_id,
                                  uint32_t attr_count,
                                  const sai_attribute_t *attr_list)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    acl_node_pt         acl_node = NULL;
    sai_acl_range_t     *p_range_node = NULL;

    STD_ASSERT(acl_range_id != NULL);

    sai_acl_lock();

    do
    {
        acl_node = sai_acl_get_acl_node();

        if(acl_node == NULL) {
            SAI_ACL_LOG_ERR ("Acl node get failed");
            sai_rc = SAI_STATUS_FAILURE;
        }

        if((p_range_node = sai_acl_range_node_alloc()) == NULL){
            SAI_ACL_LOG_ERR("Failed to allocate memory for acl range node.");
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        sai_rc = sai_acl_range_parse_update_attributes(p_range_node, attr_count,
                                                  attr_list, SAI_OP_CREATE);
        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_ACL_LOG_ERR("Failed to parse attributes for acl range");
            break;
        }

        p_range_node->acl_range_id = sai_acl_range_id_create();

        if(p_range_node->acl_range_id == SAI_NULL_OBJECT_ID)
        {
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

        sai_rc = sai_acl_npu_api_get()->create_acl_range(p_range_node);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_ACL_LOG_ERR("Npu range create failed");
            break;
        }

        SAI_ACL_LOG_TRACE("Range object id is 0x%"PRIx64"",p_range_node->acl_range_id);

        if (sai_acl_range_insert(acl_node->sai_acl_range_tree, p_range_node) != STD_ERR_OK){
            SAI_ACL_LOG_ERR("range id insertion failed in RB tree");
            sai_acl_npu_api_get()->delete_acl_range(p_range_node);
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

    }while(0);

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_ACL_LOG_TRACE("Range created successfully");
        *acl_range_id = p_range_node->acl_range_id;
    }
    else{
        SAI_ACL_LOG_ERR("Range create failed");
        sai_acl_range_free(p_range_node);
    }

    sai_acl_unlock();
    return sai_rc;
}

sai_status_t sai_delete_acl_range(sai_object_id_t acl_range_id)
{
    sai_acl_range_t  *p_range_node = NULL;
    acl_node_pt       acl_node = NULL;
    sai_status_t      sai_rc = SAI_STATUS_SUCCESS;

    if(!sai_is_obj_id_acl_range(acl_range_id)) {
        SAI_ACL_LOG_ERR("Passed object is not range object");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    SAI_ACL_LOG_TRACE("Removing range id 0x%"PRIx64"",acl_range_id);
    sai_acl_lock();
    do
    {
        acl_node = sai_acl_get_acl_node();

        if(acl_node == NULL) {
            SAI_ACL_LOG_ERR ("Acl node get failed");
            sai_rc = SAI_STATUS_FAILURE;
        }

        p_range_node = sai_acl_range_find(acl_node->sai_acl_range_tree, acl_range_id);

        if(!p_range_node){
            SAI_ACL_LOG_ERR("Range id 0x%"PRIx64" not present in tree",
                             acl_range_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if(p_range_node->ref_count != 0)
        {
            SAI_ACL_LOG_ERR("Range id is in use");
            sai_rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        sai_rc = sai_acl_npu_api_get()->delete_acl_range(p_range_node);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_ACL_LOG_ERR("Npu range remove failed for rangeid 0x%"PRIx64"",
                             acl_range_id);
            break;
        }

        p_range_node = sai_acl_range_remove(acl_node->sai_acl_range_tree, p_range_node);

        if(p_range_node == NULL)
        {
            SAI_ACL_LOG_ERR("Range remove failed for rangeid 0x%"PRIx64"",
                            acl_range_id);
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

    }while(0);

    sai_acl_unlock();

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Failed to remove acl range");
    }
    else {
        SAI_ACL_LOG_TRACE ("Removed acl range 0x%"PRIx64"", acl_range_id);
        sai_acl_range_free(p_range_node);
    }

    return sai_rc;
}

sai_status_t sai_set_acl_range(sai_object_id_t acl_range_id,
                               const sai_attribute_t *p_attr)
{
    sai_acl_range_t     range_new_node;
    sai_acl_range_t     *p_range_exist_node = NULL;
    acl_node_pt         acl_node = NULL;
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    uint_t              attr_count = 1;

    STD_ASSERT (p_attr != NULL);

    SAI_ACL_LOG_TRACE("Setting attribute Id: %d on range Id 0x%"PRIx64"",
                          p_attr->id, acl_range_id);

    if(!sai_is_obj_id_acl_range(acl_range_id)) {
        SAI_ACL_LOG_ERR("Passed object is not acl range  object");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }
    sai_acl_lock();

    do
    {
        acl_node = sai_acl_get_acl_node();

        if(acl_node == NULL) {
            SAI_ACL_LOG_ERR ("Acl node get failed");
            sai_rc = SAI_STATUS_FAILURE;
        }

        p_range_exist_node = sai_acl_range_find(acl_node->sai_acl_range_tree, acl_range_id);

        if(p_range_exist_node == NULL){
            SAI_ACL_LOG_ERR("Range id 0x%"PRIx64" not present in tree",
                             acl_range_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }


        memset(&range_new_node, 0, sizeof(sai_acl_range_t));

        range_new_node.acl_range_id = acl_range_id;

        memcpy(&range_new_node, p_range_exist_node, sizeof(sai_acl_range_t));

        sai_rc = sai_acl_range_parse_update_attributes(&range_new_node,
                                                     attr_count,
                                                     p_attr, SAI_OP_SET);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_ACL_LOG_ERR("Validation failed for attribute Id %d",
                                p_attr->id);
            break;
        }

        sai_rc = sai_acl_npu_api_get()->set_acl_range(&range_new_node, attr_count, p_attr);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_ACL_LOG_ERR("NPU set failed for attribute Id %d"
                                "on rangeid 0x%"PRIx64"",p_attr->id, acl_range_id);
            break;
        }
        SAI_ACL_LOG_TRACE("NPU attribute set success for id %d"
                              "for range 0x%"PRIx64"", p_attr->id, acl_range_id);

        sai_rc = sai_acl_range_attr_set(p_range_exist_node, p_attr);

    }while(0);

    sai_acl_unlock();

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_ACL_LOG_INFO("Set attribute success for range 0x%"PRIx64" for attr %d",
                              p_range_exist_node->acl_range_id, p_attr->id);
    }

    return sai_rc;
}

sai_status_t sai_get_acl_range(sai_object_id_t acl_range_id,
                               uint32_t attr_count,
                               sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    sai_acl_range_t *p_range_node = NULL;
    acl_node_pt         acl_node = NULL;

    STD_ASSERT(attr_list != NULL);

    SAI_ACL_LOG_TRACE("Get range attributes for id 0x%"PRIx64"", acl_range_id);

    if ((attr_count == 0) || (attr_count > SAI_ACL_RANGE_MAX_ATTR_COUNT)){
        SAI_ACL_LOG_ERR("Invalid number of attributes for acl range");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(!sai_is_obj_id_acl_range(acl_range_id)) {
        SAI_ACL_LOG_ERR("Passed object is not range object");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    SAI_ACL_LOG_TRACE("Removing range id 0x%"PRIx64"",acl_range_id);
    sai_acl_lock();
    do
    {
        acl_node = sai_acl_get_acl_node();

        if(acl_node == NULL) {
            SAI_ACL_LOG_ERR ("Acl node get failed");
            sai_rc = SAI_STATUS_FAILURE;
        }

        p_range_node = sai_acl_range_find(acl_node->sai_acl_range_tree, acl_range_id);

        if(!p_range_node){
            SAI_ACL_LOG_ERR("Range id 0x%"PRIx64" not present in tree",
                             acl_range_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_rc = sai_acl_range_parse_update_attributes(p_range_node, attr_count,
                                                  attr_list, SAI_OP_GET);
        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_ACL_LOG_ERR("Failed to parse attributes for acl range");
            break;
        }

        sai_rc = sai_acl_npu_api_get()->get_acl_range(p_range_node,
                                                      attr_count, attr_list);

        if (sai_rc != SAI_STATUS_SUCCESS){
            SAI_ACL_LOG_ERR("Npu get failed for rangeid 0x%"PRIx64"",
                                acl_range_id);
            break;
        }
    } while (0);

    sai_acl_unlock();

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_ACL_LOG_TRACE("Get attribute success for range id 0x%"PRIx64"",
                           acl_range_id);
    }
    else
    {
        SAI_ACL_LOG_ERR("Get attribute failed for range id 0x%"PRIx64"",
                           acl_range_id);
    }
    return sai_rc;
}

