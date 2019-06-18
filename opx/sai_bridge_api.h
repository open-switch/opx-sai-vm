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
 * \file    sai_bridge_api.h
 *
 * \brief Declaration of SAI BRIDGE related APIs
*/

#if !defined (__SAIBRIDGEAPI_H_)
#define __SAIBRIDGEAPI_H_
#include "saibridge.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_bridge_common.h"

/** \defgroup SAIBRIDGEAPI SAI - Bridge Utility functions
 *  Util functions in the SAI Bridge component
 *
 *  \{
 */

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @brief Write into bridge cache info for the Bridge ID
 *
 * @param[in] bridge_id Bridge SAI Object identifier
 * @param[in] bridge_info Pointer to bridge info structure
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_cache_write (sai_object_id_t bridge_id, const dn_sai_bridge_info_t *bridge_info);

/**
 * @brief Delete bridge cache info for the Bridge ID
 *
 * @param[in] bridge_id Bridge SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_cache_delete (sai_object_id_t bridge_id);

/**
 * @brief Read bridge cache info for the Bridge ID
 *
 * @param[in] bridge_id Bridge SAI Object identifier
 * @param[inout] p_bridge_info Double Pointer to bridge info structure
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_cache_read (sai_object_id_t bridge_id, dn_sai_bridge_info_t **bridge_info);

/**
 * @brief Write into bridge_port cache info for the Bridge port ID
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[in] bridge_port_info Pointer to Bridge port info structure
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_cache_write (sai_object_id_t bridge_port_id,
                                          const dn_sai_bridge_port_info_t *bridge_port_info);

/**
 * @brief Delete bridge port cache info for the Bridge port ID
 *
 * @param[in] bridge_id Bridge port SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_cache_delete (sai_object_id_t bridge_port_id);

/**
 * @brief Read bridge_port cache info for the Bridge port ID
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[inout] p_bridge_port_info Double Pointer to bridge port info structure
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_cache_read (sai_object_id_t bridge_port_id,
                                         dn_sai_bridge_port_info_t **bridge_port_info);

/**
 * @brief Check if bridge is created
 *
 * @param[in] bridge_id Bridge SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
bool sai_is_bridge_created (sai_object_id_t bridge_id);

/**
 * @brief Check if bridge port is created
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
bool sai_is_bridge_port_created (sai_object_id_t bridge_port_id);

/**
 * @brief Get total number of bridges
 *
 * @return Returns the total number of bridges in the system
 */
uint_t sai_bridge_total_count(void);

/**
 * @brief Get total number of bridge ports
 *
 * @return Returns the total number of bridge ports in the system
 */
uint_t sai_bridge_port_total_count(void);

/**
 * @brief Get the list of bridges in the system
 *
 * @param[inout] count Size of bridge_list. During out it has Number of bridges in the system.
 * @param[out] bridge_list List of bridge IDs in the system
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_list_get(uint_t *count, sai_object_id_t *bridge_list);

/**
 * @brief Get the list of bridge ports in the system
 *
 * @param[inout] count Size of bridge_port_list. During out it has Number of bridge ports in the system.
 * @param[out] bridge_port_list List of bridge port IDs in the system
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_list_get(uint_t *count, sai_object_id_t *bridge_port_list);

/**
 * @brief Map the bridge port to bridge
 *
 * @param[in] bridge_id Bridge SAI Object identifier
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_map_insert (sai_object_id_t bridge_id, sai_object_id_t bridge_port_id);

/**
 * @brief Remove bridge port mapping to the bridge
 *
 * @param[in] bridge_id Bridge SAI Object identifier
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_map_remove (sai_object_id_t bridge_id, sai_object_id_t bridge_port_id);

/**
 * @brief Get the list of bridge ports mapped to a bridge
 *
 * @param[in] bridge_id Bridge SAI Object identifier
 * @param[inout] count Size of bridge_port_list. During out it has Number of bridge ports mapped to bridge
 * @param[out] bridge_port_list List of bridge ports mapped to the bridge
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_map_port_list_get (sai_object_id_t  bridge_id,
                                           uint32_t        *count,
                                           sai_object_id_t *bridge_port_list);

/**
 * @brief Get the Bridge port at the particular index of bridge map
 *
 * @param[in] bridge_id Bridge SAI Object identifier
 * @param[in] index Index at which bridge port needs to be retrieved
 * @param[out] bridge_port_id Bridge port ID retrieved
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_map_get_bridge_port_at_index (sai_object_id_t  bridge_id,
                                                      uint_t           index,
                                                      sai_object_id_t *bridge_port_id);

/**
 * @brief Get the count of bridge ports mapped to a bridge
 *
 * @param[in] bridge_id Bridge SAI Object identifier
 * @param[out] Number of bridge ports mapped to bridge
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_map_get_port_count (sai_object_id_t  bridge_id,
                                            uint_t        *p_out_count);
#ifdef __cplusplus
}
#endif

/**
 * @brief Get the bridge mutex lock to access bridge data structure
 *
 */
