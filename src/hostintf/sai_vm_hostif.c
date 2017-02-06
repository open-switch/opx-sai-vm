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
 * @file sai_vm_hostif.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI Host interface in VM environment.
 */

#include "saitypes.h"
#include "saistatus.h"
#include "sai_hostif_common.h"
#include "sai_npu_hostif.h"

static sai_status_t sai_vm_hostif_init()
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_hostintf_send_packet(
                                       const void* buffer,
                                       size_t buff_size,
                                       uint_t attr_count,
                                       const sai_attribute_t *attr_list)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_hostif_validate_trapgroup(
                                      const sai_attribute_t *attr,
                                      dn_sai_hostif_op_t operation)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_hostif_update_trapgroup(
                                        const dn_sai_trap_node_t * trap_node,
                                        const dn_sai_trap_group_node_t *trap_group,
                                        const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}


static sai_status_t sai_vm_hostif_validate_trap(sai_hostif_trap_type_t trapid,
                                                 const sai_attribute_t *attr,
                                                 dn_sai_hostif_op_t operation)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_hostif_set_trap(dn_sai_trap_node_t *trap_node,
                                   const dn_sai_trap_group_node_t *trap_group,
                                   const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

static void sai_vm_hostintf_reg_packet_rx_fn(
                           const sai_packet_event_notification_fn pkt_rx_fn)
{
}

static void sai_vm_hostintf_dump_trap(const dn_sai_trap_node_t *trap_node)
{
}

static sai_npu_hostif_api_t sai_vm_hostif_api_table = {

    sai_vm_hostif_init,
    sai_vm_hostif_validate_trapgroup,
    sai_vm_hostif_update_trapgroup,
    sai_vm_hostif_validate_trap,
    sai_vm_hostif_set_trap,
    sai_vm_hostintf_send_packet,
    sai_vm_hostintf_reg_packet_rx_fn,
    sai_vm_hostintf_dump_trap
};

sai_npu_hostif_api_t* sai_vm_hostif_api_query (void)
{
    return &sai_vm_hostif_api_table;
}
