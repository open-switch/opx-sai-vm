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
 * \file    sai_vlan_api.h
 *
 * \brief Declaration of SAI VLAN related APIs
*/

#if !defined (__SAIVLANAPI_H_)
#define __SAIVLANAPI_H_
#include "saivlan.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_vlan_common.h"
#include "std_rbtree.h"

/** SAI VLAN API - Init VLAN Module data structures
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_UNINITIALIZED
*/
sai_status_t sai_vlan_cache_init(void);

/** SAI VLAN API - Check whther if VLAN ID is valid
      \param[in] vlan_id VLAN Identifier
      \return Success: true
              Failure: false
*/
static inline bool sai_is_valid_vlan_id(sai_vlan_id_t vlan_id)
{
    if((vlan_id < SAI_MIN_VLAN_TAG_ID) ||
        (vlan_id > SAI_MAX_VLAN_TAG_ID)) {
        return false;
    }
    return true;
}

/** SAI VLAN API - Check whther if VLAN ID is created
      \param[in] vlan_id VLAN Identifier
      \return Success: true
              Failure: false
*/
bool sai_is_vlan_created(sai_vlan_id_t vlan_id);

/** SAI VLAN API - Insert VLAN in Data cache
      \param[in] vlan_id VLAN Identifier
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_ITEM_ALREADY_EXISTS, SAI_STATUS_NO_MEMORY
*/
sai_status_t sai_insert_vlan_in_list(sai_vlan_id_t vlan_id);

/** SAI VLAN API - Remove VLAN from Data cache
      \param[in] vlan_id VLAN Identifier
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_remove_vlan_from_list(sai_vlan_id_t vlan_id);

/** SAI VLAN API - Insert VLAN member node to VLAN member list
      \param[in] vlan_member_node VLAN member info
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_NO_MEMORY
*/
sai_status_t sai_add_vlan_member_node(sai_vlan_member_node_t vlan_member_node);

/** SAI VLAN API - Find VLAN member dll node given VLAN id and port id
      \param[in] vlan_id VLAN Identifier
      \param[in] bridge_port_id Bridge Port Identifier
      \return Success: A valid pointer to VLAN member dll node
              Failure: NULL
*/
sai_vlan_member_dll_node_t* sai_find_vlan_member_node_from_port(
        sai_vlan_id_t vlan_id, sai_object_id_t bridge_port_id);

/** SAI VLAN API - Find VLAN member rb tree node given VLAN member Identifier
      \param[in] vlan_member_id VLAN Member Identifier
      \return Success: A valid pointer to VLAN member rb tree node
              Failure: NULL
 */
sai_vlan_member_node_t* sai_find_vlan_member_node(
        sai_object_id_t vlan_member_id);

/** SAI VLAN API - Check if port is a member of the given VLAN
      \param[in] vlan_id VLAN Identifier
      \param[in] bridge_port_id Bridge Port Identifier
      \return Success: true
              Failure: false
*/
bool sai_is_bridge_port_vlan_member(sai_vlan_id_t vlan_id, sai_object_id_t bridge_port_id);

/** SAI VLAN API - Remove VLAN Port node from VLAN Port list
      \param[in] vlan_member_node VLAN member info
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_INVALID_PORT_MEMBER
*/
sai_status_t sai_remove_vlan_member_node(sai_vlan_member_node_t vlan_member_node);

/** SAI VLAN API - Initialize internal vlan ID
      \param[in] vlan_id VLAN Identifier
*/
void sai_init_internal_vlan_id(sai_vlan_id_t vlan_id);

/** SAI VLAN API - Check if VLAN ID is internal VLAN ID
      \param[in] vlan_id VLAN Identifier
      \return Success: true
              Failure: false
*/
bool sai_is_internal_vlan_id(sai_vlan_id_t vlan_id);

/** SAI VLAN API - check if internal VLAN ID is initialized.
      \return Success: true
              Failure: false
*/
bool sai_is_internal_vlan_id_initialized(void);

/** SAI VLAN API - Get the internal VLAN ID.
      \return internal vlan id.
*/
sai_vlan_id_t sai_internal_vlan_id_get(void);

/** SAI VLAN API - Get Port list for a VLAN
      \param[in] vlan_id VLAN Identifier
      \param[out] vlan_port_list List of VLAN member ids and their count
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_NO_MEMORY
*/
sai_status_t sai_vlan_port_list_get(sai_vlan_id_t vlan_id, sai_object_list_t *vlan_port_list);
/** SAI VLAN API - Lock VLAN for access
*/
void sai_vlan_lock(void);

/** SAI VLAN API - Unlock VLAN after access
*/
void sai_vlan_unlock(void);

/** SAI VLAN API - Get Port cache list for dump
      \param[in] vlan_id VLAN Identifier
      \return Success: A valid pointer to portlist for vlan in cache
              Failure: NULL
*/
sai_vlan_global_cache_node_t* sai_vlan_portlist_cache_read(sai_vlan_id_t vlan_id);

/** SAI VLAN API - Check if tagging mode is valid
    \param[in] tagging_mode Tagging mode that needs to be checked
    \return Success: true
            Failure: false
*/
bool sai_is_valid_vlan_tagging_mode(sai_vlan_tagging_mode_t tagging_mode);

/** SAI VLAN API - Check if vlan obj in use
    \param[in] vlan_id VLAN Identifier
    \return  true if obj is in use
             flase otherwise
*/
bool sai_is_vlan_obj_in_use(sai_vlan_id_t vlan_id);

/** SAI VLAN API - Convert SAI VLAN uoid to VLAN id
    \param[in] vlan_obj_id SAI VLAN uoid
    \return  VLAN Identifier
*/
sai_vlan_id_t sai_vlan_obj_id_to_vlan_id(sai_object_id_t vlan_obj_id);

/** SAI VLAN API - Convert VLAN id to SAI VLAN uoid
    \param[in] vlan_id VLAN Identifier
    \return  SAI VLAN uoid
*/
sai_object_id_t sai_vlan_id_to_vlan_obj_id(sai_vlan_id_t vlan_id);

/** SAI VLAN API - Returns the RB tree pointer for VLAN member id cache
    \return  RB tree handle
 */
rbtree_handle sai_vlan_global_member_tree_get(void);

/**
 * @brief Set attribute values in vlan cache
 *
 * @param[in] vlan_obj_id VLAN Object identifier
 * @param[in] attr Attribute to be updated
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_vlan_set_attribute_in_cache(sai_object_id_t vlan_obj_id,
                                             const sai_attribute_t* attr);


/**
 * @brief Get attribute value from vlan cache
 *
 * @param[in] vlan_obj_id VLAN Object identifier
 * @param[inout] attr Attribute to be retrieved
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_vlan_get_attribute_from_cache(sai_object_id_t vlan_obj_id,
                                               sai_attribute_t* attr);

#endif