void sai_bridge_lock(void);

/**
 * @brief Release the bridge mutex lock
 *
 */
void sai_bridge_unlock(void);

/**
 * @brief Map the bridge port to lag
 *
 * @param[in] lag_id LAG identifier
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_lag_to_bridge_port_map_insert (sai_object_id_t lag,
                                                sai_object_id_t bridge_port_id);

/**
 * @brief Remove bridge port mapping to the lag
 *
 * @param[in] lag_id LAG SAI Object identifier
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_lag_to_bridge_port_map_remove (sai_object_id_t lag_id,
                                                sai_object_id_t bridge_port_id);

/**
 * @brief Get the list of bridge ports mapped to the LAG
 *
 * @param[in] lag_id LAG SAI Object identifier
 * @param[inout] count Size of bridge_port_list.During out has Number of bridge ports mapped to LAG
 * @param[out] bridge_port_list List of bridge ports mapped to the LAG
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_lag_bridge_map_port_list_get (sai_object_id_t  lag_id,
                                               uint_t          *count,
                                               sai_object_id_t *bridge_port_list);

/**
 * @brief Get the count of bridge ports mapped to the LAG
 *
 * @param[in] lag_id LAG SAI Object identifier
 * @param[out] count Number of bridge ports mapped to LAG
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_lag_map_get_bridge_port_count (sai_object_id_t  lag_id,
                                                uint_t          *p_out_count);

/**
 * @brief Map VLAN Member to bridge port
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[in] vlan_member_id VLAN Member SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_to_vlan_member_map_insert (sai_object_id_t bridge_port_id,
                                                        sai_object_id_t vlan_member_id);

/**
 * @brief Remove VLAN Member to bridge port mapping
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[in] vlan_member_id VLAN Member SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_to_vlan_member_map_remove (sai_object_id_t bridge_port_id,
                                                        sai_object_id_t vlan_member_id);

/**
 * @brief Get VLAN Member list mapped to bridge port
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[in] index Index to be retrieved in VLAN member list
 * @param[out] VLAN member object Id obtained
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_get_vlan_member_at_index (sai_object_id_t  bridge_port_id,
                                                       uint_t           index,
                                                      sai_object_id_t  *vlan_member_id);

/**
 * @brief Get Number of VLAN Members mapped to bridge port
 *
 * @param[out] Number of VLAN members mapped to bridge port
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_to_vlan_member_count_get(sai_object_id_t  bridge_port_id,
                                                      uint_t        *p_out_count);

/**
 * @brief Map STP Port to bridge port
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[in] stp_port_id STP Port SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_to_stp_port_map_insert (sai_object_id_t bridge_port_id,
                                                     sai_object_id_t stp_port_id);

/**
 * @brief Remove STP Port to bridge port mapping
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[in] stp_port_id STP Port SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_to_stp_port_map_remove (sai_object_id_t bridge_port_id,
                                                     sai_object_id_t stp_port_id);
/**
 * @brief Get STP Port list mapped to bridge port
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[in] index Index to be retrieved in STP Port list
 * @param[out] STP port id retrieved
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_get_stp_port_at_index (sai_object_id_t  bridge_port_id,
                                                      uint_t           index,
                                                      sai_object_id_t *stp_port_id);
/**
 * @brief Get Number of STP Ports mapped to bridge port
 *
 * @param[out] Number of STP Ports mapped to bridge port
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_to_stp_port_count_get(sai_object_id_t  bridge_port_id,
                                                   uint_t          *p_out_count);

/**
 * @brief Map Tunnel to bridge port
 *
 * @param[in] tunnel_id Tunnel SAI Object identifier
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_tunnel_to_bridge_port_map_insert (sai_object_id_t tunnel_id,
                                                   sai_object_id_t bridge_port_id);

/**
 * @brief Remove Tunnel to bridge port mapping
 *
 * @param[in] tunnel_id Tunnel SAI Object identifier
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_tunnel_to_bridge_port_map_remove (sai_object_id_t tunnel_id,
                                                   sai_object_id_t bridge_port_id);

/**
 * @brief Get Bridge port list mapped to a tunnel
 *
 * @param[in] tunnel_id Tunnel Object identifier
 * @param[inout] count Size of tunnel_list.During out has Number of STP Ports mapped to bridge port
 * @param[out] List of Bridge Port object IDs mapped
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_tunnel_to_bridge_port_list_get (sai_object_id_t    tunnel_id,
                                                 uint_t             *count,
                                                 sai_object_id_t    *bridge_port_list);

/**
 * @brief Get Number of Bridge ports mapped to the tunnel
 *
 * @param[out] Number of Bridge Ports mapped to the tunnel
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_tunnel_to_bridge_port_count_get(sai_object_id_t  tunnel_id,
                                                 uint_t          *p_out_count);

/**
 * @brief Get the bridge port at index in the bridge port list mapped to a
 * tunnel
 * @param[in] tunnel_id Tunnel object identifier
 * @param[in] index Index in the bridge port list from which the bridge port
 *                  would be retrieved.
 * @param[out] bridge_port Bridge port idenetifier.
 * @return SAI_STATUS_SUCCESS if successful otherwise a different error code
 *         is returned.
 * */
