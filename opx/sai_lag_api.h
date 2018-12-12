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
 * \file    sai_lag_api.h
 *
 * \brief Declaration of SAI LAG related APIs
*/

#if !defined (__SAILAGAPI_H_)
#define __SAILAGAPI_H_
#include "saitypes.h"
#include "saistatus.h"
#include "sai_lag_common.h"
#include "sai_oid_utils.h"
/** SAI LAG API - Init LAG Module data structures
*/
void sai_lag_cache_init(void);

/** SAI LAG API - Check whther if LAG ID is created
      \param[in] lag_id LAG Identifier
      \return Success: true
              Failure: false
*/
bool sai_is_lag_created(sai_object_id_t lag_id);

/** SAI LAG API - Add a LAG node in cache
      \param[in] lag_id LAG Identifier
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_NO_MEMORY
*/
sai_status_t sai_lag_node_add(sai_object_id_t lag_id);

/** SAI LAG API - Remove LAG node from cache
      \param[in] lag_id LAG Identifier
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_lag_node_remove(sai_object_id_t lag_id);

/** SAI LAG API - Get first lag node
      \return Success: A valid pointer to LAG node
              Failure: NULL
*/
sai_lag_node_t* sai_lag_get_first_node (void);

/** SAI LAG API - Get next lag node
      \param[in] lag_node Current LAG node
      \return Success: A valid pointer to LAG node
              Failure: NULL
*/
sai_lag_node_t* sai_lag_get_next_node (sai_lag_node_t* lag_node);

/** SAI LAG API - Add a Port node to LAG Node
      \param[in] lag_id LAG Identifier
      \param[in] port_id Port Identifier
      \param[in] member_id NPU layer generated object id associated
                 with the lag member port. See \sa sai_create_lag_member_fn
                 for details on how member_id is generated.
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_NO_MEMORY, SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_lag_port_node_add(sai_object_id_t lag_id,
                                   sai_object_id_t port_id,
                                   sai_object_id_t member_id);

/** SAI LAG API - Remove LAG Port node from LAG Port list
      \param[in] lag_id LAG Identifier
      \param[in] port_id Port Identifier
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_lag_port_node_remove(sai_object_id_t lag_id,
                                      sai_object_id_t port_id);


/** SAI LAG API - Check if port is part of LAG
      \param[in] lag_id LAG Identifier
      \param[in] port_id Port Identifier
      \return Success: true
              Failure: false
*/
bool sai_is_port_lag_member(sai_object_id_t lag_id,
                            sai_object_id_t port_id);

/** SAI LAG API - Check if port is part of a different LAGLAG
      \param[in] lag_id LAG Identifier
      \param[in] port_id Port Identifier
      \return Success: true
              Failure: false
*/
bool sai_is_port_part_of_different_lag(sai_object_id_t lag_id,
                                       sai_object_id_t port_id);

/** SAI LAG API - Get Port list for a LAG
      \param[in] lag_id LAG Identifier
      \param[in][out] lag_port_list List of ports and port count
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_NO_MEMORY
*/
sai_status_t sai_lag_port_list_get(sai_object_id_t lag_id,
                                   sai_object_list_t *lag_port_list);

/** SAI LAG API - Get number of ports for a LAG
      \param[in] lag_id LAG Identifier
      \param[out] port_count The number of ports in the LAG
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_lag_port_count_get(sai_object_id_t sai_lag_id,
                                    unsigned int *port_count);

/** SAI LAG API - Get NPU ID from SAI LAG Object ID
      \param[in] lag_id LAG Identifier
      \param[out] npu_lag_id NPU LAG Identifier
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_INVALID_OBJECT_TYPE
*/
sai_status_t sai_lag_npu_object_id_get(sai_object_id_t lag_id,
                                       sai_npu_object_id_t *npu_lag_id);

/** SAI LAG API - Lock LAG for access
*/
void sai_lag_lock(void);

/** SAI LAG API - Get the list of LAGs for Dump
    \return A Valid pointer to cache
*/
std_dll_head* sai_lag_list_get(void);

/** SAI LAG API - Get the Lag node to dump
    \param[in] lag_id LAG Identifier
    \return Success: A valid pointer to lag node in cache
            Failure: NULL
*/
sai_lag_node_t* sai_lag_node_get(sai_object_id_t lag_id);

/** SAI LAG API - Unlock LAG after access
*/
void sai_lag_unlock(void);

