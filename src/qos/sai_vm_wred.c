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
 * @file sai_vm_wred.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI Wred object in VM environment.
 */

#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_npu_qos.h"
#include "sai_vm_qos.h"
#include "std_assert.h"
#include "std_bit_masks.h"
#include <inttypes.h>

static const dn_sai_attribute_entry_t sai_wred_attr[] = {
    {SAI_WRED_ATTR_GREEN_ENABLE, false, true, true, true, true, true},
    {SAI_WRED_ATTR_GREEN_MIN_THRESHOLD, false, true, true, true, true, true},
    {SAI_WRED_ATTR_GREEN_MAX_THRESHOLD, false, true, true, true, true, true},
    {SAI_WRED_ATTR_GREEN_DROP_PROBABILITY, false, true, true, true, true, true},
    {SAI_WRED_ATTR_YELLOW_ENABLE, false, true, true, true, true, true},
    {SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD, false, true, true, true, true, true},
    {SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD, false, true, true, true, true, true},
    {SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY, false, true, true, true, true, true},
    {SAI_WRED_ATTR_RED_ENABLE, false, true, true, true, true, true},
    {SAI_WRED_ATTR_RED_MIN_THRESHOLD, false, true, true, true, true, true},
    {SAI_WRED_ATTR_RED_MAX_THRESHOLD, false, true, true, true, true, true},
    {SAI_WRED_ATTR_RED_DROP_PROBABILITY, false, true, true, true, true, true},
    {SAI_WRED_ATTR_WEIGHT, false, true, true, true, true, true},
    {SAI_WRED_ATTR_ECN_MARK_ENABLE, false, true, true, true, true, true},
};

static sai_status_t sai_vm_wred_create(dn_sai_qos_wred_t *p_wred_node,
                                        sai_npu_object_id_t *p_wred_id)
{
    uint_t  wred_profile_idx = 0;

    STD_ASSERT(p_wred_node != NULL);

    SAI_WRED_LOG_TRACE("NPU wred create");

    wred_profile_idx = std_find_first_bit (sai_vm_qos_wred_bitmap_get(),
                                      SAI_VM_QOS_MAX_WRED_PROFILES, 0);

    if(wred_profile_idx > SAI_VM_QOS_MAX_WRED_PROFILES){
        SAI_WRED_LOG_ERR("Map maxlimit reached for wred");
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    *p_wred_id = (sai_npu_object_id_t)wred_profile_idx;

    STD_BIT_ARRAY_CLR (sai_vm_qos_wred_bitmap_get(), wred_profile_idx);

    SAI_WRED_LOG_INFO("Wred created successfully with 0x%"PRIx64"",*p_wred_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_wred_remove(dn_sai_qos_wred_t *p_wred_node)
{
    STD_ASSERT(p_wred_node != NULL);

    uint_t npu_wred_idx = sai_uoid_npu_obj_id_get(p_wred_node->key.wred_id);

    STD_BIT_ARRAY_SET (sai_vm_qos_wred_bitmap_get(), npu_wred_idx);

    SAI_WRED_LOG_INFO("NPU wred removed for id %d",npu_wred_idx);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_wred_set(dn_sai_qos_wred_t *p_wred_node,
                                     uint32_t attr_count,
                                     const sai_attribute_t *p_attr)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_wred_get(dn_sai_qos_wred_t *p_wred_node,
                                     uint32_t attr_count,
                                     sai_attribute_t *p_attr_list)
{
    return SAI_STATUS_SUCCESS;
}
static bool sai_vm_wred_supported_on_port()
{
    return false;
}

static bool sai_vm_wred_is_hw_object()
{
    return true;
}

static sai_status_t sai_vm_wred_apply_or_remove_on_queue(sai_npu_object_id_t queue_id,
                                                          dn_sai_qos_wred_t *p_wred_node,
                                                          bool wred_set)
{
    return SAI_STATUS_SUCCESS;
}

static void sai_vm_wred_attr_table_get(const dn_sai_attribute_entry_t
                                        **vendor,
                                        uint_t *max_attr_count)
{
    *vendor = &sai_wred_attr[0];

    *max_attr_count = sizeof(sai_wred_attr)/sizeof(dn_sai_attribute_entry_t);

    return;
}

static uint_t sai_vm_wred_max_buf_size_get()
{
    return 0;
}
static sai_npu_wred_api_t sai_vm_wred_api_table = {
    sai_vm_wred_create,
    sai_vm_wred_remove,
    sai_vm_wred_set,
    sai_vm_wred_get,
    sai_vm_wred_apply_or_remove_on_queue,
    sai_vm_wred_supported_on_port,
    sai_vm_wred_is_hw_object,
    sai_vm_wred_max_buf_size_get,
    sai_vm_wred_attr_table_get
};

sai_npu_wred_api_t* sai_vm_wred_api_query (void)
{
    return &sai_vm_wred_api_table;
}

