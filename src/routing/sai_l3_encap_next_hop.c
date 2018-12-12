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
* @file sai_l3_encap_next_hop.c
*
* @brief This file contains function definitions for the SAI tunnel encap
*        next hop type functions.
*
*************************************************************************/
#include "saistatus.h"
#include "saitypes.h"
#include "sai_l3_common.h"
#include "sai_l3_mem.h"
#include "sai_l3_util.h"
#include "sai_l3_api_utils.h"
#include "sai_common_infra.h"
#include "sai_tunnel_util.h"
#include "std_assert.h"
#include "std_thread_tools.h"
#include <inttypes.h>
#include <string.h>

static std_thread_create_param_t thread;
static int sai_fib_encap_nh_route_walker_fd [SAI_FIB_MAX_FD];

void sai_fib_encap_next_hop_log_trace (sai_fib_nh_t *p_encap_nh,
                                       const char *p_trace_str)
{
    char   ip_addr_str [SAI_FIB_MAX_BUFSZ];

    SAI_NEXTHOP_LOG_TRACE ("%s Type: %s, IP Addr: %s, RIF: 0x%"PRIx64", "
                           "VR Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64", lpm_route: %p,"
                           " neighbor: %p, Ref Count: %d.", p_trace_str,
                           sai_fib_next_hop_type_str (p_encap_nh->key.nh_type),
                           sai_ip_addr_to_str (sai_fib_next_hop_ip_addr (p_encap_nh),
                           ip_addr_str, SAI_FIB_MAX_BUFSZ), p_encap_nh->key.rif_id,
                           p_encap_nh->vrf_id, p_encap_nh->next_hop_id,
                           p_encap_nh->lpm_route, p_encap_nh->neighbor,
                           p_encap_nh->ref_count);
}

void sai_fib_encap_next_hop_log_error (sai_fib_nh_t *p_encap_nh,
                                       const char *p_error_str)
{
    char   ip_addr_str [SAI_FIB_MAX_BUFSZ];

    SAI_NEXTHOP_LOG_ERR ("%s Type: %s, IP Addr: %s, RIF: 0x%"PRIx64", "
                         "VR Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64", lpm_route: %p,"
                         " neighbor: %p, Ref Count: %d.", p_error_str,
                         sai_fib_next_hop_type_str (p_encap_nh->key.nh_type),
                         sai_ip_addr_to_str (sai_fib_next_hop_ip_addr (p_encap_nh),
                         ip_addr_str, SAI_FIB_MAX_BUFSZ), p_encap_nh->key.rif_id,
                         p_encap_nh->vrf_id, p_encap_nh->next_hop_id,
                         p_encap_nh->lpm_route, p_encap_nh->neighbor,
                         p_encap_nh->ref_count);
}

sai_status_t sai_fib_encap_next_hop_tunnel_id_set (sai_fib_nh_t *p_nh_info,
                                                   uint_t attr_index,
                                                   const sai_attribute_t *p_attr)
{
    sai_status_t   status;

    STD_ASSERT (p_attr != NULL);

    if (p_nh_info->key.nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP) {

        if (p_attr->value.oid == SAI_NULL_OBJECT_ID) {

            /* Mandatory for create */
            SAI_NEXTHOP_LOG_ERR ("Tunnel Id mandatory attribute missing.");

            return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
        }

    } else if (p_attr->value.oid != SAI_NULL_OBJECT_ID) {

         SAI_NEXTHOP_LOG_ERR ("Invalid tunnel id attribute for next hop type.");

         return (sai_fib_attr_status_code_get (SAI_STATUS_INVALID_ATTRIBUTE_0,
                                               attr_index));
    }

    status = sai_tunnel_object_id_validate (p_attr->value.oid);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_NEXTHOP_LOG_ERR ("Invalid tunnel id attribute for next hop type.");

        return (sai_fib_attr_status_code_get (SAI_STATUS_INVALID_ATTR_VALUE_0,
                                              attr_index));
    }

    p_nh_info->tunnel_id = p_attr->value.oid;

    SAI_NEXTHOP_LOG_TRACE ("SAI set Next Hop Tunnel Id: 0x%"PRIx64".",
                           p_nh_info->tunnel_id);

    return SAI_STATUS_SUCCESS;
}

static void dn_sai_tunnel_encap_nh_add_to_tunnel_list (sai_fib_nh_t *p_encap_nh)
{
    dn_sai_tunnel_t *tunnel_obj = NULL;

    tunnel_obj = dn_sai_tunnel_obj_get (p_encap_nh->tunnel_id);

    if (tunnel_obj != NULL) {

        std_dll_insertatback (&tunnel_obj->tunnel_encap_nh_list,
                              &p_encap_nh->tunnel_link);
    }
}

