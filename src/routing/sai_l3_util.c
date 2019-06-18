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
* @file sai_l3_util.c
*
* @brief This file contains the util functions for SAI L3 component.
*
*************************************************************************/

#include "sai_l3_util.h"
#include "sai_l3_common.h"
#include "sai_oid_utils.h"
#include "sai_port_common.h"
#include "sai_port_utils.h"
#include "sairoute.h"
#include "saiswitch.h"
#include "sairouterinterface.h"
#include "saitypes.h"
#include "saistatus.h"
#include "ds_common_types.h"
#include "std_type_defs.h"
#include "std_ip_utils.h"
#include <string.h>
#include <arpa/inet.h>

static sai_mac_t g_zero_mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

bool sai_fib_is_mac_address_zero (const sai_mac_t *p_mac)
{
    if (memcmp (p_mac, &g_zero_mac, sizeof (sai_mac_t)))
    {
        return false;
    } else {
        return true;
    }
}

uint_t sai_fib_max_ecmp_paths_get (void)
{
    return (sai_fib_access_global_config()->max_ecmp_paths);
}

uint_t sai_fib_max_virtual_routers_get (void)
{
    return (sai_fib_access_global_config()->max_virtual_routers);
}

sai_fib_vrf_t* sai_fib_vrf_node_get (sai_object_id_t vrf_id)
{
    rbtree_handle  vrf_tree;
    sai_fib_vrf_t  vrf_entry;

    memset (&vrf_entry, 0, sizeof (sai_fib_vrf_t));
    vrf_entry.vrf_id = vrf_id;

    vrf_tree = sai_fib_access_global_config()->vrf_tree;

    return ((sai_fib_vrf_t *) std_rbtree_getexact (vrf_tree, &vrf_entry));
}

sai_fib_router_interface_t* sai_fib_router_interface_node_get (
                                           sai_object_id_t rif_id)
{
    rbtree_handle  rif_tree;
    sai_fib_router_interface_t rif_entry;

    memset (&rif_entry, 0, sizeof (sai_fib_router_interface_t));
    rif_entry.rif_id = rif_id;

    rif_tree = sai_fib_access_global_config()->router_interface_tree;

    return ((sai_fib_router_interface_t *) std_rbtree_getexact (rif_tree,
                                                                &rif_entry));
}

bool sai_fib_is_rif_created (sai_object_id_t rif_id)
{
    if(sai_fib_router_interface_node_get(rif_id) != NULL) {
        return true;
    }
    return false;
}

