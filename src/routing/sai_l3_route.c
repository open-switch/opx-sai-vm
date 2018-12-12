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
* @file sai_l3_route.c
*
* @brief This file contains function definitions for the SAI route table
*        functionality API implementation.
*
*************************************************************************/
#include "sai_l3_api_utils.h"
#include "sai_l3_mem.h"
#include "sai_l3_common.h"
#include "sai_l3_api.h"
#include "sai_l3_util.h"
#include "sai.h"
#include "sairoute.h"
#include "saiswitch.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_ip_utils.h"
#include "std_struct_utils.h"
#include "std_assert.h"
#include "std_llist.h"
#include "sai_oid_utils.h"
#include "sai_common_infra.h"
#include <string.h>
#include <inttypes.h>

static inline void sai_fib_route_log_trace (sai_fib_route_t *p_route,
                                            char *p_info_str)
{
    char addr_str [SAI_FIB_MAX_BUFSZ];

    SAI_ROUTE_LOG_TRACE ("%s, VRF: 0x%"PRIx64", Prefix: %s/%d, FWD object Type: %s, "
                         "Index: 0x%"PRIx64", Packet-action: %s, Trap Priority: %d",
                         p_info_str, p_route->vrf_id, sai_ip_addr_to_str
                         (&p_route->key.prefix, addr_str, SAI_FIB_MAX_BUFSZ),
                         p_route->prefix_len, sai_fib_route_nh_type_to_str
                         (p_route->nh_type),
                         sai_fib_route_node_nh_id_get (p_route),
                         sai_packet_action_str (p_route->packet_action),
                         p_route->trap_priority);
}

static inline void sai_fib_route_log_error (sai_fib_route_t *p_route,
                                            char *p_error_str)
{
    char addr_str [SAI_FIB_MAX_BUFSZ];

    SAI_ROUTE_LOG_ERR ("%s, VRF: 0x%"PRIx64", Prefix: %s/%d, FWD object Type: %s, "
                       "Index: 0x%"PRIx64", Packet-action: %s, Trap Priority: %d",
                       p_error_str, p_route->vrf_id, sai_ip_addr_to_str
                       (&p_route->key.prefix, addr_str, SAI_FIB_MAX_BUFSZ),
                       p_route->prefix_len, sai_fib_route_nh_type_to_str
                       (p_route->nh_type), sai_fib_route_node_nh_id_get (p_route),
                       sai_packet_action_str (p_route->packet_action),
                       p_route->trap_priority);
}

bool sai_fib_route_is_nh_info_match (sai_fib_route_t *p_route_1,
                                     sai_fib_route_t *p_route_2)
{
    if (p_route_1->nh_type != p_route_2->nh_type) {
        return false;
    }

    if (memcmp (&p_route_1->nh_info, &p_route_2->nh_info,
                STD_STR_SIZE_OF (sai_fib_route_t, nh_info))) {
        return false;
    }

    return true;
}

static void sai_fib_route_nh_ref_count_incr (sai_fib_route_t *p_route_node)
{
    sai_fib_nh_t       *p_nh_node = NULL;
    sai_fib_nh_group_t *p_grp_node = NULL;

    STD_ASSERT (p_route_node != NULL);

    SAI_ROUTE_LOG_TRACE ("Incrementing Ref count for Route FWD object type: %s,"
                         "index: 0x%"PRIx64".", sai_fib_route_nh_type_to_str (
                         p_route_node->nh_type),
                         sai_fib_route_node_nh_id_get (p_route_node));

    if (p_route_node->nh_type == SAI_OBJECT_TYPE_NEXT_HOP) {
        p_nh_node = p_route_node->nh_info.nh_node;

        if (p_nh_node) {
            sai_fib_incr_nh_ref_count (p_nh_node);

            SAI_ROUTE_LOG_TRACE ("After incrementing, ref count: %d.",
                                 p_nh_node->ref_count);
        }
    } else if (p_route_node->nh_type == SAI_OBJECT_TYPE_NEXT_HOP_GROUP) {
        p_grp_node = p_route_node->nh_info.group_node;

        if (p_grp_node) {
            sai_fib_incr_nh_group_ref_count (p_grp_node);

            SAI_ROUTE_LOG_TRACE ("After incrementing, ref count: %d.",
                                 p_grp_node->ref_count);
        }
    }
}

