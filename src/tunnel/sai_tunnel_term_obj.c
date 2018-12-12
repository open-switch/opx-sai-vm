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
* @file sai_tunnel_term_obj.c
*
* @brief This file contains function definitions for the SAI tunnel
*        termination table entry API functions.
*
*************************************************************************/
#include "saistatus.h"
#include "saitypes.h"
#include "saitunnel.h"
#include "sai_tunnel_api_utils.h"
#include "sai_oid_utils.h"
#include "sai_common_infra.h"
#include "sai_tunnel.h"
#include "sai_tunnel_util.h"
#include "sai_l3_util.h"
#include "std_assert.h"
#include "std_llist.h"
#include <inttypes.h>
#include <stdlib.h>

static inline dn_sai_tunnel_term_entry_t *dn_sai_tunnel_term_object_alloc (void)
{
    return ((dn_sai_tunnel_term_entry_t *) calloc (1,
             sizeof (dn_sai_tunnel_term_entry_t)));
}

static inline void dn_sai_tunnel_term_object_free (
                                       dn_sai_tunnel_term_entry_t *tunnel_term)
{
    free ((void *) tunnel_term);
}

static sai_status_t dn_sai_tunnel_term_ip_set (
                                     dn_sai_tunnel_term_entry_t *p_tunnel_term,
                                     const sai_attribute_t  *p_attr)
{
    char   ip_addr_str [SAI_FIB_MAX_BUFSZ];

    if ((p_attr->value.ipaddr.addr_family != SAI_IP_ADDR_FAMILY_IPV4) &&
        (p_attr->value.ipaddr.addr_family != SAI_IP_ADDR_FAMILY_IPV6)) {

        SAI_TUNNEL_LOG_ERR ("Invalid IP addr family: %d.",
                             p_attr->value.ipaddr.addr_family);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    if (p_attr->id == SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP) {
        sai_fib_ip_addr_copy (&p_tunnel_term->src_ip, &p_attr->value.ipaddr);
        SAI_TUNNEL_LOG_DEBUG ("Tunnel Termination Source IP set to %s.",
                              sai_ip_addr_to_str (&p_attr->value.ipaddr,
                               ip_addr_str, SAI_FIB_MAX_BUFSZ));
    } else {
        sai_fib_ip_addr_copy (&p_tunnel_term->dst_ip, &p_attr->value.ipaddr);
        SAI_TUNNEL_LOG_DEBUG ("Tunnel Termination Dest IP set to %s.",
                              sai_ip_addr_to_str (&p_attr->value.ipaddr,
                               ip_addr_str, SAI_FIB_MAX_BUFSZ));
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_tunnel_term_tunnel_id_set (
                                    dn_sai_tunnel_term_entry_t *p_tunnel_term,
                                    const sai_attribute_t  *p_attr)
{
    if (dn_sai_tunnel_obj_get (p_attr->value.oid) == NULL) {

        SAI_TUNNEL_LOG_ERR ("Invalid Tunnel object Id: 0x%"PRIx64".",
                            p_attr->value.oid);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_tunnel_term->tunnel_id = p_attr->value.oid;

    SAI_TUNNEL_LOG_DEBUG ("Tunnel Termination entry Tunnel Id set to 0x%"PRIx64".",
                          p_attr->value.oid);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_tunnel_term_attr_set (
                                    dn_sai_tunnel_term_entry_t *p_tunnel_term,
                                    uint_t attr_count,
                                    const sai_attribute_t *attr_list)
{
    uint_t                  attr_idx;
    sai_status_t            status;
    const sai_attribute_t  *p_attr;

    for (attr_idx = 0, p_attr = attr_list; (attr_idx < attr_count);
         ++attr_idx, ++p_attr) {

        SAI_TUNNEL_LOG_DEBUG ("Setting Tunnel Term attr id: %d at list index: %u.",
                              p_attr->id, attr_idx);

        status = SAI_STATUS_SUCCESS;

        switch (p_attr->id) {

            case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_VR_ID:
                p_tunnel_term->vr_id = p_attr->value.oid;
                break;

            case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TYPE:
                p_tunnel_term->type = p_attr->value.s32;
                break;

            case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_DST_IP:
            case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP:
                status = dn_sai_tunnel_term_ip_set (p_tunnel_term, p_attr);
                break;

            case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TUNNEL_TYPE:
                p_tunnel_term->tunnel_type = p_attr->value.s32;
                break;

            case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID:
                status = dn_sai_tunnel_term_tunnel_id_set (p_tunnel_term, p_attr);
                break;

            default:
                status = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }

        if (status != SAI_STATUS_SUCCESS) {

            SAI_TUNNEL_LOG_ERR ("Failure in Tunnel attr list. Index: %d, "
                                "Attribute Id: %d, Error: %d.", attr_idx,
                                p_attr->id, status);

            return (sai_get_indexed_ret_val (status, attr_idx));
        }
    }

    return SAI_STATUS_SUCCESS;
}

static void dn_sai_tunnel_term_add_to_tunnel_list (
                                dn_sai_tunnel_term_entry_t *p_tunnel_term)
{
    dn_sai_tunnel_t *tunnel_obj = NULL;

    tunnel_obj = dn_sai_tunnel_obj_get (p_tunnel_term->tunnel_id);

    if (tunnel_obj != NULL) {

        std_dll_insertatback (&tunnel_obj->tunnel_term_entry_list,
                              &p_tunnel_term->tunnel_link);
    }
}

static void dn_sai_tunnel_term_remove_from_tunnel_list (
                                dn_sai_tunnel_term_entry_t *p_tunnel_term)
{
    dn_sai_tunnel_t *tunnel_obj = NULL;

    tunnel_obj = dn_sai_tunnel_obj_get (p_tunnel_term->tunnel_id);

    if (tunnel_obj != NULL) {

        std_dll_remove (&tunnel_obj->tunnel_term_entry_list,
                        &p_tunnel_term->tunnel_link);
    }
}

static sai_status_t dn_sai_tunnel_term_create (sai_object_id_t *tunnel_term_id,
                                               sai_object_id_t  switch_id,
                                               uint32_t attr_count,
                                               const sai_attribute_t *attr_list)
{
    dn_sai_tunnel_term_entry_t *p_tunnel_term = NULL;
    dn_sai_tunnel_t            *p_tunnel = NULL;
    sai_status_t                status;
    int                         tunnel_term_index;
    char                        ip_addr_str[SAI_FIB_MAX_BUFSZ];
    bool                        is_encap_nh_setup = false;
    bool                        is_term_set_in_npu = false;

    STD_ASSERT (tunnel_term_id != NULL);
    STD_ASSERT (attr_list != NULL);

    SAI_TUNNEL_LOG_DEBUG ("Entering SAI Tunnel Termination entry creation.");

    status = dn_sai_tunnel_attr_list_validate (SAI_OBJECT_TYPE_TUNNEL_TERM_TABLE_ENTRY,
                                               attr_count, attr_list, SAI_OP_CREATE);
    if (status != SAI_STATUS_SUCCESS) {

        return status;
    }

    p_tunnel_term = dn_sai_tunnel_term_object_alloc();

    if (p_tunnel_term == NULL) {

        SAI_TUNNEL_LOG_CRIT ("SAI Tunnel Termination entry memory alloc failed");

        return SAI_STATUS_NO_MEMORY;
    }

    dn_sai_tunnel_lock();

    do {

        tunnel_term_index = std_find_first_bit (dn_sai_tunnel_term_obj_id_bitmap(),
                                                SAI_TUNNEL_TERM_OBJ_MAX_ID, 1);

        if (tunnel_term_index < 0) {

            SAI_TUNNEL_LOG_INFO ("No free index in tunnel term object index bitmap"
                                 " of size %d.", SAI_TUNNEL_TERM_OBJ_MAX_ID);

            status = SAI_STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        /* Validate and fill the attribute values passed in the list */
        status = dn_sai_tunnel_term_attr_set (p_tunnel_term, attr_count, attr_list);

        if (status != SAI_STATUS_SUCCESS) {
            SAI_TUNNEL_LOG_ERR ("SAI Tunnel Term entry attribute error.");

            break;
        }

        /* For vxlan underlay next hop needs to be setup when remote ip
         * is provided in the tunnel termination entry*/
        if (p_tunnel_term->tunnel_type == SAI_TUNNEL_TYPE_VXLAN) {

            p_tunnel = dn_sai_tunnel_obj_get(p_tunnel_term->tunnel_id);
            status = sai_tunnel_encap_nh_setup(&p_tunnel_term->src_ip,
                                               p_tunnel);

            if (status != SAI_STATUS_SUCCESS) {
                SAI_TUNNEL_LOG_ERR ("Failed to set up VXLAN underlay nexthop for ip"
                                    " address %s on tunnel id %"PRIx64"",
                                    sai_ip_addr_to_str(&p_tunnel_term->src_ip,
                                    ip_addr_str, sizeof(ip_addr_str)),
                                    p_tunnel_term->tunnel_id);
                break;
            }
            is_encap_nh_setup = true;
        }

        /* Create the tunnel object in NPU */
        status = sai_tunnel_npu_api_get()->tunnel_term_entry_create (p_tunnel_term);

        if (status != SAI_STATUS_SUCCESS) {
            SAI_TUNNEL_LOG_ERR ("SAI Tunnel Term entry create failed in NPU.");

            break;
        }
        is_term_set_in_npu = true;

        p_tunnel_term->term_entry_id =
                           sai_uoid_create (SAI_OBJECT_TYPE_TUNNEL_TERM_TABLE_ENTRY,
                                            tunnel_term_index);

        t_std_error rc = std_rbtree_insert (dn_sai_tunnel_term_tree_handle(),
                                            p_tunnel_term);
        if (rc != STD_ERR_OK) {

            SAI_TUNNEL_LOG_ERR ("Error in inserting Tunnel Term Id: 0x%"PRIx64" "
                                "to database", p_tunnel_term->term_entry_id);

            status = SAI_STATUS_FAILURE;

            break;
        }

        STD_BIT_ARRAY_CLR (dn_sai_tunnel_term_obj_id_bitmap(), tunnel_term_index);

        dn_sai_tunnel_term_add_to_tunnel_list (p_tunnel_term);

    } while (0);

    dn_sai_tunnel_unlock();

    if (status != SAI_STATUS_SUCCESS) {

        if(is_term_set_in_npu) {
            sai_tunnel_npu_api_get()->tunnel_term_entry_remove (p_tunnel_term);
        }

        if(is_encap_nh_setup) {
            sai_tunnel_encap_nh_teardown(&p_tunnel_term->src_ip, p_tunnel);
        }

        dn_sai_tunnel_term_object_free (p_tunnel_term);

        SAI_TUNNEL_LOG_INFO ("SAI Tunnel Termination create error: %d.", status);

    } else {

        *tunnel_term_id = p_tunnel_term->term_entry_id;

        SAI_TUNNEL_LOG_INFO ("Created SAI Tunnel Termnation entry Id: 0x%"PRIx64".",
                             *tunnel_term_id);
    }

    return status;
}

static sai_status_t dn_sai_tunnel_term_remove (sai_object_id_t tunnel_term_id)
{
    sai_status_t               status = SAI_STATUS_FAILURE;
    dn_sai_tunnel_t            *p_tunnel = NULL;
    dn_sai_tunnel_term_entry_t *p_tunnel_term = NULL;
    char                        ip_addr_str[SAI_FIB_MAX_BUFSZ];

    SAI_TUNNEL_LOG_DEBUG ("Entering SAI Tunnel Termination entry remove.");

    if (!sai_is_obj_id_tunnel_term_entry (tunnel_term_id)) {

        SAI_TUNNEL_LOG_ERR ("OID: 0x%"PRIx64" is not of tunnel term entry object.",
                            tunnel_term_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    dn_sai_tunnel_lock();

    do {
        p_tunnel_term = dn_sai_tunnel_term_entry_get (tunnel_term_id);

        if (p_tunnel_term == NULL) {

            SAI_TUNNEL_LOG_ERR ("Tunnel Term entry not found for OID: 0x%"PRIx64".",
                                tunnel_term_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        dn_sai_tunnel_term_entry_t *p_tmp_node = (dn_sai_tunnel_term_entry_t *)
            std_rbtree_remove (dn_sai_tunnel_term_tree_handle(), p_tunnel_term);

        if (p_tmp_node != p_tunnel_term) {

            SAI_TUNNEL_LOG_ERR ("Error in removing Tunnel Term entry from db.");

            break;
        }

        /* For vxlan underlay next hop needs to be setup when remote ip
         * is provided in the tunnel termination entry*/
        if (p_tunnel_term->tunnel_type == SAI_TUNNEL_TYPE_VXLAN) {

            p_tunnel = dn_sai_tunnel_obj_get(p_tunnel_term->tunnel_id);
            status = sai_tunnel_encap_nh_teardown(&p_tunnel_term->src_ip,
                                                  p_tunnel);

            if (status != SAI_STATUS_SUCCESS) {
                SAI_TUNNEL_LOG_ERR ("Failed to teardown VXLAN underlay nexthop for ip"
                                    " address %s on tunnel id %"PRIx64"",
                                    sai_ip_addr_to_str(&p_tunnel_term->src_ip,
                                    ip_addr_str, sizeof(ip_addr_str)),
                                    p_tunnel_term->tunnel_id);
                break;
            }
        }

        /* Remove the tunnel termination entry in NPU */
        status = sai_tunnel_npu_api_get()->tunnel_term_entry_remove (p_tunnel_term);

        if (status != SAI_STATUS_SUCCESS) {
            SAI_TUNNEL_LOG_ERR ("SAI Tunnel Termination entry remove failed in NPU.");

            std_rbtree_insert (dn_sai_tunnel_term_tree_handle(), p_tunnel_term);
            break;
        }

        STD_BIT_ARRAY_SET (dn_sai_tunnel_term_obj_id_bitmap(),
                           sai_uoid_npu_obj_id_get (tunnel_term_id));

        dn_sai_tunnel_term_remove_from_tunnel_list (p_tunnel_term);

        dn_sai_tunnel_term_object_free (p_tunnel_term);

    } while (0);

    dn_sai_tunnel_unlock();

    SAI_TUNNEL_LOG_INFO ("SAI Tunnel Term entry Id 0x%"PRIx64" remove status: %d.",
                         tunnel_term_id, status);

    return status;
}

static sai_status_t dn_sai_tunnel_term_set_attr (sai_object_id_t tunnel_term_id,
                                                 const sai_attribute_t *attr)
{
    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t dn_sai_tunnel_term_get_attr (sai_object_id_t tunnel_term_id,
                                                 uint32_t attr_count,
                                                 sai_attribute_t *attr_list)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

void dn_sai_tunnel_term_obj_api_fill (sai_tunnel_api_t *api_table)
{
    api_table->create_tunnel_term_table_entry        = dn_sai_tunnel_term_create;
    api_table->remove_tunnel_term_table_entry        = dn_sai_tunnel_term_remove;
    api_table->set_tunnel_term_table_entry_attribute = dn_sai_tunnel_term_set_attr;
    api_table->get_tunnel_term_table_entry_attribute = dn_sai_tunnel_term_get_attr;
}
