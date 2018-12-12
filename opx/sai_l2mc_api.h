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
 * \file    sai_l2mc_api.h
 *
 * \brief Declaration of SAI L2MC related APIs
*/

#if !defined (__SAIL2MCAPI_H_)
#define __SAIL2MCAPI_H_
#include "saitypes.h"
#include "saistatus.h"
#include "sai_l2mc_common.h"

/** SAI L2MC API - Init L2MC Module data structures
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_UNINITIALIZED
*/
sai_status_t sai_l2mc_tree_init(void);

/** SAI L2MC API - Lock for accessing L2MC tree
*/
void sai_l2mc_lock(void);
/** SAI L2MC API - API to release the L2MC lock
*/
void sai_l2mc_unlock(void);

/** SAI L2MC API - Insert L2MC group node to the tree
    \param[in] l2mc_group_info l2mc group info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_FAILURE
*/
sai_status_t sai_add_l2mc_group_node(dn_sai_l2mc_group_node_t *l2mc_group_info);
/** SAI L2MC API - Remove L2MC group node from the tree
    \param[in] l2mc_group_info l2mc group info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_OBJECT_IN_USE, SAI_STATUS_INVALID_PARAMETER
            SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_remove_l2mc_group_node(dn_sai_l2mc_group_node_t *l2mc_group_info);
/** SAI L2MC API - Find L2mc Group node from the tree
    \param[in] l2mc group object id
    \return A Valid pointer to the l2mc group node in the tree else NULL
*/
dn_sai_l2mc_group_node_t * sai_find_l2mc_group_node(sai_object_id_t l2mc_group_id);
/** SAI L2MC API - Add l2mc member node to the member tree
    \param[in] l2mc_member_node_t member node info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_FAILURE, SAI_STATUS_ITEM_NOT_FOUND
                     SAI_STATUS_NO_MEMORY
*/
sai_status_t sai_add_l2mc_member_node(dn_sai_l2mc_member_node_t l2mc_member_info);
/** SAI L2MC API - Find L2mc Member node from the tree
    \param[in] l2mc Member object id
    \return A Valid pointer to the l2mc Member node in the tree else NULL
 */
dn_sai_l2mc_member_node_t* sai_find_l2mc_member_node( sai_object_id_t l2mc_member_id);
/** SAI L2MC API - Remove l2mc member node from the member tree
    \param[in] l2mc_member_node_t member node info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_INVALID_PORT_MEMBER, SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_remove_l2mc_member_node(dn_sai_l2mc_member_node_t *l2mc_member_info);
/** SAI L2MC API - Find the L2mc member node for the l2mc group and Port
    \param[in] l2mc_group_node group node
    \param[in] sai_object_id_t port object id
    \return A Valid pointer to the l2mc member node or NULL
*/
dn_sai_l2mc_member_dll_node_t* sai_find_l2mc_member_node_from_port(
        dn_sai_l2mc_group_node_t * l2mc_group_node,
        sai_object_id_t port_id);
/** SAI L2MC API - Get the list of ports from the l2mc group
    \param[in] l2mc_group_node group node
    \param[in] sai_object_list_t List for filling the port members
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_BUFFER_OVERFLOW
*/
sai_status_t sai_l2mc_port_list_get(dn_sai_l2mc_group_node_t *l2mc_group_node,
        sai_object_list_t *l2mc_port_list);

/** SAI L2MC API - Check if member port's bridge id is valid for this multicast group
    \param[in] l2mc_group_node group node
    \param[in] bridge_port_id Bridge port ID that is to be added
    \return Success: true
            Failure: false
*/
bool sai_l2mc_is_valid_member_bridge_id(const dn_sai_l2mc_group_node_t *l2mc_group_node,
                                        sai_object_id_t bridge_port_id);


/** SAI L2MC API - Associated a bridge id to l2mc group
    \param[in] l2mc_group_id L2MC Group Object ID
    \param[in] bridge_id Bridge ID that L2MC Group is attached
    \return Success: SAI_STATUS_SUCCESS
            Failure: Appropriate error code will be returned
*/
sai_status_t sai_l2mc_add_bridge_id(sai_object_id_t l2mc_group_id,
                                    sai_object_id_t bridge_id);

/** SAI L2MC API - Remove bridge id from l2mc group
    \param[in] l2mc_group_id L2MC Group Object ID
    \param[in] bridge_id Bridge ID that L2MC Group is attached
    \return Success: SAI_STATUS_SUCCESS
            Failure: Appropriate error code will be returned
*/
sai_status_t sai_l2mc_remove_bridge_id(sai_object_id_t l2mc_group_id,
                                                sai_object_id_t bridge_id);

/** SAI L2MC API - Get Next L2MC Group
    \param[in] l2mc_group_node Current L2MC group node
    \return Success: A Valid pointer to L2MC group node in cache
            Failure: NULL
*/
dn_sai_l2mc_group_node_t* sai_l2mc_group_get_next(dn_sai_l2mc_group_node_t* l2mc_group_node);

/** SAI L2MC API - Get Next L2MC Member
    \param[in] l2mc_group_node Current L2MC Member node
    \return Success: A Valid pointer to L2MC Member node in cache
            Failure: NULL
*/
dn_sai_l2mc_member_node_t* sai_l2mc_member_get_next(dn_sai_l2mc_member_node_t* l2mc_member_node);

void sai_l2mc_group_dump_all(void);

void sai_l2mc_group_dump(sai_object_id_t l2mc_group_obj);

void sai_l2mc_member_dump_all(void);

void sai_l2mc_member_dump(sai_object_id_t l2mc_member_obj);

#endif
