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
 * @file sai_vm_l3_util.h
 *
 * @brief This file contains the macro definitions related to the
 *        SAI L3 objects in VM environment.
 */

#ifndef __SAI_VM_L3_UTIL_H__
#define __SAI_VM_L3_UTIL_H__

#include "sai_vm_defs.h"
#include "sai_oid_utils.h"
#include "saitypes.h"
#include "std_type_defs.h"

/*
 * Definitions for getting the Router Interface table index for port, LAG
 * and VLAN.
 */
static inline uint_t sai_vm_vlan_rif_id_end_get (void)
{
    return (SAI_VM_MAX_VLANS - 1);
}

static inline uint_t sai_vm_lag_rif_id_start_get (void)
{
    return ((sai_vm_vlan_rif_id_end_get ()) + 1);
}

static inline uint_t sai_vm_lag_rif_id_end_get (void)
{
    return ((sai_vm_lag_rif_id_start_get ()) + SAI_VM_MAX_LAGS - 1);
}

static inline uint_t sai_vm_bridge_rif_id_start_get (void)
{
    return ((sai_vm_lag_rif_id_end_get ()) + 1);
}

static inline uint_t sai_vm_bridge_rif_id_end_get (void)
{
    return ((sai_vm_bridge_rif_id_start_get ()) + SAI_VM_MAX_BRIDGE_RIFS - 1);
}

static inline uint_t sai_vm_port_rif_id_start_get (void)
{
    return ((sai_vm_bridge_rif_id_end_get ()) + 1);
}

static inline sai_npu_object_id_t sai_vm_vlan_rif_id_get (sai_vlan_id_t vlan_id)
{
    return ((sai_npu_object_id_t) vlan_id);
}

static inline sai_npu_object_id_t sai_vm_lag_rif_id_get (
sai_npu_object_id_t lag_id)
{
    return (sai_vm_lag_rif_id_start_get () + lag_id);
}

static inline sai_npu_object_id_t sai_vm_port_rif_id_get (
sai_npu_object_id_t port_id)
{
    return (sai_vm_port_rif_id_start_get () + port_id);
}
#endif /* __SAI_VM_L3_UTIL_H__ */
