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
* @file sai_switch_utils.c
*
* @brief This file contains SAI Common Switch Utility API's.
*        Common Switch Utility API's can be used by other SAI components.
*
*************************************************************************/
#include <string.h>
#include <stdlib.h>

#include "saiswitch.h"
#include "saitypes.h"
#include "saistatus.h"

#include "std_assert.h"
#include "std_mutex_lock.h"

#include "sai_oid_utils.h"
#include "sai_switch_common.h"
#include "sai_switch_utils.h"
#include "sai_oid_utils.h"
#include "sai_l3_util.h"
#include "sai_infra_api.h"


/*
 * Global Switch info table that includes per port info as well
 */
static sai_switch_info_t *g_sai_switch_info_table = NULL;

/*
 * Switch State change Notification function
 */
static sai_switch_state_change_notification_fn sai_switch_state_notf_fn = NULL;

static std_mutex_lock_create_static_init_fast (switch_lock);

/* Default Native Hash fields */
static sai_int32_t sai_default_hash_fields [SAI_SWITCH_DEFAULT_HASH_FIELDS_COUNT] =
{ SAI_NATIVE_HASH_FIELD_SRC_MAC, SAI_NATIVE_HASH_FIELD_DST_MAC,
  SAI_NATIVE_HASH_FIELD_IN_PORT, SAI_NATIVE_HASH_FIELD_ETHERTYPE };

void sai_switch_lock (void)
{
    std_mutex_lock (&switch_lock);
}

void sai_switch_unlock (void)
{
    std_mutex_unlock (&switch_lock);
}

/* Allocate memory for switch info table */
sai_switch_info_t *sai_switch_info_alloc(void)
{
    SAI_SWITCH_LOG_TRACE("Switch info table initialization");

    g_sai_switch_info_table = (sai_switch_info_t *)calloc(1,
                                                          (sizeof(sai_switch_info_t)));
    STD_ASSERT(!(g_sai_switch_info_table == NULL));

    return g_sai_switch_info_table;
}

/* Assumption is switch info will be allocated during Init,
 * and it can be accessed without failure checks
 */
sai_switch_info_t *sai_switch_info_get(void)
{
    return (g_sai_switch_info_table);
}

/* Initialize switch info table and fill Switch Information based
 * on the Initialization configuration */
