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
 * * @file sai_acl_table_group_member.c
 * *
 * * @brief This file contains functions for SAI ACL Table Group member operations.
 * *
 * *
 * *************************************************************************/
#include "sai_acl_type_defs.h"
#include "sai_acl_npu_api.h"
#include "sai_common_acl.h"
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
#include "sai_udf_api_utils.h"

#include <stdlib.h>
#include <inttypes.h>

static dn_sai_id_gen_info_t acl_table_group_mem_obj_info;

static const dn_sai_attribute_entry_t dn_sai_acl_table_group_member_attr[] = {
    {SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID, true, true, false, true, true, true},
    {SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID, true, true, false, true, true, true},
    {SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY, true, true, false, true, true, true},
};

bool sai_is_acl_table_group_member_id_in_use(uint64_t obj_id)
{
    acl_node_pt acl_node = NULL;

    acl_node = sai_acl_get_acl_node();

    sai_object_id_t acl_table_group_mem_id =
        sai_uoid_create(SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER,obj_id);

    if(sai_acl_table_group_member_find(acl_node->sai_acl_table_group_member_tree,
                                acl_table_group_mem_id) != NULL) {
        return true;
    } else {
        return false;
    }
}

static sai_object_id_t sai_acl_table_group_member_id_create(void)
{
    if(SAI_STATUS_SUCCESS ==
       dn_sai_get_next_free_id(&acl_table_group_mem_obj_info)) {
        return (sai_uoid_create(SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER,
                                acl_table_group_mem_obj_info.cur_id));
    }
    return SAI_NULL_OBJECT_ID;
}

void sai_acl_table_group_member_init(void)
{
    acl_table_group_mem_obj_info.cur_id = 0;
    acl_table_group_mem_obj_info.is_wrappped = false;
    acl_table_group_mem_obj_info.mask = SAI_UOID_NPU_OBJ_ID_MASK;
    acl_table_group_mem_obj_info.is_id_in_use = sai_is_acl_table_group_member_id_in_use;
}

