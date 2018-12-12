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
* @file sai_tunnel_npu_api.h
*
* @brief This file contains the prototypes for SAI Tunnel NPU plugin API
*
*************************************************************************/
#ifndef _SAI_TUNNEL_NPU_API_H_
#define _SAI_TUNNEL_NPU_API_H_

#include "saitypes.h"
#include "sai_tunnel.h"
#include "sai_common_utils.h"
#include "std_type_defs.h"

/** @defgroup SAINPUTUNNELAPI SAI - SAI Tunnel NPU Plugin API
 *   NPU plugin APIs for SAI Tunnel API functionality
 *
 *  \{
 */

/**
 * @brief Initialization of NPU specific objects for Tunnel function in NPU.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_tunnel_init_fn) (void);

/**
 * @brief De-Initialization of NPU specific objects for TUNNEL function in NPU.
 */
typedef void (*sai_npu_tunnel_deinit_fn) (void);

/**
 *  @brief Get Attribute Id properties table for Tunnel objects
 *
 * @param[in] obj_type Tunnel object type
 * @param[out] p_attr_id_table attribute id table
 * @param[out] p_max_attr_count Maximum attribute count
 */
typedef void (*sai_npu_tunnel_attr_id_table_get_fn) (
                              sai_object_type_t obj_type,
                              const dn_sai_attribute_entry_t **p_attr_id_table,
                              uint_t *p_max_attr_count);

/**
 * @brief Create a Tunnel object in NPU.
 *
 * @param[in] p_tunnel_obj Pointer to the Tunnel node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_tunnel_obj_create_fn) (
                                              dn_sai_tunnel_t *p_tunnel_obj);

/**
 * @brief Remove Tunnel object in NPU.
 *
 * @param[in] p_tunnel_obj Pointer to the Tunnel node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_tunnel_obj_remove_fn) (
                                             dn_sai_tunnel_t *p_tunnel_obj);

/**
 * @brief Validate Tunnel attribute value to be supported in NPU.
 *
 * @param[in] p_attr Pointer to the Attribute Value pair
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_tunnel_attr_validate_fn) (
                                       const sai_attribute_t *p_attr);


/**
 * @brief Create a Tunnel Termination table entry object in NPU.
 *
 * @param[in] p_tunnel_term Pointer to the Tunnel Termination entry node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_tunnel_term_create_fn) (
                                    dn_sai_tunnel_term_entry_t *p_tunnel_term);

/**
 * @brief Remove Tunnel Termination table entry object in NPU.
 *
 * @param[in] p_tunnel_term Pointer to the Tunnel Termination entry node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_tunnel_term_remove_fn) (
                                    dn_sai_tunnel_term_entry_t *p_tunnel_term);

/**
 * @brief Create a Tunnel Map entry object in NPU.
 *
 * @param[in] p_tunnel_map_entry Pointer to the Tunnel map entry node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_tunnel_map_entry_create_fn) (
                                    dn_sai_tunnel_map_entry_t *p_tunnel_map_entry);

/**
 * @brief Remove Tunnel map entry object in NPU.
 *
 * @param[in] p_tunnel_map_entry Pointer to the Tunnel map entry node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_tunnel_map_entry_remove_fn) (
                                    dn_sai_tunnel_map_entry_t *p_tunnel_map_entry);

/**
 * @brief Set Tunnel map entry attribute in NPU.
 *
 * @param[in] p_tunnel_map_entry Pointer to the Tunnel map entry node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_tunnel_map_entry_set_fn) (
                                    dn_sai_tunnel_map_entry_t *p_tunnel_map_entry);

/**
 * @brief Get tunnel statistics from the NPU.
 *
 * @param[in] tunnel_id Tunnel object id
 * @param[in] num_counters Number of counters.
 * @param[in] counter_ids Pointer to counter ids
 * @param[out] The counter values retrieved from the NPU.
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
typedef sai_status_t (*sai_npu_tunnel_stats_get_fn) (sai_object_id_t tunnel_id,
                                                     uint32_t num_counters,
                                                     const sai_tunnel_stat_t *counter_ids,
                                                     uint64_t *counters);

/**
 * @brief Clear tunnel statistics from the NPU.
 *
 * @param[in] tunnel_id Tunnel object id
 * @param[in] num_counters Number of counters.
 * @param[in] counter_ids Pointer to counter ids
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
typedef sai_status_t (*sai_npu_tunnel_stats_clear_fn) (sai_object_id_t tunnel_id,
                                                       uint32_t num_counters,
                                                       const sai_tunnel_stat_t *counter_ids);

/**
 * @brief Tunnel NPU Plugin API table.
 */
typedef struct _sai_npu_tunnel_api_t {
    sai_npu_tunnel_init_fn                   tunnel_init;
    sai_npu_tunnel_deinit_fn                 tunnel_deinit;
    sai_npu_tunnel_attr_id_table_get_fn      attr_id_table_get;
    sai_npu_tunnel_obj_create_fn             tunnel_obj_create;
    sai_npu_tunnel_obj_remove_fn             tunnel_obj_remove;
    sai_npu_tunnel_attr_validate_fn          tunnel_obj_attr_validate;
    sai_npu_tunnel_term_create_fn            tunnel_term_entry_create;
    sai_npu_tunnel_term_remove_fn            tunnel_term_entry_remove;
    sai_npu_tunnel_map_entry_create_fn       tunnel_map_entry_create;
    sai_npu_tunnel_map_entry_remove_fn       tunnel_map_entry_remove;
    sai_npu_tunnel_map_entry_set_fn          tunnel_map_entry_set;
    sai_npu_tunnel_stats_get_fn              tunnel_stats_get;
    sai_npu_tunnel_stats_clear_fn            tunnel_stats_clear;
}sai_npu_tunnel_api_t;


/**
 * \}
 */


#endif /* _SAI_TUNNEL_NPU_API_H_ */