void sai_switch_info_initialize(const sai_switch_init_config_t *switch_info)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    STD_ASSERT(sai_switch_info_ptr != NULL);

    SAI_SWITCH_LOG_TRACE("Updates the switch info table based on init configuration");

    sai_switch_info_ptr->switch_supported_capb = switch_info->supported_capb;
    sai_switch_info_ptr->switch_op_state = SAI_SWITCH_OPER_STATUS_UNKNOWN;
    sai_switch_info_ptr->switch_cpu_port = switch_info->cpu_port;
    sai_switch_info_ptr->counter_refresh_interval = SAI_DEFAULT_COUNTER_REFRESH_INTERVAL;
    sai_switch_info_ptr->switch_lport_max = switch_info->max_logical_ports;
    sai_switch_info_ptr->switch_pport_max = switch_info->max_physical_ports;
    sai_switch_info_ptr->max_lane_per_port = switch_info->max_lane_per_port;
    sai_switch_info_ptr->max_port_mtu = switch_info->max_port_mtu;

    sai_switch_info_ptr->max_virtual_routers = SAI_FIB_MAX_VIRTUAL_ROUTERS;
    sai_switch_info_ptr->max_ecmp_paths = SAI_FIB_DEFAULT_MAX_ECMP_PATHS;

    sai_switch_info_ptr->max_childs_per_hierarchy_node =
        switch_info->max_childs_per_hierarchy_node;
    sai_switch_info_ptr->hierarchy_fixed = switch_info->hierarchy_fixed;
    sai_switch_info_ptr->max_hierarchy_levels = switch_info->max_hierarchy_levels;
    sai_switch_info_ptr->max_supported_tc = switch_info->max_supported_tc;
    sai_switch_info_ptr->max_queues_per_port = switch_info->max_queues_per_port;
    sai_switch_info_ptr->max_uc_queues_per_port = switch_info->max_uc_queues_per_port;
    sai_switch_info_ptr->max_mc_queues_per_port = switch_info->max_mc_queues_per_port;
    sai_switch_info_ptr->max_queues_per_cpu_port = switch_info->max_queues_per_cpu_port;
    sai_switch_info_ptr->max_buffer_size = switch_info->max_buffer_size;
    sai_switch_info_ptr->num_pg = switch_info->num_pg;
    sai_switch_info_ptr->cell_size = switch_info->cell_size;
    sai_switch_info_ptr->ing_max_buf_pools = switch_info->ing_max_buf_pools;
    sai_switch_info_ptr->egr_max_buf_pools = switch_info->egr_max_buf_pools;
    sai_switch_info_ptr->tiles_per_buf_pool = switch_info->tiles_per_buf_pool;
    sai_switch_info_ptr->max_tile_buffer_size = switch_info->max_tile_buffer_size;
    sai_switch_info_ptr->max_tiles = switch_info->max_tiles;

    sai_switch_info_ptr->max_uc_queues_per_cpu_port =
        switch_info->max_uc_queues_per_cpu_port;
    sai_switch_info_ptr->max_mc_queues_per_cpu_port =
        switch_info->max_mc_queues_per_cpu_port;

    sai_switch_info_ptr->l2_table_size = switch_info->l2_table_size;
    sai_switch_info_ptr->l3_host_table_size = switch_info->l3_host_table_size;
    sai_switch_info_ptr->l3_route_table_size = switch_info->l3_route_table_size;

    sai_switch_info_ptr->max_mac_learn_limit = 0;
    sai_switch_info_ptr->fdb_aging_time = 0;
    sai_switch_info_ptr->fdbUcastMissPktAction = SAI_PACKET_ACTION_FORWARD;
    sai_switch_info_ptr->fdbBcastMissPktAction = SAI_PACKET_ACTION_FORWARD;
    sai_switch_info_ptr->fdbMcastMissPktAction = SAI_PACKET_ACTION_FORWARD;
    sai_switch_info_ptr->isBcastCpuFloodEnable = false;
    sai_switch_info_ptr->isMcastCpuFloodEnable = false;

    sai_switch_info_ptr->ecmp_hash_algo = SAI_HASH_ALGORITHM_CRC;
    sai_switch_info_ptr->lag_hash_algo = SAI_HASH_ALGORITHM_CRC;
    sai_switch_info_ptr->ecmp_hash_seed = 0;
    sai_switch_info_ptr->lag_hash_seed = 0;
}

sai_switch_id_t sai_switch_id_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return sai_switch_info_ptr->switch_id;
}

sai_npu_port_id_t sai_switch_get_cpu_port(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->switch_cpu_port);
}

sai_object_id_t sai_switch_cpu_port_obj_id_get(void)
{
    /* Construct the cpu_port UOID with a  cpu port object id */
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    sai_npu_port_id_t cpu_port = sai_switch_info_ptr->switch_cpu_port;

    return sai_port_id_create(SAI_PORT_TYPE_CPU,
                              sai_switch_id_get(), cpu_port);
}

sai_npu_port_id_t sai_switch_get_max_lport(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->switch_lport_max);
}

sai_npu_port_id_t sai_switch_get_max_pport(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->switch_pport_max);
}

uint_t sai_switch_get_max_lane_per_port(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->max_lane_per_port);
}

uint_t sai_switch_get_max_port_mtu(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->max_port_mtu);
}

void sai_switch_counter_refresh_interval_set(uint_t cntr_interval)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    sai_switch_info_ptr->counter_refresh_interval = cntr_interval;
}

uint_t sai_switch_counter_refresh_interval_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->counter_refresh_interval);
}

