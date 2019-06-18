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
* @file sai_l3_common.h
*
* @brief This file contains the datastructure definitions for SAI L3 objects.
*
*************************************************************************/
#ifndef __SAI_L3_COMMON_H__
#define __SAI_L3_COMMON_H__

#include "std_type_defs.h"
#include "std_llist.h"
#include "std_rbtree.h"
#include "std_radix.h"
#include "std_radical.h"

#include "saivirtualrouter.h"
#include "sairouterinterface.h"
#include "sainexthop.h"
#include "sainexthopgroup.h"
#include "sairoute.h"
#include "saiswitch.h"
#include "saitunnel.h"

/**
 * @brief SAI L3 data structure for the global parameters
 */
typedef struct _sai_fib_global_t {

    /** Nodes of type sai_fib_vrf_t */
    rbtree_handle    vrf_tree;

    /** Nodes of type sai_fib_router_interface_t */
    rbtree_handle    router_interface_tree;

    /** Nodes of type sai_fib_nh_group_t */
    rbtree_handle    nh_group_tree;

    /** Next Hop ID based tree. Nodes of type sai_fib_nh_t */
    rbtree_handle    nh_id_tree;

    /** Neighbor MAC entry tree. Nodes of type sai_fib_neighbor_mac_entry_t */
    std_rt_table    *neighbor_mac_tree;

    /** SAI_SWITCH_ATTR_MAX_ECMP_PATHS */
    uint_t           max_ecmp_paths;

    /** SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS */
    uint_t           max_virtual_routers;

    /** Number of routers created */
    uint_t           num_virtual_routers;

    /** flag to indicate if global params are initialized */
    bool             is_init_complete;

} sai_fib_global_t;


/**
 * @brief SAI L3 VRF data structure. Contains the VR attributes.
 *
 * Contains the List of RIFs, Next Hop and Route trees in the VRF.
 */
typedef struct _sai_fib_vrf_t {

    /** VR ID. Key parameter for the VRF tree */
    sai_object_id_t      vrf_id;

    /** VR attributes */
    sai_mac_t        src_mac;
    bool             v4_admin_state;
    bool             v6_admin_state;

    /** Action for Packets with TTL error */
    sai_packet_action_t ttl0_1_pkt_action;

    /** Action for Packets with IP options */
    sai_packet_action_t ip_options_pkt_action;

    /** Number of router interfaces in the VRF */
    uint_t         num_rif;

    /** Router interface list head. Nodes of type sai_fib_router_interface_t */
    std_dll_head   rif_dll_head;

    /** Next Hop tree. Nodes of type sai_fib_nh_t */
    std_rt_table    *sai_nh_tree;

    /** Route tree. Nodes of type sai_fib_route_t */
    std_rt_table    *sai_route_tree;

    /** Dummy Marker node to be passsed for the route tree radical walk */
    std_radical_ref_t  route_marker;

    /** Place holder for NPU-specific data */
    void            *hw_info;
} sai_fib_vrf_t;

/**
 * @brief SAI L3 Router Interface data structure.
 *
 * Contains the Router Interface attributes.
 */
typedef struct _sai_fib_router_interface_t {

    /** RIF Id. Key parameter for the RIF tree*/
    sai_object_id_t              rif_id;

    /** Link to the VRF RIF list */
    std_dll                      dll_glue;

    /** Port or VLAN interface */
    sai_router_interface_type_t  type;

    union {
        sai_object_id_t  port_id;
        sai_object_id_t  vlan_id;
        sai_object_id_t  bridge_id;
    } attachment;

    /** RIF attributes */
    sai_object_id_t              vrf_id;
    sai_mac_t                    src_mac;
    bool                         v4_admin_state;
    bool                         v6_admin_state;
    uint_t                       mtu;
    bool                         is_virtual;

    /** Action for Packets with IP options */
    sai_packet_action_t          ip_options_pkt_action;

    uint_t                       ref_count;

    /** Place holder for NPU-specific data */
    void                        *hw_info;
} sai_fib_router_interface_t;

/**
 * @brief Key structure for IP Next Hop
 *
 */
typedef struct _sai_fib_ip_nh_key_t {
    sai_ip_address_t            ip_addr;
} sai_fib_ip_nh_key_t;

/**
 * @brief Key structure for NH tree node
 *
 */
typedef struct _sai_fib_nh_key_t {
    /** Next Hop type */
    sai_next_hop_type_t         nh_type;

    /** Key information */
    union {
        sai_fib_ip_nh_key_t     ip_nh;
    } info;

    /** Next Hop Router Interface Id */
    sai_object_id_t             rif_id;

    /** Valid when next hop type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP */
    sai_tunnel_type_t           tunnel_type;

} sai_fib_nh_key_t;

/**
 * @brief SAI L3 Next Hop data structure.
 *
 */
