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

/*
* @file sai_mirror_common.c
*
* @brief This file contains public API handling for the SAI mirror
*        functionality
*
*************************************************************************/
#include "saitypes.h"
#include "saistatus.h"

#include "sai_mirror_defs.h"
#include "sai_mirror_api.h"
#include "sai_npu_mirror.h"
#include "sai_mirror_api.h"
#include "sai_port_common.h"
#include "sai_port_utils.h"
#include "sai_common_utils.h"
#include "sai_switch_utils.h"
#include "sai_vlan_api.h"
#include "sai_l3_util.h"
#include "sai_lag_api.h"
#include "sai_mirror_util.h"

#include "std_rbtree.h"
#include "std_assert.h"
#include "std_type_defs.h"
#include "stdbool.h"
#include "std_struct_utils.h"
#include "sai_common_infra.h"

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

static rbtree_handle mirror_sessions_tree = NULL;

rbtree_handle sai_mirror_sessions_db_get (void)
{
    return mirror_sessions_tree;
}

static inline bool sai_mirror_is_span_type_valid (sai_mirror_session_type_t span_type)
{
    switch (span_type) {
        case SAI_MIRROR_SESSION_TYPE_LOCAL:
        case SAI_MIRROR_SESSION_TYPE_REMOTE:
        case SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE:
            return true;
        default:
            return false;
    }
    return false;
}

static inline uint32_t sai_mirror_span_bitmap (void)
{
    return (sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_MONITOR_PORT) |
            sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_TYPE));
}

static inline uint32_t sai_mirror_rspan_bitmap (void)
{
    return (sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_MONITOR_PORT) |
            sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_TYPE) |
            sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_VLAN_TPID) |
            sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_VLAN_ID) |
            sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_VLAN_PRI));
}

static inline uint32_t sai_mirror_erspan_bitmap (void)
{
    return (sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_MONITOR_PORT) |
            sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_TYPE) |
            sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_ERSPAN_ENCAPSULATION_TYPE) |
            sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION) |
            sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_TOS) |
            sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS) |
            sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS) |
            sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS)|
            sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS) |
            sai_attribute_bit_set(SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE));
}

static inline bool sai_mirror_is_mandat_attribute_missing (uint32_t req_attributes,
                                                           sai_mirror_session_type_t span_type)
{
    bool result = false;
    switch (span_type) {
        case SAI_MIRROR_SESSION_TYPE_LOCAL:
            /* Pick the mandatory attributes from the given list and check whether they are set */
            result = ((req_attributes & (sai_mirror_span_bitmap())) ^ (sai_mirror_span_bitmap()));
            break;
        case SAI_MIRROR_SESSION_TYPE_REMOTE:
            /* Pick the mandatory attributes from the given list and check whether they are set */
            result = ((req_attributes & (sai_mirror_rspan_bitmap())) ^ (sai_mirror_rspan_bitmap()));
            break;
        case SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE:
            /* Pick the mandatory attributes from the given list and check whether they are set */
            result = ((req_attributes & (sai_mirror_erspan_bitmap())) ^ (sai_mirror_erspan_bitmap()));
            break;
        default:
            return false;
    }

    return result;
}