static void dn_sai_tunnel_encap_nh_remove_from_tunnel_list (sai_fib_nh_t *p_encap_nh)
{
    dn_sai_tunnel_t *tunnel_obj = NULL;

    tunnel_obj = dn_sai_tunnel_obj_get (p_encap_nh->tunnel_id);

    if (tunnel_obj != NULL) {

        std_dll_remove (&tunnel_obj->tunnel_encap_nh_list,
                        &p_encap_nh->tunnel_link);
    }
}

/*
 * Routines for add/delete Encap Next Hop to the underlay router object's
 * dependent encap next hop list
 */
static sai_fib_link_node_t *sai_fib_encap_next_hop_find_in_underlay_nh (
                                                  sai_fib_nh_t *p_encap_nh,
                                                  sai_fib_nh_t *p_underlay_nh)
{
    sai_fib_link_node_t  *p_link_node;

    for (p_link_node = (sai_fib_link_node_t *)
         sai_fib_dll_get_first (&p_underlay_nh->dep_encap_nh_list); p_link_node;
         p_link_node = (sai_fib_link_node_t *)
         sai_fib_dll_get_next (&p_underlay_nh->dep_encap_nh_list, (std_dll *)
         p_link_node))
    {
        if (p_link_node->self == p_encap_nh) {

            return p_link_node;
        }
    }

    return NULL;
}

static sai_status_t sai_fib_encap_next_hop_add_to_underlay_nh (
                                                   sai_fib_nh_t *p_encap_nh,
                                                   sai_fib_nh_t *p_underlay_nh)
{
    sai_fib_link_node_t *p_link_node = NULL;

    /* Check if the encap next hop node is added already */
    p_link_node = sai_fib_encap_next_hop_find_in_underlay_nh (p_encap_nh,
                                                              p_underlay_nh);
    if (p_link_node != NULL) {

        SAI_NEXTHOP_LOG_TRACE ("Encap Next Hop is added to underlay next "
                               "hop node's dependent list already.");

        return SAI_STATUS_SUCCESS;
    }

    /* Create a link node pointing to the encap next hop node */
    p_link_node = sai_fib_link_node_alloc ();

    if (p_link_node == NULL) {

        SAI_NEXTHOP_LOG_ERR ("Memory alloc for Tunnel Encap Next hop's "
                             "underlay next hop link node failed.");
        return SAI_STATUS_NO_MEMORY;
    }

    p_link_node->self = (void *) p_encap_nh;

    std_dll_insertatback (&p_underlay_nh->dep_encap_nh_list, &p_link_node->dll_glue);

    return SAI_STATUS_SUCCESS;
}

static void sai_fib_encap_next_hop_remove_from_underlay_nh (
                                                  sai_fib_nh_t *p_encap_nh,
                                                  sai_fib_nh_t *p_underlay_nh)
{
    sai_fib_link_node_t *p_link_node;

    p_link_node = sai_fib_encap_next_hop_find_in_underlay_nh (p_encap_nh,
                                                              p_underlay_nh);
    if (p_link_node != NULL) {

        std_dll_remove (&p_underlay_nh->dep_encap_nh_list, &p_link_node->dll_glue);

        sai_fib_link_node_free (p_link_node);
    }
}

static sai_status_t sai_fib_encap_next_hop_add_to_underlay_nhg (
                                                  sai_fib_nh_t *p_encap_nh,
                                                  sai_fib_nh_group_t *p_nh_group)
{
    sai_fib_wt_link_node_t  *p_link_node;
    sai_fib_nh_t            *p_nh_node;
    sai_status_t             status = SAI_STATUS_SUCCESS;

    /* Add to the underlay nhg */
    std_dll_insertatback (&p_nh_group->dep_encap_nh_list,
                          &p_encap_nh->underlay_nhg_link);

    /* Scan the NH list of the group */
    for (p_link_node = sai_fib_get_first_nh_from_nh_group (p_nh_group);
         p_link_node != NULL;
         p_link_node = sai_fib_get_next_nh_from_nh_group (p_nh_group, p_link_node))
    {
        p_nh_node = sai_fib_get_nh_from_dll_link_node (&p_link_node->link_node);

        status = sai_fib_encap_next_hop_add_to_underlay_nh (p_encap_nh, p_nh_node);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NEXTHOP_LOG_ERR ("Failure in adding encap next hop to "
                                 "underlay router next hop");
        }
    }

    return status;
}

static void sai_fib_encap_next_hop_remove_from_underlay_nhg (
                                                     sai_fib_nh_t *p_encap_nh,
                                                     sai_fib_nh_group_t *p_nhg)
{
    sai_fib_wt_link_node_t  *p_link_node;
    sai_fib_nh_t            *p_nh_node;

    /* Remove from the underlay nhg */
    std_dll_remove (&p_nhg->dep_encap_nh_list, &p_encap_nh->underlay_nhg_link);

    /* Scan the NH list of the group */
    for (p_link_node = sai_fib_get_first_nh_from_nh_group (p_nhg);
         p_link_node != NULL;
         p_link_node = sai_fib_get_next_nh_from_nh_group (p_nhg, p_link_node))
    {
        p_nh_node = sai_fib_get_nh_from_dll_link_node (&p_link_node->link_node);

        sai_fib_encap_next_hop_remove_from_underlay_nh (p_encap_nh, p_nh_node);
    }
}