sai_status_t sai_rif_increment_ref_count (sai_object_id_t rif_id)
{
    sai_fib_router_interface_t *p_rif_node = sai_fib_router_interface_node_get(rif_id);

    if(p_rif_node == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    p_rif_node->ref_count++;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_rif_decrement_ref_count (sai_object_id_t rif_id)
{
    sai_fib_router_interface_t *p_rif_node = sai_fib_router_interface_node_get(rif_id);

    if(p_rif_node == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    if(p_rif_node->ref_count == 0) {
        return SAI_STATUS_FAILURE;
    }
    p_rif_node->ref_count--;
    return SAI_STATUS_SUCCESS;
}

sai_fib_vrf_t* sai_fib_get_vrf_node_for_rif (sai_object_id_t rif_id)
{
    sai_fib_router_interface_t  *p_rif_node = NULL;
    sai_fib_vrf_t               *p_vrf_node = NULL;

    if ((p_rif_node = sai_fib_router_interface_node_get (rif_id))) {

        p_vrf_node = sai_fib_vrf_node_get (p_rif_node->vrf_id);
    }

    return p_vrf_node;
}

sai_fib_nh_group_t* sai_fib_next_hop_group_get (sai_object_id_t id)
{
    rbtree_handle      nh_group_tree;
    sai_fib_nh_group_t group_entry;

    memset (&group_entry, 0, sizeof (sai_fib_nh_group_t));
    group_entry.key.group_id = id;

    nh_group_tree = sai_fib_access_global_config()->nh_group_tree;

    return ((sai_fib_nh_group_t *) std_rbtree_getexact (nh_group_tree,
                                                        &group_entry));
}

sai_fib_nh_t* sai_fib_next_hop_node_get_from_id (sai_object_id_t nh_id)
{
    rbtree_handle  nh_id_tree;
    sai_fib_nh_t   nh_entry;

    memset (&nh_entry, 0, sizeof (sai_fib_nh_t));
    nh_entry.next_hop_id = nh_id;

    nh_id_tree = sai_fib_access_global_config()->nh_id_tree;

    return ((sai_fib_nh_t *) std_rbtree_getexact (nh_id_tree, &nh_entry));
}

sai_fib_nh_t* sai_fib_ip_next_hop_node_get (sai_next_hop_type_t nh_type,
                                            sai_object_id_t rif_id,
                                            sai_ip_address_t *p_ip_addr,
                                            sai_tunnel_type_t tunnel_type)
{
    sai_fib_nh_t        *p_nh_node = NULL;
    sai_fib_vrf_t       *p_vrf_node = NULL;
    sai_fib_nh_key_t     nh_key;
    sai_fib_ip_nh_key_t *p_ip_nh_key;

    memset (&nh_key, 0, sizeof (sai_fib_nh_key_t));

    nh_key.nh_type = nh_type;
    nh_key.rif_id  = rif_id;
    if (nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP) {
        nh_key.tunnel_type = tunnel_type;
    } else {
        nh_key.tunnel_type = SAI_FIB_TUNNEL_TYPE_NONE;
    }

    p_ip_nh_key      = &nh_key.info.ip_nh;

    if (sai_fib_ip_addr_copy (&p_ip_nh_key->ip_addr, p_ip_addr) !=
        SAI_STATUS_SUCCESS) {

        return NULL;
    }

    p_vrf_node = sai_fib_get_vrf_node_for_rif (rif_id);

    if (p_vrf_node) {

        p_nh_node = (sai_fib_nh_t *)
                     std_radix_getexact (p_vrf_node->sai_nh_tree,
                                         (uint8_t *) &nh_key,
                                         SAI_FIB_NH_IP_ADDR_TREE_KEY_LEN);
    }

    return p_nh_node;
}

sai_fib_wt_link_node_t *sai_fib_nh_find_group_link_node (
                                                sai_fib_nh_t *p_nh_node,
                                                sai_fib_nh_group_t *p_nh_group)
{
    sai_fib_wt_link_node_t *p_wt_link_node;

    if ((!p_nh_node)) {

        return NULL;
    }

    for (p_wt_link_node = sai_fib_get_first_nh_group_from_nh (p_nh_node);
         p_wt_link_node; p_wt_link_node = sai_fib_get_next_nh_group_from_nh (
         p_nh_node, p_wt_link_node))
    {
        if (p_nh_group ==
            sai_fib_get_nh_group_from_dll_link_node (&p_wt_link_node->link_node)) {

            return p_wt_link_node;
        }
    }

    return NULL;
}

sai_fib_wt_link_node_t *sai_fib_nh_group_find_nh_link_node (
                                                sai_fib_nh_group_t *p_nh_group,
                                                sai_fib_nh_t *p_nh_node)
{
    sai_fib_wt_link_node_t *p_wt_link_node;

    if ((!p_nh_group)) {

        return NULL;
    }

    for (p_wt_link_node = sai_fib_get_first_nh_from_nh_group (p_nh_group);
         p_wt_link_node; p_wt_link_node = sai_fib_get_next_nh_from_nh_group (
         p_nh_group, p_wt_link_node))
    {
        if (p_nh_node ==
            sai_fib_get_nh_from_dll_link_node (&p_wt_link_node->link_node)) {

            return p_wt_link_node;
        }
    }

    return NULL;
}

const char *sai_fib_rif_type_to_str (uint_t type)
{
    if (type == SAI_ROUTER_INTERFACE_TYPE_PORT) {
        return "Port";
    } else if (type == SAI_ROUTER_INTERFACE_TYPE_VLAN) {
        return "Vlan";
    } else if (type == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK) {
        return "Loopback";
    } else if (type == SAI_ROUTER_INTERFACE_TYPE_BRIDGE) {
        return "Bridge";
    } else {
        return "Invalid";
    }
}

uint_t sai_fib_rif_attachment_id_get (sai_fib_router_interface_t *p_rif_node)
{
    sai_npu_port_id_t npu_port_id = 0;

    /* This utility returns the hw id for the RIF VLAN, Port/LAG attachment
     * which is safe to be fit in the uint_t type */
    if (p_rif_node->type == SAI_ROUTER_INTERFACE_TYPE_PORT) {
        if (sai_fib_rif_is_attachment_lag (p_rif_node)) {
            return ((uint_t) sai_uoid_npu_obj_id_get (p_rif_node->attachment.port_id));
        } else {
            sai_port_to_npu_local_port (p_rif_node->attachment.port_id,
                                        &npu_port_id);
            return ((uint_t) npu_port_id);
        }
    } else if (p_rif_node->type == SAI_ROUTER_INTERFACE_TYPE_VLAN){
        return (p_rif_node->attachment.vlan_id);
    } else {
        return 0;
    }
}

static void sai_fib_convert_to_std_ip_addr (const sai_ip_address_t *p_ip_addr,
                                            hal_ip_addr_t *p_std_ip_addr)
{
    if (p_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {

        p_std_ip_addr->af_index = AF_INET;
        memcpy ((uint8_t *) &p_std_ip_addr->u.v4_addr,
                (uint8_t *) &p_ip_addr->addr.ip4,
                sizeof (p_std_ip_addr->u.v4_addr));
    } else {

        p_std_ip_addr->af_index = AF_INET6;
        memcpy ((uint8_t *) &p_std_ip_addr->u.v6_addr,
                (uint8_t *) &p_ip_addr->addr.ip6,
                sizeof (p_std_ip_addr->u.v6_addr));
    }
}

const char *sai_ip_addr_to_str (const sai_ip_address_t *p_ip_addr,
                                char *p_buf, size_t len)
{
    hal_ip_addr_t std_ip_addr;

    sai_fib_convert_to_std_ip_addr (p_ip_addr, &std_ip_addr);

    return (std_ip_to_string (&std_ip_addr, p_buf, len));
}

const char *sai_ipv4_addr_to_str (const uint8_t *p_addr_bytes,
                                  char *p_buf, size_t len)
{
    return (inet_ntop (AF_INET, (const void *) (p_addr_bytes), p_buf, len));
}

const char *sai_ipv6_addr_to_str (const uint8_t *p_addr_bytes,
                                  char *p_buf, size_t len)
{
    return (inet_ntop (AF_INET6, (const void *) (p_addr_bytes), p_buf, len));
}

sai_status_t sai_fib_ip_addr_copy (sai_ip_address_t *p_dest_ip_addr,
                                   const sai_ip_address_t *p_src_ip_addr)
{
    if (p_src_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {

        p_dest_ip_addr->addr.ip4 = p_src_ip_addr->addr.ip4;

    } else if (p_src_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV6) {

        memcpy (p_dest_ip_addr->addr.ip6, p_src_ip_addr->addr.ip6,
                sizeof (sai_ip6_t));

    } else {

        return SAI_STATUS_FAILURE;
    }

    p_dest_ip_addr->addr_family = p_src_ip_addr->addr_family;

    return SAI_STATUS_SUCCESS;
}

bool sai_fib_is_ip_addr_zero (const sai_ip_address_t *p_ip_addr)
{
    hal_ip_addr_t std_ip_addr;

    if (p_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {

        return ((p_ip_addr->addr.ip4 == 0) ? true : false);

    } else {

        sai_fib_convert_to_std_ip_addr (p_ip_addr, &std_ip_addr);

        return (std_ip_is_v6_addr_zero (&std_ip_addr));
    }
}

bool sai_fib_is_ip_addr_loopback (const sai_ip_address_t *p_ip_addr)
{
    hal_ip_addr_t std_ip_addr;

    sai_fib_convert_to_std_ip_addr (p_ip_addr, &std_ip_addr);

    if (p_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {

        /* SAI ip addr is in network byte order. */
        std_ip_addr.u.v4_addr = ntohl (std_ip_addr.u.v4_addr);

        return (std_ip_is_v4_addr_loopback (&std_ip_addr));

    } else {

        return (std_ip_is_v6_addr_loopback (&std_ip_addr));
    }
}

const char *sai_fib_next_hop_type_str (sai_next_hop_type_t type)
{
    switch (type) {
        case SAI_NEXT_HOP_TYPE_IP:
            return "IP";

        case SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP:
            return "Tunnel Encap";

        default:
            return "INVALID";
    }
}

const char *sai_fib_nh_group_type_str (sai_next_hop_group_type_t type)
{
    switch (type) {

        case SAI_NEXT_HOP_GROUP_TYPE_ECMP:
            return "ECMP";

        default:
            return "INVALID";
    }
}

bool sai_packet_action_validate (sai_packet_action_t pkt_action)
{
    if ((pkt_action == SAI_PACKET_ACTION_DROP) ||
        (pkt_action == SAI_PACKET_ACTION_FORWARD) ||
        (pkt_action == SAI_PACKET_ACTION_TRAP) ||
        (pkt_action == SAI_PACKET_ACTION_LOG))
    {
        return true;
    }

    return false;
}

const char *sai_packet_action_str (sai_packet_action_t pkt_action)
{
    switch (pkt_action) {
        case SAI_PACKET_ACTION_DROP:
            return "DROP";

        case SAI_PACKET_ACTION_FORWARD:
            return "FORWARD";

        case SAI_PACKET_ACTION_TRAP:
            return "TRAP";

        case SAI_PACKET_ACTION_LOG:
            return "LOG";

        default:
            return "INVALID";
    }
}

sai_status_t sai_fib_attr_status_code_get (sai_status_t status, uint_t attr_index)
{
    if ((status == SAI_STATUS_INVALID_ATTRIBUTE_0) ||
        (status == SAI_STATUS_INVALID_ATTR_VALUE_0) ||
        (status == SAI_STATUS_ATTR_NOT_IMPLEMENTED_0) ||
        (status == SAI_STATUS_UNKNOWN_ATTRIBUTE_0) ||
        (status == SAI_STATUS_ATTR_NOT_SUPPORTED_0)) {

        return (status + SAI_STATUS_CODE (attr_index));
    } else {

        return status;
    }
}

const char *sai_fib_route_nh_type_to_str (uint_t nh_type)
{
    if (nh_type == SAI_OBJECT_TYPE_NEXT_HOP_GROUP) {
        return "NEXT_HOP_GROUP_ID";
    } else if (nh_type == SAI_OBJECT_TYPE_NEXT_HOP) {
        return "NEXT_HOP";
    } else if (nh_type == SAI_FIB_ROUTE_NH_TYPE_NONE) {
        return "NONE";
    } else {
        return "INVALID";
    }
}

sai_object_id_t sai_fib_route_node_nh_id_get (sai_fib_route_t *p_route)
{
    if (!p_route) {
        return 0;
    }

    if ((p_route->nh_type == SAI_OBJECT_TYPE_NEXT_HOP) &&
        (p_route->nh_info.nh_node)) {
        return (p_route->nh_info.nh_node)->next_hop_id;
    } else if ((p_route->nh_type == SAI_OBJECT_TYPE_NEXT_HOP_GROUP) &&
               (p_route->nh_info.group_node)) {
        return (p_route->nh_info.group_node)->key.group_id;
    } else {
        return 0;
    }
}

sai_fib_neighbor_mac_entry_t *sai_fib_neighbor_mac_entry_find (
                                const sai_fib_neighbor_mac_entry_key_t *p_key)
{
    return ((sai_fib_neighbor_mac_entry_t *)
            std_radix_getexact (sai_fib_access_global_config()->neighbor_mac_tree,
                                (uint8_t *) p_key,
                                SAI_FIB_NEIGHBOR_MAC_ENTRY_TREE_KEY_LEN));
}

sai_fib_nh_t* sai_fib_get_first_neighbor_from_mac_entry (
                                sai_fib_neighbor_mac_entry_t *p_mac_entry)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
                   sai_fib_dll_get_first (&p_mac_entry->neighbor_list)))) {

        return ((sai_fib_nh_t *) (p_temp -
                                  SAI_FIB_NEIGHBOR_MAC_ENTRY_DLL_GLUE_OFFSET));
    } else {

        return NULL;
    }
}

sai_fib_nh_t* sai_fib_get_next_neighbor_from_mac_entry (
                                sai_fib_neighbor_mac_entry_t *p_mac_entry,
                                sai_fib_nh_t *p_neighbor)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
                   sai_fib_dll_get_next (&p_mac_entry->neighbor_list,
                                         &p_neighbor->mac_entry_link)))) {

        return ((sai_fib_nh_t *) (p_temp -
                                  SAI_FIB_NEIGHBOR_MAC_ENTRY_DLL_GLUE_OFFSET));
    } else {

        return NULL;
    }
}

