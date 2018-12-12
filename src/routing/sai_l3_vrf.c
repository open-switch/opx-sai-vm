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
* @file  sai_l3_vrf.c
*
* @brief This file contains function definitions for SAI virtual router
*        functionality API implementation.
*
*************************************************************************/
#include "sai_l3_api_utils.h"
#include "sai_l3_mem.h"
#include "sai_l3_common.h"
#include "sai_l3_util.h"
#include "sai_l3_api.h"
#include "sai_modules_init.h"
#include "sai_switch_utils.h"
#include "saistatus.h"
#include "std_mac_utils.h"
#include "std_assert.h"
#include "sai_common_infra.h"
#include "sai_lag_callback.h"
#include "sai_fdb_main.h"
#include "sai_l3_next_hop_group_utl.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

static inline bool sai_fib_vrf_is_admin_state_valid (bool state)
{
    return ((state == true) || (state == false));
}

static inline void sai_fib_vrf_log_trace (sai_fib_vrf_t *p_vrf_node,
                                          char *p_info_str)
{
    char p_buf [SAI_FIB_MAX_BUFSZ];

    SAI_ROUTER_LOG_TRACE ("%s, VR ID: 0x%"PRIx64", V4 admin state: %s, V6 admin state:"
                          " %s, IP Options packet action: %d (%s), MAC: %s, "
                          "Number of router interfaces: %d.", p_info_str,
                          p_vrf_node->vrf_id, (p_vrf_node->v4_admin_state)?
                          "ON" : "OFF", (p_vrf_node->v6_admin_state)?
                          "ON" : "OFF", p_vrf_node->ip_options_pkt_action,
                          sai_packet_action_str
                          (p_vrf_node->ip_options_pkt_action),
                          std_mac_to_string ((const hal_mac_addr_t *)
                          &p_vrf_node->src_mac, p_buf, SAI_FIB_MAX_BUFSZ),
                          p_vrf_node->num_rif);
}

