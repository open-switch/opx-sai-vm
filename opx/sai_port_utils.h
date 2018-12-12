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
* @file sai_port_utils.h
*
* @brief This file contains SAI Common Port Utility API signatures.
*  Common Port utility API's can be used by other SAI components.
*
*  SAI Port Id used in the below APIs refers to the actual NPU switch ports
*  and doesn't include virtual ports like LAG and Tunnel ports and not
*  even CPU port. CPU port info is available as part of switch information.
*
*  Below API's are used to get the switch port related information like
*  port capabilities such as Breakout mode, port speed, PHY device type.
*
*************************************************************************/

/* OPENSOURCELICENSE */

#ifndef __SAI_PORT_UTILS_H__
#define __SAI_PORT_UTILS_H__

#include "saitypes.h"
#include "saiswitch.h"
#include "saiport.h"
#include "saistatus.h"

#include "std_type_defs.h"
#include "std_rbtree.h"
#include "sai_switch_common.h"
#include "sai_port_common.h"
#include "sai_switch_utils.h"

/** \defgroup SAIPORTAPI SAI - Port Utility
 *  Common Utility functions for SAI Port component
 *
 *  \ingroup SAIAPI
 *  \{
 */

/** Logging utility for SAI Virtual Router API */
#define SAI_PORT_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_PORT, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_PORT, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI Virtual Router API */
#define SAI_PORT_LOG_TRACE(msg, ...) \
        SAI_PORT_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_PORT_LOG_CRIT(msg, ...) \
        SAI_PORT_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_PORT_LOG_ERR(msg, ...) \
        SAI_PORT_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_PORT_LOG_INFO(msg, ...) \
        SAI_PORT_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_PORT_LOG_WARN(msg, ...) \
        SAI_PORT_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_PORT_LOG_NTC(msg, ...) \
        SAI_PORT_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/**
 * @brief Initialize the Port information table.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_info_init(void);

/**
 * @brief Initialize the port attributes to its default value
 */
void sai_port_attr_defaults_init(void);

/**
 * @brief Get the entire port information table.
 *
 * @return pointer to global port information table.
 */
sai_port_info_table_t sai_port_info_table_get(void);

/**
 * @brief Get the port info for a given switch port number
 *
 * @param[in] port  switch port id to index the port table
 * @return pointer to port info for the given port id
 */
sai_port_info_t *sai_port_info_get(sai_object_id_t port);

/**
 * @brief Get the first node in the port info table
 *
 * @return pointer to first port info node in the table
 */
static inline sai_port_info_t *sai_port_info_getfirst(void)
{
    return ((sai_port_info_t *)std_rbtree_getfirst(sai_port_info_table_get()));
}

/**
 * @brief Get the next node in port info table for the given port info node
 *
 * @param[in] port_info  current port info node
 * @return pointer to next port info node in the table
 */
static inline sai_port_info_t *sai_port_info_getnext(sai_port_info_t *port_info)
{
    return (sai_port_info_t *)std_rbtree_getnext(sai_port_info_table_get(), port_info);
}

/**
 * @brief Check if a given switch port is valid
 *
 * @param[in] port  switch port id to be validated
 * @return Success - true if valid port
 *         Failure - false if not a valid port
 */
bool sai_is_port_valid(sai_object_id_t port);

/**
 * @brief Get all the port attributes info for a given logical or CPU port
 *
 * @param[in] port  sai switch port number
 * @param[in] port_info  Pointer to  SAI port info structure
 * @return pointer to port attribute info corresponding to a given
 *  valid port id else NULL for an invalid port
 */
sai_port_attr_info_t *sai_port_attr_info_get_for_update(sai_object_id_t port,
                                                        sai_port_info_t *port_info);

/**
 * @brief Get all the port attributes info for a given logical or CPU port as const pointer
 *
 * @param[in] port  sai switch port number
 * @param[in] port_info  Pointer to  SAI port info structure
 * @return pointer to port attribute info corresponding to a given
 *  valid port id else NULL for an invalid port
 */
const sai_port_attr_info_t *sai_port_attr_info_read_only_get(sai_object_id_t port,
                                                            const sai_port_info_t *port_info_table);
