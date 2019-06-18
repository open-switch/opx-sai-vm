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
 * @file sai_vm_buffer.c
 *
 * @brief This file contains the buffer initialization, default settings of
 *        for buffer objects and SAI API implementation for managing the
 *        SAI buffer object in VM Environment.
 */

#include "sai_npu_qos.h"

#include "saistatus.h"
#include "saitypes.h"
#include "sai_qos_common.h"
#include "saibuffer.h"
#include <inttypes.h>
#include "sai_qos_util.h"
#include "sai_qos_buffer_util.h"
#include "std_bit_masks.h"
#include "sai_vm_qos.h"
#include "std_assert.h"

/**
 * Vendor attribute array for buffer containing the attribute
 * values and properties for create,set and get functionality.
 */
static const dn_sai_attribute_entry_t sai_qos_buffer_pool_attr[] =  {
    /*            ID                      MC     VC     VS    VG     IMP    SUP */
    { SAI_BUFFER_POOL_ATTR_SHARED_SIZE , false, false, false, true , true , true },
    { SAI_BUFFER_POOL_ATTR_TYPE        , true , true , false, true , true , true },
    { SAI_BUFFER_POOL_ATTR_SIZE        , true , true , true , true , true , true },
    { SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE, false, true , true , true , true , true},
    { SAI_BUFFER_POOL_ATTR_XOFF_SIZE   , false, true , true , true , true , true},
};

static const dn_sai_attribute_entry_t sai_qos_buffer_profile_attr[] =  {
    /*            ID                              MC     VC     VS    VG     IMP    SUP */
    { SAI_BUFFER_PROFILE_ATTR_POOL_ID           , true , true , true , true , true , true },
    { SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE       , true , true , true , true , true , true },
    { SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE    , false, true , true , true , true , true },
    { SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH , false, true , true , true , true , true },
    { SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH  , false, true , true , true , true , true },
    { SAI_BUFFER_PROFILE_ATTR_XOFF_TH           , false, true , true , true , true , true },
    { SAI_BUFFER_PROFILE_ATTR_XON_TH            , false, true , true , true , true , true },
    { SAI_BUFFER_PROFILE_ATTR_XON_OFFSET_TH     , false, true , true , true , true , true },
};

/**
 * Function returning the attribute array for buffer pool and
 * count of the total number of attributes.
 */
static void sai_vm_buffer_pool_attr_table_get (const dn_sai_attribute_entry_t **p_vendor,
                                         uint_t *p_max_attr_count)
{
    *p_vendor = &sai_qos_buffer_pool_attr[0];

    *p_max_attr_count = sizeof(sai_qos_buffer_pool_attr)/
                               sizeof(sai_qos_buffer_pool_attr[0]);

    return;
}
/**
 * Function returning the attribute array for buffer profile and
 * count of the total number of attributes.
 */
static void sai_vm_buffer_profile_attr_table_get (const dn_sai_attribute_entry_t **p_vendor,
                                                   uint_t *p_max_attr_count)
{
    *p_vendor = &sai_qos_buffer_profile_attr[0];

    *p_max_attr_count = sizeof(sai_qos_buffer_profile_attr)/
                               sizeof(sai_qos_buffer_profile_attr[0]);

    return;
}

static sai_status_t sai_vm_buffer_init ()
{
    dn_sai_vm_qos_tcb_t *vm_qos_tcb = sai_vm_qos_access_tcb();
    uint_t num_bytes_per_kb = 1024;

    vm_qos_tcb->ing_max_buf_pools = sai_switch_ing_max_buf_pools_get();
    vm_qos_tcb->egr_max_buf_pools = sai_switch_egr_max_buf_pools_get();
    vm_qos_tcb->ing_num_sp = 0;
    vm_qos_tcb->egr_num_sp = 0;
    vm_qos_tcb->cell_size = sai_switch_cell_size_get();
    vm_qos_tcb->num_pg = sai_switch_num_pg_get();
    vm_qos_tcb->ing_buffer_size = (sai_switch_max_buffer_size_get() * num_bytes_per_kb);
    vm_qos_tcb->egr_buffer_size = vm_qos_tcb->ing_buffer_size;

    vm_qos_tcb->ing_sp_bitmap =
        std_bitmap_create_array (vm_qos_tcb->ing_max_buf_pools);

    if(vm_qos_tcb->ing_sp_bitmap == NULL)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    vm_qos_tcb->egr_sp_bitmap =
        std_bitmap_create_array (vm_qos_tcb->egr_max_buf_pools);

    if(vm_qos_tcb->egr_sp_bitmap == NULL)
    {
        return SAI_STATUS_NO_MEMORY;
    }


    return SAI_STATUS_SUCCESS;
}

