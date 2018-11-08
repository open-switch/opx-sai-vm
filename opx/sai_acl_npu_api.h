/************************************************************************
 * * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
 * *
 * * This source code is confidential, proprietary, and contains trade
 * * secrets that are the sole property of Dell Inc.
 * * Copy and/or distribution of this source code or disassembly or reverse
 * * engineering of the resultant object code are strictly forbidden without
 * * the written consent of Dell Inc.
 * *
 * ************************************************************************/
/**
 * * @file sai_acl_npu_api.h
 * *
 * * @brief This file contains the API signatures defined for SAI ACL component.
 * *
 * *************************************************************************/

#ifndef _SAI_ACL_NPU_API_H_
#define _SAI_ACL_NPU_API_H_

#include "sai_common_utils.h"
#include "sai_acl_type_defs.h"
#include "std_type_defs.h"
#include "saistatus.h"
#include "sai_qos_common.h"

/** \defgroup SAIACLNPUAPIs SAI - ACL NPU specific function implementations
 *  NPU specific functions for SAI ACL component
 *
 *  \{
 */

/**
 * @brief Create a ACL Table in NPU.
 *
 * @param[inout] acl_table Pointer to the ACL Table node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_create_acl_table_fn)(sai_acl_table_t *acl_table);

/**
 * @brief Delete ACL Table from NPU.
 *
 * @param[inout] acl_table  Pointer to the ACL Table node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_delete_acl_table_fn)(sai_acl_table_t *acl_table);

/**
 * @brief NPU ACL Table Field validation.
 *
 * @param[in] acl_table  Pointer to the ACL Table node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_validate_acl_table_field_fn)(sai_acl_table_t *acl_table);

/**
 * @brief Create ACL Rule in NPU.
 *
 * @param[inout] acl_table  Pointer to the ACL Table node
 * @param[inout] acl_rule  Pointer to the ACL Rule node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_create_acl_rule_fn)(sai_acl_table_t *acl_table,
                                                   sai_acl_rule_t *acl_rule);

/**
 * @brief Delete ACL Rule from NPU.
 *
 * @param[inout] acl_table  Pointer to the ACL Table node
 * @param[inout] acl_rule  Pointer to the ACL Rule node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_delete_acl_rule_fn)(sai_acl_table_t *acl_table,
                                                   sai_acl_rule_t *acl_rule);

/**
 * @brief Copy ACL Rule NPU specific information
 *
 * @param[inout] src_rule  Pointer to ACL Rule to which information
 *                             needs to be copied
 * @param[in] dst_rule  Pointer to ACL Rule from which information
 *                          needs to be copied
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_copy_acl_rule_fn)(sai_acl_rule_t *src_rule,
                                                 sai_acl_rule_t *dst_rule);

/**
 * @brief Set ACL Rule Attributes in NPU
 *
 * @param[in] acl_table  Pointer to the ACL Table node
 * @param[in] set_rule   Pointer to ACL Rule derived from set ACL Attribute list
 * @param[inout] compare_rule    Pointer to ACL Rule used for comparison
 * @param[in] given_rule    Pointer to original unmodified ACL rule
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_set_acl_rule_fn)(sai_acl_table_t *acl_table,
                                                sai_acl_rule_t *set_rule,
                                                sai_acl_rule_t *compare_rule,
                                                sai_acl_rule_t *given_rule);

/**
 * @brief Free ACL Rule NPU related data structures
 *
 * @param[inout] acl_rule  Pointer to ACL Rule for which NPU related
 *                             information needs to be freed
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_free_acl_rule_fn)(sai_acl_rule_t *acl_rule);

/**
 * @brief Create ACL Counter in NPU.
 *
 * @param[in] acl_table  Pointer to the ACL Table node
 * @param[inout] acl_cntr  Pointer to the ACL Counter node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_create_acl_cntr_fn)(sai_acl_table_t *acl_table,
                                                   sai_acl_counter_t *acl_cntr);

/**
 * @brief Delete ACL Counter from NPU.
 *
 * @param[inout] acl_cntr  Pointer to the ACL Counter node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_delete_acl_cntr_fn)(sai_acl_counter_t *acl_cntr);

/**
 * @brief Set ACL Counter Attributes in NPU
 *
 * @param[in] acl_cntr    Pointer to ACL Counter node
 * @param[in] count_value   Count value that needs to be set in NPU
 * @param[in] byte_val      Bool to indicate the type of counter that needs to be set
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_set_acl_cntr_fn)(sai_acl_counter_t *acl_cntr,
                                                uint64_t count_value, bool byte_val);

/**
 * @brief Get ACL Counter Attributes from NPU
 *
 * @param[in] acl_cntr  Pointer to ACL Counter node
 * @param[in] num_count Number of counter values to fetch
 * @param[out] count_value   Count value fetch from  NPU
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_get_acl_cntr_fn)(sai_acl_counter_t *acl_cntr,
                                                uint_t num_count,
                                                uint64_t *count_value);

/**
 * @brief Attach ACL Counter to ACL Rule in NPU
 *
 * @param[inout] acl_rule  Pointer to ACL Rule node
 * @param[inout] acl_cntr  Pointer to ACL Counter node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_attach_cntr_to_acl_rule_fn)(sai_acl_rule_t *acl_rule,
                                                           sai_acl_counter_t *acl_cntr);

/**
 * @brief Detach ACL Counter from ACL Rule in NPU
 *
 * @param[inout] acl_rule  Pointer to ACL Rule node
 * @param[inout] acl_cntr  Pointer to ACL Counter node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_detach_cntr_from_acl_rule_fn)(sai_acl_rule_t *acl_rule,
                                                             sai_acl_counter_t *acl_cntr);

/**
 * @brief Update Policer acl entries on policer update
 *
 * @param[in] p_policer Pointer to existing Policer node
 * @param[in] p_policer_new  Pointer to new Policer node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_update_policer_acl_rule_fn)(dn_sai_qos_policer_t *p_policer,
                                                           dn_sai_qos_policer_t *p_policer_new);

/**
 * @brief Based on attribute, fetch respective priority
 *
 * @param[in] attr Pointer to SAI attribute
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_get_acl_attribute_fn)(sai_attribute_t *attr);

/**
 * @brief Dump NPU specific information contained in the ACL Table node
 *
 * @param[in] acl_table ACL Table node
 */
