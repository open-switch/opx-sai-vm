/************************************************************************
* LEGALESE:   "Copyright (c); 2015, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file sai_switch_utils.h
*
* @brief This file contains SAI Common Switch Utility API Signatures.
*  Common Switch utility APIs can be used by other SAI components
*  It includes utility APIs to get switch information like switch physical Id,
*  switch Mac address, max logical port, CPU connected port of the switch, etc.
*
*************************************************************************/

/* OPENSOURCELICENSE */

#ifndef __SAI_SWITCH_UTILS_H__
#define __SAI_SWITCH_UTILS_H__

#include "saitypes.h"
#include "saiswitch.h"
#include "saiport.h"
#include "saistatus.h"

#include "sai_event_log.h"
#include "sai_switch_common.h"

/** \defgroup SAISWITCHAPI SAI - Switch and infra implementation
 *   Switch and Infra APIs for SAI. APIS are to be used by SAI
 *   only and not the upper layers.
 *
 *  \ingroup SAIAPI
 *  \{
 */

/** Logging utility for SAI Virtual Router API */
#define SAI_SWITCH_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_SWITCH, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_SWITCH, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI Virtual Router API */
#define SAI_SWITCH_LOG_TRACE(msg, ...) \
        SAI_SWITCH_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_SWITCH_LOG_CRIT(msg, ...) \
        SAI_SWITCH_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_SWITCH_LOG_ERR(msg, ...) \
        SAI_SWITCH_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_SWITCH_LOG_INFO(msg, ...) \
        SAI_SWITCH_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_SWITCH_LOG_WARN(msg, ...) \
        SAI_SWITCH_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_SWITCH_LOG_NTC(msg, ...) \
        SAI_SWITCH_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/** Number of default switch hash fields */
#define SAI_SWITCH_DEFAULT_HASH_FIELDS_COUNT (4)

/** Default switch ID init **/
#define SAI_DEFAULT_SWITCH_ID 0

/**
 * @brief Initializes the switch info table and updates the switch info based on
 *  switch initialization configuration.
 *
 * @param[in] switch_id switch instance id
 * @param[in] pointer to switch initialization config
 *
 */
void sai_switch_info_initialize(const sai_switch_init_config_t *switch_info);

/**
 * @brief Allocate memory for switch info table
 *
 * @return pointer to allocated global switch info table memory
 */
sai_switch_info_t *sai_switch_info_alloc(void);

/**
 * @brief Get the switch info table.
 *
 * @return pointer to global switch info table.
 */
sai_switch_info_t *sai_switch_info_get(void);

/**
 * @brief Get the physical NPU switch identifier.
 *
 * @return Physical id of the NPU
 */
sai_switch_id_t sai_switch_id_get(void);

/**
 * @brief Get the current operational state of the switch
 *
 * @param[out] operstate  Operational state of the switch
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_switch_oper_status_get(sai_switch_oper_status_t *operstate);

/**
 * @brief Set the current operational state of the switch
 *
 * @param[in] operstate  Operational state of the switch
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_switch_oper_status_set(sai_switch_oper_status_t operstate);

/**
 * @brief Get the mac address of the switch
 *
 * @param[out] mac  Mac address value of type sai_mac_t
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_switch_mac_address_get(sai_mac_t *mac);

/**
 * @brief Get the Maximum virtual routers configurable in the switch.
 *
 * @param[out] p_max_vrf pointer to maximum number of virtual routers.
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_switch_max_virtual_routers_get (uint_t *p_max_vrf);

/**
 * @brief Get the Maximum ECMP Paths configured in the switch.
 *
 * @param[out] p_max_ecmp_paths pointer to maximum ECMP paths.
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_switch_max_ecmp_paths_get (uint_t *p_max_ecmp_paths);

/**
 * @brief Check if a given switch capability bit mask is supported.
 *
 * @param[in] capb_mask capability bit mask in sai_port_capability_t
 * @sa sai_switch_capability_t
 * @return Success - true if supported
 *         Failure - false if not supported
 */
