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
* @file sai_l3_router interface.c
*
* @brief This file contains function definitions for SAI router interface
*        functionality API implementation.
*
*************************************************************************/
#include "sai_l3_api_utils.h"
#include "sai_l3_mem.h"
#include "sai_l3_common.h"
#include "sai_l3_util.h"
#include "sai_l3_api.h"
#include "sai_switch_utils.h"
#include "sai_port_utils.h"
#include "sai_port_common.h"
#include "sai_vlan_api.h"
#include "sai_lag_api.h"
#include "saistatus.h"
#include "std_mac_utils.h"
#include "std_assert.h"
#include "sai_common_infra.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

static inline bool sai_fib_is_rif_type_valid (uint_t type)
{
    return ((type == SAI_ROUTER_INTERFACE_TYPE_PORT) ||
            (type == SAI_ROUTER_INTERFACE_TYPE_VLAN) ||
            (type == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK) ||
            (type == SAI_ROUTER_INTERFACE_TYPE_BRIDGE));
}

static inline bool sai_fib_rif_is_admin_state_valid (bool state)
{
    return ((state == true) || (state == false));
}

static inline bool sai_fib_rif_is_port_valid (sai_object_id_t port_id)
{
    return ((sai_is_port_valid (port_id)) &&
            (port_id != sai_switch_cpu_port_obj_id_get ()));
}

static inline bool sai_fib_rif_is_attr_create_only(uint_t attr_id)
{
    if ((SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID == attr_id) ||
        (SAI_ROUTER_INTERFACE_ATTR_TYPE == attr_id) ||
        (SAI_ROUTER_INTERFACE_ATTR_PORT_ID == attr_id) ||
        (SAI_ROUTER_INTERFACE_ATTR_VLAN_ID == attr_id) ||
        (SAI_ROUTER_INTERFACE_ATTR_IS_VIRTUAL == attr_id)) {
        return true;
    } else {
        return false;
    }
}

static inline uint64_t sai_fib_rif_port_or_vlan_id_get (
sai_fib_router_interface_t *p_rif_node)
{
    if (p_rif_node->type == SAI_ROUTER_INTERFACE_TYPE_PORT) {
        return ((uint64_t) p_rif_node->attachment.port_id);
    } else if (p_rif_node->type == SAI_ROUTER_INTERFACE_TYPE_VLAN) {
        return ((uint64_t) p_rif_node->attachment.vlan_id);
    } else {
        return 0;
    }
}

static inline void sai_fib_rif_log_trace (
sai_fib_router_interface_t *p_rif_node, char *p_info_str)
{
    char p_buf [SAI_FIB_MAX_BUFSZ];

    SAI_RIF_LOG_TRACE ("%s, RIF Id: 0x%"PRIx64" (VR: 0x%"PRIx64", %s 0x%"PRIx64"),"
                       "ref_count: %d, V4 admin state: %s, V6 admin state: %s, "
                       "MTU: %d, MAC: %s, IP Options packet action: %d (%s).",
                       p_info_str, p_rif_node->rif_id, p_rif_node->vrf_id,
                       sai_fib_rif_type_to_str (p_rif_node->type),
                       sai_fib_rif_port_or_vlan_id_get (p_rif_node),
                       p_rif_node->ref_count, (p_rif_node->v4_admin_state)?
                       "ON" : "OFF", (p_rif_node->v6_admin_state)? "ON" : "OFF",
                       p_rif_node->mtu, std_mac_to_string
                       ((const hal_mac_addr_t *)&p_rif_node->src_mac, p_buf,
                        SAI_FIB_MAX_BUFSZ), p_rif_node->ip_options_pkt_action,
                       sai_packet_action_str
                       (p_rif_node->ip_options_pkt_action));
}

static inline void sai_fib_rif_log_error (
sai_fib_router_interface_t *p_rif_node, char *p_error_str)
{
    SAI_RIF_LOG_ERR ("%s, RIF Id: 0x%"PRIx64" (VR: 0x%"PRIx64", %s 0x%"PRIx64").",
                     p_error_str, p_rif_node->rif_id, p_rif_node->vrf_id,
                     sai_fib_rif_type_to_str (p_rif_node->type),
                     sai_fib_rif_port_or_vlan_id_get (p_rif_node));
}

