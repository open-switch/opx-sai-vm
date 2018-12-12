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

/***
 * \file    sai_bridge_npu_api.h
 *
 * \brief Declaration of SAI BRIDGE NPU APIs
*/

#if !defined (__SAIBRIDGENPUAPI_H_)
#define __SAIBRIDGENPUAPI_H_

#include "saibridge.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_bridge_common.h"
#include "sai_lag_callback.h"
/** \defgroup SAIBRIDGENPUAPIs SAI - Bridge NPU specific function implementations
 *  NPU specific functions for SAI Bridge component
 *
 *  \{
 */


/**
 * @brief Initialize the NPU vendor specific bridge init configuration
 *
 * @param init - true for init, false for deinit
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_bridge_init_fn)(bool init);


/**
 * @brief Create bridge in NPU
 *
 * @param[out] bridge_id Bridge object ID that is created
 * @param[inout] bridge_info Pointer to bridge info structure
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_bridge_create_fn)(sai_object_id_t      *bridge_id,
                                                 dn_sai_bridge_info_t *bridge_info);

/**
 * @brief Remove bridge in NPU
 *
 * @param[inout] bridge_info Pointer to bridge info structure
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_bridge_remove_fn)(dn_sai_bridge_info_t *bridge_info);

/**
 * @brief Set bridge attribute in NPU
 *
 * @param[inout] bridge_info Pointer to bridge info structure
 * @param[in] attr Pointer to attribute that is set
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_bridge_set_attr_fn)(dn_sai_bridge_info_t  *bridge_info,
                                                   const sai_attribute_t *attr);

/**
 * @brief Create bridge port in NPU
 *
 * @param[out] bridge_port_id Bridge port object ID that is created
 * @param[inout] bridge_port_info Pointer to bridge port info structure
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_bridge_port_create_fn)(sai_object_id_t           *bridge_port_id,
                                                      dn_sai_bridge_port_info_t *bridge_port_info);

/**
 * @brief Remove bridge port in NPU
 *
 * @param[inout] bridge_port_info Pointer to bridge port info structure
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_bridge_port_remove_fn)(dn_sai_bridge_port_info_t *bridge_port_info);

/**
 * @brief Set bridge port attribute in NPU
 *
 * @param[in] bridge_port_info Pointer to bridge port info structure
 * @param[in] attr Pointer to attribute that is set
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_bridge_port_set_attr_fn)(const dn_sai_bridge_port_info_t
                                                        *bridge_port_info,
                                                        const sai_attribute_t     *attr);

/**
 * @brief NPU Bridge port lag hadnler
 *
 * @param[inout] bridge_port_info Pointer to bridge port info structure
 * @param[in] lag_id SAI LAG Object ID
 * @param[in] add_ports True if ports are added, false if ports are removed
 * @param[in] port_list
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_bridge_port_lag_handler_fn)(dn_sai_bridge_port_info_t
                                                           *bridge_port_info,
                                                           sai_object_id_t lag_id,bool add_ports,
                                                           const sai_object_list_t *port_list);
/**
 * @brief Dump bridge NPU info
 *
 * @param[in] bridge_info Pointer to bridge info structure
 */
typedef void (*sai_npu_bridge_dump_fn)(const dn_sai_bridge_info_t *bridge_info);

/**
 * @brief Dump bridge port NPU info
 *
 * @param[in] bridge_port_info Pointer to bridge port info structure
 */
typedef void (*sai_npu_bridge_port_dump_fn)(const dn_sai_bridge_port_info_t *bridge_port_info);

/**
 * @brief Dump NPU related tables
 *
 * @param[in] bridge_port_info Pointer to bridge port info structure
 */
typedef void (*sai_npu_bridge_dump_npu_fn)(sai_bridge_debug_dump_t dump_type);

