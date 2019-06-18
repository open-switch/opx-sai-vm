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
* @file  sai_l3_debug.c
*
* @brief This file contains function definitions for debug routines to
*        dump the data structure information for L3 SAI objects.
*
*************************************************************************/
#include "saitypes.h"
#include "sai_l3_util.h"
#include "sai_l3_common.h"
#include "sai_debug_utils.h"
#include "std_type_defs.h"
#include "std_mac_utils.h"
#include "std_struct_utils.h"
#include <arpa/inet.h>
#include <string.h>
#include <inttypes.h>

#define SAI_FIB_DBG_MAX_BUFSZ  (256)

static inline uint_t sai_fib_addr_family_bitlen (void)
{
    return ((STD_STR_SIZE_OF(sai_ip_address_t, addr_family)) * BITS_PER_BYTE);
}

void sai_fib_dump_help (void)
{
    SAI_DEBUG ("******************** Dump routines ***********************");

    SAI_DEBUG ("  void sai_fib_dump_vr (sai_object_id_t vr_id)");
    SAI_DEBUG ("  void sai_fib_dump_all_vr (void)");
    SAI_DEBUG ("  void sai_fib_dump_all_rif_in_vr (sai_object_id_t vr_id)");
    SAI_DEBUG ("  void sai_fib_dump_rif (sai_object_id_t rif_id)");
    SAI_DEBUG ("  void sai_fib_dump_all_rif (void)");
    SAI_DEBUG ("  void sai_fib_dump_nh (sai_object_id_t nh_id)");
    SAI_DEBUG ("  void sai_fib_dump_all_nh (void)");
    SAI_DEBUG ("  void sai_fib_dump_neighbor_entry (int af_family, ");
    SAI_DEBUG ("       const char *ip_str, sai_object_id_t rif_id)");
    SAI_DEBUG ("  void sai_fib_dump_all_neighbor_in_vr (sai_object_id_t vr_id)");
    SAI_DEBUG ("  void sai_fib_dump_nh_group (sai_object_id_t group_id)");
    SAI_DEBUG ("  void sai_fib_dump_all_nh_group (void)");
    SAI_DEBUG ("  void sai_fib_dump_nh_list_from_nh_group (sai_object_id_t group_id)");
    SAI_DEBUG ("  void sai_fib_dump_route_entry (sai_object_id_t vrf, ");
    SAI_DEBUG ("       int af, char *ip_str, uint_t prefix_len)");
    SAI_DEBUG ("  void sai_fib_dump_all_route_in_vr (sai_object_id_t vr_id)");
    SAI_DEBUG ("  void sai_fib_dump_neighbor_mac_entry_tree (void)");
    SAI_DEBUG ("  void sai_fib_dump_dep_encap_nh_list_for_route (");
    SAI_DEBUG ("  sai_object_id_t vr, int af, char *ip_str, uint_t prefix_len)");
    SAI_DEBUG ("  void sai_fib_dump_dep_encap_nh_list_for_nexthop (int af, ");
    SAI_DEBUG ("       const char *ip_str, sai_object_id_t rif_id)");
    SAI_DEBUG ("  void sai_fib_dump_dep_encap_nh_list_for_nhg (sai_object_id_t nhg_id");
    SAI_DEBUG ("  void sai_fib_dump_dep_route_list_for_encap_nh (sai_object_id_t nh_id");
    SAI_DEBUG ("  void sai_fib_dump_dep_nhg_list_for_encap_nh (sai_object_id_t nh_id");
}

void sai_fib_dump_vr_node (sai_fib_vrf_t *p_vrf_node)
{
    char p_buf [SAI_FIB_DBG_MAX_BUFSZ];

    if (p_vrf_node == NULL) {
        SAI_DEBUG ("VR node is NULL.");
        return;
    }

    SAI_DEBUG ("*************** Dumping VR information ******************");
    SAI_DEBUG ("VR Id: 0x%"PRIx64", p_vrf_node: %p, V4 admin state: %s, "
               "V6 admin state: %s, IP Opt action: %d (%s), MAC: %s, "
               "Number of RIFs: %d, IP NH tree: %p, Route tree: %p.",
               p_vrf_node->vrf_id, p_vrf_node,
               (p_vrf_node->v4_admin_state)? "ON" : "OFF",
               (p_vrf_node->v6_admin_state)? "ON" : "OFF",
               p_vrf_node->ip_options_pkt_action,
               sai_packet_action_str (p_vrf_node->ip_options_pkt_action),
               std_mac_to_string ((const hal_mac_addr_t *)&p_vrf_node->src_mac,
               p_buf, SAI_FIB_DBG_MAX_BUFSZ), p_vrf_node->num_rif,
               p_vrf_node->sai_nh_tree, p_vrf_node->sai_route_tree);
}

