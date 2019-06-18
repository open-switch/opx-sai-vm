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
 * @file sai_vm_npu_api_query.c
 *
 * @brief This file contains function definition for SAI NPU API query in
 *        VM environment.
 */

#include "sai_npu_api_plugin.h"
#include "sai_vm_npu_api.h"
#include <string.h>

static sai_npu_api_t sai_vm_npu_api_table;

sai_npu_api_t* sai_npu_api_query (void)
{
    memset (&sai_vm_npu_api_table, 0, sizeof (sai_vm_npu_api_table));

    sai_vm_npu_api_table.router_api = sai_vm_router_api_query ();
    sai_vm_npu_api_table.rif_api = sai_vm_rif_api_query ();
    sai_vm_npu_api_table.neighbor_api = sai_vm_neighbor_api_query ();
    sai_vm_npu_api_table.nexthop_api = sai_vm_nexthop_api_query ();
    sai_vm_npu_api_table.nh_group_api = sai_vm_nh_group_api_query ();
    sai_vm_npu_api_table.route_api = sai_vm_route_api_query ();

    sai_vm_npu_api_table.switch_api = sai_vm_switch_api_query ();
    sai_vm_npu_api_table.port_api = sai_vm_port_api_query ();
    sai_vm_npu_api_table.vlan_api = sai_vm_vlan_api_query ();
    sai_vm_npu_api_table.fdb_api = sai_vm_fdb_api_query ();
    sai_vm_npu_api_table.acl_api = sai_vm_acl_api_query ();
    sai_vm_npu_api_table.lag_api = sai_vm_lag_api_query ();
    sai_vm_npu_api_table.mirror_api = sai_vm_mirror_api_query ();
    sai_vm_npu_api_table.stp_api = sai_vm_stp_api_query ();
    sai_vm_npu_api_table.shell_api = sai_vm_shell_api_query ();
    sai_vm_npu_api_table.samplepacket_api = sai_vm_samplepacket_api_query ();
    sai_vm_npu_api_table.hostif_api = sai_vm_hostif_api_query ();
    sai_vm_npu_api_table.qos_api = sai_vm_qos_api_query ();
    sai_vm_npu_api_table.queue_api = sai_vm_queue_api_query ();
    sai_vm_npu_api_table.sched_group_api = sai_vm_sched_group_api_query ();
    sai_vm_npu_api_table.scheduler_api = sai_vm_scheduler_api_query ();
    sai_vm_npu_api_table.qos_map_api = sai_vm_qos_map_api_query();
    sai_vm_npu_api_table.policer_api = sai_vm_policer_api_query();
    sai_vm_npu_api_table.wred_api = sai_vm_wred_api_query();
    sai_vm_npu_api_table.udf_api = sai_vm_udf_api_query ();
    sai_vm_npu_api_table.buffer_api = sai_vm_buffer_api_query ();
    sai_vm_npu_api_table.tunnel_api = sai_vm_tunnel_api_query ();
    sai_vm_npu_api_table.bridge_api = sai_vm_bridge_api_query ();
    sai_vm_npu_api_table.l2mc_api = sai_vm_l2mc_api_query ();
    sai_vm_npu_api_table.mcast_api = sai_vm_mcast_api_query ();
    sai_vm_npu_api_table.l3_ipmc_api = sai_vm_l3_ipmc_api_query ();
    sai_vm_npu_api_table.l3_mcast_api = sai_vm_l3_mcast_api_query ();
    return &sai_vm_npu_api_table;
}