static void sai_fib_route_nh_ref_count_decr (sai_fib_route_t *p_route_node)
{
    sai_fib_nh_t       *p_nh_node = NULL;
    sai_fib_nh_group_t *p_grp_node = NULL;

    STD_ASSERT (p_route_node != NULL);

    SAI_ROUTE_LOG_TRACE ("Decrementing Ref count for Route FWD object type: %s,"
                         "index: 0x%"PRIx64".", sai_fib_route_nh_type_to_str (
                         p_route_node->nh_type),
                         sai_fib_route_node_nh_id_get (p_route_node));

    if (p_route_node->nh_type == SAI_OBJECT_TYPE_NEXT_HOP) {
        p_nh_node = p_route_node->nh_info.nh_node;

        if (p_nh_node) {
            sai_fib_decr_nh_ref_count (p_nh_node);

            SAI_ROUTE_LOG_TRACE ("After decrementing, ref count: %d.",
                                 p_nh_node->ref_count);
        }
    } else if (p_route_node->nh_type == SAI_OBJECT_TYPE_NEXT_HOP_GROUP) {
        p_grp_node = p_route_node->nh_info.group_node;

        if (p_grp_node) {
            sai_fib_decr_nh_group_ref_count (p_grp_node);

            SAI_ROUTE_LOG_TRACE ("After decrementing, ref count: %d.",
                                 p_grp_node->ref_count);
        }
    }
}

