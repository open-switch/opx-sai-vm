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
* @file sai_tunnel.h
*
* @brief This file contains the datastructure definitions for SAI Tunnel.
*
*************************************************************************/
#ifndef __SAI_TUNNEL_H__
#define __SAI_TUNNEL_H__

#include "saitunnel.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_type_defs.h"
#include "std_llist.h"
#include "std_rbtree.h"
#include "std_struct_utils.h"
/**
 * @brief SAI Tunnel data structure for the global parameters
 */
typedef struct _dn_sai_tunnel_global_t {

    /** Nodes of type dn_sai_tunnel_t */
    rbtree_handle    tunnel_db;

    /** Nodes of type dn_sai_tunnel_term_entry_t */
    rbtree_handle    tunnel_term_table_db;

    /** Nodes of type dn_sai_tunnel_map_t */
    rbtree_handle    tunnel_map_db;

    /** Nodes of type dn_sai_tunnel_map_entry_t */
    rbtree_handle    tunnel_map_entry_db;

    /** Bitmap for tunnel object index */
    uint8_t          *tunnel_obj_id_bitmap;

    /** Bitmap for tunnel termination object index */
    uint8_t          *tunnel_term_id_bitmap;

    /** flag to indicate if global params are initialized */
    bool             is_init_complete;
} dn_sai_tunnel_global_t;

/**
 * @brief SAI Tunnel attributes structure.
 *
 */
typedef struct _dn_sai_tunnel_params_t {

    /** Tunnel TTL mode attribute. */
    sai_tunnel_ttl_mode_t  ttl_mode;
    /** Tunnel DSCP mode attribute. */
    sai_tunnel_dscp_mode_t dscp_mode;
    /** TTL value for user defined tunnel ttl mode. */
    sai_uint8_t            ttl;
    /** DSCP value for user defined tunnel dscp mode. */
    sai_uint8_t            dscp;

} dn_sai_tunnel_params_t;

/**
 * @brief SAI Tunnel object data structure.
 * Contains the encap and decap attributes.
 *
 */
typedef struct _dn_sai_tunnel_t {

    /** Tunnel Id. Key parameter for the tunnel db */
    sai_object_id_t         tunnel_id;

    sai_tunnel_type_t       tunnel_type;
    sai_object_id_t         underlay_rif;
    sai_object_id_t         overlay_rif;
    sai_object_id_t         underlay_vrf;
    sai_object_id_t         overlay_vrf;

    /** Encap attributes */
    sai_ip_address_t        src_ip;
    dn_sai_tunnel_params_t  encap;

    /** Decap attributes */
    dn_sai_tunnel_params_t  decap;

    /** List of Tunnel Encap Next Hops in the tunnel. */
    std_dll_head            tunnel_encap_nh_list;

    /** List of Tunnel Termination entries in the tunnel. */
    std_dll_head            tunnel_term_entry_list;

    /** List of Tunnel Encap Mappers in the tunnel. */
    sai_object_list_t       tunnel_encap_mapper_list;

    /** List of Tunnel Decap Mappers in the tunnel. */
    sai_object_list_t       tunnel_decap_mapper_list;

    /** Number of bridge ports that reference this tunnel. */
    uint_t                  ref_count;

    /** Place holder for NPU-specific data */
    void                   *hw_info;
} dn_sai_tunnel_t;

/**
 * @brief SAI Tunnel Termination table entry data structure.
 *
 */
typedef struct _dn_sai_tunnel_term_entry_t {

    /** Tunnel termination entry Id. */
    sai_object_id_t                term_entry_id;

    /** Tunnel termination entry VR Id. */
    sai_object_id_t                vr_id;

    /** Tunnel termination entry type. */
    sai_tunnel_term_table_entry_type_t type;

    /** Tunnel termination entry keys. */
    sai_ip_address_t               src_ip;
    sai_ip_address_t               dst_ip;

    sai_tunnel_type_t              tunnel_type;

    /** Tunnel object id. */
    sai_object_id_t                tunnel_id;

    /** Tunnel node list pointers */
    std_dll                        tunnel_link;

    /** Place holder for NPU-specific data */
    void                           *hw_info;

} dn_sai_tunnel_term_entry_t;

/**
 * @brief SAI Tunnel map key value data structure.
 * Contains the members which can as act either as key
 * or as value.
 *
 */
typedef union _dn_sai_tunnel_map_key_value_t {

    /** Bridge object id */
    sai_object_id_t          bridge_oid;

    /** VXLAN virtual network identifier*/
    sai_uint32_t             vnid;

} dn_sai_tunnel_map_key_value_t;

/*
 * @brief SAI Tunnel map entry data structure.
 *
 */
typedef struct _dn_sai_tunnel_map_entry_t {

    /** Link to tunnel map node's tunnel map entry list*/
    std_dll                       tunnel_map_link;

    /** Tunnel map entry object id */
    sai_object_id_t               tunnel_map_entry_id;

    /** Tunnel map object id to which this node is
     *  associated */
    sai_object_id_t               tunnel_map_id;

    /** Tunnel map type */
    sai_tunnel_map_type_t         type;

    /** Tunnel map entry key */
    dn_sai_tunnel_map_key_value_t key;

    /** Tunnel map entry value */
    dn_sai_tunnel_map_key_value_t value;

} dn_sai_tunnel_map_entry_t;

/**
 * @brief SAI Tunnel map data structure.
 *
 */
typedef struct _dn_sai_tunnel_map_t {

    /** Tunnel Map object Id. */
    sai_object_id_t                map_id;

    /** Tunnel map type */
    sai_tunnel_map_type_t          type;

    /** List of tunnel map entries associated with
     *  this tunnel map */
    std_dll_head                   tunnel_map_entry_list;

    /** Count of tunnel map entries associated with
     *  this tunnel map */
    uint_t                         tunnel_map_entry_count;

    /** Reference count */
    uint_t                         ref_count;

    /** Place holder for NPU-specific data */
    void                          *hw_info;
} dn_sai_tunnel_map_t;

#define SAI_TUNNEL_MAP_ENTRY_DLL_GLUE_OFFSET \
        STD_STR_OFFSET_OF(dn_sai_tunnel_map_entry_t, tunnel_map_link)

#endif /* __SAI_TUNNEL_H__ */
