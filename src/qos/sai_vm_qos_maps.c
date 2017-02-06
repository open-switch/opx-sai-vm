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

/*
 * @file sai_vm_qos_maps.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI QOS Maps object in VM environment.
 */

#include "sai_qos_common.h"
#include "sai_switch_utils.h"
#include "sai_port_utils.h"
#include "sai_qos_util.h"
#include "sai_event_log.h"
#include "sai_vm_qos.h"
#include "std_assert.h"
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "sai_common_utils.h"
#include "sai_npu_qos.h"

/**
 * Vendor attribute array for qosmaps containing the attribute
 * values and properties for create,set and get functionality.
 */
static const dn_sai_attribute_entry_t sai_qos_maps_attr[] = {
    {SAI_QOS_MAP_ATTR_TYPE, true, true, false, true, true, true},
    {SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST, false, true, true, true, true, true},
};

static sai_status_t sai_vm_qos_map_create(dn_sai_qos_map_t *p_map,
                                    sai_npu_object_id_t *p_map_id)
{
    uint_t              map_idx = 0;

    STD_ASSERT(p_map != NULL);

    SAI_MAPS_LOG_TRACE("NPU map set/create for maptype %d",p_map->map_type);

    map_idx = std_find_first_bit (sai_vm_qos_maps_bitmap_get(),
            SAI_VM_QOS_MAX_MAPS, 0);

    if((map_idx < 0) ||
            (map_idx > SAI_VM_QOS_MAX_MAPS)){
        SAI_MAPS_LOG_TRACE("Map maxlimit reached for maptype %d",
                p_map->map_type);
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    *p_map_id = (sai_npu_object_id_t)map_idx;

    STD_BIT_ARRAY_CLR (sai_vm_qos_maps_bitmap_get(), map_idx);
    SAI_MAPS_LOG_INFO("Map created succesfully for maptype %d",p_map->map_type);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_qos_map_remove(dn_sai_qos_map_t *p_map)
{
    uint_t hw_map_id = 0;

    STD_ASSERT(p_map != NULL);

    hw_map_id = sai_get_id_from_npu_object
               (sai_uoid_npu_obj_id_get(p_map->key.map_id));

    STD_BIT_ARRAY_SET (sai_vm_qos_maps_bitmap_get(),
                       hw_map_id);

    SAI_MAPS_LOG_TRACE("NPU map removed for id :%d",hw_map_id);
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_qos_map_attribute_set(dn_sai_qos_map_t *p_map,
                                           uint_t attr_flags)
{
    /** @TODO: to be implemented */
    return SAI_STATUS_SUCCESS;
}


static sai_status_t sai_vm_qos_maps_get_value_list (dn_sai_qos_map_t *p_map,
                                                     sai_attribute_t *p_attr)
{
    STD_ASSERT(p_map != NULL);
    STD_ASSERT(p_attr != NULL);

    SAI_MAPS_LOG_TRACE("Map list get for maptype %d", p_map->map_type);

    if(p_attr->value.qosmap.count < p_map->map_to_value.count)
    {
        SAI_MAPS_LOG_ERR("Passed count is less than count in list");
        p_attr->value.qosmap.count = p_map->map_to_value.count;

        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    p_attr->value.qosmap.count = p_map->map_to_value.count;

    memcpy(&p_attr->value.qosmap,
           &p_map->map_to_value, sizeof(p_map->map_to_value));


    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_qos_map_attribute_get (dn_sai_qos_map_t *p_map,
                                                  uint_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    size_t          list_idx = 0;
    sai_attribute_t  *p_attr = NULL;

    STD_ASSERT (p_map != NULL);
    STD_ASSERT (attr_list != NULL);

    SAI_MAPS_LOG_TRACE("NPU get for mapid 0x%"PRIx64"",p_map->key.map_id);

    for (list_idx = 0, p_attr = &attr_list[0]; list_idx < attr_count; ++list_idx, ++p_attr)
    {
        SAI_MAPS_LOG_TRACE("Getting attr_list [%d], Attribute id: %d.",
                               list_idx, p_attr->id);

        switch(p_attr->id)
        {
            case SAI_QOS_MAP_ATTR_TYPE:
                p_attr->value.s32 = p_map->map_type;
                break;

            case SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST:
                sai_rc = sai_vm_qos_maps_get_value_list (p_map, p_attr);
                break;

            default:
                sai_rc = sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                                 list_idx);
                break;
        }
    }
    return sai_rc;
}

static bool sai_vm_qos_map_is_hw_object(sai_qos_map_type_t map_type)
{
    /** @TODO: to be implemented */
    return false;
}

static sai_status_t sai_vm_qos_port_map_set(sai_object_id_t port_id,
                                      sai_object_id_t map_id,
                                      sai_qos_map_type_t map_type,
                                      bool map_set)
{
    /** @TODO: to be implemented */
    return SAI_STATUS_SUCCESS;
}

static bool sai_vm_qos_is_map_type_supported(sai_qos_map_type_t map_type)
{
    /** @TODO: to be implemented */
    return true;
}

/**
 * Function returning the attribute array for Qos Maps and
 * count of the total number of attributes.
 */

static void sai_vm_qos_maps_attr_table_get(const dn_sai_attribute_entry_t
                                          **vendor,
                                          uint_t *max_attr_count)
{
    *vendor = &sai_qos_maps_attr[0];

    *max_attr_count = sizeof(sai_qos_maps_attr)/sizeof(dn_sai_attribute_entry_t);

    return;
}
static sai_npu_qos_map_api_t sai_vm_qos_map_api_table = {

    sai_vm_qos_map_create,
    sai_vm_qos_map_remove,
    sai_vm_qos_map_attribute_set,
    sai_vm_qos_map_attribute_get,
    sai_vm_qos_port_map_set,
    sai_vm_qos_map_is_hw_object,
    sai_vm_qos_is_map_type_supported,
    sai_vm_qos_maps_attr_table_get
};

sai_npu_qos_map_api_t* sai_vm_qos_map_api_query (void)
{
    return &sai_vm_qos_map_api_table;
}


