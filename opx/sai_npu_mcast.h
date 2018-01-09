/************************************************************************
* * LEGALESE:   "Copyright (c) 2017, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
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