static sai_status_t sai_fib_encap_next_hop_add_to_underlay_route_nh (
                                                    sai_fib_nh_t *p_encap_nh,
                                                    sai_fib_route_t *p_route)
{
    if ((p_route->packet_action != SAI_PACKET_ACTION_FORWARD) &&
        (p_route->packet_action != SAI_PACKET_ACTION_LOG)) {

        return SAI_STATUS_SUCCESS;
    }

    if (p_route->nh_type == SAI_OBJECT_TYPE_NEXT_HOP_GROUP) {

        return (sai_fib_encap_next_hop_add_to_underlay_nhg (p_encap_nh,
                                        p_route->nh_info.group_node));

    } else if (p_route->nh_type == SAI_OBJECT_TYPE_NEXT_HOP) {

        return (sai_fib_encap_next_hop_add_to_underlay_nh (p_encap_nh,
                                        p_route->nh_info.nh_node));
    }

    return SAI_STATUS_SUCCESS;
}

static void sai_fib_encap_next_hop_remove_from_underlay_route_nh (
                                                     sai_fib_nh_t *p_encap_nh,
                                                     sai_fib_route_t *p_route)
{
    if ((p_route->packet_action != SAI_PACKET_ACTION_FORWARD) &&
        (p_route->packet_action != SAI_PACKET_ACTION_LOG)) {

        return;
    }

    if (p_route->nh_type == SAI_OBJECT_TYPE_NEXT_HOP_GROUP) {

        sai_fib_encap_next_hop_remove_from_underlay_nhg (p_encap_nh,
                                                         p_route->nh_info.group_node);

    } else if (p_route->nh_type == SAI_OBJECT_TYPE_NEXT_HOP) {

        sai_fib_encap_next_hop_remove_from_underlay_nh (p_encap_nh,
                                                        p_route->nh_info.nh_node);
    }
}

static void sai_fib_encap_next_hop_remove_from_underlay_obj (
                                                    sai_fib_nh_t *p_encap_nh)

{
    if (p_encap_nh->neighbor != NULL) {

        sai_fib_encap_next_hop_remove_from_underlay_nh (p_encap_nh,
                                                        p_encap_nh->neighbor);
        p_encap_nh->neighbor = NULL;
    }

    if (p_encap_nh->lpm_route != NULL) {

        sai_fib_encap_next_hop_remove_from_underlay_route_nh (p_encap_nh,
                                                              p_encap_nh->lpm_route);
        std_dll_remove (&p_encap_nh->lpm_route->dep_encap_nh_list,
                        &p_encap_nh->underlay_route_link);

        p_encap_nh->lpm_route = NULL;
    }
}

/*
 * Routines for resolving encap next hop object.
 */
static void sai_fib_encap_nh_dep_routes_add_to_changelist (sai_fib_nh_t *p_encap_nh)
{
    sai_fib_route_t  *p_route;
    sai_fib_vrf_t    *p_vrf_node = NULL;

    for (p_route = sai_fib_get_first_dep_route_from_nh (p_encap_nh);
         p_route != NULL;
         p_route = sai_fib_get_next_dep_route_from_nh (p_encap_nh, p_route)) {

        p_vrf_node = sai_fib_vrf_node_get (p_route->vrf_id);

        STD_ASSERT (p_vrf_node != NULL);

        std_radical_appendtochangelist (p_vrf_node->sai_route_tree,
                                        &p_route->rt_head);
    }
}