void sai_fib_dump_vr (sai_object_id_t vr_id)
{
    sai_fib_vrf_t *p_vrf_node = NULL;

    p_vrf_node = sai_fib_vrf_node_get (vr_id);

    if (p_vrf_node == NULL) {
        SAI_DEBUG ("VR node does not exist with VR ID 0x%"PRIx64".",
                   vr_id);
        return;
    }

    sai_fib_dump_vr_node (p_vrf_node);
}

void sai_fib_dump_all_vr (void)
{
    rbtree_handle  vr_tree;
    sai_fib_vrf_t *p_vrf_node = NULL;

    SAI_DEBUG ("Current Virtual Routers count: %d.",
               sai_fib_num_virtual_routers_get ());
    SAI_DEBUG ("Max Virutal Routers count: %d.",
               sai_fib_max_virtual_routers_get ());

    vr_tree = sai_fib_access_global_config()->vrf_tree;

    p_vrf_node = std_rbtree_getfirst (vr_tree);

    while (p_vrf_node != NULL) {
        sai_fib_dump_vr_node (p_vrf_node);

        p_vrf_node = std_rbtree_getnext (vr_tree, p_vrf_node);
    }
}

void sai_fib_dump_rif_node (sai_fib_router_interface_t *p_rif_node)
{
    char p_buf [SAI_FIB_DBG_MAX_BUFSZ];

    if (p_rif_node == NULL) {
        SAI_DEBUG ("RIF node is NULL.");
        return;
    }

    SAI_DEBUG ("************* Dumping RIF information **************");
    SAI_DEBUG ("RIF Id: 0x%"PRIx64", p_rif_node: %p, Type: %s, "
               "Attachment Id: 0x%"PRIx64", VR Id: 0x%"PRIx64", "
               "V4 admin state: %s, V6 admin state: %s, MTU: %d, "
               "MAC: %s, IP Opt action: %d (%s), ref_count: %d, "
               "Virtual:%s.",
               p_rif_node->rif_id, p_rif_node,
               sai_fib_rif_type_to_str (p_rif_node->type),
               p_rif_node->attachment.port_id, p_rif_node->vrf_id,
               (p_rif_node->v4_admin_state)? "ON" : "OFF",
               (p_rif_node->v6_admin_state)? "ON" : "OFF",
               p_rif_node->mtu, std_mac_to_string
               ((const hal_mac_addr_t *)&p_rif_node->src_mac, p_buf,
                SAI_FIB_DBG_MAX_BUFSZ), p_rif_node->ip_options_pkt_action,
               sai_packet_action_str (p_rif_node->ip_options_pkt_action),
               p_rif_node->ref_count, (p_rif_node->is_virtual?"true":"false"));
}

void sai_fib_dump_all_rif_in_vr (sai_object_id_t vr_id)
{
    sai_fib_vrf_t *p_vrf_node = NULL;
    sai_fib_router_interface_t *p_rif_node = NULL;
    unsigned int count = 0;

    p_vrf_node = sai_fib_vrf_node_get (vr_id);

    if (p_vrf_node == NULL) {
        SAI_DEBUG ("VR node does not exist with VR ID 0x%"PRIx64".",
                   vr_id);

        return;
    }

    SAI_DEBUG ("****** Dumping RIF nodes on VR Id: 0x%"PRIx64" ******",
               vr_id);
    for (p_rif_node = sai_fib_get_first_rif_from_vrf (p_vrf_node);
         p_rif_node != NULL;
         p_rif_node = sai_fib_get_next_rif_from_vrf (p_vrf_node, p_rif_node)) {

        SAI_DEBUG (" RIF Node %d.", ++count);

        sai_fib_dump_rif_node (p_rif_node);
    }
}

void sai_fib_dump_rif (sai_object_id_t rif_id)
{
    sai_fib_router_interface_t *p_rif_node = NULL;

    p_rif_node = sai_fib_router_interface_node_get (rif_id);

    if (p_rif_node == NULL) {
        SAI_DEBUG ("RIF node does not exist with RIF ID 0x%"PRIx64".",
                   rif_id);
        return;
    }

    sai_fib_dump_rif_node (p_rif_node);
}

