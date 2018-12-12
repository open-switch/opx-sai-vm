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
 * \file    sai_npu_lag.h
 *
 * \brief Declaration of SAI NPU LAG APIs
*/

#if !defined (__SAINPULAG_H_)
#define __SAINPULAG_H_

#include "saitypes.h"
#include "saistatus.h"
#include "sailag.h"
#include "sai_lag_common.h"

/** SAI NPU LAG - -Perform LAG NPU related initialization
 */
typedef void (*sai_npu_lag_init_fn)(void);

/** SAI NPU LAG - Create a LAG
  \param[out] lag_id LAG Identifier
  \return Success: SAI_STATUS_SUCCESS
Failure: Appropriate error code on failure
 */
typedef sai_status_t (*sai_npu_lag_create_fn)(sai_object_id_t *lag_id);

/** SAI NPU LAG - Remove a LAG
  \param[in] lag_id LAG Identifier
  \return Success: SAI_STATUS_SUCCESS
Failure: Appropriate error code on failure
 */
typedef sai_status_t (*sai_npu_lag_remove_fn)(sai_object_id_t lag_id);

/** SAI NPU LAG - Add Ports to a LAG
  \param[in] lag_id LAG Identifier
  \param[in] lag_port_list List of ports to be added to LAG
  \param[out] lag_member_id_list List of object ids generated for
              the lag members.
  \return Success: SAI_STATUS_SUCCESS
Failure: Appropriate error code on failure
 */
typedef sai_status_t (*sai_npu_add_ports_to_lag_fn)(sai_object_id_t lag_id,
                                                    const sai_object_list_t *lag_port_list,
                                                    sai_object_list_t *lag_member_id_list);

/** SAI NPU LAG - Remove Ports from a LAG
  \param[in] lag_id LAG Identifier
  \param[in] lag_port_list List of ports to be removed from LAG
  \return Success: SAI_STATUS_SUCCESS
Failure: Appropriate error code on failure
 */
typedef sai_status_t (*sai_npu_remove_ports_from_lag_fn)(sai_object_id_t lag_id,
                                                         const sai_object_list_t *lag_port_list);

/** SAI NPU LAG - Set LAG Port Flags
  \param[in] lag_id LAG Identifier
  \param[in] port_id Port Identifier
  \param[in] is_ingress Direction. Ingress if 'true' else 'false'
  \param[in] value Disable flag Value to be set. Combined with 'is_ingress'
  provides values for ingress_disable and egress_disable.
  \return Success: SAI_STATUS_SUCCESS
Failure: Appropriate error code on failure
 */
typedef sai_status_t (*sai_npu_lag_port_flag_set_fn)(sai_object_id_t lag_id,
                                                     sai_object_id_t port_id,
                                                     bool            is_ingress,
                                                     bool            value);

/** SAI NPU LAG - Get LAG Port Flags
  \param[in] lag_id LAG Identifier
  \param[in] port_id Port Identifier
  \param[in] is_ingress Direction. Ingress if 'true' else 'false'
  \param[in] value Disable flag Value to be obtained. Combined with
  'is_ingress' provides values for ingress_disable and egress_disable.
  \return Success: SAI_STATUS_SUCCESS
Failure: Appropriate error code on failure
 */
typedef sai_status_t (*sai_npu_lag_port_flag_get_fn)(sai_object_id_t lag_id,
                                                     sai_object_id_t port_id,
                                                     bool            is_ingress,
                                                     bool            *value);


/**
 * @brief Set LAG Attribute in NPU
 *
 * @param[in] lag_info Pointer to LAG node
 * @param[in] attr to be applied
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_lag_attribute_set_fn)(sai_lag_node_t *lag_info,
                                                     const sai_attribute_t *attr);

/**
 * @brief Get LAG Attribute from member's NPU table for debug
 *
 * @param[in] lag_id LAG object ID
 * @param[in] lag_member_id LAG member object ID
 * @param[in] attr_count Number of attributes to be retrieved
 * @param[inout] attr_list Attributes to be retrieved
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_lag_attr_get_from_member_fn)(sai_object_id_t lag_id,
                                                            sai_object_id_t lag_member_id,
                                                            uint_t attr_count,
                                                            sai_attribute_t *attr_list);
/**
 * @brief LAG NPU API table.
 */
typedef struct _sai_npu_lag_api_t {
    sai_npu_lag_init_fn                 lag_init;
    sai_npu_lag_create_fn               lag_create;
    sai_npu_lag_remove_fn               lag_remove;
    sai_npu_add_ports_to_lag_fn         add_ports_to_lag;
    sai_npu_remove_ports_from_lag_fn    remove_ports_from_lag;
    sai_npu_lag_port_flag_set_fn        lag_port_flag_set;
    sai_npu_lag_port_flag_get_fn        lag_port_flag_get;
    sai_npu_lag_attribute_set_fn        lag_attribute_set;
    sai_npu_lag_attr_get_from_member_fn lag_attribute_get_from_member;
} sai_npu_lag_api_t;
#endif
