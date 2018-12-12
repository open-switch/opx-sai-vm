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
* @file sai_stp_api.h
*
* @brief This file contains the prototype declarations for common stp
*        APIs
*
*************************************************************************/
#ifndef __SAI_STP_API_H__
#define __SAI_STP_API_H__

#include "sai_stp_defs.h"
#include "saistp.h"
#include "saitypes.h"

/*
 * Allocate memory for stp info node
 */
dn_sai_stp_info_t *sai_stp_info_node_alloc (void);

/*
 * Free memory for stp info node
 */
void sai_stp_info_node_free (dn_sai_stp_info_t *p_info_info);

/*
 * Allocate memory for vlan node
 */
sai_vlan_id_t * sai_stp_vlan_node_alloc (void);

/*
 * Free memory for vlan node
 */
void sai_stp_vlan_node_free (sai_vlan_id_t *p_vlan_node);

/*
 * Mutex lock for SAI STP
 */
void sai_stp_lock(void);

/*
 * Mutex Unlock for SAI STP
 */
void sai_stp_unlock(void);

/*
 * Mutex lock initialization for SAI STP
 */
void sai_stp_mutex_lock_init (void);

/*
 * Remove the association of any stp instance to the give vlan
 */
sai_status_t sai_stp_vlan_destroy (sai_vlan_id_t vlan_id);

/*
 * Update the stp instance to the given vlan
 */
sai_status_t sai_stp_vlan_handle (sai_vlan_id_t vlan_id, sai_object_id_t stp_inst_id);

/*
 * Update the default stp instance association to the given vlan
 */
sai_status_t sai_stp_default_stp_vlan_update (sai_vlan_id_t vlan_id);

/*
 * Retrieve the stp instance of the given vlan
 */
sai_status_t sai_stp_vlan_stp_get (sai_vlan_id_t vlan_id, sai_object_id_t *p_stp_id);

/*
 * Retrieve the default stp instance
 */
sai_status_t sai_l2_stp_default_instance_id_get (sai_attribute_t *attr);

/*
 * Software cache dump
 */
void sai_stp_dump(void);

/*
 * Read the stp state of port and instance id
 */
sai_status_t sai_npu_stp_port_state_get (sai_object_id_t stp_inst_id,
                                         sai_object_id_t port_id,
                                         sai_stp_port_state_t *port_state);


/*
 * Read STP instance from VLAN ID
 */
sai_object_id_t sai_stp_get_instance_from_vlan_map(sai_vlan_id_t vlan_id);

/*
 * Check if MAC learning is allowed based on STP state
 */
bool sai_stp_can_bridge_port_learn_mac (sai_vlan_id_t vlan_id, sai_object_id_t port_id);

/*
 * Allocate memory for stp info node
 */
dn_sai_stp_port_info_t *sai_stp_port_node_alloc (void);

/*
 * Free memory for stp info node
 */
void sai_stp_port_node_free (dn_sai_stp_port_info_t *p_stp_port_node);

/*
 * Check if SAI STP port state valid
 */
bool sai_stp_port_state_valid(sai_stp_port_state_t port_state);

/*
 * Returns SAI STP global RB tree handle
 */
rbtree_handle sai_stp_global_info_tree_get(void);

/*
 * Returns SAI STP port RB tree handle
 */
rbtree_handle sai_stp_port_tree_get(void);


sai_status_t sai_stp_port_info_create(sai_object_id_t stp_port_id, sai_object_id_t stp_inst_id,
                                      sai_object_id_t bridge_port_id,
                                      sai_stp_port_state_t port_state);

sai_status_t sai_stp_port_info_remove(sai_object_id_t stp_port_id);

sai_status_t sai_stp_info_create(sai_object_id_t stp_inst_id);

sai_status_t sai_stp_inst_valid_for_delete(sai_object_id_t stp_inst_id);

sai_status_t sai_stp_info_remove(sai_object_id_t stp_inst_id);

#endif /* __SAI_STP_API_H__ */
