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
* @file sai_samplepacket_api.h
*
* @brief This file contains the prototype declarations for common samplepacket
*        APIs
*
*************************************************************************/
#ifndef __SAI_SAMPLEPACKET_API_H__
#define __SAI_SAMPLEPACKET_API_H__

#include "sai_samplepacket_defs.h"

/*
 * Allocate memory for samplepacket session node
 * return Allocated memory of type (dn_sai_samplepacket_session_info_t *)
 */
dn_sai_samplepacket_session_info_t *sai_samplepacket_session_node_alloc (void);

/*
 * Free memory for samplepacket session node
 * param - p_session_info pointer to be freed
 */
void sai_samplepacket_session_node_free (dn_sai_samplepacket_session_info_t *p_session_info);

/*
 * Allocate memory for source port node
 * return Allocated memory of type (dn_sai_samplepacket_port_info_t *)
 */
dn_sai_samplepacket_port_info_t * sai_samplepacket_port_node_alloc (void);

/*
 * Free memory for source port node
 * param - p_node_info pointer to be freed
 */
void sai_samplepacket_port_node_free (dn_sai_samplepacket_port_info_t *p_node_info);

/*
 * Attach a source port to a samplepacket session
 * param - session_id Samplepacket session Id
 * param - samplepacket_port_id Samplepacket source port Id
 * param - direction Samplepacketing direction of the source port
 * return - SAI_STATUS_SUCCESS if successful otherwise appropriate error code
 */
sai_status_t sai_samplepacket_session_port_add  (sai_object_id_t session_id,
                                           sai_object_id_t samplepacket_port_id,
                                           sai_samplepacket_direction_t direction,
                                           dn_sai_samplepacket_mode_t sample_mode);

/*
 * Detach a source port to a samplepacket session
 * param - session_id Samplepacket session Id
 * param - samplepacket_port_id Samplepacket source port Id
 * param - direction Sampling direction of the source port
 * param - sample_mode flow_based or port_based sampling
 * return - SAI_STATUS_SUCCESS if successful otherwise appropriate error code
 */
sai_status_t sai_samplepacket_session_port_remove (sai_object_id_t session_id,
                                             sai_object_id_t samplepacket_port_id,
                                             sai_samplepacket_direction_t direction,
                                             dn_sai_samplepacket_mode_t sample_mode);

/*
 * Handle port samplepacketing for a port
 * param - port_id Samplepacket source port Id
 * param - attr Attribute value to be set (Samplepacket session list) on the port
 * param - samplepacket_direction Samplepacketing direction of the source port
 * return - SAI_STATUS_SUCCESS if successful otherwise appropriate error code
 */
sai_status_t sai_samplepacket_handle_per_port (sai_object_id_t port_id,
                                         const sai_attribute_t *attr,
                                         sai_samplepacket_direction_t samplepacket_direction);

/*
 * Mutex lock for SAI Samplepacket Session Datastructure
 */
void sai_samplepacket_lock(void);

/*
 * Mutex Unlock for SAI Samplepacket Session Datastructure
 */
void sai_samplepacket_unlock(void);

/*
 * Mutex lock initialization for SAI Samplepacket Session Datastructure
 */
void sai_samplepacket_mutex_lock_init (void);

/*
 * Checks if Samplepacket session is available
 * param - samplepacket_object Samplepacket session Id
 * return - true if samplepacket object is valid false otherwise
 */
bool sai_samplepacket_is_valid_samplepacket_session (sai_object_id_t samplepacket_object);

/*
 * Checks if the samplepacket session is valid to be appiled on a acl and cached.
 */
sai_status_t sai_samplepacket_validate_object (sai_object_list_t *portlist,
                            sai_object_id_t sample_object,
                            sai_samplepacket_direction_t sample_direction,
                            bool validate,
                            bool update);

/*
 * Initializes acl specific samplepacket datastructure
 */
sai_status_t sai_acl_samplepacket_init ();

/*
 * Removes acl specific samplepacket object and clear the config
 */
sai_status_t sai_samplepacket_remove_object (sai_object_list_t *portlist,
        sai_object_id_t sample_object,
        sai_samplepacket_direction_t sample_direction);

/*
 * Retrieve the samplepacket datastructure
 */
rbtree_handle sai_samplepacket_sessions_db_get(void);

/*
 * Dump function for samplepacket session params
 * param - samplepacket session Id
 * param - all Set to dump 'all' or single session
 * return - void
 */
void sai_samplepacket_dump(sai_object_id_t session_id, bool all);

/*
 * Dump properties of a samplepacket session
 */
void sai_samplepacket_dump_session_node (sai_object_id_t samplepacket_session_id);

sai_status_t sai_samplepacket_port_cleanup_on_fanout (sai_object_id_t port_id);

sai_status_t sai_samplepacket_port_config_post_fanout (sai_object_id_t port_id);
#endif /* __SAI_SAMPLEPACKET_API_H__ */
