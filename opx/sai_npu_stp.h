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
 * @file    sai_npu_stp.h
 *
 * @brief  SAI NPU specific Spanning Tree Group APIs
 *
*************************************************************************/

/* OPENSOURCELICENSE */

#ifndef __SAI_NPU_STP_H_
#define __SAI_NPU_STP_H_

#include <saistp.h>
#include <saitypes.h>
#include <saistatus.h>

#include "sai_stp_defs.h"
#include "sai_oid_utils.h"

/** @defgroup SAISTPNPUAPI SAI - L2 STP Utility
 *  Utility functions for SAI L2 STP component
 *
 *  \{
 */

/**
 * @brief SAI NPU STP Initialization
 *
 * @param[out] p_def_inst_id place holder for the default instance id
 * @param[out] p_l3_inst_id place holder for the l3 stg instance id
 * @return SAI_STATUS_SUCCESS if successfully intialized otherwise appropriate error
 * code is returned
 */
typedef sai_status_t (*sai_npu_stp_init_fn)(sai_npu_object_id_t *p_def_inst_id,
                                            sai_npu_object_id_t *p_l3_inst_id);

/**
 * @brief SAI NPU default STP Instance retrieve
 *
 * @param[out] p_def_inst_id place holder for the default instance id
 * @return SAI_STATUS_SUCCESS if successfully intialized otherwise appropriate error
 * code is returned
 */
typedef sai_status_t (*sai_npu_stp_default_instance_get_fn) (
                                           sai_npu_object_id_t *p_def_inst_id);

/**
 * @brief SAI NPU STP instance Create
 *
 * @param[out] p_stg_id place holder for the stg instance id
 * @return SAI_STATUS_SUCCESS if successfully created otherwise appropriate error
 * code is returned
 */
typedef sai_status_t (*sai_npu_stp_instance_create_fn) (sai_npu_object_id_t *p_stg_id);

/**
 * @brief SAI NPU STP instance destroy
 *
 * @param[in] stp_inst_id STG id
 * @return SAI_STATUS_SUCCESS if successfully destroyed otherwise appropriate error
 * code is returned
 */
typedef sai_status_t (*sai_npu_stp_instance_remove_fn) (sai_object_id_t stp_inst_id);

/**
 * @brief Add Vlan to the STG instance
 *
 * @param[in] stp_inst_id STP instance Id
 * @param[in] vlan_id vlan Id to be added
 * @return SAI_STATUS_SUCCESS if vlan is added to the stg otherwise
 * appropriate sai error code would be returned
 */
typedef sai_status_t (*sai_npu_stp_vlan_add_fn) (sai_object_id_t stp_inst_id,
                                                 sai_vlan_id_t vlan_id);

/**
 * @brief Remove Vlan from the STG instance
 *
 * @param[in] stp_inst_id STP instance Id
 * @param[in] vlan_id vlan Id to be added
 * @return SAI_STATUS_SUCCESS if vlan is removed from the stg otherwise
 * appropriate sai error code would be returned
 */
typedef sai_status_t (*sai_npu_stp_vlan_remove_fn) (sai_object_id_t stp_inst_id,
                                                    sai_vlan_id_t vlan_id);

/**
 * @brief Retrieve stp instance attribute
 *
 * @param[in] stp_inst_id STP instance Id
 * @param[in] attr_count Number of attributes
 * @param[inout] p_attr_list attribute list to be filled
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_stp_attribute_get_fn) (sai_object_id_t stp_inst_id,
                                                      uint32_t attr_count,
                                                      sai_attribute_t *p_attr_list);

/**
 * @brief Set stp port state
 *
 * @param[in] stp_inst_id STP instance Id
 * @param[in] port_id Port to set the stp state
 * @param[in] state Stp state
 * @return SAI_STATUS_SUCCESS if stp port state is set otherwise
 * appropriate sai error code would be returned
 */
typedef sai_status_t (*sai_npu_stp_port_state_set_fn) (sai_object_id_t stp_inst_id,
                                                       sai_object_id_t port_id,
                                                       sai_stp_port_state_t state);

/**
 * @brief Get stp port state
 *
 * @param[in] stp_inst_id STP instance Id
 * @param[in] port_id Port to set the stp state
 * @param[out] p_state Stp state
 * @return SAI_STATUS_SUCCESS if retrieval of stp port state is successful otherwise
 * appropriate sai error code would be returned
 */
typedef sai_status_t (*sai_npu_stp_port_state_get_fn) (sai_object_id_t stp_inst_id,
                                                       sai_object_id_t port_id,
                                                       sai_stp_port_state_t *p_state);

/**
 * @brief Retrieve the STP instance for the given vlan
 *
 * @param[in] vlan_id vlan Id to be added
 * @param[out] p_stp_id place holder for the retrived STP instance Id
 * @return SAI_STATUS_SUCCESS if retrieval of stp instance is successful otherwise
 * appropriate sai error code would be returned
 */
typedef sai_status_t (*sai_npu_vlan_stp_get_fn) (sai_vlan_id_t vlan_id,
                                                 sai_npu_object_id_t *p_stp_id);

/**
 * @brief Handle notifications from bridge port init
 *
 * @param[in] port_id SAI port object identifier
 * @param[in] create true if port create, false for delete
 * @return SAI_STATUS_SUCCESS if retrieval of stp instance is successful otherwise
 * appropriate sai error code would be returned
 */
typedef sai_status_t (*sai_npu_stp_port_notif_handler_fn)(sai_object_id_t port_id,
                                                          bool create);

/**
 * @brief Handle notifications from LAG module
 *
 * @param[in] p_stp_port_info STP port info structure
 * @param[in] lag_id LAG identifier
 * @param[in] port_cnt Number of ports added/removed from LAG
 * @param[in] port_list List of ports added/removed from LAG
 * @param[in] is_add Specifies if ports are added or removed from LAG
 * @return SAI_STATUS_SUCCESS if retrieval of stp instance is successful otherwise
 * appropriate sai error code would be returned
 */
typedef sai_status_t (*sai_npu_stp_port_lag_handler_fn)(const dn_sai_stp_port_info_t
                                                        *p_stp_port_info, sai_object_id_t lag_id,
                                                        uint_t port_cnt,
                                                        sai_object_id_t *port_list, bool is_add);

/**
 * @brief STP NPU API table.
 */
typedef struct _sai_npu_stp_api_t {
    sai_npu_stp_init_fn                  stp_init;
    sai_npu_stp_default_instance_get_fn  default_instance_get;
    sai_npu_stp_instance_create_fn       instance_create;
    sai_npu_stp_instance_remove_fn       instance_remove;
    sai_npu_stp_vlan_add_fn              vlan_add;
    sai_npu_stp_vlan_remove_fn           vlan_remove;
    sai_npu_stp_attribute_get_fn         attribute_get;
    sai_npu_stp_port_state_set_fn        port_state_set;
    sai_npu_stp_port_state_get_fn        port_state_get;
    sai_npu_vlan_stp_get_fn              vlan_stp_get;
    sai_npu_stp_port_notif_handler_fn    stp_port_notif_handler;
    sai_npu_stp_port_lag_handler_fn      stp_port_lag_handler;
} sai_npu_stp_api_t;

/**
 * \}
 */

#endif /* __SAI_NPU_STP_H_ */