void sai_fib_dump_all_rif (void)
{
    rbtree_handle  rif_tree;
    sai_fib_router_interface_t *p_rif_node = NULL;
    unsigned int count = 0;

    rif_tree = sai_fib_access_global_config()->router_interface_tree;

    p_rif_node = std_rbtree_getfirst (rif_tree);

    SAI_DEBUG ("******* Dumping all RIF nodes *******");
    while (p_rif_node != NULL) {
        SAI_DEBUG (" RIF Node %d.", ++count);
        sai_fib_dump_rif_node (p_rif_node);

        p_rif_node = std_rbtree_getnext (rif_tree, p_rif_node);
    }
}

void sai_fib_dump_route_node (sai_fib_route_t *p_route)
{
    char addr_str [SAI_FIB_DBG_MAX_BUFSZ];

    if (p_route == NULL) {
        SAI_DEBUG ("Route node is NULL.");
        return;
    }

    SAI_DEBUG ("************ Dumping Route information *************");
    SAI_DEBUG ("%p, IP Prefix: %s/%d, VRF: 0x%"PRIx64", NH obj Type: %s, "
               "NH Obj Id: 0x%"PRIx64", Packet-action: %s, Trap Prio: %d.",
               p_route, sai_ip_addr_to_str (&p_route->key.prefix, addr_str,
               SAI_FIB_DBG_MAX_BUFSZ), p_route->prefix_len, p_route->vrf_id,
               sai_fib_route_nh_type_to_str (p_route->nh_type),
               sai_fib_route_node_nh_id_get (p_route),
               sai_packet_action_str (p_route->packet_action),
               p_route->trap_priority);
}

void sai_fib_dump_route_entry (sai_object_id_t vrf, int af_family, char *ip_str,
                               uint_t prefix_len)
{
    sai_fib_route_key_t  key;
    sai_fib_vrf_t       *p_vrf_node = NULL;
    sai_fib_route_t     *p_route = NULL;
    uint_t               key_len = 0;

    memset (&key, 0, sizeof (sai_fib_route_key_t));

    p_vrf_node = sai_fib_vrf_node_get (vrf);

    if (p_vrf_node == NULL) {
        SAI_DEBUG ("VR node does not exist with VRF ID 0x%"PRIx64".",
                   vrf);
        return;
    }

    if (af_family == AF_INET) {
        key.prefix.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

        inet_pton (AF_INET, (const char *)ip_str,
                   (void *)&key.prefix.addr.ip4);
    } else if (af_family == AF_INET6) {
        key.prefix.addr_family = SAI_IP_ADDR_FAMILY_IPV6;

        inet_pton (AF_INET6, (const char *)ip_str,
                   (void *)&key.prefix.addr.ip6);
    } else {
        SAI_DEBUG ("af_family must be AF_INET or AF_INET6. af_family %d is not valid.",
                   af_family);

        return;
    }

    key_len = sai_fib_addr_family_bitlen() + prefix_len;

    p_route =
        (sai_fib_route_t *) std_radix_getexact (p_vrf_node->sai_route_tree,
                                                (uint8_t *)&key, key_len);

    if (p_route == NULL) {
        SAI_DEBUG ("Route node does not exist for VRF ID 0x%"PRIx64", "
                   "IP Route: %s, prefix len: %d", vrf, ip_str, prefix_len);

        return;
    }

    sai_fib_dump_route_node (p_route);
}

void sai_fib_dump_all_route_in_vr (sai_object_id_t vrf)
{
    sai_fib_route_key_t  route_node_key;
    sai_fib_vrf_t       *p_vrf_node = NULL;
    sai_fib_route_t     *p_route = NULL;
    uint_t               key_len = 0;
    unsigned int         count = 0;

    p_vrf_node = sai_fib_vrf_node_get (vrf);

    if (p_vrf_node == NULL) {
        SAI_DEBUG ("VR node does not exist with VRF ID 0x%"PRIx64".",
                   vrf);
        return;
    }

    memset (&route_node_key, 0, sizeof (sai_fib_route_key_t));

    key_len  =  sai_fib_addr_family_bitlen();

    p_route =
        (sai_fib_route_t *) std_radix_getexact (p_vrf_node->sai_route_tree,
                                                (uint8_t *)&route_node_key,
                                                 key_len);

    if (p_route == NULL) {
        p_route =
            (sai_fib_route_t *) std_radix_getnext (p_vrf_node->sai_route_tree,
                                                   (uint8_t *)&route_node_key,
                                                   key_len);
    }

    SAI_DEBUG ("******* Dumping all Route nodes *******");
    while (p_route != NULL) {
        SAI_DEBUG (" Route Node %d.", ++count);
        sai_fib_dump_route_node (p_route);

        memcpy (&route_node_key, &p_route->key, sizeof (sai_fib_route_key_t));

        key_len = sai_fib_addr_family_bitlen() + p_route->prefix_len;

        p_route =
            (sai_fib_route_t *) std_radix_getnext (p_vrf_node->sai_route_tree,
                                                   (uint8_t *)&route_node_key,
                                                   key_len);
    }
}

