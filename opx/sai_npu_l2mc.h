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
 * \file    sai_npu_l2mc.h
 *
 * \brief Declaration of SAI NPU L2MC APIs
*/

#if !defined (__SAINPUL2MC_H_)
#define __SAINPUL2MC_H_

#include "sail2mc.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_l2mc_common.h"

/** SAI NPU L2MC - L2MC Init Function
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_l2mc_init_fn)(void);

/** SAI NPU L2MC - Create a L2MC
  \param[inout]  l2mc_group_node L2MC group info
  \param[out] l2mc_group_node SAI L2MC group uoid
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_l2mc_group_create_fn)(
        dn_sai_l2mc_group_node_t *l2mc_group_node);

/** SAI NPU L2MC - Delete a L2MC
  \param[inout] l2mc_group_node L2MC group info
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_l2mc_group_delete_fn)(
                     dn_sai_l2mc_group_node_t *l2mc_group_node);

/** SAI NPU L2MC - Add port to L2MC Group
  \param[in]  l2mc_member_node L2MC member info
  \param[out] l2mc_member_node SAI L2MC member uoid
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE,SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_l2mc_member_create_fn)(
        dn_sai_l2mc_member_node_t *l2mc_member_node);

/** SAI NPU L2MC - Remove port from L2MC
  \param[in]  l2mc_member_node L2MC member info
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_l2mc_member_remove_fn)(
        const dn_sai_l2mc_member_node_t *l2mc_member_node);

/**
 * @brief Handle LAG changes as notified by bridge port module
 *
 * @param[in] l2mc_member_node L2MC member info
 * @param[in] lag_id LAG identifier
 * @param[in] port_cnt Number of ports added/removed from LAG
 * @param[in] port_list List of ports added/removed from LAG
 * @param[in] is_add Specifies if ports are added or removed from LAG
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_l2mc_member_lag_notif_handler_fn)(const dn_sai_l2mc_member_node_t
                                                                 *l2mc_member_node,
                                                                 sai_object_id_t lag_id,
                                                                 uint_t port_cnt,
                                                                 const sai_object_id_t *port_list,
                                                                 bool is_add);
/** SAI NPU L2MC - Dump L2MC Hardware info
  \param[in] hw_info - L2MC Hardware Info
 */
typedef void (*sai_npu_l2mc_group_dump_hw_info_fn)(const void *hw_info);

/**
 * @brief L2MC NPU API table.
 */
typedef struct _sai_npu_l2mc_api_t {
    sai_npu_l2mc_init_fn                      l2mc_init;
    sai_npu_l2mc_group_create_fn              l2mc_group_create;
    sai_npu_l2mc_group_delete_fn              l2mc_group_delete;
    sai_npu_l2mc_member_create_fn             l2mc_member_create;
    sai_npu_l2mc_member_remove_fn             l2mc_member_remove;
    sai_npu_l2mc_member_lag_notif_handler_fn  l2mc_member_lag_notif_handler;
    sai_npu_l2mc_group_dump_hw_info_fn        l2mc_group_dump_hw_info;
} sai_npu_l2mc_api_t;

#endif
