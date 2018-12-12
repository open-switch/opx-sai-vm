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
 * \file    sai_npu_vlan.h
 *
 * \brief Declaration of SAI NPU VLAN APIs
*/

#if !defined (__SAINPUVLAN_H_)
#define __SAINPUVLAN_H_

#include "saivlan.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_vlan_common.h"

/** SAI NPU VLAN - -Perform VLAN NPU related initialization
  \return Success: SAI_STATUS_SUCCESS
          Failure: Appropriate failure error code
*/
typedef sai_status_t (*sai_npu_vlan_init_fn)(void);

/** SAI NPU VLAN - Create a VLAN
  \param[in] vlan_id VLAN Identifier
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_vlan_create_fn)(sai_vlan_id_t vlan_id);

/** SAI NPU VLAN - Delete a VLAN
  \param[in] vlan_id VLAN Identifier
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_vlan_delete_fn)(sai_vlan_id_t vlan_id);

/** SAI NPU VLAN - Add port to VLAN
  \param[in]  vlan_member_node VLAN member info
  \param[out] vlan_member_node SAI VLAN member uoid
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE,SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_vlan_member_create_fn)(
        sai_vlan_member_node_t *vlan_member_node);

/** SAI NPU VLAN - Remove port from VLAN
  \param[in]  vlan_member_node VLAN member info
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_vlan_member_remove_fn)(
        const sai_vlan_member_node_t *vlan_member_node);

/** SAI NPU VLAN - Get Counter collection on a VLAN
  \param[in] vlan_id VLAN Identifier
  \param[in] counter_ids Type of counter to enable
  \param[in] number_of_counters Number of counters to get
  \param[out] counters The value of the counters
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_get_vlan_stats_fn)(sai_vlan_id_t vlan_id,
                                                     const sai_vlan_stat_t *counter_ids,
                                                     unsigned int number_of_counters,
                                                     uint64_t* counters);

/** SAI NPU VLAN - Clear Counter collection on a VLAN
  \param[in] vlan_id VLAN Identifier
  \param[in] counter_ids Type of counter to enable
  \param[in] number_of_counters Number of counters to get
  \return Success: SAI_STATUS_SUCCESS
           Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_clear_vlan_stats_fn) (sai_vlan_id_t vlan_id,
                                                     const sai_vlan_stat_t *counter_ids,
                                                     unsigned int number_of_counters);


/** SAI NPU VLAN - Set VLAN member tagging mode
  \param[in] vlan_member_node VLAN member info
  \return Success: SAI_STATUS_SUCCESS
Failure: Appropriate failure error code
 */
typedef sai_status_t (*sai_npu_set_vlan_tagging_mode)(
                       const sai_vlan_member_node_t *vlan_member_node);


/**
 * @brief Handle LAG changes as notified by bridge port module
 *
 * @param[in] vlan_member_node VLAN member info
 * @param[in] lag_id LAG identifier
 * @param[in] port_cnt Number of ports added/removed from LAG
 * @param[in] port_list List of ports added/removed from LAG
 * @param[in] is_add Specifies if ports are added or removed from LAG
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_vlan_member_lag_notif_handler_fn)(const sai_vlan_member_node_t
                                                                 *vlan_member_node,
                                                                 sai_object_id_t lag_id,
                                                                 uint_t port_cnt,
                                                                 const sai_object_id_t *port_list,
                                                                 bool is_add);

/**
 * @brief Set VLAN attribute in NPU
 *
 * @param[in] vlan_obj_id SAI VLAN object identifier
 * @param[in] attr Attribute that is to be applied on VLAN
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_vlan_set_attribute_fn)(sai_object_id_t vlan_obj_id,
                                                      const sai_attribute_t *attr);

/**
 * @brief VLAN NPU API table.
 */
typedef struct _sai_npu_vlan_api_t {
    sai_npu_vlan_init_fn                      vlan_init;
    sai_npu_vlan_create_fn                    vlan_create;
    sai_npu_vlan_delete_fn                    vlan_delete;
    sai_npu_vlan_set_attribute_fn             vlan_set_attribute;
    sai_npu_vlan_member_create_fn             vlan_member_create;
    sai_npu_vlan_member_remove_fn             vlan_member_remove;
    sai_npu_get_vlan_stats_fn                 get_vlan_stats;
    sai_npu_clear_vlan_stats_fn               clear_vlan_stats;
    sai_npu_set_vlan_tagging_mode             set_vlan_member_tagging_mode;
    sai_npu_vlan_member_lag_notif_handler_fn  vlan_member_lag_notif_handler;
} sai_npu_vlan_api_t;

#endif
