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
* @file sai_qos_port_util.h
*
* @brief This file contains SAI QOS Port Utility API signatures.
*
*************************************************************************/
#ifndef __SAI_QOS_PORT_UTIL_H__
#define __SAI_QOS_PORT_UTIL_H__

#include "sai_qos_common.h"
#include "saistatus.h"
#include "std_rbtree.h"

/**
 * @brief Initialize the Port Pool RB tree
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_qos_port_pool_tree_init(void);

/**
 * @brief Add the Port Pool node into RB tree
 *
 * @param[in] p_port_pool_node Pointer to Port Pool node
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_qos_add_port_pool_node(dn_sai_qos_port_pool_t *p_port_pool_node);

/**
 * @brief Find the Port Pool node given the SAI Port Pool Object ID
 *
 * @param[in] port_pool_id SAI Port Pool object ID
 *
 * @return Valid pointer to Port Pool node otherwise NULL
 *
 */
dn_sai_qos_port_pool_t* sai_qos_port_pool_node_get_from_obj_id(sai_object_id_t port_pool_id);

/**
 * @brief Find the Port Pool node given SAI Port object ID and SAI Pool object ID
 *
 * @param[in] port_id SAI Port object ID
 * @param[in] pool_id SAI Pool object ID
 *
 * @return Valid pointer to Port Pool node otherwise NULL
 *
 */
dn_sai_qos_port_pool_t* sai_qos_port_pool_node_get(sai_object_id_t port_id, sai_object_id_t pool_id);

/**
 * @brief Remove the Port Pool node from RB tree given the Port Pool ID
 *
 * @param[in] port_pool_id SAI Port Pool object ID
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 *
 */
sai_status_t sai_qos_remove_port_pool_node(sai_object_id_t port_pool_id);

/**
 * @brief Get unicast front end port queues that belong to the port pool based on count
 * @param[in] port_pool_id Port pool ID
 * @param[out] p_queue_list Pointer to queue list buffer. Modifies the list count
 *                          based on the number of queues updated in the list
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 * error code is returned.
 */
sai_status_t sai_qos_port_pool_get_wred_queue_ids(sai_object_id_t port_pool_id,
        sai_object_list_t *p_queue_list);

/**
 * @brief Gets the first port pool node attached to QOS Port node
 *
 * @param[in] p_port_node Pointer to the QOS Port node
 *
 * @return Valid pointer to Port Pool node otherwise NULL
 *
 */
dn_sai_qos_port_pool_t *sai_qos_port_pool_get_first_node(dn_sai_qos_port_t *p_port_node);

/**
 * @brief Gets the next port pool node attached to QOS Port node
 *
 * @param[in] p_port_node Pointer to the QOS Port node
 * @param[in] p_port_pool_node Pointer to the current Port Pool node
 *
 * @return Valid pointer to Port Pool node otherwise NULL
 *
 */
dn_sai_qos_port_pool_t *sai_qos_port_pool_get_next_node(dn_sai_qos_port_t *p_port_node,
        dn_sai_qos_port_pool_t *p_port_pool_node);

#endif /*__SAI_QOS_PORT_UTIL_H__*/