/**
 * @brief Update a specific port attribute info for a given logical port
 *
 * @param[in] port  sai switch port number
 * @param[in] port_info  Pointer to  SAI port info structure
 * @param[in] attr pointer to the port attribute with attr id and value
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_attr_info_cache_set(sai_object_id_t port_id,
                                          sai_port_info_t *port_info,
                                          const sai_attribute_t *attr);

/**
 * @brief Get a specific port attribute info for a given logical port
 *
 * @param[in] port  sai switch port number
 * @param[in] port_info  Pointer to  SAI port info structure
 * @param[inout] attr pointer to the port attribute with attr id as input and
 *  value as output
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_attr_info_cache_get(sai_object_id_t port_id,
                                          const sai_port_info_t *port_info,
                                          sai_attribute_t *attr);


/**
 * @brief Get the port group of a given switch port.
 *  In a multi-lane port, all its lanes are part of the same port group.
 *  In a single-lane port, each port is part of individual port group.
 *
 * @param[in] port  sai switch port number
 * @param[out] port_group port group it is part of
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_port_group_get(sai_object_id_t port, uint_t *port_group);

/**
 * @brief Get the local/logical port for the given sai switch port;
 * local/Logical port is used to index the vendor SDK APIs
 *
 * @param[in] port  sai switch port number
 * @param[out] local_port_id  local port id for the given sai switch port id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_to_npu_local_port(sai_object_id_t port,
                                        sai_npu_port_id_t *local_port_id);

/**
 * @brief Get the switch port for the given local/logical port;
 * Local/Logical port is used to index the vendor SDK APIs
 *
 * @param[in] local_port_id  local port id for the given sai switch port id
 * @param[out] port  sai switch port number
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_npu_local_port_to_sai_port(sai_npu_port_id_t local_port_id,
                                            sai_object_id_t *port);

/**
 * @brief Check if a given port capability is supported.
 *
 * @param[in] sai_port_info  Pointer to SAI port info structure
 * @param[in] capb_mask  capability bit mask in sai_port_capability_t
 * @param[out] value  true or false
 * @sa sai_port_capability_t
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
static inline bool sai_is_port_capb_supported(const sai_port_info_t *sai_port_info,
                                              uint64_t capb_mask)
{
    if(sai_port_info->port_supported_capb & capb_mask) {
        return true;
    }
    return false;
}

/**
 * @brief Check if a given port capability is enabled.
 *
 * @param[in] port  sai switch port number
 * @param[in] capb_mask  capability bit mask in sai_port_capability_t
 * @param[out] value  true or false
 * @sa sai_port_capability_t
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_is_port_capb_enabled(sai_object_id_t port,
                                      uint64_t capb_mask, bool *value);

/**
 * @brief Check if a given breakout type is enabled in the port
 *
 * @param[in] port  sai switch port number
 * @param[in] breakout_type  breakout mode types in sai_port_capability_t
 * @param[out] value  true or false
 * @sa sai_port_capability_t
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
static inline bool sai_port_is_breakout_enabled(const sai_port_info_t *port_info)
{
    return (port_info->port_enabled_capb & SAI_PORT_CAP_BREAKOUT_MODE);
}

/**
 * @brief Set the port forwarding mode
 *
 * @param[in] port  sai switch port number
 * @param[in] fwd_mode  port forwarding mode - switching or routing
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_forwarding_mode_set(sai_object_id_t port,
                                          sai_port_fwd_mode_t fwd_mode);

/**
 * @brief Get the port forwarding mode
 *
 * @param[in] port  sai switch port number
 * @param[out] fwd_mode  port forwarding mode - switching or routing
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_forwarding_mode_get(sai_object_id_t port,
                                          sai_port_fwd_mode_t *fwd_mode);

/**
 * @brief Get the string for the port forwarding mode
 *
 * @param[in] fwd_mode port forwarding mode - unknown or switching or routing
 * @return String for the specified port forwarding mode.
 */
const char *sai_port_forwarding_mode_to_str (sai_port_fwd_mode_t fwd_mode);

/**
 * @brief Mutex lock port for access
 */
void sai_port_lock(void);

/**
 * @brief Mutex unlock port after access
 */
void sai_port_unlock(void);

/**
 * @brief Retrieve/Create the port node for the applications running on the port
 *
 * @param[in] port_id  sai switch port number
 * @return port node in the port_applications_tree if create/get successful otherwise NULL
 */
sai_port_application_info_t* sai_port_application_info_create_and_get (sai_object_id_t port_id);

/**
 * @brief Checks all the application running on the port and removes the node
 *
 * @param[in] p_port_node Node to be removed
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_application_info_remove (sai_port_application_info_t *p_port_node);

/**
 * @brief Retrieve the port node for the applications running on the port
 *
 * @param[in] port_id  sai switch port number
 * @return port node in the port_applications_tree if get successful otherwise NULL
 */
sai_port_application_info_t* sai_port_application_info_get (sai_object_id_t port_id);

/**
 * @brief Get first application port node from tree.
 *
 * @return Pointer to the first application port node.
 */
sai_port_application_info_t *sai_port_first_application_node_get (void);

/**
 * @brief Get next application port node from tree.
 *
 * @param[in] p_port_node   Pointer to the Qos port node
 * @return Pointer to the next port node in tree.
 */
sai_port_application_info_t *sai_port_next_application_node_get (
                                          sai_port_application_info_t *p_port_node);

