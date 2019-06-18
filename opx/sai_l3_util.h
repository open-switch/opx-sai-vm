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
* @file sai_l3_util.h
*
* @brief This file contains the utility functions for SAI L3 component.
*
*************************************************************************/
#ifndef __SAI_L3_UTIL_H__
#define __SAI_L3_UTIL_H__

#include "std_type_defs.h"
#include "std_llist.h"
#include "std_struct_utils.h"
#include "std_bit_masks.h"
#include "sai_l3_common.h"
#include "sai_event_log.h"
#include "sai_oid_utils.h"
#include "saitypes.h"
#include <string.h>

/** \defgroup SAIL3UTILS SAI - L3 Utility functions
 *  Util functions in the SAI L3 component
 *
 *  \{
 */

/** Offset of the DLL glue field in Router Interface datastructure */
#define SAI_FIB_RIF_DLL_GLUE_OFFSET \
         STD_STR_OFFSET_OF (sai_fib_router_interface_t, dll_glue)

/** Offset of the DLL glue field in DLL link node datastructure */
#define SAI_FIB_LINK_NODE_DLL_GLUE_OFFSET \
         STD_STR_OFFSET_OF (sai_fib_link_node_t, dll_glue)

/** Offset of the link node field in DLL Weighted link node datastructure */
#define SAI_FIB_WT_LINK_NODE_DLL_GLUE_OFFSET \
         STD_STR_OFFSET_OF (sai_fib_wt_link_node_t, link_node)

/** Offset of the MAC entry list pointer field in Neighbor datastructure */
#define SAI_FIB_NEIGHBOR_MAC_ENTRY_DLL_GLUE_OFFSET \
         STD_STR_OFFSET_OF (sai_fib_nh_t, mac_entry_link)

/** Offset of the NH Dependent route list pointer field in route node */
#define SAI_FIB_ROUTE_NH_DEP_DLL_GLUE_OFFSET \
         STD_STR_OFFSET_OF (sai_fib_route_t, nh_dep_route_link)

/** Offset of the underlay route list pointer field in the encap next hop */
#define SAI_FIB_ENCAP_NH_UNDERLAY_ROUTE_DLL_GLUE_OFFSET \
         STD_STR_OFFSET_OF (sai_fib_nh_t, underlay_route_link)

/** Offset of the underlay nhg list pointer field in the encap next hop */
#define SAI_FIB_ENCAP_NH_UNDERLAY_NHG_DLL_GLUE_OFFSET \
         STD_STR_OFFSET_OF (sai_fib_nh_t, underlay_nhg_link)

/** Key length for Next Hop IP address radix tree */
#define SAI_FIB_NH_IP_ADDR_TREE_KEY_LEN  \
         (sizeof (sai_fib_nh_key_t) * BITS_PER_BYTE)

/** Key length for Neighbor MAC entry radix tree */
#define SAI_FIB_NEIGHBOR_MAC_ENTRY_TREE_KEY_LEN  \
         (sizeof (sai_fib_neighbor_mac_entry_key_t) * BITS_PER_BYTE)

/** Constant for IPv6 Address length in bytes */
#define SAI_IPV6_ADDR_NUM_BYTES               (16)

/** Constant for IPv4 Address length in bytes */
#define SAI_IPV4_ADDR_NUM_BYTES               (4)

/** Constant for util functions */
#define SAI_FIB_MAX_BUFSZ                     (256)

/** Flag to indicate Next hop Group type attribute is set in attr mask */
#define SAI_FIB_NH_GROUP_TYPE_ATTR_FLAG       (0x1)

/** Flag to indicate Next hop Group NH list attribute is set in attr mask */
#define SAI_FIB_NH_GROUP_NH_LIST_ATTR_FLAG    (0x2)

/** Flag to indicate Next hop Group NH count attribute is set in attr mask */
#define SAI_FIB_NH_GROUP_NH_COUNT_ATTR_FLAG   (0x4)

/** Flag to indicate Neighbor dest mac attribute is set in attr mask */
#define SAI_FIB_NEIGHBOR_DEST_MAC_ATTR_FLAG   (0x2)

/** Flag to indicate Neighbor packet action attribute is set attr mask */
#define SAI_FIB_NEIGHBOR_PKT_ACTION_ATTR_FLAG (0x4)

/** Flag to indicate Neighbor no_host_route attribute is set attr mask */
#define SAI_FIB_NEIGHBOR_NO_HOST_ROUTE_ATTR_FLAG (0x8)

/** Flag to indicate Neighbor Meta Data attribute is set attr mask */
#define SAI_FIB_NEIGHBOR_META_DATA_ATTR_FLAG (0x10)

/** Flag to indicate Neighbor Egress Port attribute is set attr mask */
#define SAI_FIB_NEIGHBOR_PORT_ID_ATTR_FLAG (0x20)