static sai_status_t sai_fib_vrf_mac_attr_set (sai_fib_vrf_t *p_vrf_node,
                                              const sai_mac_t *p_mac)
{
    char p_buf [SAI_FIB_MAX_BUFSZ];

    if (sai_fib_is_mac_address_zero (p_mac)) {
        SAI_ROUTER_LOG_ERR ("MAC address is not valid.");

        /*
         * Attribute index based return code will be recalculated by the
         * caller of this function.
         */
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    memcpy (p_vrf_node->src_mac, p_mac, sizeof (sai_mac_t));

    SAI_ROUTER_LOG_TRACE ("VRF MAC attribute set to %s.", std_mac_to_string
                          ((const hal_mac_addr_t *)&p_vrf_node->src_mac, p_buf,
                          SAI_FIB_MAX_BUFSZ));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_vrf_v4_admin_state_attr_set (
sai_fib_vrf_t *p_vrf_node, bool state)
{
    if (!(sai_fib_vrf_is_admin_state_valid (state))) {
        SAI_ROUTER_LOG_ERR ("%d is not a valid VRF admin state.", state);

        /*
         * Attribute index based return code will be recalculated by the
         * caller of this function.
         */
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_vrf_node->v4_admin_state = state;

    SAI_ROUTER_LOG_TRACE ("VRF V4 Admin state set to %s.", (state)? "ON" :
                          "OFF");

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_vrf_v6_admin_state_attr_set (
sai_fib_vrf_t *p_vrf_node, bool state)
{
    if (!(sai_fib_vrf_is_admin_state_valid (state))) {
        SAI_ROUTER_LOG_ERR ("%d is not a valid VRF admin state.", state);

        /*
         * Attribute index based return code will be recalculated by the
         * caller of this function.
         */
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_vrf_node->v6_admin_state = state;

    SAI_ROUTER_LOG_TRACE ("VRF V6 Admin state set to %s.", (state)? "ON" :
                          "OFF");

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_vrf_ip_options_attr_set (sai_fib_vrf_t *p_vrf_node,
                                                     uint_t ip_opt_pkt_action)
{
    if (!(sai_packet_action_validate (ip_opt_pkt_action))) {
        SAI_ROUTER_LOG_ERR ("IP OPTIONS Packet Action attribute value %d is "
                            "not valid.", ip_opt_pkt_action);

        /*
         * Attribute index based return code will be recalculated by the
         * caller of this function.
         */
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_vrf_node->ip_options_pkt_action = ip_opt_pkt_action;

    SAI_ROUTER_LOG_TRACE ("IP OPTIONS packet action set to %d (%s).",
                          ip_opt_pkt_action,
                          sai_packet_action_str (ip_opt_pkt_action));

    return SAI_STATUS_SUCCESS;
}

/*
 * Attribute index based return code will be recalculated by the caller of this
 * function.
 */
static sai_status_t sai_fib_vrf_ttl_violation_attr_set (
sai_fib_vrf_t *p_vrf_node, uint_t pkt_action)
{
    if (!(sai_packet_action_validate (pkt_action))) {
        SAI_ROUTER_LOG_ERR ("TTL violation Packet Action attribute value %d is "
                            "not valid.", pkt_action);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_vrf_node->ttl0_1_pkt_action = pkt_action;

    SAI_ROUTER_LOG_TRACE ("TTL violation packet action set to %d (%s).",
                          pkt_action, sai_packet_action_str (pkt_action));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_vrf_create_params_validate (
uint_t attr_count, const sai_attribute_t *attr_list)
{
    if (attr_count > SAI_FIB_VRF_MAX_ATTR_COUNT) {
        SAI_ROUTER_LOG_ERR ("Attribute count %d is invalid. "
                            "Expected count: <%d - %d>.",
                            attr_count, SAI_FIB_VRF_MANDATORY_ATTR_COUNT,
                            SAI_FIB_VRF_MAX_ATTR_COUNT);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count != 0) && (!attr_list)) {
        SAI_ROUTER_LOG_ERR ("Attribute count %d. But attribute list is "
                            "not provided.", attr_count);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

static void sai_fib_vrf_attributes_init (sai_fib_vrf_t *p_vrf_node)
{
    sai_mac_t mac;

    STD_ASSERT(p_vrf_node != NULL);

    p_vrf_node->v4_admin_state = SAI_FIB_V4_ADMIN_STATE_DFLT;
    p_vrf_node->v6_admin_state = SAI_FIB_V6_ADMIN_STATE_DFLT;
    p_vrf_node->ip_options_pkt_action = SAI_FIB_IP_OPT_PKT_ACTION_DFLT;
    p_vrf_node->ttl0_1_pkt_action = SAI_FIB_TTL_VIOLATION_PKT_ACTION_DFLT;

    sai_switch_mac_address_get (&mac);

    memcpy (p_vrf_node->src_mac, mac, sizeof (sai_mac_t));
}

static sai_status_t sai_fib_vrf_attributes_parse (
sai_fib_vrf_t *p_vrf_node, uint_t attr_count, const sai_attribute_t *attr_list,
uint_t *p_attr_flags)
{
    sai_status_t           sai_rc = SAI_STATUS_SUCCESS;
    uint_t                 list_idx = 0;
    const sai_attribute_t *p_attr = NULL;

    STD_ASSERT(p_vrf_node != NULL);
    STD_ASSERT(p_attr_flags != NULL);

    SAI_ROUTER_LOG_TRACE ("Parsing VRF attributes, attribute count: %d.",
                          attr_count);

    for (list_idx = 0; list_idx < attr_count; list_idx++) {
        p_attr = &attr_list [list_idx];

        SAI_ROUTER_LOG_TRACE ("Parsing attr_list [%d], Attribute id: %d.",
                              list_idx, p_attr->id);

        switch (p_attr->id) {
            case SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS:
                sai_rc =
                    sai_fib_vrf_mac_attr_set (p_vrf_node, &p_attr->value.mac);

                *p_attr_flags |= SAI_FIB_SRC_MAC_ATTR_FLAG;
                break;

            case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE:
                sai_rc =
                    sai_fib_vrf_v4_admin_state_attr_set (p_vrf_node,
                                                         p_attr->value.booldata);

                *p_attr_flags |= SAI_FIB_V4_ADMIN_STATE_ATTR_FLAG;
                break;

            case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE:
                sai_rc =
                    sai_fib_vrf_v6_admin_state_attr_set (p_vrf_node,
                                                         p_attr->value.booldata);

                *p_attr_flags |= SAI_FIB_V6_ADMIN_STATE_ATTR_FLAG;
                break;

            case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_PACKET_ACTION:
                sai_rc = sai_fib_vrf_ttl_violation_attr_set (p_vrf_node,
                                                             p_attr->value.s32);

                *p_attr_flags |= SAI_FIB_TTL_VIOLATION_ATTR_FLAG;
                break;

            case SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_IP_OPTIONS_PACKET_ACTION:
                sai_rc = sai_fib_vrf_ip_options_attr_set (p_vrf_node,
                                                          p_attr->value.s32);

                *p_attr_flags |= SAI_FIB_IP_OPTIONS_ATTR_FLAG;
                break;

            case SAI_VIRTUAL_ROUTER_ATTR_UNKNOWN_L3_MULTICAST_PACKET_ACTION:
                sai_rc = SAI_STATUS_ATTR_NOT_IMPLEMENTED_0;
                break;

            default:
                SAI_ROUTER_LOG_ERR ("Attribute id [%d]: %d is not a known "
                                    "attribute for VRF functionality.",
                                    list_idx, p_attr->id);

                sai_rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
        }

        if (sai_rc == SAI_STATUS_SUCCESS) {
            sai_rc = sai_router_npu_api_get()->vr_attr_validate (p_attr);

            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_ROUTER_LOG_ERR ("NPU validation failed for VRF attribute ID"
                                    ": %d.", p_attr->id);
            }
        }

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ROUTER_LOG_ERR ("Failed to set VRF attr_list [%d], "
                                "Attribute Id: %d, Error: %d.",
                                list_idx, p_attr->id, sai_rc);

            return (sai_fib_attr_status_code_get (sai_rc, list_idx));
        }
    }

    /*
     * MAC must be non-zero at end of parsing - either attribute list based
     * MAC input or the switch MAC in case MAC attribute is not passed in list.
     */
    if (sai_fib_is_mac_address_zero ((const sai_mac_t *)&p_vrf_node->src_mac)) {
        SAI_ROUTER_LOG_ERR ("MAC address is not valid.");

        return SAI_STATUS_ADDR_NOT_FOUND;
    }

    SAI_ROUTER_LOG_TRACE ("VRF attributes parsing success, attr_flags: 0x%x.",
                          *p_attr_flags);

    return sai_rc;
}

static void sai_fib_vrf_free_resources (sai_fib_vrf_t *p_vrf_node)
{
    if (!p_vrf_node) {
        return;
    }

    if (p_vrf_node->sai_nh_tree) {
        std_radix_destroy (p_vrf_node->sai_nh_tree);

        p_vrf_node->sai_nh_tree = NULL;
    }

    if (p_vrf_node->sai_route_tree) {
        std_radix_destroy (p_vrf_node->sai_route_tree);

        p_vrf_node->sai_route_tree = NULL;
    }

    sai_fib_vrf_node_free (p_vrf_node);

    return;
}

static sai_status_t sai_fib_vrf_node_trees_create (sai_fib_vrf_t *p_vrf_node)
{
    char tree_name_str [SAI_FIB_RDX_MAX_NAME_LEN];

    STD_ASSERT(p_vrf_node != NULL);

    SAI_ROUTER_LOG_TRACE ("Creating VRF NH Tree for VR: 0x%"PRIx64".",
                          p_vrf_node->vrf_id);

    snprintf (tree_name_str, SAI_FIB_RDX_MAX_NAME_LEN, "VRF_%d_NH_Tree",
              (uint_t)p_vrf_node->vrf_id);

    p_vrf_node->sai_nh_tree = std_radix_create (tree_name_str,
                                                SAI_FIB_NH_IP_ADDR_TREE_KEY_LEN,
                                                NULL, NULL, 0);

    if (!p_vrf_node->sai_nh_tree) {
        SAI_ROUTER_LOG_ERR ("Failed to create NH Radix Tree for VRF 0x%"PRIx64".",
                            p_vrf_node->vrf_id);

        sai_router_npu_api_get()->vr_remove (p_vrf_node);

        sai_fib_vrf_free_resources (p_vrf_node);

        return SAI_STATUS_NO_MEMORY;
    }

    SAI_ROUTER_LOG_TRACE ("Creating VRF Route Tree for VR: 0x%"PRIx64".",
                          p_vrf_node->vrf_id);

    snprintf (tree_name_str, SAI_FIB_RDX_MAX_NAME_LEN, "VRF_%d_Route_Tree",
              (uint_t)p_vrf_node->vrf_id);

    p_vrf_node->sai_route_tree = std_radix_create (tree_name_str,
                                                   SAI_FIB_ROUTE_TREE_KEY_SIZE,
                                                   NULL, NULL, 0);

    if (!p_vrf_node->sai_route_tree) {
        SAI_ROUTER_LOG_ERR ("Failed to create Route Radix Tree for VRF 0x%"PRIx64".",
                            p_vrf_node->vrf_id);

        sai_router_npu_api_get()->vr_remove (p_vrf_node);

        sai_fib_vrf_free_resources (p_vrf_node);

        return SAI_STATUS_NO_MEMORY;
    }

    std_radix_enable_radical (p_vrf_node->sai_route_tree);

    std_radical_walkconstructor (p_vrf_node->sai_route_tree,
                                 &p_vrf_node->route_marker);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_vrf_node_insert_to_tree (sai_fib_vrf_t *p_vrf_node)
{
    rbtree_handle vrf_tree = NULL;
    t_std_error   err_rc = STD_ERR_OK;

    vrf_tree = sai_fib_access_global_config()->vrf_tree;

    err_rc = std_rbtree_insert (vrf_tree, p_vrf_node);

    if (STD_IS_ERR(err_rc)) {
        SAI_ROUTER_LOG_ERR ("Failed to insert VRF node for VRF 0x%"PRIx64" to VRF Tree",
                            p_vrf_node->vrf_id);

        sai_router_npu_api_get()->vr_remove (p_vrf_node);

        sai_fib_vrf_free_resources (p_vrf_node);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static bool sai_fib_vrf_is_nh_tree_empty (std_rt_table *p_nh_tree)
{
    sai_fib_nh_key_t nh_node_key;

    STD_ASSERT (p_nh_tree != NULL);

    memset (&nh_node_key, 0, sizeof (sai_fib_nh_key_t));

    if (std_radix_getexact (p_nh_tree, (uint8_t *)&nh_node_key,
                            SAI_FIB_NH_IP_ADDR_TREE_KEY_LEN)) {
        return false;
    }

    if (std_radix_getnext (p_nh_tree, (uint8_t *)&nh_node_key,
                           SAI_FIB_NH_IP_ADDR_TREE_KEY_LEN)) {
        return false;
    }

    return true;
}

static bool sai_fib_vrf_is_route_tree_empty (std_rt_table *p_route_tree)
{
    sai_fib_route_key_t route_node_key;
    uint_t              key_len = 0;

    STD_ASSERT (p_route_tree != NULL);

    memset (&route_node_key, 0, sizeof (sai_fib_route_key_t));

    key_len = sai_fib_route_key_len_get (0);

    if (std_radix_getexact (p_route_tree, (uint8_t *)&route_node_key,
                            key_len)) {
        return false;
    }

    if (std_radix_getnext (p_route_tree, (uint8_t *)&route_node_key,
                           key_len)) {
        return false;
    }

    return true;
}

static inline void sai_fib_vrf_default_route_add (sai_fib_vrf_t *p_vrf_node)
{
    /* Add the default route node for IPv4 family */
    sai_fib_internal_default_route_node_add (p_vrf_node, SAI_IP_ADDR_FAMILY_IPV4);

    /* Add the default route node for IPv6 family */
    sai_fib_internal_default_route_node_add (p_vrf_node, SAI_IP_ADDR_FAMILY_IPV6);
}

static inline void sai_fib_vrf_default_route_del (sai_fib_vrf_t *p_vrf_node)
{
    /* Delete the default route node for IPv4 family */
    sai_fib_internal_default_route_node_del (p_vrf_node, SAI_IP_ADDR_FAMILY_IPV4);

    /* Delete the default route node for IPv6 family */
    sai_fib_internal_default_route_node_del (p_vrf_node, SAI_IP_ADDR_FAMILY_IPV6);
}

static sai_status_t sai_fib_virtual_router_create (
sai_object_id_t *vr_obj_id, sai_object_id_t switch_id, uint32_t attr_count,
const sai_attribute_t *attr_list)
{
    sai_status_t         sai_rc = SAI_STATUS_SUCCESS;
    sai_fib_vrf_t       *p_vrf_node = NULL;
    uint_t               attr_flags = 0;
    sai_npu_object_id_t  vr_hw_id;

    SAI_ROUTER_LOG_TRACE ("Virtual Router Creation, attr_count: %d.",
                          attr_count);

    STD_ASSERT (vr_obj_id != NULL);

    sai_rc = sai_fib_vrf_create_params_validate (attr_count, attr_list);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ROUTER_LOG_ERR ("Input parameters validation failed "
                            "for Virtual Router.");

        return sai_rc;
    }

    sai_fib_lock ();

    do {
        p_vrf_node = sai_fib_vrf_node_alloc ();

        if (!p_vrf_node) {
            SAI_ROUTER_LOG_ERR ("VRF node memory allocation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;

            break;
        }

        sai_fib_vrf_attributes_init (p_vrf_node);

        sai_rc = sai_fib_vrf_attributes_parse (p_vrf_node, attr_count,
                                               attr_list, &attr_flags);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ROUTER_LOG_ERR ("Failed to parse VRF attributes.");

            sai_fib_vrf_free_resources (p_vrf_node);

            break;
        }

        sai_fib_vrf_log_trace (p_vrf_node, "VRF attributes parsing success");

        sai_rc = sai_router_npu_api_get()->vr_create (p_vrf_node, &vr_hw_id);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ROUTER_LOG_ERR ("VRF creation failed in NPU.");

            sai_fib_vrf_free_resources (p_vrf_node);

            break;
        }

        *vr_obj_id = sai_uoid_create (SAI_OBJECT_TYPE_VIRTUAL_ROUTER,
                                      vr_hw_id);

        p_vrf_node->vrf_id = *vr_obj_id;

        sai_fib_vrf_log_trace (p_vrf_node, "VRF created in NPU");

        std_dll_init (&p_vrf_node->rif_dll_head);

        sai_rc = sai_fib_vrf_node_trees_create (p_vrf_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }

        sai_rc = sai_fib_vrf_node_insert_to_tree (p_vrf_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }

        sai_fib_num_virtual_routers_incr ();

        sai_fib_vrf_default_route_add (p_vrf_node);

    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_ROUTER_LOG_INFO ("VR Obj Id: 0x%"PRIx64", VRF ID: 0x%"PRIx64" created, "
                             "Total Virtual Routers created: %d.", *vr_obj_id,
                             vr_hw_id, sai_fib_num_virtual_routers_get ());
    } else {
        SAI_ROUTER_LOG_ERR ("Failed to create VRF ID.");
    }

    sai_fib_unlock ();

    return sai_rc;
}

static sai_status_t sai_fib_virtual_router_remove (
sai_object_id_t vr_id)
{
    sai_status_t   sai_rc = SAI_STATUS_SUCCESS;
    rbtree_handle  vrf_tree = NULL;
    sai_fib_vrf_t *p_vrf_node = NULL;

    SAI_ROUTER_LOG_TRACE ("VR ID: 0x%"PRIx64".", vr_id);

    if (!sai_is_obj_id_vr (vr_id)) {
        SAI_ROUTER_LOG_ERR ("0x%"PRIx64" is not a valid VR obj id.", vr_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_fib_lock ();

    do {
        p_vrf_node = sai_fib_vrf_node_get (vr_id);

        if (!p_vrf_node) {
            SAI_ROUTER_LOG_ERR ("VR ID 0x%"PRIx64" does not exist in VRF tree.", vr_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }

        sai_fib_vrf_log_trace (p_vrf_node, "VRF node Info.");

        sai_fib_vrf_default_route_del (p_vrf_node);

        if (p_vrf_node->num_rif != 0) {
            SAI_ROUTER_LOG_ERR ("VR ID 0x%"PRIx64" currently has %d router interface(s), "
                                "VRF can not be deleted.", vr_id, p_vrf_node->num_rif);

            sai_rc = SAI_STATUS_OBJECT_IN_USE;

            break;
        }

        if (!(sai_fib_vrf_is_nh_tree_empty (p_vrf_node->sai_nh_tree))) {
            SAI_ROUTER_LOG_ERR ("VR ID 0x%"PRIx64" can't be deleted, since NH Tree is "
                                "not empty.",vr_id);

            sai_rc = SAI_STATUS_OBJECT_IN_USE;

            break;
        }

        if (!(sai_fib_vrf_is_route_tree_empty (p_vrf_node->sai_route_tree))) {
            SAI_ROUTER_LOG_ERR ("VR ID 0x%"PRIx64" can't be deleted, since Route Tree is "
                                "not empty.", vr_id);

            sai_rc = SAI_STATUS_OBJECT_IN_USE;

            break;
        }

        sai_rc = sai_router_npu_api_get()->vr_remove (p_vrf_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ROUTER_LOG_ERR ("VRF deletion for VR ID 0x%"PRIx64" failed in NPU.",
                                vr_id);

            break;
        }

        vrf_tree = sai_fib_access_global_config()->vrf_tree;

        std_rbtree_remove (vrf_tree, p_vrf_node);

        sai_fib_vrf_free_resources (p_vrf_node);

        sai_fib_num_virtual_routers_decr ();
    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_ROUTER_LOG_INFO ("VR ID: 0x%"PRIx64" removed, Total Virtual Routers: %d.",
                             vr_id, sai_fib_num_virtual_routers_get ());
    } else {
        SAI_ROUTER_LOG_ERR ("Failed to remove VR ID: 0x%"PRIx64".", vr_id);

        if (sai_rc != SAI_STATUS_INVALID_OBJECT_ID) {
            sai_fib_vrf_default_route_add (p_vrf_node);
        }
    }

    sai_fib_unlock ();

    return sai_rc;
}

static sai_status_t sai_fib_virtual_router_attribute_set (
sai_object_id_t vr_id, const sai_attribute_t *p_attr)
{
    sai_status_t   sai_rc = SAI_STATUS_FAILURE;
    sai_fib_vrf_t *p_vrf_node = NULL;
    sai_fib_vrf_t  vrf_node_in;
    uint_t         attr_flags = 0;

    STD_ASSERT (p_attr != NULL);

    SAI_ROUTER_LOG_TRACE ("Setting Attribute ID: %d on VRF: 0x%"PRIx64".",
                          p_attr->id, vr_id);

    if (!sai_is_obj_id_vr (vr_id)) {
        SAI_ROUTER_LOG_ERR ("0x%"PRIx64" is not a valid VR obj id.", vr_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_fib_lock ();

    do {
        p_vrf_node = sai_fib_vrf_node_get (vr_id);

        if (!p_vrf_node) {
            SAI_ROUTER_LOG_ERR ("VR ID 0x%"PRIx64" does not exist.", vr_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_fib_vrf_log_trace (p_vrf_node, "VRF node current Info");

        /* Copy existing node info to new node */
        memcpy (&vrf_node_in, p_vrf_node, sizeof (sai_fib_vrf_t));

        /* Update the new node with input attribute info */
        sai_rc = sai_fib_vrf_attributes_parse (&vrf_node_in, 1, /* attr_count */
                                               p_attr, &attr_flags);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ROUTER_LOG_ERR ("Failed to set VRF Attribute ID: %d, Error: "
                                "%d.", p_attr->id, sai_rc);

            break;
        }

        sai_fib_vrf_log_trace (&vrf_node_in, "VRF attributes parsing success");

        sai_rc = sai_router_npu_api_get()->vr_attr_set (&vrf_node_in, attr_flags);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ROUTER_LOG_ERR ("Failed to set VRF Attribute ID: %d in NPU, "
                                "Error: %d.", p_attr->id, sai_rc);

            break;
        }

        /* Update the attribute info on the VRF node in SAI DB */
        memcpy (p_vrf_node, &vrf_node_in, sizeof (sai_fib_vrf_t));
    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        sai_fib_vrf_log_trace (p_vrf_node, "VRF updated Info");
    } else {
        SAI_ROUTER_LOG_ERR ("Failed to set/update VRF attributes.");
    }

    sai_fib_unlock ();

    return sai_rc;
}

static sai_status_t sai_fib_virtual_router_attribute_get (
sai_object_id_t vr_id, uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t   sai_rc = SAI_STATUS_FAILURE;
    sai_fib_vrf_t *p_vrf_node = NULL;

    STD_ASSERT (attr_list != NULL);

    SAI_ROUTER_LOG_TRACE ("Getting Attributes for VRF: 0x%"PRIx64", count: %d.",
                          vr_id, attr_count);

    if (!sai_is_obj_id_vr (vr_id)) {
        SAI_ROUTER_LOG_ERR ("0x%"PRIx64" is not a valid VR obj id.", vr_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if ((attr_count == 0) || (attr_count > SAI_FIB_VRF_MAX_ATTR_COUNT)) {
        SAI_ROUTER_LOG_ERR ("Attribute count %d is invalid. Expected count: "
                            "<1-%d>.", attr_count, SAI_FIB_VRF_MAX_ATTR_COUNT);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_fib_lock ();

    do {
        p_vrf_node = sai_fib_vrf_node_get (vr_id);

        if (!p_vrf_node) {
            SAI_ROUTER_LOG_ERR ("VR ID 0x%"PRIx64" does not exist.", vr_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_fib_vrf_log_trace (p_vrf_node, "VRF node current Info");

        sai_rc = sai_router_npu_api_get()->vr_attr_get (p_vrf_node, attr_count,
                                                        attr_list);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ROUTER_LOG_ERR ("Failed to get VRF Attributes from NPU, "
                                "Error: %d.", sai_rc);

            break;
        }
    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        sai_fib_vrf_log_trace (p_vrf_node, "VRF get attributes completed");
    } else {
        SAI_ROUTER_LOG_ERR ("Failed to get VRF attributes.");
    }

    sai_fib_unlock ();

    return sai_rc;
}

static sai_virtual_router_api_t sai_virtual_router_method_table = {
    sai_fib_virtual_router_create,
    sai_fib_virtual_router_remove,
    sai_fib_virtual_router_attribute_set,
    sai_fib_virtual_router_attribute_get
};

sai_virtual_router_api_t *sai_vr_api_query (void)
{
    return (&sai_virtual_router_method_table);
}

sai_status_t sai_fib_vrf_rif_list_update (
sai_fib_router_interface_t *p_rif_node, bool is_add)
{
    sai_object_id_t  vr_id = 0;
    sai_fib_vrf_t   *p_vrf_node = NULL;

    STD_ASSERT (p_rif_node != NULL);

    vr_id = p_rif_node->vrf_id;

    p_vrf_node = sai_fib_vrf_node_get (vr_id);

    if (!p_vrf_node) {
        SAI_ROUTER_LOG_ERR ("VR ID 0x%"PRIx64" does not exist in VRF tree.", vr_id);

        return SAI_STATUS_FAILURE;
    }

    if (is_add) {
        std_dll_insertatback (&p_vrf_node->rif_dll_head, &p_rif_node->dll_glue);

        p_vrf_node->num_rif++;
    } else {
        std_dll_remove (&p_vrf_node->rif_dll_head, &p_rif_node->dll_glue);

        p_vrf_node->num_rif--;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_router_ecmp_max_paths_set (uint_t max_paths)
{
    sai_status_t  sai_rc;

    sai_fib_lock ();

    do {
        sai_rc = sai_router_npu_api_get()->ecmp_max_paths_set (max_paths);

        if (sai_rc != SAI_STATUS_SUCCESS) {

            SAI_ROUTER_LOG_ERR ("Error: %d in setting ECMP max paths value: %d "
                                "in NPU.", sai_rc, max_paths);

            break;
        }

        sai_fib_access_global_config()->max_ecmp_paths = max_paths;

        SAI_ROUTER_LOG_INFO ("Set ECMP max paths value: %u.", max_paths);

    } while(0);

    sai_fib_unlock ();

    return sai_rc;
}

sai_status_t sai_router_init (void)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;

    SAI_ROUTER_LOG_TRACE ("Router Init.");

    sai_rc = sai_fib_global_init ();

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ROUTER_LOG_CRIT ("SAI FIB Global Data structures init failed.");

        return sai_rc;
    }

    /* Initialize the UOID generator for next hop group member id */
    sai_fib_next_hop_grp_member_gen_info_init();

    sai_rc = sai_router_npu_api_get()->fib_init ();

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ROUTER_LOG_CRIT ("SAI FIB NPU objects configuration init failed.");

        return sai_rc;
    }

    /* Register RIF callback function for LAG update */
    sai_lag_event_callback_register (SAI_MODULE_ROUTER_INTERFACE, sai_rif_lag_callback);

    /* Register Neighbor callback function for FDB entry updates */
    sai_l2_fdb_register_internal_callback (sai_neighbor_fdb_callback);

    /* Create the Tunnel Encap Next Hop Dependent route thread */
    sai_fib_encap_nh_dep_route_walker_create ();

    SAI_ROUTER_LOG_INFO ("Router Init complete.");

    return sai_rc;
}

sai_status_t sai_fib_get_vr_id_for_rif (sai_object_id_t rif_id,
                                        sai_object_id_t *vr_id)
{
    sai_status_t  status = SAI_STATUS_SUCCESS;

    sai_fib_lock ();

    sai_fib_vrf_t *p_vrf_node = sai_fib_get_vrf_node_for_rif (rif_id);

    if (p_vrf_node != NULL) {

        *vr_id = p_vrf_node->vrf_id;

    } else {

        status = SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_fib_unlock();

    return status;
}