/**
 * @brief Get breakout mode port capability value from breakout mode
 *
 * @param[in] mode breakout mode type
 * @return breakout mode port capability of type sai_port_capability_t
 */
static inline sai_port_capability_t sai_port_capb_from_break_mode(sai_port_breakout_mode_type_t mode)
{
    if(mode == SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE) {
        return SAI_PORT_CAP_BREAKOUT_MODE_2X;

    } else if(mode == SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE) {
        return SAI_PORT_CAP_BREAKOUT_MODE_4X;
    }

    return SAI_PORT_CAP_BREAKOUT_MODE_1X;
}

/**
 * @brief Get breakout mode from breakout mode port capability value
 *
 * @param[in] capb port capability
 * @return breakout mode of type sai_port_breakout_mode_type_t
 */
static inline sai_port_breakout_mode_type_t sai_port_break_mode_from_capb(sai_port_capability_t capb)
{
    if(capb == SAI_PORT_CAP_BREAKOUT_MODE_2X) {
        return SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE;

    } else if(capb == SAI_PORT_CAP_BREAKOUT_MODE_4X) {
        return SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE;
    }

    return SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE;
}

/**
 * @brief Get the port lane count needed for a specific breakout mode
 *
 * @param[in] mode breakout mode type
 * @return port lane count corresponding to the breakout mode
 */
static inline sai_port_lane_count_t sai_port_breakout_lane_count_get(sai_port_breakout_mode_type_t mode)
{
    if(mode == SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE) {
        return SAI_PORT_LANE_COUNT_ONE;

    } else if(mode == SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE) {
        return SAI_PORT_LANE_COUNT_TWO;
    }

    return SAI_PORT_LANE_COUNT_FOUR;
}

/**
 * @brief Get the port count for a specific breakout mode
 *
 * @param[in] mode breakout mode type
 * @return port lane count corresponding to the breakout mode
 */
static inline uint_t sai_port_breakout_port_count_get(sai_port_breakout_mode_type_t mode)
{
    if(mode == SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE) {
        return 4;

    } else if(mode == SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE) {
        return 2;
    }

    return 1;
}

static inline sai_port_breakout_mode_type_t sai_port_get_breakout_mode_from_port_count (uint_t count)
{

    switch (count) {
        case 4:
            return SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE;

        case 2:
            return SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE;

        case 1:
            return SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE;

        default:
            return SAI_PORT_BREAKOUT_MODE_TYPE_MAX;
    }
    /* Wont reach here. For compilation and avoiding coverity issues*/
    return SAI_PORT_BREAKOUT_MODE_TYPE_MAX;
}
/**
 * @brief Get the HW lane list for a given SAI logical port.
 * CPU port is not supported by thie API.
 *
 * @param[in] port_id  sai switch port number
 * @param[in] sai_port_info Pointer to SAI port info structure
 * @param[inout] list of hardware lanes for the port
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_attr_hw_lane_list_get(sai_object_id_t port_id,
                                            const sai_port_info_t *sai_port_info,
                                            sai_attribute_value_t *value);

/**
 * @brief Get the supported breakout mode(s) for a given SAI logical port.
 * CPU port is not supported by thie API.
 *
 * @param[in] port_id  sai switch port number
 * @param[in] sai_port_info Pointer to SAI port info structure
 * @param[inout] value list of supported breakout mode(s) for the port
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_attr_supported_breakout_mode_get(sai_object_id_t port_id,
                                                       const sai_port_info_t *sai_port_info,
                                                       sai_attribute_value_t *value);

/**
 * @brief Get the current breakout mode for a given SAI logical port.
 * CPU port is not supported by thie API.
 *
 * @param[in] port_info Pointer to SAI port info structure
 * @return Current breakout mode of the port
 */
sai_port_breakout_mode_type_t sai_port_current_breakout_mode_get(const sai_port_info_t *port_info);

/**
 * @brief Updates the port info before applying breakout mode
 *
 * @param[in] port_info  Pointer to SAI port info structure
 * @param[in] breakout_cfg Structure containing breakout config
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_breakout_mode_update (sai_port_info_t * const port_info,
                                            sai_port_breakout_config_t  *breakout_cfg);


/**
 * @brief Get the list of  SAI logical port and it doesn't include CPU port
 *
 * @param[out] port_list port list to be filled. Memory allocated by caller.
 */
void sai_port_logical_list_get(sai_object_list_t *port_list);