void sai_fib_dump_ip_nh_node (sai_fib_nh_t *p_next_hop)
{
    char   mac_addr_str [SAI_FIB_DBG_MAX_BUFSZ];
    char   ip_addr_str [SAI_FIB_DBG_MAX_BUFSZ];

    if (p_next_hop == NULL) {
        SAI_DEBUG ("Next Hop node is NULL.");
        return;
    }

    SAI_DEBUG ("************ Dumping IP Next Hop information *************");
    SAI_DEBUG ("p_nh_node: %p, NH Type: %s, IP Addr: %s, RIF: 0x%"PRIx64", "
               "Next Hop Id: 0x%"PRIx64", MAC: %s, Port: 0x%"PRIx64", "
               "Packet action: %s, Port unresolved: %d Owner flag: 0x%x, VRF: 0x%"PRIx64", "
               "ref_count: %d.", p_next_hop,
               sai_fib_next_hop_type_str (p_next_hop->key.nh_type),
               sai_ip_addr_to_str (sai_fib_next_hop_ip_addr (p_next_hop),
               ip_addr_str, SAI_FIB_DBG_MAX_BUFSZ), p_next_hop->key.rif_id,
               p_next_hop->next_hop_id, std_mac_to_string ((const hal_mac_addr_t *)
               &p_next_hop->mac_addr, mac_addr_str, SAI_FIB_DBG_MAX_BUFSZ),
               p_next_hop->port_id, sai_packet_action_str (p_next_hop->packet_action),
               p_next_hop->port_unresolved, p_next_hop->owner_flag,p_next_hop->vrf_id,
               p_next_hop->ref_count);
}

void sai_fib_dump_tunnel_encap_nh_node (sai_fib_nh_t *p_encap_nh)
{
    char   ip_addr_str [SAI_FIB_DBG_MAX_BUFSZ];

    if (p_encap_nh== NULL) {
        SAI_DEBUG ("Next Hop node is NULL.");
        return;
    }

    SAI_DEBUG ("************ Dumping Tunnel Encap Next Hop *************");
    SAI_DEBUG ("p_nh_node: %p, NH Type: %s, IP Addr: %s, RIF: 0x%"PRIx64", "
               "VR Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64", lpm_route: %p,"
               " neighbor: %p, Ref Count: %d.", p_encap_nh,
               sai_fib_next_hop_type_str (p_encap_nh->key.nh_type),
               sai_ip_addr_to_str (sai_fib_next_hop_ip_addr (p_encap_nh),
               ip_addr_str, SAI_FIB_MAX_BUFSZ), p_encap_nh->key.rif_id,
               p_encap_nh->vrf_id, p_encap_nh->next_hop_id,
               p_encap_nh->lpm_route, p_encap_nh->neighbor,
               p_encap_nh->ref_count);
}

void sai_fib_dump_nh (sai_object_id_t nh_id)
{
    sai_fib_nh_t *p_nh_node = NULL;

    p_nh_node = sai_fib_next_hop_node_get_from_id (nh_id);

    if (p_nh_node == NULL) {
        SAI_DEBUG ("Next Hop node does not exist with ID 0x%"PRIx64".",
                   nh_id);
        return;
    }

    if (p_nh_node->key.nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP) {
        sai_fib_dump_tunnel_encap_nh_node (p_nh_node);
    } else {
        sai_fib_dump_ip_nh_node (p_nh_node);
    }
}

void sai_fib_dump_all_nh (void)
{
    rbtree_handle  nh_id_tree;
    sai_fib_nh_t *p_nh_node = NULL;
    unsigned int count = 0;

    nh_id_tree = sai_fib_access_global_config()->nh_id_tree;

    p_nh_node = std_rbtree_getfirst (nh_id_tree);

    SAI_DEBUG ("******* Dumping all NH nodes *******");
    while (p_nh_node != NULL) {
        SAI_DEBUG (" NH Node %d.", ++count);
        if (p_nh_node->key.nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP) {
            sai_fib_dump_tunnel_encap_nh_node (p_nh_node);
        } else {
            sai_fib_dump_ip_nh_node (p_nh_node);
        }

        p_nh_node = std_rbtree_getnext (nh_id_tree, p_nh_node);
    }
}