void sai_fib_ip_prefix_mask_get (sai_ip_address_t *ip_mask,
                                 uint_t prefix_len)
{
    hal_ip_addr_t std_ip_addr;

    std_ip_addr.af_index = (ip_mask->addr_family == SAI_IP_ADDR_FAMILY_IPV4) ?
                             AF_INET : AF_INET6;

    std_ip_get_mask_from_prefix_len (std_ip_addr.af_index, prefix_len,
                                     &std_ip_addr);

    if (ip_mask->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
        memcpy ((uint8_t *) &ip_mask->addr.ip4,
                (uint8_t *) &std_ip_addr.u.v4_addr,
                sizeof (std_ip_addr.u.v4_addr));
        /* SAI ip addr is in network byte order. */
        ip_mask->addr.ip4 = htonl (ip_mask->addr.ip4);

    } else {
        memcpy ((uint8_t *) ip_mask->addr.ip6,
                (uint8_t *) std_ip_addr.u.v6_addr,
                sizeof (std_ip_addr.u.v6_addr));
    }
}

sai_fib_route_t* sai_fib_get_first_dep_route_from_nh (sai_fib_nh_t *p_next_hop)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
                   sai_fib_dll_get_first (&p_next_hop->dep_route_list)))) {

        return ((sai_fib_route_t *) (p_temp -
                                     SAI_FIB_ROUTE_NH_DEP_DLL_GLUE_OFFSET));
    } else {

        return NULL;
    }
}

