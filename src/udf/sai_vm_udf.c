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
 * @file sai_vm_udf.c
 *
 * @brief This file contains function implementations of NPU Plugin APIs
 *        for SAI UDF in VM environment.
 */

#include "saitypes.h"
#include "saistatus.h"
#include "sai_udf_common.h"
#include "sai_udf_npu_api.h"
#include "sai_vm_udf.h"
#include "std_assert.h"
#include <string.h>

static bool udf_group_id_in_use[SAI_VM_MAX_UDF_GROUP_NUMBER];
static bool udf_id_in_use[SAI_VM_MAX_UDF_NUMBER];
static sai_vm_udf_match_t udf_match[SAI_VM_MAX_UDF_MATCH_NUMBER];

static sai_status_t sai_vm_udf_init (void)
{
    return SAI_STATUS_SUCCESS;
}

static void sai_vm_udf_deinit (void)
{
}

static sai_status_t sai_vm_udf_group_create (dn_sai_udf_group_t *p_udf_group,
                                             sai_npu_object_id_t *p_group_id)
{
    STD_ASSERT (p_udf_group != NULL);
    STD_ASSERT (p_group_id != NULL);

    uint_t index;

    for(index = 0; index < SAI_VM_MAX_UDF_GROUP_NUMBER; index++) {
        if(!udf_group_id_in_use[index]) {
            udf_group_id_in_use[index] = true;
            *p_group_id = index;
            return SAI_STATUS_SUCCESS;
        }
    }

    SAI_UDF_LOG_ERR("VM: UDF Group Index unavailable");

    return SAI_STATUS_INSUFFICIENT_RESOURCES;
}