void sai_fib_dump_neighbor_entry (int af_family, const char *ip_str,
                                  sai_object_id_t rif_id)
{
    sai_fib_router_interface_t *p_rif_node = NULL;
    sai_fib_nh_t               *p_nh_node = NULL;
    sai_ip_address_t            ip_addr;

    memset (&ip_addr, 0, sizeof (sai_ip_address_t));

    p_rif_node = sai_fib_router_interface_node_get (rif_id);

    if (p_rif_node == NULL) {
        SAI_DEBUG ("RIF node does not exist with RIF ID 0x%"PRIx64".",
                   rif_id);
        return;
    }

    if (af_family == AF_INET) {
        ip_addr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

        inet_pton (AF_INET, (const char *)ip_str, (void *)&ip_addr.addr.ip4);
    } else if (af_family == AF_INET6) {
        ip_addr.addr_family = SAI_IP_ADDR_FAMILY_IPV6;

        inet_pton (AF_INET6, (const char *)ip_str, (void *)ip_addr.addr.ip6);
    } else {
        SAI_DEBUG ("af_family must be AF_INET or AF_INET6. af_family %d is not "
                   "valid.", af_family);

        return;
    }

    p_nh_node = sai_fib_ip_next_hop_node_get (SAI_NEXT_HOP_TYPE_IP, rif_id, &ip_addr,
                                              SAI_FIB_TUNNEL_TYPE_NONE);

    if (p_nh_node == NULL) {
        SAI_DEBUG ("Next Hop node does not exist for RIF ID 0x%"PRIx64", "
                   "IP Addr: %s, AF: %d", rif_id, ip_str, af_family);

        return;
    }

    sai_fib_dump_ip_nh_node (p_nh_node);
}

void sai_fib_dump_all_neighbor_in_vr (sai_object_id_t vrf)
{

    sai_fib_nh_key_t  nh_key;
    sai_fib_vrf_t    *p_vrf_node = NULL;
    sai_fib_nh_t     *p_nh_node = NULL;
    unsigned int      count = 0;

    p_vrf_node = sai_fib_vrf_node_get (vrf);

    if (p_vrf_node == NULL) {
        SAI_DEBUG ("VR node does not exist with VRF ID 0x%"PRIx64".",
                   vrf);
        return;
    }

    memset (&nh_key, 0, sizeof (sai_fib_nh_key_t));

    p_nh_node = (sai_fib_nh_t *)
                       std_radix_getexact (p_vrf_node->sai_nh_tree,
                                           (uint8_t *) &nh_key,
                                            SAI_FIB_NH_IP_ADDR_TREE_KEY_LEN);
    if (p_nh_node == NULL) {

        p_nh_node = (sai_fib_nh_t *)
            std_radix_getnext (p_vrf_node->sai_nh_tree,
                    (uint8_t *) &nh_key,
                    SAI_FIB_NH_IP_ADDR_TREE_KEY_LEN);
    }

    SAI_DEBUG ("******* Dumping all Neighbor nodes *******");

    while (p_nh_node != NULL) {
        if (sai_fib_is_owner_neighbor (p_nh_node)) {
            SAI_DEBUG (" Neighbor Node %d.", ++count);
            sai_fib_dump_ip_nh_node (p_nh_node);
        }

        memcpy (&nh_key, &p_nh_node->key, sizeof (sai_fib_nh_key_t));

        p_nh_node = (sai_fib_nh_t *)
            std_radix_getnext (p_vrf_node->sai_nh_tree,
                    (uint8_t *) &nh_key,
                    SAI_FIB_NH_IP_ADDR_TREE_KEY_LEN);
    }
}

void sai_fib_dump_nh_group_node (sai_fib_nh_group_t *p_group)
{
    if (p_group == NULL) {
        SAI_DEBUG ("Next Hop Group node is NULL.");
        return;
    }

    SAI_DEBUG ("****** Dumping Next Hop Group information ******");
    SAI_DEBUG ("NH Group Id: 0x%"PRIx64", p_nh_group: %p, "
               "NH Group Type: %s, NH Count: %d, ref_count: %d.",
               p_group->key.group_id, p_group,
               sai_fib_nh_group_type_str (p_group->type),
               p_group->nh_count, p_group->ref_count);
}