sai_fib_route_t* sai_fib_get_next_dep_route_from_nh (sai_fib_nh_t *p_next_hop,
                                                     sai_fib_route_t *p_route)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
                   sai_fib_dll_get_next (&p_next_hop->dep_route_list,
                                         &p_route->nh_dep_route_link)))) {

        return ((sai_fib_route_t *) (p_temp -
                                     SAI_FIB_ROUTE_NH_DEP_DLL_GLUE_OFFSET));
    } else {

        return NULL;
    }
}

sai_fib_nh_t* sai_fib_route_get_first_dep_encap_nh (sai_fib_route_t *p_route)
{

    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
                   sai_fib_dll_get_first (&p_route->dep_encap_nh_list)))) {

        return ((sai_fib_nh_t *) (p_temp -
                                  SAI_FIB_ENCAP_NH_UNDERLAY_ROUTE_DLL_GLUE_OFFSET));
    } else {

        return NULL;
    }
}

sai_fib_nh_t* sai_fib_route_get_next_dep_encap_nh (sai_fib_route_t *p_route,
                                                   sai_fib_nh_t *p_encap_nh)
{

    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
                   sai_fib_dll_get_next (&p_route->dep_encap_nh_list,
                                         &p_encap_nh->underlay_route_link)))) {

        return ((sai_fib_nh_t *) (p_temp -
                                  SAI_FIB_ENCAP_NH_UNDERLAY_ROUTE_DLL_GLUE_OFFSET));
    } else {

        return NULL;
    }
}

