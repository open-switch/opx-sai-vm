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

/*********
* @file sai_common_infra.h
*
* @brief API Query and Init related APIs.
*
*************************************************************************/
/** OPENSOURCELICENSE */

#ifndef __SAI_COMMON_INFRA_H
#define __SAI_COMMON_INFRA_H

#include "sai.h"
#include "sai_npu_api_plugin.h"

/*
 * Temporary macro for log level. Will be removed.
 */

#define SAI_SWITCH_EV_LOG_LEVEL 3

/*
 * Temporary Macro for default switch Identifier
 */
#define SAI_DEFAULT_SWTICH_ID  0

/*
 * Temporary Macro for sai npu library name
 */
#define SAI_DEFAULT_NPU_LIB_NAME  NULL

/*
 * Get SAI service method table
 */
const sai_service_method_table_t *sai_service_method_table_get(void);

/*
 * Stub functions for sai_api_query for respective API Ids.
 * Will be removed once the actual implementation is avaiable
 * from all modules.
 */
sai_port_api_t *sai_port_api_query(void);

sai_fdb_api_t *sai_fdb_api_query(void);

sai_vlan_api_t *sai_vlan_api_query(void);

sai_lag_api_t *sai_lag_api_query(void);

sai_virtual_router_api_t *sai_vr_api_query(void);

sai_route_api_t *sai_route_api_query(void);

sai_next_hop_api_t *sai_nexthop_api_query(void);

sai_next_hop_group_api_t *sai_nexthop_group_api_query(void);

sai_router_interface_api_t *sai_router_intf_api_query(void);

sai_neighbor_api_t *sai_neighbor_api_query(void);

sai_acl_api_t *sai_acl_api_query(void);

sai_stp_api_t *sai_stp_api_query (void);

sai_mirror_api_t *sai_mirror_api_query(void);

sai_qos_map_api_t *sai_qosmaps_api_query (void);

sai_hostif_api_t *sai_hostif_api_query(void);

sai_samplepacket_api_t* sai_samplepacket_api_query(void);

sai_policer_api_t* sai_policer_api_query(void);

sai_scheduler_group_api_t* sai_qos_sched_group_api_query(void);

sai_scheduler_api_t* sai_qos_scheduler_api_query(void);

sai_queue_api_t* sai_qos_queue_api_query(void);

sai_wred_api_t* sai_wred_api_query(void);

sai_buffer_api_t* sai_qos_buffer_api_query(void);

sai_udf_api_t* sai_udf_api_query(void);

sai_hash_api_t* sai_hash_api_query(void);

sai_tunnel_api_t* sai_tunnel_api_query(void);

sai_bridge_api_t* sai_bridge_api_query(void);

sai_l2mc_group_api_t* sai_l2mc_group_api_query(void);

sai_l2mc_api_t* sai_l2mc_entry_api_query(void);

sai_ipmc_group_api_t* sai_ipmc_group_api_query(void);

sai_rpf_group_api_t* sai_ipmc_rpf_group_api_query(void);

sai_ipmc_repl_group_api_t* sai_ipmc_repl_group_api_query(void);

sai_ipmc_api_t* sai_ipmc_entry_api_query(void);

sai_status_t sai_extn_module_api_query (sai_api_t sai_api_id, void** api_method_table);

sai_status_t sai_npu_api_initialize (const char *lib_name);

sai_npu_api_t* sai_npu_api_table_get (void);

void sai_npu_api_uninitialize (void);

static inline sai_npu_neighbor_api_t* sai_neighbor_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->neighbor_api));
}

static inline sai_npu_nexthop_api_t* sai_nexthop_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->nexthop_api));
}

static inline sai_npu_nh_group_api_t * sai_nh_group_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->nh_group_api));
}

static inline sai_npu_route_api_t* sai_route_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->route_api));
}

static inline sai_npu_rif_api_t* sai_rif_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->rif_api));
}

static inline sai_npu_router_api_t* sai_router_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->router_api));
}

static inline sai_npu_fdb_api_t* sai_fdb_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->fdb_api));
}

static inline sai_npu_switch_api_t* sai_switch_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->switch_api));
}

static inline sai_npu_port_api_t* sai_port_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->port_api));
}

static inline sai_npu_mirror_api_t* sai_mirror_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->mirror_api));
}

static inline sai_npu_stp_api_t* sai_stp_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->stp_api));
}

static inline sai_npu_acl_api_t* sai_acl_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->acl_api));
}

static inline sai_npu_shell_api_t* sai_shell_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->shell_api));
}

static inline sai_npu_vlan_api_t* sai_vlan_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->vlan_api));
}

static inline sai_npu_lag_api_t* sai_lag_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->lag_api));
}

static inline sai_npu_samplepacket_api_t* sai_samplepacket_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->samplepacket_api));
}

static inline sai_npu_hostif_api_t* sai_hostif_npu_api_get(void)
{
    return ((sai_npu_api_table_get()->hostif_api));
}

static inline const sai_npu_qos_api_t* sai_qos_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->qos_api));
}

static inline const sai_npu_queue_api_t* sai_queue_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->queue_api));
}

static inline const sai_npu_sched_group_api_t* sai_sched_group_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->sched_group_api));
}

static inline const sai_npu_policer_api_t* sai_qos_policer_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->policer_api));
}

static inline const sai_npu_qos_map_api_t * sai_qos_map_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->qos_map_api));
}

static inline const sai_npu_scheduler_api_t* sai_scheduler_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->scheduler_api));
}

static inline const sai_npu_wred_api_t * sai_qos_wred_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->wred_api));
}

static inline const sai_npu_buffer_api_t * sai_buffer_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->buffer_api));
}

static inline const sai_npu_udf_api_t *sai_udf_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->udf_api));
}

static inline const sai_npu_tunnel_api_t *sai_tunnel_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->tunnel_api));
}

static inline const sai_npu_bridge_api_t *sai_bridge_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->bridge_api));
}

static inline const sai_npu_l2mc_api_t *sai_l2mc_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->l2mc_api));
}

static inline const sai_npu_mcast_api_t *sai_mcast_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->mcast_api));
}

static inline const sai_npu_l3_ipmc_api_t *sai_l3_ipmc_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->l3_ipmc_api));
}

static inline const sai_npu_l3_mcast_api_t *sai_l3_mcast_npu_api_get (void)
{
    return ((sai_npu_api_table_get()->l3_mcast_api));
}

#endif
