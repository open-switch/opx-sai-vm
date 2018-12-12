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
 * * @file sai_vm_acl_slice.c
 * *
 * * @brief This file contains functions for SAI ACL VM slice
 * *        operations.
 * *
 *************************************************************************/

#include "sai_common_acl.h"
#include "sai_acl_utils.h"
#include "sai_switch_utils.h"
#include "std_assert.h"
#include "saistatus.h"

#include <stdlib.h>

static sai_status_t sai_acl_slice_attr_get(rbtree_handle acl_table_tree,
                                            sai_acl_slice_t *acl_slice_node,
                                            uint32_t attr_count,
                                            sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    STD_ASSERT(acl_slice_node != NULL);
    STD_ASSERT(attr_list != NULL);

    uint_t attr_index = 0;
    sai_attribute_t *p_attr = NULL;

    for(attr_index = 0, p_attr = &attr_list[0]; attr_index < attr_count; ++attr_index, ++p_attr)
    {
        switch (p_attr->id)
        {
            case SAI_ACL_SLICE_ATTR_EXTENSIONS_SLICE_ID:
                p_attr->value.u32 = acl_slice_node->slice_id;
                break;
            case SAI_ACL_SLICE_ATTR_EXTENSIONS_PIPE_ID:
                p_attr->value.u32 = acl_slice_node->pipe_id;
                break;
            case SAI_ACL_SLICE_ATTR_EXTENSIONS_ACL_STAGE:
                p_attr->value.s32 = acl_slice_node->acl_stage;
                break;
            case SAI_ACL_SLICE_ATTR_EXTENSIONS_ACL_TABLE_LIST:
                p_attr->value.objlist.count = 0;
                SAI_ACL_LOG_TRACE("ACL slice to app table is NPU specific info,"
                                    " Not applicable for VM");
                break;
            case SAI_ACL_SLICE_ATTR_EXTENSIONS_USED_ACL_ENTRY:
                p_attr->value.u32 = 0;
                SAI_ACL_LOG_TRACE("ACL slice used entry is NPU specific info,"
                                    " Not applicable for VM");
                break;
            case SAI_ACL_SLICE_ATTR_EXTENSIONS_AVAILABLE_ACL_ENTRY:
                p_attr->value.u32 = acl_slice_node->slice_depth;
                break;

            default:
                SAI_ACL_LOG_ERR("Attribute id: %d is not a known attribute "
                        "for acl slice object",p_attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
        }
    }

    return sai_rc;
}

sai_status_t sai_npu_get_acl_slice_attribute(sai_object_id_t acl_slice_id,
                                                uint32_t attr_count,
                                                sai_attribute_t *attr_list)
{
    STD_ASSERT(attr_list != NULL);
    acl_node_pt  acl_node = NULL;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_acl_slice_t *acl_slice_node = NULL;
    rbtree_handle acl_table_tree = NULL;

    sai_acl_lock();
    do {
        acl_node = sai_acl_get_acl_node();
        if(acl_node == NULL) {
            SAI_ACL_LOG_ERR ("Acl node get failed");
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

        acl_slice_node = sai_acl_slice_find(acl_node->sai_acl_slice_tree,
                acl_slice_id);
        if (acl_slice_node == NULL) {
            SAI_ACL_LOG_ERR ("ACL slice object %"PRIx64" not present",
                    acl_slice_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        acl_table_tree = acl_node->sai_acl_table_tree;
        if(acl_table_tree == NULL)
        {
            SAI_ACL_LOG_ERR ("ACL table node not present");
            break;
        }

        sai_rc = sai_acl_slice_attr_get(acl_table_tree, acl_slice_node,
                                        attr_count, attr_list);
        if ((sai_rc != SAI_STATUS_SUCCESS) && (sai_rc != SAI_STATUS_BUFFER_OVERFLOW))
        {
            SAI_ACL_LOG_ERR ("ACL slice attribute get failed");
        }
    } while(0);

    sai_acl_unlock();

    return sai_rc;
}

void sai_acl_dump_slice_usage(void)
{
    acl_node_pt acl_node = NULL;
    sai_acl_slice_t *acl_slice = NULL;
    rbtree_handle acl_slice_tree = NULL;
    sai_object_id_t object_id = 0;
    uint32_t count = 0;
    sai_uint32_t used_entry = 0;

    acl_node = sai_acl_get_acl_node();
    if (acl_node == NULL)
    {
        SAI_ACL_LOG_ERR("ACL Parent Node not present");
        return;
    }

    acl_slice_tree = acl_node->sai_acl_slice_tree;
    if(acl_slice_tree == NULL)
    {
        SAI_ACL_LOG_ERR("ACL Slice not present");
        return;
    }

    acl_slice = (sai_acl_slice_t *)std_rbtree_getfirst(acl_slice_tree);
    if (acl_slice == NULL)
    {
        SAI_ACL_LOG_ERR("No ACL slice present in the ACL Slice tree");
        return;
    }

    for ( ; acl_slice != NULL; acl_slice = (sai_acl_slice_t *)
            std_rbtree_getnext(acl_slice_tree,acl_slice))
    {
        object_id = 0;
        count = 0;
        used_entry = 0;
        SAI_ACL_LOG_TRACE("ACL_SLICE: object_id %"PRIu64" slice_id %d pipe_id %d "
                "acl_stage %d slice_depth %d",
                acl_slice->acl_slice_id, acl_slice->slice_id,
                acl_slice->pipe_id, acl_slice->acl_stage,
                acl_slice->slice_depth);

        SAI_ACL_LOG_TRACE("ACL_SLICE: Table_object_id %"PRIu64" num_table %d"
                " used_entry %d", object_id, count, used_entry);
    }

    return;
}

