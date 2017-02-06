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
 * @file sai_vm_qos.h
 *
 * @brief This file contains the function prototypes for SAI QOS Utilities
 *        in VM environment.
 */

#ifndef __SAI_VM_QOS_H__
#define __SAI_VM_QOS_H__

#include "saiport.h"

#define SAI_VM_QOS_MAX_SUPPORTED_QUEUES        (4096)
#define SAI_VM_QOS_MAX_SCHEDULER_PROFILES      (4096)
#define SAI_VM_QOS_MAX_SCHEDULER_GROUPS        (4096)
#define SAI_VM_QOS_MAX_BUFFER_PROFILES         (4096)
#define SAI_VM_QOS_MAX_WRED_PROFILES           (128)
#define SAI_VM_QOS_MAX_MAPS                    (4096)
#define SAI_VM_QOS_MAX_POLICERS                (4096)
#define SAI_POOL_TYPE_BITPOS   16
#define SAI_POOL_SPID_BITPOS   0

#define SAI_POOL_TYPE_MASK   0xFFFF0000
#define SAI_POOL_SPID_MASK   0x0000FFFF

#define SAI_VM_POOL_ID_CREATE(type, id) \
        (((type <<  SAI_POOL_TYPE_BITPOS) & SAI_POOL_TYPE_MASK) | \
         ((id << SAI_POOL_SPID_BITPOS) & SAI_POOL_SPID_MASK))

#define SAI_POOL_SPID_GET(npu_pool_id) \
        ((npu_pool_id & SAI_POOL_SPID_MASK) >> SAI_POOL_SPID_BITPOS)

#define SAI_POOL_TYPE_GET(npu_pool_id) \
        ((npu_pool_id & SAI_POOL_TYPE_MASK) >> SAI_POOL_TYPE_BITPOS)

#define SAI_VM_PGID_PORT_NUM_BITPOS  16
#define SAI_VM_PGID_PG_NUM_BITPOS    0

#define SAI_VM_PGID_PORT_NUM_MASK   0xFFFF0000
#define SAI_VM_PGID_PG_NUM_MASK     0x0000FFFF

#define SAI_VM_PG_ID_CREATE(port, pg) \
        (((port << SAI_VM_PGID_PORT_NUM_BITPOS ) & SAI_VM_PGID_PORT_NUM_MASK) | \
         ((pg << SAI_VM_PGID_PG_NUM_BITPOS) & SAI_VM_PGID_PG_NUM_MASK))

#define SAI_VM_PG_NUM_GET(npu_pgid) \
        ((npu_pgid & SAI_VM_PGID_PG_NUM_MASK) >> SAI_VM_PGID_PG_NUM_BITPOS)

#define SAI_VM_PORT_NUM_GET(npu_pgid) \
        ((npu_pgid & SAI_VM_PGID_PORT_NUM_MASK) >> SAI_VM_PGID_PORT_NUM_BITPOS)

typedef struct _dn_sai_vm_qos_tcb_t
{
    uint8_t *queue_bitmap;
    uint8_t *scheduler_bitmap;
    uint8_t *scheduler_group_bitmap;
    uint8_t *wred_bitmap;
    uint8_t *buffer_profile_bitmap;
    uint8_t *ing_sp_bitmap;
    uint8_t *egr_sp_bitmap;
    uint8_t *maps_bitmap;
    uint8_t *policer_bitmap;
    uint_t  ing_max_buf_pools;
    uint_t  egr_max_buf_pools;
    uint_t  ing_num_sp;
    uint_t  egr_num_sp;
    uint_t  cell_size;
    uint_t  num_pg;
    uint_t  ing_buffer_size;
    uint_t  egr_buffer_size;
} dn_sai_vm_qos_tcb_t;

dn_sai_vm_qos_tcb_t *sai_vm_qos_access_tcb (void);

static inline uint8_t *sai_vm_qos_queue_bitmap_get (void)
{
    return (sai_vm_qos_access_tcb()->queue_bitmap);
}

static inline uint8_t *sai_vm_qos_scheduler_bitmap_get (void)
{
    return (sai_vm_qos_access_tcb()->scheduler_bitmap);
}

static inline uint8_t *sai_vm_qos_scheduler_group_bitmap_get (void)
{
    return (sai_vm_qos_access_tcb()->scheduler_group_bitmap);
}

static inline uint8_t *sai_vm_qos_ing_sp_bitmap_get (void)
{
    return (sai_vm_qos_access_tcb()->ing_sp_bitmap);
}

static inline uint8_t *sai_vm_qos_egr_sp_bitmap_get (void)
{
    return (sai_vm_qos_access_tcb()->egr_sp_bitmap);
}

static inline uint8_t *sai_vm_qos_wred_bitmap_get (void)
{
    return (sai_vm_qos_access_tcb()->wred_bitmap);
}

static inline uint8_t *sai_vm_qos_buffer_profile_bitmap_get (void)
{
    return (sai_vm_qos_access_tcb()->buffer_profile_bitmap);
}

static inline uint8_t *sai_vm_qos_maps_bitmap_get (void)
{
    return (sai_vm_qos_access_tcb()->maps_bitmap);
}

static inline uint8_t *sai_vm_qos_policer_bitmap_get (void)
{
    return (sai_vm_qos_access_tcb()->policer_bitmap);
}

sai_status_t sai_qos_port_attribute_get(sai_npu_object_id_t port_id,
                                        sai_port_attr_t port_attr,
                                        sai_attribute_value_t *value);
#endif /* __SAI_VM_QOS_H__ */
