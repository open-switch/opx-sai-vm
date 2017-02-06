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
 * @file sai_vm_policer.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI Policer object in VM environment.
 */

#include "sai_qos_common.h"
#include "sai_switch_utils.h"
#include "sai_port_utils.h"
#include "sai_qos_util.h"
#include "sai_event_log.h"
#include "sai_npu_qos.h"
#include "sai_vm_qos.h"
#include "std_assert.h"
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

static const dn_sai_attribute_entry_t sai_qos_policer_attr[] = {
    {SAI_POLICER_ATTR_METER_TYPE, true, true, false, true, true, true},
    {SAI_POLICER_ATTR_MODE, false, true, false, true, true, true},
    {SAI_POLICER_ATTR_COLOR_SOURCE, false, true, false, true, true, true},
    {SAI_POLICER_ATTR_CBS, false, true, true, true, true, true},
    {SAI_POLICER_ATTR_CIR, false, true, true, true, true, true},
    {SAI_POLICER_ATTR_PBS, false, true, true, true, true, true},
    {SAI_POLICER_ATTR_PIR, false, true, true, true, true, true},
    {SAI_POLICER_ATTR_GREEN_PACKET_ACTION, false, true, true, true, true, true},
    {SAI_POLICER_ATTR_RED_PACKET_ACTION, false, true, true, true, true, true},
    {SAI_POLICER_ATTR_YELLOW_PACKET_ACTION, false, true, true, true, true, true},
    {SAI_POLICER_ATTR_ENABLE_COUNTER_LIST, false, true, true, true, false, false},
};

static sai_status_t sai_vm_qos_policer_create(dn_sai_qos_policer_t *p_policer,
                                              sai_npu_object_id_t  *p_pol_id)
{
    uint_t              policer_idx = 0;

    STD_ASSERT(p_policer != NULL);

    SAI_POLICER_LOG_TRACE("NPU policer create for policer");

    policer_idx = std_find_first_bit (sai_vm_qos_policer_bitmap_get(),
            SAI_VM_QOS_MAX_POLICERS, 0);

    if((policer_idx < 0) ||
            (policer_idx > SAI_VM_QOS_MAX_POLICERS)){
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    *p_pol_id = (sai_npu_object_id_t)policer_idx;

    STD_BIT_ARRAY_CLR (sai_vm_qos_policer_bitmap_get(), policer_idx);
    SAI_POLICER_LOG_INFO("Map created succesfully for policertype");

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_qos_policer_remove(sai_object_id_t policer_id)
{
    uint_t policer_idx = 0;

    policer_idx = sai_get_id_from_npu_object
               (sai_uoid_npu_obj_id_get(policer_id));

    STD_BIT_ARRAY_SET (sai_vm_qos_policer_bitmap_get(),
                       policer_idx);

    SAI_MAPS_LOG_TRACE("Policer removed for idx :%d",policer_idx);
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_qos_policer_attribute_set(dn_sai_qos_policer_t *p_policer_node,
                                                      const sai_attribute_t *pattr)
{
    return SAI_STATUS_SUCCESS;
}

static void sai_vm_qos_policer_action_fill(sai_attribute_t *p_attr,
                                       dn_sai_qos_policer_t *p_policer)
{
    STD_ASSERT(p_attr != NULL);
    STD_ASSERT(p_policer != NULL);
    size_t idx = 0;
    bool is_found = false;

    for(idx = 0; idx < p_policer->action_count; idx ++){
        if(p_attr->id == p_policer->action_list[idx].action){
            is_found = true;
            p_attr->value.s32 = p_policer->action_list[idx].value;
            break;
        }
    }
    if(!is_found){
        p_attr->value.s32 = SAI_PACKET_ACTION_FORWARD;
    }
}

static sai_status_t sai_vm_qos_policer_attribute_get (dn_sai_qos_policer_t *p_policer,
                                                      uint_t attr_count,
                                                      sai_attribute_t *attr_list)
{
    size_t   attr_index = 0;
    sai_attribute_t *p_attr = NULL;

    STD_ASSERT(attr_list != NULL);
    STD_ASSERT(p_policer != NULL);

    for(attr_index = 0, p_attr = &attr_list[0]; attr_index < attr_count; ++attr_index, ++p_attr)
    {

        switch (p_attr->id)
        {
            case SAI_POLICER_ATTR_METER_TYPE:
                p_attr->value.s32 = p_policer->meter_type;
                break;

            case SAI_POLICER_ATTR_MODE:
                p_attr->value.s32 = p_policer->policer_mode;
                break;

            case SAI_POLICER_ATTR_COLOR_SOURCE:
                p_attr->value.s32 = p_policer->color_source;
                break;

            case SAI_POLICER_ATTR_CBS:
                p_attr->value.u64 = p_policer->cbs;
                break;

            case SAI_POLICER_ATTR_CIR:
                p_attr->value.u64 = p_policer->cir;
                break;

            case SAI_POLICER_ATTR_PBS:
                p_attr->value.u64 = p_policer->pbs;
                break;

            case SAI_POLICER_ATTR_PIR:
                p_attr->value.u64 = p_policer->pir;
                break;

            case SAI_POLICER_ATTR_GREEN_PACKET_ACTION:
            case SAI_POLICER_ATTR_RED_PACKET_ACTION:
            case SAI_POLICER_ATTR_YELLOW_PACKET_ACTION:
                sai_vm_qos_policer_action_fill(p_attr, p_policer);
                break;

            case SAI_POLICER_ATTR_ENABLE_COUNTER_LIST:
                break;

            default:
                SAI_POLICER_LOG_ERR ("Attribute id: %d is not a known attribute "
                                     "for Policer.", p_attr->id);

                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static bool sai_vm_qos_policer_is_storm_control_hw(void)
{
        return false;
}

static bool sai_vm_qos_is_policer_acl_reinstall_needed(void)
{
        return false;
}


static sai_status_t sai_vm_qos_storm_control_policer_port_set(sai_object_id_t port_obj_id,
                                                               dn_sai_qos_policer_t *p_policer,
                                                               uint_t type, bool is_add)
{
    return SAI_STATUS_SUCCESS;
}
static void sai_vm_qos_policer_attr_table_get(const dn_sai_attribute_entry_t
                                               **vendor,
                                               uint_t *max_attr_count)
{
    *vendor = &sai_qos_policer_attr[0];

    *max_attr_count = sizeof(sai_qos_policer_attr)/sizeof(dn_sai_attribute_entry_t);

    return;
}

static bool sai_vm_qos_is_port_policer_supported(void)
{
    return false;
}

static sai_npu_policer_api_t sai_vm_qos_policer_api_table = {
    sai_vm_qos_policer_create,
    sai_vm_qos_policer_remove,
    sai_vm_qos_policer_attribute_set,
    sai_vm_qos_policer_attribute_get,
    sai_vm_qos_policer_attr_table_get,
    sai_vm_qos_policer_is_storm_control_hw,
    sai_vm_qos_is_policer_acl_reinstall_needed,
    sai_vm_qos_storm_control_policer_port_set,
    sai_vm_qos_is_port_policer_supported,

};

sai_npu_policer_api_t* sai_vm_policer_api_query(void)
{
    return &sai_vm_qos_policer_api_table;
}