static sai_status_t sai_fib_rif_vrf_attr_set (
sai_fib_router_interface_t *p_rif_node, sai_object_id_t vr_id)
{
    sai_fib_vrf_t *p_vrf_node = NULL;

    if (!sai_is_obj_id_vr (vr_id)) {
        SAI_RIF_LOG_ERR ("0x%"PRIx64" is not a valid VR obj id.", vr_id);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_vrf_node = sai_fib_vrf_node_get (vr_id);

    if (!p_vrf_node) {
        SAI_RIF_LOG_ERR ("VR ID 0x%"PRIx64" does not exist in VRF tree.", vr_id);

        /*
         * Attribute index based return code will be recalculated by the
         * caller of this function.
         */
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_rif_node->vrf_id = vr_id;

    SAI_RIF_LOG_TRACE ("Router Interface VR attribute set to 0x%"PRIx64".", vr_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_rif_type_attr_set (
sai_fib_router_interface_t *p_rif_node, uint_t type)
{
    sai_mac_t mac = {0};

    if (!(sai_fib_is_rif_type_valid (type))) {
        SAI_RIF_LOG_ERR ("%d is not a valid Router Interface type.", type);

        /*
         * Attribute index based return code will be recalculated by the
         * caller of this function.
         */
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    sai_switch_mac_address_get (&mac);

    /* For loopback rif reset the mac to zero from the switch mac address which
     * was set during initialization of the rif node*/
    if((type == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK) &&
       (memcmp(p_rif_node->src_mac, mac, sizeof (sai_mac_t)) == 0)) {
        memset(p_rif_node->src_mac, 0, sizeof (sai_mac_t));
    }

    if((!sai_fib_is_mac_address_zero((const sai_mac_t *)&p_rif_node->src_mac)) &&
       (type == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK)) {
        SAI_RIF_LOG_ERR ("Src MAC address cannot be specified on a Loopback "
                         "Router Interface");

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_rif_node->type = type;

    SAI_RIF_LOG_TRACE ("Router Interface Type attribute set to %d (%s).",
                       type, sai_fib_rif_type_to_str (type));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_rif_mac_attr_set (
sai_fib_router_interface_t *p_rif_node, const sai_mac_t *p_mac)
{
    char p_buf [SAI_FIB_MAX_BUFSZ];

    if (sai_fib_is_mac_address_zero (p_mac) ||
        p_rif_node->type == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK) {
        SAI_RIF_LOG_ERR ("MAC address is not valid.");

        /*
         * Attribute index based return code will be recalculated by the
         * caller of this function.
         */
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    memcpy (p_rif_node->src_mac, p_mac, sizeof (sai_mac_t));

    SAI_RIF_LOG_TRACE ("Router Interface MAC attribute set to %s.",
                       std_mac_to_string ((const hal_mac_addr_t *)
                       &p_rif_node->src_mac, p_buf, SAI_FIB_MAX_BUFSZ));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_rif_v4_admin_state_attr_set (
sai_fib_router_interface_t *p_rif_node, bool state)
{
    if (!(sai_fib_rif_is_admin_state_valid (state))) {
        SAI_RIF_LOG_ERR ("%d is not a valid Router Interface admin state.",
                         state);

        /*
         * Attribute index based return code will be recalculated by the
         * caller of this function.
         */
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_rif_node->v4_admin_state = state;

    SAI_RIF_LOG_TRACE ("Router Interface V4 Admin state set to %s.",
                       (state)? "ON" : "OFF");

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_rif_v6_admin_state_attr_set (
sai_fib_router_interface_t *p_rif_node, bool state)
{
    if (!(sai_fib_rif_is_admin_state_valid (state))) {
        SAI_RIF_LOG_ERR ("%d is not a valid Router Interface admin state.",
                         state);

        /*
         * Attribute index based return code will be recalculated by the
         * caller of this function.
         */
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_rif_node->v6_admin_state = state;

    SAI_RIF_LOG_TRACE ("Router Interface V6 Admin state set to %s.",
                       (state)? "ON" : "OFF");

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_rif_mtu_attr_set (
sai_fib_router_interface_t *p_rif_node, uint_t mtu)
{
    p_rif_node->mtu = mtu;

    SAI_RIF_LOG_TRACE ("Router Interface MTU set to %d.", p_rif_node->mtu);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_rif_port_id_set (
sai_fib_router_interface_t *p_rif_node, sai_object_id_t port_id)
{
    bool  is_port = sai_is_obj_id_port (port_id);
    bool  is_lag  = sai_is_obj_id_lag (port_id);

    if ((!is_lag) && (!is_port)) {

        SAI_RIF_LOG_ERR ("0x%"PRIx64" is not an expected obj id.", port_id);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    if ((is_lag && !sai_is_lag_created (port_id)) ||
        (is_port && !sai_fib_rif_is_port_valid (port_id))) {

        SAI_RIF_LOG_ERR ("0x%"PRIx64" is not a valid %s obj id.",
                         port_id, is_port ? "port" : "lag");

        /*
         * Attribute index based return code will be recalculated by the
         * caller of this function.
         */
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_rif_node->attachment.port_id = port_id;

    SAI_RIF_LOG_TRACE ("Router Interface port ID set to 0x%"PRIx64".", port_id);

    return SAI_STATUS_SUCCESS;
}

/*
 * Attribute index based return code will be recalculated by the caller of this
 * function.
 */
static sai_status_t sai_fib_rif_vlan_id_set (
sai_fib_router_interface_t *p_rif_node, sai_object_id_t vlan_obj_id)
{
    sai_vlan_id_t vlan_id = sai_vlan_obj_id_to_vlan_id(vlan_obj_id);

    if (!(sai_is_valid_vlan_id (vlan_id))) {
        SAI_RIF_LOG_ERR ("%d is not a valid VLAN ID.", vlan_id);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    if (!(sai_is_vlan_created (vlan_id))) {
        SAI_RIF_LOG_ERR ("VLAN ID %d is not yet created.", vlan_id);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_rif_node->attachment.vlan_id = vlan_obj_id;

    SAI_RIF_LOG_TRACE ("Router Interface VLAN ID set to %d.", vlan_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_rif_create_attr_validate (sai_attr_id_t id,
                                                      uint_t type,
                                                      bool *is_port_set,
                                                      bool *is_vlan_set,
                                                      bool *is_vrf_set)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    bool is_rif_type_logical = (type == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK) ||
        (type == SAI_ROUTER_INTERFACE_TYPE_BRIDGE) ? true : false;
    bool is_rif_physical_attr_set = (*is_port_set || *is_vlan_set) ? true : false;

    SAI_RIF_LOG_TRACE ("Validating Attribute id: %d, type: %d, is_port_set: %d,"
                       " is_vlan_set: %d, is_vrf_set: %d", id, type,
                       *is_port_set, *is_vlan_set, *is_vrf_set);

    switch (id)
    {
        case SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID:
            *is_vrf_set = true;
            break;

        case SAI_ROUTER_INTERFACE_ATTR_TYPE:
            if (((*is_port_set) && (type == SAI_ROUTER_INTERFACE_TYPE_VLAN)) ||
                ((*is_vlan_set) && (type == SAI_ROUTER_INTERFACE_TYPE_PORT))) {
                /*
                 * Attribute index based return code will be recalculated by
                 * the caller of this function.
                 */
                sai_rc = SAI_STATUS_INVALID_ATTRIBUTE_0;
            } else if(is_rif_type_logical && is_rif_physical_attr_set) {
                /* For logical rif types, physical rif attributes should not be
                 * set. */
                sai_rc = SAI_STATUS_INVALID_ATTRIBUTE_0;
            }

            break;

        case SAI_ROUTER_INTERFACE_ATTR_PORT_ID:
            if ((*is_vlan_set) || (type == SAI_ROUTER_INTERFACE_TYPE_VLAN) ||
                (type == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK) ||
                (type == SAI_ROUTER_INTERFACE_TYPE_BRIDGE)) {
                sai_rc = SAI_STATUS_INVALID_ATTRIBUTE_0;
            } else {
                *is_port_set = true;
            }

            break;

        case SAI_ROUTER_INTERFACE_ATTR_VLAN_ID:
            if ((*is_port_set) || (type == SAI_ROUTER_INTERFACE_TYPE_PORT) ||
                (type == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK) ||
                (type == SAI_ROUTER_INTERFACE_TYPE_BRIDGE)) {
                sai_rc = SAI_STATUS_INVALID_ATTRIBUTE_0;
            } else {
                *is_vlan_set = true;
            }
            break;

        case SAI_ROUTER_INTERFACE_ATTR_IS_VIRTUAL:
            break;

        default:
            SAI_RIF_LOG_TRACE ("Attribute id: %d - validation is not done.",
                               id);
    }

    return sai_rc;
}

static sai_status_t sai_fib_rif_create_attr_set (
sai_fib_router_interface_t *p_rif_node, const sai_attribute_t *p_attr)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_rif_node != NULL);

    STD_ASSERT(p_attr != NULL);

    SAI_RIF_LOG_TRACE ("Setting Router Interface CREATE attribute, id: %d.",
                       p_attr->id);

    switch (p_attr->id) {
        case SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID:
            sai_rc = sai_fib_rif_vrf_attr_set (p_rif_node, p_attr->value.oid);

            break;

        case SAI_ROUTER_INTERFACE_ATTR_TYPE:
            sai_rc = sai_fib_rif_type_attr_set (p_rif_node, p_attr->value.s32);

            break;

        case SAI_ROUTER_INTERFACE_ATTR_PORT_ID:
            sai_rc = sai_fib_rif_port_id_set (p_rif_node, p_attr->value.oid);

            break;

        case SAI_ROUTER_INTERFACE_ATTR_VLAN_ID:
            sai_rc = sai_fib_rif_vlan_id_set (p_rif_node, p_attr->value.oid);

            break;

        case SAI_ROUTER_INTERFACE_ATTR_IS_VIRTUAL:
            p_rif_node->is_virtual = p_attr->value.booldata;
            break;

        default:
            SAI_RIF_LOG_ERR ("Attribute id: %d is not a known CREATE attribute "
                             "for Router Interface.", p_attr->id);

            /*
             * Attribute index based return code will be recalculated by the
             * caller of this function.
             */
            return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
    }

    return sai_rc;
}

static sai_status_t sai_fib_rif_optional_attr_set (
sai_fib_router_interface_t *p_rif_node, const sai_attribute_t *p_attr,
uint_t *p_flags)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_rif_node != NULL);

    STD_ASSERT(p_attr != NULL);

    STD_ASSERT(p_flags != NULL);

    SAI_RIF_LOG_TRACE ("Setting Router Interface OPTIONAL attribute, id: %d.",
                       p_attr->id);

    switch (p_attr->id) {
        case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
            sai_rc = sai_fib_rif_mac_attr_set (p_rif_node, &p_attr->value.mac);

            *p_flags |= SAI_FIB_SRC_MAC_ATTR_FLAG;
            break;

        case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
            sai_rc = sai_fib_rif_v4_admin_state_attr_set (p_rif_node,
                                                          p_attr->value.booldata);

            *p_flags |= SAI_FIB_V4_ADMIN_STATE_ATTR_FLAG;
            break;

        case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE:
            sai_rc = sai_fib_rif_v6_admin_state_attr_set (p_rif_node,
                                                          p_attr->value.booldata);

            *p_flags |= SAI_FIB_V6_ADMIN_STATE_ATTR_FLAG;
            break;

        case SAI_ROUTER_INTERFACE_ATTR_MTU:
            sai_rc = sai_fib_rif_mtu_attr_set (p_rif_node, p_attr->value.u32);

            *p_flags |= SAI_FIB_MTU_ATTR_FLAG;
            break;

        case SAI_ROUTER_INTERFACE_ATTR_INGRESS_ACL:
        case SAI_ROUTER_INTERFACE_ATTR_EGRESS_ACL:
            sai_rc = SAI_STATUS_SUCCESS;
            break;

        case SAI_ROUTER_INTERFACE_ATTR_V4_MCAST_ENABLE:
        case SAI_ROUTER_INTERFACE_ATTR_V6_MCAST_ENABLE:
            sai_rc = SAI_STATUS_ATTR_NOT_IMPLEMENTED_0;
            break;

        default:
            SAI_RIF_LOG_ERR ("Attribute id: %d is not a known OPTIONAL attribute"
                             " for Router Interface.", p_attr->id);

            /*
             * Attribute index based return code will be recalculated by the
             * caller of this function.
             */
            return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
    }

    SAI_RIF_LOG_TRACE ("Updated attribute flags: 0x%x.", *p_flags);


    return sai_rc;
}

static void sai_fib_rif_attributes_init (sai_fib_router_interface_t *p_rif_node)
{
    sai_mac_t mac;

    STD_ASSERT(p_rif_node != NULL);

    sai_switch_mac_address_get (&mac);

    memcpy (p_rif_node->src_mac, mac, sizeof (sai_mac_t));

    p_rif_node->v4_admin_state = SAI_FIB_V4_ADMIN_STATE_DFLT;
    p_rif_node->v6_admin_state = SAI_FIB_V6_ADMIN_STATE_DFLT;
    p_rif_node->mtu = SAI_FIB_MTU_DFLT;
    p_rif_node->ip_options_pkt_action = SAI_FIB_IP_OPT_PKT_ACTION_DFLT;
    p_rif_node->type = SAI_FIB_RIF_TYPE_NONE;
}

static sai_status_t sai_fib_rif_inherit_vrf_attributes (
sai_fib_router_interface_t *p_rif_node, uint_t rif_attr_flags)
{
    sai_fib_vrf_t *p_vrf_node = NULL;

    STD_ASSERT(p_rif_node != NULL);

    SAI_RIF_LOG_TRACE ("Inheriting attributes from VR 0x%"PRIx64" to RIF node, "
                       "Attribute flags: 0x%x.", p_rif_node->vrf_id,
                       rif_attr_flags);

    p_vrf_node = sai_fib_vrf_node_get (p_rif_node->vrf_id);

    if (!p_vrf_node) {
        SAI_RIF_LOG_ERR ("VR ID 0x%"PRIx64" does not exist in VRF tree.",
                         p_rif_node->vrf_id);

        return SAI_STATUS_FAILURE;
    }

    if (!(SAI_FIB_V4_ADMIN_STATE_ATTR_FLAG & rif_attr_flags)) {
        SAI_RIF_LOG_TRACE ("Applying VR V4 admin state to RIF.");

        p_rif_node->v4_admin_state = p_vrf_node->v4_admin_state;
    }

    if (!(SAI_FIB_V6_ADMIN_STATE_ATTR_FLAG & rif_attr_flags)) {
        SAI_RIF_LOG_TRACE ("Applying VR V6 admin state to RIF.");

        p_rif_node->v6_admin_state = p_vrf_node->v6_admin_state;
    }

    if (!(SAI_FIB_SRC_MAC_ATTR_FLAG & rif_attr_flags)) {
        SAI_RIF_LOG_TRACE ("Applying VR SRC MAC to RIF.");

        memcpy (p_rif_node->src_mac, p_vrf_node->src_mac,
                sizeof (sai_mac_t));
    }

    p_rif_node->ip_options_pkt_action = p_vrf_node->ip_options_pkt_action;

    return SAI_STATUS_SUCCESS;
}

static bool sai_fib_rif_is_duplicate (sai_fib_router_interface_t *p_rif_node)
{
    rbtree_handle  rif_tree;
    sai_fib_router_interface_t *p_db_rif_node;

    STD_ASSERT(p_rif_node != NULL);

    if((p_rif_node->type == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK)||
       (p_rif_node->type == SAI_ROUTER_INTERFACE_TYPE_BRIDGE)) {
        return false;
    }

    rif_tree = sai_fib_access_global_config()->router_interface_tree;

    p_db_rif_node = std_rbtree_getfirst (rif_tree);

    while (p_db_rif_node != NULL) {

        if ((p_rif_node->type == p_db_rif_node->type) &&
            (sai_fib_rif_port_or_vlan_id_get (p_rif_node) ==
            (sai_fib_rif_port_or_vlan_id_get (p_db_rif_node)))) {

            if (p_rif_node->vrf_id != p_db_rif_node->vrf_id) {
                sai_fib_rif_log_trace (p_db_rif_node, "Another RIF exists "
                             "for the same Port or VLAN on different VRF.");

                return true;
            }

            if ((!(memcmp (p_rif_node->src_mac, p_db_rif_node->src_mac,
                sizeof (sai_mac_t))))) {
                sai_fib_rif_log_trace (p_db_rif_node, "Duplicate RIF object info.");

                return true;
            }
        }

        p_db_rif_node = std_rbtree_getnext (rif_tree, p_db_rif_node);
    }

    return false;
}

static sai_status_t sai_fib_rif_attributes_parse (
sai_fib_router_interface_t *p_rif_node, uint_t attr_count,
const sai_attribute_t *attr_list)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    bool             is_port_set = false;
    bool             is_vlan_set = false;
    bool             is_vrf_set = false;
    uint_t           type = SAI_FIB_RIF_TYPE_NONE;
    uint_t           flags = 0;
    uint_t           list_idx = 0;
    const sai_attribute_t *p_attr = NULL;

    STD_ASSERT(p_rif_node != NULL);

    STD_ASSERT(attr_list != NULL);

    SAI_RIF_LOG_TRACE ("Parsing attributes for Router Interface create, "
                       "attribute count: %d.", attr_count);

    for (list_idx = 0; list_idx < attr_count; list_idx++) {
        p_attr = &attr_list [list_idx];

        SAI_RIF_LOG_TRACE ("Parsing attr_list [%d], Attribute id: %d.",
                           list_idx, p_attr->id);

        if (sai_fib_rif_is_attr_create_only(p_attr->id)) {
            SAI_RIF_LOG_TRACE ("Mandatory attribute for Router Interface "
                               "create.");

            type = (p_attr->id == SAI_ROUTER_INTERFACE_ATTR_TYPE)?
                (p_attr->value.s32) : (p_rif_node->type);

            sai_rc =
                sai_fib_rif_create_attr_validate (p_attr->id, type,
                                                  &is_port_set, &is_vlan_set,
                                                  &is_vrf_set);

            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_RIF_LOG_ERR ("Router Interface Type/Port/Vlan/Loopback/1D attribute "
                                 "validation failed.");
            } else {
                sai_rc = sai_fib_rif_create_attr_set (p_rif_node, p_attr);
            }
        } else {
            sai_rc = sai_fib_rif_optional_attr_set (p_rif_node, p_attr, &flags);
        }

        if (sai_rc == SAI_STATUS_SUCCESS) {
            sai_rc = sai_rif_npu_api_get()->rif_attr_validate (p_rif_node->type,
                                                               p_attr);

            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_RIF_LOG_ERR ("NPU validation failed for RIF attribute ID: "
                                 "%d, Type: %s.", p_attr->id,
                                 sai_fib_rif_type_to_str (p_rif_node->type));
            }
        }

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_ERR ("Failed to set Router Interface attr_list [%d], "
                             "Attribute Id: %d, Error: %d.", list_idx,
                             p_attr->id, sai_rc);

            return (sai_fib_attr_status_code_get (sai_rc, list_idx));
        }
    }

    if(type == SAI_ROUTER_INTERFACE_TYPE_BRIDGE) {
        if(!is_vrf_set) {
            SAI_RIF_LOG_ERR ("VRF id not provided when creating 1D bridge RIF");
            return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
        }
    } else if (((!is_port_set) && (!is_vlan_set) && (type != SAI_ROUTER_INTERFACE_TYPE_LOOPBACK)) ||
        (!is_vrf_set) || (type == SAI_FIB_RIF_TYPE_NONE)) {
        SAI_RIF_LOG_ERR ("One or more Mandatory attributes are missing, "
                         "is_port_set: %d, is_vlan_set: %d, is_vrf_set: %d, "
                         "type: %s.", is_port_set, is_vlan_set, is_vrf_set,
                         sai_fib_rif_type_to_str (type));

        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    SAI_RIF_LOG_TRACE ("optional attributes flags: 0x%x", flags);

    sai_fib_rif_inherit_vrf_attributes (p_rif_node, flags);

    if (sai_fib_rif_is_duplicate (p_rif_node)) {

        SAI_RIF_LOG_ERR ("Duplicate RIF object creation error.");

        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    sai_fib_rif_log_trace (p_rif_node,
                           "Router Interface attributes parsing success");

    return sai_rc;
}

static bool sai_fib_rif_is_create_attr_count_valid (uint_t attr_count)
{
    if ((attr_count < SAI_FIB_RIF_MANDATORY_ATTR_COUNT) ||
        (attr_count > SAI_FIB_RIF_MAX_ATTR_COUNT)) {
        SAI_RIF_LOG_ERR ("Attribute count %d is invalid. Expected count: "
                         "<%d - %d>.",attr_count,
                         SAI_FIB_RIF_MANDATORY_ATTR_COUNT,
                         SAI_FIB_RIF_MAX_ATTR_COUNT);

        return false;
    }

    return true;
}

static sai_status_t sai_port_routing_mode_set (sai_object_id_t sai_port_id)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    sai_port_fwd_mode_t fwd_mode = SAI_PORT_FWD_MODE_UNKNOWN;
    bool                is_lag = sai_is_obj_id_lag(sai_port_id);

    SAI_RIF_LOG_TRACE ("Setting routing mode for Port: 0x%"PRIx64"",
                       sai_port_id);
    /* validate forwarding mode for the port. */
    do {
        if(is_lag) {
            sai_rc = sai_lag_forwarding_mode_get (sai_port_id, &fwd_mode);
        } else {
            sai_rc = sai_port_forwarding_mode_get (sai_port_id, &fwd_mode);
        }

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_ERR ("Failed to get forwarding mode for port obj id 0x%"PRIx64", "
                             "Error: %d.", sai_port_id, sai_rc);
            break;
        }

        if (fwd_mode == SAI_PORT_FWD_MODE_SWITCHING) {
            SAI_RIF_LOG_WARN ("Port 0x%"PRIx64" is in switching mode", sai_port_id);

            break;
        }
    } while (0);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    if (fwd_mode == SAI_PORT_FWD_MODE_UNKNOWN ||
        fwd_mode == SAI_PORT_FWD_MODE_SWITCHING) {
        /* Set forwarding mode for the port as ROUTING. */
        fwd_mode = SAI_PORT_FWD_MODE_ROUTING;
        if(is_lag) {
            sai_rc = sai_lag_forwarding_mode_set (sai_port_id, fwd_mode);
        } else {
            sai_rc = sai_port_forwarding_mode_set (sai_port_id, fwd_mode);
        }

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_ERR ("Failed to set routing mode for port 0x%"PRIx64", "
                             "Error: %d.", sai_port_id, sai_rc);
            return sai_rc;
        }
        SAI_RIF_LOG_TRACE ("RIF Port: 0x%"PRIx64" mode set to ROUTING.", sai_port_id);
    } else {
        SAI_RIF_LOG_TRACE ("Duplicate update. RIF Port: 0x%"PRIx64" mode already"
                           " set to %s (%d).", sai_port_id,
                           sai_port_forwarding_mode_to_str (fwd_mode), fwd_mode);
    }

    return sai_rc;
}

static sai_status_t sai_port_routing_mode_reset (sai_object_id_t sai_port_id)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    sai_port_fwd_mode_t fwd_mode = SAI_PORT_FWD_MODE_UNKNOWN;
    bool                is_lag = sai_is_obj_id_lag(sai_port_id);

    SAI_RIF_LOG_TRACE ("Resetting routing mode for Port: 0x%"PRIx64"",
                       sai_port_id);
    /* Check and Reset the forwarding mode for the port. */

    do {
        if(is_lag) {
            sai_rc = sai_lag_forwarding_mode_get (sai_port_id, &fwd_mode);
        } else {
            sai_rc = sai_port_forwarding_mode_get (sai_port_id, &fwd_mode);
        }

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_ERR ("Failed to get forwarding mode for port obj id 0x%"PRIx64", "
                             "Error: %d.", sai_port_id, sai_rc);
            break;
        }

        if (fwd_mode == SAI_PORT_FWD_MODE_UNKNOWN) {
            SAI_RIF_LOG_TRACE ("Mode is already reset on port 0x%"PRIx64"", sai_port_id);

            break;
        }

        fwd_mode = SAI_PORT_FWD_MODE_UNKNOWN;
        if(is_lag) {
            sai_rc = sai_lag_forwarding_mode_set (sai_port_id, fwd_mode);
        } else {
            sai_rc = sai_port_forwarding_mode_set (sai_port_id, fwd_mode);
        }

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_ERR ("Failed to reset forwarding mode for port 0x%"PRIx64", "
                             "Error: %d.", sai_port_id, sai_rc);
            break;
        }
    } while (0);

    return sai_rc;
}

static sai_status_t sai_fib_rif_routing_config_update (sai_fib_router_interface_t *p_rif,
                                                       bool is_set)
{
    sai_status_t        status = SAI_STATUS_SUCCESS;
    sai_object_id_t     sai_oid = 0;

    if ((p_rif->type == SAI_ROUTER_INTERFACE_TYPE_VLAN) ||
        (p_rif->type == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK)||
        (p_rif->type == SAI_ROUTER_INTERFACE_TYPE_BRIDGE)){
        return status;
    }

    sai_oid = p_rif->attachment.port_id;

    if (is_set) {
        status = sai_port_routing_mode_set (sai_oid);
    } else {
        status = sai_port_routing_mode_reset (sai_oid);
    }

    return status;
}

static void sai_fib_rif_free_resources (sai_fib_router_interface_t *p_rif_node,
                                        bool is_rif_set_in_npu,
                                        bool is_rif_set_in_vrf_list,
                                        bool is_routing_cfg_set)
{
    if (p_rif_node == NULL) {
        return;
    }

    SAI_RIF_LOG_TRACE ("is_rif_set_in_npu: %d, is_rif_set_in_vrf_list: %d, "
                       "is_routing_cfg_set: %d.", is_rif_set_in_npu,
                       is_rif_set_in_vrf_list, is_routing_cfg_set);

    /* Remove NPU settings from NPU, if it was already applied on this RIF. */
    if (is_rif_set_in_npu) {
        sai_rif_npu_api_get()->rif_remove (p_rif_node);
    }

    /* Delete RIF node from the VRF's RIF list, if it was already added. */
    if (is_rif_set_in_vrf_list) {
        sai_fib_vrf_rif_list_update (p_rif_node, false);
    }

    /* Reset routing config on the RIF */
    if (is_routing_cfg_set) {
        sai_fib_rif_routing_config_update (p_rif_node, false);
    }

    sai_fib_rif_node_free (p_rif_node);

    return;
}

static sai_object_id_t sai_fib_get_lag_id_from_attr(uint_t attr_count, const sai_attribute_t *attr_list)
{
    uint_t attr_idx;

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        if(attr_list[attr_idx].id == SAI_ROUTER_INTERFACE_ATTR_PORT_ID) {
            if(sai_is_obj_id_lag(attr_list[attr_idx].value.oid)) {
                return attr_list[attr_idx].value.oid;
            }
            break;
        }
    }
    return SAI_NULL_OBJECT_ID;
}

static sai_status_t sai_fib_router_interface_create (
sai_object_id_t *rif_obj_id, sai_object_id_t switch_id, uint32_t attr_count,
const sai_attribute_t *attr_list)
{
    sai_status_t                sai_rc = SAI_STATUS_SUCCESS;
    bool                        is_rif_set_in_npu = false;
    bool                        is_rif_set_in_vrf_list = false;
    bool                        is_routing_cfg_set = false;
    rbtree_handle               rif_tree = NULL;
    sai_fib_router_interface_t *p_rif_node = NULL;
    sai_npu_object_id_t         rif_hw_id;
    sai_object_id_t             lag_attach_id = SAI_NULL_OBJECT_ID;
    sai_status_t                tmp_sai_rc = SAI_STATUS_SUCCESS;

    SAI_RIF_LOG_TRACE ("Creating Router interface, attr_count: %d.",
                       attr_count);

    STD_ASSERT (rif_obj_id != NULL);
    STD_ASSERT (attr_list != NULL);

    if (!(sai_fib_rif_is_create_attr_count_valid (attr_count))) {
        SAI_RIF_LOG_ERR ("Input parameters validation failed for RIF.");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((p_rif_node = sai_fib_rif_node_alloc ()) == NULL) {
        SAI_RIF_LOG_ERR ("Failed to allocate memory for RIF node.");

        return SAI_STATUS_NO_MEMORY;
    }

    sai_fib_rif_attributes_init (p_rif_node);

    lag_attach_id = sai_fib_get_lag_id_from_attr(attr_count, attr_list);
    sai_fib_lock ();
    /*
     * Take LAG Lock to prevent LAG from being modified while RIF operates on
     * LAG Members
     */
    if(lag_attach_id != SAI_NULL_OBJECT_ID) {
        sai_lag_lock();
    }
    do {
        sai_rc = sai_fib_rif_attributes_parse (p_rif_node, attr_count,
                                               attr_list);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_ERR ("Failed to parse RIF attributes for create.");
            break;
        }

        sai_rc = sai_fib_rif_routing_config_update (p_rif_node, true);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_ERR ("Failed to set routing config for RIF.");
            break;
        }

        is_routing_cfg_set = true;

        sai_rc = sai_rif_npu_api_get()->rif_create (p_rif_node, &rif_hw_id);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            sai_fib_rif_log_error (p_rif_node, "RIF creation failed in NPU");
            break;
        }

        *rif_obj_id = sai_uoid_create (SAI_OBJECT_TYPE_ROUTER_INTERFACE,
                                       rif_hw_id);

        is_rif_set_in_npu = true;
        sai_fib_rif_log_trace (p_rif_node, "RIF created in NPU");

        p_rif_node->rif_id = *rif_obj_id;

        /* Add the RIF node to VRF's Router interface list */
        sai_rc = sai_fib_vrf_rif_list_update (p_rif_node, true);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            sai_fib_rif_log_error (p_rif_node, "Failed to add RIF node to the "
                                   "RIF List in VR node");
            break;
        }

        is_rif_set_in_vrf_list = true;

        rif_tree = sai_fib_access_global_config()->router_interface_tree;

        if ((std_rbtree_insert (rif_tree, p_rif_node)) != STD_ERR_OK) {
            sai_fib_rif_log_error (p_rif_node, "Failed to insert to RIF Tree");

            sai_rc = SAI_STATUS_FAILURE;
            break;
        }
    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        sai_fib_rif_log_trace (p_rif_node, "Router interface creation success");


        SAI_RIF_LOG_INFO ("Router Interface: 0x%"PRIx64" created.", *rif_obj_id);
    } else {
        sai_fib_rif_log_error (p_rif_node, "Failed to create Router Interface");

        sai_fib_rif_free_resources (p_rif_node, is_rif_set_in_npu,
                                    is_rif_set_in_vrf_list, is_routing_cfg_set);
    }


    if (lag_attach_id != SAI_NULL_OBJECT_ID) {
        sai_lag_unlock();
        tmp_sai_rc = sai_fib_lag_rif_mapping_insert(lag_attach_id, *rif_obj_id);
        if (tmp_sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_TRACE ("Create map between lag id 0x%"PRIx64" and rif id 0x%"PRIx64" "
                               "failed with error code %d", lag_attach_id, *rif_obj_id, tmp_sai_rc);
        }
    }

    sai_fib_unlock ();
    return sai_rc;
}

static sai_status_t sai_fib_router_interface_remove (
sai_object_id_t rif_id)
{
    sai_status_t                sai_rc = SAI_STATUS_SUCCESS;
    rbtree_handle               rif_tree = NULL;
    sai_fib_router_interface_t *p_rif_node = NULL;
    sai_object_id_t             lag_attach_id;
    sai_status_t                tmp_sai_rc = SAI_STATUS_SUCCESS;

    SAI_RIF_LOG_TRACE ("Deleting Router Interface: 0x%"PRIx64".", rif_id);

    if (!sai_is_obj_id_rif (rif_id)) {
        SAI_RIF_LOG_ERR ("0x%"PRIx64" is not a valid RIF obj id.", rif_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_fib_lock ();
    p_rif_node = sai_fib_router_interface_node_get (rif_id);

    if (!p_rif_node) {
        SAI_RIF_LOG_ERR ("RIF Id: 0x%"PRIx64" does not exist.", rif_id);
        sai_fib_unlock ();
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    lag_attach_id = sai_fib_rif_is_attachment_lag (p_rif_node) ?
        p_rif_node->attachment.port_id : SAI_NULL_OBJECT_ID;

    /*
     * Take LAG Lock to prevent LAG from being modified while RIF operates on
     * LAG Members
     */
    if(lag_attach_id != SAI_NULL_OBJECT_ID) {
        sai_lag_lock();
    }
    do {

        sai_fib_rif_log_trace (p_rif_node, "Router Interface Info");

        if (p_rif_node->ref_count != 0)
        {
            SAI_RIF_LOG_ERR ("Router Interface 0x%"PRIx64" is currently in use with one"
                             " or more next-hop objects - can not be deleted.",
                             rif_id);

            sai_rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        sai_rc = sai_rif_npu_api_get()->rif_remove (p_rif_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_ERR ("Router interface deletion failed in NPU for "
                             "RIF Id: 0x%"PRIx64".", p_rif_node->rif_id);

            break;
        }

        /* Delete from the VRF's Router interface list */
        sai_fib_vrf_rif_list_update (p_rif_node, false);

        rif_tree = sai_fib_access_global_config()->router_interface_tree;

        std_rbtree_remove (rif_tree, p_rif_node);

        sai_fib_rif_routing_config_update (p_rif_node, false);

        sai_fib_rif_node_free (p_rif_node);
    } while (0);


    if (lag_attach_id != SAI_NULL_OBJECT_ID) {
        sai_lag_unlock();
        if (sai_rc == SAI_STATUS_SUCCESS) {
            tmp_sai_rc = sai_fib_lag_rif_mapping_remove (lag_attach_id, rif_id);

            if (tmp_sai_rc != SAI_STATUS_SUCCESS) {
                SAI_RIF_LOG_TRACE ("Delete map between lag id 0x%"PRIx64" and rif id 0x%"PRIx64" "
                                   "failed with error code %d", lag_attach_id, rif_id, tmp_sai_rc);
            }
        }
    }
    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_RIF_LOG_INFO ("Removed Router Interface: 0x%"PRIx64".", rif_id);

    } else {
        SAI_RIF_LOG_ERR ("Failed to remove Router Interface: 0x%"PRIx64".", rif_id);
    }

    sai_fib_unlock ();
    return sai_rc;
}

static sai_status_t sai_fib_router_interface_attribute_set (
sai_object_id_t rif_id, const sai_attribute_t *p_attr)
{
    sai_status_t                sai_rc = SAI_STATUS_SUCCESS;
    sai_fib_router_interface_t *p_rif_node = NULL;
    sai_fib_router_interface_t  rif_node_in;
    uint_t                      attr_flags = 0;

    STD_ASSERT (p_attr != NULL);

    SAI_RIF_LOG_TRACE ("Setting Attribute ID: %d on Router Interface: 0x%"PRIx64".",
                       p_attr->id, rif_id);

    if (!sai_is_obj_id_rif (rif_id)) {
        SAI_RIF_LOG_ERR ("0x%"PRIx64" is not a valid RIF obj id.", rif_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_fib_lock ();

    do {
        p_rif_node = sai_fib_router_interface_node_get (rif_id);

        if (!p_rif_node) {
            SAI_RIF_LOG_ERR ("RIF Id: 0x%"PRIx64" does not exist.", rif_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }

        if (sai_fib_rif_is_attr_create_only (p_attr->id)) {
            SAI_RIF_LOG_ERR ("create-only attribute passed in Router Interface"
                             " set.");

            sai_rc = SAI_STATUS_INVALID_ATTRIBUTE_0;

            break;
        }

        sai_fib_rif_log_trace (p_rif_node, "Router Interface current Info");

        sai_rc = sai_rif_npu_api_get()->rif_attr_validate (p_rif_node->type,
                                                           p_attr);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_TRACE ("NPU validation failed for RIF attribute ID: %d, "
                             "Type: %s.", p_attr->id,
                             sai_fib_rif_type_to_str (p_rif_node->type));

            break;
        }

        /* Copy existing node info to new node */
        memcpy (&rif_node_in, p_rif_node, sizeof (sai_fib_router_interface_t));

        /* Update the new node with input attribute info */
        sai_rc = sai_fib_rif_optional_attr_set (&rif_node_in, p_attr,
                                                &attr_flags);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_ERR ("Failed to set Router Interface Attribute ID: %d, "
                             "Error: %d.", p_attr->id, sai_rc);

            break;
        }

        if (sai_fib_rif_is_node_info_duplicate (&rif_node_in, p_rif_node)) {
            SAI_RIF_LOG_TRACE ("Attribute ID: %d already set with input value. "
                               "Update is not required in NPU.", p_attr->id);

            break;
        }

        sai_rc = sai_rif_npu_api_get()->rif_attr_set (&rif_node_in, attr_flags);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_ERR ("Failed to set Router Interface Attribute ID: %d "
                             "in NPU, Error: %d.", p_attr->id, sai_rc);

            break;
        }

        memcpy (p_rif_node, &rif_node_in, sizeof (sai_fib_router_interface_t));
    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        sai_fib_rif_log_trace (p_rif_node, "Router Interface updated Info");
    } else {
        SAI_RIF_LOG_TRACE ("Failed to set/update Route Interface attributes.");
    }

    sai_fib_unlock ();

    return sai_rc;
}

static sai_status_t sai_fib_router_interface_attribute_get (
sai_object_id_t rif_id, uint32_t attr_count,
sai_attribute_t *attr_list)
{
    sai_status_t                sai_rc = SAI_STATUS_FAILURE;
    sai_fib_router_interface_t *p_rif_node = NULL;

    STD_ASSERT (attr_list != NULL);

    SAI_RIF_LOG_TRACE ("Getting Attributes for RIF Id: 0x%"PRIx64", count: %d.",
                       rif_id, attr_count);

    if (!sai_is_obj_id_rif (rif_id)) {
        SAI_RIF_LOG_ERR ("0x%"PRIx64" is not a valid RIF obj id.", rif_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if ((attr_count == 0) || (attr_count > SAI_FIB_RIF_MAX_ATTR_COUNT)) {
        SAI_RIF_LOG_ERR ("Attribute count %d is invalid. Expected count: "
                         "<1-%d>.", attr_count, SAI_FIB_RIF_MAX_ATTR_COUNT);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_fib_lock ();

    do {
        p_rif_node = sai_fib_router_interface_node_get (rif_id);

        if (!p_rif_node) {
            SAI_RIF_LOG_ERR ("RIF Id: 0x%"PRIx64" does not exist.", rif_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_fib_rif_log_trace (p_rif_node, "RIF node current Info");

        sai_rc = sai_rif_npu_api_get()->rif_attr_get (p_rif_node,
                                                      attr_count, attr_list);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_ERR ("Failed to get RIF Attributes from NPU, "
                             "Error: %d.", sai_rc);

            break;
        }
    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        sai_fib_rif_log_trace (p_rif_node, "RIF get attributes completed");
    } else {
        SAI_RIF_LOG_ERR ("Failed to get RIF attributes.");
    }

    sai_fib_unlock ();

    return sai_rc;
}

static sai_status_t sai_rif_lag_member_update (sai_object_id_t rif_id,
sai_object_id_t lag_id, const sai_object_list_t *port_id_list, bool is_add)

{
    sai_status_t                sai_rc = SAI_STATUS_FAILURE;
    sai_fib_router_interface_t *p_rif_node = NULL;

    STD_ASSERT (port_id_list != NULL);

    if (port_id_list->count == 0 || port_id_list->list == NULL) {
        SAI_RIF_LOG_ERR ("LAG member update passed with invalid port list.");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_fib_lock ();

    do {
        p_rif_node = sai_fib_router_interface_node_get (rif_id);

        if (!p_rif_node) {
            SAI_RIF_LOG_TRACE ("RIF Id: 0x%"PRIx64" does not exist.", rif_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_fib_rif_log_trace (p_rif_node, "Router Interface Info");

        /* Validate the input LAG Id */
        if (p_rif_node->attachment.port_id != lag_id) {
            SAI_RIF_LOG_TRACE ("RIF attachment Id 0x%"PRIx64" is not same as the "
                               "LAG Id 0x%"PRIx64".",
                               p_rif_node->attachment.port_id, lag_id);

            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

        sai_rc = sai_rif_npu_api_get()->rif_lag_member_update (p_rif_node,
                                                               port_id_list,
                                                               is_add);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_RIF_LOG_TRACE ("Failed to add LAG member for Router Interface "
                               "in NPU, Error: %d.", sai_rc);

            break;
        }

    } while (0);

    sai_fib_unlock ();

    SAI_RIF_LOG_TRACE ("RIF Id: 0x%"PRIx64", LAG Id: 0x%"PRIx64", is_add: %d, "
                       "Returning %d from LAG member update callback.", rif_id,
                       lag_id, is_add, sai_rc);

    return sai_rc;
}

sai_status_t sai_rif_lag_callback (sai_object_id_t lag_id,
                                   sai_lag_operation_t lag_opcode,
                                   const sai_object_list_t *port_list)
{
    sai_object_id_t rif_id = SAI_NULL_OBJECT_ID;

    SAI_RIF_LOG_TRACE ("RIF LAG callback for RIF Id: 0x%"PRIx64", LAG Id: "
                       "0x%"PRIx64", op_code: %d", rif_id, lag_id, lag_opcode);

    /* Validate lag_opcode */
    if ((lag_opcode != SAI_LAG_OPER_ADD_PORTS) &&
        (lag_opcode != SAI_LAG_OPER_DEL_PORTS)) {

        SAI_RIF_LOG_TRACE ("LAG Id: 0x%"PRIx64" op_code: %d is not handled.",
                           lag_id, lag_opcode);

        return SAI_STATUS_SUCCESS;
    }

    sai_fib_get_rif_id_from_lag_id (lag_id, &rif_id);
    if(rif_id == SAI_NULL_OBJECT_ID) {
        SAI_RIF_LOG_TRACE ("LAG Id: 0x%"PRIx64" has no RIF mapping", lag_id);
        return SAI_STATUS_SUCCESS;
    }

    return sai_rif_lag_member_update (rif_id, lag_id, port_list,
                                      ((lag_opcode == SAI_LAG_OPER_ADD_PORTS)
                                       ? true : false));
}

static sai_router_interface_api_t sai_router_interface_method_table = {
    sai_fib_router_interface_create,
    sai_fib_router_interface_remove,
    sai_fib_router_interface_attribute_set,
    sai_fib_router_interface_attribute_get,
};

sai_router_interface_api_t *sai_router_intf_api_query (void)
{
    return (&sai_router_interface_method_table);
}