/** Constant for default value of the switch attr */
#define SAI_FIB_DEFAULT_MAX_ECMP_PATHS        (64)

/** Default V4 Admin State for VRF and Router interface objects. */
#define SAI_FIB_V4_ADMIN_STATE_DFLT                 (true)

/** Default V6 Admin State for VRF and Router interface objects. */
#define SAI_FIB_V6_ADMIN_STATE_DFLT                 (true)

/** Default packet action for packets with IP Options. */
#define SAI_FIB_IP_OPT_PKT_ACTION_DFLT              (SAI_PACKET_ACTION_TRAP)

/** Default packet action for packets with TTL violation. */
#define SAI_FIB_TTL_VIOLATION_PKT_ACTION_DFLT       (SAI_PACKET_ACTION_TRAP)

/** Default MTU value for Router interface objects. */
#define SAI_FIB_MTU_DFLT                            (1514)

/** Flag to indicate VRF attribute is set in attribute mask */
#define SAI_FIB_VRF_ATTR_FLAG                       (0x1)

/** Flag to indicate MAC attribute is set in attribute mask */
#define SAI_FIB_SRC_MAC_ATTR_FLAG                   (0x2)

/** Flag to indicate V4 admin state attribute is set in attribute mask */
#define SAI_FIB_V4_ADMIN_STATE_ATTR_FLAG            (0x4)

/** Flag to indicate V6 admin state attribute is set in attribute mask */
#define SAI_FIB_V6_ADMIN_STATE_ATTR_FLAG            (0x8)

/** Flag to indicate MTU attribute is set in attribute mask */
#define SAI_FIB_MTU_ATTR_FLAG                       (0x10)

/** Flag to indicate IP Options attribute is set in attribute mask */
#define SAI_FIB_IP_OPTIONS_ATTR_FLAG                (0x20)

/** Flag to indicate TTL violations attribute is set in attribute mask */
#define SAI_FIB_TTL_VIOLATION_ATTR_FLAG             (0x40)

/** Maximum number of Virtual Routers */
#define SAI_FIB_MAX_VIRTUAL_ROUTERS                 (512)

/** Maximum number of packet actions supported in SAI - sai_packet_action_t */
#define SAI_MAX_PACKET_ACTIONS                      (8)

/** Next-hop Type is not initialized */
#define SAI_FIB_ROUTE_NH_TYPE_NONE           ((uint_t) -1)

/** Tunnel type is not valid */
#define SAI_FIB_TUNNEL_TYPE_NONE                    (0)

/** Constant for IPv6 Address Family prefix bit length */
#define SAI_IPV6_ADDR_PREFIX_LEN  (SAI_IPV6_ADDR_NUM_BYTES * BITS_PER_BYTE)

/** Constant for IPv4 Address Family prefix bit length */
#define SAI_IPV4_ADDR_PREFIX_LEN  (SAI_IPV4_ADDR_NUM_BYTES * BITS_PER_BYTE)

/** Logging utility for SAI Virtual Router API */
#define SAI_ROUTER_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_VIRTUAL_ROUTER, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_ROUTER, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Logging utility for SAI Router Interface API */
#define SAI_RIF_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_ROUTER_INTERFACE, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_ROUTER_INTF, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Logging utility for SAI Next Hop API */
#define SAI_NEXTHOP_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_NEXT_HOP, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_NEXT_HOP, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Logging utility for SAI Neighbor API */
#define SAI_NEIGHBOR_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_NEIGHBOR, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_NEIGHBOR, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Logging utility for SAI Next Hop Group API */
#define SAI_NH_GROUP_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_NEXT_HOP_GROUP, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_NEXT_HOP_GROUP, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Logging utility for SAI Route API */
#define SAI_ROUTE_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (ev_log_t_SAI_ROUTE, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_ROUTER, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI Virtual Router API */
#define SAI_ROUTER_LOG_TRACE(msg, ...) \
        SAI_ROUTER_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_ROUTER_LOG_CRIT(msg, ...) \
        SAI_ROUTER_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_ROUTER_LOG_ERR(msg, ...) \
        SAI_ROUTER_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_ROUTER_LOG_INFO(msg, ...) \
        SAI_ROUTER_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_ROUTER_LOG_WARN(msg, ...) \
        SAI_ROUTER_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_ROUTER_LOG_NTC(msg, ...) \
        SAI_ROUTER_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/** Per log level based macros for SAI Router Interface API */
#define SAI_RIF_LOG_TRACE(msg, ...) \
        SAI_RIF_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_RIF_LOG_CRIT(msg, ...) \
        SAI_RIF_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_RIF_LOG_ERR(msg, ...) \
        SAI_RIF_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_RIF_LOG_INFO(msg, ...) \
        SAI_RIF_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_RIF_LOG_WARN(msg, ...) \
        SAI_RIF_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_RIF_LOG_NTC(msg, ...) \
        SAI_RIF_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/** Per log level based macros for SAI Neighbor API */