bool sai_is_switch_capb_supported(uint64_t capb_mask);

/**
 * @brief Set the given switch capability supported bit mask
 *
 * @param[in] capb_val capability bit mask in sai_port_capability_t
 * @sa sai_switch_capability_t
 */
void sai_switch_supported_capability_set(uint64_t capb_val);

/**
 * @brief Check if Switch Bandwidth Over subscription mode is supported
 *
 * @return Success - true if supported
 *         Failure - false if not supported
 */
static inline bool sai_switch_oversubscription_supported(void)
{
    return sai_is_switch_capb_supported(SAI_SWITCH_CAP_OVER_SUBSCRIPTION_MODE);
}

/**
 * @brief Check if unified forwarding table is supported
 *
 * @return Success - true if supported
 *         Failure - false if not supported
 */
static inline bool sai_switch_uft_supported(void)
{
    return sai_is_switch_capb_supported(SAI_SWITCH_CAP_UNIFIED_FORWARDING_TABLE);
}

/**
 * @brief Check if cut through packet switching mode is supported
 *
 * @return Success - true if supported
 *         Failure - false if not supported
 */
static inline bool sai_switch_cut_through_supported(void)
{
    return sai_is_switch_capb_supported(SAI_SWITCH_CAP_CUT_THROUGH_MODE);
}

/**
 * @brief Check if store and forward packet switching mode is supported
 *
 * @return Success - true if supported
 *         Failure - false if not supported
 */
static inline bool sai_switch_store_and_forward_supported(void)
{
    return sai_is_switch_capb_supported(SAI_SWITCH_CAP_STORE_AND_FORWARD_MODE);
}

/**
 * @brief Check if a given switch capability is enabled.
 *
 * @param[in] capb_mask capability bit mask in sai_port_capability_t
 * @sa sai_switch_capability_t
 * @return Success - true if enabled
 *         Failure - false if not enabled
 */
bool sai_is_switch_capb_enabled(uint64_t capb_mask);

/**
 * @brief Enable the given switch capabilities flags
 *
 * @param[in] enable flag to enable or disable the capb_val
 * @param[in] capb_val capability bit mask in sai_switch_capability_t
 * @sa sai_switch_capability_t
 */
void sai_switch_capablility_enable(bool enable, uint64_t capb_val);

/**
 * @brief Check if Switch Bandwidth Over subscription mode is enabled
 *
 * @return Success - true if enabled
 *         Failure - false if not enabled
 */
static inline bool sai_switch_oversubscription_enabled(void)
{
    return sai_is_switch_capb_enabled(SAI_SWITCH_CAP_OVER_SUBSCRIPTION_MODE);
}

/**
 * @brief Check if unified forwarding table is enabled
 *
 * @return Success - true if enabled
 *         Failure - false if not enabled
 */
static inline bool sai_switch_uft_enabled(void)
{
    return sai_is_switch_capb_enabled(SAI_SWITCH_CAP_UNIFIED_FORWARDING_TABLE);
}

/**
 * @brief Check if cut through packet switching mode is enabled
 *
 * @return Success - true if enabled
 *         Failure - false if not enabled
 */
static inline bool sai_switch_cut_through_enabled(void)
{
    return sai_is_switch_capb_enabled(SAI_SWITCH_CAP_CUT_THROUGH_MODE);
}

/**
 * @brief Check if store and forward packet switching mode is enabled
 *
 * @return Success - true if enabled
 *         Failure - false if not enabled
 */
static inline bool sai_switch_store_and_forward_enabled(void)
{
    return sai_is_switch_capb_enabled(SAI_SWITCH_CAP_STORE_AND_FORWARD_MODE);
}

/**
 * @brief Get the port in the switch that is connected to the CPU
 *
 * @return CPU connected port of the switch
 */
