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
* @file sai_npu_switch.h
*
* @brief This file contains API signatures for SAI NPU Switch component.
*        SAI Common use these API's for Initializing the NPU switch and
*        for switch level attributes set/get.
*
*******************************************************************************/

/* OPENSOURCELICENSE */

#ifndef __SAI_NPU_SWITCH_H__
#define __SAI_NPU_SWITCH_H__

#include "saitypes.h"
#include "saistatus.h"
#include "saiport.h"
#include "saiswitch.h"

#include "std_type_defs.h"
#include "std_config_node.h"
#include "sai_switch_common.h"

/** \defgroup SAINPUSWITCHAPI SAI - NPU switch Functionality
 *   Switch functions for SAI NPU component
 *
 * \ingroup SAISWITCHAPI
 * \{
 */

/**
 * @brief Set Packet switching mode
 *
 * @param[in] mode  packet switching mode of type sai_switch_switching_mode_t
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_switching_mode_set_fn)(const sai_switch_switching_mode_t mode);

/**
 * @brief Get Packet switching mode
 *
 * @param[out] mode  packet switching mode of type sai_switch_switching_mode_t
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_switching_mode_get_fn)(sai_switch_switching_mode_t *mode);

/**
 * @brief Get current value of the maximum switch temperature in Celsius
 *
 * @param[out] value  current value of the maximum temperature in Celsius
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_switch_temp_get_fn)(sai_attribute_value_t *value);

/**
 * @brief Initialize the NPU SDK
 *
 * @param[in] switch_id  Switch hardware id from adapter host
 * @return SAI_STATUS_SUCCESS if initialization is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_switch_init_fn)(sai_switch_info_t *sai_switch_info);

/**
 * @brief Initialize the NPU vendor specific init configuration
 *
 * @param[in] switch_id  Switch hardware id from adapter host
 * @param[in] vendor_node NPU vendor config node
 * @return SAI_STATUS_SUCCESS if initialization is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_switch_init_config_fn)(sai_switch_info_t *sai_switch_info,
                                                      std_config_node_t vendor_node);

/**
 * @brief Get the packet action set for TTL1 packets
 *
 * @param[out] action packet action set for TTL1 packets
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_ttl1_violation_action_get_fn)(sai_packet_action_t *action);

/**
 * @brief Set the packet action for TTL1 packets
 *
 * @param[in] action packet action to be set for TTL1 packets
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_ttl1_violation_action_set_fn)(sai_packet_action_t action);

/**
 * @brief Set the SW Statistics counter refresh interval in seconds
 *
 * @param[in] cntr_interval counter interval in seconds and value Zero
 * is used for setting HW statistics mode
 */
typedef sai_status_t (*sai_npu_switch_counter_refresh_interval_set_fn)(uint_t cntr_interval);

