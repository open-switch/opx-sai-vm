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
* @file sai_udf_npu_api.h
*
* @brief This file contains the prototypes for SAI NPU UDF APIs.
*
*************************************************************************/
#ifndef _SAI_UDF_NPU_API_H_
#define _SAI_UDF_NPU_API_H_

#include "saitypes.h"
#include "sai_oid_utils.h"
#include "sai_udf_common.h"
#include "std_type_defs.h"

/** @defgroup SAINPUUDFAPI SAI - SAI UDF NPU Plugin API
 *   NPU plugin APIs for SAI UDF functionality
 *
 *  \{
 */

/**
 * @brief Initialization of NPU specific objects for UDF function in NPU.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_udf_init_fn) (void);

/**
 * @brief De-Initialization of NPU specific objects for UDF function in NPU.
 */
typedef void (*sai_npu_udf_deinit_fn) (void);

/**
 * @brief Create a UDF Match object in NPU.
 *
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Attribute Id, Value pairs for the UDF Match object
 * @param[out] p_match_id Pointer to UDF Match object Id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_udf_match_create_fn) (uint_t attr_count,
                                               const sai_attribute_t *attr_list,
                                               sai_npu_object_id_t *p_match_id);

/**
 * @brief Remove a UDF Match object in NPU.
 *
 * @param[in] udf_match_id UDF Match object Id to be removed.
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_udf_match_remove_fn) (sai_object_id_t udf_match_id);


/**
 * @brief Sets an attribute to the UDF Match object in NPU.
 *
 * @param[in] udf_match_id UDF Match object Id
 * @param[in] sai_attr  Attribute Id, Value pair for the attribute to be set
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_udf_match_attribute_set_fn) (
                                              sai_object_id_t udf_match_id,
                                              const sai_attribute_t *sai_attr);

/**
 * @brief Get attributes from the UDF Match object in NPU.
 *
 * @param[in] udf_match_id UDF Match object Id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Attribute Id, Value pairs for the UDF Match object
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_udf_match_attribute_get_fn) (
                                                sai_object_id_t udf_match_id,
                                                uint_t attr_count,
                                                sai_attribute_t *attr_list);

/**
 * @brief Create a UDF Group object in NPU.
 *
 * @param[in] p_udf_group Pointer to the UDF Group node
 * @param[out] p_group_id Pointer to UDF Group object Id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_udf_group_create_fn) (
                                             dn_sai_udf_group_t *p_udf_group,
                                             sai_npu_object_id_t *p_group_id);

/**
 * @brief Remove UDF Group object in NPU.
 *
 * @param[in] p_udf_group Pointer to the UDF Group node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_udf_group_remove_fn) (
                                             dn_sai_udf_group_t *p_udf_group);


/**
 * @brief Sets an attribute to the UDF Group object in NPU.
 *
 * @param[in] p_udf_group Pointer to the UDF Group node
 * @param[in] sai_attr  Attribute Id, Value pair for the attribute to be set
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_udf_group_attribute_set_fn) (
                                               dn_sai_udf_group_t *p_udf_group,
                                               const sai_attribute_t *sai_attr);

/**
 * @brief Get attributes from the UDF Group object in NPU.
 *
 * @param[in] p_udf_group Pointer to the UDF Group node
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Attribute Id, Value pairs for the UDF Match object
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_udf_group_attribute_get_fn) (
                                         const dn_sai_udf_group_t *p_udf_group,
                                         uint_t attr_count,
                                         sai_attribute_t *attr_list);

/**
 * @brief Get NPU object id for the UDF Group object.
 *
 * @param[in] p_udf_group Pointer to the UDF Group node
 * @param[out] p_hw_id Pointer to HW Id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_udf_group_hw_id_get_fn) (
                                          const dn_sai_udf_group_t *p_udf_group,
                                          sai_npu_object_id_t *p_hw_id);
/**
 * @brief Create a UDF object in NPU.
 *
 * @param[in] p_udf_obj Pointer to the UDF node
 * @param[out] p_udf_id Pointer to UDF object Id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_udf_create_fn) (dn_sai_udf_t *p_udf_obj,
                                               sai_npu_object_id_t *p_udf_id);

/**
 * @brief Remove UDF object in NPU.
 *
 * @param[in] p_udf_obj Pointer to the UDF node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_udf_remove_fn) (dn_sai_udf_t *p_udf_obj);


/**
 * @brief Sets an attribute to the UDF object in NPU.
 *
 * @param[in] p_udf_obj Pointer to the UDF node
 * @param[in] attr  Attribute Id, Value pair for the attribute to be set
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_udf_attribute_set_fn) (
                                               dn_sai_udf_t *p_udf_obj,
                                               const sai_attribute_t *attr);

/**
 * @brief Get attributes from the UDF object in NPU.
 *
 * @param[in] p_udf_obj Pointer to the UDF node
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Attribute Id, Value pairs for the UDF Match object
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_udf_attribute_get_fn) (
                                                const dn_sai_udf_t *p_udf_obj,
                                                uint_t attr_count,
                                                sai_attribute_t *attr_list);

/**
 * @brief UDF NPU Plugin API table.
 */
typedef struct _sai_npu_udf_api_t {
    sai_npu_udf_init_fn                     udf_init;
    sai_npu_udf_deinit_fn                   udf_deinit;
    sai_npu_udf_match_create_fn             udf_match_create;
    sai_npu_udf_match_remove_fn             udf_match_remove;
    sai_npu_udf_match_attribute_set_fn      udf_match_attribute_set;
    sai_npu_udf_match_attribute_get_fn      udf_match_attribute_get;
    sai_npu_udf_group_create_fn             udf_group_create;
    sai_npu_udf_group_remove_fn             udf_group_remove;
    sai_npu_udf_group_hw_id_get_fn          udf_group_hw_id_get;
    sai_npu_udf_group_attribute_set_fn      udf_group_attribute_set;
    sai_npu_udf_group_attribute_get_fn      udf_group_attribute_get;
    sai_npu_udf_create_fn                   udf_create;
    sai_npu_udf_remove_fn                   udf_remove;
    sai_npu_udf_attribute_set_fn            udf_attribute_set;
    sai_npu_udf_attribute_get_fn            udf_attribute_get;
}sai_npu_udf_api_t;


/**
 * \}
 */


#endif /* _SAI_UDF_NPU_API_H_ */
