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
* @file sai_npu_port.h
*
* @brief This file contains API signatures for SAI NPU port component.
*        SAI Common use these API to Get/Set Port level Attributes and
*        to register for link state change notifications
*
*******************************************************************************/

/* OPENSOURCELICENSE */

#ifndef __SAI_NPU_PORT_H__
#define __SAI_NPU_PORT_H__

#include "saitypes.h"
#include "saistatus.h"
#include "saiport.h"

#include "std_type_defs.h"
#include "sai_port_common.h"
#include "sai_debug_utils.h"

/** \defgroup SAINPUPORTAPI SAI - NPU port Functionality
 *   Port functions for SAI NPU component
 *
 * \ingroup SAIPORTAPI
 * \{
 */

/**
 * @brief SAI Port create
 *
 * @param[out] port_id  Port Identifier
   @param[in] attr_count Number of attributes
 * @param[in] attr_list  Attribute id and value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_port_create_fn)(sai_object_id_t *port_id, uint32_t attr_count,
                                                      const sai_attribute_t *attr_list);
/**
 * @brief SAI Port Remove
 *
 * @param[in] port_id  Port Identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_port_remove_fn)(sai_object_id_t port_id);

/**
 * @brief Set port attribute
 *
 * @param[in] port_id  Port Identifier
 * @param[in] sai_port_info Pointer to SAI Port info structure
 * @param[in] attr  Attribute id and value
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_port_set_attribute_fn)(sai_object_id_t port_id,
                                                      sai_port_info_t *sai_port_info,
                                                      const sai_attribute_t *attr);

/**
 * @brief Get Port attributes
 *
 * @param[in] port_id  Port Identifier
 * @param[in] sai_port_info Pointer to SAI Port info structure
 * @param[in] attr_count  The number of attributes in the attribute array
 * @param[inout] attr_list The list of attributes in sai_attribute_t,
 *  where attr_id is the input and sai_attribute_value_t is the output value
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_port_get_attribute_fn)(sai_object_id_t port_id,
                                                      const sai_port_info_t *sai_port_info,
                                                      uint_t attr_count,
                                                      sai_attribute_t *attr_list);

/**
 * @brief Get statistics counters
 *
 * @param[in] port_id  Port Identifier
 * @param[in] sai_port_info Pointer to SAI Port info structure
 * @param[in] counter_ids Types of counter to enable
 * @param[in] number_of_counters  Number of counters to get
 * @param[out] counters  Values of the counters
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_port_get_stats_fn)(sai_object_id_t port_id,
                                                  const sai_port_info_t *sai_port_info,
                                                  const sai_port_stat_t *counter_ids,
                                                  uint32_t number_of_counters,
                                                  uint64_t* counters);

/**
 * @brief Clear statistics counters of the specified port
 *
 * @param[in] port_id Port Identifier
 * @param[in] sai_port_info Pointer to SAI Port info structure
 * @param[in] counter_ids Types of counter to clear
 * @param[in] number_of_counters Number of counters to clear
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_port_clear_stats_fn)(sai_object_id_t port_id,
                                                    const sai_port_info_t *sai_port_info,
                                                    const sai_port_stat_t *counter_ids,
                                                    uint32_t number_of_counters);

/**
 * @brief Clear all statistics counters of the specified port
 *
 * @param[in] port_id Port Identifier
 * @param[in] sai_port_info Pointer to SAI Port info structure
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_port_clear_all_stats_fn) (sai_object_id_t port_id,
                                                         const sai_port_info_t *sai_port_info);

/**
 * @brief Register Callback for link state notification from SAI to Adapter Host
 *
 * @param[in] link_state_cb_fn link state callback function; use NULL as input
 *            to unregister from the callback notifications
 * @warning Calling this API for the second time will overwrite the existing
 *          registered function
 */
typedef void (*sai_npu_reg_link_state_cb_fn)(
                             sai_port_state_change_notification_fn link_state_cb_fn);

/**
 * @brief Update the port packet switching mode based on the port event type ADD/DELETE and
 *  the port operating speed. As packet switching mode depends on port speed, it should be
 *  updated for events like ADD/DELETE during port breakout.
 *
 * @param[in] count Count of port events
 * @param[in] data pointer to list of port id and its event type Add/Delete
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_port_switching_mode_update_fn)(uint32_t count,
                                                              sai_port_event_notification_t *data);

/**
 * @brief Enable/Disable link scan on a port
 *
 * @param[in] port_id Object ID of the port for which link scan needs to be enabled
 * @param[in] sai_port_info Pointer to SAI Port info structure
 * @param[in] enable  boolean to enable or disable linkscan
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_port_linkscan_mode_set_fn) (sai_object_id_t port_id,
                                                           const sai_port_info_t *sai_port_info,
                                                           bool enable);

/*Debug Function for SAI BCM port */
typedef sai_status_t (*sai_npu_port_debug_fn) (sai_object_id_t port_id,sai_port_debug_function_t fn_name);
/**
 * @brief PORT NPU API table.
 */
typedef struct _sai_npu_port_api_t {
    sai_npu_port_create_fn                      npu_port_create;
    sai_npu_port_remove_fn                      npu_port_remove;
    sai_npu_port_set_attribute_fn               port_set_attribute;
    sai_npu_port_get_attribute_fn               port_get_attribute;
    sai_npu_port_get_stats_fn                   port_get_stats;
    sai_npu_port_clear_stats_fn                 port_clear_stats;
    sai_npu_port_clear_all_stats_fn             port_clear_all_stats;
    sai_npu_reg_link_state_cb_fn                reg_link_state_cb;
    sai_npu_port_switching_mode_update_fn       switching_mode_update;
    sai_npu_port_linkscan_mode_set_fn           linkscan_mode_set;
    sai_npu_port_debug_fn                       port_debug;
} sai_npu_port_api_t;

/**
 * \}
 */

#endif /* __SAI_NPU_PORT_H__ */