static sai_status_t sai_vm_udf_group_remove (dn_sai_udf_group_t *p_udf_group)
{
    STD_ASSERT (p_udf_group != NULL);

    uint_t udf_group_idx =
             (uint_t)sai_uoid_npu_obj_id_get(p_udf_group->key.group_obj_id);

    if(udf_group_idx < SAI_VM_MAX_UDF_GROUP_NUMBER) {
        if(udf_group_id_in_use [udf_group_idx]) {
            udf_group_id_in_use [udf_group_idx] = false;
            return SAI_STATUS_SUCCESS;
        } else {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
    }

    return SAI_STATUS_INVALID_OBJECT_ID;
}

static sai_status_t sai_vm_udf_group_hw_id_get (
                                         const dn_sai_udf_group_t *p_udf_group,
                                         sai_npu_object_id_t *p_hw_id)
{
    STD_ASSERT (p_hw_id != NULL);
    STD_ASSERT (p_udf_group != NULL);

    uint_t udf_group_idx =
             (uint_t)sai_uoid_npu_obj_id_get(p_udf_group->key.group_obj_id);

    if(udf_group_idx < SAI_VM_MAX_UDF_GROUP_NUMBER) {
        if(udf_group_id_in_use [udf_group_idx]) {
            *p_hw_id = udf_group_idx;
            return SAI_STATUS_SUCCESS;
        } else {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
    }
    return SAI_STATUS_INVALID_OBJECT_ID;
}

static sai_status_t sai_vm_udf_group_attribute_set (
                                             dn_sai_udf_group_t *p_udf_group,
                                             const sai_attribute_t *attr)
{
    STD_ASSERT (p_udf_group != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_udf_group_attribute_get (
                                         const dn_sai_udf_group_t *p_udf_group,
                                         uint_t attr_count,
                                         sai_attribute_t *attr_list)
{
    STD_ASSERT (p_udf_group != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_udf_obj_create (dn_sai_udf_t *udf_obj,
                                        sai_npu_object_id_t *udf_id)
{
    STD_ASSERT (udf_obj != NULL);
    STD_ASSERT (udf_id != NULL);

    uint_t index;

    for(index = 0; index < SAI_VM_MAX_UDF_NUMBER; index++) {
        if(!udf_id_in_use[index]) {
            udf_id_in_use[index] = true;
            *udf_id = index;
            return SAI_STATUS_SUCCESS;
        }
    }

    SAI_UDF_LOG_ERR("VM: UDF Index unavailable");

    return SAI_STATUS_INSUFFICIENT_RESOURCES;
}

static sai_status_t sai_vm_udf_obj_remove (dn_sai_udf_t *udf_obj)
{
    STD_ASSERT (udf_obj != NULL);

    uint_t udf_idx =
             (uint_t)sai_uoid_npu_obj_id_get(udf_obj->key.udf_obj_id);

    if(udf_idx < SAI_VM_MAX_UDF_NUMBER) {
        if(udf_id_in_use [udf_idx]) {
            udf_id_in_use [udf_idx] = false;
            return SAI_STATUS_SUCCESS;
        } else {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
    }
    return SAI_STATUS_INVALID_OBJECT_ID;
}

static sai_status_t sai_vm_udf_attribute_set (dn_sai_udf_t *udf_obj,
                                               const sai_attribute_t *attr)
{
    STD_ASSERT (udf_obj != NULL);
    STD_ASSERT (attr != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_udf_attribute_get (const dn_sai_udf_t *udf_obj,
                                               uint_t attr_count,
                                               sai_attribute_t *attr_list)
{
    STD_ASSERT (udf_obj != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_udf_match_fill (sai_vm_udf_match_t *udf_match,
                                           uint_t attr_count,
                                           const sai_attribute_t *attr_list)
{
    sai_status_t            rc = SAI_STATUS_SUCCESS;
    const sai_attribute_t  *p_attr;
    uint_t                  attr_idx;

    STD_ASSERT (udf_match != NULL);

    for (attr_idx = 0, p_attr = attr_list; (attr_idx < attr_count);
         ++attr_idx, ++p_attr) {

        switch (p_attr->id) {

            case SAI_UDF_MATCH_ATTR_L2_TYPE:
                udf_match->l2_type_data = p_attr->value.aclfield.data.u16;
                udf_match->l2_type_mask = p_attr->value.aclfield.mask.u16;
                break;

            case SAI_UDF_MATCH_ATTR_L3_TYPE:
                udf_match->l3_type_data = p_attr->value.aclfield.data.u8;
                udf_match->l3_type_mask = p_attr->value.aclfield.mask.u8;
                break;

            case SAI_UDF_MATCH_ATTR_GRE_TYPE:
                udf_match->gre_type_data = p_attr->value.aclfield.data.u16;
                udf_match->gre_type_mask = p_attr->value.aclfield.mask.u16;
                break;

            case SAI_UDF_MATCH_ATTR_PRIORITY:
                udf_match->priority = p_attr->value.u8;
                break;

            default:
                rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }
        if (rc != SAI_STATUS_SUCCESS) {
            return sai_get_indexed_ret_val (rc, attr_idx);
        }
    }

    return rc;
}

static sai_status_t sai_vm_udf_match_create (uint_t attr_count,
                                              const sai_attribute_t *attr_list,
                                              sai_npu_object_id_t *p_match_id)
{
    STD_ASSERT (p_match_id != NULL);

    uint_t index;

    for(index = 0; index < SAI_VM_MAX_UDF_MATCH_NUMBER; index++) {
        if(!udf_match[index].udf_match_in_use) {
            memset (&udf_match[index], 0, sizeof (sai_vm_udf_match_t));
            udf_match[index].udf_match_in_use = true;
            *p_match_id = index;
            return (sai_vm_udf_match_fill (&udf_match[index], attr_count,
                                           attr_list));
        }
    }

    SAI_UDF_LOG_ERR("VM: UDF Match Index unavailable");

    return SAI_STATUS_INSUFFICIENT_RESOURCES;
}

static sai_status_t sai_vm_udf_match_remove (sai_object_id_t udf_match_id)
{
    uint_t match_idx = (uint_t)sai_uoid_npu_obj_id_get(udf_match_id);

    if(match_idx < SAI_VM_MAX_UDF_MATCH_NUMBER) {
        if(udf_match[match_idx].udf_match_in_use) {
            udf_match[match_idx].udf_match_in_use = false;
            memset (&udf_match[match_idx], 0, sizeof (sai_vm_udf_match_t));
            return SAI_STATUS_SUCCESS;
        } else {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
    }

    return SAI_STATUS_INVALID_OBJECT_ID;
}

static sai_status_t sai_vm_udf_match_attribute_set (
                                                sai_object_id_t udf_match_id,
                                                const sai_attribute_t *sai_attr)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_udf_match_attr_list_fill (
                                         const sai_vm_udf_match_t *udf_match,
                                         uint_t attr_count,
                                         sai_attribute_t *attr_list)
{
    sai_status_t       rc = SAI_STATUS_SUCCESS;
    sai_attribute_t   *p_attr;
    uint_t             attr_idx;

    STD_ASSERT (udf_match != NULL);

    for (attr_idx = 0, p_attr = attr_list; (attr_idx < attr_count);
         ++attr_idx, ++p_attr) {

        switch (p_attr->id) {

            case SAI_UDF_MATCH_ATTR_L2_TYPE:
                p_attr->value.aclfield.data.u16 = udf_match->l2_type_data;
                p_attr->value.aclfield.mask.u16 = udf_match->l2_type_mask;
                break;

             case SAI_UDF_MATCH_ATTR_L3_TYPE:
                p_attr->value.aclfield.data.u8 = udf_match->l3_type_data;
                p_attr->value.aclfield.mask.u8 = udf_match->l3_type_mask;
                break;

            case SAI_UDF_MATCH_ATTR_GRE_TYPE:
                p_attr->value.aclfield.data.u16 = udf_match->gre_type_data;
                p_attr->value.aclfield.mask.u16 = udf_match->gre_type_mask;
                break;

            case SAI_UDF_MATCH_ATTR_PRIORITY:
                p_attr->value.u8 = udf_match->priority;
                break;

            default:
                rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }

        if (rc != SAI_STATUS_SUCCESS) {
            return sai_get_indexed_ret_val (rc, attr_idx);
        }
    }

    return rc;
}

static sai_status_t sai_vm_udf_match_attribute_get (
                                                sai_object_id_t udf_match_id,
                                                uint_t attr_count,
                                                sai_attribute_t *attr_list)
{
    uint_t match_idx = (uint_t)sai_uoid_npu_obj_id_get(udf_match_id);

    if(match_idx < SAI_VM_MAX_UDF_MATCH_NUMBER) {
        if(!udf_match[match_idx].udf_match_in_use) {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
    } else {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    return (sai_vm_udf_match_attr_list_fill (&udf_match[match_idx], attr_count,
                                             attr_list));
}

/*
 * NPU plugin API method table
 */
static sai_npu_udf_api_t sai_vm_udf_api_table;

sai_npu_udf_api_t* sai_vm_udf_api_query (void)
{
    sai_vm_udf_api_table.udf_init                = sai_vm_udf_init;
    sai_vm_udf_api_table.udf_deinit              = sai_vm_udf_deinit;
    sai_vm_udf_api_table.udf_group_create        = sai_vm_udf_group_create;
    sai_vm_udf_api_table.udf_group_remove        = sai_vm_udf_group_remove;
    sai_vm_udf_api_table.udf_group_hw_id_get     = sai_vm_udf_group_hw_id_get;
    sai_vm_udf_api_table.udf_group_attribute_set = sai_vm_udf_group_attribute_set;
    sai_vm_udf_api_table.udf_group_attribute_get = sai_vm_udf_group_attribute_get;
    sai_vm_udf_api_table.udf_create              = sai_vm_udf_obj_create;
    sai_vm_udf_api_table.udf_remove              = sai_vm_udf_obj_remove;
    sai_vm_udf_api_table.udf_attribute_set       = sai_vm_udf_attribute_set;
    sai_vm_udf_api_table.udf_attribute_get       = sai_vm_udf_attribute_get;
    sai_vm_udf_api_table.udf_match_create        = sai_vm_udf_match_create;
    sai_vm_udf_api_table.udf_match_remove        = sai_vm_udf_match_remove;
    sai_vm_udf_api_table.udf_match_attribute_set = sai_vm_udf_match_attribute_set;
    sai_vm_udf_api_table.udf_match_attribute_get = sai_vm_udf_match_attribute_get;

    return &sai_vm_udf_api_table;
}