sai_npu_port_id_t sai_switch_get_cpu_port(void);

/**
 * @brief Get the switch attribute for CPU Port Object Id
 *
 * @return CPU Port's Object Id in the switch
 */
sai_object_id_t sai_switch_cpu_port_obj_id_get (void);

/**
 * @brief Get maximum number of logical switch ports in the switch. It includes
 *  the logical sub ports that are part of breakout capable physical ports.
 *
 * @return Maximum logical switch ports count in the switch
 */
sai_npu_port_id_t sai_switch_get_max_lport(void);

/**
 * @brief Get the maximum port mtu supported by the switch
 *
 * @return Maximum port mtu
 */
uint32_t sai_switch_get_max_port_mtu(void);

/**
 * @brief Get the maximum physical number of switch ports in the switch
 *
 * @return Maximum physical ports count in the switch
 */
sai_npu_port_id_t sai_switch_get_max_pport(void);

/**
 * @brief Get the maximum SerDes lanes available per port in the switch
 *
 * @return Maximum SerDes lanes available in a port
 */
uint_t sai_switch_get_max_lane_per_port(void);

/**
 * @brief Set the SW Statistics counter refresh interval in seconds
 *
 * @param[in] cntr_interval counter interval in seconds and value Zero
 * is used for setting HW based statistics counter
 */
void sai_switch_counter_refresh_interval_set(uint_t cntr_interval);

/**
 * @brief Get the Statistics counter refresh interval in seconds
 *
 * @return get the counter interval in seconds with zero being HW counter
 */
uint_t sai_switch_counter_refresh_interval_get(void);

/**
 * @brief Register with call back function for switch operational state
 *        change notifications
 *
 * @param[in] state_notification_fn Switch operational state notification function;
 *            use NULL as input to unregister from the callback notifications
 * @warning Calling this API for the second time will overwrite the existing
 *          registered function
 */
void sai_switch_state_register_callback(sai_switch_state_change_notification_fn state_notification_fn);

/**
 * @brief Get the ECMP hash seed value from switch info table.
 *
 * @param[out] ecmp_hash_seed_value ECMP hash seed value set in the switch
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_switch_ecmp_hash_seed_value_get(sai_switch_hash_seed_t
                                                 *ecmp_hash_seed_value);

/**
 * @brief Get the LAG hash seed value from switch info table.
 *
 * @param[out] lag_hash_seed_value LAG hash seed value set in the switch
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_switch_lag_hash_seed_value_get(sai_switch_hash_seed_t
                                                *lag_hash_seed_value);

/**
 * @brief Get the default Native Hash fields.
 *
 * @param[inout] field_list List filled with the native hash field types.
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_switch_default_native_hash_fields_get (
                                                 sai_s32_list_t *field_list);

/**
 *
 * @brief Utility to take simple mutex lock for switch resources access.
 */
void sai_switch_lock (void);

/**
 *@brief Utility to release simple mutex unlock for switch resources access.
 */
void sai_switch_unlock (void);

/**
 * @brief Get the maximum buffer size in the switch
 *
 * @return Maximum supported buffer size in the switch
 */
static inline uint_t sai_switch_max_buffer_size_get (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->max_buffer_size);
}

/**
 * @brief Get the number of priority groups supported
 *
 * @return Number of priority groups supported
 */
static inline uint_t sai_switch_num_pg_get (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->num_pg);
}

/**
 * @brief Get the size of a cell in MMU
 *
 * @return Size of a Cell in MMU
 */
static inline uint_t sai_switch_cell_size_get (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->cell_size);
}

/**
 * @brief Get the maximum number of service pools at the ingress
 *
 * @return Maximum number of service pools supported at ingress
 */
static inline uint_t sai_switch_ing_max_buf_pools_get (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->ing_max_buf_pools);
}

/**
 * @brief Get the maximum number of service pools at the egress
 *
 * @return Maximum number of service pools supported at egress
 */