#define SAI_NEIGHBOR_LOG_TRACE(msg, ...) \
        SAI_NEIGHBOR_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_NEIGHBOR_LOG_CRIT(msg, ...) \
        SAI_NEIGHBOR_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_NEIGHBOR_LOG_ERR(msg, ...) \
        SAI_NEIGHBOR_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_NEIGHBOR_LOG_INFO(msg, ...) \
        SAI_NEIGHBOR_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_NEIGHBOR_LOG_WARN(msg, ...) \
        SAI_NEIGHBOR_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_NEIGHBOR_LOG_NTC(msg, ...) \
        SAI_NEIGHBOR_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/** Per log level based macros for SAI NextHop API */
#define SAI_NEXTHOP_LOG_TRACE(msg, ...) \
        SAI_NEXTHOP_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_NEXTHOP_LOG_CRIT(msg, ...) \
        SAI_NEXTHOP_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_NEXTHOP_LOG_ERR(msg, ...) \
        SAI_NEXTHOP_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_NEXTHOP_LOG_INFO(msg, ...) \
        SAI_NEXTHOP_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_NEXTHOP_LOG_WARN(msg, ...) \
        SAI_NEXTHOP_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_NEXTHOP_LOG_NTC(msg, ...) \
        SAI_NEXTHOP_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/** Per log level based macros for SAI NextHopGroup API */
#define SAI_NH_GROUP_LOG_TRACE(msg, ...) \
        SAI_NH_GROUP_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_NH_GROUP_LOG_CRIT(msg, ...) \
        SAI_NH_GROUP_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_NH_GROUP_LOG_ERR(msg, ...) \
        SAI_NH_GROUP_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_NH_GROUP_LOG_INFO(msg, ...) \
        SAI_NH_GROUP_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_NH_GROUP_LOG_WARN(msg, ...) \
        SAI_NH_GROUP_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_NH_GROUP_LOG_NTC(msg, ...) \
        SAI_NH_GROUP_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/** Per log level based macros for SAI Route API */
#define SAI_ROUTE_LOG_TRACE(msg, ...) \
        SAI_ROUTE_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_ROUTE_LOG_CRIT(msg, ...) \
        SAI_ROUTE_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_ROUTE_LOG_ERR(msg, ...) \
        SAI_ROUTE_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_ROUTE_LOG_INFO(msg, ...) \
        SAI_ROUTE_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_ROUTE_LOG_WARN(msg, ...) \
        SAI_ROUTE_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_ROUTE_LOG_NTC(msg, ...) \
        SAI_ROUTE_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/**
 * @brief Accessor function for SAI L3 global config structure.
 *
 * @return Pointer to the global config structure instance.
 */
sai_fib_global_t *sai_fib_access_global_config (void);

/**
 * @brief Initialization function for SAI L3 global params.
 *
 * @warning This function is not thread safe. If two threads ever call this
 * function then this function could be invoked twice,
 * if one thread is preempted in the middle of execution of this function.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_fib_global_init (void);

/**
 * @brief Cleanup function for SAI L3 global params.
 *
 */
void sai_fib_global_cleanup (void);

/**
 * @brief Accessor function for ECMP max-paths global parameter.
 *
 * @return Value of the max ecmp paths attribute.
 */
uint_t sai_fib_max_ecmp_paths_get (void);

/**
 * @brief Accessor function for max virtual routers global parameter.
 *
 * @return Value of the max virtual routers attribute.
 */
uint_t sai_fib_max_virtual_routers_get (void);

/**
 * @brief Get SAI FIB VRF node.
 *
 * @param[in] vrf_id    Virtual router id
 * @return Pointer to the VRF node if vrf id is found otherwise NULL
 */
sai_fib_vrf_t  *sai_fib_vrf_node_get (sai_object_id_t vrf_id);

/**
 * @brief Get SAI FIB Router Interface node.
 *
 * @param[in] rif_id    Router Interface id
 * @return Pointer to the router interface node if rif id is found
 * otherwise NULL
 */
sai_fib_router_interface_t *sai_fib_router_interface_node_get (
                                 sai_object_id_t rif_id);

/**
 * @brief Check if router interface is create
 *
 * @param[in] rif_id    Router Interface id
 * @return true if rif_id is created
 * otherwise false
 */
bool sai_fib_is_rif_created (sai_object_id_t rif_id);

sai_status_t sai_rif_increment_ref_count (sai_object_id_t rif_id);

sai_status_t sai_rif_decrement_ref_count (sai_object_id_t rif_id);
/**
 * @brief Get SAI FIB Next Hop node using next_hop_id.
 *
 * @param[in] nh_id    Next Hop id
 * @return Pointer to the next hop node if next hop id is found
 * otherwise NULL
 */
sai_fib_nh_t *sai_fib_next_hop_node_get_from_id (sai_object_id_t nh_id);

