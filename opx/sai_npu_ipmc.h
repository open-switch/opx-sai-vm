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
 * \file    sai_npu_ipmc.h
 *
 * \brief Declaration of SAI NPU IPMC APIs
*/

#if !defined (__SAINPUIPMC_H_)
#define __SAINPUIPMC_H_

#include "sail2mc.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_l2mc_common.h"
#include "sai_ipmc_common.h"

/** SAI NPU IPMC - IPMC Init Function
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_ipmc_init_fn)(void);

/** SAI NPU IPMC - Creates an IPMC replication group
  \param[inout]  ipmc_repl_group_node IPMC replication group info
  \param[out] ipmc_repl_group_node SAI IPMC Repl group uoid
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_ipmc_repl_group_create_fn)(
        dn_sai_ipmc_repl_group_node_t *ipmc_repl_group_node);

/** SAI NPU IPMC - Delete an IPMC Replication group
  \param[inout] ipmc_repl_group_node IPMC Replication group info
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_ipmc_repl_group_delete_fn)(
                     dn_sai_ipmc_repl_group_node_t *ipmc_repl_group_node);

/** SAI NPU IPMC - Creates an IPMC RPF group
  \param[inout]  ipmc_rpf_group_node IPMC rpf group info
  \param[out] ipmc_rpf_group_node SAI IPMC RPF group uoid
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_ipmc_rpf_group_create_fn)(
        dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node);

/** SAI NPU IPMC - Delete an IPMC RPF group
  \param[inout] ipmc_rpf_group_node IPMC RPF group info
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_ipmc_rpf_group_delete_fn)(
                     dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node);

/** SAI NPU IPMC - Modified an IPMC RPF group
  \param[inout]  ipmc_rpf_group_node IPMC rpf group info
  \param[out] ipmc_rpf_group_node SAI IPMC RPF group uoid
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_ipmc_rpf_group_modify_fn)(
        dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node);

/** SAI NPU IPMC - Add L3 Router Interface to IPMC RPF Group
  \param[in]  ipmc_rpf_group_node IPMC RPF Group info
  \param[in]  ipmc_rpf_group_member_node IPMC RPF Group member info
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_ipmc_rpf_group_member_create_fn)(
        dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node,
        dn_sai_ipmc_rpf_group_member_node_t *ipmc_rpf_group_member_node);

/** SAI NPU IPMC - Remove L3 Router Interface from IPMC RPF Group
  \param[in]  ipmc_rpf_group_node IPMC RPF Group info
  \param[in]  ipmc_rpf_group_member_node IPMC RPF Group member info
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_ipmc_rpf_group_member_remove_fn)(
        dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node,
        dn_sai_ipmc_rpf_group_member_node_t *ipmc_rpf_group_member_node);

/** SAI NPU IPMC - Add an egress phyical port into L3 Router Interface of IPMC RPF Group
  \param[in]  ipmc_rpf_group_node IPMC RPF Group info
  \param[in]  ipmc_rpf_group_member_node IPMC RPF Group member info
  \param[in]  port_obj_id object id of the port
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_ipmc_rpf_group_member_port_add_fn)(
        dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node,
        dn_sai_ipmc_rpf_group_member_node_t *ipmc_rpf_group_member_node,
        sai_object_id_t port_obj_id);

/** SAI NPU IPMC - Remove an egress physical port from L3 Router Interface of the IPMC RPF Group
  \param[in]  ipmc_rpf_group_node IPMC RPF Group info
  \param[in]  ipmc_rpf_group_member_node IPMC RPF Group member info
  \param[in]  port_obj_id object id of the port
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_ipmc_rpf_group_member_port_remove_fn)(
        dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node,
        dn_sai_ipmc_rpf_group_member_node_t *ipmc_rpf_group_member_node,
        sai_object_id_t port_obj_id);

/** SAI NPU IPMC - Dump IPMC RPF Group/Member Hardware info
  \param[in] hw_info - IPMC RPF Group/Member Hardware Info
 */
typedef void (*sai_npu_ipmc_rpf_group_member_dump_hw_info_fn)(const void *hw_info);

/**
 * @brief L3 IPMC NPU API table.
 */
typedef struct _sai_npu_l3_ipmc_api_t {
    sai_npu_ipmc_init_fn                      ipmc_init;
    sai_npu_ipmc_repl_group_create_fn         ipmc_repl_group_create;
    sai_npu_ipmc_repl_group_delete_fn         ipmc_repl_group_remove;
    sai_npu_ipmc_rpf_group_create_fn          ipmc_rpf_group_create;
    sai_npu_ipmc_rpf_group_delete_fn          ipmc_rpf_group_remove;
    sai_npu_ipmc_rpf_group_modify_fn          ipmc_rpf_group_modify;
    sai_npu_ipmc_rpf_group_member_create_fn   ipmc_rpf_group_member_create;
    sai_npu_ipmc_rpf_group_member_remove_fn   ipmc_rpf_group_member_remove;
    sai_npu_ipmc_rpf_group_member_port_add_fn ipmc_rpf_group_member_port_add;
    sai_npu_ipmc_rpf_group_member_port_remove_fn ipmc_rpf_group_member_port_remove;
    sai_npu_ipmc_rpf_group_member_dump_hw_info_fn  ipmc_rpf_group_member_dump_hw_info;
} sai_npu_l3_ipmc_api_t;

#endif