static sai_status_t sai_fib_uc_route_entry_validate (
const sai_route_entry_t *uc_route_entry)
{
    STD_ASSERT (uc_route_entry != NULL);

    if ((uc_route_entry->destination.addr_family != SAI_IP_ADDR_FAMILY_IPV4) &&
        (uc_route_entry->destination.addr_family != SAI_IP_ADDR_FAMILY_IPV6)) {
        SAI_ROUTE_LOG_ERR ("%d is not a valid value for IP address family.",
                           uc_route_entry->destination.addr_family);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_route_input_params_validate (
const sai_route_entry_t *uc_route_entry, uint_t attr_count)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

   SAI_ROUTE_LOG_TRACE ("SAI Route creation parameters validate, "
                        "attribute count: %d.", attr_count);

    sai_rc = sai_fib_uc_route_entry_validate (uc_route_entry);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ROUTE_LOG_ERR ("sai_route_entry_t validation failed.");

        return sai_rc;
    }

    if ((attr_count == 0) || (attr_count > SAI_FIB_ROUTE_MAX_ATTR_COUNT)) {
        SAI_ROUTE_LOG_ERR ("Attribute count is invalid. Expected count: "
                           "<1 - %d>.", SAI_FIB_ROUTE_MAX_ATTR_COUNT);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_route_nh_id_attr_set (sai_fib_route_t *p_route_node,
                                                  sai_object_id_t nh_id)
{
    sai_fib_nh_t *p_nh_node = NULL;

    p_nh_node = sai_fib_next_hop_node_get_from_id (nh_id);

    if (!p_nh_node) {
        SAI_ROUTE_LOG_ERR ("Next hop Id 0x%"PRIx64" does not exist.", nh_id);

        /*
         * Attribute index based return code will be recalculated by the
         * caller of this function.
         */
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_route_node->nh_type = SAI_OBJECT_TYPE_NEXT_HOP;
    p_route_node->nh_info.nh_node = p_nh_node;

   SAI_ROUTE_LOG_TRACE ("Route set with Next hop Id attribute value 0x%"PRIx64".",
                        nh_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_route_nh_grp_id_attr_set (
sai_fib_route_t *p_route_node, sai_object_id_t nh_grp_id)
{
    sai_fib_nh_group_t *p_nh_grp_node = NULL;

    p_nh_grp_node = sai_fib_next_hop_group_get (nh_grp_id);

    if (!p_nh_grp_node) {
        SAI_ROUTE_LOG_ERR ("Next Hop Group Id 0x%"PRIx64" does not exist.",
                           nh_grp_id);

        /*
         * Attribute index based return code will be recalculated by the
         * caller of this function.
         */
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_route_node->nh_type = SAI_OBJECT_TYPE_NEXT_HOP_GROUP;
    p_route_node->nh_info.group_node = p_nh_grp_node;

    SAI_ROUTE_LOG_TRACE ("Route set with Next Hop Group Id attribute value "
                         "0x%"PRIx64".", nh_grp_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_route_pkt_action_attr_set (
sai_fib_route_t *p_route_node, uint_t pkt_action)
{
    if (!(sai_packet_action_validate (pkt_action))) {
        SAI_ROUTE_LOG_ERR ("Packet Action attribute value %d is not valid.",
                           pkt_action);

        /*
         * Attribute index based return code will be recalculated by the
         * caller of this function.
         */
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_route_node->packet_action = pkt_action;

    SAI_ROUTE_LOG_TRACE ("Route set with packet action attribute value %d (%s).",
                         pkt_action, sai_packet_action_str (pkt_action));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_route_attributes_parse (
sai_fib_route_t *p_route_node, uint_t attr_count,
const sai_attribute_t *attr_list)
{
    sai_status_t           sai_rc = SAI_STATUS_SUCCESS;
    const sai_attribute_t *p_attr = NULL;
    uint_t                 list_idx = 0;
    sai_object_type_t      nh_type;
    sai_attribute_t        sai_attr_get_range;

    STD_ASSERT (p_route_node != NULL);

    STD_ASSERT (attr_list != NULL);

    SAI_ROUTE_LOG_TRACE ("Parsing attributes for Route create/Set Attributes, "
                         "attribute count: %d.", attr_count);

    for (list_idx = 0; list_idx < attr_count; list_idx++)
    {
        p_attr = &attr_list [list_idx];

        SAI_ROUTE_LOG_TRACE ("Parsing attr_list [%d], Attribute id: %d.",
                             list_idx, p_attr->id);

        switch (p_attr->id) {
            case SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID:
                nh_type = sai_object_type_query (p_attr->value.oid);

                if (nh_type == SAI_OBJECT_TYPE_NEXT_HOP) {

                    sai_rc = sai_fib_route_nh_id_attr_set (p_route_node,
                                                           p_attr->value.oid);

                } else if (nh_type == SAI_OBJECT_TYPE_NEXT_HOP_GROUP) {

                    sai_rc =
                        sai_fib_route_nh_grp_id_attr_set (p_route_node,
                                                          p_attr->value.oid);

                } else {
                    sai_rc = SAI_STATUS_INVALID_ATTR_VALUE_0;
                }

                break;

            case SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION:
                sai_rc = sai_fib_route_pkt_action_attr_set (p_route_node,
                                                            p_attr->value.s32);

                break;

            case SAI_ROUTE_ENTRY_ATTR_TRAP_PRIORITY:
                p_route_node->trap_priority = p_attr->value.u8;
                break;

            case SAI_ROUTE_ENTRY_ATTR_META_DATA:
                memset(&sai_attr_get_range, 0, sizeof(sai_attribute_t));

                sai_attr_get_range.id = SAI_SWITCH_ATTR_ROUTE_DST_USER_META_DATA_RANGE;
                sai_rc = sai_acl_npu_api_get()->get_acl_attribute(&sai_attr_get_range);

                if (sai_rc != SAI_STATUS_SUCCESS) {
                    SAI_ROUTE_LOG_ERR ("Failed to fetch L3 Route Meta Data range");
                    break;
                }

                /* Validate the L3 Route Meta data range */
                if ((p_attr->value.u32 < sai_attr_get_range.value.u32range.min) ||
                    (p_attr->value.u32 > sai_attr_get_range.value.u32range.max)) {
                    sai_rc = SAI_STATUS_INVALID_ATTR_VALUE_0;
                    break;
                }

                p_route_node->meta_data = p_attr->value.u32;
                break;

            default:
                SAI_ROUTE_LOG_ERR ("Attribute id: %d is not a known attribute "
                                   "for Route.", p_attr->id);

                sai_rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
        }

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ROUTE_LOG_ERR ("Attribute id: %d passed on list index: %d "
                               "is not valid.", p_attr->id, list_idx);

            return (sai_fib_attr_status_code_get (sai_rc, list_idx));
        }
    }

    if (((p_route_node->packet_action == SAI_PACKET_ACTION_FORWARD) ||
         (p_route_node->packet_action == SAI_PACKET_ACTION_LOG)) &&
        (p_route_node->nh_type == SAI_FIB_ROUTE_NH_TYPE_NONE)) {
        SAI_ROUTE_LOG_ERR ("Route Packet action is %s, but Next hop/"
                           "Next hop group ID attribute is not provided.",
                           sai_packet_action_str (p_route_node->packet_action));

        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    return SAI_STATUS_SUCCESS;
}

static uint_t sai_fib_route_entry_prefix_len_get (
const sai_route_entry_t *p_uc_route)
{
    uint_t                 prefix_len = 0;
    const sai_ip_prefix_t *p_ip_prefix = NULL;

    STD_ASSERT(p_uc_route != NULL);

    p_ip_prefix = &p_uc_route->destination;

    if (p_ip_prefix->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
        prefix_len = std_ip_v4_mask_to_prefix_len (ntohl(p_ip_prefix->mask.ip4));
    } else if (p_ip_prefix->addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
        prefix_len =
            std_ip_v6_mask_to_prefix_len ((uint8_t *)p_ip_prefix->mask.ip6,
                                          HAL_INET6_LEN);
    }

    return prefix_len;
}

static void sai_fib_route_entry_ip_prefix_fill (
const sai_route_entry_t *p_uc_route, sai_ip_address_t *p_ip_addr,
uint_t *prefix_len)
{
    const sai_ip_prefix_t *p_ip_prefix = NULL;

    STD_ASSERT(p_uc_route != NULL);

    STD_ASSERT(p_ip_addr != NULL);

    STD_ASSERT(prefix_len != NULL);

    memset (p_ip_addr, 0, sizeof (sai_ip_address_t));

    p_ip_prefix = &p_uc_route->destination;

    p_ip_addr->addr_family = p_ip_prefix->addr_family;

    if (p_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
        p_ip_addr->addr.ip4 = p_ip_prefix->addr.ip4;
    } else if (p_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
        memcpy (p_ip_addr->addr.ip6, p_ip_prefix->addr.ip6, sizeof (sai_ip6_t));
    }

    (*prefix_len) = sai_fib_route_entry_prefix_len_get (p_uc_route);
}

static sai_fib_route_t *sai_fib_route_node_get (
const sai_route_entry_t *p_uc_route)
{
    sai_ip_address_t  ip_addr;
    uint_t            prefix_len = 0;
    sai_fib_route_t  *p_route_node = NULL;
    sai_fib_vrf_t    *p_vrf_node = NULL;
    uint_t            key_len = 0;

    STD_ASSERT(p_uc_route != NULL);

    p_vrf_node = sai_fib_vrf_node_get (p_uc_route->vr_id);

    if (!p_vrf_node) {
        SAI_ROUTE_LOG_ERR ("VRF ID 0x%"PRIx64" does not exist in VRF tree.",
                           p_uc_route->vr_id);

        return p_route_node;
    }

    sai_fib_route_entry_ip_prefix_fill (p_uc_route, &ip_addr, &prefix_len);

    key_len = sai_fib_route_key_len_get (prefix_len);

    p_route_node =
        (sai_fib_route_t *) std_radix_getexact (p_vrf_node->sai_route_tree,
                                                (uint8_t *)&ip_addr, key_len);

    return p_route_node;
}

static void sai_fib_route_node_init (
sai_fib_route_t *p_route_node, const sai_route_entry_t *p_uc_route)
{
    memset (p_route_node, 0, sizeof (sai_fib_route_t));

    p_route_node->vrf_id = p_uc_route->vr_id;

    sai_fib_route_entry_ip_prefix_fill (p_uc_route, &p_route_node->key.prefix,
                                        &p_route_node->prefix_len);

    p_route_node->nh_type = SAI_FIB_ROUTE_NH_TYPE_NONE;
    p_route_node->packet_action = SAI_FIB_ROUTE_DFLT_PKT_ACTION;
    p_route_node->trap_priority = SAI_FIB_ROUTE_DFLT_TRAP_PRIO;

    std_dll_init (&p_route_node->dep_encap_nh_list);
}

static sai_status_t sai_fib_route_node_insert_to_tree (
sai_fib_route_t *p_route_node, std_rt_table *sai_route_tree)
{
    uint_t key_len = 0;

    STD_ASSERT(p_route_node != NULL);

    STD_ASSERT(sai_route_tree != NULL);

    p_route_node->rt_head.rth_addr = (u_char *) (&p_route_node->key);

    key_len = sai_fib_route_key_len_get (p_route_node->prefix_len);

    if (!(std_radix_insert (sai_route_tree, (std_rt_head *)&p_route_node->rt_head,
                            key_len))) {
        sai_fib_route_log_error (p_route_node,
                                 "Failed to insert Route node into tree");

        sai_fib_route_node_free (p_route_node);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static inline bool sai_fib_is_default_route_entry (
const sai_route_entry_t *uc_route)
{
    sai_ip_address_t  ip_addr;
    uint_t            prefix_len;

    sai_fib_route_entry_ip_prefix_fill (uc_route, &ip_addr, &prefix_len);

    return (sai_fib_is_ip_addr_zero (&ip_addr) && (prefix_len == 0));
}

/* IPv4 route prefix and mask is expected in Network Byte Order */
static sai_status_t sai_fib_route_create (
const sai_route_entry_t *uc_route_entry, uint32_t attr_count,
const sai_attribute_t *attr_list)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_fib_route_t *p_route_node = NULL;
    sai_fib_vrf_t   *p_vrf_node = NULL;
    sai_ip_address_t ip_addr;
    bool             is_dflt_route_node = false;
    bool             is_route_alloc = false;
    sai_fib_route_t  old_route_info;

    STD_ASSERT (uc_route_entry != NULL);
    STD_ASSERT (attr_list != NULL);

    memset (&ip_addr, 0, sizeof (sai_ip_address_t));

    sai_rc = sai_fib_route_input_params_validate (uc_route_entry, attr_count);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ROUTE_LOG_ERR ("Input paramters validation failed for Route entry.");

        return sai_rc;
    }

    sai_fib_lock ();

    do {
        p_vrf_node = sai_fib_vrf_node_get (uc_route_entry->vr_id);

        if (!p_vrf_node) {
            SAI_ROUTE_LOG_ERR ("VRF ID 0x%"PRIx64" does not exist.",
                               uc_route_entry->vr_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }


        if ((p_route_node = sai_fib_route_node_get (uc_route_entry)) != NULL) {

            if (!sai_fib_is_default_route_entry (uc_route_entry)) {

                sai_fib_route_log_error (p_route_node,
                                         "Route Node is already existing");

                sai_rc = SAI_STATUS_ITEM_ALREADY_EXISTS;
                break;

            } else {

                if (p_route_node->hw_info != NULL) {

                    sai_fib_route_log_error (p_route_node, "Default Route "
                                             "Node is already created");

                    sai_rc = SAI_STATUS_ITEM_ALREADY_EXISTS;
                    break;
                }

                memcpy (&old_route_info, p_route_node, sizeof (sai_fib_route_t));

                sai_fib_route_log_trace (p_route_node,
                                         "Default Route is being created.");
                p_route_node->packet_action = SAI_FIB_ROUTE_DFLT_PKT_ACTION;

                is_dflt_route_node = true;
            }
        }

        if (p_route_node == NULL) {

            p_route_node = sai_fib_route_node_alloc ();

            if (!p_route_node) {
                SAI_ROUTE_LOG_ERR ("Failed to allocate memory for Route Node");

                sai_rc = SAI_STATUS_NO_MEMORY;
                break;
            }

            is_route_alloc = true;

            sai_fib_route_node_init (p_route_node, uc_route_entry);
        }

        sai_fib_route_log_trace (p_route_node, "Route Info before parsing");

        sai_rc = sai_fib_route_attributes_parse (p_route_node, attr_count,
                                                 attr_list);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            sai_fib_route_log_error (p_route_node,
                                     "Failed to parse input Route attributes");
            break;
        }

        sai_fib_route_log_trace (p_route_node, "Parsing attributes successful");

        sai_rc = sai_route_npu_api_get()->route_create (p_route_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            sai_fib_route_log_error (p_route_node, "Failed to create route");

            break;
        }

        sai_fib_route_log_trace (p_route_node, "Created Route in NPU");

        if (is_route_alloc) {

            sai_rc = sai_fib_route_node_insert_to_tree (p_route_node,
                                                        p_vrf_node->sai_route_tree);

            if (sai_rc != SAI_STATUS_SUCCESS) {
                break;
            }
        }

        sai_fib_route_affected_encap_nh_update (p_route_node, SAI_OP_CREATE);

        sai_fib_route_nh_ref_count_incr (p_route_node);

        sai_fib_encap_nh_dep_route_add (p_route_node);

    } while (0);

    sai_fib_unlock ();

    if (sai_rc == SAI_STATUS_SUCCESS) {
       SAI_ROUTE_LOG_INFO ("Route Add success");
    } else {
        if ((p_route_node != NULL) && (is_route_alloc)) {
            sai_fib_route_node_free (p_route_node);
        }
        if (is_dflt_route_node) {
            memcpy (p_route_node, &old_route_info, sizeof (sai_fib_route_t));
        }
        SAI_ROUTE_LOG_ERR ("Route Add failed.");
    }

    return sai_rc;
}

/* IPv4 route prefix and mask is expected in Network Byte Order */
static sai_status_t sai_fib_route_remove (
const sai_route_entry_t *uc_route_entry)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_fib_route_t *p_route_node = NULL;
    sai_fib_vrf_t   *p_vrf_node = NULL;

   SAI_ROUTE_LOG_TRACE ("SAI Route removal.");

    STD_ASSERT (uc_route_entry != NULL);

    sai_rc = sai_fib_uc_route_entry_validate (uc_route_entry);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ROUTE_LOG_ERR ("sai_route_entry_t validation failed.");

        return sai_rc;
    }

    sai_fib_lock ();

    do {
        p_vrf_node = sai_fib_vrf_node_get (uc_route_entry->vr_id);

        if (!p_vrf_node) {
            SAI_ROUTE_LOG_ERR ("VRF ID 0x%"PRIx64" does not exist in VRF tree.",
                               uc_route_entry->vr_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        p_route_node = sai_fib_route_node_get (uc_route_entry);

        if (!p_route_node) {
            SAI_ROUTE_LOG_ERR ("Route entry does not exist.");

            sai_rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        sai_fib_route_log_trace (p_route_node, "Route to be removed in NPU");

        sai_rc = sai_route_npu_api_get()->route_remove (p_route_node);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            sai_fib_route_log_error (p_route_node,
                                     "Failed to remove Route in NPU");

            break;
        }

        if (!sai_fib_is_default_route_entry (uc_route_entry)) {

            std_radix_remove (p_vrf_node->sai_route_tree,
                              (std_rt_head *)&p_route_node->rt_head);
        } else {
            p_route_node->packet_action = SAI_PACKET_ACTION_DROP;
        }

        sai_fib_route_affected_encap_nh_update (p_route_node, SAI_OP_REMOVE);

        sai_fib_encap_nh_dep_route_remove (p_route_node);

        sai_fib_route_nh_ref_count_decr (p_route_node);

        p_route_node->nh_type = SAI_FIB_ROUTE_NH_TYPE_NONE;

        if (!sai_fib_is_default_route_entry (uc_route_entry)) {

            sai_fib_route_node_free (p_route_node);
        }

    } while (0);

    sai_fib_unlock ();

    if (sai_rc == SAI_STATUS_SUCCESS) {
       SAI_ROUTE_LOG_INFO ("SAI Route removal successful.");
    } else {
        SAI_ROUTE_LOG_ERR ("SAI Route removal failed.");
    }

    return sai_rc;
}

static sai_status_t sai_fib_route_attribute_set (
const sai_route_entry_t *uc_route_entry, const sai_attribute_t *attr)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_fib_route_t  route_node_in;
    sai_fib_route_t *p_route_node = NULL;
    sai_fib_vrf_t   *p_vrf_node = NULL;
    uint_t           attr_count = 1;
    bool             nh_info_set = false;

   SAI_ROUTE_LOG_TRACE ("Setting Route attribute");

    STD_ASSERT (uc_route_entry != NULL);
    STD_ASSERT (attr != NULL);

    sai_rc = sai_fib_route_input_params_validate (uc_route_entry, attr_count);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ROUTE_LOG_ERR ("Input paramters validation failed for Route entry.");

        return sai_rc;
    }

    sai_fib_lock ();

    do {
        p_vrf_node = sai_fib_vrf_node_get (uc_route_entry->vr_id);

        if (!p_vrf_node) {
            SAI_ROUTE_LOG_ERR ("VRF ID 0x%"PRIx64" does not exist in VRF tree.",
                               uc_route_entry->vr_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        p_route_node = sai_fib_route_node_get (uc_route_entry);

        if (!p_route_node) {
            SAI_ROUTE_LOG_ERR ("Route entry does not exist.");

            sai_rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        memcpy (&route_node_in, p_route_node, sizeof (sai_fib_route_t));

        sai_fib_route_log_trace (&route_node_in,
                                 "Existing Route Info before parsing");

        sai_rc = sai_fib_route_attributes_parse (&route_node_in, attr_count,
                                                 attr);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            sai_fib_route_log_error (&route_node_in,
                                     "Failed to parse input Route attributes");

            break;
        }

        sai_fib_route_log_trace (&route_node_in, "Parsing attributes success");

        sai_rc = sai_route_npu_api_get()->route_attr_set (&route_node_in, attr_count,
                                                          attr);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            sai_fib_route_log_error (&route_node_in,
                                     "Failed to Set/Modify Route in NPU");

            break;
        }

        if (!(sai_fib_route_is_nh_info_match (p_route_node, &route_node_in))) {
            SAI_ROUTE_LOG_TRACE ("NH info change.");

            sai_fib_route_nh_ref_count_decr (p_route_node);

            sai_fib_encap_nh_dep_route_remove (p_route_node);

            nh_info_set = true;
        }

        sai_fib_route_attr_set_affected_encap_nh_update (p_route_node,
                                                         &route_node_in);
        memcpy (p_route_node, &route_node_in, sizeof (sai_fib_route_t));

        if (nh_info_set) {

            sai_fib_route_nh_ref_count_incr (p_route_node);

            sai_fib_encap_nh_dep_route_add (p_route_node);
        }

    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        sai_fib_route_log_trace (p_route_node,
                                 "Setting Route attributes successful");
    } else {
        SAI_ROUTE_LOG_ERR ("Setting Route attributes failed.");
    }

    sai_fib_unlock ();

    return sai_rc;
}

static sai_status_t sai_fib_route_attribute_get (
const sai_route_entry_t *uc_route_entry, uint32_t attr_count,
sai_attribute_t *attr_list)
{
    sai_status_t     sai_rc = SAI_STATUS_FAILURE;
    sai_fib_route_t *p_route_node = NULL;

    SAI_ROUTE_LOG_TRACE ("SAI Route attributes get");

    if ((!attr_count)) {

        SAI_ROUTE_LOG_ERR ("SAI Route Get attribute. Invalid input."
                           " attr_count: %d.", attr_count);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT (uc_route_entry != NULL);
    STD_ASSERT (attr_list != NULL);

    sai_fib_lock ();

    do {
        p_route_node = sai_fib_route_node_get (uc_route_entry);

        if (!p_route_node) {
            SAI_ROUTE_LOG_ERR ("Route entry does not exist.");

            sai_rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        sai_rc = sai_route_npu_api_get()->route_attr_get (p_route_node, attr_count,
                                              attr_list);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            sai_fib_route_log_error (p_route_node,
                                     "Failed to Get Route attributes from NPU");

            break;
        }
    } while (0);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        sai_fib_route_log_trace (p_route_node,
                                 "Route attributes Get successful");
    } else {
        SAI_ROUTE_LOG_ERR ("Route attributes Get failed.");
    }

    sai_fib_unlock ();

    return sai_rc;
}

sai_status_t sai_fib_internal_default_route_node_add (sai_fib_vrf_t *p_vrf_node,
                                                      sai_ip_addr_family_t af)
{
    sai_status_t               status;
    sai_route_entry_t  route_entry;
    sai_fib_route_t           *p_route_node = NULL;

    memset (&route_entry, 0, sizeof (sai_route_entry_t));

    route_entry.vr_id = p_vrf_node->vrf_id;
    route_entry.destination.addr_family = af;

    p_route_node = sai_fib_route_node_alloc ();

    if (p_route_node == NULL) {
        SAI_ROUTER_LOG_ERR ("Failed to allocate memory for default route node");

        return SAI_STATUS_NO_MEMORY;
    }

    sai_fib_route_node_init (p_route_node, &route_entry);

    p_route_node->packet_action = SAI_PACKET_ACTION_DROP;

    status = sai_fib_route_node_insert_to_tree (p_route_node,
                                                p_vrf_node->sai_route_tree);

    if (status != SAI_STATUS_SUCCESS) {
        SAI_ROUTER_LOG_ERR ("Failed to insert default route node in route db");

        return status;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_fib_internal_default_route_node_del (sai_fib_vrf_t *p_vrf_node,
                                                      sai_ip_addr_family_t af)
{
    sai_route_entry_t  route_entry;
    sai_fib_route_t           *p_route_node = NULL;

    memset (&route_entry, 0, sizeof (sai_route_entry_t));

    route_entry.vr_id = p_vrf_node->vrf_id;
    route_entry.destination.addr_family = af;

    p_route_node = sai_fib_route_node_get (&route_entry);

    if (p_route_node == NULL) {
        SAI_ROUTER_LOG_ERR ("Internal default route node does not exist.");


        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    std_radix_remove (p_vrf_node->sai_route_tree,
                      (std_rt_head *)&p_route_node->rt_head);

    sai_fib_route_node_free (p_route_node);

    return SAI_STATUS_SUCCESS;
}

static sai_route_api_t sai_route_method_table = {
    sai_fib_route_create,
    sai_fib_route_remove,
    sai_fib_route_attribute_set,
    sai_fib_route_attribute_get
};

sai_route_api_t *sai_route_api_query (void)
{
    return &sai_route_method_table;
}