/**
 * @brief Get SAI FIB Next Hop Group node.
 *
 * @param[in] id  Next Hop Group id
 * @return Pointer to the next hop group node if next hop group is found
 * otherwise NULL
 */
sai_fib_nh_group_t *sai_fib_next_hop_group_get (sai_object_id_t id);

/**
 * @brief Get SAI FIB VRF node for the RIF Id.
 *
 * @param[in] rif_id   Router Interface Id
 * @return Pointer to the VRF node if vrf id is found otherwise NULL
 */
sai_fib_vrf_t* sai_fib_get_vrf_node_for_rif (sai_object_id_t rif_id);

/**
 * @brief Get SAI FIB IP next hop node for the Next hop IP address.
 *
 * @param[in] nh_type   Next Hop type
 * @param[in] rif_id    Router interface id
 * @param[in] p_ip_addr   Pointer to the Next Hop IP address
 * @param[in] tunnel_type Tunnel type when next hop is of type
 *                        SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP
 * @return Pointer to the next hop node if found otherwise NULL
 */
sai_fib_nh_t* sai_fib_ip_next_hop_node_get (sai_next_hop_type_t nh_type,
                                            sai_object_id_t rif_id,
                                            sai_ip_address_t *p_ip_addr,
                                            sai_tunnel_type_t tunnel_type);

/**
 * @brief Find if the next hop group node is present in next hop's group list.
 *
 * @param[in] p_nh_node   Pointer to the Next Hop node.
 * @param[in] p_nh_group   Pointer to the Next Hop group node.
 * @return Pointer to the link node in next hop group list if found otherwise NULL
 */
sai_fib_wt_link_node_t *sai_fib_nh_find_group_link_node (
                                                sai_fib_nh_t *p_nh_node,
                                                sai_fib_nh_group_t *p_nh_group);

/**
 * @brief Find if the next hop node is present in next hop group's nh list.
 *
 * @param[in] p_nh_group   Pointer to the Next Hop Group node.
 * @param[in] p_nh_node   Pointer to the Next Hop node.
 * @return Pointer to the link node in next hop list if found otherwise NULL
 */
sai_fib_wt_link_node_t *sai_fib_nh_group_find_nh_link_node (
                                                sai_fib_nh_group_t *p_nh_group,
                                                sai_fib_nh_t *p_nh_node);
/**
 * @brief Utility to convert SAI IP address structure to string.
 * Wrapper to the STD IP Utility function.
 *
 * @param[in] p_ip_addr  Pointer to the SAI IP address structure
 * @param[in] p_buf   Pointer to the input buffer to fill the string
 * @param[in] len   Length of the buffer
 * @return Pointer to buffer filled with the IP address string.
 */
const char *sai_ip_addr_to_str (const sai_ip_address_t *p_ip_addr,
                                char *p_buf, size_t len);

/**
 * @brief Utility to convert IPv4 address bytes to string format.
 *
 * @param[in] p_addr_bytes  Pointer to the IP address bytes
 * @param[in] p_buf   Pointer to the input buffer to fill the string
 * @param[in] len   Length of the buffer
 * @return Pointer to buffer filled with the IP address string.
 */
const char *sai_ipv4_addr_to_str (const uint8_t *p_addr_bytes,
                                  char *p_buf, size_t len);

/**
 * @brief Utility to convert IPv6 address bytes to string format.
 *
 * @param[in] p_addr_bytes  Pointer to the IP address bytes
 * @param[in] p_buf   Pointer to the input buffer to fill the string
 * @param[in] len   Length of the buffer
 * @return Pointer to buffer filled with the IP address string.
 */
const char *sai_ipv6_addr_to_str (const uint8_t *p_addr_bytes,
                                  char *p_buf, size_t len);

/**
 * @brief Utility to check if the SAI IP address is zero ip address.
 * Wrapper to the STD IP Utility function.
 *
 * @param[in] p_ip_addr   Pointer to the SAI IP address structure
 * @return true if IP address is zero else false.
 */
bool sai_fib_is_ip_addr_zero (const sai_ip_address_t *p_ip_addr);

/**
 * @brief Utility to check if the SAI IP address is loopback ip address.
 * Wrapper to the STD IP Utility function.
 *
 * @param[in] p_ip_addr   Pointer to the SAI IP address structure
 * @return true if IP address is loopback address else false.
 */
bool sai_fib_is_ip_addr_loopback (const sai_ip_address_t *p_ip_addr);

/**
 * @brief Utility to copy SAI IP address structure fields.
 *
 * @param[in] p_dest_ip_addr  Pointer to the destination SAI IP addr structure
 * @param[in] p_src_ip_addr   Const Pointer to the source SAI IP addr structure
 * @return SAI_STATUS_SUCCESS if copied successfully else SAI_STATUS_FAILURE.
 */