sai_status_t sai_tunnel_to_bridge_port_get_at_index(sai_object_id_t tunnel_id,
                                                    uint_t index,
                                                    sai_object_id_t *bridge_port);
/**
 * @brief Update attribute value in bridge cache
 *
 * @param[inout] bridge_info Pointer to bridge info structure
 * @param[in] attr Attribute that needs to be updated
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_update_attr_value_in_cache(dn_sai_bridge_info_t *bridge_info,
                                                   const sai_attribute_t *attr);

/**
 * @brief Get attribute values from bridge cache
 *
 * @param[in] bridge_info Pointer to bridge info structure
 * @param[in] attr_count Number of attributes retrieved
 * @param[inout] attr_list List of attributes retrieved
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_get_attr_value_from_bridge_info (const dn_sai_bridge_info_t *bridge_info,
                                                         uint_t attr_count,
                                                         sai_attribute_t *attr_list);

/**
 * @brief Update attribute value in bridge port cache
 *
 * @param[inout] bridge_port_info Pointer to bridge port info structure
 * @param[in] attr Attribute that needs to be updated
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_update_attr_value_in_cache (dn_sai_bridge_port_info_t *bridge_port_info,
                                                         const sai_attribute_t *attr);

/**
 * @brief Get attribute values from bridge port cache
 *
 * @param[in] bridge_port_info Pointer to bridge port info structure
 * @param[in] attr_count Number of attributes retrieved
 * @param[inout] attr_list List of attributes retrieved
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_get_attr_value_from_bridge_port_info (const dn_sai_bridge_port_info_t
                                                                   *bridge_port_info,
                                                                   uint_t attr_count,
                                                                   sai_attribute_t *attr_list);


/**
 * @brief Get Bridge ID from Bridge port ID
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[out] bridge_id Bridge Identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_get_bridge_id(sai_object_id_t bridge_port_id,
                                         sai_object_id_t *bridge_id);

/**
 * @brief Get attached port ID from Bridge port ID
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[out] sai_port_id SAI Port object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_get_port_id(sai_object_id_t bridge_port_id,
                                         sai_object_id_t *sai_port_id);

/**
 * @brief Get attached vlan ID from Bridge port ID
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[out] vlan_id VLAN identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_get_vlan_id(sai_object_id_t  bridge_port_id,
                                         uint16_t        *vlan_id);

/**
 * @brief Get attached RIF ID from Bridge port ID
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[out] rif_id SAI Router interface identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_get_rif_id(sai_object_id_t  bridge_port_id,
                                        sai_object_id_t *rif_id);

/**
 * @brief Get attached Tunnel ID from Bridge port ID
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[out] tunnel_id SAI Tunnel object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_get_tunnel_id(sai_object_id_t  bridge_port_id,
                                           sai_object_id_t *tunnel_id);

/**
 * @brief Get the bridge hadware info from bridge port info
 *
 * @param[inout] bridge_port_info Pointer to bridge port info
 * @return A pointer to bridge hardware info
 */