/**
 * @brief NPU Bridge Get stats
 *
 * @param[inout] bridge_info Pointer to bridge info structure
 * @param[in] number_of_counters Number of Counters
 * @param[in] counter_ids Counter IDs to be retrieved
 * @param[out] counters Value of counters retrieved
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_bridge_get_stats_fn) (const dn_sai_bridge_info_t *bridge_info,
                                                     uint32_t number_of_counters,
                                                     const sai_bridge_stat_t *counter_ids,
                                                     uint64_t *counters);

/**
 * @brief NPU Bridge Clear stats
 *
 * @param[inout] bridge_info Pointer to bridge info structure
 * @param[in] number_of_counters Number of Counters
 * @param[in] counter_ids Counter IDs to be Cleared
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_bridge_clear_stats_fn) (const dn_sai_bridge_info_t *bridge_info,
                                                       uint32_t number_of_counters,
                                                       const sai_bridge_stat_t *counter_ids);

/**
 * @brief NPU Bridge port Get stats
 *
 * @param[inout] bridge_port_info Pointer to bridge port info structure
 * @param[in] number_of_counters Number of Counters
 * @param[in] counter_ids Counter IDs to be retrieved
 * @param[out] counters Value of counters retrieved
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_bridge_port_get_stats_fn) (const dn_sai_bridge_port_info_t *bridge_port_info,
                                                          uint32_t number_of_counters,
                                                          const sai_bridge_port_stat_t *counter_ids,
                                                          uint64_t *counters);

/**
 * @brief NPU Bridge port Clear stats
 *
 * @param[inout] bridge_port_info Pointer to bridge port info structure
 * @param[in] number_of_counters Number of Counters
 * @param[in] counter_ids Counter IDs to be Cleared
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_bridge_port_clear_stats_fn) (const dn_sai_bridge_port_info_t *bridge_port_info,
                                                            uint32_t number_of_counters,
                                                            const sai_bridge_port_stat_t *counter_ids);

/**
 *  @brief Get Vendor attribute properties table of bridge
 *
 * @param[in] bridge_type Associated bridge type
 * @param[out] p_vendor Vendor attribute table
 * @param[out] p_max_attr_count Maximum attribute count
 */
typedef sai_status_t (*sai_npu_bridge_attr_table_get_fn) (sai_bridge_type_t bridge_type,
                                                          const dn_sai_attribute_entry_t **p_vendor,
                                                          uint_t *p_max_attr_count);

/**
 *  @brief Get Vendor attribute properties table of bridge port
 *
 * @param[in] bridge_port_type Associated Bridge port type
 * @param[out] p_vendor Vendor attribute table
 * @param[out] p_max_attr_count Maximum attribute count
 */
typedef sai_status_t (*sai_npu_bridge_port_attr_table_get_fn) (sai_bridge_port_type_t
                                                               bridge_port_type,
                                                               const dn_sai_attribute_entry_t
                                                               **p_vendor,
                                                               uint_t *p_max_attr_count);

/**
 * @brief Bridge NPU Router API table.
 */
typedef struct _sai_npu_bridge_api_t {
    sai_npu_bridge_init_fn                      bridge_init;
    sai_npu_bridge_create_fn                    bridge_create;
    sai_npu_bridge_remove_fn                    bridge_remove;
    sai_npu_bridge_set_attr_fn                  bridge_set_attribute;
    sai_npu_bridge_get_stats_fn                 bridge_get_stats;
    sai_npu_bridge_clear_stats_fn               bridge_clear_stats;
    sai_npu_bridge_port_create_fn               bridge_port_create;
    sai_npu_bridge_port_remove_fn               bridge_port_remove;
    sai_npu_bridge_port_set_attr_fn             bridge_port_set_attribute;
    sai_npu_bridge_port_get_stats_fn            bridge_port_get_stats;
    sai_npu_bridge_port_clear_stats_fn          bridge_port_clear_stats;
    sai_npu_bridge_port_lag_handler_fn          bridge_port_lag_handler;
    sai_npu_bridge_attr_table_get_fn            bridge_attr_table_get;
    sai_npu_bridge_port_attr_table_get_fn       bridge_port_attr_table_get;
    sai_npu_bridge_dump_fn                      bridge_dump_hw_info;
    sai_npu_bridge_port_dump_fn                 bridge_port_dump_hw_info;
    sai_npu_bridge_dump_npu_fn                  bridge_dump_npu;
} sai_npu_bridge_api_t;

/**
 * \}
 */

#endif