static sai_status_t sai_fib_encap_nh_lpm_route_attr_set (
                                       sai_fib_nh_t *p_encap_nh,
                                       sai_fib_route_t *p_new_route_info)
{
    sai_fib_route_t      *p_old_route = p_encap_nh->lpm_route;
    sai_status_t          status;

    if ((p_encap_nh->packet_action == p_new_route_info->packet_action) &&
        ((p_old_route != NULL) &&
         (sai_fib_route_is_nh_info_match (p_new_route_info, p_old_route)))) {

        sai_fib_encap_next_hop_log_trace (p_encap_nh, "No change in Encap Next"
                                          " Hop Route NH attr.");

        return SAI_STATUS_SUCCESS;
    }

    p_encap_nh->packet_action = p_new_route_info->packet_action;

    status = sai_nexthop_npu_api_get()->encap_nh_route_resolve (p_encap_nh,
                                                                p_new_route_info);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_NEXTHOP_LOG_ERR ("Failure in updating Encap Next Hop in NPU.");

        return status;
    }

    if (p_old_route != NULL) {

        /* Remove dependencies with the old underlay route nh objects */
        sai_fib_encap_next_hop_remove_from_underlay_route_nh (p_encap_nh,
                                                              p_old_route);
    }

    /* Create dependencies with the underlay route nh objects */
    status = sai_fib_encap_next_hop_add_to_underlay_route_nh (p_encap_nh,
                                                              p_new_route_info);
    if (status != SAI_STATUS_SUCCESS) {

        SAI_NEXTHOP_LOG_ERR ("Failure in creating dependencies for encap next "
                             "hop with the underlay router objects.");

        return status;
    }

    /* Update the routes in Encap Next Hop's dependent route list */
    sai_fib_encap_nh_dep_routes_add_to_changelist (p_encap_nh);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_fib_encap_nh_lpm_route_resolve (sai_fib_vrf_t *p_vrf_node,
                                                 sai_fib_nh_t *p_encap_nh)
{
    sai_fib_route_t *p_old_route = p_encap_nh->lpm_route;
    sai_fib_route_t *p_new_route = NULL;
    sai_status_t     status;
    char             ip_addr_str [SAI_FIB_MAX_BUFSZ];

    /* Get the best matching route for the Encap Next hop ip address */
    uint_t prefix_len =
        sai_fib_ip_addr_family_len_get (sai_fib_next_hop_ip_addr (p_encap_nh));

    p_new_route = (sai_fib_route_t *)
                  std_radix_getbest (p_vrf_node->sai_route_tree,
                                     (uint8_t *) sai_fib_next_hop_ip_addr (p_encap_nh),
                                     sai_fib_route_key_len_get (prefix_len));

    if (p_new_route == NULL) {

        SAI_NEXTHOP_LOG_ERR ("Failure in resolving LPM route for Encap Next "
                             "Hop IP Address: %s in VRF: 0x%"PRIx64".",
                             sai_ip_addr_to_str (sai_fib_next_hop_ip_addr (p_encap_nh),
                             ip_addr_str, SAI_FIB_MAX_BUFSZ), p_vrf_node->vrf_id);

        return SAI_STATUS_FAILURE;
    }

    if (p_old_route == p_new_route) {

        sai_fib_encap_next_hop_log_trace (p_encap_nh, "No change in Encap Next "
                                          "Hop LPM route resolution.");

        return SAI_STATUS_SUCCESS;
    }

    status = sai_fib_encap_nh_lpm_route_attr_set (p_encap_nh, p_new_route);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_NEXTHOP_LOG_ERR ("Failure in Encap Next Hop LPM route attr set.");

        return status;
    }

    p_encap_nh->lpm_route = p_new_route;

    sai_fib_encap_next_hop_log_trace (p_encap_nh, "Encap Next Hop resolved with "
                                      "LPM route in underlay router");
    if (p_old_route != NULL) {

        /* Remove from the old underlay route's dependent list */
        std_dll_remove (&p_old_route->dep_encap_nh_list,
                        &p_encap_nh->underlay_route_link);
    }

    /* Add to the underlay route's dependent list */
    std_dll_insertatback (&p_new_route->dep_encap_nh_list,
                          &p_encap_nh->underlay_route_link);

    return SAI_STATUS_SUCCESS;
}

static sai_fib_nh_t *sai_fib_encap_nh_neighbor_find (sai_fib_vrf_t *p_vrf,
                                                     sai_fib_nh_t *p_encap_nh)
{
    sai_fib_nh_t      *p_neighbor;
    sai_fib_nh_key_t   key;

    memset (&key, 0, sizeof (sai_fib_nh_key_t));

    key.nh_type = SAI_NEXT_HOP_TYPE_IP;

    sai_fib_ip_addr_copy (&key.info.ip_nh.ip_addr,
                          sai_fib_next_hop_ip_addr (p_encap_nh));

    p_neighbor = (sai_fib_nh_t *)
        std_radix_getnext (p_vrf->sai_nh_tree, (uint8_t *) &key,
                           SAI_FIB_NH_IP_ADDR_TREE_KEY_LEN);

    while (p_neighbor != NULL) {

        if (memcmp (&p_neighbor->key.info.ip_nh.ip_addr,
                    sai_fib_next_hop_ip_addr (p_encap_nh),
                    sizeof (sai_ip_address_t)) != 0) {

            break;
        }

        if (sai_fib_is_owner_neighbor (p_neighbor)) {

            return p_neighbor;
        }

        memcpy (&key, &p_neighbor->key, sizeof (sai_fib_nh_key_t));

        p_neighbor = (sai_fib_nh_t *)
            std_radix_getnext (p_vrf->sai_nh_tree, (uint8_t *) &key,
                               SAI_FIB_NH_IP_ADDR_TREE_KEY_LEN);
    }

    return NULL;
}