/**
 * @brief Set the algorithm for ecmp hashing
 *
 * @param[in] ecmp algorithm to be set for ecmp hashing
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_ecmp_hash_algorithm_set_fn) (sai_hash_algorithm_t ecmp_algo);

/**
 * @brief Set the algorithm for lag hashing
 *
 * @param[in] lag algorithm to be set for lag hashing
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_lag_hash_algorithm_set_fn) (sai_hash_algorithm_t lag_algo);

/**
 * @brief Set the seed value for ecmp hashing
 *
 * @param[in] hash seed value be set for ecmp hashing
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_ecmp_hash_seed_value_set_fn) (sai_switch_hash_seed_t
                                                             ecmp_hash_seed_value);

/**
 * @brief Set the seed value for lag hashing
 *
 * @param[in] hash seed value be set for lag hashing
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_lag_hash_seed_value_set_fn) (sai_switch_hash_seed_t
                                                            lag_hash_seed_value);

/**
 * @brief Set the hash field for ecmp hashing
 *
 * @param[in] ecmp hash field to be set for ecmp hashing
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_ecmp_hash_field_set_fn) (sai_s32_list_t *ecmp_hash_field_list);

/**
 * @brief Set the hash field for lag hashing
 *
 * @param[in] lag hash field to be set for lag hashing
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_lag_hash_field_set_fn) (sai_s32_list_t *lag_hash_field_list);

/**
 * @brief Get the algorithm of ecmp hashing
 *
 * @param[out] ecmp hashing algorithm value is stored in ecmp_algo param
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_ecmp_hash_algorithm_get_fn) (sai_hash_algorithm_t *ecmp_algo);

/**
 * @brief Get the algorithm of lag hashing
 *
 * @param[out] lag hashing algorithm value is stored in lag_algo param
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_lag_hash_algorithm_get_fn) (sai_hash_algorithm_t *lag_algo);

/**
 * @brief Get the seed value of ecmp hashing
 *
 * @param[out] hash seed value is stored in ecmp_hash_see_value param
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_ecmp_hash_seed_value_get_fn) (sai_switch_hash_seed_t
                                                             *ecmp_hash_seed_value);

/**
 * @brief Get the seed value of lag hashing
 *
 * @param[out] hash seed value is stored in lag_hash_see_value param
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_lag_hash_seed_value_get_fn) (sai_switch_hash_seed_t
                                                            *lag_hash_seed_value);

/**
 * @brief Get the hash field bitmap of ecmp hashing
 *
 * @param[out] ecmp hash field bitmap is stored in ecmp_hash_field_list param
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_ecmp_hash_field_get_fn) (sai_s32_list_t *ecmp_hash_field_list);

/**
 * @brief Get the hash field bitmap of lag hashing
 *
 * @param[out] lag hash field bitmap is stored in lag_hash_field_list param
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_lag_hash_field_get_fn) (sai_s32_list_t *lag_hash_field_list);

/**
 * @brief Get the max number of ports that can be part of a LAG
 *
 * @param[out] max_members Count of the maximum number of ports that can be added to LAG
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_lag_max_members_get_fn) (uint32_t *max_members);

/**
 * @brief Get the max number of LAGs that can be created per switch
 *
 * @param[out] max_members Maximum number of LAGs that can be created per switch
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_lag_max_number_get_fn) (uint32_t *max_number);

/**
 * @brief Set the Switch Hash attribute with native hash fields list.
 *
 * @param[in] attr_id Switch Hash attribute id to be set.
 * @param[in] native_field_list List of native hash fields.
 * @param[in] udf_list List of UDF hash fields.
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_switch_hash_fields_set_fn) (
                                        sai_attr_id_t attr_id,
                                        const sai_s32_list_t *native_field_list,
                                        const sai_object_list_t *udf_list);

/**
 * brief Get the min and max split horizon id range.
 *
 * @param[out] min_value The minimum split horizon range value.
 * @param[out] max_value The maximum split horizon range value.
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_switch_vxlan_split_horizon_range_get_fn)(
                                                           uint32_t *min_value,
                                                           uint32_t *max_value);
/**
 * @brief Switch NPU API table.
 */
typedef struct _sai_npu_switch_api_t {
    sai_npu_switch_init_fn                            switch_init;
    sai_npu_switch_init_config_fn                     switch_init_config;
    sai_npu_switching_mode_set_fn                     switching_mode_set;
    sai_npu_switching_mode_get_fn                     switching_mode_get;
    sai_npu_switch_temp_get_fn                        switch_temp_get;
    sai_npu_ttl1_violation_action_set_fn              ttl_action_set;
    sai_npu_ttl1_violation_action_get_fn              ttl_action_get;
    sai_npu_switch_counter_refresh_interval_set_fn    counter_refresh_interval_set;
    sai_npu_ecmp_hash_algorithm_set_fn                ecmp_hash_algorithm_set;
    sai_npu_ecmp_hash_seed_value_set_fn               ecmp_hash_seed_value_set;
    sai_npu_ecmp_hash_algorithm_get_fn                ecmp_hash_algorithm_get;
    sai_npu_ecmp_hash_seed_value_get_fn               ecmp_hash_seed_value_get;
    sai_npu_lag_hash_algorithm_set_fn                 lag_hash_algorithm_set;
    sai_npu_lag_hash_seed_value_set_fn                lag_hash_seed_value_set;
    sai_npu_lag_hash_algorithm_get_fn                 lag_hash_algorithm_get;
    sai_npu_lag_hash_seed_value_get_fn                lag_hash_seed_value_get;
    sai_npu_lag_max_members_get_fn                    lag_max_members_get;
    sai_npu_lag_max_number_get_fn                     lag_max_number_get;
    sai_npu_switch_hash_fields_set_fn                 switch_hash_fields_set;
    sai_npu_switch_vxlan_split_horizon_range_get_fn   vxlan_split_horizon_range_get;
} sai_npu_switch_api_t;

/**
 * \}
 */


#endif /* __SAI_NPU_SWITCH_H__ */