typedef void (*sai_npu_dump_acl_table)(const sai_acl_table_t *acl_table);

/**
 * @brief Dump NPU specific information contained in the ACL Rule node
 *
 * @param[in] acl_rule ACL Rule node
 */
typedef void (*sai_npu_dump_acl_rule)(const sai_acl_rule_t *acl_rule);

/**
 * @brief Dump NPU specific information contained in the ACL Counter node
 *
 * @param[in] acl_counter ACL Counter node
 */
typedef void (*sai_npu_dump_acl_counter)(const sai_acl_counter_t *acl_counter);

/**
 * @brief Create ACL range id in NPU
 *
 * @param[in] acl_range Pointer to acl range node
 */
typedef sai_status_t (*sai_npu_create_acl_range_fn) (sai_acl_range_t *acl_range);

/**
 * @brief Delete ACL range id in NPU
 *
 * @param[in] acl_range Pointer to acl range node
 */
typedef sai_status_t (*sai_npu_delete_acl_range_fn) (sai_acl_range_t *acl_range);

/**
 * @brief Set ACL range id attributes in NPU
 *
 * @param[in] acl_range  Pointer to acl range node
 * @param[in] attr_count Number of attributes to set
 * @param[in] p_attr     Pointer to the attribute list
 */
typedef sai_status_t (*sai_npu_range_set_fn)(sai_acl_range_t *acl_range,
                                             uint_t attr_count, const sai_attribute_t *p_attr);

/**
 * @brief Set ACL range id attributes in NPU
 *
 * @param[in] acl_range  Pointer to acl range node
 * @param[in] attr_count Number of attributes to set
 * @param[inout] p_attr_list  Pointer to the attribute list to get
 */
typedef sai_status_t (*sai_npu_range_get_fn)(sai_acl_range_t *acl_range,
                                             uint_t attr_count, sai_attribute_t *p_attr_list);

/**
 *  @brief Get Vendor attribute properties table
 *
 * @param[out] vendor_attr Vendor attribute table
 * @param[out] max_count Maximum attribute count
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef void (*sai_npu_attribute_table_get_fn) (
                                            const dn_sai_attribute_entry_t **p_vendor_attr,
                                            uint_t *p_max_count);

/**
 *  @brief Dump the counters from all acl entries.
 *
 */
typedef void (*sai_npu_dump_counters) (void);

/**
 *  @brief Dump the counter of a specific entry.
 *
 *  @param[in] entry_id Entry for which counter should be dumped.
 */
typedef void (*sai_npu_dump_counter_per_entry) (int entry_id);

/**
 * @brief Get ACL slice attribute
 *
 * @param[in] acl_slice Pointer to the ACL Slice node
 * @param[in] p_attr    Pointer to the attribute list
 */
typedef sai_status_t (*sai_npu_attribute_acl_slice_get_fn)(sai_object_id_t acl_slice_id,
                                                            uint32_t attr_count,
                                                            sai_attribute_t *attr_list);
/**
 * @brief ACL NPU API table.
 */
typedef struct _sai_npu_acl_api_t {
    sai_npu_create_acl_table_fn               create_acl_table;
    sai_npu_delete_acl_table_fn               delete_acl_table;
    sai_npu_validate_acl_table_field_fn       validate_acl_table_field;
    sai_npu_create_acl_rule_fn                create_acl_rule;
    sai_npu_delete_acl_rule_fn                delete_acl_rule;
    sai_npu_copy_acl_rule_fn                  copy_acl_rule;
    sai_npu_set_acl_rule_fn                   set_acl_rule;
    sai_npu_free_acl_rule_fn                  free_acl_rule;
    sai_npu_create_acl_cntr_fn                create_acl_cntr;
    sai_npu_delete_acl_cntr_fn                delete_acl_cntr;
    sai_npu_set_acl_cntr_fn                   set_acl_cntr;
    sai_npu_get_acl_cntr_fn                   get_acl_cntr;
    sai_npu_attach_cntr_to_acl_rule_fn        attach_cntr_to_acl_rule;
    sai_npu_detach_cntr_from_acl_rule_fn      detach_cntr_from_acl_rule;
    sai_npu_update_policer_acl_rule_fn        update_policer_acl_rule;
    sai_npu_get_acl_attribute_fn              get_acl_attribute;
    sai_npu_dump_acl_table                    dump_acl_table;
    sai_npu_dump_acl_rule                     dump_acl_rule;
    sai_npu_dump_acl_counter                  dump_acl_counter;
    sai_npu_create_acl_range_fn               create_acl_range;
    sai_npu_delete_acl_range_fn               delete_acl_range;
    sai_npu_range_set_fn                      set_acl_range;
    sai_npu_range_get_fn                      get_acl_range;
    sai_npu_attribute_table_get_fn            attribute_table_get;
    sai_npu_dump_counters                     dump_all_counters;
    sai_npu_dump_counter_per_entry            dump_entry_counter;
    sai_npu_attribute_acl_slice_get_fn        get_acl_slice_attribute;
} sai_npu_acl_api_t;

/**
 * \}
 */

#endif /* _SAI_ACL_NPU_API_H_ */