static sai_status_t sai_fib_encap_nh_neighbor_resolve (sai_fib_nh_t *p_encap_nh,
                                                       sai_fib_nh_t *p_neighbor)
{
    sai_status_t   status;

    p_encap_nh->packet_action = p_neighbor->packet_action;

    status = sai_nexthop_npu_api_get()->encap_nh_neighbor_resolve (
                                                      p_encap_nh, p_neighbor);
    if (status != SAI_STATUS_SUCCESS) {

        SAI_NEXTHOP_LOG_ERR ("Failed to update Encap Next Hop with IP "
                             "Neighbor entry in NPU.");

        return status;
    }

    p_encap_nh->neighbor = p_neighbor;

    if (p_encap_nh->neighbor != NULL) {

        sai_fib_encap_next_hop_log_trace (p_encap_nh, "Encap Next Hop resolved with "
                                          "IP Neighbor entry in underlay router");

        /* Add Encap Next Hop to the underlay neighbor dependent list */
        sai_fib_encap_next_hop_add_to_underlay_nh (p_encap_nh,
                                                   p_encap_nh->neighbor);
    }

    /* Update the routes in Encap Next Hop's dependent route list */
    sai_fib_encap_nh_dep_routes_add_to_changelist (p_encap_nh);

    return status;
}

static sai_status_t sai_fib_encap_nh_underlay_obj_resolve (
                                                    sai_fib_nh_t *p_encap_nh)
{
    sai_object_id_t  underlay_vrf_id;
    sai_fib_vrf_t   *p_vrf_node = NULL;
    sai_fib_nh_t    *p_neighbor = NULL;
    sai_status_t     status = SAI_STATUS_SUCCESS;

    /* Get the underlay VRF to be used for resolving the next hop ip address */
    underlay_vrf_id = dn_sai_tunnel_underlay_vr_get (p_encap_nh->tunnel_id);

    p_vrf_node = sai_fib_vrf_node_get (underlay_vrf_id);

    if (p_vrf_node == NULL) {

        SAI_NEXTHOP_LOG_ERR ("Underlay VRF node not found for VRF Id: "
                             "0x%"PRIx64".", underlay_vrf_id);

        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Resolve Neighbor for the directly connected next hop case */
    p_neighbor = sai_fib_encap_nh_neighbor_find (p_vrf_node, p_encap_nh);

    if (p_neighbor != NULL) {

        status = sai_fib_encap_nh_neighbor_resolve (p_encap_nh, p_neighbor);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NEXTHOP_LOG_ERR ("Encap Next Hop Neighbor resolve failed.");
        }

        return status;
    }

    /* Resolve LPM route for the Encap Next Hop IP Address */
    status = sai_fib_encap_nh_lpm_route_resolve (p_vrf_node, p_encap_nh);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_NEXTHOP_LOG_ERR ("Encap Next Hop LPM route resolve failed.");

        return status;
    }

    return status;
}

/* Routines for Encap Next Hop dependent route resolution */
void sai_fib_encap_nh_dep_route_add (sai_fib_route_t *p_route)
{
    sai_fib_nh_t *p_encap_nh = p_route->nh_info.nh_node;

    if ((p_route->nh_type == SAI_OBJECT_TYPE_NEXT_HOP) &&
        (sai_fib_is_tunnel_encap_next_hop (p_encap_nh))) {

        std_dll_insertatback (&p_encap_nh->dep_route_list,
                              &p_route->nh_dep_route_link);
    }
}

void sai_fib_encap_nh_dep_route_remove (sai_fib_route_t *p_route)
{
    sai_fib_nh_t *p_encap_nh = p_route->nh_info.nh_node;

    if ((p_route->nh_type == SAI_OBJECT_TYPE_NEXT_HOP) &&
        (sai_fib_is_tunnel_encap_next_hop (p_encap_nh))) {

        std_dll_remove (&p_encap_nh->dep_route_list,
                        &p_route->nh_dep_route_link);
    }
}

int sai_fib_encap_nh_dep_route_walk_cb (std_radical_head_t *p_rt_head,
                                        va_list ap)
{
    sai_fib_route_t  *p_route = (sai_fib_route_t *) p_rt_head;
    char              addr_str [SAI_FIB_MAX_BUFSZ];
    sai_status_t      status;

    if (p_route == NULL) {

        SAI_ROUTE_LOG_ERR ("Tunnel Encap NextHop Dep Route pointer is NULL.");

        return SAI_STATUS_FAILURE;
    }

    SAI_ROUTE_LOG_TRACE ("Tunnel Encap Dep Route, VRF: 0x%"PRIx64", Prefix: "
                         "%s/%d, FWD object Type: %s, "
                         "Index: 0x%"PRIx64", Packet-action: %s, Trap Priority: %d",
                         p_route->vrf_id, sai_ip_addr_to_str
                         (&p_route->key.prefix, addr_str, SAI_FIB_MAX_BUFSZ),
                         p_route->prefix_len, sai_fib_route_nh_type_to_str
                         (p_route->nh_type),
                         sai_fib_route_node_nh_id_get (p_route),
                         sai_packet_action_str (p_route->packet_action),
                         p_route->trap_priority);

    status = sai_route_npu_api_get()->route_create (p_route);

    if (status != SAI_STATUS_SUCCESS) {
        SAI_ROUTE_LOG_ERR ("Failed to update Tunnel Encap NextHop Dep Route in NPU.");
    }

    return status;
}

