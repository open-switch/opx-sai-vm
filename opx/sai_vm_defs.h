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
 * @file sai_vm_defs.h
 *
 * @brief This file contains the macro definitions related to the
 *        SAI objects in VM environment.
 */

#ifndef __SAI_VM_DEFS_H__
#define __SAI_VM_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sai_l3_util.h"
#include "sai_oid_utils.h"
#include "sai_acl_type_defs.h"
#include "std_type_defs.h"

/* MAX Table sizes */
/* TODO Get from XML config file in future. */
#define SAI_VM_MAX_VLANS                 (4096)
#define SAI_VM_MAX_LAGS                  (256)
#define SAI_VM_MAX_VRF                   (SAI_FIB_MAX_VIRTUAL_ROUTERS)
#define SAI_VM_FDB_TABLE_SIZE            (8192) /* 8 K */
#define SAI_VM_RIF_TABLE_SIZE            (8192) /* 8 K */
#define SAI_VM_NH_TABLE_SIZE             (49152) /* 48 K */
#define SAI_VM_NH_GRP_TABLE_SIZE         (4096) /* 4 K */
#define SAI_VM_ACL_TABLE_MAX_ENTRIES     (512)
#define SAI_VM_ACL_TABLE_MAX_COUNTERS    (512)
#define SAI_VM_MAX_BRIDGE_RIFS           (4096)
#define SAI_VM_INVALID_RIF_ID            ((sai_npu_object_id_t)-1)

#define SAI_VM_VLAN_ID_DEFAULT           (1)

#define SAI_VM_DFLT_L3_VLAN_ID           (4095)

#define SAI_VM_DFLT_STP_INSTANCE_ID      (0)
#define SAI_VM_DFLT_VLAN_MAX_LEARN_LIMIT (0)
#define SAI_VM_DFLT_VLAN_LEARN_DISABLE   (0)
#define SAI_VM_DFLT_VLAN_META_DATA       (0)

#define SAI_VM_SHELL_PROMPT              "SAI-VM.%d> "
#define SAI_VM_MAX_BUFSZ                 (256)

/* Switch temperature in Celsius */
#define SAI_VM_SWITCH_TEMP 40
/* Switch max lag number and max lag members*/
#define SAI_VM_SWITCH_MAX_LAG_NUMBER  1024
#define SAI_VM_SWITCH_MAX_LAG_MEMBERS 256

/* Switch max stp instances */
#define SAI_VM_SWITCH_MAX_STP_INSTANCES  256

/* Min/Max ACL Priorities for Table/Rule */
#define SAI_VM_ACL_GROUP_PRIO_LOWEST   (0)
#define SAI_VM_ACL_GROUP_PRIO_HIGHEST  (11)
#define SAI_VM_ACL_ENTRY_PRIO_LOWEST   (0)
#define SAI_VM_ACL_ENTRY_PRIO_HIGHEST  (0x7fffffff)

/* Min/Max Meta Data Range */
#define SAI_VM_ACL_FDB_DST_USER_META_DATA_MIN   (0)
#define SAI_VM_ACL_FDB_DST_USER_META_DATA_MAX   (63)
#define SAI_VM_ACL_ROUTE_DST_USER_META_DATA_MIN (0)
#define SAI_VM_ACL_ROUTE_DST_USER_META_DATA_MAX (7)
#define SAI_VM_ACL_NEIGHBOR_DST_USER_META_DATA_MIN (0)
#define SAI_VM_ACL_NEIGHBOR_DST_USER_META_DATA_MAX (7)
#define SAI_VM_ACL_PORT_USER_META_DATA_MIN (0)
#define SAI_VM_ACL_PORT_USER_META_DATA_MAX (7)
#define SAI_VM_ACL_VLAN_USER_META_DATA_MIN (0)
#define SAI_VM_ACL_VLAN_USER_META_DATA_MAX (4095)
#define SAI_VM_ACL_USER_META_DATA_MIN (0)
#define SAI_VM_ACL_USER_META_DATA_MAX (1023)


#define SAI_VM_DEFAULT_HASH_FIELD_BITMAP   ((1 << SAI_HASH_SRC_IP) | \
                                            (1 << SAI_HASH_DST_IP) | \
                                            (1 << SAI_HASH_IN_PORT) | \
                                            (1 << SAI_HASH_VLAN_ID) | \
                                            (1 << SAI_HASH_IP_PROTOCOL) | \
                                            (1 << SAI_HASH_ETHERTYPE) | \
                                            (1 << SAI_HASH_SOURCE_MAC) | \
                                            (1 << SAI_HASH_DEST_MAC) | \
                                            (1 << SAI_HASH_L4_SOURCE_PORT) | \
                                            (1 << SAI_HASH_L4_DEST_PORT))

#define SAI_VM_SWITCH_MIN_SPLIT_HORIZON_ID  (1)
#define SAI_VM_SWITCH_MAX_SPLIT_HORIZON_ID  (1)

typedef struct _sai_vm_tcb_t {
    /* Bit set indicates object ID is available for use */
    sai_switch_id_t  switch_id;
    uint8_t         *vrf_id_bitmap;
    uint8_t         *nh_id_bitmap;
    uint8_t         *nh_grp_id_bitmap;
    uint8_t         *acl_cntr_id_bitmap [SAI_ACL_TABLE_ID_MAX];
} sai_vm_tcb_t;

sai_vm_tcb_t *sai_vm_access_tcb (void);

static inline void sai_vm_switch_id_set (sai_switch_id_t switch_id)
{
    sai_vm_access_tcb()->switch_id = switch_id;
}

static inline sai_switch_id_t sai_vm_switch_id_get (void)
{
    return (sai_vm_access_tcb()->switch_id);
}

static inline uint8_t *sai_vm_access_vrf_bitmap (void)
{
    return (sai_vm_access_tcb()->vrf_id_bitmap);
}

static inline uint8_t *sai_vm_access_nh_bitmap (void)
{
    return (sai_vm_access_tcb()->nh_id_bitmap);
}

static inline uint8_t *sai_vm_access_nh_grp_bitmap (void)
{
    return (sai_vm_access_tcb()->nh_grp_id_bitmap);
}

static inline uint8_t *sai_vm_access_acl_cntr_bitmap (uint_t table_id)
{
    return (sai_vm_access_tcb()->acl_cntr_id_bitmap [table_id]);
}

#ifdef __cplusplus
}
#endif

#endif /* __SAI_VM_DEFS_H__ */