void *sai_bridge_port_info_get_bridge_hw_info(dn_sai_bridge_port_info_t  *bridge_port_info);


/**
 * @brief Initialize the bridge info with defaults
 *
 * @param[inout] bridge_info Pointer to bridge info
 */
void sai_bridge_init_default_bridge_info(dn_sai_bridge_info_t *bridge_info);

/**
 * @brief Initialize the bridge port info with defaults
 *
 * @param[inout] bridge_port_info Pointer to bridge port info
 */
void sai_bridge_init_default_bridge_port_info(dn_sai_bridge_port_info_t *bridge_port_info);

/**
 * @brief Increment bridge reference count
 *
 * @param[in] bridge_id Bridge SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_increment_ref_count(sai_object_id_t bridge_id);

/**
 * @brief Decrement bridge reference count
 *
 * @param[in] bridge_id Bridge SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_decrement_ref_count(sai_object_id_t bridge_id);

/**
 * @brief Increment bridge port reference count
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_increment_ref_count(sai_object_id_t bridge_port_id);

/**
 * @brief Decrement bridge port reference count
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_decrement_ref_count(sai_object_id_t bridge_port_id);

/**
 * @brief Increment bridge port fdb learnt count
 *
 * @param[in] bridge_port_id bridge port SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_increment_fdb_count(sai_object_id_t bridge_port_id);

/**
 * @brief Decrement bridge port fdb learnt count
 *
 * @param[in] bridge_port_id bridge port SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_decrement_fdb_count(sai_object_id_t bridge_port_id);

/**
 * @brief Check if attachment is of type port
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @return true if attachment is of type port, false otherwise
 */
bool sai_is_bridge_port_type_port(sai_object_id_t bridge_port_id);

/**
 * @brief Check if attachment is of type port
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @return true if attachment is of type sub port, false otherwise
 */
bool sai_is_bridge_port_type_sub_port(sai_object_id_t bridge_port_id);

/**
 * @brief Check if attachment is of type tunnel
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @return true if attachment is of type tunnel, false otherwise
 */
bool sai_is_bridge_port_type_tunnel(sai_object_id_t bridge_port_id);