sai_status_t sai_fib_ip_addr_copy (sai_ip_address_t *p_dest_ip_addr,
                                   const sai_ip_address_t *p_src_ip_addr);

/**
 * @brief Utility to validate packet action value.
 *
 * @param[in] pkt_action   Input SAI packet action value
 * @return true if the input is a valid packet action value else false.
 */
bool sai_packet_action_validate (sai_packet_action_t pkt_action);

/**
 * @brief Utility to convert packet action value to corresponding name string
 *
 * @param[in] pkt_action   Input SAI packet action value
 * @return Packet action name string.
 */
const char *sai_packet_action_str (sai_packet_action_t pkt_action);

/**
 * @brief Utility to return string corresponding to Next Hop Group type.
 *
 * @param[in] type Next Hop Group type enumeration.
 * @return String corresponding to the Next Hop Group type.
 */
const char *sai_fib_nh_group_type_str (sai_next_hop_group_type_t type);

/**
 * @brief Utility to return string corresponding to Next Hop type.
 *
 * @param[in] type Next Hop type enumeration.
 * @return String corresponding to the Next Hop type.
 */
const char *sai_fib_next_hop_type_str (sai_next_hop_type_t type);

/**
 * @brief Utility to return string corresponding to router interface type.
 *
 * @param[in] type Router interface type which can be port or vlan.
 * @return String corresponding to the router interface type.
 */
const char *sai_fib_rif_type_to_str (uint_t type);

/**
 * @brief Utility to return port/VLAN/LAG ID for the router interface node.
 *
 * @param[in] p_rif_node Pointer to the router interface node.
 * @return port/VLAN/LAG ID for the router interface node.
 */
uint_t sai_fib_rif_attachment_id_get (sai_fib_router_interface_t *p_rif_node);

/**
 * @brief Utility to return string corresponding to Next-hop type.
 *
 * @param[in] nh_type Next-hop Type.
 * @return String corresponding to the Next-hop type.
 */
const char *sai_fib_route_nh_type_to_str (uint_t nh_type);

/**
 * @brief Utility to get the Next-hop object ID for the Route node.
 *
 *  @param[in] p_route Pointer to route node.
 *  @return Next-hop object ID for the Route node.
 */
sai_object_id_t sai_fib_route_node_nh_id_get (sai_fib_route_t *p_route);

/**
 * @brief Utility to check if MAC address is zero.
 *
 * @param[in] p_mac  MAC address.
 * @return true if MAC address is zero else false.
 */
bool sai_fib_is_mac_address_zero (const sai_mac_t *p_mac);

/**
 * @brief Utility to return the SAI status code based on error code and
 * the attribute's index on the attribute list.
 *
 * @param[in] status  Attribute's error return code.
 * @param[in] attr_index  Attribute's index on the attribute list.
 * @return Attribute index based SAI status code.
 */
sai_status_t sai_fib_attr_status_code_get (sai_status_t status, uint_t attr_index);

/**
 * @brief Utility to find a MAC entry node in Neighbor MAC entry tree.
 *
 * @param[in] p_key  Key information to lookup MAC entry node.
 * @return Pointer to the node if found otherwise NULL.
 */
sai_fib_neighbor_mac_entry_t *sai_fib_neighbor_mac_entry_find (
                                const sai_fib_neighbor_mac_entry_key_t *p_key);

/**
 * @brief Get first Neighbor node from MAC entry's Neighbor list.
 * Helper to scan MAC entry Neighbor list.
 *
 * @param[in] p_mac_entry MAC entry node pointer.
 * @return Pointer to the first node if found otherwise NULL.
 */
sai_fib_nh_t* sai_fib_get_first_neighbor_from_mac_entry (
                                sai_fib_neighbor_mac_entry_t *p_mac_entry);

/**
 * @brief Get next Neighbor node from MAC entry's Neighbor list.
 * Helper to scan MAC entry Neighbor list.
 *
 * @param[in] p_mac_entry MAC entry node pointer.
 * @param[in] p_neighbor Pointer to Neighbor node to which next node is got.
 * @return Pointer to the node if found otherwise NULL.
 */
sai_fib_nh_t* sai_fib_get_next_neighbor_from_mac_entry (
                                sai_fib_neighbor_mac_entry_t *p_mac_entry,
                                sai_fib_nh_t *p_neighbor);

/**
 * @brief Get IP address mask for a prefix length.
 *
 * @param[out] ip_mask IP address mask to be filled.
 * @param[in] prefix_len IP Prefix length for the IP address family.
 * @return Pointer to the node if found otherwise NULL.
 */
void sai_fib_ip_prefix_mask_get (sai_ip_address_t *ip_mask,
                                 uint_t prefix_len);

/**
 * @brief Get first route node from Next Hop's dependent route list.
 *
 * @param[in] p_next_hop Next Hop node pointer.
 * @return Pointer to the node if found otherwise NULL.
 */
