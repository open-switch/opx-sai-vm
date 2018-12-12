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
 * @file    sai_npu_mirror.h
 *
 * @brief  SAI NPU specific Mirror APIs
 *
*************************************************************************/

#ifndef __SAI_NPU_MIRROR_H_
#define __SAI_NPU_MIRROR_H_

#include <saimirror.h>
#include <saitypes.h>
#include <saistatus.h>

#include "sai_mirror_defs.h"
#include "sai_oid_utils.h"

/** @defgroup SAIMIRRORNPUAPI SAI - Port Mirror Utility
 *  Utility functions for SAI Port Mirror component
 *
 *  \{
 */

/**
 * @brief SAI NPU Mirror Initialization
 *
 * @return SAI_STATUS_SUCCESS if successfully intialized otherwise appropriate error
 * code is returned
 */
typedef sai_status_t (*sai_npu_mirror_init_fn)(void);

/**
 * @brief SAI NPU Mirror Session Create
 *
 * @param[in] p_session_info Mirror session node
 * @return SAI_STATUS_SUCCESS if successfully created otherwise appropriate error
 * code is returned
 */
typedef sai_status_t (*sai_npu_mirror_session_create_fn) (sai_mirror_session_info_t *p_session_info,
                                                          sai_npu_object_id_t *npu_object_id);

/**
 * @brief SAI NPU Mirror Session destroy
 *
 * @param[in] session_id Mirror session id
 * @return SAI_STATUS_SUCCESS if successfully destroyed otherwise appropriate error
 * code is returned
 */
typedef sai_status_t (*sai_npu_mirror_session_destroy_fn) (sai_object_id_t session_id);

/**
 * @brief Add the mirror source port to the given session
 *
 * @param[in] session_id Mirror session Id
 * @param[in] mirror_port Mirror source port Id
 * @param[in] direction Mirroring direction
 * @return SAI_STATUS_SUCCESS if mirror port is added to the session otherwise
 * appropriate sai error code would be returned
 */
typedef sai_status_t (*sai_npu_mirror_session_port_add_fn) (sai_object_id_t session_id,
                                                            sai_object_id_t mirror_port,
                                                            sai_mirror_direction_t direction);

/**
 * @brief Remove the mirror source port from the given session
 *
 * @param[in] session_id Mirror session Id
 * @param[in] mirror_port Mirror source port Id
 * @param[in] direction Mirroring direction
 * @return SAI_STATUS_SUCCESS if mirror port is removed from the session otherwise
 * appropriate sai error code would be returned
 */
typedef sai_status_t (*sai_npu_mirror_session_port_remove_fn) (sai_object_id_t session_id,
                                                               sai_object_id_t mirror_port,
                                                               sai_mirror_direction_t direction);

/**
 * @brief Set mirror session attribute
 *
 * @param[in] session_id Mirror session Id
 * @param[in] span_type Mirroring type
 * @param[in] attr attribute to be set
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_mirror_session_set_fn) (sai_object_id_t session_id,
                                                       sai_mirror_session_type_t span_type,
                                                       const sai_attribute_t *attr);

/**
 * @brief Retrieve mirror session attribute
 *
 * @param[in] session_id Mirror session Id
 * @param[in] span_type Mirroring type
 * @param[inout] attr_list attribute list to be filled
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_mirror_session_get_fn) (sai_object_id_t session_id,
                                                       sai_mirror_session_type_t span_type,
                                                       uint32_t attr_count,
                                                       sai_attribute_t *attr_list);

/**
 * @brief Validate mirror session attribute and its value for npu support
 *
 * @param[in] session_id Mirror session Id
 * @param[in] attr_list List of attributes
 * @param[in] attr_index attribute_index
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_mirror_session_validate_attribs_fn) (sai_object_id_t session_id,
                                                                    const sai_attribute_t
                                                                            *attr_list,
                                                                    uint32_t attr_index);
/**
 * @brief Mirror NPU API table.
 */
typedef struct _sai_npu_mirror_api_t {
    sai_npu_mirror_init_fn                       mirror_init;
    sai_npu_mirror_session_create_fn             session_create;
    sai_npu_mirror_session_destroy_fn            session_destroy;
    sai_npu_mirror_session_port_add_fn           session_port_add;
    sai_npu_mirror_session_port_remove_fn        session_port_remove;
    sai_npu_mirror_session_set_fn                session_set;
    sai_npu_mirror_session_get_fn                session_get;
    sai_npu_mirror_session_validate_attribs_fn   session_attribs_validate;
} sai_npu_mirror_api_t;


/**
 * \}
 */

#endif /* __SAI_NPU_MIRROR_H_ */