/**
 * @brief Create a mapping from port,vlan to bridge port id
 *
 * @param[in] port_id SAI Port Object identifier
 * @param[in] vlan_id VLAN identifier
 * @param[in] bridge_port_id SAI bridge Port Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_vlan_to_bridge_port_map_insert (sai_object_id_t port_id,
                                                             sai_vlan_id_t vlan_id,
                                                             sai_object_id_t bridge_port_id);

/**
 * @brief Remove mapping from port,vlan to bridge port id
 *
 * @param[in] port_id SAI Port Object identifier
 * @param[in] vlan_id VLAN identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_vlan_to_bridge_port_map_remove (sai_object_id_t port_id,
                                                             sai_vlan_id_t vlan_id);

/**
 * @brief Check if already a bridge port exists for port vlan combination
 *
 * @param[in] port_id SAI Port Object identifier
 * @param[in] vlan_id VLAN identifier
 * @return true if already a bridge port exists, false otherwise
 */
bool sai_bridge_is_sub_port_created(sai_object_id_t port_id, sai_vlan_id_t vlan_id);

/*
 * @brief Check if the bridge and tunnel are connected via a
 * tunnel bridge port.
 *
 * @param[in] bridge_id SAI Bridge Object Identifier
 * @param[in] tunnel_id SAI Tunnel Object Identifier
 * @return true if they are connected, false otherwise
 */
bool sai_bridge_is_bridge_connected_to_tunnel(sai_object_id_t bridge_id,
                                              sai_object_id_t tunnel_id);

/**
 * @brief Check if bridge port attached to a loag
 *
 * @param[in] bridge_port_id SAI Bridge Port Object identifier
 * @return true if bridge port is attached to lag, false otherwise
 */
bool sai_is_bridge_port_obj_lag(sai_object_id_t bridge_port_id);

/**
 * @brief Get bridge port type
 *
 * @param[in] bridge_port_id SAI bridge Port Object identifier
 * @param[out] bridge_port_type Bridge port type
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_get_type(sai_object_id_t bridge_port_id,
                                      sai_bridge_port_type_t *bridge_port_type);

/**
 * @brief Get bridge port admin state
 *
 * @param[in] bridge_port_id SAI bridge Port Object identifier
 * @param[out] admin state Bridge port admin status
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_get_admin_state(sai_object_id_t bridge_port_id,
                                             bool *admin_state);

/**
 * @brief Map L2MC Member to bridge port
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[in] l2mc_member_id L2MC Member SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_to_l2mc_member_map_insert (sai_object_id_t bridge_port_id,
                                                        sai_object_id_t l2mc_member_id);

/**
 * @brief Remove L2MC Member to bridge port mapping
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[in] l2mc_member_id L2MC Member SAI Object identifier
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_to_l2mc_member_map_remove (sai_object_id_t bridge_port_id,
                                                        sai_object_id_t l2mc_member_id);

/**
 * @brief Get L2MC Member at Index from list of members mapped to bridge port
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[in] Index to be retrieved
 * @param[out] List of L2MC member object ID retrieved
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_get_l2mc_member_at_index (sai_object_id_t  bridge_port_id,
                                                       uint_t           index,
                                                       sai_object_id_t *l2mc_member_id);


/**
 * @brief Get Number of L2MC Members mapped to bridge port
 *
 * @param[out] Number of L2MC members mapped to bridge port
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_port_to_l2mc_member_count_get(sai_object_id_t  bridge_port_id,
                                                      uint_t        *p_out_count);

/**
 * @brief Clean up any L2MC reference object in bridge
 *
 * @param[in] bridge_info Pointer to bridge info
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_bridge_cleanup_l2mc_group_references(const dn_sai_bridge_info_t *bridge_info);

/**
 * @brief Check if duplicate attribute value set on bridge
 *
 * @param[in] bridge_info Pointer to bridge info
 * @param[in] attr Attribute to be applied
 * @return true if attribute value is duplicate, false otherwise
 */
bool sai_bridge_is_duplicate_attr_val(const dn_sai_bridge_info_t *bridge_info,
                                      const sai_attribute_t *attr);