void sai_fib_dump_nh_group (sai_object_id_t group_id)
{
    sai_fib_nh_group_t *p_grp_node = NULL;

    p_grp_node = sai_fib_next_hop_group_get (group_id);

    if (p_grp_node == NULL) {
        SAI_DEBUG ("Next Hop Group node does not exist with ID "
                   "0x%"PRIx64".", group_id);

        return;
    }

    sai_fib_dump_nh_group_node (p_grp_node);
}

void sai_fib_dump_all_nh_group (void)
{
    rbtree_handle  grp_id_tree;
    sai_fib_nh_group_t *p_grp_node = NULL;
    unsigned int count = 0;

    grp_id_tree = sai_fib_access_global_config()->nh_group_tree;

    p_grp_node = std_rbtree_getfirst (grp_id_tree);

    SAI_DEBUG ("******* Dumping all NH Group nodes *******");
    while (p_grp_node != NULL) {
        SAI_DEBUG (" NH Group Node %d.", ++count);
        sai_fib_dump_nh_group_node (p_grp_node);

        p_grp_node = std_rbtree_getnext (grp_id_tree, p_grp_node);
    }
}

void sai_fib_dump_nh_list_from_nh_group (sai_object_id_t group_id)
{
    sai_fib_nh_group_t     *p_nh_group = NULL;
    sai_fib_wt_link_node_t *p_nh_link_node = NULL;
    sai_fib_nh_t           *p_nh_node = NULL;
    unsigned int            count = 0;

    p_nh_group = sai_fib_next_hop_group_get (group_id);

    if (p_nh_group == NULL) {
        SAI_DEBUG ("Next Hop Group node does not exist with ID "
                   "0x%"PRIx64".", group_id);

        return;
    }

    SAI_DEBUG ("****** Dumping NH nodes on NH Group Id: 0x%"PRIx64"******",
               group_id);

    for (p_nh_link_node = sai_fib_get_first_nh_from_nh_group (p_nh_group);
         p_nh_link_node != NULL;
         p_nh_link_node = sai_fib_get_next_nh_from_nh_group (p_nh_group, p_nh_link_node))
    {
        SAI_DEBUG (" Next Hop Node %d, Weight in Group: %d.",
                   ++count, p_nh_link_node->weight);

        p_nh_node = sai_fib_get_nh_from_dll_link_node (&p_nh_link_node->link_node);

        if (p_nh_node->key.nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP) {
            sai_fib_dump_tunnel_encap_nh_node (p_nh_node);
        } else {
            sai_fib_dump_ip_nh_node (p_nh_node);
        }
    }
}

void sai_fib_dump_neighbor_mac_entry_tree (void)
{
    sai_fib_nh_t   *p_nh_node = NULL;
    char            mac_addr_str [SAI_FIB_DBG_MAX_BUFSZ];
    unsigned int    count;
    sai_fib_neighbor_mac_entry_key_t key;
    sai_fib_neighbor_mac_entry_t     *p_mac_entry = NULL;

    memset (&key, 0, sizeof (sai_fib_neighbor_mac_entry_key_t));

    p_mac_entry = (sai_fib_neighbor_mac_entry_t *) std_radix_getexact (
                             sai_fib_access_global_config()->neighbor_mac_tree,
                             (uint8_t *) &key,
                             SAI_FIB_NEIGHBOR_MAC_ENTRY_TREE_KEY_LEN);

    if (p_mac_entry == NULL) {

        p_mac_entry = (sai_fib_neighbor_mac_entry_t *) std_radix_getnext (
                             sai_fib_access_global_config()->neighbor_mac_tree,
                             (uint8_t *) &key,
                             SAI_FIB_NEIGHBOR_MAC_ENTRY_TREE_KEY_LEN);
    }

    while (p_mac_entry != NULL) {
        count = 0;
        SAI_DEBUG ("**** Dumping Neighbor list for MAC entry (%d,%s) ****",
                   p_mac_entry->key.vlan_id, std_mac_to_string ((const hal_mac_addr_t *)
                   &p_mac_entry->key.mac_addr, mac_addr_str, SAI_FIB_DBG_MAX_BUFSZ));

        for (p_nh_node = sai_fib_get_first_neighbor_from_mac_entry (p_mac_entry);
             p_nh_node != NULL;
             p_nh_node = sai_fib_get_next_neighbor_from_mac_entry (p_mac_entry, p_nh_node))
        {
            SAI_DEBUG (" Neighbor Node %d.", ++count);
            sai_fib_dump_ip_nh_node (p_nh_node);
        }

        memcpy (&key, &p_mac_entry->key, sizeof (sai_fib_neighbor_mac_entry_key_t));

        p_mac_entry = (sai_fib_neighbor_mac_entry_t *) std_radix_getnext (
                             sai_fib_access_global_config()->neighbor_mac_tree,
                             (uint8_t *) &key,
                             SAI_FIB_NEIGHBOR_MAC_ENTRY_TREE_KEY_LEN);
    }
}