static inline uint_t sai_switch_egr_max_buf_pools_get (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->egr_max_buf_pools);
}

/**
 * @brief Get the maximum number of tiles per buffer  pool
 *
 * @return Maximum number of tiles per buffer pool
 */
static inline uint_t sai_switch_max_tiles_per_buf_pool_get (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->tiles_per_buf_pool);
}

/**
 * @brief Get the maximum buffer size per tile in the switch
 *
 * @return Maximum supported buffer size per tile in the switch
 */
static inline uint_t sai_switch_max_tile_buffer_size_get (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->max_tile_buffer_size);
}

/**
 * @brief Get the maximum number of tiles in the switch
 *
 * @return Maximum number of tiles in the switch
 */
static inline uint_t sai_switch_max_tiles_get (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->max_tiles);
}

/**
 * @brief Check if tiles per buffer pool supported or not in the switch
 *
 * @return true if supported else false
 */
static inline bool sai_switch_is_tiles_per_buf_pool_supported (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return ((sai_switch_info_ptr->max_tile_buffer_size != 0) ? true : false);
}

/**
 * @brief Get the maximum traffic class supported per port in the switch
 *
 * @return Maximum supported tarffic class per port in switch
 */
static inline uint_t sai_switch_max_traffic_class_get (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->max_supported_tc);
}

/**
 * @brief Get the maximum unicast queues per port in the switch
 * @param[in] port_id  port UOID
 * @return Maximum unicast queues per port in switch
 */
static inline uint_t sai_switch_max_uc_queues_per_port_get (sai_object_id_t port_id)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    if(sai_is_obj_id_cpu_port(port_id)) {
        return (sai_switch_info_ptr->max_uc_queues_per_cpu_port);
    }
    return (sai_switch_info_ptr->max_uc_queues_per_port);
}

/**
 * @brief Get the maximum multicast queues per port in the switch
 * @param[in] port_id  port UOID
 * @return Maximum multicast queues per port in switch
 */
static inline uint_t sai_switch_max_mc_queues_per_port_get (sai_object_id_t port_id)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    if(sai_is_obj_id_cpu_port(port_id)) {
        return (sai_switch_info_ptr->max_mc_queues_per_cpu_port);
    }
    return (sai_switch_info_ptr->max_mc_queues_per_port);
}

/**
 * @brief Get the maximum queues per port in the switch
 * @param[in] port_id  port UOID
 * @return Maximum queues per port in switch
 */
static inline uint_t sai_switch_max_queues_per_port_get (sai_object_id_t port_id)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    if(sai_is_obj_id_cpu_port(port_id)) {
        return (sai_switch_info_ptr->max_queues_per_cpu_port);
    }

    return (sai_switch_info_ptr->max_queues_per_port);
}

/**
 * @brief Get the maximum hierarchy levels supported per port
 *        in the switch
 *
 * @return Maximum hierarchy levels per port in switch
 */
static inline uint_t sai_switch_max_hierarchy_levels_get (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->max_hierarchy_levels);
}

/**
 * @brief Get the leaf hierarchy level per port in the switch
 *
 * @return Leaf hierarchy levels per port in switch
 */
static inline uint_t sai_switch_leaf_hierarchy_level_get (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->max_hierarchy_levels - 1);
}
/**
 * @brief Get the maximum unicast queues in the switch
 * @return Maximum unicast queues per port in switch
 **/
static inline uint_t sai_switch_max_uc_queues_get (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->max_uc_queues_per_port);
}

/**
 * @brief Get the maximum multicast queues in the switch
 * @return Maximum multicast queues per port in switch
 **/
static inline uint_t sai_switch_max_mc_queues_get (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->max_mc_queues_per_port);
}

/**
 * @brief Get the maximum queues per port in the switch
 * @return Maximum queues per port in switch
 */
static inline uint_t sai_switch_max_queues_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->max_queues_per_port);
}

