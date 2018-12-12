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
* @file  sai_qos_port_utils.c
*
* @brief This file contains utility function definitions for SAI Qos port
*
*************************************************************************/

#include "sai_port_utils.h"
#include "std_struct_utils.h"
#include "sai_qos_util.h"
#include "sai_qos_buffer_util.h"
#include "inttypes.h"

#include <stdlib.h>
#include <string.h>

/* RB tree to get Port Pool node given Port Pool ID */
static rbtree_handle port_pool_object_tree;

/* RB tree to get Port Pool node given Port ID and Pool ID */
static rbtree_handle port_pool_tree;

sai_status_t sai_qos_port_pool_tree_init(void)
{
    port_pool_object_tree = std_rbtree_create_simple("SAI Port Pool Object tree",
            STD_STR_OFFSET_OF(dn_sai_qos_port_pool_t, port_pool_id),
            STD_STR_SIZE_OF(dn_sai_qos_port_pool_t, port_pool_id));

    if(NULL == port_pool_object_tree) {
        return SAI_STATUS_FAILURE;
    }

    port_pool_tree = std_rbtree_create_simple("SAI Port Pool tree",
            STD_STR_OFFSET_OF(dn_sai_qos_port_pool_t, port_id),
            (STD_STR_SIZE_OF(dn_sai_qos_port_pool_t, port_id) +
             STD_STR_SIZE_OF(dn_sai_qos_port_pool_t, pool_id)));

    if(NULL == port_pool_tree) {
        std_rbtree_destroy(port_pool_object_tree);
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_add_port_pool_node(dn_sai_qos_port_pool_t *p_port_pool_node)
{
    dn_sai_qos_port_t *p_port_node = NULL;
    dn_sai_qos_port_pool_t *p_new_port_pool_node = NULL;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    p_port_node = sai_qos_port_node_get(p_port_pool_node->port_id);
    if(NULL == p_port_node) {
        return SAI_STATUS_FAILURE;
    }

    do {
        if((p_new_port_pool_node = (dn_sai_qos_port_pool_t *)
                    calloc(1, sizeof(dn_sai_qos_port_pool_t))) == NULL) {
            SAI_PORT_LOG_ERR("Unable to allocate memory to add Port Pool 0x%"PRIx64".",
                    p_port_pool_node->port_pool_id);
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        memcpy(p_new_port_pool_node, p_port_pool_node, sizeof(dn_sai_qos_port_pool_t));

        if(std_rbtree_insert(port_pool_object_tree, (void *)p_new_port_pool_node) != STD_ERR_OK) {
            SAI_PORT_LOG_ERR("Unable to add Port Pool 0x%"PRIx64" to RB object tree.",
                    p_port_pool_node->port_pool_id);
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

        if(std_rbtree_insert(port_pool_tree, (void *)p_new_port_pool_node) != STD_ERR_OK) {
            SAI_PORT_LOG_ERR("Unable to add Port Pool 0x%"PRIx64" to RB tree.",
                    p_port_pool_node->port_pool_id);
            std_rbtree_remove(port_pool_object_tree, (void *)p_new_port_pool_node);
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

        std_dll_insertatback(&p_port_node->port_pool_dll_head, &p_new_port_pool_node->port_dll_glue);

        SAI_PORT_LOG_TRACE("Added Port Pool 0x%"PRIx64" to RB tree.",
                p_port_pool_node->port_pool_id);
    } while(0);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        free(p_new_port_pool_node);
    }

    return sai_rc;
}

dn_sai_qos_port_pool_t* sai_qos_port_pool_node_get_from_obj_id(sai_object_id_t port_pool_id)
{
    dn_sai_qos_port_pool_t port_pool_node;

    memset(&port_pool_node, 0, sizeof(dn_sai_qos_port_pool_t));

    port_pool_node.port_pool_id = port_pool_id;

    return (dn_sai_qos_port_pool_t *)std_rbtree_getexact(port_pool_object_tree, (void *)&port_pool_node);
}

dn_sai_qos_port_pool_t* sai_qos_port_pool_node_get(sai_object_id_t port_id, sai_object_id_t pool_id)
{
    dn_sai_qos_port_pool_t port_pool_node;

    memset(&port_pool_node, 0, sizeof(dn_sai_qos_port_pool_t));

    port_pool_node.port_id = port_id;
    port_pool_node.pool_id = pool_id;

    return (dn_sai_qos_port_pool_t *)std_rbtree_getexact(port_pool_tree, (void *)&port_pool_node);
}

sai_status_t sai_qos_remove_port_pool_node(sai_object_id_t port_pool_id)
{
    dn_sai_qos_port_t *p_port_node = NULL;
    dn_sai_qos_port_pool_t port_pool_node, *p_port_pool_node = NULL;

    p_port_pool_node = sai_qos_port_pool_node_get_from_obj_id(port_pool_id);
    if(NULL == p_port_pool_node) {
        SAI_PORT_LOG_ERR("Port Pool 0x%"PRIx64" not found in DB.", port_pool_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    p_port_node = sai_qos_port_node_get(p_port_pool_node->port_id);
    if(NULL == p_port_node) {
        return SAI_STATUS_FAILURE;
    }

    memcpy(&port_pool_node, p_port_pool_node, sizeof(dn_sai_qos_port_pool_t));

    p_port_pool_node =
        (dn_sai_qos_port_pool_t *)std_rbtree_remove(port_pool_object_tree, (void *)&port_pool_node);
    if(NULL == p_port_pool_node) {
        SAI_PORT_LOG_ERR("Port Pool 0x%"PRIx64" remove from RB object tree failed.", port_pool_id);
        return SAI_STATUS_FAILURE;
    }

    p_port_pool_node = NULL;
    p_port_pool_node =
        (dn_sai_qos_port_pool_t *)std_rbtree_remove(port_pool_tree, (void *)&port_pool_node);
    if(NULL == p_port_pool_node) {
        SAI_PORT_LOG_ERR("Port Pool 0x%"PRIx64" remove from RB tree failed.", port_pool_id);
        return SAI_STATUS_FAILURE;
    }

    std_dll_remove(&p_port_node->port_pool_dll_head, &p_port_pool_node->port_dll_glue);

    SAI_PORT_LOG_TRACE("Removed Port Pool 0x%"PRIx64" from RB tree.",
            p_port_pool_node->port_pool_id);

    free(p_port_pool_node);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_port_pool_get_wred_queue_ids(sai_object_id_t port_pool_id,
        sai_object_list_t *p_queue_list)
{
    dn_sai_qos_port_pool_t *p_port_pool_node = NULL;
    dn_sai_qos_port_t           *p_port_node = NULL;
    dn_sai_qos_queue_t          *p_queue_node = NULL;
    dn_sai_qos_buffer_profile_t *p_buffer_profile_node = NULL;
    uint_t                      num = 0;

    p_port_pool_node = sai_qos_port_pool_node_get_from_obj_id(port_pool_id);
    if(NULL == p_port_pool_node) {
        SAI_WRED_LOG_ERR("Port pool 0x%"PRIx64" not found in DB.",port_pool_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    p_port_node = sai_qos_port_node_get(p_port_pool_node->port_id);
    if(NULL != p_port_node) {
        for (p_queue_node = sai_qos_port_get_first_queue(p_port_node);
                p_queue_node != NULL;
                p_queue_node = sai_qos_port_get_next_queue(p_port_node, p_queue_node)) {
            if(sai_qos_is_queue_type_ucast(p_queue_node->queue_type)) {
                p_buffer_profile_node = sai_qos_buffer_profile_node_get(p_queue_node->buffer_profile_id);
                if((p_buffer_profile_node != NULL) &&
                        (p_buffer_profile_node->buffer_pool_id == p_port_pool_node->pool_id)) {
                    p_queue_list->list[num] = p_queue_node->key.queue_id;
                    num++;

                    if(num == p_queue_list->count) {
                        return SAI_STATUS_SUCCESS;
                    }
                }
            }
        }
    } else {
        SAI_WRED_LOG_ERR("QOS Port node is NULL for Port 0x%"PRIx64"",p_port_pool_node->port_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    p_queue_list->count = num;

    return SAI_STATUS_SUCCESS;
}

dn_sai_qos_port_pool_t *sai_qos_port_pool_get_first_node(dn_sai_qos_port_t *p_port_node)
{
    return ((dn_sai_qos_port_pool_t *)std_dll_getfirst(&p_port_node->port_pool_dll_head));
}

dn_sai_qos_port_pool_t *sai_qos_port_pool_get_next_node(dn_sai_qos_port_t *p_port_node,
        dn_sai_qos_port_pool_t *p_port_pool_node)
{
    return ((dn_sai_qos_port_pool_t *)std_dll_getnext(&p_port_node->port_pool_dll_head,
                (std_dll *)p_port_pool_node));
}
