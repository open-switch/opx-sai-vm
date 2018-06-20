/************************************************************************
* LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file sai_l3_next_hop.c
*
* @brief This file contains function definitions for the SAI next hop
*        functionality API.
*
*************************************************************************/

#include "sai_l3_util.h"
#include "sai_l3_api.h"
#include "saistatus.h"
#include "sainexthop.h"
#include "std_mac_utils.h"
#include "std_assert.h"

#include "sai_l3_mem.h"
#include "sai_l3_api_utils.h"
#include "sai_common_infra.h"
#include <string.h>
#include <inttypes.h>

static inline void sai_fib_ip_next_hop_log_trace (sai_fib_nh_t *p_next_hop,
                                                  const char *p_trace_str)
{
    char   mac_addr_str [SAI_FIB_MAX_BUFSZ];
    char   ip_addr_str [SAI_FIB_MAX_BUFSZ];

    SAI_NEXTHOP_LOG_TRACE ("%s Type: %s, IP Addr: %s, RIF: 0x%"PRIx64", "
                  "Next Hop Id: 0x%"PRIx64", Owner flag: 0x%x, "
                  "VRF Id: 0x%"PRIx64", Mac Addr: %s, Port: 0x%"PRIx64""
                  ", Packet action: %s, Ref Count: %d.", p_trace_str,
                  sai_fib_next_hop_type_str (p_next_hop->key.nh_type),
                  sai_ip_addr_to_str (sai_fib_next_hop_ip_addr (p_next_hop),
                  ip_addr_str, SAI_FIB_MAX_BUFSZ), p_next_hop->key.rif_id,
                  p_next_hop->next_hop_id, p_next_hop->owner_flag,
                  p_next_hop->vrf_id, std_mac_to_string (
                  (const hal_mac_addr_t *)&p_next_hop->mac_addr,
                  mac_addr_str, SAI_FIB_MAX_BUFSZ), p_next_hop->port_id,
                  sai_packet_action_str (p_next_hop->packet_action),
                  p_next_hop->ref_count);
}

static inline void sai_fib_ip_next_hop_log_error (sai_fib_nh_t *p_next_hop,
                                                  const char *p_error_str)
{
    char   mac_addr_str [SAI_FIB_MAX_BUFSZ];
    char   ip_addr_str [SAI_FIB_MAX_BUFSZ];

    SAI_NEXTHOP_LOG_ERR ("%s Type: %s, IP Addr: %s, RIF: 0x%"PRIx64", "
                "Next Hop Id: 0x%"PRIx64", Owner flag: 0x%x, "
                "VRF Id: 0x%"PRIx64", Mac Addr: %s, Port: 0x%"PRIx64""
                ", Packet action: %s, Ref Count: %d.", p_error_str,
                sai_fib_next_hop_type_str (p_next_hop->key.nh_type),
                sai_ip_addr_to_str (sai_fib_next_hop_ip_addr (p_next_hop),
                ip_addr_str, SAI_FIB_MAX_BUFSZ), p_next_hop->key.rif_id,
                p_next_hop->next_hop_id, p_next_hop->owner_flag,
                p_next_hop->vrf_id, std_mac_to_string (
                (const hal_mac_addr_t *)&p_next_hop->mac_addr,
                mac_addr_str, SAI_FIB_MAX_BUFSZ), p_next_hop->port_id,
                sai_packet_action_str (p_next_hop->packet_action),
                p_next_hop->ref_count);
}

void sai_fib_next_hop_log_trace (sai_fib_nh_t *p_next_hop,
                                 const char *p_trace_str)
{
    STD_ASSERT (p_next_hop != NULL);

    if (p_next_hop->key.nh_type == SAI_NEXT_HOP_TYPE_IP) {

        sai_fib_ip_next_hop_log_trace (p_next_hop, p_trace_str);

    } else if (p_next_hop->key.nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP) {

        sai_fib_encap_next_hop_log_trace (p_next_hop, p_trace_str);
    }
}

void sai_fib_next_hop_log_error (sai_fib_nh_t *p_next_hop,
                                 const char *p_error_str)
{
    STD_ASSERT (p_next_hop != NULL);

    if (p_next_hop->key.nh_type == SAI_NEXT_HOP_TYPE_IP) {

        sai_fib_ip_next_hop_log_error (p_next_hop, p_error_str);

    } else if (p_next_hop->key.nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP) {

        sai_fib_encap_next_hop_log_error (p_next_hop, p_error_str);
    }
}