static sai_status_t sai_mirror_fill_erspan_attribute_params (
                    sai_mirror_session_info_t *p_session_info,
                    const sai_attribute_t *attr_list,
                    uint32_t attr_index)
{
    sai_status_t error = SAI_STATUS_SUCCESS;

    switch (attr_list->id) {
        case SAI_MIRROR_SESSION_ATTR_ERSPAN_ENCAPSULATION_TYPE:
            p_session_info->session_params.encap_type = attr_list->value.s32;
            break;

        case SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION:
            p_session_info->session_params.ip_hdr_version = attr_list->value.u8;
            break;

        case SAI_MIRROR_SESSION_ATTR_TOS:
            p_session_info->session_params.tos = attr_list->value.u8;
            break;

        case SAI_MIRROR_SESSION_ATTR_TTL:
            p_session_info->session_params.ttl = attr_list->value.u8;
            break;

        case SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS:
            if (sai_fib_is_ip_addr_zero(&attr_list->value.ipaddr)) {
                SAI_MIRROR_LOG_ERR("Zero IP Address is not valid");
                error = SAI_STATUS_CODE((abs)SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index);
                break;
            }
            memcpy (&p_session_info->session_params.src_ip,
                    &attr_list->value.ipaddr, sizeof(p_session_info->session_params.src_ip));
            break;

        case SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS:
            if (sai_fib_is_ip_addr_zero(&attr_list->value.ipaddr)) {
                SAI_MIRROR_LOG_ERR("Zero IP Address is not valid");
                error = SAI_STATUS_CODE((abs)SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index);
                break;
            }
            memcpy (&p_session_info->session_params.dst_ip,
                    &attr_list->value.ipaddr, sizeof(p_session_info->session_params.dst_ip));
            break;

        case SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS:
            if (sai_fib_is_mac_address_zero(&attr_list->value.mac)) {
                SAI_MIRROR_LOG_ERR("Zero MAC Address is not valid");
                error = SAI_STATUS_CODE((abs)SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index);
                break;
            }
            memcpy (p_session_info->session_params.src_mac,
                    attr_list->value.mac, sizeof(p_session_info->session_params.src_mac));
            break;

        case SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS:
            if (sai_fib_is_mac_address_zero(&attr_list->value.mac)) {
                SAI_MIRROR_LOG_ERR("Zero MAC Address is not valid");
                error = SAI_STATUS_CODE((abs)SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index);
                break;
            }
            memcpy (p_session_info->session_params.dst_mac,
                    attr_list->value.mac, sizeof(p_session_info->session_params.dst_mac));
            break;

        case SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE:
           p_session_info->session_params.gre_protocol = attr_list->value.u16;
           break;

        default:
            error = SAI_STATUS_CODE((abs)SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + attr_index);
            SAI_MIRROR_LOG_ERR("Attribute id %d is not valid", attr_list->id);
    }

    return error;
}