/**
 * @brief Check if duplicate attribute value set on bridge port
 *
 * @param[in] bridge_info Pointer to bridge port info
 * @param[in] attr Attribute to be applied
 * @return true if attribute value is duplicate, false otherwise
 */
bool sai_bridge_port_is_duplicate_attr_val(const dn_sai_bridge_port_info_t *bridge_port_info,
                                           const sai_attribute_t *attr);
/**
 * @brief Get attached port info from bridge port info
 *
 * @param[inout] bridge_port_info Pointer to bridge port info
 * @return Pointer to attached port info structure
 */
static inline dn_sai_bridge_port_t* sai_bridge_port_get_port_info(dn_sai_bridge_port_info_t
                                                                         *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        return NULL;
    }
    return &bridge_port_info->attachment.port;
}

/**
 * @brief Get attached sub port info from bridge port info
 *
 * @param[inout] bridge_port_info Pointer to bridge port info
 * @return Pointer to attached sub port info structure
 */
static inline dn_sai_bridge_sub_port_t* sai_bridge_port_get_sub_port_info(dn_sai_bridge_port_info_t
                                                                                  *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        return NULL;
    }
    return &bridge_port_info->attachment.sub_port;
}

/**
 * @brief Get attached router port info from bridge port info
 *
 * @param[inout] bridge_port_info Pointer to bridge port info
 * @return Pointer to attached router port info structure
 */
static inline dn_sai_bridge_router_port_t* sai_bridge_port_get_router_port_info(
                                                     dn_sai_bridge_port_info_t *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        return NULL;
    }
    return &bridge_port_info->attachment.router_port;
}

/**
 * @brief Get attached tunnel port info from bridge port info
 *
 * @param[inout] bridge_port_info Pointer to bridge port info
 * @return Pointer to attached tunnel port info structure
 */
static inline dn_sai_bridge_tunnel_port_t* sai_bridge_port_get_tunnel_port_info(
                                                     dn_sai_bridge_port_info_t *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        return NULL;
    }
    return &bridge_port_info->attachment.tunnel_port;
}

/**
 * @brief Get port id from bridge port info
 *
 * @param[in] bridge_port_info Pointer to bridge port info
 * @return Port object id that is associated with the bridge port info
 */
static inline sai_object_id_t sai_bridge_port_info_get_port_id(const dn_sai_bridge_port_info_t
                                                               *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        return SAI_NULL_OBJECT_ID;
    }

    if(bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_PORT) {
        return bridge_port_info->attachment.port.port_id;
    } else if (bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_SUB_PORT) {
        return bridge_port_info->attachment.sub_port.port_id;
    } else {
        return SAI_NULL_OBJECT_ID;
    }
}

/**
 * @brief Get VLAN id from bridge port info
 *
 * @param[in] bridge_port_info Pointer to bridge port info
 * @return VLAN id that is associated with the bridge port info
 */
static inline sai_vlan_id_t sai_bridge_port_info_get_vlan_id(const dn_sai_bridge_port_info_t
                                                             *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        return VLAN_UNDEF;
    }
    if (bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_SUB_PORT) {
        return bridge_port_info->attachment.sub_port.vlan_id;
    }
    return VLAN_UNDEF;

}

/**
 * @brief Get RIF id from bridge port info
 *
 * @param[in] bridge_port_info Pointer to bridge port info
 * @return RIF id that is associated with the bridge port info
 */
static inline sai_object_id_t sai_bridge_port_info_get_rif_id(const dn_sai_bridge_port_info_t
                                                              *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        return SAI_NULL_OBJECT_ID;
    }
    if ((bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_1D_ROUTER) ||
            (bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_1Q_ROUTER)) {
        return bridge_port_info->attachment.router_port.rif_id;
    }
    return SAI_NULL_OBJECT_ID;
}

/**
 * @brief Get Tunnel id from bridge port info
 *
 * @param[in] bridge_port_info Pointer to bridge port info
 * @return Tunnel id that is associated with the bridge port info
 */