void sai_fib_dump_dep_encap_nh_list_for_route (sai_object_id_t vrf, int af,
                                               char *ip_str, uint_t prefix_len)
{
    sai_fib_route_key_t  key;
    sai_fib_vrf_t       *p_vrf_node = NULL;
    sai_fib_route_t     *p_route = NULL;
    sai_fib_nh_t        *p_nh_node = NULL;
    uint_t               key_len = 0;
    unsigned int         count = 0;
    std_dll             *p_temp;

    memset (&key, 0, sizeof (sai_fib_route_key_t));

    p_vrf_node = sai_fib_vrf_node_get (vrf);

    if (p_vrf_node == NULL) {
        SAI_DEBUG ("VR node does not exist with VRF ID 0x%"PRIx64".",
                   vrf);
        return;
    }

    if (af == AF_INET) {
        key.prefix.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

        inet_pton (AF_INET, (const char *)ip_str,
                   (void *)&key.prefix.addr.ip4);
    } else if (af == AF_INET6) {
        key.prefix.addr_family = SAI_IP_ADDR_FAMILY_IPV6;

        inet_pton (AF_INET6, (const char *)ip_str,
                   (void *)&key.prefix.addr.ip6);
    } else {
        SAI_DEBUG ("af must be AF_INET or AF_INET6. Invalid %d value passed.", af);
        return;
    }

    key_len = sai_fib_addr_family_bitlen() + prefix_len;

    p_route =
        (sai_fib_route_t *) std_radix_getexact (p_vrf_node->sai_route_tree,
                                                (uint8_t *)&key, key_len);

    if (p_route == NULL) {
        SAI_DEBUG ("Route node does not exist for VRF ID 0x%"PRIx64", "
                   "IP Route: %s, prefix len: %d", vrf, ip_str, prefix_len);

        return;
    }

    sai_fib_dump_route_node (p_route);

    for (p_temp = sai_fib_dll_get_first (&p_route->dep_encap_nh_list); p_temp;
         p_temp = sai_fib_dll_get_next (&p_route->dep_encap_nh_list, p_temp)) {

         SAI_DEBUG (" Encap Next Hop Node %d.", ++count);
         p_nh_node = (sai_fib_nh_t *) ((uint8_t *) p_temp -
                      SAI_FIB_ENCAP_NH_UNDERLAY_ROUTE_DLL_GLUE_OFFSET);

         sai_fib_dump_tunnel_encap_nh_node (p_nh_node);
    }
}

void sai_fib_dump_dep_encap_nh_list_for_nexthop (int af, const char *ip_str,
                                                 sai_object_id_t rif_id)
{
    sai_fib_link_node_t        *p_link_node;
    sai_fib_router_interface_t *p_rif_node = NULL;
    sai_fib_nh_t               *p_nh_node = NULL;
    sai_fib_nh_t               *p_encap_nh_node = NULL;
    sai_ip_address_t            ip_addr;
    unsigned int                count = 0;
    std_dll                    *p_temp;

    memset (&ip_addr, 0, sizeof (sai_ip_address_t));

    p_rif_node = sai_fib_router_interface_node_get (rif_id);

    if (p_rif_node == NULL) {
        SAI_DEBUG ("RIF node does not exist with RIF ID 0x%"PRIx64".",
                   rif_id);
        return;
    }

    if (af == AF_INET) {
        ip_addr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

        inet_pton (AF_INET, (const char *)ip_str, (void *)&ip_addr.addr.ip4);
    } else if (af == AF_INET6) {
        ip_addr.addr_family = SAI_IP_ADDR_FAMILY_IPV6;

        inet_pton (AF_INET6, (const char *)ip_str, (void *)ip_addr.addr.ip6);
    } else {
        SAI_DEBUG ("af must be AF_INET or AF_INET6. Invalid %d value passed.", af);
        return;
    }

    p_nh_node = sai_fib_ip_next_hop_node_get (SAI_NEXT_HOP_TYPE_IP, rif_id, &ip_addr,
                                              SAI_FIB_TUNNEL_TYPE_NONE);

    if (p_nh_node == NULL) {
        SAI_DEBUG ("Next Hop node does not exist for RIF ID 0x%"PRIx64", "
                   "IP Addr: %s, AF: %d", rif_id, ip_str, af);

        return;
    }

    for (p_temp = sai_fib_dll_get_first (&p_nh_node->dep_encap_nh_list);
         p_temp != NULL;
         p_temp = sai_fib_dll_get_next (&p_nh_node->dep_encap_nh_list, p_temp))
    {
         SAI_DEBUG (" Encap Next Hop Node %d.", ++count);
         p_link_node = (sai_fib_link_node_t *) ((uint8_t *) p_temp -
                                 SAI_FIB_LINK_NODE_DLL_GLUE_OFFSET);
         p_encap_nh_node = sai_fib_get_nh_from_dll_link_node (p_link_node);
         sai_fib_dump_tunnel_encap_nh_node (p_encap_nh_node);
    }
}