static sai_status_t sai_mirror_fill_attribute_params (
            sai_mirror_session_info_t *p_session_info,
            const sai_attribute_t *attr_list,
            uint32_t attr_index)
{
    sai_status_t error = SAI_STATUS_SUCCESS;

    switch (attr_list->id) {
        case SAI_MIRROR_SESSION_ATTR_TYPE:
            p_session_info->span_type = attr_list->value.s32;
            if (!(sai_mirror_is_span_type_valid(p_session_info->span_type))) {
                SAI_MIRROR_LOG_ERR ("Mirror span type invalid span_type %d",
                                        p_session_info->span_type);
                error = SAI_STATUS_CODE((abs)SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index);
            }
            break;

        case SAI_MIRROR_SESSION_ATTR_TC:
            p_session_info->session_params.class_of_service = attr_list->value.u8;
            break;

        case SAI_MIRROR_SESSION_ATTR_VLAN_TPID:
            p_session_info->session_params.tpid = attr_list->value.u16;
            break;

        case SAI_MIRROR_SESSION_ATTR_VLAN_ID:
            if (!(sai_is_valid_vlan_id (attr_list->value.u16))) {
                SAI_MIRROR_LOG_ERR("Vlan id %u is not valid", attr_list->value.u16);
                error = SAI_STATUS_CODE((abs)SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index);
                break;
            }

            if (!(sai_is_vlan_created (attr_list->value.u16))) {
                SAI_MIRROR_LOG_ERR("Vlan id %u is not created", attr_list->value.u16);
                error = SAI_STATUS_CODE((abs)SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index);
                break;
            }

            p_session_info->session_params.vlan_id = attr_list->value.u16;
            break;

        case SAI_MIRROR_SESSION_ATTR_VLAN_PRI:
            p_session_info->session_params.vlan_priority = attr_list->value.u8;
            break;

        case SAI_MIRROR_SESSION_ATTR_ERSPAN_ENCAPSULATION_TYPE:
        case SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION:
        case SAI_MIRROR_SESSION_ATTR_TOS:
        case SAI_MIRROR_SESSION_ATTR_TTL:
        case SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS:
        case SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS:
        case SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS:
        case SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS:
        case SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE:
            error = sai_mirror_fill_erspan_attribute_params (p_session_info, attr_list, attr_index);
            break;

        case SAI_MIRROR_SESSION_ATTR_MONITOR_PORT:
            if (sai_is_obj_id_port (attr_list->value.oid)) {
                if(!sai_is_port_valid(attr_list->value.oid)) {
                    SAI_MIRROR_LOG_ERR("Port id 0x%"PRIx64" is not valid", attr_list->value.oid);
                    error = SAI_STATUS_CODE((abs)SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index);
                    break;
                }
            } else if (sai_is_obj_id_lag(attr_list->value.oid)) {
                if (!sai_is_lag_created(attr_list->value.oid)) {
                    SAI_MIRROR_LOG_ERR("Lag id 0x%"PRIx64" is not valid", attr_list->value.oid);
                    error = SAI_STATUS_CODE((abs)SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index);
                    break;
                }
            } else {
                SAI_MIRROR_LOG_ERR("Id 0x%"PRIx64" is not valid", attr_list->value.oid);
                error = SAI_STATUS_CODE((abs)SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index);
                break;
            }

            p_session_info->monitor_port = attr_list->value.oid;
            break;


        default:
            error = SAI_STATUS_CODE((abs)SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + attr_index);
            SAI_MIRROR_LOG_ERR("Attribute id %d is not valid", attr_list->id);
    }

    return error;
}

sai_status_t sai_mirror_session_create (sai_object_id_t *session_id,
                                        _In_ sai_object_id_t switch_id,
                                        uint32_t attr_count,
                                        const sai_attribute_t *attr_list)
{
    sai_mirror_session_info_t *p_session_info          = NULL;
    uint32_t                   validate_req_attributes = 0;
    uint32_t                   attr_index              = 0;
    sai_status_t               error                   = SAI_STATUS_SUCCESS;
    sai_npu_object_id_t        npu_object_id           = 0;

    STD_ASSERT (attr_list != NULL);
    STD_ASSERT (attr_count);

    sai_mirror_lock();

    do {
        if (!mirror_sessions_tree) {
            SAI_MIRROR_LOG_ERR ("Mirror initialization not done");
            error = SAI_STATUS_FAILURE;
            break;
        }

        p_session_info = sai_mirror_session_node_alloc ();
        if (!(p_session_info)) {
            SAI_MIRROR_LOG_ERR ("Memory allocation failed");
            error = SAI_STATUS_NO_MEMORY;
            break;
        }

        for (attr_index = 0; attr_index < attr_count; ++attr_index)
        {
            /* This is done to check if all the mandatory arguments are set */
            validate_req_attributes = validate_req_attributes |
                                      sai_attribute_bit_set(attr_list->id);

            error = sai_mirror_fill_attribute_params (p_session_info, attr_list, attr_index);

            /* Skip from NPU create if some attributes are invalid or has invalid values */
            if (error != SAI_STATUS_SUCCESS) {
                break;
            }

            error = sai_mirror_npu_api_get()->session_attribs_validate(p_session_info->session_id,
                    attr_list,
                    attr_index);

            /* Skip from NPU create if some attributes are not supported in npu */
            if (error != SAI_STATUS_SUCCESS) {
                SAI_MIRROR_LOG_ERR ("NPU Mirror session validate failed");
                break;
            }

            attr_list++;
        }

        if (error != SAI_STATUS_SUCCESS) {
            SAI_MIRROR_LOG_ERR ("Invalid attributes are set");
            break;
        }

        /* Check if all the mandatory attributes are set */
        if ((sai_mirror_is_mandat_attribute_missing(validate_req_attributes,
                                                    p_session_info->span_type))) {
            SAI_MIRROR_LOG_ERR ("Required attributes are not set");
            error = SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
            break;
        }


        if ((error = sai_mirror_npu_api_get()->session_create(p_session_info, &npu_object_id)) !=
                     SAI_STATUS_SUCCESS) {
            SAI_MIRROR_LOG_ERR ("Mirror session create failed");
            break;
        }

        p_session_info->session_id = sai_uoid_create (SAI_OBJECT_TYPE_MIRROR_SESSION, npu_object_id);

        if (std_rbtree_insert (mirror_sessions_tree, (void *) p_session_info) != STD_ERR_OK) {
            SAI_MIRROR_LOG_ERR ("Node insertion failed for session 0x%"PRIx64"",
                                                                p_session_info->session_id);
            sai_mirror_npu_api_get()->session_destroy (p_session_info->session_id);
            error = SAI_STATUS_FAILURE;
            break;
        }

        p_session_info->source_ports_tree = std_rbtree_create_simple ("source_ports_tree",
                0, sizeof(sai_mirror_port_info_t));

        if (p_session_info->source_ports_tree == NULL) {
            SAI_MIRROR_LOG_ERR ("Source port tree creations failed for session 0x%"PRIx64"",
                                                                p_session_info->session_id);
            std_rbtree_remove (mirror_sessions_tree, (void *) p_session_info);
            sai_mirror_npu_api_get()->session_destroy (p_session_info->session_id);
            error = SAI_STATUS_FAILURE;
            break;
        }

        *session_id = p_session_info->session_id;
    } while (0);

    if ((p_session_info) && (error != SAI_STATUS_SUCCESS)) {
        sai_mirror_session_node_free ((sai_mirror_session_info_t *)p_session_info);
    }
    sai_mirror_unlock();

    return error;
}

