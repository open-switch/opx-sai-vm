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
* @file sai_switch_common.h
*
* @brief This file contains the data structure definitions for SAI Switch objects.
*
*************************************************************************/

/* OPENSOURCELICENSE */

#ifndef __SAI_SWITCH_COMMON_H__
#define __SAI_SWITCH_COMMON_H__

#include "std_type_defs.h"
#include "std_rbtree.h"

#include "saitypes.h"
#include "saiswitch.h"
#include "saiport.h"
#include "saihash.h"
#include "sai_port_common.h"
#include "sai_hash_object.h"

/** \defgroup SAISWITCHAPI SAI - Switch Structures
 *   SAI Switch component Data structures and definition
 *
 *  \ingroup SAIAPI
 *  \{
 */


/**
 * Switch default counter refresh interval in seconds
 */
#define SAI_DEFAULT_COUNTER_REFRESH_INTERVAL (1)

/**
 * @brief SAI switch capabilities and information.
 */
typedef struct _sai_switch_info_t {
    /** Switch Id passed from adapter host */
    sai_switch_id_t          switch_id;

    /** Port level info of all ports in the switch [sai_port_info_t] */
    sai_port_info_table_t    port_info_table;

    /** Switch supported capability flags [sai_switch_capability_t] */
    uint64_t                 switch_supported_capb;

    /** Switch capabilities enabled [sai_switch_capability_t] */
    uint64_t                 switch_enabled_capb;

    /** Switch Operation Status */
    sai_switch_oper_status_t switch_op_state;

    /** Switch Mac address */
    sai_mac_t                switch_mac_addr;

    /** Switch's CPU connected port */
    sai_npu_port_id_t        switch_cpu_port;

    /** Counter statistics collection refresh interval with default
     * value 1 sec(SW cache) and value 0 for HW counters read */
    uint_t                   counter_refresh_interval;

    /** Maximum number of logical ports count*/
    sai_npu_port_id_t        switch_lport_max;

    /** Maximum number of physical ports count */
    sai_npu_port_id_t        switch_pport_max;

    /** Maximum possible lanes per port in the switch */
    uint_t                   max_lane_per_port;

    /** Maximum port MTU size in bytes */
    uint32_t                 max_port_mtu;

    /** Maximum number of Virtual Routers */
    uint_t                   max_virtual_routers;

    /** Maximum ECMP paths */
    uint_t                   max_ecmp_paths;

    /** L2 Table size */
    uint_t                   l2_table_size;

    /** L3 host Table size */
    uint_t                   l3_host_table_size;

    /** L3 route Table size */
    uint_t                   l3_route_table_size;

    /** Maximum number of traffic classes supported */
    uint_t                   max_supported_tc;

    /** Maximum number of unicast queues per port */
    uint_t                   max_uc_queues_per_port;

    /** Maximum number of multicast queues per port */
    uint_t                   max_mc_queues_per_port;

    /** Total Number unicast/multicast queues per port. */
    uint_t                   max_queues_per_port;

    /** Maximum number of unicast queues per cpu port */
    uint_t                   max_uc_queues_per_cpu_port;

    /** Maximum number of multicast queues per cpu port */
    uint_t                   max_mc_queues_per_cpu_port;

    /** Total Number unicast/multicast queues per cpu port. */
    uint_t                   max_queues_per_cpu_port;

    /** Max Hierarchy scheduler group levels supported per port */
    uint_t                   max_hierarchy_levels;

    /** Max Hierarchy childs per scheduler group node */
    uint_t                   max_childs_per_hierarchy_node;

    /** Hierarchy is fixed or flexible supported per port */
    bool                     hierarchy_fixed;

    /** Maximum number of learned MAC addresses */
    uint32_t                 max_mac_learn_limit;

    /** FDB aging time */
    uint32_t                 fdb_aging_time;

    /** packet action for FDB unicast miss. */
    sai_packet_action_t      fdbUcastMissPktAction;

    /** packet action for FDB broadcast miss. */
    sai_packet_action_t      fdbBcastMissPktAction;

    /** packet action for FDB multicast miss. */
    sai_packet_action_t      fdbMcastMissPktAction;

    /** L2 broadcast flood control to CPU port */
    bool                     isBcastCpuFloodEnable;

    /** L2 multicast flood control to CPU port */
    bool                     isMcastCpuFloodEnable;

    /** Ecmp hash seed value */
    sai_uint32_t             ecmp_hash_seed;

    /** Lag hash seed value */
    sai_uint32_t             lag_hash_seed;

    /** Ecmp hash algorithm value */
    sai_hash_algorithm_t     ecmp_hash_algo;

    /** Lag hash algorithm value */
    sai_hash_algorithm_t     lag_hash_algo;

    /** Default TC for the switch */
    uint_t                   default_tc;

    /** Max buffer size supported */
    uint_t                   max_buffer_size;

    /** Num Priority groups supported */
    uint_t                   num_pg;

    /** Size of a cell in MMU */
    uint_t                   cell_size;

    /** Max service pools supported at ingress */
    uint_t                   ing_max_buf_pools;

    /** Max service pools supported at egress */
    uint_t                   egr_max_buf_pools;

    /** Max tiles per service pools */
    uint_t                   tiles_per_buf_pool;

    /** Max tile buffer size */
    uint_t                   max_tile_buffer_size;

    /** Max tiles in the switch */
    uint_t                   max_tiles;

    /** Applicaton info on the ports */
    rbtree_handle            port_applications_tree;

    /* number of LAG members */
    uint_t                   num_lag_members;

    /* number of LAGs */
    uint_t                   num_lag;

    /* number of ECMP groups */
    uint_t                   num_ecmp_groups;

    /* software linkscan timer interval */
    uint_t                   sw_linkscan_interval;

    /* switch  hardward info */
    sai_s8_list_t             switch_hardware_info;

    /*Firmware file path info*/
    sai_s8_list_t            microcode_module_name;

    /*Switch profile ID info */
    uint32_t                 profile_id;

    /*Switch init flag */
    bool                     switch_init_flag;

    /*Switch Object ID */
    sai_object_id_t          switch_object_id;


} sai_switch_info_t;