/**
 * @brief Get the maximum childs supported per hierarchy node in the switch
 *
 * @return Maximum childs per each hierarchy node in switch
 */
static inline uint_t sai_switch_max_hierarchy_node_childs_get (void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->max_childs_per_hierarchy_node);
}

/**
 * @brief Get the default TC in the switch
 *
 * @return Default TC used in the switch
 */
static inline uint_t sai_switch_default_tc_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    return (sai_switch_info_ptr->default_tc);
}

static inline uint_t sai_switch_fdb_table_size_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->l2_table_size);
}

static inline void sai_switch_fdb_table_size_set(uint_t fdb_table_size)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    sai_switch_info_ptr->l2_table_size = fdb_table_size;
}

static inline uint_t sai_switch_l3_route_table_size_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->l3_route_table_size);
}

static inline void sai_switch_l3_route_table_size_set(uint_t l3_route_table_size)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    sai_switch_info_ptr->l3_route_table_size = l3_route_table_size;
}

static inline uint_t sai_switch_l3_host_table_size_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->l3_host_table_size);
}

static inline void sai_switch_l3_host_table_size_set(uint_t l3_host_table_size)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    sai_switch_info_ptr->l3_host_table_size = l3_host_table_size;
}

static inline uint_t sai_switch_num_lag_members_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->num_lag_members);
}

static inline void sai_switch_num_lag_members_set(uint_t num_lag_members)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    sai_switch_info_ptr->num_lag_members = num_lag_members;
}

static inline uint_t sai_switch_num_lag_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->num_lag);
}

static inline void sai_switch_num_lag_set(uint_t num_lag)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    sai_switch_info_ptr->num_lag = num_lag;
}

static inline uint_t sai_switch_num_ecmp_members_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->max_ecmp_paths);
}

static inline void sai_switch_num_ecmp_members_set(uint_t num_ecmp_members)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    sai_switch_info_ptr->max_ecmp_paths = num_ecmp_members;
}

static inline uint_t sai_switch_num_ecmp_groups_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->num_ecmp_groups);
}

static inline void sai_switch_num_ecmp_groups_set(uint_t num_ecmp_groups)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    sai_switch_info_ptr->num_ecmp_groups = num_ecmp_groups;
}

static inline uint_t sai_switch_num_unicast_queues_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->max_uc_queues_per_port);
}

static inline void sai_switch_num_unicast_queues_set(uint_t num_unicast_queues)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    sai_switch_info_ptr->max_uc_queues_per_port = num_unicast_queues;
}

static inline uint_t sai_switch_num_multicast_queues_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->max_mc_queues_per_port);
}

static inline void sai_switch_num_multicast_queues_set(uint_t num_multicast_queues)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    sai_switch_info_ptr->max_mc_queues_per_port = num_multicast_queues;
}

static inline uint_t sai_switch_num_queues_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->max_queues_per_port);
}

static inline void sai_switch_num_queues_set(uint_t num_queues)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    sai_switch_info_ptr->max_queues_per_port = num_queues;
}

static inline uint_t sai_switch_num_cpu_queues_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->max_queues_per_cpu_port);
}

static inline void sai_switch_num_cpu_queues_set(uint_t num_cpu_queues)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    sai_switch_info_ptr->max_queues_per_cpu_port = num_cpu_queues;
}

/**
 * @brief Get the maximum number of native hash fields.
 * @return Maximum number of native hash fields defined in SAI.
 **/
static inline uint_t sai_switch_max_native_hash_fields (void)
{
    return (SAI_NATIVE_HASH_FIELD_IN_PORT + 1);
}

static inline void sai_switch_software_linkscan_interval_set(uint_t interval)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    sai_switch_info_ptr->sw_linkscan_interval = interval;
}

static inline uint_t sai_switch_software_linkscan_interval_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return (sai_switch_info_ptr->sw_linkscan_interval);
}

/**
 * \}
 */


#endif /* __SAI_SWITCH_UTILS_H__ */
