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
 * @file sai_vm_npu_api.h
 *
 * @brief This file contains VM NPU API related function prototypes.
 */

#ifndef _SAI_VM_NPU_API_H
#define _SAI_VM_NPU_API_H

#include "sai_npu_api_plugin.h"
#include "sai_npu_switch.h"
#include "sai_npu_port.h"
#include "sai_npu_vlan.h"
#include "sai_npu_fdb.h"
#include "sai_acl_npu_api.h"
#include "sai_npu_lag.h"
#include "sai_npu_mirror.h"
#include "sai_npu_stp.h"
#include "sai_shell_npu.h"
#include "sai_l3_api.h"
#include "sai_npu_qos.h"
#include "sai_udf_npu_api.h"

sai_npu_switch_api_t* sai_vm_switch_api_query (void);
sai_npu_port_api_t* sai_vm_port_api_query (void);
sai_npu_vlan_api_t* sai_vm_vlan_api_query (void);
sai_npu_fdb_api_t* sai_vm_fdb_api_query (void);
sai_npu_acl_api_t* sai_vm_acl_api_query (void);
sai_npu_lag_api_t* sai_vm_lag_api_query (void);
sai_npu_mirror_api_t* sai_vm_mirror_api_query (void);
sai_npu_stp_api_t* sai_vm_stp_api_query (void);
sai_npu_router_api_t* sai_vm_router_api_query (void);
sai_npu_rif_api_t* sai_vm_rif_api_query (void);
sai_npu_route_api_t* sai_vm_route_api_query (void);
sai_npu_nexthop_api_t* sai_vm_nexthop_api_query (void);
sai_npu_neighbor_api_t* sai_vm_neighbor_api_query (void);
sai_npu_nh_group_api_t* sai_vm_nh_group_api_query (void);
sai_npu_shell_api_t* sai_vm_shell_api_query (void);
sai_npu_samplepacket_api_t* sai_vm_samplepacket_api_query (void);
sai_npu_hostif_api_t* sai_vm_hostif_api_query (void);
sai_npu_qos_api_t* sai_vm_qos_api_query (void);
sai_npu_queue_api_t* sai_vm_queue_api_query (void);
sai_npu_sched_group_api_t* sai_vm_sched_group_api_query (void);
sai_npu_qos_map_api_t* sai_vm_qos_map_api_query(void);
sai_npu_scheduler_api_t* sai_vm_scheduler_api_query (void);
sai_npu_policer_api_t* sai_vm_policer_api_query(void);
sai_npu_wred_api_t* sai_vm_wred_api_query(void);
sai_npu_udf_api_t* sai_vm_udf_api_query(void);
sai_npu_buffer_api_t* sai_vm_buffer_api_query (void);
sai_npu_api_t* sai_npu_api_query (void);
sai_npu_tunnel_api_t* sai_vm_tunnel_api_query (void);
sai_npu_bridge_api_t* sai_vm_bridge_api_query (void);
sai_npu_l2mc_api_t* sai_vm_l2mc_api_query (void);
sai_npu_mcast_api_t* sai_vm_mcast_api_query (void);
sai_npu_l3_ipmc_api_t* sai_vm_l3_ipmc_api_query (void);
sai_npu_l3_mcast_api_t* sai_vm_l3_mcast_api_query (void);
#endif /* _SAI_VM_NPU_API_H */
