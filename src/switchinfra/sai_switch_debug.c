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
* @file sai_switch_debug.c
*
* @brief This file contains SAI Switch component debug Utilities
*
*************************************************************************/
#include <stdio.h>
#include <inttypes.h>

#include "saitypes.h"
#include "std_mac_utils.h"

#include "sai_debug_utils.h"
#include "sai_switch_common.h"
#include "sai_switch_utils.h"
#include "sai_fdb_common.h"

void sai_switch_capability_flag_dump(void)
{
    SAI_DEBUG("sai_switch_capability_t enum values");

    SAI_DEBUG("SAI_SWITCH_CAP_OVER_SUBSCRIPTION_MODE is 0x%x",
              SAI_SWITCH_CAP_OVER_SUBSCRIPTION_MODE);
    SAI_DEBUG("SAI_SWITCH_CAP_CUT_THROUGH_MODE is 0x%x",
              SAI_SWITCH_CAP_CUT_THROUGH_MODE);
    SAI_DEBUG("SAI_SWITCH_CAP_STORE_AND_FORWARD_MODE is 0x%x",
              SAI_SWITCH_CAP_STORE_AND_FORWARD_MODE);
    SAI_DEBUG("SAI_SWITCH_CAP_UNIFIED_FORWARDING_TABLE is 0x%x",
              SAI_SWITCH_CAP_UNIFIED_FORWARDING_TABLE);
    SAI_DEBUG("SAI_SWITCH_CAP_DYNAMIC_BREAKOUT_MODE is 0x%x",
              SAI_SWITCH_CAP_DYNAMIC_BREAKOUT_MODE);
}

void sai_switch_info_dump(void)
{
    char mac_str[SAI_MAC_STR_LEN] = {0};

    const sai_switch_info_t *switch_info = sai_switch_info_get();

    SAI_DEBUG("###### SWITCH INFO TABLE DUMP #######");

    SAI_DEBUG("Switch id is %d", switch_info->switch_id);

    SAI_DEBUG("supported capabilities bitmap 0x%lx", switch_info->switch_supported_capb);

    SAI_DEBUG("Enabled capabilities bitmap 0x%lx", switch_info->switch_enabled_capb);

    SAI_DEBUG("Counter Refresh Interval is %d seconds", switch_info->counter_refresh_interval);

    SAI_DEBUG("MAC address is %s", std_mac_to_string(&(switch_info->switch_mac_addr), mac_str,
                                                     sizeof(mac_str)));

    SAI_DEBUG("Operational state is %d", switch_info->switch_op_state);

    SAI_DEBUG("CPU port is %d", switch_info->switch_cpu_port);

    SAI_DEBUG("Max number of logical port %d", switch_info->switch_lport_max);

    SAI_DEBUG("Max number of physical port %d", switch_info->switch_pport_max);

    SAI_DEBUG("Max number of lanes per port %d", switch_info->max_lane_per_port);

    SAI_DEBUG("Max number of virtual routers %d", switch_info->max_virtual_routers);

    SAI_DEBUG("Max Number of ECMP paths %d", switch_info->max_ecmp_paths);

    SAI_DEBUG("L2 table size %d", switch_info->l2_table_size);

    SAI_DEBUG("L3 host table size %d", switch_info->l3_host_table_size);

    SAI_DEBUG("L3 route table size %d", switch_info->l3_route_table_size);

    SAI_DEBUG("Max Number of UCAST Queues per port %d\r\n", switch_info->max_uc_queues_per_port);

    SAI_DEBUG("Max Number of MCAST Queues per port %d\r\n", switch_info->max_mc_queues_per_port);

    SAI_DEBUG("Total Number of Queues per port %d\r\n", switch_info->max_queues_per_port);

    SAI_DEBUG("Max Number of UCAST Queues per CPU port %d\r\n",
           switch_info->max_uc_queues_per_cpu_port);

    SAI_DEBUG("Max Number of MCAST Queues per CPU port %d\r\n",
           switch_info->max_mc_queues_per_cpu_port);

    SAI_DEBUG("Total Number of Queues per CPU port %d\r\n", switch_info->max_queues_per_cpu_port);

    SAI_DEBUG("Switch Hierarchy Fixed per port %d\r\n", switch_info->hierarchy_fixed);

    SAI_DEBUG("Max Number of Hierarchy levels supported per port %d\r\n", switch_info->max_hierarchy_levels);

    SAI_DEBUG("Max Number of Childs per Hierarchy node %d\r\n", switch_info->max_childs_per_hierarchy_node);

    SAI_DEBUG("Max Number of ingress service pools %d\r\n", switch_info->ing_max_buf_pools);

    SAI_DEBUG("Max Number of egress service pools %d\r\n", switch_info->egr_max_buf_pools);

    SAI_DEBUG("Max Number of tiles per service pool %d\r\n", switch_info->tiles_per_buf_pool);

    SAI_DEBUG("Max buffer size per tile %d\r\n", switch_info->max_tile_buffer_size);

    SAI_DEBUG("Max buffer size in switch %d\r\n", switch_info->max_buffer_size);

    sai_switch_capability_flag_dump();
}