static inline sai_object_id_t sai_bridge_port_info_get_tunnel_id(const dn_sai_bridge_port_info_t
                                                                 *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        return SAI_NULL_OBJECT_ID;
    }

    if (bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_TUNNEL) {
        return bridge_port_info->attachment.tunnel_port.tunnel_oid;
    }
    return SAI_NULL_OBJECT_ID;
}

/**
 * @brief Check if bridge port info is of type port
 *
 * @param[in] bridge_port_info Pointer to bridge port info
 * @return true if bridge_port is of type port, false otherwise
 */
static inline bool sai_bridge_port_info_is_type_port(const dn_sai_bridge_port_info_t
                                                     *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        return false;
    }
    return (bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_PORT);
}

/**
 * @brief Check if bridge port info is of type sub port
 *
 * @param[in] bridge_port_info Pointer to bridge port info
 * @return true if bridge_port is of type sub port, false otherwise
 */
static inline bool sai_bridge_port_info_is_type_sub_port(const dn_sai_bridge_port_info_t
                                                         *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        return false;
    }
    return (bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_SUB_PORT);
}

/**
 * @brief Check if bridge port info is of type RIF
 *
 * @param[in] bridge_port_info Pointer to bridge port info
 * @return true if bridge_port is of type RIF, false otherwise
 */
static inline bool sai_bridge_port_info_is_type_rif(const dn_sai_bridge_port_info_t
                                                    *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        return false;
    }
    return ((bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_1D_ROUTER) ||
            (bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_1D_ROUTER));
}

/**
 * @brief Check if bridge port info is of type tunnel
 *
 * @param[in] bridge_port_info Pointer to bridge port info
 * @return true if bridge_port is of type tunnel, false otherwise
 */
static inline bool sai_bridge_port_info_is_type_tunnel(const dn_sai_bridge_port_info_t
                                                       *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        return false;
    }
    return (bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_TUNNEL);
}

/**
 * @brief Return hardware info from bridge port info
 *
 * @param[inout] bridge_port_info Pointer to bridge port info
 * @return A pointer to hardware info
 */
static inline void *sai_bridge_port_get_sub_port_hw_info(dn_sai_bridge_port_info_t
                                                         *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        return NULL;
    }
    return (bridge_port_info->attachment.sub_port.hw_info);
}

/**
 * @brief Set hardware info in bridge port info
 *
 * @param[inout] bridge_port_info Pointer to bridge port info
 * @param[inout] ptr Pointer which needs to be saved in hardware info
 * @return A pointer to hardware info
 */
static inline sai_status_t sai_bridge_port_set_sub_port_hw_info(dn_sai_bridge_port_info_t
                                                                 *bridge_port_info, void *ptr)
{
    if(bridge_port_info == NULL) {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    bridge_port_info->attachment.sub_port.hw_info = ptr;
    return SAI_STATUS_SUCCESS;
}

/**
 * @brief Get flood type from bridge attribute
 *
 * @param[in] bridge_attr Bridge attribute
 * @param[out] flood_type Flood type that maps to the attribute
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
static inline sai_status_t sai_bridge_get_flood_type_from_attr(sai_bridge_attr_t bridge_attr,
                                                               dn_sai_bridge_flood_type_t *flood_type)
{
    if(flood_type == NULL) {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    switch (bridge_attr) {
        case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
        case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
            *flood_type = SAI_BRIDGE_FLOOD_TYPE_UNKNOWN_UNICAST;
            break;;

        case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
        case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
            *flood_type = SAI_BRIDGE_FLOOD_TYPE_UNKNOWN_MULTICAST;
            break;;

        case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
        case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP:
            *flood_type = SAI_BRIDGE_FLOOD_TYPE_BROADCAST;
            break;;

        default:
            return SAI_STATUS_INVALID_PARAMETER;
    }
    return SAI_STATUS_SUCCESS;
}

/**
 * \}
 */
#endif
