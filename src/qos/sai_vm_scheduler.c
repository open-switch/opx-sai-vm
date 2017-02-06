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
 * @file sai_vm_scheduler.c
 *
 * @brief This file contains SAI API implementation for managing the
 *        SAI scheduler object in VM Environment.
 */

#include "sai_npu_qos.h"

#include "saistatus.h"
#include "saischeduler.h"
#include "saitypes.h"
#include <inttypes.h>
#include "std_assert.h"
#include "sai_qos_util.h"
#include "sai_qos_common.h"
#include "sai_vm_qos.h"


/**
 * Vendor attribute array for scheduler containing the attribute
 * values and properties for create,set and get functionality.
 */
static const dn_sai_attribute_entry_t sai_qos_scheduler_attr[] =  {
    /*            ID                                  MC     VC     VS    VG  IMP    SUP */
    { SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM,       false, true, true, true, true,  true },
    { SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT,          false, true, true, true, true,  true },
    { SAI_SCHEDULER_ATTR_SHAPER_TYPE,                false, true, true, true, true,  true },
    { SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE,         false, true, true, true, true,  true },
    { SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE,   false, true, true, true, true,  true },
    { SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE,         false, true, true, true, true,  true },
    { SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE,   false, true, true, true, true,  true },
};

/**
 * Function returning the attribute array for scheduler and
 * count of the total number of attributes.
 */
static void sai_vm_scheduler_attr_table_get (const dn_sai_attribute_entry_t **p_vendor,
                                         uint_t *p_max_attr_count)
{
    *p_vendor = &sai_qos_scheduler_attr[0];

    *p_max_attr_count = sizeof(sai_qos_scheduler_attr)/sizeof(sai_qos_scheduler_attr[0]);

    return;
}

static sai_status_t sai_vm_scheduler_create (dn_sai_qos_scheduler_t *p_sched_node,
                                             sai_object_id_t *p_sched_oid)
{
    uint_t free_id = 0;

    STD_ASSERT (p_sched_node != NULL);
    STD_ASSERT (p_sched_oid != NULL);

    SAI_SCHED_LOG_TRACE("NPU Scheduler create");

    free_id = std_find_first_bit (sai_vm_qos_scheduler_bitmap_get (),
                                   SAI_VM_QOS_MAX_SCHEDULER_PROFILES, 0);

    if (free_id > SAI_VM_QOS_MAX_SCHEDULER_PROFILES) {
        SAI_SCHED_LOG_TRACE("Scheduler max limit exceeded.");
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    STD_BIT_ARRAY_CLR (sai_vm_qos_scheduler_bitmap_get(), free_id);

    *p_sched_oid = sai_uoid_create (SAI_OBJECT_TYPE_SCHEDULER,
                                    (sai_npu_object_id_t) free_id);

    SAI_SCHED_LOG_TRACE ("Scheduler creation successful in NPU. "
                         "Scheduler oid 0x%"PRIx64".",*p_sched_oid);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_scheduler_remove (dn_sai_qos_scheduler_t *p_sched_node)
{
    STD_ASSERT (p_sched_node != NULL);

    uint_t  sched_id = sai_uoid_npu_obj_id_get(p_sched_node->key.scheduler_id);

    STD_BIT_ARRAY_SET (sai_vm_qos_scheduler_bitmap_get(), sched_id);

    SAI_SCHED_LOG_TRACE ("Scheduler delete successful in NPU. "
                         "Scheduler oid 0x%"PRIx64".",
                         p_sched_node->key.scheduler_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_scheduler_attribute_set (
                             dn_sai_qos_scheduler_t *p_sched_node,
                             uint_t attr_count,
                             const sai_attribute_t *p_attr_list)
{
    /* TODO */
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_scheduler_attribute_get (dn_sai_qos_scheduler_t *p_sched_node,
                                                    uint_t attr_count,
                                                    sai_attribute_t *p_attr_list)
{
    sai_status_t            sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t         *p_attr = NULL;
    size_t                  list_index = 0;

    STD_ASSERT (p_sched_node != NULL);
    STD_ASSERT (p_attr_list != NULL);

    SAI_SCHED_LOG_TRACE ("NPU Scheduler attribute get, attr_count: "
                         "%d", attr_count);

    for (list_index = 0, p_attr = p_attr_list;
         list_index < attr_count; ++list_index, ++p_attr) {

        SAI_SCHED_LOG_TRACE("NPU Scheduler attribute set, attr_id %d",
                            p_attr->id);

        switch (p_attr->id)
        {
            case SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM:
                p_attr->value.s32 = p_sched_node->sched_algo;
                break;

            case SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT:
                p_attr->value.u8 = p_sched_node->weight;
                break;

            case SAI_SCHEDULER_ATTR_SHAPER_TYPE:
                 p_attr->value.s32 = p_sched_node->shape_type;
                break;

            case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE:
                p_attr->value.u64 = p_sched_node->min_bandwidth_rate;
                break;

            case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE:
                p_attr->value.u64 = p_sched_node->min_bandwidth_burst;
                break;

            case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE:
                p_attr->value.u64 = p_sched_node->max_bandwidth_rate;
                break;

            case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE:
                p_attr->value.u64 = p_sched_node->max_bandwidth_burst;

            default:
                SAI_SCHED_LOG_ERR ("Unknown attribute id: %d.", p_attr->id);
                /* Attribute validation done in common, Not expected this case */
                sai_rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR ("Failure in get queue attr list at index: %d.",
                               list_index);

            return sai_rc;
        }
    }
    return sai_rc;
}

static bool sai_vm_scheduler_is_hw_object (void)
{
    return false;
}

static sai_status_t sai_vm_scheduler_set (sai_object_id_t oid,
                                          dn_sai_qos_scheduler_t *p_old_sched_node,
                                          dn_sai_qos_scheduler_t *p_new_sched_node)
{
    /* TODO */
    return SAI_STATUS_SUCCESS;
}
static sai_npu_scheduler_api_t sai_vm_scheduler_api_table = {
    sai_vm_scheduler_create,
    sai_vm_scheduler_remove,
    sai_vm_scheduler_attribute_set,
    sai_vm_scheduler_attribute_get,
    sai_vm_scheduler_is_hw_object,
    sai_vm_scheduler_attr_table_get,
    sai_vm_scheduler_set
};

sai_npu_scheduler_api_t* sai_vm_scheduler_api_query (void)
{
    return &sai_vm_scheduler_api_table;
}