/**
 * @brief Get the port type for a given sai port
 *
 * @param[in] port_id  sai switch port number - can be Logical or CPU port
 * @param[in] sai_port_info  SAI port info structure
 * @param[out] value  pointer to get the port type
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_attr_type_get(sai_object_id_t port_id,
                                    const sai_port_info_t *sai_port_info,
                                    sai_attribute_value_t *value);

/**
 * @brief Update the supported speed values for the given sai port
 *
 * @param[in] speed_capb Supported speed bitmap
 * @param[in] sai_port_info  SAI port info structure
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_attr_supported_speed_update(sai_port_info_t * const sai_port_info,
                                                  uint_t speed_capb);

/**
 * @brief Initialize the port attribute values to default ones
 *
 * @param[inout] port_attr_info Pointer to port attribute information
 */
void sai_port_attr_info_defaults_init(sai_port_attr_info_t *port_attr_info);


/**
 * @brief Check whether a port is oper up
 *
 * @param[in] port_obj SAI Port object identifier
   @return true if port is oper up. false otherwise
 */
bool sai_port_is_oper_up(sai_object_id_t port_obj);
/**
 * \}
 */


/**
 * @brief Get port info from NPU Physical port id
 *
 * @param[in] phy_port_id NPU Physical port identifier
 * @return Pointer to port info structure if found. NULL if not found
 *
 */
sai_port_info_t *sai_port_info_get_from_npu_phy_port(sai_npu_port_id_t phy_port_id);

/**
 * @brief Update validity on create. Mark the create port as valid and rest as invalid
 *        Other ports validity will be updated on their respective create
 *
 * @param[in] create_port The port used in create API
 * @param[in] sai_port_info The port info for 1st port in the port group
 * @param[in] new_mode The new breakout mode applied
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_port_update_valdity_on_create (sai_object_id_t create_port,
                                                sai_port_info_t *sai_control_port_info,
                                                sai_port_breakout_mode_type_t new_mode);

/**
 * @brief Check whether if breakout mode is supported
 *
 * @param[in] sai_port_info Pointer to SAI port info structure
 * @param[in] breakout_mode Port breakout mode
 * @return true if supported
 *         false if not supported
 *
 */
bool sai_port_is_breakout_mode_supported (const sai_port_info_t *sai_port_info,
                                          sai_port_breakout_mode_type_t breakout_mode);


/**
 * @brief Set the default 1Q bridge port associated for the port in port cache
 *
 * @param[in] port_id SAI Port identifier
 * @param[in] bridge_port_id Bridge port identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_port_def_bridge_port_set (sai_object_id_t port, sai_object_id_t bridge_port_id);

/**
 * @brief Get the default 1Q bridge port associated for the port from port cache
 *
 * @param[in] port_id SAI Port identifier
 * @param[out] bridge_port_id Bridge port identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_port_def_bridge_port_get (sai_object_id_t port, sai_object_id_t *bridge_port_id);

/**
 * @brief Set the LAG associated for the port in port cache
 *
 * @param[in] port_id SAI Port identifier
 * @param[in] lag_id LAG identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_port_lag_set (sai_object_id_t port, sai_object_id_t lag_id);

/**
 * @brief Get LAG associated for the port from port cache
 *
 * @param[in] port_id SAI Port identifier
 * @param[out] lag_id LAG identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_port_lag_get (sai_object_id_t port, sai_object_id_t *lag_id);

/**
 * @brief Increment the port reference count
 *
 * @param[in] port_id SAI Port identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_port_increment_ref_count (sai_object_id_t port);

/**
 * @brief Decrement the port reference count
 *
 * @param[in] port_id SAI Port identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_port_decrement_ref_count (sai_object_id_t port);

/**
 * @brief Check if the port is in use
 *
 * @param[in] port_id SAI Port identifier
 * @return true if reference count greater than zero, false otherwise
 *
 */
bool sai_is_port_in_use (sai_object_id_t port);

/**
 * @brief Increment the vlan count on the port
 *
 * @param[in] port_id SAI Port identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_port_increment_vlan_count (sai_object_id_t port);

/**
 * @brief Decrement the vlan count on the port
 *
 * @param[in] port_id SAI Port identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_port_decrement_vlan_count (sai_object_id_t port);

/**
 * @brief Get the vlan count on the port
 *
 * @param[in] port_id SAI Port identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
uint_t sai_port_get_vlan_count (sai_object_id_t port);

/**
 * @brief Check the attribute value applied on port is duplicate
 *
 * @param[in] port_id SAI Port identifier
 * @param[in] port_info A pointer to port info structure for the port
 * @param[in] attr Attribute to be checked
 * @return true if attribute contains duplicate value, false otherwise
 *
 */
bool sai_port_is_duplicate_attribute_val (sai_object_id_t port_id,
                                          const sai_port_info_t *port_info,
                                          const sai_attribute_t *attr);


/**
 * @brief Get PVID of the port
 *
 * @param[in] port_id SAI Port identifier
 * @param[out] pvid Port's Port VLAN ID
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_port_pvid_get (sai_object_id_t port, uint16_t *pvid);

#endif /* __SAI_PORT_UTILS_H__ */