sai_status_t sai_switch_oper_status_get(sai_switch_oper_status_t *operstate)
{
    STD_ASSERT(operstate != NULL);
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    *operstate = sai_switch_info_ptr->switch_op_state;

    SAI_SWITCH_LOG_TRACE("Switch get Operational status value %d", *operstate);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_switch_oper_status_set(sai_switch_oper_status_t operstate)
{
    sai_switch_state_change_notification_fn temp_notification_fn = NULL;
    sai_object_id_t switch_id =SAI_DEFAULT_SWITCH_ID;

    sai_switch_lock();
    temp_notification_fn = sai_switch_state_notf_fn;
    sai_switch_unlock();

    if(temp_notification_fn != NULL) {
        temp_notification_fn(switch_id,operstate);
    }

    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    if(sai_switch_info_ptr == NULL) {
        SAI_SWITCH_LOG_ERR("Switch info not initialized");
        return SAI_STATUS_UNINITIALIZED;
    }

    sai_switch_info_ptr->switch_op_state = operstate;

    SAI_SWITCH_LOG_TRACE("Switch set Operational status %d", operstate);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_switch_mac_address_get(sai_mac_t *mac)
{
    STD_ASSERT(mac != NULL);
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    memcpy(mac, sai_switch_info_ptr->switch_mac_addr,
           sizeof(sai_switch_info_ptr->switch_mac_addr));

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_switch_max_virtual_routers_get (uint_t *p_max_vrf)
{
    STD_ASSERT(p_max_vrf != NULL);
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    *p_max_vrf = sai_switch_info_ptr->max_virtual_routers;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_switch_max_ecmp_paths_get (uint_t *p_max_ecmp_paths)
{
    STD_ASSERT(p_max_ecmp_paths != NULL);
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    *p_max_ecmp_paths = sai_switch_info_ptr->max_ecmp_paths;

    return SAI_STATUS_SUCCESS;
}

/* Switch capabilities enable/disable API's */
/* Switch capability support API's*/
bool sai_is_switch_capb_supported(uint64_t capb_mask)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->switch_supported_capb & capb_mask) ? true : false;
}

void sai_switch_supported_capability_set(uint64_t capb_val)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    sai_switch_info_ptr->switch_supported_capb |= capb_val;
}

/* Switch capability Enable API's */
bool sai_is_switch_capb_enabled(uint64_t capb_mask)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->switch_enabled_capb & capb_mask) ? true : false;
}

void sai_switch_capablility_enable(bool enable, uint64_t capb_val)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    /* Enable only the supported capabilities */
    capb_val = capb_val & (sai_switch_info_ptr->switch_supported_capb);

    if(enable) {
        sai_switch_info_ptr->switch_enabled_capb |= capb_val;
    } else {
        sai_switch_info_ptr->switch_enabled_capb &= ~capb_val;
    }
}

/* Switch State notification registration callback
 * Null input will unregister from callback */
void sai_switch_state_register_callback(sai_switch_state_change_notification_fn state_notification_fn)
{
    SAI_SWITCH_LOG_INFO("Switch state change notification registration");

    sai_switch_lock();
    sai_switch_state_notf_fn = state_notification_fn;
    sai_switch_unlock();
}

/* Get the ecmp hash seed value from switch db */
sai_status_t sai_switch_ecmp_hash_seed_value_get(sai_switch_hash_seed_t
                                                *ecmp_hash_seed_value)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    STD_ASSERT(sai_switch_info_ptr != NULL);
    STD_ASSERT(ecmp_hash_seed_value != NULL);

    *ecmp_hash_seed_value = sai_switch_info_ptr->ecmp_hash_seed;
    return SAI_STATUS_SUCCESS;
}

/* Get the lag hash seed value from switch db */
sai_status_t sai_switch_lag_hash_seed_value_get(sai_switch_hash_seed_t
                                                *lag_hash_seed_value)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    STD_ASSERT(sai_switch_info_ptr != NULL);
    STD_ASSERT(lag_hash_seed_value != NULL);

    *lag_hash_seed_value = sai_switch_info_ptr->lag_hash_seed;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_switch_default_native_hash_fields_get (
                                                 sai_s32_list_t *field_list)
{
    STD_ASSERT(field_list != NULL);
    STD_ASSERT(field_list->list != NULL);

    if (field_list->count < SAI_SWITCH_DEFAULT_HASH_FIELDS_COUNT) {
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    memcpy (field_list->list, sai_default_hash_fields,
            sizeof(sai_default_hash_fields));

    return SAI_STATUS_SUCCESS;
}
