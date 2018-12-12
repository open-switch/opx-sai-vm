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
* @file sai_vlan_utils.c
*
* @brief This file contains utility APIs for SAI VLAN module
*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "saivlan.h"
#include "saistatus.h"
#include "saitypes.h"
#include "std_llist.h"
#include "sai_vlan_api.h"
#include "sai_vlan_common.h"
#include "std_mutex_lock.h"
#include "std_assert.h"
#include "sai_switch_utils.h"
#include "sai_port_utils.h"
#include "sai_oid_utils.h"
#include "sai_gen_utils.h"
#include "sai_bridge_api.h"
#include "sai_lag_api.h"

static sai_vlan_global_cache_node_t *global_vlan_list[SAI_MAX_VLAN_TAG_ID+1];
static std_mutex_lock_create_static_init_fast(vlan_lock);
static sai_vlan_id_t sai_internal_vlan_id = VLAN_UNDEF;
static rbtree_handle global_vlan_member_tree;

rbtree_handle sai_vlan_global_member_tree_get(void)
{
    return global_vlan_member_tree;
}

void sai_vlan_lock(void)
{
    std_mutex_lock(&vlan_lock);
}

void sai_vlan_unlock(void)
{
    std_mutex_unlock(&vlan_lock);
}

sai_vlan_global_cache_node_t* sai_vlan_portlist_cache_read(sai_vlan_id_t vlan_id)
{
    return global_vlan_list[vlan_id];
}

void sai_init_internal_vlan_id(sai_vlan_id_t vlan_id)
{
    sai_internal_vlan_id = vlan_id;
}

bool sai_is_internal_vlan_id_initialized (void)
{
    return (sai_internal_vlan_id != VLAN_UNDEF);
}

bool sai_is_internal_vlan_id(sai_vlan_id_t vlan_id)
{
    return (vlan_id == sai_internal_vlan_id) ? true : false;
}

sai_vlan_id_t sai_internal_vlan_id_get(void)
{
    return sai_internal_vlan_id;
}

sai_vlan_id_t sai_vlan_obj_id_to_vlan_id(sai_object_id_t vlan_obj_id)
{
    return ((sai_vlan_id_t)sai_uoid_npu_obj_id_get(vlan_obj_id));
}

sai_object_id_t sai_vlan_id_to_vlan_obj_id(sai_vlan_id_t vlan_id)
{
    return (sai_uoid_create(SAI_OBJECT_TYPE_VLAN, vlan_id));
}

sai_status_t sai_vlan_cache_init(void)
{
    sai_vlan_id_t vlan_id = 0;

    SAI_VLAN_LOG_TRACE("Performing VLAN Module Init");
    for(vlan_id = 0; vlan_id <= SAI_MAX_VLAN_TAG_ID; vlan_id++)
    {
        global_vlan_list[vlan_id] = NULL;
    }
    global_vlan_member_tree = std_rbtree_create_simple("SAI VLAN member tree",
            STD_STR_OFFSET_OF(sai_vlan_member_node_t, vlan_member_id),
            STD_STR_SIZE_OF(sai_vlan_member_node_t, vlan_member_id));

    return SAI_STATUS_SUCCESS;
}

bool sai_is_vlan_created(sai_vlan_id_t vlan_id)
{
    if((vlan_id > SAI_MAX_VLAN_TAG_ID) ||
        (vlan_id < SAI_MIN_VLAN_TAG_ID)) {
        return false;
    }
    return ((global_vlan_list[vlan_id] != NULL) ? true:false);
}