sai_fib_nh_t* sai_fib_nh_group_get_first_dep_encap_nh (
                                                sai_fib_nh_group_t *p_nh_group)
{

    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
                   sai_fib_dll_get_first (&p_nh_group->dep_encap_nh_list)))) {

        return ((sai_fib_nh_t *) (p_temp -
                                  SAI_FIB_ENCAP_NH_UNDERLAY_NHG_DLL_GLUE_OFFSET));
    } else {

        return NULL;
    }
}

sai_fib_nh_t* sai_fib_nh_group_get_next_dep_encap_nh (
                                                sai_fib_nh_group_t *p_nh_group,
                                                sai_fib_nh_t *p_encap_nh)
{

    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
                   sai_fib_dll_get_next (&p_nh_group->dep_encap_nh_list,
                                         &p_encap_nh->underlay_nhg_link)))) {

        return ((sai_fib_nh_t *) (p_temp -
                                  SAI_FIB_ENCAP_NH_UNDERLAY_NHG_DLL_GLUE_OFFSET));
    } else {

        return NULL;
    }
}

bool sai_fib_is_ip_addr_in_prefix (sai_ip_address_t *prefix,
                                   sai_ip_address_t *mask,
                                   sai_ip_address_t *ip_address)
{
    uint_t  byte;

    if (prefix->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {

        return (((prefix->addr.ip4) & (mask->addr.ip4)) ==
                ((ip_address->addr.ip4) & (mask->addr.ip4)));

    } else if (prefix->addr_family == SAI_IP_ADDR_FAMILY_IPV6) {

        for (byte = 0; byte < HAL_INET6_LEN; byte++)
        {
            if (((prefix->addr.ip6[byte]) & (mask->addr.ip6[byte])) !=
                ((ip_address->addr.ip6[byte]) & (mask->addr.ip6[byte]))) {

                return false;
            }
        }

        return true;

    } else {

        return false;
    }
}
