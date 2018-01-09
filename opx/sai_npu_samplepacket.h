/************************************************************************
* LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
 * @file    sai_npu_samplepacket.h
 *
 * @brief  SAI NPU specific Samplepacket APIs
 *
*************************************************************************/

#ifndef __SAI_NPU_SAMPLEPACKET_H_
#define __SAI_NPU_SAMPLEPACKET_H_

#include <saisamplepacket.h>
#include <saitypes.h>
#include <saistatus.h>

#include "sai_samplepacket_defs.h"
#include "sai_oid_utils.h"

/** @defgroup SAISAMPLEPACKETNPUAPI SAI - Port Samplepacket Utility
 *  Utility functions for SAI Port Samplepacket component
 *
 *  \{
 */

/**
 * @brief SAI NPU Samplepacket Initialization
 *
 * @return SAI_STATUS_SUCCESS if successfully intialized otherwise appropriate error
 * code is returned
 */
typedef sai_status_t (*sai_npu_samplepacket_init_fn)(void);

/**
 * @brief SAI NPU Samplepacket Session Create
 *
 * @param[in] p_session_info Samplepacket session node
 * @param[in] npu_object_id place holder for samplepacket object id
 * @return SAI_STATUS_SUCCESS if successfully created otherwise appropriate error
 * code is returned
 */
typedef sai_status_t (*sai_npu_samplepacket_session_create_fn) (dn_sai_samplepacket_session_info_t *p_session_info,
                                                                sai_npu_object_id_t *npu_object_id);

/**
 * @brief SAI NPU Samplepacket Session destroy
 *
 * @param[in] session_id Samplepacket session id
 * @return SAI_STATUS_SUCCESS if successfully destroyed otherwise appropriate error
 * code is returned
 */
typedef sai_status_t (*sai_npu_samplepacket_session_destroy_fn) (sai_object_id_t session_id);

/**
 * @brief Add the samplepacket source port to the given session
 *
 * @param[in] session_id Samplepacket session Id
 * @param[in] samplepacket_port portId on which sampling has to be done
 * @param[in] direction Sampling direction
 * @return SAI_STATUS_SUCCESS if samplepacket port is added to the session otherwise
 * appropriate sai error code would be returned
 */
typedef sai_status_t (*sai_npu_samplepacket_session_port_add_fn) (dn_sai_samplepacket_session_info_t *p_session_info,
                                                                  sai_object_id_t samplepacket_port,
                                                                  sai_samplepacket_direction_t direction);

/**
 * @brief Remove the samplepacket source port from the given session
 *
 * @param[in] session_id Samplepacket session Id
 * @param[in] samplepacket_port portId on which sampling has to be done
 * @param[in] direction Sampling direction
 * @return SAI_STATUS_SUCCESS if samplepacket port is removed from the session otherwise
 * appropriate sai error code would be returned
 */
typedef sai_status_t (*sai_npu_samplepacket_session_port_remove_fn) (sai_object_id_t session_id,
                                                                     sai_object_id_t samplepacket_port,
                                                                     sai_samplepacket_direction_t direction);

/**
 * @brief Set samplepacket session attribute
 *
 * @param[in] session_id Samplepacket session Id
 * @param[in] attr attribute to be set
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_samplepacket_session_set_fn) (dn_sai_samplepacket_session_info_t *p_session_info,
                                                             const sai_attribute_t *attr);

/**
 * @brief Retrieve samplepacket session attribute
 *
 * @param[in] session_id Samplepacket session Id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list attribute list to be filled
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_samplepacket_session_get_fn) (dn_sai_samplepacket_session_info_t *p_session_info,
                                                            uint32_t attr_count,
                                                            sai_attribute_t *attr_list);

/**
 * @brief Add the acl in-port to the given samplepacket session
 *
 * @param[in] session_id Samplepacket session Id
 * @param[in] samplepacket_port inPort in acl rule
 * @param[in] direction Sampling direction
 * @return SAI_STATUS_SUCCESS if sample rate is programmed to the port otherwise
 * appropriate sai error code would be returned
 */
typedef sai_status_t (*sai_npu_samplepacket_session_acl_port_add_fn)
                     (dn_sai_samplepacket_session_info_t *p_session_info,
                      sai_object_id_t samplepacket_port,
                      sai_samplepacket_direction_t direction);

/**
 * @brief Remove the acl in-port from the given samplepacket session
 *
 * @param[in] session_id Samplepacket session Id
 * @param[in] samplepacket_port inPort in acl rule
 * @param[in] direction Samplepacketing direction
 * @return SAI_STATUS_SUCCESS if sample rate is removed from the port otherwise
 * appropriate sai error code would be returned
 */
typedef sai_status_t (*sai_npu_samplepacket_session_acl_port_remove_fn)
                     (sai_object_id_t session_id,
                     sai_object_id_t samplepacket_port,
                     sai_samplepacket_direction_t direction);
/**
 * @brief SamplePacket NPU API table.
 */
typedef struct _sai_npu_samplepacket_api_t {
    sai_npu_samplepacket_init_fn                        samplepacket_init;
    sai_npu_samplepacket_session_create_fn              session_create;
    sai_npu_samplepacket_session_destroy_fn             session_destroy;
    sai_npu_samplepacket_session_port_add_fn            session_port_add;
    sai_npu_samplepacket_session_port_remove_fn         session_port_remove;
    sai_npu_samplepacket_session_set_fn                 session_set;
    sai_npu_samplepacket_session_get_fn                 session_get;
    sai_npu_samplepacket_session_acl_port_add_fn        session_acl_port_add;
    sai_npu_samplepacket_session_acl_port_remove_fn     session_acl_port_remove;

} sai_npu_samplepacket_api_t;

/**
 * \}
 */

#endif /* __SAI_NPU_SAMPLEPACKET_H_ */