sai_fib_route_t* sai_fib_get_first_dep_route_from_nh (sai_fib_nh_t *p_next_hop);

/**
 * @brief Get next route node from Next Hop's dependent route list.
 *
 * @param[in] p_next_hop Next Hop node pointer.
 * @param[in] p_route Pointer to route node to which next node is got.
 * @return Pointer to the node if found otherwise NULL.
 */
sai_fib_route_t* sai_fib_get_next_dep_route_from_nh (sai_fib_nh_t *p_next_hop,
                                                     sai_fib_route_t *p_route);

/**
 * @brief Get first next hop node from route's dependent encap next hop list.
 *
 * @param[in] p_route Route node pointer.
 * @return Pointer to the node if found otherwise NULL.
 */
sai_fib_nh_t* sai_fib_route_get_first_dep_encap_nh (sai_fib_route_t *p_route);

/**
 * @brief Get next next hop node from route's dependent encap next hop list.
 *
 * @param[in] p_route Route node pointer.
 * @param[in] p_encap_nh Pointer to next hop node to which next node is got.
 * @return Pointer to the node if found otherwise NULL.
 */
sai_fib_nh_t* sai_fib_route_get_next_dep_encap_nh (sai_fib_route_t *p_route,
                                                   sai_fib_nh_t *p_encap_nh);

/**
 * @brief Get first next hop node from nh group's dependent encap next hop list.
 *
 * @param[in] p_nh_group Next Hop Group node pointer.
 * @return Pointer to the node if found otherwise NULL.
 */
sai_fib_nh_t* sai_fib_nh_group_get_first_dep_encap_nh (
                                                sai_fib_nh_group_t *p_nh_group);

/**
 * @brief Get next next hop node from nh group's dependent encap next hop list.
 *
 * @param[in] p_nh_group Next Hop Group node pointer.
 * @param[in] p_encap_nh Pointer to next hop node to which next node is got.
 * @return Pointer to the node if found otherwise NULL.
 */
sai_fib_nh_t* sai_fib_nh_group_get_next_dep_encap_nh (
                                                sai_fib_nh_group_t *p_nh_group,
                                                sai_fib_nh_t *p_encap_nh);
/**
 * @brief To check if the given IP address falls under the prefix/mask.
 *
 * @param[in] p_prefix Pointer to the prefix address.
 * @param[in] p_mask Pointer to the mask address.
 * @param[in] p_ip_address Pointer to IP address to be checked.
 * @return true if the condition is met otherwise false.
 */
bool sai_fib_is_ip_addr_in_prefix (sai_ip_address_t *p_prefix,
                                   sai_ip_address_t *p_mask,
                                   sai_ip_address_t *p_ip_address);

/**
 * @brief Utility to take simple mutex lock for FIB resources access.
 */
void sai_fib_lock (void);

/**
 * @brief Utility to release simple mutex lock for FIB resources access.
 */
void sai_fib_unlock (void);

/**
 * @brief Utility to check is_init_complete flag for SAI L3 component.
 *
 * @return Value of the is_init_complete flag.
 */
static inline bool sai_fib_is_init_complete (void)
{
    return (sai_fib_access_global_config()->is_init_complete);
}

/**
 * @brief Utility to get the number of virtual routers configured currently.
 *
 * @return Number of virtual routers.
 */
static inline uint32_t sai_fib_num_virtual_routers_get (void)
{
        return (sai_fib_access_global_config()->num_virtual_routers);
}

/**
 * @brief Utility to check whether router configurations exist.
 *
 * @return If router configuration is existing, returns true else false.
 */
static inline bool sai_fib_is_router_configured (void)
{
    return ((sai_fib_num_virtual_routers_get ()) > 0);
}

/**
 * @brief Utility to increase the number of virtual routers configured.
 */
static inline void sai_fib_num_virtual_routers_incr (void)
{
        sai_fib_access_global_config()->num_virtual_routers++;
}

/**
 * @brief Utility to decrease the number of virtual routers configured.
 */
static inline void sai_fib_num_virtual_routers_decr (void)
{
        sai_fib_access_global_config()->num_virtual_routers--;
}

/**
 * @brief Utility to check if VRF ID is valid.
 *
 * @param[in] vr_id Virtual Router ID.
 * @return true if valid, else false.
 */
static inline bool sai_fib_is_vrf_id_valid (uint_t vr_id)
{
    return (vr_id < (sai_fib_max_virtual_routers_get ()));
}

/**
 * @brief Wrapper to the STD DLL utility to get first node from a list.
 *
 * @param[in] p_dll_head  Pointer to the list head
 * @return Pointer to the first node in DLL
 */
static inline std_dll *sai_fib_dll_get_first (std_dll_head *p_dll_head)
{
    return (std_dll_getfirst (p_dll_head));
}

/**
 * @brief Wrapper to the STD DLL utility to get next node from a list.
 *
 * @param[in] p_dll_head  Pointer to the list head
 * @param[in] p_dll  Pointer to the previous node
 * @return Pointer to the next node in DLL
 */
