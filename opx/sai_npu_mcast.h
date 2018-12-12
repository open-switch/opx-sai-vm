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
 * \file    sai_npu_mcast.h
 *
 * \brief Declaration of SAI NPU MCAST APIs
*/

#if !defined (__SAINPUMCAST_H_)
#define __SAINPUMCAST_H_

#include "sail2mc.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_l2mc_common.h"
#include "sai_mcast_common.h"

/** SAI NPU MCAST - Create a MCAST Entry
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_mcast_int_fn)(void);

/** SAI NPU MCAST - Create a MCAST Entry
  \param[in]  mcast_entry_node MCAST entry info
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_mcast_entry_create_fn)(
        const dn_sai_mcast_entry_node_t *mcast_entry_node);

/** SAI NPU MCAST - Delete a MCAST Entry
  \param[in] mcast_entry_node MCAST entry info
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_mcast_entry_remove_fn)(
        const dn_sai_mcast_entry_node_t *mcast_entry_node);

/**
 * @brief MCAST NPU API table.
 */
typedef struct _sai_npu_mcast_api_t {
    sai_npu_mcast_int_fn                       mcast_init;
    sai_npu_mcast_entry_create_fn              mcast_entry_create;
    sai_npu_mcast_entry_remove_fn              mcast_entry_remove;
} sai_npu_mcast_api_t;

#endif