static bool sai_fib_next_hop_type_validate (sai_next_hop_type_t type)
{
    switch (type) {
        case SAI_NEXT_HOP_TYPE_IP:
            return true;

        case SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP:
            return true;

        default:
            return false;
    }
}

static inline bool sai_fib_next_hop_rif_validate (
                                             sai_object_id_t rif_id)
{
    /* Get the RIF node */
    sai_fib_router_interface_t *p_rif_node =
                       sai_fib_router_interface_node_get(rif_id);

    if((p_rif_node != NULL) &&
       (p_rif_node->type != SAI_ROUTER_INTERFACE_TYPE_LOOPBACK)) {
        return true;
    }
    return false;
}

sai_status_t sai_fib_next_hop_ip_address_validate (
                                            const sai_ip_address_t *p_ip_addr)
{
    char   ip_addr_str [SAI_FIB_MAX_BUFSZ];

    if ((p_ip_addr->addr_family != SAI_IP_ADDR_FAMILY_IPV4) &&
        (p_ip_addr->addr_family != SAI_IP_ADDR_FAMILY_IPV6)) {

        SAI_NEXTHOP_LOG_ERR ("Invalid IP addr family: %d.",
                             p_ip_addr->addr_family);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((sai_fib_is_ip_addr_zero (p_ip_addr)) ||
        (sai_fib_is_ip_addr_loopback (p_ip_addr))) {

        SAI_NEXTHOP_LOG_ERR ("Invalid Next Hop IP address: %s.",
                             sai_ip_addr_to_str (p_ip_addr, ip_addr_str,
                                                 SAI_FIB_MAX_BUFSZ));

        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

/*
 * This is a helper routine for parsing attribute list input. Caller must
 * recalculate the error code with the attribute's index in the list.
 */
sai_status_t sai_fib_next_hop_rif_set (sai_fib_nh_t *p_nh_info,
                                       const sai_attribute_value_t *p_value,
                                       bool is_create_req)
{
    sai_object_id_t   rif_id;
    sai_fib_vrf_t    *p_vrf_node = NULL;

    STD_ASSERT (p_value != NULL);

    if (!is_create_req) {

        SAI_NEXTHOP_LOG_ERR ("Next Hop RIF create-only attr is set.");

        return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    if (!sai_is_obj_id_rif (p_value->oid)) {

        SAI_NEXTHOP_LOG_ERR ("Invalid SAI object id passed.");

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    rif_id = p_value->oid;

    if (!(sai_fib_next_hop_rif_validate (rif_id))) {

        SAI_NEXTHOP_LOG_ERR ("Invalid RIF id: 0x%"PRIx64".", rif_id);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_vrf_node = sai_fib_get_vrf_node_for_rif (rif_id);

    if (p_vrf_node == NULL) {

        SAI_NEXTHOP_LOG_ERR ("VRF node does not exist for RIF Id: 0x%"PRIx64".",
                             rif_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    p_nh_info->key.rif_id  = rif_id;
    p_nh_info->vrf_id      = p_vrf_node->vrf_id;

    SAI_NEXTHOP_LOG_TRACE ("SAI set Next Hop RIF Id: 0x%"PRIx64".", rif_id);

    return SAI_STATUS_SUCCESS;
}

/*
 * This is a helper routine for parsing attribute list input. Caller must
 * recalculate the error code with the attribute's index in the list.
 */
static sai_status_t sai_fib_next_hop_type_set (sai_fib_nh_t *p_nh_info,
                                         const sai_attribute_value_t *p_value,
                                         bool is_create_req)
{
    sai_next_hop_type_t nh_type;

    STD_ASSERT (p_value != NULL);

    if (!is_create_req) {

        SAI_NEXTHOP_LOG_ERR ("Next Hop Type create-only attr is set.");

        return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    nh_type = p_value->s32;

    if (!(sai_fib_next_hop_type_validate (nh_type))) {

        SAI_NEXTHOP_LOG_ERR ("Invalid Next Hop type attribute value: %d.",
                             nh_type);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_nh_info->key.nh_type = nh_type;

    SAI_NEXTHOP_LOG_TRACE ("SAI set Next Hop type: %s.",
                           sai_fib_next_hop_type_str (nh_type));

    return SAI_STATUS_SUCCESS;
}

/*
 * This is a helper routine for parsing attribute list input. Caller must
 * recalculate the error code with the attribute's index in the list.
 */
static sai_status_t sai_fib_next_hop_ip_addr_set (sai_fib_nh_t *p_nh_info,
                                          const sai_attribute_value_t *p_value,
                                          bool is_create_req)
{
    sai_status_t       status;
    char               ip_addr_str [SAI_FIB_MAX_BUFSZ];
    sai_ip_address_t  *p_ip_addr = NULL;

    STD_ASSERT (p_value != NULL);

    if (!is_create_req) {

        SAI_NEXTHOP_LOG_ERR ("Next Hop IP address create-only attr is set.");

        return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    status = sai_fib_next_hop_ip_address_validate (&p_value->ipaddr);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_NEXTHOP_LOG_ERR ("Invalid Next Hop IP addr attribute value.");

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_ip_addr = sai_fib_next_hop_ip_addr (p_nh_info);

    status = sai_fib_ip_addr_copy (p_ip_addr, &p_value->ipaddr);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_NEXTHOP_LOG_ERR ("Failed to copy Next Hop IP addr attribute value.");

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    SAI_NEXTHOP_LOG_TRACE ("SAI set Next Hop IP Address: %s.",
                           sai_ip_addr_to_str (p_ip_addr, ip_addr_str,
                                               SAI_FIB_MAX_BUFSZ));

    return SAI_STATUS_SUCCESS;
}

static inline void sai_fib_next_hop_attr_copy (sai_fib_nh_t *p_dest_nh_info,
                                               sai_fib_nh_t *p_src_nh_info)
{
    /* @TODO Add new attributes if added in future */
}

static inline void sai_fib_next_hop_default_attr_set (sai_fib_nh_t *p_nh_node)
{
    /* Next hop node default packet action */
    p_nh_node->packet_action = SAI_PACKET_ACTION_TRAP;
}

static inline void sai_fib_next_hop_info_reset (sai_fib_nh_t *p_next_hop)
{
    sai_fib_reset_next_hop_owner (p_next_hop, SAI_FIB_OWNER_NEXT_HOP);

    p_next_hop->next_hop_id = 0;
}

static sai_status_t sai_fib_next_hop_info_fill (sai_fib_nh_t *p_nh_info,
                                           uint_t attr_count,
                                           const sai_attribute_t *attr_list,
                                           bool is_create_req)
{
    sai_status_t             status = SAI_STATUS_FAILURE;
    uint_t                   attr_index;
    const sai_attribute_t   *p_attr = NULL;
    bool                     is_type_attr_set = false;
    bool                     is_ip_attr_set = false;
    bool                     is_rif_attr_set = false;
    const sai_attribute_t   *p_tunnel_id_attr = NULL;
    uint_t                   tunnel_id_attr_index = 0;

    STD_ASSERT (p_nh_info != NULL);
    STD_ASSERT (attr_list != NULL);

    if ((!attr_count)) {

        SAI_NEXTHOP_LOG_ERR ("SAI Next Hop fill from attr list. Invalid input. "
                             "attr_count: %d.", attr_count);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (attr_index = 0; attr_index < attr_count; attr_index++)
    {
        p_attr = &attr_list [attr_index];
        status = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;

        switch (p_attr->id) {

            case SAI_NEXT_HOP_ATTR_TYPE:
                status = sai_fib_next_hop_type_set (p_nh_info, &p_attr->value,
                                                    is_create_req);
                is_type_attr_set = true;
                break;

            case SAI_NEXT_HOP_ATTR_IP:
                status = sai_fib_next_hop_ip_addr_set (p_nh_info,
                                                       &p_attr->value,
                                                       is_create_req);
                is_ip_attr_set = true;
                break;

            case SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID:
                status = sai_fib_next_hop_rif_set (p_nh_info, &p_attr->value,
                                                   is_create_req);
                is_rif_attr_set = true;
                break;

            case SAI_NEXT_HOP_ATTR_TUNNEL_ID:
                p_tunnel_id_attr     = p_attr;
                tunnel_id_attr_index = attr_index;
                status               = SAI_STATUS_SUCCESS;
                break;
            case SAI_NEXT_HOP_ATTR_SEGMENTROUTE_SIDLIST_ID:
            case SAI_NEXT_HOP_ATTR_SEGMENTROUTE_ENDPOINT_TYPE:
            case SAI_NEXT_HOP_ATTR_SEGMENTROUTE_ENDPOINT_POP_TYPE:
            case SAI_NEXT_HOP_ATTR_LABELSTACK:
                status = SAI_STATUS_NOT_SUPPORTED;
                SAI_NEXTHOP_LOG_ERR ("unsupported atttribute id: %d.", p_attr->id);
                break;
            default:
                SAI_NEXTHOP_LOG_ERR ("Invalid atttribute id: %d.", p_attr->id);
                break;
        }

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NEXTHOP_LOG_ERR ("Failure in Next Hop attr list. Index: %d, "
                                 "Attribute Id: %d, Error: %d.", attr_index,
                                 p_attr->id, status);

            return (sai_fib_attr_status_code_get (status, attr_index));
        }
    }

    if ((is_create_req) &&
        ((!is_type_attr_set) || (!is_rif_attr_set))) {

        /* Mandatory for create */
        SAI_NEXTHOP_LOG_ERR ("Mandatory Next Hop %s attribute missing.",
                             (!is_type_attr_set) ? "Type" : "RIF");

        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    if ((p_nh_info->key.nh_type == SAI_NEXT_HOP_TYPE_IP) ||
        (p_nh_info->key.nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP)) {

        if ((is_create_req) && ((!is_ip_attr_set))) {

            /* Mandatory for create */
            SAI_NEXTHOP_LOG_ERR ("Mandatory IP Next Hop attribute missing.");

            return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
        }
    }

    if (p_nh_info->key.nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP) {

        if ((is_create_req) && (p_tunnel_id_attr == NULL)) {
            SAI_NEXTHOP_LOG_ERR ("Mandatory Tunnel attribute missing.");

            return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
        }

        status = sai_fib_encap_next_hop_tunnel_id_set (p_nh_info,
                                      tunnel_id_attr_index, p_tunnel_id_attr);
    }

    return status;
}

static sai_status_t sai_fib_insert_nh_node_in_vrf_database (
                                                     sai_fib_vrf_t *p_vrf_node,
                                                     sai_fib_nh_t *p_nh_node)
{
    std_rt_head    *p_radix_head = NULL;

    STD_ASSERT (p_vrf_node != NULL);
    STD_ASSERT (p_nh_node != NULL);

    /* Insert into the Next Hop IP address based view in VRF */
    p_nh_node->rt_head.rth_addr = (uint8_t *)(&p_nh_node->key);

    p_radix_head = std_radix_insert (p_vrf_node->sai_nh_tree,
                                     &p_nh_node->rt_head,
                                     SAI_FIB_NH_IP_ADDR_TREE_KEY_LEN);
    if (!p_radix_head) {

        sai_fib_next_hop_log_error (p_nh_node, "Failed to insert IP next hop "
                                    "node in radix tree.");

        return SAI_STATUS_FAILURE;
    }

    if (p_radix_head != (std_rt_head *) p_nh_node) {

        sai_fib_next_hop_log_error (p_nh_node, "Duplicate IP next hop "
                                    "node in radix tree.");

        return SAI_STATUS_FAILURE;
    }

    sai_fib_next_hop_log_trace (p_nh_node, "Inserted IP Next hop node in VRF "
                                "database.");

    return SAI_STATUS_SUCCESS;
}

static void sai_fib_remove_nh_node_from_vrf_database (sai_fib_vrf_t *p_vrf_node,
                                                      sai_fib_nh_t *p_nh_node)
{
    STD_ASSERT (p_vrf_node != NULL);
    STD_ASSERT (p_nh_node != NULL);

    sai_fib_next_hop_log_trace (p_nh_node, "Remove IP Next hop node from "
                                "VRF database.");

    /* Remove from the Next Hop IP address view */
    std_radix_remove (p_vrf_node->sai_nh_tree, &p_nh_node->rt_head);
}

static sai_status_t sai_fib_insert_nh_node_in_nh_id_database (
                                                      sai_fib_nh_t *p_nh_node)
{
    rbtree_handle   nh_id_tree = NULL;
    t_std_error     rc;

    STD_ASSERT (p_nh_node != NULL);

    nh_id_tree = sai_fib_access_global_config()->nh_id_tree;

    /* Insert into the Next Hop Id global view */
    rc = std_rbtree_insert (nh_id_tree, p_nh_node);

    if (STD_IS_ERR (rc)) {

        sai_fib_next_hop_log_error (p_nh_node, "Error in inserting next hop id"
                                    " in RB tree.");

        return SAI_STATUS_FAILURE;
    }

    SAI_NEXTHOP_LOG_TRACE ("Inserted Next Hop node in NH Id database. "
                           "NH Id: 0x%"PRIx64".", p_nh_node->next_hop_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_remove_nh_node_from_nh_id_database (
                                                      sai_fib_nh_t *p_nh_node)
{
    sai_fib_nh_t   *p_tmp_node = NULL;
    rbtree_handle   nh_id_tree = NULL;

    STD_ASSERT (p_nh_node != NULL);

    nh_id_tree = sai_fib_access_global_config()->nh_id_tree;

    /* Remove from the Next Hop Id global view */
    p_tmp_node = (sai_fib_nh_t *) std_rbtree_remove (nh_id_tree, p_nh_node);

    if (p_tmp_node != p_nh_node) {

        sai_fib_next_hop_log_error (p_nh_node, "Error in removing next hop id"
                                    " in RB tree.");

        return SAI_STATUS_FAILURE;
    }

    SAI_NEXTHOP_LOG_TRACE ("Removed Next Hop node from NH Id database. "
                           "NH Id: 0x%"PRIx64".", p_nh_node->next_hop_id);

    return SAI_STATUS_SUCCESS;
}

static void sai_fib_nh_rif_ref_count_incr (sai_fib_nh_t *p_next_hop)
{
    sai_fib_router_interface_t *p_rif_node = NULL;

    STD_ASSERT (p_next_hop != NULL);

    /* Get the RIF node */
    p_rif_node = sai_fib_router_interface_node_get (p_next_hop->key.rif_id);

    if (p_rif_node) {

        p_rif_node->ref_count++;

        SAI_NEXTHOP_LOG_TRACE ("After Incr. Next Hop RIF Id: 0x%"PRIx64". "
                               "Ref count: %d.", p_next_hop->key.rif_id,
                               p_rif_node->ref_count);
    }
}

static void sai_fib_nh_rif_ref_count_decr (sai_fib_nh_t *p_next_hop)
{
    sai_fib_router_interface_t *p_rif_node = NULL;

    STD_ASSERT (p_next_hop != NULL);

    /* Get the RIF node */
    p_rif_node = sai_fib_router_interface_node_get (p_next_hop->key.rif_id);

    if (p_rif_node) {

        STD_ASSERT (p_rif_node->ref_count > 0);

        if (p_rif_node->ref_count > 0) {

            p_rif_node->ref_count--;

            SAI_NEXTHOP_LOG_TRACE ("After Decr. Next Hop RIF Id: 0x%"PRIx64". "
                                   "Ref count: %d.", p_next_hop->key.rif_id,
                                   p_rif_node->ref_count);
        }
    }
}

sai_fib_nh_t *sai_fib_ip_next_hop_node_create (sai_object_id_t vrf_id,
                                               sai_fib_nh_t *p_nh_info,
                                               sai_status_t *p_status)
{
    sai_fib_nh_t  *p_nh_node = NULL;
    sai_fib_vrf_t *p_vrf_node = NULL;

    STD_ASSERT (p_nh_info != NULL);
    STD_ASSERT (p_status != NULL);

    /* Get the VRF node */
    p_vrf_node = sai_fib_vrf_node_get (vrf_id);

    if ((!p_vrf_node)) {

        SAI_NEXTHOP_LOG_ERR ("VRF node does not exist for VRF Id: 0x%"PRIx64".",
                             vrf_id);
        *p_status = SAI_STATUS_INVALID_OBJECT_ID;

        return NULL;
    }

    p_nh_node = sai_fib_nh_node_alloc();

    if (!p_nh_node) {

        SAI_NEXTHOP_LOG_ERR ("SAI IP Next Hop node create. Failed to "
                             "allocate memory.");

        *p_status = SAI_STATUS_NO_MEMORY;

        return NULL;
    }

    /* Copy the next hop info into the next hop node */
    memcpy (p_nh_node, p_nh_info, sizeof (sai_fib_nh_t));

    /* Insert the IP next hop node in VRF database */
    *p_status = sai_fib_insert_nh_node_in_vrf_database (p_vrf_node, p_nh_node);

    if ((*p_status) != SAI_STATUS_SUCCESS) {

        SAI_NEXTHOP_LOG_ERR ("SAI IP Next Hop node failed to insert in "
                             "VRF database.");

        sai_fib_nh_node_free (p_nh_node);

        return NULL;
    }

    /* Initialize the NH group list */
    std_dll_init (&p_nh_node->nh_group_list);

    /* Increment RIF ref count */
    sai_fib_nh_rif_ref_count_incr (p_nh_node);

    /* Initialize the Dependent Encap NH list */
    std_dll_init (&p_nh_node->dep_encap_nh_list);

    /* Initialize the Dependent Route list */
    std_dll_init (&p_nh_node->dep_route_list);

    return p_nh_node;
}

sai_status_t sai_fib_check_and_delete_ip_next_hop_node (
                                                sai_object_id_t vrf_id,
                                                sai_fib_nh_t *p_nh_node)
{
    sai_fib_vrf_t *p_vrf_node = NULL;

    STD_ASSERT (p_nh_node != NULL);

    if ((!(p_nh_node->owner_flag))) {

        /* Get the VRF node */
        p_vrf_node = sai_fib_vrf_node_get (vrf_id);

        if ((!p_vrf_node)) {

            SAI_NEXTHOP_LOG_ERR ("VRF node does not exist for VRF Id: 0x%"PRIx64".",
                                 vrf_id);

            return SAI_STATUS_INVALID_OBJECT_ID;
        }

        sai_fib_remove_nh_node_from_vrf_database (p_vrf_node, p_nh_node);

        /* Decrement RIF ref count */
        sai_fib_nh_rif_ref_count_decr (p_nh_node);

        sai_fib_nh_node_free (p_nh_node);
    }

    return SAI_STATUS_SUCCESS;
}

static inline bool sai_fib_is_next_hop_used (sai_fib_nh_t *p_next_hop)
{
    return (p_next_hop->ref_count ? true : false);
}

static inline bool sai_fib_is_next_hop_in_nh_group (sai_fib_nh_t *p_next_hop)
{
    sai_fib_wt_link_node_t  *p_group_link_node = NULL;

    p_group_link_node = sai_fib_get_first_nh_group_from_nh (p_next_hop);

    return ((p_group_link_node != NULL) ? true : false);
}

sai_status_t sai_fib_ip_next_hop_create (sai_fib_nh_t *p_nh_info,
                                         sai_fib_nh_t **p_out_next_hop_node)
{
    sai_status_t          status;
    sai_fib_nh_t         *p_nh_node = NULL;
    sai_npu_object_id_t   next_hop_hw_id;

    STD_ASSERT (p_nh_info != NULL);
    STD_ASSERT (p_out_next_hop_node != NULL);

    /* Get an existing next hop node */
    p_nh_node = sai_fib_ip_next_hop_node_get (p_nh_info->key.nh_type,
                                         p_nh_info->key.rif_id,
                                         sai_fib_next_hop_ip_addr (p_nh_info),
                                         p_nh_info->key.tunnel_type);

    if (p_nh_node) {

        if (sai_fib_is_owner_next_hop (p_nh_node)) {

            sai_fib_next_hop_log_error (p_nh_node, "Next Hop already exists.");

            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }

        /* Copy the next hop attributes into the next hop node */
        sai_fib_next_hop_attr_copy (p_nh_node, p_nh_info);

    } else {

        p_nh_node = sai_fib_ip_next_hop_node_create (p_nh_info->vrf_id,
                                                     p_nh_info, &status);
        if ((!p_nh_node)) {

            sai_fib_next_hop_log_error (p_nh_info, "Failed to create next hop "
                                        "node for IP Next Hop info.");

            return status;
        }
    }

    sai_fib_set_next_hop_owner (p_nh_node, SAI_FIB_OWNER_NEXT_HOP);

    if (p_nh_node->key.nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP) {

        /* Create the IP Tunnel Encap next hop */
        status = sai_fib_encap_next_hop_create (p_nh_node, &next_hop_hw_id);

    } else {

        /* Create the IP next hop in NPU */
        status = sai_nexthop_npu_api_get()->nexthop_create (p_nh_node, &next_hop_hw_id);

    }

    if (status != SAI_STATUS_SUCCESS) {

        sai_fib_next_hop_log_error (p_nh_node, "Failed to create next hop "
                                    "in NPU.");

        sai_fib_next_hop_info_reset (p_nh_node);

        sai_fib_check_and_delete_ip_next_hop_node (p_nh_node->vrf_id,
                                                   p_nh_node);

        return status;
    }

    p_nh_node->next_hop_id = sai_uoid_create (SAI_OBJECT_TYPE_NEXT_HOP,
                                              next_hop_hw_id);

    *p_out_next_hop_node   = p_nh_node;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_fib_ip_next_hop_remove (sai_fib_nh_t *p_nh_node)
{
    sai_status_t  status;

    STD_ASSERT (p_nh_node != NULL);

    /* Check if the next hop is part of any nh group */
    if (sai_fib_is_next_hop_in_nh_group (p_nh_node)) {

        sai_fib_next_hop_log_error (p_nh_node, "Next Hop is part of NH group.");

        return SAI_STATUS_FAILURE;
    }

    if (p_nh_node->key.nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP) {

        /* Remove the IP Tunnel Encap next hop */
        status = sai_fib_encap_next_hop_remove (p_nh_node);

    } else {

        /* Remove the IP next hop in NPU */
        status = sai_nexthop_npu_api_get()->nexthop_remove (p_nh_node);
    }

    if (status != SAI_STATUS_SUCCESS) {

        sai_fib_next_hop_log_error (p_nh_node, "Failed to remove next hop "
                                    "in NPU.");

        return status;
    }

    /* Reset the next hop attributes in next hop node */
    sai_fib_next_hop_info_reset (p_nh_node);

    sai_fib_next_hop_log_trace (p_nh_node, "IP Next hop deletion successful.");

    /* Free the next hop node */
    sai_fib_check_and_delete_ip_next_hop_node (p_nh_node->vrf_id, p_nh_node);

    return SAI_STATUS_SUCCESS;
}

/* Next Hop IPv4 address attribute is expected in Network Byte Order */
static sai_status_t sai_fib_next_hop_create (sai_object_id_t *p_next_hop_id,
                                             sai_object_id_t switch_id,
                                             uint32_t attr_count,
                                             const sai_attribute_t *attr_list)
{
    sai_status_t   status;
    sai_fib_nh_t  *p_nh_node = NULL;
    sai_fib_nh_t   nh_info;

    SAI_NEXTHOP_LOG_TRACE ("SAI Next Hop creation, attr_count: %d.",
                           attr_count);

    STD_ASSERT (p_next_hop_id != NULL);

    memset (&nh_info, 0, sizeof (sai_fib_nh_t));

    sai_fib_next_hop_default_attr_set (&nh_info);

    sai_fib_lock ();

    do {
        status = sai_fib_next_hop_info_fill (&nh_info, attr_count, attr_list,
                                             true);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NEXTHOP_LOG_ERR ("SAI Next Hop creation. Error in "
                                 "attribute list.");

            break;
        }

        if ((nh_info.key.nh_type == SAI_NEXT_HOP_TYPE_IP) ||
            (nh_info.key.nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP)) {

            /* Create the IP Next hop */
            status = sai_fib_ip_next_hop_create (&nh_info, &p_nh_node);

        } else {

            status = SAI_STATUS_NOT_SUPPORTED;
        }

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NEXTHOP_LOG_ERR ("SAI Next Hop creation failure. Error: %d.",
                                 status);

            break;
        }

        /* Insert the next hop id in SAI database */
        status = sai_fib_insert_nh_node_in_nh_id_database (p_nh_node);

        if (status != SAI_STATUS_SUCCESS) {

            sai_fib_next_hop_log_error (p_nh_node, "Failure to insert the "
                                        "next hop Id in database.");

            if ((nh_info.key.nh_type == SAI_NEXT_HOP_TYPE_IP) ||
                (nh_info.key.nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP)) {

                sai_fib_ip_next_hop_remove (p_nh_node);
            }

            break;
        }
    } while (0);

    if (status == SAI_STATUS_SUCCESS) {

        (*p_next_hop_id) = p_nh_node->next_hop_id;

        sai_fib_next_hop_log_trace (p_nh_node, "Next hop creation successful.");

        SAI_NEXTHOP_LOG_INFO ("Next Hop: 0x%"PRIx64" created.", (*p_next_hop_id));
    }

    sai_fib_unlock ();

    return status;
}

static sai_status_t sai_fib_next_hop_remove (sai_object_id_t next_hop_id)
{
    sai_status_t   status;
    sai_fib_nh_t  *p_nh_node = NULL;

    SAI_NEXTHOP_LOG_TRACE ("SAI Next Hop deletion, next_hop_id: 0x%"PRIx64".",
                           next_hop_id);

    if (!sai_is_obj_id_next_hop (next_hop_id)) {
        SAI_NEXTHOP_LOG_ERR ("0x%"PRIx64" is not a valid Next Hop obj id.",
                             next_hop_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_fib_lock ();

    do {
        /* Get the next hop node */
        p_nh_node = sai_fib_next_hop_node_get_from_id (next_hop_id);

        if (!p_nh_node) {

            SAI_NEXTHOP_LOG_ERR ("Next Hop Id not found: 0x%"PRIx64".", next_hop_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }

        sai_fib_next_hop_log_trace (p_nh_node, "Next hop to be removed.");

        if ((!sai_fib_is_owner_next_hop (p_nh_node))) {

            sai_fib_next_hop_log_error (p_nh_node, "Next Hop is not found.");

            status = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }

        /* Check if the next hop is pointed by another object */
        if (sai_fib_is_next_hop_used (p_nh_node)) {

            sai_fib_next_hop_log_error (p_nh_node, "Next Hop is being used.");

            status = SAI_STATUS_OBJECT_IN_USE;

            break;
        }

        /* Remove the next hop id from database */
        status = sai_fib_remove_nh_node_from_nh_id_database (p_nh_node);

        if (status != SAI_STATUS_SUCCESS) {

            sai_fib_next_hop_log_error (p_nh_node, "Next Hop node failed to "
                                        "remove from NH Id database.");

            status = SAI_STATUS_FAILURE;

            break;
        }

        if ((p_nh_node->key.nh_type == SAI_NEXT_HOP_TYPE_IP) ||
            (p_nh_node->key.nh_type == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP)) {

            /* Remove the IP Next hop */
            status = sai_fib_ip_next_hop_remove (p_nh_node);

        } else {

            status = SAI_STATUS_NOT_SUPPORTED;
        }

        if (status != SAI_STATUS_SUCCESS) {

            sai_fib_next_hop_log_error (p_nh_node, "Next Hop remove failure.");

            sai_fib_insert_nh_node_in_nh_id_database (p_nh_node);

            break;

        } else {
            SAI_NEXTHOP_LOG_INFO ("Next Hop: 0x%"PRIx64" removed.", next_hop_id);
        }
    } while (0);

    sai_fib_unlock ();

    return status;
}

static sai_status_t sai_fib_next_hop_attribute_set (
                                                sai_object_id_t next_hop_id,
                                                const sai_attribute_t *p_attr)
{
    /* @TODO Update if any new settable attribute is added in future */

    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t sai_fib_next_hop_attribute_get (
                                                sai_object_id_t next_hop_id,
                                                uint32_t attr_count,
                                                sai_attribute_t *attr_list)
{
    sai_status_t   status = SAI_STATUS_FAILURE;
    sai_fib_nh_t  *p_nh_node = NULL;

    SAI_NEXTHOP_LOG_TRACE ("SAI Next Hop Get Attribute, next_hop_id: 0x%"PRIx64".",
                           next_hop_id);

    if (!sai_is_obj_id_next_hop (next_hop_id)) {
        SAI_NEXTHOP_LOG_ERR ("0x%"PRIx64" is not a valid Next Hop obj id.",
                             next_hop_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if ((!attr_count)) {

        SAI_NEXTHOP_LOG_ERR ("SAI Next Hop Get attribute. Invalid input."
                             " attr_count: %d.", attr_count);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT (attr_list != NULL);

    sai_fib_lock ();

    do {
        /* Get the next hop node */
        p_nh_node = sai_fib_next_hop_node_get_from_id (next_hop_id);

        if (!p_nh_node) {

            SAI_NEXTHOP_LOG_ERR ("Next Hop Id not found: 0x%"PRIx64".", next_hop_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }

        if ((!sai_fib_is_owner_next_hop (p_nh_node))) {

            sai_fib_next_hop_log_error (p_nh_node, "SAI Next Hop Get Attribute."
                                        "Next Hop not found.");

            status = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }

        /* Get the next hop attributes from NPU */
        status = sai_nexthop_npu_api_get()->nexthop_attribute_get (p_nh_node,
                                                       attr_count, attr_list);
        if (status != SAI_STATUS_SUCCESS) {

            sai_fib_next_hop_log_error (p_nh_node, "Failed to get Next Hop "
                                        "attribute from NPU.");
            break;
        }
    } while (0);

    if (status == SAI_STATUS_SUCCESS) {

        sai_fib_next_hop_log_trace (p_nh_node, "SAI Next Hop Get Attribute "
                                    "success.");
    } else {

        SAI_NEXTHOP_LOG_ERR ("SAI Next Hop Get Attribute failed.");
    }

    sai_fib_unlock ();

    return status;
}

static sai_next_hop_api_t sai_next_hop_method_table = {
    sai_fib_next_hop_create,
    sai_fib_next_hop_remove,
    sai_fib_next_hop_attribute_set,
    sai_fib_next_hop_attribute_get,
};

sai_next_hop_api_t *sai_nexthop_api_query (void)
{
    return &sai_next_hop_method_table;
}

