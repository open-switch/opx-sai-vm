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
* @file sai_l2mc_utils.c
*
* @brief This file contains utility APIs for SAI L2MC module
*************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "saitypes.h"
#include "saistatus.h"
#include "saitypes.h"
#include "std_llist.h"
#include "sai_gen_utils.h"
#include "sai_l2mc_common.h"
#include "std_mutex_lock.h"
#include "std_assert.h"
#include "std_rbtree.h"
#include "sai_oid_utils.h"
#include "sai_gen_utils.h"
#include "sai_bridge_api.h"

static rbtree_handle global_l2mc_group_tree;
static std_mutex_lock_create_static_init_fast(l2mc_lock);
static rbtree_handle global_l2mc_member_tree;

rbtree_handle sai_l2mc_global_member_tree_get(void)
{
    return global_l2mc_member_tree;
}

void sai_l2mc_lock(void)
{
    std_mutex_lock(&l2mc_lock);
}

void sai_l2mc_unlock(void)
{
    std_mutex_unlock(&l2mc_lock);
}

sai_status_t sai_l2mc_tree_init(void)
{

    SAI_L2MC_LOG_TRACE("Performing L2MC Module Init");

    /* Create the global tree for L2MC group objects */
    global_l2mc_group_tree = std_rbtree_create_simple("SAI L2MC group tree",
            STD_STR_OFFSET_OF(dn_sai_l2mc_group_node_t, l2mc_group_id),
            STD_STR_SIZE_OF(dn_sai_l2mc_group_node_t, l2mc_group_id));

    /* Create the global tree for L2MC group memeber objects */
    global_l2mc_member_tree = std_rbtree_create_simple("SAI L2MC member tree",
            STD_STR_OFFSET_OF(dn_sai_l2mc_member_node_t, l2mc_member_id),
            STD_STR_SIZE_OF(dn_sai_l2mc_member_node_t, l2mc_member_id));

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_add_l2mc_group_node(dn_sai_l2mc_group_node_t *l2mc_group_info)
{
    dn_sai_l2mc_group_node_t *l2mc_group_node = NULL;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    do {
        if ((l2mc_group_node = (dn_sai_l2mc_group_node_t*)
                    calloc(1, sizeof(dn_sai_l2mc_group_node_t))) == NULL) {
            SAI_L2MC_LOG_CRIT("Unable to add l2mc group Id 0x%"PRIx64" memory %lu unavailable",
                    sai_uoid_npu_obj_id_get(l2mc_group_info->l2mc_group_id),
                    sizeof(dn_sai_l2mc_group_node_t));
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }
        memcpy(l2mc_group_node, l2mc_group_info, sizeof(dn_sai_l2mc_group_node_t));
        std_dll_init(&(l2mc_group_node->member_list));
        l2mc_group_node->port_count = 0;

        if(std_rbtree_insert(global_l2mc_group_tree, l2mc_group_node)
                != STD_ERR_OK) {
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }
        SAI_L2MC_LOG_TRACE("Added L2MC Group id 0x%"PRIx64"",
            sai_uoid_npu_obj_id_get(l2mc_group_info->l2mc_group_id));

    } while(0);

    if(sai_rc != SAI_STATUS_SUCCESS){
        free(l2mc_group_node);
    }
    return sai_rc;
}

dn_sai_l2mc_group_node_t * sai_find_l2mc_group_node(sai_object_id_t l2mc_group_id)
{
    dn_sai_l2mc_group_node_t l2mc_group_info, *l2mc_group_node = NULL;

    if(sai_is_obj_id_l2mc_group(l2mc_group_id)) {
        memset(&l2mc_group_info, 0, sizeof(l2mc_group_info));
        l2mc_group_info.l2mc_group_id = l2mc_group_id;
        l2mc_group_node = (dn_sai_l2mc_group_node_t *)
            std_rbtree_getexact(global_l2mc_group_tree, &l2mc_group_info);
    }

    return l2mc_group_node;
}

dn_sai_l2mc_group_node_t* sai_l2mc_group_get_next(dn_sai_l2mc_group_node_t* l2mc_group_node)
{
    STD_ASSERT (l2mc_group_node!= NULL);
    return (dn_sai_l2mc_group_node_t *)std_rbtree_getnext(global_l2mc_group_tree,
                                                           l2mc_group_node);
}

sai_status_t sai_l2mc_add_bridge_id(sai_object_id_t l2mc_group_id,
                                    sai_object_id_t bridge_id)
{
    dn_sai_l2mc_group_node_t *l2mc_group_node = sai_find_l2mc_group_node(l2mc_group_id);

    if(l2mc_group_node == NULL) {
       SAI_L2MC_LOG_ERR("L2MC Object 0x%"PRIx64" not found", l2mc_group_id);
       return SAI_STATUS_ITEM_NOT_FOUND;
    }
    if(l2mc_group_node->bridge_id != SAI_NULL_OBJECT_ID) {
        if(l2mc_group_node->bridge_id != bridge_id) {
            return SAI_STATUS_OBJECT_IN_USE;
        }
    } else {
        l2mc_group_node->bridge_id = bridge_id;
    }
    l2mc_group_node->bridge_ref_count++;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_l2mc_remove_bridge_id(sai_object_id_t l2mc_group_id,
                                       sai_object_id_t bridge_id)
{
    dn_sai_l2mc_group_node_t *l2mc_group_node = sai_find_l2mc_group_node(l2mc_group_id);

    if(l2mc_group_node == NULL) {
       SAI_L2MC_LOG_ERR("L2MC Object 0x%"PRIx64" not found", l2mc_group_id);
       return SAI_STATUS_ITEM_NOT_FOUND;
    }
    if((l2mc_group_node->bridge_id != bridge_id) || (l2mc_group_node->bridge_ref_count == 0)) {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    l2mc_group_node->bridge_ref_count--;
    if((l2mc_group_node->bridge_ref_count == 0) && (l2mc_group_node->port_count == 0)) {
        l2mc_group_node->bridge_id = SAI_NULL_OBJECT_ID;
    }
    return SAI_STATUS_SUCCESS;
}


bool sai_l2mc_is_valid_member_bridge_id(const dn_sai_l2mc_group_node_t *l2mc_group_node,
                                        sai_object_id_t bridge_port_id)
{
    sai_status_t    sai_rc = SAI_STATUS_FAILURE;
    sai_object_id_t bridge_id = SAI_NULL_OBJECT_ID;

    STD_ASSERT(l2mc_group_node != NULL);

    sai_rc = sai_bridge_port_get_bridge_id(bridge_port_id, &bridge_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_L2MC_LOG_ERR("Error %d in getting bridge id for bridge port 0x%"PRIx64"",
                         sai_rc, bridge_port_id);
        return false;
    }

    if(l2mc_group_node->bridge_id == SAI_NULL_OBJECT_ID) {
        return true;
    }
    return (bridge_id == l2mc_group_node->bridge_id);
}

sai_status_t sai_remove_l2mc_group_node(dn_sai_l2mc_group_node_t *l2mc_group_info)
{
    dn_sai_l2mc_group_node_t *l2mc_group_node = NULL;
    STD_ASSERT(l2mc_group_info != NULL);

    if(sai_is_obj_id_l2mc_group(l2mc_group_info->l2mc_group_id)) {
        l2mc_group_node = (dn_sai_l2mc_group_node_t *)
            std_rbtree_getexact(global_l2mc_group_tree, l2mc_group_info);
        if(l2mc_group_node != NULL) {
            if(l2mc_group_node->port_count > 0) {
                return SAI_STATUS_OBJECT_IN_USE;
            }
            std_rbtree_remove(global_l2mc_group_tree, l2mc_group_node);
            SAI_L2MC_LOG_TRACE("Deleted L2mc group Id 0x%"PRIx64"", l2mc_group_info->l2mc_group_id);
            free(l2mc_group_node);
            return SAI_STATUS_SUCCESS;
        }
        SAI_L2MC_LOG_WARN("L2mc Id 0x%"PRIx64" not found", l2mc_group_info->l2mc_group_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    return SAI_STATUS_INVALID_PARAMETER;
}

sai_status_t sai_add_l2mc_member_node(dn_sai_l2mc_member_node_t l2mc_member_info)
{
    dn_sai_l2mc_member_node_t *l2mc_member_node = NULL;
    dn_sai_l2mc_member_dll_node_t *l2mc_member_dll_node = NULL;
    dn_sai_l2mc_group_node_t *l2mc_group_node = NULL;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t member_bridge_id = SAI_NULL_OBJECT_ID;

    if((l2mc_group_node = sai_find_l2mc_group_node(l2mc_member_info.l2mc_group_id))
                == NULL ) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    do {
        if((l2mc_member_node = (dn_sai_l2mc_member_node_t *)
                    calloc(1, sizeof(dn_sai_l2mc_member_node_t))) == NULL) {
            SAI_L2MC_LOG_CRIT("Unable to add  port 0x%"PRIx64" L2mc Group Id 0x%"PRIx64""
                              " memory %lu unavailable", l2mc_member_info.bridge_port_id,
                              l2mc_member_info.l2mc_group_id, sizeof(dn_sai_l2mc_member_node_t));
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        if((l2mc_member_dll_node = (dn_sai_l2mc_member_dll_node_t *)
                    calloc(1, sizeof(dn_sai_l2mc_member_dll_node_t))) == NULL) {
            SAI_L2MC_LOG_CRIT("Unable to add  port 0x%"PRIx64" L2mc Group Id 0x%"PRIx64""
                              " memory %lu unavailable", l2mc_member_info.bridge_port_id,
                              l2mc_member_info.l2mc_group_id,
                              sizeof(dn_sai_l2mc_member_dll_node_t));
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        *l2mc_member_node = l2mc_member_info;
        l2mc_member_dll_node->l2mc_member_info = l2mc_member_node;

        if(std_rbtree_insert(global_l2mc_member_tree, l2mc_member_node)
                != STD_ERR_OK) {
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

        std_dll_insertatback(&(l2mc_group_node->member_list),
                &(l2mc_member_dll_node->node));
        sai_bridge_port_get_bridge_id(l2mc_member_info.bridge_port_id, &member_bridge_id);
        l2mc_group_node->bridge_id = member_bridge_id;
        l2mc_group_node->port_count++;

        SAI_L2MC_LOG_TRACE("Added  port 0x:%"PRIx64" L2mc Group Id 0x%"PRIx64"",
                           l2mc_member_info.bridge_port_id, l2mc_member_info.l2mc_group_id);
    } while(0);

    if(SAI_STATUS_SUCCESS != sai_rc) {
        if(l2mc_member_node) {
            free(l2mc_member_node);
        }
        if(l2mc_member_dll_node) {
            free(l2mc_member_dll_node);
        }
    }

    return sai_rc;
}

dn_sai_l2mc_member_node_t* sai_find_l2mc_member_node(
        sai_object_id_t l2mc_member_id)
{
    dn_sai_l2mc_member_node_t l2mc_member_info, *l2mc_member_node = NULL;

    if(sai_is_obj_id_l2mc_member(l2mc_member_id)) {
        memset(&l2mc_member_info, 0, sizeof(l2mc_member_info));
        l2mc_member_info.l2mc_member_id = l2mc_member_id;
        l2mc_member_node = (dn_sai_l2mc_member_node_t *)
            std_rbtree_getexact(global_l2mc_member_tree, &l2mc_member_info);
    }

    return l2mc_member_node;
}

dn_sai_l2mc_member_node_t* sai_l2mc_member_get_next(dn_sai_l2mc_member_node_t* l2mc_member_node)
{
    STD_ASSERT (l2mc_member_node!= NULL);
    return (dn_sai_l2mc_member_node_t *)std_rbtree_getnext(global_l2mc_member_tree,
                                                           l2mc_member_node);
}

dn_sai_l2mc_member_dll_node_t* sai_find_l2mc_member_node_from_port(
        dn_sai_l2mc_group_node_t * l2mc_group_node,
        sai_object_id_t bridge_port_id)
{
    dn_sai_l2mc_member_dll_node_t *l2mc_member_dll_node = NULL;
    std_dll *node = NULL;

    if(l2mc_group_node != NULL) {
        for(node = std_dll_getfirst(&(l2mc_group_node->member_list));
                node != NULL;
                node = std_dll_getnext(&(l2mc_group_node->member_list),node)) {
            l2mc_member_dll_node = (dn_sai_l2mc_member_dll_node_t *)node;
            if(l2mc_member_dll_node->l2mc_member_info->bridge_port_id == bridge_port_id)
                return l2mc_member_dll_node;
        }
        SAI_L2MC_LOG_TRACE("port 0x%"PRIx64" not found on L2mc Group Id 0x%"PRIx64"",
                bridge_port_id, sai_uoid_npu_obj_id_get(l2mc_group_node->l2mc_group_id));
    }

    return NULL;
}

sai_status_t sai_remove_l2mc_member_node(dn_sai_l2mc_member_node_t *l2mc_member_info)
{
    dn_sai_l2mc_member_dll_node_t *l2mc_member_dll_node = NULL;
    dn_sai_l2mc_member_node_t *l2mc_member_node = NULL;
    dn_sai_l2mc_group_node_t *l2mc_group_node = NULL;

    STD_ASSERT(l2mc_member_info != NULL);

    if((l2mc_group_node = sai_find_l2mc_group_node(l2mc_member_info->l2mc_group_id))
                == NULL ) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    l2mc_member_dll_node = sai_find_l2mc_member_node_from_port(
            l2mc_group_node, l2mc_member_info->bridge_port_id);
    if(l2mc_member_dll_node != NULL) {
        std_dll_remove(&(l2mc_group_node->member_list),
                &(l2mc_member_dll_node->node));
        l2mc_group_node->port_count--;
        if((l2mc_group_node->port_count == 0)  && (l2mc_group_node->bridge_ref_count == 0)){
            l2mc_group_node->bridge_id = SAI_NULL_OBJECT_ID;
        }

        l2mc_member_node = l2mc_member_dll_node->l2mc_member_info;
        l2mc_member_node = std_rbtree_remove(global_l2mc_member_tree,
                l2mc_member_node);

        SAI_L2MC_LOG_TRACE("port 0x%"PRIx64" removed from L2mc Group Id 0x%"PRIx64"",
                l2mc_member_info->bridge_port_id, l2mc_member_info->l2mc_group_id);
        free(l2mc_member_dll_node);
        free(l2mc_member_node);
        return SAI_STATUS_SUCCESS;
    }
    return SAI_STATUS_INVALID_PORT_MEMBER;
}

sai_status_t sai_l2mc_port_list_get(dn_sai_l2mc_group_node_t *l2mc_group_node,
        sai_object_list_t *l2mc_port_list)
{
    dn_sai_l2mc_member_dll_node_t *l2mc_member_dll_node = NULL;
    std_dll *node = NULL;
    unsigned int port_idx = 0;

    STD_ASSERT(l2mc_port_list != NULL);
    STD_ASSERT(l2mc_group_node != NULL);
    if(l2mc_port_list->count < l2mc_group_node->port_count) {
        l2mc_port_list->count = l2mc_group_node->port_count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }
    for(node = std_dll_getfirst(&(l2mc_group_node->member_list));
            node != NULL;
            node = std_dll_getnext(&(l2mc_group_node->member_list),node)) {
        l2mc_member_dll_node = (dn_sai_l2mc_member_dll_node_t *)node;
        l2mc_port_list->list[port_idx] =
            l2mc_member_dll_node->l2mc_member_info->l2mc_member_id;
        port_idx++;
    }

    l2mc_port_list->count = port_idx;
    return SAI_STATUS_SUCCESS;
}