static void sai_fib_encap_nh_signal_dep_route_walk (void)
{
    int rc = 0;
    bool wake = true;

    if ((rc = write (sai_fib_encap_nh_route_walker_fd[SAI_FIB_WRITE_FD], &wake,
                     sizeof(bool))) != sizeof(bool)) {
        SAI_NEXTHOP_LOG_ERR ("Writing to Encap Next Hop Dependent route walker "
                             "thread's pipe failed");
    }
}

static void sai_fib_encap_nh_dep_route_changelist_walk (void)
{
    rbtree_handle  vr_tree;
    sai_fib_vrf_t *p_vrf_node = NULL;
    int ret;

    sai_fib_lock ();

    /* Scan the VRFs */
    vr_tree = sai_fib_access_global_config()->vrf_tree;

    p_vrf_node = std_rbtree_getfirst (vr_tree);

    while (p_vrf_node) {

        std_radical_walkchangelist (p_vrf_node->sai_route_tree,
                                    &p_vrf_node->route_marker,
                                    sai_fib_encap_nh_dep_route_walk_cb, 0,
                                    SAI_FIB_MAX_DEP_ROUTES_WALK_COUNT,
                                    std_radix_getversion(p_vrf_node->sai_route_tree),
                                    &ret);

        p_vrf_node = std_rbtree_getnext (vr_tree, p_vrf_node);
    }

    sai_fib_unlock ();
}

/* Encap Next Hop resolution for underlay lpm route attribute set */
void sai_fib_route_attr_set_affected_encap_nh_update (
                                           sai_fib_route_t *p_route_node,
                                           sai_fib_route_t *p_new_route_info)
{
    sai_fib_nh_t     *p_encap_nh;
    sai_fib_nh_t     *p_next_encap_nh;
    sai_fib_vrf_t    *p_vrf_node = NULL;
    bool              encap_nh_resolved = false;

    p_vrf_node = sai_fib_vrf_node_get (p_route_node->vrf_id);

    if (p_vrf_node == NULL) {
        SAI_ROUTE_LOG_ERR ("VRF ID 0x%"PRIx64" does not exist in VRF tree.",
                           p_route_node->vrf_id);
        return;
    }

    /* Scan the tunnel encap next hops in the route node */
    for (p_encap_nh = sai_fib_route_get_first_dep_encap_nh (p_route_node);
         p_encap_nh != NULL; p_encap_nh = p_next_encap_nh) {

        p_next_encap_nh =
                 sai_fib_route_get_next_dep_encap_nh (p_route_node, p_encap_nh);

        sai_fib_encap_nh_lpm_route_attr_set (p_encap_nh, p_new_route_info);
        encap_nh_resolved = true;
    }

    if(encap_nh_resolved) {
        sai_fib_encap_nh_signal_dep_route_walk ();
    }
}


/* Encap Next Hop resolution for underlay lpm route changes */
static void sai_fib_route_create_affected_encap_nh_update (
                                                    sai_fib_route_t *p_new_route,
                                                    bool *encap_nh_resolved)
{
    sai_fib_route_t  *p_less_specific;
    sai_fib_nh_t     *p_encap_nh;
    sai_fib_nh_t     *p_next_encap_nh;
    sai_fib_vrf_t    *p_vrf_node = NULL;
    sai_ip_address_t  mask;

    p_vrf_node = sai_fib_vrf_node_get (p_new_route->vrf_id);

    if (p_vrf_node == NULL) {
        SAI_ROUTE_LOG_ERR ("VRF ID 0x%"PRIx64" does not exist in VRF tree.",
                           p_new_route->vrf_id);
        return;
    }

    p_less_specific = (sai_fib_route_t *)
                  std_radix_getlessspecific (p_vrf_node->sai_route_tree,
                                             (std_rt_head *)(&p_new_route->rt_head));

    memset (&mask, 0, sizeof (sai_ip_address_t));

    mask.addr_family = p_new_route->key.prefix.addr_family;

    sai_fib_ip_prefix_mask_get (&mask, p_new_route->prefix_len);

    /* Scan the less specific routes in the VRF */
    while (p_less_specific != NULL)
    {
        /* Scan the tunnel encap next hops in the less specific route */
        for (p_encap_nh = sai_fib_route_get_first_dep_encap_nh (p_less_specific);
             p_encap_nh != NULL; p_encap_nh = p_next_encap_nh) {

            p_next_encap_nh =
            sai_fib_route_get_next_dep_encap_nh (p_less_specific, p_encap_nh);

            if (sai_fib_is_ip_addr_in_prefix (&p_new_route->key.prefix, &mask,
                                              sai_fib_next_hop_ip_addr (p_encap_nh))) {

                /* Resolve the Encap Next Hop */
                sai_fib_encap_nh_lpm_route_resolve (p_vrf_node, p_encap_nh);
                *encap_nh_resolved = true;
            }
        }

        p_less_specific = (sai_fib_route_t *)
            std_radix_getlessspecific (p_vrf_node->sai_route_tree,
                                       (std_rt_head *)(&p_less_specific->rt_head));
    }
}