sai_status_t sai_insert_vlan_in_list(sai_vlan_id_t vlan_id)
{
    sai_vlan_global_cache_node_t *vplist = NULL;

    if(sai_is_vlan_created(vlan_id)) {
        SAI_VLAN_LOG_INFO("Vlan Id %d already created",
                vlan_id);
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    vplist=(sai_vlan_global_cache_node_t *)calloc(1, sizeof(sai_vlan_global_cache_node_t));
    if(vplist == NULL) {
        SAI_VLAN_LOG_CRIT("No memory  to create Vlan Id %d memory:%lu",
                vlan_id, sizeof(sai_vlan_global_cache_node_t));
        return SAI_STATUS_NO_MEMORY;
    }

    vplist->vlan_id=vlan_id;
    std_dll_init(&(vplist->member_list));
    global_vlan_list[vlan_id] = vplist;
    SAI_VLAN_LOG_TRACE("Vlan Id %d Inserted in cache",
            vlan_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_vlan_from_list(sai_vlan_id_t vlan_id)
{
    if(!sai_is_vlan_created(vlan_id)) {
        SAI_VLAN_LOG_WARN("Vlan Id %d not found", vlan_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if(global_vlan_list[vlan_id]->port_count > 0) {
        return SAI_STATUS_OBJECT_IN_USE;
    }

    free(global_vlan_list[vlan_id]);
    global_vlan_list[vlan_id] = NULL;
    SAI_VLAN_LOG_TRACE("Deleted Vlan Id %d", vlan_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_add_vlan_member_node(sai_vlan_member_node_t vlan_member_info)
{
    sai_vlan_member_node_t *vlan_member_node = NULL;
    sai_vlan_member_dll_node_t *vlan_member_dll_node = NULL;
    sai_port_fwd_mode_t fwd_mode;
    sai_status_t ret_val = SAI_STATUS_SUCCESS;
    sai_vlan_id_t vlan_id = VLAN_UNDEF;
    sai_object_id_t   port_obj_id = SAI_NULL_OBJECT_ID;

    vlan_id = sai_vlan_obj_id_to_vlan_id(vlan_member_info.vlan_id);

    do {
        ret_val = sai_bridge_port_get_port_id(vlan_member_info.bridge_port_id, &port_obj_id);

        if(ret_val != SAI_STATUS_SUCCESS) {
            SAI_VLAN_LOG_ERR("Error in getting port obj id from bridge port 0x%"PRIx64"",
                             vlan_member_info.bridge_port_id);
            break;
        }

        if((vlan_member_node = (sai_vlan_member_node_t *)
                    calloc(1, sizeof(sai_vlan_member_node_t))) == NULL) {
            SAI_VLAN_LOG_CRIT("Unable to add  bridge port 0x%"PRIx64" Vlan Id %d \
                    memory %lu unavailable",
                    vlan_member_info.bridge_port_id,
                    vlan_id,
                    sizeof(sai_vlan_member_node_t));
            ret_val = SAI_STATUS_NO_MEMORY;
            break;
        }

        if((vlan_member_dll_node = (sai_vlan_member_dll_node_t *)
                    calloc(1, sizeof(sai_vlan_member_dll_node_t))) == NULL) {
            SAI_VLAN_LOG_CRIT("Unable to add bridge port 0x%"PRIx64" Vlan obj 0x%"PRIx64" \
                    memory %lu unavailable",
                    vlan_member_info.bridge_port_id,
                    vlan_member_info.vlan_id,
                    sizeof(sai_vlan_member_dll_node_t));
            ret_val = SAI_STATUS_NO_MEMORY;
            break;
        }

        *vlan_member_node = vlan_member_info;
        vlan_member_dll_node->vlan_member_info = vlan_member_node;

        if(std_rbtree_insert(global_vlan_member_tree, vlan_member_node)
                != STD_ERR_OK) {
            ret_val = SAI_STATUS_FAILURE;
            break;
        }

        std_dll_insertatback(&(global_vlan_list[vlan_id]->member_list),
                &(vlan_member_dll_node->node));
        global_vlan_list[vlan_id]->port_count++;

        if(sai_port_get_vlan_count(port_obj_id) == 0) {
            fwd_mode = SAI_PORT_FWD_MODE_SWITCHING;
            if(sai_is_obj_id_lag(port_obj_id)) {
                sai_lag_forwarding_mode_set (port_obj_id, fwd_mode);
            } else {
                sai_port_forwarding_mode_set (port_obj_id, fwd_mode);
            }
        }
        sai_port_increment_vlan_count(port_obj_id);

        SAI_VLAN_LOG_TRACE("Added bridge port 0x:%"PRIx64" Vlan Id %d",
                vlan_member_info.bridge_port_id, vlan_id);
    } while(0);

    if(SAI_STATUS_SUCCESS != ret_val) {
        free(vlan_member_node);
        free(vlan_member_dll_node);
    }

    return ret_val;
}

sai_vlan_member_node_t* sai_find_vlan_member_node(
        sai_object_id_t vlan_member_id)
{
    sai_vlan_member_node_t vlan_member_info, *vlan_member_node = NULL;

    if(sai_is_obj_id_vlan_member(vlan_member_id)) {
        memset(&vlan_member_info, 0, sizeof(vlan_member_info));
        vlan_member_info.vlan_member_id = vlan_member_id;
        vlan_member_node = (sai_vlan_member_node_t *)
            std_rbtree_getexact(global_vlan_member_tree, &vlan_member_info);
    }

    return vlan_member_node;
}

sai_vlan_member_dll_node_t* sai_find_vlan_member_node_from_port(sai_vlan_id_t vlan_id,
                                                                sai_object_id_t bridge_port_id)
{
    sai_vlan_member_dll_node_t *vlan_member_dll_node = NULL;
    std_dll *node = NULL;

    if(global_vlan_list[vlan_id]) {
        for(node = std_dll_getfirst(&(global_vlan_list[vlan_id]->member_list));
                node != NULL;
                node = std_dll_getnext(&(global_vlan_list[vlan_id]->member_list),node)) {
            vlan_member_dll_node = (sai_vlan_member_dll_node_t *)node;
            if(vlan_member_dll_node->vlan_member_info->bridge_port_id == bridge_port_id)
                return vlan_member_dll_node;
        }
        SAI_VLAN_LOG_TRACE("port 0x%"PRIx64" not found on Vlan Id %d",
                           bridge_port_id, vlan_id);
    }

    return NULL;
}

bool sai_is_vlan_obj_in_use(sai_vlan_id_t vlan_id)
{
    if((global_vlan_list[vlan_id]) &&
            (global_vlan_list[vlan_id]->port_count > 0)) {
        return true;
    } else {
        return false;
    }
}

bool sai_is_bridge_port_vlan_member(sai_vlan_id_t vlan_id,
                                    sai_object_id_t bridge_port_id)
{
    bool ret_val = false;

    if(sai_find_vlan_member_node_from_port(vlan_id, bridge_port_id) != NULL) {
        ret_val = true;
    }
    return ret_val;
}

sai_status_t sai_remove_vlan_member_node(sai_vlan_member_node_t vlan_member_info)
{
    sai_vlan_member_dll_node_t *vlan_member_dll_node = NULL;
    sai_vlan_member_node_t *vlan_member_node = NULL;
    sai_port_fwd_mode_t fwd_mode;
    sai_vlan_id_t vlan_id = VLAN_UNDEF;
    sai_status_t ret_val;
    sai_object_id_t   port_obj_id = SAI_NULL_OBJECT_ID;

    vlan_id = sai_vlan_obj_id_to_vlan_id(vlan_member_info.vlan_id);

    ret_val = sai_bridge_port_get_port_id(vlan_member_info.bridge_port_id, &port_obj_id);

    if(ret_val != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR("Error in getting port obj id from bridge port 0x%"PRIx64"",
                vlan_member_info.bridge_port_id);
        return ret_val;
    }

    vlan_member_dll_node = sai_find_vlan_member_node_from_port(vlan_id,
                                                               vlan_member_info.bridge_port_id);
    if(vlan_member_dll_node != NULL) {
        std_dll_remove(&(global_vlan_list[vlan_id]->member_list),
                &(vlan_member_dll_node->node));
        global_vlan_list[vlan_id]->port_count--;

        sai_port_decrement_vlan_count(port_obj_id);
        vlan_member_node = vlan_member_dll_node->vlan_member_info;
        vlan_member_node = std_rbtree_remove(global_vlan_member_tree,
                vlan_member_node);

        if(sai_port_get_vlan_count(port_obj_id) == 0) {
            fwd_mode = SAI_PORT_FWD_MODE_UNKNOWN;
            if(sai_is_obj_id_lag(port_obj_id)) {
                sai_lag_forwarding_mode_set (port_obj_id, fwd_mode);
            } else {
                sai_port_forwarding_mode_set (port_obj_id, fwd_mode);
            }
        }

        SAI_VLAN_LOG_TRACE("Bridge port 0x%"PRIx64" removed from Vlan Id %d",
                           vlan_member_info.bridge_port_id, vlan_id);
        free(vlan_member_dll_node);
        free(vlan_member_node);
        return SAI_STATUS_SUCCESS;
    }
    return SAI_STATUS_INVALID_PORT_MEMBER;
}

sai_status_t sai_vlan_port_list_get(sai_vlan_id_t vlan_id,
        sai_object_list_t *vlan_port_list)
{
    sai_vlan_member_dll_node_t *vlan_member_dll_node = NULL;
    std_dll *node = NULL;
    unsigned int port_idx = 0;

    STD_ASSERT(vlan_port_list != NULL);
    if(vlan_port_list->count < global_vlan_list[vlan_id]->port_count) {
        vlan_port_list->count = global_vlan_list[vlan_id]->port_count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }
    for(node = std_dll_getfirst(&(global_vlan_list[vlan_id]->member_list));
            node != NULL;
            node = std_dll_getnext(&(global_vlan_list[vlan_id]->member_list),node)) {
        vlan_member_dll_node = (sai_vlan_member_dll_node_t *)node;
        vlan_port_list->list[port_idx] =
            vlan_member_dll_node->vlan_member_info->vlan_member_id;
        port_idx++;
    }

    vlan_port_list->count = port_idx;
    return SAI_STATUS_SUCCESS;
}

bool sai_is_valid_vlan_tagging_mode(sai_vlan_tagging_mode_t tagging_mode)
{
    switch(tagging_mode) {
        case SAI_VLAN_TAGGING_MODE_UNTAGGED:
        case SAI_VLAN_TAGGING_MODE_TAGGED:
        case SAI_VLAN_TAGGING_MODE_PRIORITY_TAGGED:
            return true;
    }
    return false;
}

sai_status_t sai_vlan_set_attribute_in_cache(sai_object_id_t vlan_obj_id,
                                             const sai_attribute_t* attr)
{
    sai_vlan_id_t vlan_id = sai_vlan_obj_id_to_vlan_id(vlan_obj_id);

    if(attr == NULL) {
        SAI_VLAN_LOG_TRACE("attr is NULL in set vlan attr in cache for vlan %d", vlan_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch(attr->id){
        /* This is handled in STP module. Nothing to update in cache */
        case SAI_VLAN_ATTR_STP_INSTANCE:
            return SAI_STATUS_SUCCESS;

        case SAI_VLAN_ATTR_LEARN_DISABLE:
            global_vlan_list[vlan_id]->learn_disable = attr->value.booldata;
            break;

        case SAI_VLAN_ATTR_META_DATA:
            global_vlan_list[vlan_id]->meta_data = attr->value.u32;
            break;

        case SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES:
            global_vlan_list[vlan_id]->max_learned_address =  attr->value.u32;
            break;

        case SAI_VLAN_ATTR_IPV4_MCAST_LOOKUP_KEY_TYPE:
        case SAI_VLAN_ATTR_IPV6_MCAST_LOOKUP_KEY_TYPE:
        case SAI_VLAN_ATTR_UNKNOWN_NON_IP_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_UNKNOWN_IPV4_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_UNKNOWN_IPV6_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_UNKNOWN_LINKLOCAL_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_INGRESS_ACL:
        case SAI_VLAN_ATTR_EGRESS_ACL:
        case SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
        case SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
        case SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
        case SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
        case SAI_VLAN_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
        case SAI_VLAN_ATTR_BROADCAST_FLOOD_GROUP:

            return SAI_STATUS_ATTR_NOT_IMPLEMENTED_0;

        case SAI_VLAN_ATTR_VLAN_ID:
        case SAI_VLAN_ATTR_MEMBER_LIST:
            return SAI_STATUS_INVALID_ATTRIBUTE_0;

        default:
            return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_vlan_get_attribute_from_cache(sai_object_id_t vlan_obj_id, sai_attribute_t* attr)
{
    sai_vlan_id_t vlan_id = sai_vlan_obj_id_to_vlan_id(vlan_obj_id);

    if(attr == NULL) {
        SAI_VLAN_LOG_TRACE("attr is NULL in set vlan attr in cache for vlan %d", vlan_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch(attr->id){
        /* This is handled in STP module. Nothing to update in cache */
        case SAI_VLAN_ATTR_STP_INSTANCE:
            return SAI_STATUS_SUCCESS;

        case SAI_VLAN_ATTR_VLAN_ID:
            attr->value.u16 = vlan_id;
            break;

        case SAI_VLAN_ATTR_LEARN_DISABLE:
            attr->value.booldata = global_vlan_list[vlan_id]->learn_disable;
            break;

        case SAI_VLAN_ATTR_META_DATA:
            attr->value.u32 = global_vlan_list[vlan_id]->meta_data;
            break;

        case SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES:
            attr->value.u32 = global_vlan_list[vlan_id]->max_learned_address;
            break;

        case SAI_VLAN_ATTR_MEMBER_LIST:
            return sai_vlan_port_list_get(vlan_id, &(attr->value.objlist));

        case SAI_VLAN_ATTR_IPV4_MCAST_LOOKUP_KEY_TYPE:
        case SAI_VLAN_ATTR_IPV6_MCAST_LOOKUP_KEY_TYPE:
        case SAI_VLAN_ATTR_UNKNOWN_NON_IP_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_UNKNOWN_IPV4_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_UNKNOWN_IPV6_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_UNKNOWN_LINKLOCAL_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_INGRESS_ACL:
        case SAI_VLAN_ATTR_EGRESS_ACL:
        case SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
        case SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
        case SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
        case SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
        case SAI_VLAN_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
        case SAI_VLAN_ATTR_BROADCAST_FLOOD_GROUP:

            return SAI_STATUS_ATTR_NOT_IMPLEMENTED_0;

        default:
            return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
    }
    return SAI_STATUS_SUCCESS;
}