sai_status_t sai_mirror_session_remove (sai_object_id_t session_id)
{
    sai_mirror_session_info_t *p_session_info = NULL;
    sai_status_t               error          = SAI_STATUS_SUCCESS;

    if (!sai_is_obj_id_mirror_session (session_id)) {
        SAI_MIRROR_LOG_ERR ("0x%"PRIx64" is not a valid Mirror obj", session_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_mirror_lock();
    SAI_MIRROR_LOG_TRACE ("Destroy mirror session 0x%"PRIx64"", session_id);

    do {
        if (!mirror_sessions_tree) {
            SAI_MIRROR_LOG_ERR ("Mirror initialization not done");
            error = SAI_STATUS_FAILURE;
            break;
        }

        p_session_info = (sai_mirror_session_info_t *) std_rbtree_getexact (
                                     mirror_sessions_tree, (void *)&session_id);
        if (!(p_session_info)) {
            SAI_MIRROR_LOG_ERR ("Mirror session not found 0x%"PRIx64"", session_id);
            error = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        /* Check if any source port is still attached to the session */

        if (std_rbtree_getfirst(p_session_info->source_ports_tree) != NULL) {
            SAI_MIRROR_LOG_ERR ("Source ports are still attached to mirror session 0x%"PRIx64"",
                                 session_id);
            error = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        if ((error = sai_mirror_npu_api_get()->session_destroy (p_session_info->session_id)) !=
             SAI_STATUS_SUCCESS) {
            SAI_MIRROR_LOG_ERR ("Mirror session destroy failed 0x%"PRIx64"", session_id);
            break;
        }

        if (p_session_info->source_ports_tree) {
            std_rbtree_destroy (p_session_info->source_ports_tree);
        }

        if (std_rbtree_remove (mirror_sessions_tree , (void *)p_session_info) != p_session_info) {
            SAI_MIRROR_LOG_ERR ("Mirror session node remove failed 0x%"PRIx64"", session_id);
            error = SAI_STATUS_FAILURE;
            break;
        }

        sai_mirror_session_node_free (p_session_info);

    } while (0);

    sai_mirror_unlock();

    return error;
}

sai_status_t sai_mirror_session_port_add (sai_object_id_t session_id,
                                          sai_object_id_t mirror_port_id,
                                          sai_mirror_direction_t mirror_direction)
{
    sai_mirror_session_info_t *p_session_info = NULL;
    sai_mirror_port_info_t    *p_source_node  = NULL;
    sai_status_t               error          = SAI_STATUS_SUCCESS;
    sai_mirror_port_info_t     tmp_source_node;
    bool                       is_source_node_created = false;

    memset (&tmp_source_node, 0, sizeof(tmp_source_node));

    if (!sai_is_obj_id_mirror_session (session_id)) {
        SAI_MIRROR_LOG_ERR ("0x%"PRIx64" is not a valid Mirror obj", session_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_mirror_lock();
    SAI_MIRROR_LOG_TRACE ("Add port 0x%"PRIx64" to mirror session 0x%"PRIx64" in direction %d",
                           mirror_port_id, session_id, mirror_direction);
    do {
        if(!sai_is_port_valid(mirror_port_id)) {
            SAI_MIRROR_LOG_ERR("Port id 0x%"PRIx64" is not valid", mirror_port_id);
            error = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }
        if (!mirror_sessions_tree) {
            SAI_MIRROR_LOG_ERR ("Mirror initialization not done");
            error = SAI_STATUS_FAILURE;
            break;
        }

        p_session_info = (sai_mirror_session_info_t *) std_rbtree_getexact (
                                        mirror_sessions_tree, (void *)&session_id);
        if (p_session_info == NULL) {
            SAI_MIRROR_LOG_ERR ("Mirror Session not found 0x%"PRIx64"", session_id);
            error = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if (!p_session_info->source_ports_tree) {
            SAI_MIRROR_LOG_ERR ("Source ports tree initialization not done");
            error = SAI_STATUS_FAILURE;
            break;
        }

        tmp_source_node.mirror_port = mirror_port_id;
        tmp_source_node.mirror_direction   = mirror_direction;

        p_source_node = std_rbtree_getexact (p_session_info->source_ports_tree,
                                            (void *) &tmp_source_node);
        if (p_source_node == NULL) {
            p_source_node = sai_source_port_node_alloc();
            if (p_source_node == NULL) {
                SAI_MIRROR_LOG_ERR ("Memory allocation failed for source port 0x%"PRIx64"",
                        mirror_port_id);
                error = SAI_STATUS_NO_MEMORY;
                break;
            }
            is_source_node_created = true;
        } else {
            SAI_MIRROR_LOG_ERR ("Mirror port 0x%"PRIx64" already exists for direction %d",
                                mirror_port_id, mirror_direction);
            error = SAI_STATUS_ITEM_ALREADY_EXISTS;
            break;
        }

        p_source_node->mirror_direction = mirror_direction;
        p_source_node->mirror_port = mirror_port_id;

        if ((error = sai_mirror_npu_api_get()->session_port_add (session_id, mirror_port_id,
                            mirror_direction)) != SAI_STATUS_SUCCESS) {
            SAI_MIRROR_LOG_ERR ("Mirror Port addition failed for port 0x%"PRIx64"",
                    mirror_port_id);
            break;
        }

        if (std_rbtree_insert(p_session_info->source_ports_tree, (void *)p_source_node) !=
                             STD_ERR_OK) {
            sai_mirror_npu_api_get()->session_port_remove (p_session_info->session_id, mirror_port_id,
                                                mirror_direction);
            error = SAI_STATUS_FAILURE;
            break;
        }

    } while (0);

    if ((p_source_node) && (is_source_node_created) && (error != SAI_STATUS_SUCCESS)) {
        sai_source_port_node_free (p_source_node);
    }

    sai_mirror_unlock();
    return error;
}

sai_status_t sai_mirror_session_port_remove  (sai_object_id_t session_id,
                                              sai_object_id_t mirror_port_id,
                                              sai_mirror_direction_t mirror_direction)
{
    sai_mirror_session_info_t *p_session_info = NULL;
    sai_mirror_port_info_t    *p_source_node  = NULL;
    sai_status_t               error          = SAI_STATUS_SUCCESS;
    sai_mirror_port_info_t    tmp_source_node;

    memset (&tmp_source_node, 0, sizeof(tmp_source_node));

    if (!sai_is_obj_id_mirror_session (session_id)) {
        SAI_MIRROR_LOG_ERR ("0x%"PRIx64" is not a valid Mirror obj", session_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_mirror_lock();
    SAI_MIRROR_LOG_TRACE ("Remove port 0x%"PRIx64" from mirror session 0x%"PRIx64" for direction %d",
                           mirror_port_id, session_id, mirror_direction);

    do {
        if(!(sai_is_port_valid(mirror_port_id))) {
            SAI_MIRROR_LOG_ERR("Port id 0x%"PRIx64" is not valid", mirror_port_id);
            error = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if (!mirror_sessions_tree) {
            SAI_MIRROR_LOG_ERR ("Mirror initialization not done");
            error = SAI_STATUS_FAILURE;
            break;
        }

        p_session_info = (sai_mirror_session_info_t *) std_rbtree_getexact (
                                  mirror_sessions_tree, (void *)&session_id);
        if (p_session_info == NULL) {
            SAI_MIRROR_LOG_ERR ("Mirror session not found 0x%"PRIx64"", session_id);
            error = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if (!p_session_info->source_ports_tree) {
            SAI_MIRROR_LOG_ERR ("Source ports tree initialization not done");
            error = SAI_STATUS_FAILURE;
            break;
        }

        tmp_source_node.mirror_port = mirror_port_id;
        tmp_source_node.mirror_direction   = mirror_direction;

        p_source_node = std_rbtree_getexact (p_session_info->source_ports_tree,
                                            (void *) &tmp_source_node);
        if (p_source_node == NULL) {
            SAI_MIRROR_LOG_ERR ("Mirror port not found 0x%"PRIx64"", mirror_port_id);
            error = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        if ((error = sai_mirror_npu_api_get()->session_port_remove (session_id, mirror_port_id,
                            mirror_direction)) != SAI_STATUS_SUCCESS) {
            SAI_MIRROR_LOG_ERR ("Mirror port 0x%"PRIx64" removal from session 0x%"PRIx64" failed",
                                   mirror_port_id, session_id);
            break;
        }

        if (std_rbtree_remove (p_session_info->source_ports_tree, (void *)p_source_node) !=
                p_source_node) {
            SAI_MIRROR_LOG_ERR ("Mirror port node remove failed 0x%"PRIx64"", mirror_port_id);
            error = SAI_STATUS_FAILURE;
            break;
        }

        sai_source_port_node_free (p_source_node);

    } while (0);

    sai_mirror_unlock();

    return error;
}

sai_status_t sai_mirror_session_attribute_set (sai_object_id_t session_id,
                                               const sai_attribute_t *attr)
{
    sai_status_t error = SAI_STATUS_SUCCESS;
    uint32_t attr_index = 0 ;
    sai_mirror_session_info_t *p_session_info = NULL;
    sai_mirror_session_info_t p_orig_session_info;

    STD_ASSERT (attr != NULL);

    if (!sai_is_obj_id_mirror_session (session_id)) {
        SAI_MIRROR_LOG_ERR ("0x%"PRIx64" is not a valid Mirror obj", session_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    SAI_MIRROR_LOG_TRACE ("Attribute set for session_id 0x%"PRIx64"", session_id);

    sai_mirror_lock();

    do {

        p_session_info = (sai_mirror_session_info_t *) std_rbtree_getexact (
                                  mirror_sessions_tree, (void *)&session_id);
        if (p_session_info == NULL) {
            SAI_MIRROR_LOG_ERR ("Mirror session not found 0x%"PRIx64"", session_id);
            error = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        memcpy (&p_orig_session_info, p_session_info, sizeof(sai_mirror_session_info_t));

        if ((error = sai_mirror_fill_attribute_params (p_session_info, attr, attr_index)) !=
             SAI_STATUS_SUCCESS) {
            SAI_MIRROR_LOG_ERR ("Mirror fill attribute parameters failed %d", error);
            break;
        }

        error = sai_mirror_npu_api_get()->session_set (session_id, p_session_info->span_type, attr);

        if (error != SAI_STATUS_SUCCESS) {
            SAI_MIRROR_LOG_ERR ("NPU Attribute set failed %d", error);
            memcpy (p_session_info, &p_orig_session_info, sizeof(sai_mirror_session_info_t));
            break;
        }

    } while (0);

    sai_mirror_unlock();

    return error;
}

sai_status_t sai_mirror_session_attribute_get (sai_object_id_t session_id,
                                               uint32_t attr_count,
                                               sai_attribute_t *attr_list)
{
    sai_status_t error = SAI_STATUS_SUCCESS;
    sai_mirror_session_info_t *p_session_info = NULL;

    STD_ASSERT (attr_list != NULL);
    STD_ASSERT (attr_count);

    if (!sai_is_obj_id_mirror_session (session_id)) {
        SAI_MIRROR_LOG_ERR ("0x%"PRIx64" is not a valid Mirror obj", session_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_mirror_lock();
    SAI_MIRROR_LOG_TRACE ("Attribute get for session_id 0x%"PRIx64"", session_id);

    do {

        p_session_info = (sai_mirror_session_info_t *) std_rbtree_getexact (
                mirror_sessions_tree, (void *)&session_id);

        if (p_session_info == NULL) {
            SAI_MIRROR_LOG_ERR ("Mirror session not found 0x%"PRIx64"", session_id);
            error = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        error = sai_mirror_npu_api_get()->session_get (session_id, p_session_info->span_type,
                                            attr_count, attr_list);

    } while (0);

    sai_mirror_unlock();
    return error;
}

sai_status_t sai_mirror_init (void)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    sai_mirror_mutex_lock_init ();

    mirror_sessions_tree = std_rbtree_create_simple ("mirror_sessions_tree",
                           STD_STR_OFFSET_OF(sai_mirror_session_info_t, session_id),
                           STD_STR_SIZE_OF(sai_mirror_session_info_t, session_id));

    if (mirror_sessions_tree == NULL) {
        SAI_MIRROR_LOG_ERR ("Mirror sessions tree create failed");
        return SAI_STATUS_FAILURE;
    }

    ret = sai_mirror_npu_api_get()->mirror_init();

    if (ret != SAI_STATUS_SUCCESS) {
        SAI_MIRROR_LOG_ERR ("Mirror NPU Initialization failed %d", ret);
    }

    return ret;
}

bool sai_mirror_is_valid_mirror_session (sai_object_id_t session_id)
{
    sai_mirror_lock();

    if (!(std_rbtree_getexact (
                    mirror_sessions_tree, (void *)&session_id))) {
        SAI_MIRROR_LOG_ERR ("Mirror session not found 0x%"PRIx64"", session_id);
        return false;
    }

    sai_mirror_unlock();

    return true;
}

static sai_mirror_api_t sai_mirror_method_table =
{
    sai_mirror_session_create,
    sai_mirror_session_remove,
    sai_mirror_session_attribute_set,
    sai_mirror_session_attribute_get,
};

sai_mirror_api_t* sai_mirror_api_query(void)
{
    return (&sai_mirror_method_table);
}