static inline sai_object_id_t sai_npu_create_buffer_pool_object (sai_buffer_pool_type_t type,
                                                                 uint_t index)
{
    return sai_uoid_create(SAI_OBJECT_TYPE_BUFFER_POOL,
                           SAI_VM_POOL_ID_CREATE(type, index));
}


static sai_status_t sai_vm_buffer_pool_create (dn_sai_qos_buffer_pool_t *p_buf_pool_node,
                                                sai_object_id_t *p_buf_pool_oid)
{
    uint_t free_idx = 0;
    uint_t num_sp = 0;
    uint_t max_buf_pools = 0;
    uint_t buffer_size = 0;
    uint8_t *free_idx_bmp;
    dn_sai_vm_qos_tcb_t *vm_qos_tcb = sai_vm_qos_access_tcb();

    STD_ASSERT (p_buf_pool_node != NULL);
    STD_ASSERT (p_buf_pool_oid != NULL);

    if(p_buf_pool_node->pool_type == SAI_BUFFER_POOL_TYPE_INGRESS) {
        num_sp = vm_qos_tcb->ing_num_sp;
        max_buf_pools = vm_qos_tcb->ing_max_buf_pools;
        free_idx_bmp = vm_qos_tcb->ing_sp_bitmap;
        buffer_size =  vm_qos_tcb->ing_buffer_size;
    } else if (p_buf_pool_node->pool_type == SAI_BUFFER_POOL_TYPE_EGRESS) {
        num_sp = vm_qos_tcb->egr_num_sp;
        max_buf_pools = vm_qos_tcb->egr_max_buf_pools;
        free_idx_bmp = vm_qos_tcb->egr_sp_bitmap;
        buffer_size =  vm_qos_tcb->egr_buffer_size;

    } else {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(num_sp == max_buf_pools) {
        SAI_BUFFER_LOG_WARN ("Maximum allowed pools is %u", max_buf_pools);
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    if(buffer_size < p_buf_pool_node->shared_size) {
        SAI_BUFFER_LOG_WARN ("Buffer not available. Remaining:%d",buffer_size);
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }
    free_idx = std_find_first_bit (free_idx_bmp, max_buf_pools, 0);
    STD_BIT_ARRAY_CLR(free_idx_bmp, free_idx);
    num_sp++;

    if(p_buf_pool_node->pool_type == SAI_BUFFER_POOL_TYPE_INGRESS) {
        vm_qos_tcb->ing_num_sp = num_sp;
        vm_qos_tcb->ing_buffer_size -= p_buf_pool_node->shared_size;
    } else {
        vm_qos_tcb->egr_num_sp = num_sp;
        vm_qos_tcb->egr_buffer_size -= p_buf_pool_node->shared_size;
    }
    *p_buf_pool_oid = sai_npu_create_buffer_pool_object (p_buf_pool_node->pool_type,
            free_idx);


    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_buffer_pool_remove (dn_sai_qos_buffer_pool_t *p_buf_pool_node)
{
    uint_t npu_pool_id = sai_uoid_npu_obj_id_get(p_buf_pool_node->key.pool_id);
    uint_t sp_id = SAI_POOL_SPID_GET(npu_pool_id);
    dn_sai_vm_qos_tcb_t *vm_qos_tcb = sai_vm_qos_access_tcb();

    if (p_buf_pool_node->pool_type == SAI_BUFFER_POOL_TYPE_INGRESS) {
        vm_qos_tcb->ing_num_sp--;
        vm_qos_tcb->ing_buffer_size += p_buf_pool_node->size;
        STD_BIT_ARRAY_SET(vm_qos_tcb->ing_sp_bitmap, sp_id);

    } else if (p_buf_pool_node->pool_type == SAI_BUFFER_POOL_TYPE_EGRESS) {
        vm_qos_tcb->egr_num_sp--;
        vm_qos_tcb->egr_buffer_size += p_buf_pool_node->size;
        STD_BIT_ARRAY_SET(vm_qos_tcb->egr_sp_bitmap, sp_id);

    } else {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;

}

static sai_status_t sai_vm_buffer_pool_update (dn_sai_qos_buffer_pool_t *p_buf_pool_node,
                                                uint_t new_size)
{
    uint_t shared_size = (new_size + p_buf_pool_node->shared_size -
                          p_buf_pool_node->size);
    dn_sai_vm_qos_tcb_t *vm_qos_tcb = sai_vm_qos_access_tcb();
    uint_t cur_size = p_buf_pool_node->shared_size;
    uint_t free_buffer = 0;

    STD_ASSERT (p_buf_pool_node != NULL);

    if (p_buf_pool_node->pool_type == SAI_BUFFER_POOL_TYPE_INGRESS) {
        free_buffer = vm_qos_tcb->ing_buffer_size;
    } else if (p_buf_pool_node->pool_type == SAI_BUFFER_POOL_TYPE_EGRESS) {
        free_buffer = vm_qos_tcb->egr_buffer_size;
    } else {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((free_buffer + cur_size) < shared_size) {
        SAI_BUFFER_LOG_WARN("Cannot update pool. Buffer avaiable :%u", free_buffer);
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    /** Case where an object is applied buffer profile and bytes to cell does round off there*/
    if(new_size == p_buf_pool_node->size) {
        shared_size -= 1;
    }

    if(p_buf_pool_node->pool_type == SAI_BUFFER_POOL_TYPE_INGRESS) {
        vm_qos_tcb->ing_buffer_size = (free_buffer + cur_size - shared_size);
    } else {
        vm_qos_tcb->egr_buffer_size = (free_buffer + cur_size - shared_size);
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_buffer_pool_attr_set (dn_sai_qos_buffer_pool_t *p_buf_pool_node,
                                                  const sai_attribute_t *attr)
{
    sai_status_t ret_val;

    switch(attr->id) {
        case SAI_BUFFER_POOL_ATTR_SHARED_SIZE:
        case SAI_BUFFER_POOL_ATTR_TYPE:
            ret_val = SAI_STATUS_INVALID_ATTRIBUTE_0;
            break;
        case SAI_BUFFER_POOL_ATTR_SIZE:
            ret_val = sai_vm_buffer_pool_update(p_buf_pool_node, attr->value.u32);
            break;
        case SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE:
        case SAI_BUFFER_POOL_ATTR_XOFF_SIZE:
            ret_val = SAI_STATUS_SUCCESS;
            break;
        default:
            ret_val = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
    }

    return ret_val;
}

static sai_status_t sai_vm_buffer_pool_attr_get (const dn_sai_qos_buffer_pool_t  *p_buf_pool_node,
                                                 uint32_t attr_count,
                                                 sai_attribute_t *attr_list)
{
    uint_t attr_idx;

    STD_ASSERT(p_buf_pool_node != NULL);
    STD_ASSERT(attr_list != NULL);

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        switch(attr_list[attr_idx].id) {
            case SAI_BUFFER_POOL_ATTR_TYPE:
                attr_list[attr_idx].value.s32 = p_buf_pool_node->pool_type;
                break;

            case SAI_BUFFER_POOL_ATTR_SIZE:
                attr_list[attr_idx].value.u32 = p_buf_pool_node->size;
                break;

            case SAI_BUFFER_POOL_ATTR_XOFF_SIZE:
                attr_list[attr_idx].value.u32 = p_buf_pool_node->xoff_size;
                break;

            case SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE:
                attr_list[attr_idx].value.s32 = p_buf_pool_node->threshold_mode;
                break;

            case SAI_BUFFER_POOL_ATTR_SHARED_SIZE:
                attr_list[attr_idx].value.u32 = p_buf_pool_node->shared_size;
                break;

            default:
                return sai_get_indexed_ret_val(SAI_STATUS_UNKNOWN_ATTRIBUTE_0, attr_idx);
        }
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_buffer_profile_create (dn_sai_qos_buffer_profile_t
                                                   *p_buffer_profile_node,
                                                   sai_object_id_t *buffer_profile_id)
{
    uint_t free_idx = 0;
    STD_ASSERT (p_buffer_profile_node != NULL);
    STD_ASSERT (buffer_profile_id != NULL);

    free_idx = std_find_first_bit (sai_vm_qos_buffer_profile_bitmap_get(),
                                   SAI_VM_QOS_MAX_BUFFER_PROFILES, 0);

    *buffer_profile_id = sai_uoid_create (SAI_OBJECT_TYPE_BUFFER_PROFILE, free_idx);

    STD_BIT_ARRAY_CLR (sai_vm_qos_buffer_profile_bitmap_get(), free_idx);
    SAI_BUFFER_LOG_TRACE ("Buffer profile creation successful in VM. "
                          "Buffer profile oid 0x%"PRIx64".",*buffer_profile_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_buffer_profile_remove (dn_sai_qos_buffer_profile_t
                                                   *p_buffer_profile_node)
{
    sai_npu_object_id_t npu_index;
    dn_sai_vm_qos_tcb_t *vm_qos_tcb = sai_vm_qos_access_tcb();
    STD_ASSERT (p_buffer_profile_node != NULL);

    npu_index = sai_uoid_npu_obj_id_get(p_buffer_profile_node->key.profile_id);
    STD_BIT_ARRAY_SET (vm_qos_tcb->buffer_profile_bitmap, npu_index);
    SAI_BUFFER_LOG_TRACE ("Buffer profile deletion successful in VM. "
                          "Buffer profile oid 0x%"PRIx64".",
                           p_buffer_profile_node->key.profile_id);

    return SAI_STATUS_SUCCESS;
}
static sai_status_t sai_vm_buffer_profile_attr_set (sai_object_id_t obj_id,
                                                     dn_sai_qos_buffer_profile_t
                                                     *p_buf_profile_node,
                                                     const sai_attribute_t *old_attr,
                                                     const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_apply_buffer_profile (sai_object_id_t obj_id,
                                                 dn_sai_qos_buffer_profile_t
                                                 *p_old_buf_profile_node,
                                                  dn_sai_qos_buffer_profile_t
                                                  *p_buf_profile_node,  bool is_retry)
{

    /* TODO */
    return SAI_STATUS_SUCCESS;
}
static sai_status_t sai_vm_pg_create (dn_sai_qos_pg_t *p_pg_node,
                                       uint_t index,
                                       sai_object_id_t *pg_id)
{
    STD_ASSERT (p_pg_node != NULL);
    STD_ASSERT (pg_id != NULL);
    sai_npu_port_id_t local_port = 0;

    if(sai_port_to_npu_local_port(p_pg_node->port_id, &local_port)
                                 != SAI_STATUS_SUCCESS) {
        SAI_BUFFER_LOG_WARN("Unable to find the VM port mapping \
                for port :0x%"PRIx64"",p_pg_node->port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *pg_id = sai_uoid_create(SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP,
                             SAI_VM_PG_ID_CREATE (local_port, index));
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_buffer_pool_stats_get (sai_object_id_t pool_id,
                                                  const sai_buffer_pool_stat_t *counter_ids,
                                                  uint32_t number_of_counters,
                                                  uint64_t* counters)
{
    STD_ASSERT (counter_ids != NULL);
    STD_ASSERT (counters != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_pg_attr_get (dn_sai_qos_pg_t *p_pg_node,
                                                uint32_t attr_count,
                                                sai_attribute_t *attr_list)
{
    STD_ASSERT (p_pg_node != NULL);
    STD_ASSERT (attr_list != NULL);

    return SAI_STATUS_SUCCESS;
}


static sai_status_t sai_vm_pg_stats_get (sai_object_id_t pg_id,
                                         const sai_ingress_priority_group_stat_t
                                         *counter_ids, uint32_t number_of_counters,
                                         uint64_t* counters)
{
    STD_ASSERT (counter_ids != NULL);
    STD_ASSERT (counters != NULL);

    return SAI_STATUS_SUCCESS;

}

static sai_status_t sai_vm_buffer_pg_stats_clear (sai_object_id_t pg_id,
                                                  const sai_ingress_priority_group_stat_t
                                                  *counter_ids, uint32_t number_of_counters)
{
    STD_ASSERT (counter_ids != NULL);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_vm_pg_destroy (dn_sai_qos_pg_t *p_pg_node)
{
    STD_ASSERT (p_pg_node != NULL);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_vm_qos_check_buffer_size (sai_object_id_t pool_id,
                 const dn_sai_qos_buffer_profile_t *p_buf_profile_node, uint_t add_size)
{
    return SAI_STATUS_SUCCESS;
}

static sai_npu_buffer_api_t sai_vm_buffer_api_table = {
    sai_vm_buffer_init,
    sai_vm_buffer_pool_create,
    sai_vm_buffer_pool_remove,
    sai_vm_buffer_pool_attr_set,
    sai_vm_buffer_pool_attr_get,
    sai_vm_buffer_pool_stats_get,
    sai_vm_buffer_profile_create,
    sai_vm_buffer_profile_remove,
    sai_vm_buffer_profile_attr_set,
    sai_vm_apply_buffer_profile,
    sai_vm_pg_create,
    sai_vm_pg_destroy,
    sai_vm_pg_attr_get,
    sai_vm_pg_stats_get,
    sai_vm_buffer_pg_stats_clear,
    sai_vm_buffer_pool_attr_table_get,
    sai_vm_buffer_profile_attr_table_get,
    sai_vm_qos_check_buffer_size,
};

sai_npu_buffer_api_t* sai_vm_buffer_api_query (void)
{
    return &sai_vm_buffer_api_table;
}