static inline std_dll *sai_fib_dll_get_next (std_dll_head *p_dll_head,
                                             std_dll *p_dll)
{
    return ((p_dll != NULL) ? std_dll_getnext (p_dll_head, p_dll) : NULL);
}

/**
 * @brief Get first RIF node from VRF's RIF list. Helper to scan VRF RIF list.
 *
 * @param[in] p_vrf   Pointer to the VRF node
 * @return Pointer to the first RIF node in DLL
 */
static inline sai_fib_router_interface_t *sai_fib_get_first_rif_from_vrf (
                                            sai_fib_vrf_t *p_vrf )
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_fib_dll_get_first (&p_vrf->rif_dll_head))))
    {
        return ((sai_fib_router_interface_t *) (p_temp -
                                                SAI_FIB_RIF_DLL_GLUE_OFFSET));
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief Get next RIF node from VRF's RIF list. Helper to scan VRF RIF list.
 *
 * @param[in] p_vrf   Pointer to the VRF node
 * @param[in] p_rif  Pointer to the RIF node to which next node is got
 * @return Pointer to the next RIF node in DLL
 */
static inline sai_fib_router_interface_t *sai_fib_get_next_rif_from_vrf (
                                            sai_fib_vrf_t *p_vrf,
                                            sai_fib_router_interface_t *p_rif )
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_fib_dll_get_next (&p_vrf->rif_dll_head,
                                                     &p_rif->dll_glue))))
    {
        return ((sai_fib_router_interface_t *) (p_temp -
                                                SAI_FIB_RIF_DLL_GLUE_OFFSET));
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief Get first NH Group link node from NH's group list.
 * Helper to scan NH group list.
 *
 * @param[in] p_nh   Pointer to the next hop node
 * @return Pointer to the first NH group link node in DLL
 */
static inline sai_fib_wt_link_node_t *sai_fib_get_first_nh_group_from_nh (
                                            sai_fib_nh_t *p_nh )
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_fib_dll_get_first (&p_nh->nh_group_list))))
    {
        return ((sai_fib_wt_link_node_t *) (p_temp -
                                        SAI_FIB_WT_LINK_NODE_DLL_GLUE_OFFSET));
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief Get next NH Group link node from NH's group list.
 * Helper to scan NH group list.
 *
 * @param[in] p_nh   Pointer to the next hop node
 * @param[in] p_link_node Pointer to the link node to which next node is got
 * @return Pointer to the next NH Group link node in DLL
 */
static inline sai_fib_wt_link_node_t *sai_fib_get_next_nh_group_from_nh (
                                            sai_fib_nh_t *p_nh,
                                            sai_fib_wt_link_node_t *p_link_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_fib_dll_get_next (&p_nh->nh_group_list,
                                             &p_link_node->link_node.dll_glue))))
    {
        return ((sai_fib_wt_link_node_t *) (p_temp -
                                        SAI_FIB_WT_LINK_NODE_DLL_GLUE_OFFSET));
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief Get NH Group node pointer from the DLL link node.
 * Helper to scan NH Group list.
 *
 * @param[in] p_link_node    Pointer to the DLL link node
 * @return 'self' field of the DLL link node which points to NH Group node.
 */
static inline sai_fib_nh_group_t *sai_fib_get_nh_group_from_dll_link_node (
                                              sai_fib_link_node_t *p_link_node)
{
    return ((sai_fib_nh_group_t *) p_link_node->self);
}

/**
 * @brief Get first next hop link node from NH Group's nh list.
 * Helper to scan NH Group nh list.
 *
 * @param[in] p_nh_group   Pointer to the nh group node
 * @return Pointer to the first Next Hop link node in DLL
 */
static inline sai_fib_wt_link_node_t *sai_fib_get_first_nh_from_nh_group (
                                            sai_fib_nh_group_t *p_nh_group )
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_fib_dll_get_first (&p_nh_group->nh_list))))
    {
        return ((sai_fib_wt_link_node_t *) (p_temp -
                                        SAI_FIB_WT_LINK_NODE_DLL_GLUE_OFFSET));
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief Get next next hop link node from NH Group's nh list.
 * Helper to scan NH Group nh list.
 *
 * @param[in] p_nh_group   Pointer to the nh group node
 * @param[in] p_link_node Pointer to the link node to which next node is got
 * @return Pointer to the next Next Hop link node in DLL
 */
static inline sai_fib_wt_link_node_t *sai_fib_get_next_nh_from_nh_group (
                                            sai_fib_nh_group_t *p_nh_group,
                                            sai_fib_wt_link_node_t *p_link_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_fib_dll_get_next (&p_nh_group->nh_list,
                                            &p_link_node->link_node.dll_glue))))
    {
        return ((sai_fib_wt_link_node_t *) (p_temp -
                                        SAI_FIB_WT_LINK_NODE_DLL_GLUE_OFFSET));
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief Get Next Hop node pointer from the DLL link node.
 * Helper to scan NH Group nh list.
 *
 * @param[in] p_link_node    Pointer to the DLL link node
 * @return 'self' field of the DLL link node which points to Next Hop node.
 */
static inline sai_fib_nh_t *sai_fib_get_nh_from_dll_link_node (
                                              sai_fib_link_node_t *p_link_node)
{
    return ((sai_fib_nh_t *) p_link_node->self);
}

/**
 * @brief Return the IP address key field for the next hop node.
 *
 * @param[in] p_next_hop   Pointer to the next hop node
 * @return Pointer to the IP address key field in the next hop node
 */
static inline sai_ip_address_t *sai_fib_next_hop_ip_addr (sai_fib_nh_t *p_next_hop)
{
    return (&p_next_hop->key.info.ip_nh.ip_addr);
}

/**
 * @brief Check if SAI neighbor object is owner of the next hop node.
 *
 * @param[in] p_next_hop   Pointer to the next hop node
 * @return true if neighbor is owner else false
 */
static inline bool sai_fib_is_owner_neighbor (sai_fib_nh_t *p_next_hop)
{
    if (p_next_hop) {

        if (p_next_hop->owner_flag & (0x1 << SAI_FIB_OWNER_NEIGHBOR)) {

            return true;
        }
    }

    return false;
}

/**
 * @brief Check if SAI next hop object is owner of the next hop node.
 *
 * @param[in] p_next_hop   Pointer to the next hop node
 * @return true if next hop is owner else false
 */
static inline bool sai_fib_is_owner_next_hop (sai_fib_nh_t *p_next_hop)
{
    if (p_next_hop) {

        if (p_next_hop->owner_flag & (0x1 << SAI_FIB_OWNER_NEXT_HOP)) {

            return true;
        }
    }

    return false;
}

/**
 * @brief Check whether the router interface nodes are same.
 *
 * @param[in] p_rif_1 Pointer to router interface node.
 * @param[in] p_rif_2 Pointer to another router interface node to be compared.
 * @return true if the router interface node info are same, else false
 */
static inline bool sai_fib_rif_is_node_info_duplicate (
sai_fib_router_interface_t *p_rif_1, sai_fib_router_interface_t *p_rif_2)
{
    return (!(memcmp (p_rif_1, p_rif_2, sizeof (sai_fib_router_interface_t))));
}

/**
 * @brief Check whether the router interface is attached to LAG object.
 *
 * @param[in] p_rif_node Pointer to router interface node.
 * @return true if the router interface is attached to LAG, else false
 */
static inline bool sai_fib_rif_is_attachment_lag (sai_fib_router_interface_t *p_rif_node)
{
    return (sai_is_obj_id_lag (p_rif_node->attachment.port_id));
}

/**
 * @brief Check whether the route is a IPv6 full length route.
 *
 * @param[in] p_route Pointer to route node.
 * @return true if the route is a IPv6 full length route, else false
 */
static inline bool sai_fib_is_ipv6_full_length_route (sai_fib_route_t *p_route)
{
    return ((SAI_IP_ADDR_FAMILY_IPV6 == p_route->key.prefix.addr_family) &&
            (SAI_IPV6_ADDR_PREFIX_LEN == p_route->prefix_len));
}

/**
 * @brief Return the prefix length for the IP address family.
 *
 * @param[in] ip_addr Pointer to IP address struct.
 * @return Length in bits for the IP address family
 */
static inline uint_t sai_fib_ip_addr_family_len_get (sai_ip_address_t *ip_addr)
{
    return ((SAI_IP_ADDR_FAMILY_IPV6 == ip_addr->addr_family) ?
             (SAI_IPV6_ADDR_PREFIX_LEN) : (SAI_IPV4_ADDR_PREFIX_LEN));
}

/**
 * @brief Check if the Next Hop object is a Tunnel Encap Next Hop.
 *
 * @param[in] p_next_hop Pointer to Next Hop node.
 * @return true if Next Hop object is Tunnel Encap Next Hop false otherwise.
 */
static inline bool sai_fib_is_tunnel_encap_next_hop (sai_fib_nh_t *p_next_hop)
{
    return (p_next_hop->key.nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP);
}

/**
 * @brief Check if the 1d bridge RIF is attached to a 1D bridge.
 *
 * @param[in] p_rif_node Pointer to the RIF node.
 * @return true if 1D RIF is attached to the 1D router bridgeport, false
 * otherwise.
 */
static inline bool sai_fib_rif_is_attached_to_bridge(sai_fib_router_interface_t *p_rif_node)
{
    return (sai_is_obj_id_bridge(p_rif_node->attachment.bridge_id));
}
/**
 * \}
 */

#endif /* __SAI_L3_UTIL_H__ */