/**
 * @brief SAI switch initialization configuration
 */
typedef struct _sai_switch_init_config_t {

    /** Switch's CPU connected port */
    sai_npu_port_id_t        cpu_port;

    /** Maximum number of logical ports count*/
    sai_npu_port_id_t        max_logical_ports;

    /** Maximum number of physical ports count */
    sai_npu_port_id_t        max_physical_ports;

    /** Maximum port MTU size in bytes */
    uint32_t                 max_port_mtu;

    /** Maximum possible lanes per port in the switch */
    uint_t                   max_lane_per_port;

    /** Switch supported capability flags [sai_switch_capability_t] */
    uint64_t                 supported_capb;

    /** L2 Table size */
    uint_t                   l2_table_size;

    /** L3 host Table size */
    uint_t                   l3_host_table_size;

    /** L3 route Table size */
    uint_t                   l3_route_table_size;

    /** Maximum number of traffic classes supported */
    uint_t                   max_supported_tc;

    /** Maximum number of unicast queues per port */
    uint_t                   max_uc_queues_per_port;

    /** Maximum number of multicast queues per port */
    uint_t                   max_mc_queues_per_port;

    /** Total Number unicast/multicast queues per port. */
    uint_t                   max_queues_per_port;

    /** Maximum number of unicast queues per cpu port */
    uint_t                   max_uc_queues_per_cpu_port;

    /** Maximum number of multicast queues per cpu port */
    uint_t                   max_mc_queues_per_cpu_port;

    /** Total Number unicast/multicast queues per cpu port. */
    uint_t                   max_queues_per_cpu_port;

    /** Max Hierarchy scheduler group levels supported per port */
    uint_t                   max_hierarchy_levels;

    /** Max Hierarchy childs per scheduler group node */
    uint_t                   max_childs_per_hierarchy_node;

    /** Hierarchy is fixed or flexible supported per port */
    bool                     hierarchy_fixed;

    /** Max buffer size supported */
    uint_t                   max_buffer_size;

    /** Num Priority groups supported */
    uint_t                   num_pg;

    /** Size of a cell in MMU */
    uint_t                   cell_size;

    /** Max service pools supported at ingress*/
    uint_t                   ing_max_buf_pools;

    /** Max service pools supported at egress*/
    uint_t                   egr_max_buf_pools;

    /** Max tiles per service pools */
    uint_t                   tiles_per_buf_pool;

    /** Max tiles in the switch */
    uint_t                   max_tiles;

    /** Max tile buffer size */
    uint_t                   max_tile_buffer_size;

} sai_switch_init_config_t;

/**
 * @brief Masks for Switch Capability Flags
 */
typedef enum _sai_switch_capbility_t {
    /** Switch Bandwidth Over subscription */
    SAI_SWITCH_CAP_OVER_SUBSCRIPTION_MODE = (1 << 0),

    /** Packet Cut through Switching */
    SAI_SWITCH_CAP_CUT_THROUGH_MODE = (1 << 1),

    /** Packet Store and Forward Switching */
    SAI_SWITCH_CAP_STORE_AND_FORWARD_MODE = (1 << 2),

    /** Unified Forwarding Table  */
    SAI_SWITCH_CAP_UNIFIED_FORWARDING_TABLE = (1 << 3),

    /** On the fly dynamic breakout support */
    SAI_SWITCH_CAP_DYNAMIC_BREAKOUT_MODE = (1 << 4),

    /** Add new capabilities above this */
    SAI_SWITCH_CAP_MAX = (1 << 5)
} sai_switch_capability_t;

/**
 * \}
 */

#endif /* __SAI_SWITCH_COMMON_H__ */
