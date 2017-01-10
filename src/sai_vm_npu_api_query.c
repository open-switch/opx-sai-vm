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
 * @file sai_vm_npu_api_query.c
 *
 * @brief This file contains function definition for SAI NPU API query in
 *        VM environment.
 *************************************************************************/
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
    return &sai_vm_npu_api_table;
}

