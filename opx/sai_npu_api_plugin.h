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
/**
* @file sai_npu_api_plugin.h
*
* @brief This file contains the SAI NPU API method table definition.
*
*************************************************************************/
#ifndef __SAI_NPU_API_PLUGIN_H__
#define __SAI_NPU_API_PLUGIN_H__

#include "sai_l3_api.h"
#include "sai_acl_npu_api.h"
#include "sai_npu_fdb.h"
#include "sai_npu_lag.h"
#include "sai_npu_mirror.h"
#include "sai_npu_port.h"
#include "sai_npu_stp.h"
#include "sai_npu_switch.h"
#include "sai_npu_vlan.h"
#include "sai_npu_l2mc.h"
#include "sai_npu_mcast.h"
#include "sai_shell_npu.h"
#include "sai_npu_samplepacket.h"
#include "sai_npu_hostif.h"
#include "sai_npu_qos.h"
#include "sai_udf_npu_api.h"
#include "sai_tunnel_npu_api.h"
#include "sai_bridge_npu_api.h"

/**
 * @brief NPU API method table.
 */
typedef struct _sai_npu_api_t {

    sai_npu_switch_api_t       *switch_api;
    sai_npu_port_api_t         *port_api;
    sai_npu_vlan_api_t         *vlan_api;
    sai_npu_fdb_api_t          *fdb_api;
    sai_npu_router_api_t       *router_api;
    sai_npu_rif_api_t          *rif_api;
    sai_npu_neighbor_api_t     *neighbor_api;
    sai_npu_nexthop_api_t      *nexthop_api;
    sai_npu_nh_group_api_t     *nh_group_api;
    sai_npu_route_api_t        *route_api;
    sai_npu_acl_api_t          *acl_api;
    sai_npu_lag_api_t          *lag_api;
    sai_npu_mirror_api_t       *mirror_api;
    sai_npu_stp_api_t          *stp_api;
    sai_npu_shell_api_t        *shell_api;
    sai_npu_samplepacket_api_t *samplepacket_api;
    sai_npu_hostif_api_t       *hostif_api;
    const sai_npu_qos_map_api_t      *qos_map_api;
    const sai_npu_qos_api_t          *qos_api;
    const sai_npu_queue_api_t        *queue_api;
    const sai_npu_sched_group_api_t  *sched_group_api;
    const sai_npu_scheduler_api_t    *scheduler_api;
    const sai_npu_policer_api_t      *policer_api;
    const sai_npu_wred_api_t         *wred_api;
    const sai_npu_udf_api_t          *udf_api;
    const sai_npu_buffer_api_t       *buffer_api;
    const sai_npu_tunnel_api_t       *tunnel_api;
    const sai_npu_bridge_api_t       *bridge_api;
    const sai_npu_l2mc_api_t         *l2mc_api;
    const sai_npu_mcast_api_t        *mcast_api;
} sai_npu_api_t;

#endif /* __SAI_NPU_API_PLUGIN_H__ */