void sai_fib_dump_dep_encap_nh_list_for_nhg (sai_object_id_t nhg_id)
{
    sai_fib_nh_group_t   *p_grp_node = NULL;
    sai_fib_nh_t         *p_nh_node = NULL;
    unsigned int          count = 0;
    std_dll              *p_temp;

    p_grp_node = sai_fib_next_hop_group_get (nhg_id);

    if (p_grp_node == NULL) {
        SAI_DEBUG ("Next Hop Group node does not exist with ID "
                   "0x%"PRIx64".", nhg_id);

        return;
    }

    for (p_temp = sai_fib_dll_get_first (&p_grp_node->dep_encap_nh_list); p_temp;
         p_temp = sai_fib_dll_get_next (&p_grp_node->dep_encap_nh_list, p_temp))
    {
         SAI_DEBUG (" Encap Next Hop Node %d.", ++count);
         p_nh_node = (sai_fib_nh_t *) ((uint8_t *) p_temp -
                      SAI_FIB_ENCAP_NH_UNDERLAY_NHG_DLL_GLUE_OFFSET);

         sai_fib_dump_tunnel_encap_nh_node (p_nh_node);
    }
}

void sai_fib_dump_dep_route_list_for_encap_nh (sai_object_id_t nh_id)
{
    sai_fib_nh_t    *p_nh_node = NULL;
    sai_fib_route_t *p_route = NULL;
    std_dll         *p_temp;
    unsigned int     count = 0;

    p_nh_node = sai_fib_next_hop_node_get_from_id (nh_id);

    if (p_nh_node == NULL) {
        SAI_DEBUG ("Next Hop node does not exist with ID 0x%"PRIx64".",
                   nh_id);
        return;
    }

    for (p_temp = sai_fib_dll_get_first (&p_nh_node->dep_route_list); p_temp;
         p_temp = sai_fib_dll_get_next (&p_nh_node->dep_route_list, p_temp))
    {
        SAI_DEBUG (" Route Node %d.", ++count);
        p_route = (sai_fib_route_t *) ((uint8_t *) p_temp -
                                       SAI_FIB_ROUTE_NH_DEP_DLL_GLUE_OFFSET);
        sai_fib_dump_route_node (p_route);
    }
}

void sai_fib_dump_dep_nhg_list_for_encap_nh (sai_object_id_t nh_id)
{
    sai_fib_wt_link_node_t *p_wt_link_node;
    sai_fib_nh_t *p_nh_node = NULL;
    sai_fib_nh_group_t *p_nh_group = NULL;
    unsigned int count = 0;

    p_nh_node = sai_fib_next_hop_node_get_from_id (nh_id);

    if (p_nh_node == NULL) {
        SAI_DEBUG ("Next Hop node does not exist with ID 0x%"PRIx64".",
                   nh_id);
        return;
    }

    for (p_wt_link_node = sai_fib_get_first_nh_group_from_nh (p_nh_node);
         p_wt_link_node; p_wt_link_node = sai_fib_get_next_nh_group_from_nh (
         p_nh_node, p_wt_link_node))
    {
        p_nh_group =
        sai_fib_get_nh_group_from_dll_link_node (&p_wt_link_node->link_node);

        SAI_DEBUG (" NH Group Node %d.", ++count);
        sai_fib_dump_nh_group_node (p_nh_group);
    }
}
