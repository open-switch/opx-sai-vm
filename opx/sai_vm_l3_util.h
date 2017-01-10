
/************************************************************************
 * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
 *
 * This source code is confidential, proprietary, and contains trade
 * secrets that are the sole property of Dell Inc.
 * Copy and/or distribution of this source code or disassembly or reverse
 * engineering of the resultant object code are strictly forbidden without
 * the written consent of Dell Inc.
 *
 ************************************************************************/
/*
 * @file sai_vm_l3_util.h
 *
 * @brief This file contains the macro definitions related to the
 *        SAI L3 objects in VM environment.
 *************************************************************************/

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

static inline uint_t sai_vm_port_rif_id_start_get (void)
{
    return ((sai_vm_lag_rif_id_end_get ()) + 1);
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