static void sai_fib_route_del_affected_encap_nh_update (
                                                      sai_fib_route_t *p_route,
                                                      bool *encap_nh_resolved)
{
    sai_fib_nh_t     *p_encap_nh;
    sai_fib_nh_t     *p_next_encap_nh;
    sai_fib_vrf_t    *p_vrf_node = NULL;

    p_vrf_node = sai_fib_vrf_node_get (p_route->vrf_id);

    if (p_vrf_node == NULL) {
        SAI_ROUTE_LOG_ERR ("VRF ID 0x%"PRIx64" does not exist in VRF tree.",
                           p_route->vrf_id);
        return;
    }

    /* Scan the tunnel encap next hops in the less specific route */
    for (p_encap_nh = sai_fib_route_get_first_dep_encap_nh (p_route);
         p_encap_nh != NULL; p_encap_nh = p_next_encap_nh) {

        p_next_encap_nh = sai_fib_route_get_next_dep_encap_nh (p_route, p_encap_nh);

        sai_fib_encap_nh_lpm_route_resolve (p_vrf_node, p_encap_nh);
        *encap_nh_resolved = true;
    }
}

void sai_fib_route_affected_encap_nh_update (sai_fib_route_t *p_route,
                                             dn_sai_operations_t op_code)
{
    bool encap_nh_resolved = false;
    /* Default route is handled like route attribute set case */
    if (sai_fib_is_default_route_node (p_route)) {

        sai_fib_route_attr_set_affected_encap_nh_update (p_route, p_route);

        return;
    }

    if (op_code == SAI_OP_CREATE) {

        sai_fib_route_create_affected_encap_nh_update (p_route, &encap_nh_resolved);

    } else if (op_code == SAI_OP_REMOVE) {

        sai_fib_route_del_affected_encap_nh_update (p_route, &encap_nh_resolved);
    }

    if(encap_nh_resolved) {
        sai_fib_encap_nh_signal_dep_route_walk ();
    }

    return;
}

/* Encap Next Hop resolution for underlay neighbor create/remove */
void sai_fib_neighbor_affected_encap_nh_resolve (sai_fib_nh_t *p_neighbor,
                                                 dn_sai_operations_t op_type)
{
    sai_fib_nh_t      *p_encap_nh;
    sai_fib_vrf_t     *p_vrf_node = NULL;
    sai_fib_nh_key_t   key;
    bool               encap_nh_resolved = false;

    memset (&key, 0, sizeof (sai_fib_nh_key_t));

    p_vrf_node = sai_fib_vrf_node_get (p_neighbor->vrf_id);

    if (p_vrf_node == NULL) {

        SAI_NEXTHOP_LOG_ERR ("Underlay VRF node not found for tunnel Id.");

        return;
    }

    key.nh_type = SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP;

    sai_fib_ip_addr_copy (&key.info.ip_nh.ip_addr,
                          sai_fib_next_hop_ip_addr (p_neighbor));

    p_encap_nh = (sai_fib_nh_t *)
        std_radix_getnext (p_vrf_node->sai_nh_tree, (uint8_t *) &key,
                           SAI_FIB_NH_IP_ADDR_TREE_KEY_LEN);

    while (p_encap_nh != NULL) {

        if (memcmp (&p_neighbor->key.info.ip_nh.ip_addr,
                    sai_fib_next_hop_ip_addr (p_encap_nh),
                    sizeof (sai_ip_address_t)) != 0) {

            break;
        }

        /* Remove from the old underlay route object if there is any */
        sai_fib_encap_next_hop_remove_from_underlay_obj (p_encap_nh);

        if (op_type == SAI_OP_CREATE) {

            sai_fib_encap_nh_neighbor_resolve (p_encap_nh, p_neighbor);

        } else {

            p_encap_nh->neighbor = NULL;

            /* Resolve a LPM route for the IP address */
            sai_fib_encap_nh_lpm_route_resolve (p_vrf_node, p_encap_nh);
        }

        encap_nh_resolved = true;

        memcpy (&key, &p_encap_nh->key, sizeof (sai_fib_nh_key_t));

        p_encap_nh = (sai_fib_nh_t *)
            std_radix_getnext (p_vrf_node->sai_nh_tree, (uint8_t *) &key,
                               SAI_FIB_NH_IP_ADDR_TREE_KEY_LEN);
    }

    if (encap_nh_resolved) {
        sai_fib_encap_nh_signal_dep_route_walk();
    }
}

