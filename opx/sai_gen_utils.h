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
 * \file    sai_gen_utils.h
 *
 * \brief Contains SAI general utility APIs
*/

#if !defined (__SAIGENUTILS_H_)
#define __SAIGENUTILS_H_

#include "saitypes.h"
#include "sai.h"
#include "std_type_defs.h"

#define SAI_NUM_API_ID (SAI_API_MAX)
#define SAI_NUM_API_CUSTOM_ID (SAI_API_CUSTOM_RANGE_END - SAI_API_CUSTOM_RANGE_START)
#define SAI_API_CUSTOM_CHECK(ID) ((ID >= SAI_API_CUSTOM_RANGE_START) && (ID < SAI_API_CUSTOM_RANGE_END))
#define SAI_API_CUSTOM_INDEX(ID) (ID - SAI_API_CUSTOM_RANGE_START)

#ifdef __cplusplus
extern "C"{
#endif

/** SAI GEN API - Check if SAI OID is already in use
    \param[in] id SAI OID
    \return Success: If OID in use
            Failure: Otherwise
*/
typedef bool (*dn_sai_id_in_use_check_fn)(uint64_t id);

/** SAI GEN API - Reentrant buffer for dn_sai_get_next_free_id */
typedef struct _dn_sai_id_gen_info {
    /** current_id: Most recent used id */
    uint64_t cur_id;
    /** is_wrappped: Set if current_id is wrapped around */
    bool is_wrappped;
    /** mask: Max possible ID. Used to identify wrap around */
    uint64_t mask;
    /** is_id_in_use: Function to check if ID is already in use */
    dn_sai_id_in_use_check_fn is_id_in_use;
} dn_sai_id_gen_info_t;

/** SAI GEN API - Get next free unused SAI OID
    \param[in,out] info Pointer to rentrant buffer
    \return Success: SAI_STATUS_SUCCESS cur_id is the newly generated ID
            Failure: SAI_STATUS_FAILURE
 */
sai_status_t dn_sai_get_next_free_id(dn_sai_id_gen_info_t *info);

/** SAI GEN API - Get indexed attribute return type
      \param[in] ret_val Original attribute return val with index 0
      \param[in] attr_index The index of the attribute
      \return Success: Indexed attribute return value
              Failure: SAI_STATUS_INVALID_PARAMETER
*/
sai_status_t sai_get_indexed_ret_val(sai_status_t ret_val,unsigned int attr_index);

/* SAI GEN API - Compare two object nodes for linked list
     \param[in] current Current node in the linked list
     \param[in] node The node to be checked in the list
     \param[in] len The length of the object
     \return 1 if current > node
             0 if current = node
            -1 if current < node
*/
int sai_port_node_compare(const void *current,
                          const void *node, uint_t len);

/** SAI GEN API - Validate if a port list is valid
      \param[in] sai_port_list Port list containing the list of ports
      \param[out] invalid_idx Index of first invalid port
      \return Success: true if list is valid
              Failure: false
*/
bool sai_is_port_list_valid(const sai_object_list_t *sai_port_list, unsigned int *invalid_idx);

/** SAI GEN API - SAI log level set routine
      \param[in] api_id SAI API id
      \param[in] level SAI log level
*/
void sai_log_level_set (sai_api_t api_id, sai_log_level_t level);

/** SAI GEN API - To check if a given SAI log level is enabled for the API
      \param[in] api_id SAI API id
      \param[in] level SAI log level
      \return true if the level is enabled false otherwise
*/
bool sai_is_log_enabled (sai_api_t api_id, sai_log_level_t level);

/** SAI GEN API - To get the switch id in string format for logs
 */
const char* sai_switch_id_str_get (void);

/** SAI GEN API - Initialize SAI log level for the APIs
 */
void sai_log_init (void);

/**
 * @brief Find attribute in the attribute list
 *
 * @param[in] attr_id  Attribute id which needs to be found
 * @param[in] attr_count Count of attributes in the attribute list
 * @param[in] attr_list Attribute list
 * @param[out] attr_index Index of attribute in the list if present in list
 * @param[out] attr_val Attribute value of the attribute if present in list
 * @return SAI_STATUS_SUCCESS if attribute is found else
 *  SAI_STATUS_ITEM_NOT_FOUND is returned.
 */
sai_status_t sai_find_attr_in_attrlist(sai_attr_id_t attr_id,
                                       uint_t attr_count,
                                       const sai_attribute_t *attr_list,
                                       uint_t *attr_index,
                                       const sai_attribute_value_t **attr_val);

/**
 * @brief Check if attribute list has duplicate attributes
 *
 * @param[in] attr_count Count of attributes in the attribute list
 * @param[in] attr_list Attribute list
 * @param[out] dup_index Index of the first duplicate attribute if found
 * @return SAI_STATUS_SUCCESS if duplicate attribute is found else
 *         SAI_STATUS_ITEM_NOT_FOUND is returned.
 */
bool dn_sai_check_duplicate_attr(uint_t attr_count, const sai_attribute_t *attr_list,
                                 uint_t *dup_index);
#ifdef __cplusplus
}
#endif
#endif