typedef struct _sai_fib_nh_t {
    /** Radix head for NH tree */
    std_rt_head                 rt_head;

    /** Key structure for NH tree */
    sai_fib_nh_key_t            key;

    /** Key for NH ID tree */
    sai_object_id_t             next_hop_id;

    sai_object_id_t             vrf_id;
    sai_mac_t                   mac_addr;

    /** Indicate if the node belongs to next hop and/or neighbor object */
    uint_t                      owner_flag;

    /** Packet action for the next-hop. */
    sai_packet_action_t         packet_action;

    /** True if Unable to resolve port id from FDB entry */
    bool                        port_unresolved;

    /** Back reference to NH group(s). Nodes of type sai_fib_wt_link_node_t,
     * the 'self' field points to sai_fib_nh_group_t */
    std_dll_head                nh_group_list;

    /** Neighbor host route entry to be added or not */
    bool                        no_host_route;

    uint_t                      ref_count;

    /** Neighbor Meta Data */
    uint_t                      meta_data;

    /** Egress Port Id for VLAN RIF based Neighbor entries */
    sai_object_id_t             port_id;

    /** Neighbor MAC entry node list pointers */
    std_dll                     mac_entry_link;

    /** List of Tunnel Encap Next Hops that depends on this Next Hop.
     * Nodes of type sai_fib_link_node_t, the 'self' field points to
     * sai_fib_nh_t */
    std_dll_head                dep_encap_nh_list;

    /** Tunnel object id for Tunnel Encap Next Hop */
    sai_object_id_t             tunnel_id;

    /** List of overlay routes that are dependent on Tunnel Encap Next Hop. */
    std_dll_head                dep_route_list;

    /** Best Matching route for the Tunnel Encap Next Hop IP Address */
    struct _sai_fib_route_t    *lpm_route;

    /** Directly connected neighbor for the Tunnel Encap Next Hop IP Address */
    struct _sai_fib_nh_t       *neighbor;

    /** Tunnel node list pointers */
    std_dll                     tunnel_link;

    /** Underlay Next Hop Group node list pointers */
    std_dll                     underlay_nhg_link;

    /** Underlay Route node list pointers */
    std_dll                     underlay_route_link;

    /** Place holder for NPU-specific data */
    void                       *hw_info;
} sai_fib_nh_t;

/**
 * @brief Key structure for Neighbor MAC entry node
 *
 */
typedef struct _sai_fib_neighbor_mac_entry_key_t {
    /** Neighbor VLAN */
    sai_vlan_id_t    vlan_id;
    /** Neighbor MAC address */
    sai_mac_t        mac_addr;
} sai_fib_neighbor_mac_entry_key_t;

/**
 * @brief SAI Neighbor MAC entry based data structure
 *
 */
typedef struct _sai_fib_neighbor_mac_entry_t {
    /** Radix head for Neighbor MAC entry tree node */
    std_rt_head                       rt_head;

    /** Key information for Neighbor MAC entry */
    sai_fib_neighbor_mac_entry_key_t  key;

    /** Head of the list of Neighbor nodes */
    std_dll_head                      neighbor_list;
} sai_fib_neighbor_mac_entry_t;

/**
 * @brief Data structure used for DLL link nodes
 *
 */
typedef struct _sai_fib_link_node_t {

    /** Link used to point to the list nodes */
    std_dll                     dll_glue;

    /** Pointer to data node */
    void                       *self;
} sai_fib_link_node_t;

/**
 * @brief Data structure used for DLL Weighted link nodes. Weight
 * represents the number of times this node is added in the list.
 *
 */
typedef struct _sai_fib_wt_link_node_t {

    /** DLL link node structure */
    sai_fib_link_node_t   link_node;

    /** Weight of the node */
    uint_t                weight;

} sai_fib_wt_link_node_t;

/**
 * @brief Key structure for NH Group tree
 *
 */
typedef struct _sai_fib_nh_group_key_t {
    sai_object_id_t     group_id;
} sai_fib_nh_group_key_t;

/**
 * @brief SAI L3 NH Group data structure.
 *
 */
typedef struct _sai_fib_nh_group_t {

    /** Key structure for NH Group tree*/
    sai_fib_nh_group_key_t      key;

    sai_next_hop_group_type_t   type;

    /** Number of next hops added to the group */
    uint_t                      nh_count;

    /** List of next hops added to the group. Nodes of type
     * sai_fib_wt_link_node_t, the 'self' field points to
     * sai_fib_nh_t */
    std_dll_head                nh_list;

    uint_t                      ref_count;

    /** List of Encap Next Hops that depends on this NH Group. */
    std_dll_head                dep_encap_nh_list;

    /** Place holder for NPU-specific data */
    void                       *hw_info;
} sai_fib_nh_group_t;

/**
 * @brief Key structure for Route tree
 *
 */
typedef struct _sai_fib_route_key_t {
    sai_ip_address_t   prefix;
} sai_fib_route_key_t;

/**
 * @brief SAI L3 Route data structure.
 *
 */
typedef struct _sai_fib_route_t {
    /** Radical tree head for route ree */
    std_radical_head_t         rt_head;

    /** Key structure for route tree*/
    sai_fib_route_key_t        key;

    /** Prefix length of the route */
    uint_t                     prefix_len;

    /** VRF Id the route belongs to */
    sai_object_id_t            vrf_id;

    /** ROUTE_ATTR_NEXT_HOP_ID/ROUTE_ATTR_NEXT_HOP_GROUP_ID */
    uint_t                     nh_type;

    union {
        /** Pointer to the NH group associated with the route */
        sai_fib_nh_group_t        *group_node;

        /** Pointer to the NH associated with the route */
        sai_fib_nh_t              *nh_node;
    } nh_info;

    /** Route attributes */
    sai_packet_action_t        packet_action;
    uint_t                     trap_priority;

    /** Route Meta Data */
    uint_t                     meta_data;

    /** List of Encap Next Hops that depends on this route. */
    std_dll_head               dep_encap_nh_list;

    /** Next Hop dependent route list pointers */
    std_dll                    nh_dep_route_link;

    /** Place holder for NPU-specific data */
    void                      *hw_info;
} sai_fib_route_t;

/**
 * @brief Enumeration to indicate ownership of the SAI Next Hop node.
 *
 */
typedef enum _sai_fib_nh_owner_flag {

    SAI_FIB_OWNER_NEIGHBOR,
    SAI_FIB_OWNER_NEXT_HOP,

} sai_fib_nh_owner_flag;


#endif /* __SAI_L3_COMMON_H__ */