/* Encap Next Hop resolution for underlay neighbor attribute set */
void sai_fib_neighbor_dep_encap_nh_list_update (sai_fib_nh_t *p_neighbor,
                                                sai_fib_nh_t *p_attr_info,
                                                uint_t attr_flags)
{
    sai_fib_link_node_t *p_link_node;
    sai_fib_nh_t        *p_encap_nh;
    sai_status_t         status;

    for (p_link_node = (sai_fib_link_node_t *)
         sai_fib_dll_get_first (&p_neighbor->dep_encap_nh_list); p_link_node;
         p_link_node = (sai_fib_link_node_t *)
         sai_fib_dll_get_next (&p_neighbor->dep_encap_nh_list, (std_dll *)
         p_link_node))
    {
        p_encap_nh = (sai_fib_nh_t *) p_link_node->self;

        status = sai_nexthop_npu_api_get()->encap_nh_neighbor_attr_set (
                             p_encap_nh, p_neighbor, p_attr_info, attr_flags);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NEXTHOP_LOG_ERR ("Failed to update Encap Next Hop for "
                                 "Neighbor attribute set in NPU.");
        }
    }
}

/* Encap Next Hop resolution for underlay Next Hop Group NH list changes */
void sai_fib_nh_group_dep_encap_nh_update (sai_fib_nh_group_t *p_nh_group,
                                           sai_fib_nh_t *p_underlay_nh,
                                           bool is_add)
{
    sai_fib_nh_t  *p_encap_nh;
    sai_status_t   status;

    /* Scan the tunnel encap next hops in the less specific route */
    for (p_encap_nh = sai_fib_nh_group_get_first_dep_encap_nh (p_nh_group);
         p_encap_nh != NULL;
         p_encap_nh = sai_fib_nh_group_get_next_dep_encap_nh (p_nh_group,
         p_encap_nh)) {

        if (is_add) {
            /* @TODO Add the underlay next hop in NPU */

            status = sai_fib_encap_next_hop_add_to_underlay_nh (p_encap_nh,
                                                                p_underlay_nh);
            if (status != SAI_STATUS_SUCCESS) {

                SAI_NEXTHOP_LOG_ERR ("Failed to add Encap Next Hop to the underlay "
                                     "Next Hop's dependent encap nh list.");
            }
        } else {
            /* @TODO Delete the underlay next hop in NPU */

            sai_fib_encap_next_hop_remove_from_underlay_nh (p_encap_nh,
                                                            p_underlay_nh);
        }
    }
}

/* Encap Next Hop dependent route walker */
static void *sai_fib_encap_nh_dep_route_walk_main (void *param)
{
    int len = 0;
    bool wake;

    while(1) {
        len = read (sai_fib_encap_nh_route_walker_fd[SAI_FIB_READ_FD], &wake,
                    sizeof(bool));
        if (len && wake) {
           sai_fib_encap_nh_dep_route_changelist_walk ();
        }
    }

    return NULL;
}

sai_status_t sai_fib_encap_nh_dep_route_walker_create (void)
{
    if (pipe (sai_fib_encap_nh_route_walker_fd) != 0) {
        SAI_ROUTER_LOG_CRIT ("Encap NH Dep Route walker pipe init failed");

        return SAI_STATUS_FAILURE;
    }

    std_thread_init_struct (&thread);

    thread.name = "sai_fib_encap_nh_dep_route_walker";
    thread.thread_function = sai_fib_encap_nh_dep_route_walk_main;

    if (std_thread_create (&thread) != STD_ERR_OK) {
        SAI_ROUTER_LOG_CRIT ("Encap NH Dep Router walker thread create failed");

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_fib_encap_next_hop_create (sai_fib_nh_t *p_encap_nh,
                                            sai_npu_object_id_t *hw_id)
{
    sai_status_t  status;

    /* Create the Encap Next Hop object in NPU */
    status = sai_nexthop_npu_api_get()->nexthop_create (p_encap_nh, hw_id);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_NEXTHOP_LOG_ERR ("Failure in creating Encap Next hop object in NPU.");

        return status;
    }

    /* Resolve the Encap Next Hop in underlay router */
    status = sai_fib_encap_nh_underlay_obj_resolve (p_encap_nh);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_NEXTHOP_LOG_ERR ("Failure in resolving underlay router object for "
                             "the Encap Next hop.");

        sai_nexthop_npu_api_get()->nexthop_remove (p_encap_nh);

        return status;
    }

    dn_sai_tunnel_encap_nh_add_to_tunnel_list (p_encap_nh);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_fib_encap_next_hop_remove (sai_fib_nh_t *p_encap_nh)
{
    sai_status_t  status;

    /* Remove the Encap Next Hop in NPU */
    status = sai_nexthop_npu_api_get()->nexthop_remove (p_encap_nh);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_NEXTHOP_LOG_ERR ("Failure in removing the encap next hop in NPU.");

        return status;
    }

    sai_fib_encap_next_hop_remove_from_underlay_obj (p_encap_nh);

    dn_sai_tunnel_encap_nh_remove_from_tunnel_list (p_encap_nh);

    return SAI_STATUS_SUCCESS;
}

