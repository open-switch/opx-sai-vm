/************************************************************************
* * LEGALESE:   "Copyright (c) 2019, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/***
 * \file    sai_npu_l3_mcast.h
 *
 * \brief Declaration of SAI NPU L3 MCAST APIs
*/

#if !defined (__SAINPUL3MCAST_H_)
#define __SAINPUL3MCAST_H_

#include "sail2mc.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_l2mc_common.h"
#include "sai_mcast_common.h"
#include "sai_ipmc_common.h"
#include "sai_l3_mcast_common.h"

/** SAI NPU MCAST - Create a L3 MCAST Entry
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_l3_mcast_int_fn)(void);

/** SAI NPU MCAST - Create a L3 MCAST Entry
  \param[in]  mcast_entry_node MCAST entry info
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_l3_mcast_entry_create_fn)(
        const dn_sai_l3_mcast_entry_node_t *l3_mcast_entry_node);

/** SAI NPU MCAST - Delete a L3 MCAST Entry
  \param[in] mcast_entry_node MCAST entry info
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_l3_mcast_entry_remove_fn)(
        const dn_sai_l3_mcast_entry_node_t *l3_mcast_entry_node);

/** SAI NPU MCAST - Update a L3 MCAST Entry
  \param[in] mcast_entry_node MCAST entry info
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_l3_mcast_entry_update_fn)(
        const dn_sai_l3_mcast_entry_node_t *l3_mcast_entry_node);

/** SAI NPU MCAST - Get a L3 MCAST Entry
  \param[in] mcast_entry_node MCAST entry info
  \param[out] mcast_entry_node MCAST entry info pkt action
  \ if entry not hit action will set to Trap else Forward
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_l3_mcast_entry_get_fn)(
        const dn_sai_l3_mcast_entry_node_t *l3_mcast_entry_node);

/**
 * @brief MCAST NPU API table.
 */
typedef struct _sai_npu_l3_mcast_api_t {
    sai_npu_l3_mcast_int_fn                       l3_mcast_init;
    sai_npu_l3_mcast_entry_create_fn              l3_mcast_entry_create;
    sai_npu_l3_mcast_entry_remove_fn              l3_mcast_entry_remove;
    sai_npu_l3_mcast_entry_update_fn              l3_mcast_entry_update;
    sai_npu_l3_mcast_entry_get_fn                 l3_mcast_entry_get;
} sai_npu_l3_mcast_api_t;

#endif