/** SAI LAG API - Get the Lag member port id, given the member-id
      \param[in] lag_id LAG Identifier
      \param[in] member_id LAG member object id.
      \param[out] port_id Port Id of the lag member.
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_lag_get_port_id_from_member_id (sai_object_id_t  lag_id,
                                                 sai_object_id_t  member_id,
                                                 sai_object_id_t *port_id);

/** SAI LAG API - Get the Lag Id and member port id, given the member-id
      \param[in] member_id LAG member object id.
      \param[out] lag_id LAG Identifier
      \param[out] port_id Port Id of the lag member.
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_lag_get_info_from_member_id (sai_object_id_t  member_id,
                                              sai_object_id_t *lag_id,
                                              sai_object_id_t *port_id);

/** SAI LAG API - Get the [ing/egr] disable status of the given Lag member
      \param[in] lag_id LAG Identifier
      \param[in] port_id LAG member port identifier
      \param[in] is_ingress Direction - true if ingress, else false
      \param[out] status [ing/egr] disable status of the given Lag member
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_INVALID_PARAMETER
*/
sai_status_t sai_lag_member_get_disable_status (sai_object_id_t  lag_id,
                                                sai_object_id_t  port_id,
                                                bool             is_ingress,
                                                bool            *status);

/** SAI LAG API - Set the [ing/egr] disable status of the given Lag member
      \param[in] lag_id LAG Identifier
      \param[in] port_id LAG member port identifier
      \param[in] is_ingress Direction - true if ingress, else false
      \param[in] status [ing/egr] disable status of the given Lag member
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_INVALID_PARAMETER
*/
sai_status_t sai_lag_member_set_disable_status (sai_object_id_t lag_id,
                                                sai_object_id_t port_id,
                                                bool            is_ingress,
                                                bool            status);

/** SAI LAG API - Get the member_id of the given Lag member
      \param[in] lag_id LAG Identifier
      \param[in] port_id LAG member port identifier
      \param[out] member_id Member Id of the given Lag member
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_INVALID_PARAMETER
*/
sai_status_t sai_lag_member_get_member_id (sai_object_id_t  lag_id,
                                           sai_object_id_t  port_id,
                                           sai_object_id_t *member_id);

/** SAI LAG API - Set the member_id of the given Lag member
      \param[in] lag_id LAG Identifier
      \param[in] port_id LAG member port identifier
      \param[in] member_id Member Id to be set for the given Lag member
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_INVALID_PARAMETER
*/
sai_status_t sai_lag_member_set_member_id (sai_object_id_t lag_id,
                                           sai_object_id_t port_id,
                                           sai_object_id_t member_id);

/**
 * @brief Increment LAG reference count
 *
 * @param[in] lag_id SAI LAG identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_lag_increment_ref_count(sai_object_id_t lag_id);

/**
 * @brief Decrement LAG reference count
 *
 * @param[in] lag_id SAI LAG identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_lag_decrement_ref_count(sai_object_id_t lag_id);

/**
 * @brief Set the default 1Q bridge port associated with the LAG
 *
 * @param[in] lag_id SAI LAG identifier
 * @param[in] bridge_port_id Bridge port identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_lag_def_bridge_port_set(sai_object_id_t lag_id, sai_object_id_t bridge_port_id);


/**
 * @brief Get the default 1Q bridge port associated for the lag from lag cache
 *
 * @param[in] lag_id SAI LAG identifier
 * @param[out] bridge_port_id Bridge port identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_lag_def_bridge_port_get (sai_object_id_t lag_id, sai_object_id_t *bridge_port_id);

/**
 * @brief Check if the LAG is in use
 *
 * @param[in] lag_id SAI LAG identifier
 * @return true if ref_count > 0, false otherwise
 *
 */
bool sai_is_lag_in_use (sai_object_id_t lag_id);

/**
 * @brief Get the forwarding mode of the lag
 *
 * @param[in] lag_id SAI LAG identifier
 * @param[out] fwd_mode Forwarding mode
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_lag_forwarding_mode_get(sai_object_id_t lag_id, sai_port_fwd_mode_t *fwd_mode);

/**
 * @brief Set the forwarding mode of the lag
 *
 * @param[in] lag_id SAI LAG identifier
 * @param[in] fwd_mode Forwarding mode
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_lag_forwarding_mode_set(sai_object_id_t lag_id, sai_port_fwd_mode_t fwd_mode);

/**
 * @brief Set the attribute in lag node
 *
 * @param[inout] lag_node Pointer to LAG node to be updated
 * @param[in] attr Attribute to be set
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_lag_node_set_attr (sai_lag_node_t *lag_node, const sai_attribute_t *attr);

/**
 * @brief Get the attribute from lag node
 *
 * @param[in] lag_node Pointer to LAG node to be updated
 * @param[inout] attr Attribute to be retrieved
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_lag_node_get_attr (const sai_lag_node_t *lag_node, sai_attribute_t *attr);

/**
 * @brief Get PVID of LAG
 *
 * @param[in] lag_id LAG Object Identifier
 * @param[out] PVID assigned to the LAG
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_lag_pvid_get(sai_object_id_t lag_id, uint16_t *pvid);
#endif