static inline void dn_sai_acl_table_group_member_attr_table_get (
                                                 const dn_sai_attribute_entry_t **p_attr_table,
                                                 uint_t *p_attr_count)
{
    *p_attr_table = &dn_sai_acl_table_group_member_attr[0];

    *p_attr_count = (sizeof(dn_sai_acl_table_group_member_attr)) /
        (sizeof(dn_sai_acl_table_group_member_attr[0]));
}
static sai_status_t sai_acl_table_group_member_attr_get(
                                          sai_acl_table_group_member_t *p_acl_table_group_mem_node,
                                          uint32_t attr_count,
                                          sai_attribute_t *p_attr_list)
{
    STD_ASSERT(p_acl_table_group_mem_node != NULL);
    STD_ASSERT(p_attr_list != NULL);
    uint_t attr_index = 0;
    sai_attribute_t *p_attr = NULL;

    for(attr_index = 0, p_attr = &p_attr_list[0]; attr_index < attr_count; ++attr_index, ++p_attr)
    {
        switch (p_attr->id)
        {
            case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID:
                p_attr->value.oid = p_acl_table_group_mem_node->acl_group_id;
                break;

            case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID:
                p_attr->value.oid = p_acl_table_group_mem_node->acl_table_id;
                break;

            case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY:
                p_attr->value.u32 = p_acl_table_group_mem_node->priority;
                break;

            default:
                SAI_ACL_LOG_ERR("Attribute id: %d is not a known attribute "
                                "for acltable group member",p_attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_table_group_member_attr_set(
                                      sai_acl_table_group_member_t *p_acl_table_group_mem_node,
                                      const sai_attribute_t *p_attr)
{
    sai_status_t  sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT (p_acl_table_group_mem_node != NULL);
    STD_ASSERT (p_attr != NULL);

    switch(p_attr->id) {
        case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID:
            p_acl_table_group_mem_node->acl_group_id = p_attr->value.oid;
            break;


        case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID:
            p_acl_table_group_mem_node->acl_table_id = p_attr->value.oid;
            break;

        case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY:
            p_acl_table_group_mem_node->priority = p_attr->value.u32;
            break;

        default:
            SAI_ACL_LOG_ERR ("Attribute id: %d is not a known attribute "
                              "for saiacl table group member.", p_attr->id);
            sai_rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            break;
    }

    return sai_rc;
}

static sai_status_t sai_acl_table_group_member_parse_update_attributes(
                                      sai_acl_table_group_member_t *p_acl_table_group_mem_node,
                                      uint32_t attr_count,
                                      const sai_attribute_t *attr_list,
                                                     dn_sai_operations_t op_type)
{
    sai_status_t               sai_rc = SAI_STATUS_SUCCESS;
    uint_t                     list_idx = 0;
    uint_t                     max_vendor_attr_count = 0;
    const dn_sai_attribute_entry_t   *p_vendor_attr = NULL;

    STD_ASSERT(p_acl_table_group_mem_node != NULL);

    dn_sai_acl_table_group_member_attr_table_get(&p_vendor_attr, &max_vendor_attr_count);

    STD_ASSERT(p_vendor_attr != NULL);
    STD_ASSERT(max_vendor_attr_count != 0);

    sai_rc = sai_attribute_validate(attr_count,attr_list, p_vendor_attr,
                                    op_type, max_vendor_attr_count);

    if(sai_rc != SAI_STATUS_SUCCESS){
        SAI_ACL_LOG_ERR("Attribute validation failed for %d operation",
                            op_type);
        return sai_rc;
    }

    if(op_type != SAI_OP_GET) {
        for (list_idx = 0; list_idx < attr_count; list_idx++)
        {
            sai_rc = sai_acl_table_group_member_attr_set(p_acl_table_group_mem_node,
                                                         &attr_list[list_idx]);
            if(sai_rc != SAI_STATUS_SUCCESS) {
                sai_rc = sai_get_indexed_ret_val(sai_rc, list_idx);
                return sai_rc;
            }
        }
    }

    return sai_rc;
}
sai_status_t sai_acl_table_group_table_id_update(sai_object_id_t sai_acl_group_id,
                                   sai_acl_table_group_member_t *p_acl_table_group_mem_node,
                                   bool is_add)
{
    acl_node_pt            acl_node = NULL;
    sai_acl_table_group_t  *p_acl_table_group_node = NULL;

    STD_ASSERT(p_acl_table_group_mem_node != NULL);

    acl_node = sai_acl_get_acl_node();

    p_acl_table_group_node = sai_acl_table_group_find(acl_node->sai_acl_table_group_tree,
                                                      sai_acl_group_id);

    if (p_acl_table_group_node == NULL) {
        SAI_ACL_LOG_ERR ("ACL table group not present for "
                         "Rule ID 0x%"PRIx64"", sai_acl_group_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if(is_add) {
        std_dll_insertatback(&p_acl_table_group_node->table_list_head,
                             &p_acl_table_group_mem_node->member_link);

    }
    else {
        std_dll_remove(&p_acl_table_group_node->table_list_head,
                       &p_acl_table_group_mem_node->member_link);
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_table_group_member_create(sai_object_id_t *acl_table_group_mem_id,
        sai_object_id_t switch_id,
        uint32_t attr_count,
        const sai_attribute_t *attr_list)
{
    sai_status_t           sai_rc = SAI_STATUS_SUCCESS;
    acl_node_pt            acl_node = NULL;
    sai_acl_table_t        *acl_table = NULL;
    sai_acl_table_group_member_t  *p_acl_table_group_mem_node = NULL;

    STD_ASSERT(acl_table_group_mem_id != NULL);

    SAI_ACL_LOG_TRACE("Acl table group member create");

    sai_acl_lock();

    do
    {
        acl_node = sai_acl_get_acl_node();

        if(acl_node == NULL) {
            SAI_ACL_LOG_ERR ("Acl node get failed");
            sai_rc = SAI_STATUS_FAILURE;
        }

        if((p_acl_table_group_mem_node = sai_acl_table_group_member_node_alloc()) == NULL){
            SAI_ACL_LOG_ERR("Failed to allocate memory for acltable group node.");
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        sai_rc = sai_acl_table_group_member_parse_update_attributes(p_acl_table_group_mem_node,
                                                                    attr_count,
                                                                    attr_list, SAI_OP_CREATE);
        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_ACL_LOG_ERR("Failed to parse attributes for table member");
            break;
        }

        p_acl_table_group_mem_node->acl_table_group_member_id =
            sai_acl_table_group_member_id_create();

        p_acl_table_group_mem_node->ref_count = 0;

        if( p_acl_table_group_mem_node->acl_table_group_member_id == SAI_NULL_OBJECT_ID)
        {
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }
        sai_rc = sai_acl_table_group_member_insert(acl_node->sai_acl_table_group_member_tree,
                                                   p_acl_table_group_mem_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("Failed to insert ACL table group "
                             "Id 0x%"PRIx64"  rc %d",
                             p_acl_table_group_mem_node->acl_table_group_member_id, sai_rc);
            break;
        }

        /**
         * The priority given in the member will be used for creating the table in
         * the NPu. So copying the priority field to the acl_table node.
         * */
        acl_table = sai_acl_table_find(acl_node->sai_acl_table_tree,
                                       p_acl_table_group_mem_node->acl_table_id);
        if (acl_table == NULL) {
            SAI_ACL_LOG_ERR ("Table get failed: "
                             "ACL Table 0x%"PRIx64" not found",
                             p_acl_table_group_mem_node->acl_table_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        acl_table->acl_table_priority = p_acl_table_group_mem_node->priority;

        sai_rc = sai_acl_table_group_table_id_update(p_acl_table_group_mem_node->acl_group_id,
                                            p_acl_table_group_mem_node, true);
    }while(0);

    if(sai_rc == SAI_STATUS_SUCCESS){
        *acl_table_group_mem_id =  p_acl_table_group_mem_node->acl_table_group_member_id;


        SAI_ACL_LOG_TRACE("Acl table group created successfully");
    }
    else{
        SAI_ACL_LOG_ERR("Acl table group create failed");
        sai_acl_table_group_member_free(p_acl_table_group_mem_node);
    }

    sai_acl_unlock();
    return sai_rc;
}

sai_status_t sai_acl_table_group_member_delete(sai_object_id_t acl_table_group_mem_id)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    acl_node_pt  acl_node = NULL;
    sai_acl_table_group_member_t *p_acl_table_group_mem_node = NULL;

    sai_acl_lock();
    do {
        acl_node = sai_acl_get_acl_node();

        if(acl_node == NULL) {
            SAI_ACL_LOG_ERR ("Acl node get failed");
            sai_rc = SAI_STATUS_FAILURE;
        }

        p_acl_table_group_mem_node = sai_acl_table_group_member_find(
                                                        acl_node->sai_acl_table_group_member_tree,
                                                        acl_table_group_mem_id);

        if (p_acl_table_group_mem_node == NULL) {
            SAI_ACL_LOG_ERR ("ACL table group not present for "
                             "Rule ID 0x%"PRIx64"", acl_table_group_mem_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        p_acl_table_group_mem_node = sai_acl_table_group_member_remove(acl_node->sai_acl_table_group_member_tree,
                                            p_acl_table_group_mem_node);

        if (p_acl_table_group_mem_node == NULL) {
            /* Some internal error in RB tree, log an error */
            SAI_ACL_LOG_ERR ("Failure removing ACL table group Id 0x%"PRIx64" "
                             "from Database", acl_table_group_mem_id);

            sai_rc = SAI_STATUS_FAILURE;
            break;
        }
        sai_rc = sai_acl_table_group_table_id_update(p_acl_table_group_mem_node->acl_group_id,
                                            p_acl_table_group_mem_node, false);
    } while(0);


    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Failed to remove acl table group");
    }
    else {
        SAI_ACL_LOG_TRACE ("Removed acl table group 0x%"PRIx64"", acl_table_group_mem_id);
        sai_acl_table_group_member_free(p_acl_table_group_mem_node);
    }

    sai_acl_unlock();
    return sai_rc;
}

sai_status_t sai_acl_table_group_member_attribute_set(sai_object_id_t acl_table_group_mem_id,
                                     const sai_attribute_t *p_attr)
{
    acl_node_pt  acl_node = NULL;
    uint_t       attr_count = 1;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_acl_table_group_member_t *p_acl_table_group_mem_node_exist = NULL;
    sai_acl_table_group_member_t acl_table_group_mem_new;

    sai_acl_lock();

    do {
        acl_node = sai_acl_get_acl_node();

        if(acl_node == NULL) {
            SAI_ACL_LOG_ERR ("Acl node get failed");
            sai_rc = SAI_STATUS_FAILURE;
        }

        p_acl_table_group_mem_node_exist = sai_acl_table_group_member_find(
                                                    acl_node->sai_acl_table_group_member_tree,
                                                    acl_table_group_mem_id);

        if (p_acl_table_group_mem_node_exist == NULL) {
            SAI_ACL_LOG_ERR ("ACL table group not present for "
                             "Rule ID 0x%"PRIx64"", acl_table_group_mem_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        memset(&acl_table_group_mem_new, 0, sizeof(sai_acl_table_group_member_t));

        acl_table_group_mem_new.acl_table_group_member_id =
            p_acl_table_group_mem_node_exist->acl_table_group_member_id;

        sai_rc = sai_acl_table_group_member_parse_update_attributes(&acl_table_group_mem_new,
                                                             attr_count, p_attr,
                                                             SAI_OP_SET);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR("Attribute validation failed ");
            break;
        }

        sai_rc = sai_acl_table_group_member_attr_set(p_acl_table_group_mem_node_exist,
                                                     p_attr);
    } while(0);

    sai_acl_unlock();

    if(sai_rc == SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_TRACE("Acl group attr set success for attrid %d on 0x%"PRIx64"",
                          p_attr->id, p_acl_table_group_mem_node_exist->acl_table_group_member_id);
    }

    return sai_rc;
}

sai_status_t sai_acl_table_group_member_attribute_get(sai_object_id_t acl_table_group_mem_id,
                                               uint32_t attr_count,
                                               sai_attribute_t *attr_list)
{
    STD_ASSERT(attr_list != NULL);

    acl_node_pt  acl_node = NULL;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_acl_table_group_member_t *p_acl_table_group_mem_node_exist = NULL;


    sai_acl_lock();
    do {
        acl_node = sai_acl_get_acl_node();

        if(acl_node == NULL) {
            SAI_ACL_LOG_ERR ("Acl node get failed");
            sai_rc = SAI_STATUS_FAILURE;
        }

        p_acl_table_group_mem_node_exist = sai_acl_table_group_member_find(
                                                          acl_node->sai_acl_table_group_member_tree,
                                                          acl_table_group_mem_id);

        if (p_acl_table_group_mem_node_exist == NULL) {
            SAI_ACL_LOG_ERR ("ACL table group not present for "
                             "Rule ID 0x%"PRIx64"", acl_table_group_mem_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_rc = sai_acl_table_group_member_parse_update_attributes(
                                                           p_acl_table_group_mem_node_exist,
                                                           attr_count, attr_list,
                                                           SAI_OP_GET);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR("Attribute validation failed ");
            break;
        }

        sai_rc = sai_acl_table_group_member_attr_get(p_acl_table_group_mem_node_exist,
                                              attr_count, attr_list);


    } while(0);

    sai_acl_unlock();

    if(sai_rc == SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_TRACE("Acl group attr get success for 0x%"PRIx64"",
                          p_acl_table_group_mem_node_exist->acl_table_group_member_id);
    }

    return sai_rc;
}
