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
* @file sai_qos_buffer_util.c
*
* @brief This file contains the util functions for SAI Qos buffer component.
*
*************************************************************************/

#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_buffer_util.h"
#include "std_assert.h"
#include "std_assert.h"
#include "sai_oid_utils.h"
#include "sai_gen_utils.h"

#include "saitypes.h"
#include "saistatus.h"

#include "std_type_defs.h"
#include "std_assert.h"
#include "std_llist.h"
#include "std_struct_utils.h"

#include <string.h>
#include <inttypes.h>
#include <stdio.h>

/** Offset of the buffer profile dll glue in the qos port data structure */
#define SAI_QOS_PORT_BUFFER_PROFILE_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF (dn_sai_qos_port_t, buffer_profile_dll_glue)

/** Offset of the buffer profile dll glue in the qos queue data structure */
#define SAI_QOS_QUEUE_BUFFER_PROFILE_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF (dn_sai_qos_queue_t, buffer_profile_dll_glue)

/** Offset of the buffer pool dll glue in the qos buffer profile data structure */
#define SAI_QOS_BUFFER_PROFILE_POOL_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF (dn_sai_qos_buffer_profile_t, buffer_pool_dll_glue)

/** Offset of the buffer profile dll glue in the qos pg data structure */
#define SAI_QOS_PG_BUFFER_PROFILE_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF (dn_sai_qos_pg_t, buffer_profile_dll_glue)

/** Offset of the port dll glue in the qos pg data structure */
#define SAI_QOS_PG_PORT_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF (dn_sai_qos_pg_t, port_dll_glue)

dn_sai_qos_buffer_pool_t *sai_qos_buffer_pool_node_get (sai_object_id_t buffer_pool_id)
{
    rbtree_handle                 buffer_pool_tree;
    dn_sai_qos_buffer_pool_t      buffer_pool_node;

    memset (&buffer_pool_node, 0, sizeof (dn_sai_qos_buffer_pool_t));

    buffer_pool_node.key.pool_id = buffer_pool_id;

    buffer_pool_tree = sai_qos_access_global_config()->buffer_pool_tree;

    if (NULL == buffer_pool_tree)
        return NULL;

    return ((dn_sai_qos_buffer_pool_t *) std_rbtree_getexact (buffer_pool_tree, &buffer_pool_node));
}

dn_sai_qos_buffer_profile_t *sai_qos_buffer_profile_node_get (sai_object_id_t buffer_profile_id)
{
    rbtree_handle                    buffer_profile_tree;
    dn_sai_qos_buffer_profile_t      buffer_profile_node;

    memset (&buffer_profile_node, 0, sizeof (dn_sai_qos_buffer_profile_t));

    buffer_profile_node.key.profile_id = buffer_profile_id;

    buffer_profile_tree = sai_qos_access_global_config()->buffer_profile_tree;

    if (NULL == buffer_profile_tree)
        return NULL;

    return ((dn_sai_qos_buffer_profile_t *) std_rbtree_getexact (buffer_profile_tree, &buffer_profile_node));
}


dn_sai_qos_queue_t *sai_qos_buffer_profile_get_first_queue (dn_sai_qos_buffer_profile_t
                                                            *p_qos_buffer_profile_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
                   sai_qos_dll_get_first (&p_qos_buffer_profile_node->queue_dll_head)))) {
        return ((dn_sai_qos_queue_t *) (p_temp -
                                        SAI_QOS_QUEUE_BUFFER_PROFILE_DLL_GLUE_OFFSET));
    }
    return NULL;
}

dn_sai_qos_queue_t *sai_qos_buffer_profile_get_next_queue (dn_sai_qos_buffer_profile_t
                                                           *p_qos_buffer_profile_node,
                                                           dn_sai_qos_queue_t *p_queue_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next (&p_qos_buffer_profile_node->queue_dll_head,
                                                     &p_queue_node->buffer_profile_dll_glue)))) {
        return ((dn_sai_qos_queue_t *) (p_temp -
                                        SAI_QOS_QUEUE_BUFFER_PROFILE_DLL_GLUE_OFFSET));
    }
    return NULL;
}

dn_sai_qos_port_t *sai_qos_buffer_profile_get_first_port (dn_sai_qos_buffer_profile_t
                                                          *p_qos_buffer_profile_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
                   sai_qos_dll_get_first (&p_qos_buffer_profile_node->port_dll_head)))) {
        return ((dn_sai_qos_port_t *) (p_temp -
                                        SAI_QOS_PORT_BUFFER_PROFILE_DLL_GLUE_OFFSET));
    }
    return NULL;
}

dn_sai_qos_port_t *sai_qos_buffer_profile_get_next_port (dn_sai_qos_buffer_profile_t
                                                         *p_qos_buffer_profile_node,
                                                         dn_sai_qos_port_t *p_port_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next (&p_qos_buffer_profile_node->port_dll_head,
                                                     &p_port_node->buffer_profile_dll_glue)))) {
        return ((dn_sai_qos_port_t *) (p_temp -
                                        SAI_QOS_PORT_BUFFER_PROFILE_DLL_GLUE_OFFSET));
    }
    return NULL;
}

dn_sai_qos_buffer_profile_t *sai_qos_buffer_pool_get_first_buffer_profile (dn_sai_qos_buffer_pool_t
                                                                           *p_qos_buffer_pool_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
                   sai_qos_dll_get_first (&p_qos_buffer_pool_node->buffer_profile_dll_head)))) {
        return ((dn_sai_qos_buffer_profile_t *) (p_temp -
                                        SAI_QOS_BUFFER_PROFILE_POOL_DLL_GLUE_OFFSET));
    }
    return NULL;
}

dn_sai_qos_buffer_profile_t *sai_qos_buffer_pool_get_next_buffer_profile (dn_sai_qos_buffer_pool_t
                                                                          *p_qos_buffer_pool_node,
                                                                          dn_sai_qos_buffer_profile_t
                                                                          *p_buffer_profile_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next (&p_qos_buffer_pool_node->buffer_profile_dll_head,
                                                     &p_buffer_profile_node->buffer_pool_dll_glue)))) {
        return ((dn_sai_qos_buffer_profile_t *) (p_temp -
                                        SAI_QOS_BUFFER_PROFILE_POOL_DLL_GLUE_OFFSET));
    }
    return NULL;
}

sai_status_t sai_qos_get_buffer_profile_id (sai_object_id_t obj_id,
                                            sai_object_id_t *profile_id)
{
     sai_object_type_t   obj_type = sai_uoid_obj_type_get (obj_id);
     dn_sai_qos_queue_t *queue_node = NULL;
     dn_sai_qos_port_t  *port_node = NULL;
     dn_sai_qos_pg_t    *pg_node = NULL;

     switch(obj_type) {
         case SAI_OBJECT_TYPE_PORT:
             port_node = sai_qos_port_node_get(obj_id);
             if(port_node == NULL ) {
                 SAI_BUFFER_LOG_ERR ("Error port obj not found for id 0x%"PRIx64"",
                         obj_id);
                 return SAI_STATUS_ITEM_NOT_FOUND;
             }
             *profile_id = port_node->buffer_profile_id;
             break;

         case SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP:
             pg_node = sai_qos_pg_node_get(obj_id);
             if(pg_node == NULL ) {
                 SAI_BUFFER_LOG_ERR ("Error pg obj not found for id 0x%"PRIx64"",
                         obj_id);
                 return SAI_STATUS_ITEM_NOT_FOUND;
             }
             *profile_id = pg_node->buffer_profile_id;
             break;

         case SAI_OBJECT_TYPE_QUEUE:
             queue_node = sai_qos_queue_node_get(obj_id);
             if(queue_node == NULL ) {
                 SAI_BUFFER_LOG_ERR ("Error queue obj not found for id 0x%"PRIx64"",
                         obj_id);
                 return SAI_STATUS_ITEM_NOT_FOUND;
             }
             *profile_id = queue_node->buffer_profile_id;
             break;

         default:
             return SAI_STATUS_INVALID_OBJECT_TYPE;
     }
     return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_get_buffer_pool_id (sai_object_id_t obj_id,
                                         sai_object_id_t *pool_id)
{
    sai_object_id_t profile_id;
    dn_sai_qos_buffer_profile_t *p_buffer_profile_node = NULL;
    sai_status_t sai_rc;

    sai_rc = sai_qos_get_buffer_profile_id(obj_id, &profile_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    if(profile_id == SAI_NULL_OBJECT_ID) {
        *pool_id = SAI_NULL_OBJECT_ID;
    } else {
        p_buffer_profile_node = sai_qos_buffer_profile_node_get(profile_id);

        if(p_buffer_profile_node == NULL) {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
        *pool_id = p_buffer_profile_node->buffer_pool_id;
    }
    return SAI_STATUS_SUCCESS;
}


sai_status_t sai_qos_get_th_mode (dn_sai_qos_buffer_profile_t *p_profile_node,
                                  sai_buffer_profile_threshold_mode_t *th_mode)
{
    dn_sai_qos_buffer_pool_t *p_pool_node = NULL;

    STD_ASSERT (p_profile_node != NULL);
    if (p_profile_node->profile_th_enable) {
        *th_mode = p_profile_node->threshold_mode;

    } else {
        p_pool_node = sai_qos_buffer_pool_node_get(p_profile_node->buffer_pool_id);

        if(p_pool_node == NULL) {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
        *th_mode = p_pool_node->threshold_mode;

    }
    return SAI_STATUS_SUCCESS;
}

dn_sai_qos_pg_t *sai_qos_pg_node_get (sai_object_id_t pg_id)
{
    rbtree_handle        pg_tree;
    dn_sai_qos_pg_t      pg_node;

    memset (&pg_node, 0, sizeof (dn_sai_qos_pg_t));

    pg_node.key.pg_id = pg_id;

    pg_tree = sai_qos_access_global_config()->pg_tree;

    if (NULL == pg_tree)
        return NULL;

    return ((dn_sai_qos_pg_t *) std_rbtree_getexact (pg_tree, &pg_node));
}

dn_sai_qos_pg_t *sai_qos_buffer_profile_get_first_pg (dn_sai_qos_buffer_profile_t
                                                      *p_qos_buffer_profile_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
                   sai_qos_dll_get_first (&p_qos_buffer_profile_node->pg_dll_head)))) {
        return ((dn_sai_qos_pg_t *) (p_temp -
                                        SAI_QOS_PG_BUFFER_PROFILE_DLL_GLUE_OFFSET));
    }
    return NULL;
}

dn_sai_qos_pg_t *sai_qos_buffer_profile_get_next_pg (dn_sai_qos_buffer_profile_t
                                                     *p_qos_buffer_profile_node,
                                                     dn_sai_qos_pg_t *p_pg_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next (&p_qos_buffer_profile_node->pg_dll_head,
                                                     &p_pg_node->buffer_profile_dll_glue)))) {
        return ((dn_sai_qos_pg_t *) (p_temp -
                                        SAI_QOS_PG_BUFFER_PROFILE_DLL_GLUE_OFFSET));
    }
    return NULL;
}

dn_sai_qos_pg_t *sai_qos_port_get_first_pg (dn_sai_qos_port_t  *p_qos_port_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
                   sai_qos_dll_get_first (&p_qos_port_node->pg_dll_head)))) {
        return ((dn_sai_qos_pg_t *) (p_temp -
                                        SAI_QOS_PG_PORT_DLL_GLUE_OFFSET));
    }
    return NULL;
}

dn_sai_qos_pg_t *sai_qos_port_get_next_pg (dn_sai_qos_port_t  *p_qos_port_node,
                                           dn_sai_qos_pg_t *p_pg_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next (&p_qos_port_node->pg_dll_head,
                                                     &p_pg_node->port_dll_glue)))) {
        return ((dn_sai_qos_pg_t *) (p_temp -
                                        SAI_QOS_PG_PORT_DLL_GLUE_OFFSET));
    }
    return NULL;
}

sai_status_t sai_qos_port_get_num_pg (sai_object_id_t port_id, uint_t *num_pg)
{
    dn_sai_qos_port_t  *port_node = NULL;

    STD_ASSERT (num_pg != NULL);
    port_node = sai_qos_port_node_get(port_id);

    if(port_node == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *num_pg = port_node->num_pg;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_port_get_pg_list (sai_object_id_t port_id,
                                       sai_object_list_t *pg_list)
{
    dn_sai_qos_port_t  *port_node = NULL;
    dn_sai_qos_pg_t  *pg_node = NULL;
    uint_t pg_idx = 0;

    STD_ASSERT (pg_list != NULL);
    port_node = sai_qos_port_node_get(port_id);

    if(port_node == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if(pg_list->count < port_node->num_pg) {
        pg_list->count = port_node->num_pg;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    pg_list->count = port_node->num_pg;

    pg_node = sai_qos_port_get_first_pg (port_node);

    while(pg_node != NULL) {

        if(pg_idx > pg_list->count) {
            return SAI_STATUS_FAILURE;
        }

        pg_list->list[pg_idx] = pg_node->key.pg_id;

        pg_node = sai_qos_port_get_next_pg(port_node, pg_node);
        pg_idx++;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_buffer_pool_get_first_pg_id (sai_object_id_t pool_id,
                                                  sai_object_id_t *pg_id)
{
    dn_sai_qos_buffer_pool_t *p_pool_node = NULL;
    dn_sai_qos_buffer_profile_t *p_profile_node;
    dn_sai_qos_pg_t             *pg_node;

    p_pool_node = sai_qos_buffer_pool_node_get(pool_id);

    if (p_pool_node == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (p_pool_node->pool_type != SAI_BUFFER_POOL_TYPE_INGRESS) {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    for (p_profile_node = sai_qos_buffer_pool_get_first_buffer_profile(p_pool_node);
        p_profile_node != NULL; p_profile_node =
        sai_qos_buffer_pool_get_next_buffer_profile(p_pool_node, p_profile_node)) {

        pg_node = sai_qos_buffer_profile_get_first_pg(p_profile_node);

        if (pg_node != NULL) {
            *pg_id = pg_node->key.pg_id;
            return SAI_STATUS_SUCCESS;
        }
    }
    return SAI_STATUS_FAILURE;
}

sai_status_t sai_qos_buffer_pool_get_first_queue_id (sai_object_id_t pool_id,
                                                     sai_object_id_t *queue_id)
{
    dn_sai_qos_buffer_pool_t *p_pool_node = NULL;
    dn_sai_qos_buffer_profile_t *p_profile_node;
    dn_sai_qos_queue_t             *queue_node;

    p_pool_node = sai_qos_buffer_pool_node_get(pool_id);

    if (p_pool_node == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (p_pool_node->pool_type != SAI_BUFFER_POOL_TYPE_EGRESS) {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (p_profile_node = sai_qos_buffer_pool_get_first_buffer_profile(p_pool_node);
        p_profile_node != NULL; p_profile_node =
        sai_qos_buffer_pool_get_next_buffer_profile(p_pool_node, p_profile_node)) {

        queue_node = sai_qos_buffer_profile_get_first_queue(p_profile_node);

        if (queue_node != NULL) {
            *queue_id = queue_node->key.queue_id;
            return SAI_STATUS_SUCCESS;
        }
    }
    return SAI_STATUS_FAILURE;
}

bool sai_qos_is_buffer_pool_xoff_size_configured (sai_object_id_t pool_id)
{
    dn_sai_qos_buffer_pool_t *p_buffer_pool_node = NULL;
    p_buffer_pool_node = sai_qos_buffer_pool_node_get(pool_id);

    if (p_buffer_pool_node != NULL) {
        if (p_buffer_pool_node->xoff_size > 0)
            return true;
    }
    return false;
}

uint_t sai_qos_buffer_profile_get_reserved_xoff_th_get (const dn_sai_qos_buffer_profile_t
                                                      *p_qos_buffer_profile_node)
{
    dn_sai_qos_buffer_pool_t *p_buffer_pool_node = NULL;

    STD_ASSERT (p_qos_buffer_profile_node != NULL);

    if (p_qos_buffer_profile_node->buffer_pool_id != SAI_NULL_OBJECT_ID) {
        p_buffer_pool_node =
            sai_qos_buffer_pool_node_get(p_qos_buffer_profile_node->buffer_pool_id);

        STD_ASSERT (p_buffer_pool_node != NULL);

        if (p_buffer_pool_node->xoff_size > 0)
            return 0;
    }

    return p_qos_buffer_profile_node->xoff_th;
}

sai_status_t sai_qos_buffer_pool_get_wred_queue_ids(sai_object_id_t pool_id,
        sai_object_list_t *p_queue_list)
{
    dn_sai_qos_buffer_pool_t    *p_pool_node = NULL;
    dn_sai_qos_buffer_profile_t *p_profile_node;
    dn_sai_qos_queue_t          *p_queue_node;
    uint_t                      num = 0;

    p_pool_node = sai_qos_buffer_pool_node_get(pool_id);

    if(p_pool_node == NULL) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for(p_profile_node = sai_qos_buffer_pool_get_first_buffer_profile(p_pool_node);
            p_profile_node != NULL;
            p_profile_node = sai_qos_buffer_pool_get_next_buffer_profile(p_pool_node, p_profile_node)) {
        for(p_queue_node = sai_qos_buffer_profile_get_first_queue(p_profile_node);
                p_queue_node != NULL;
                p_queue_node = sai_qos_buffer_profile_get_next_queue(p_profile_node, p_queue_node)) {
            if((sai_qos_is_queue_type_ucast(p_queue_node->queue_type)) &&
                    (sai_is_obj_id_logical_port(p_queue_node->port_id))) {
                p_queue_list->list[num] = p_queue_node->key.queue_id;
                num++;

                if(num == p_queue_list->count) {
                    return SAI_STATUS_SUCCESS;
                }
            }
        }
    }

    p_queue_list->count = num;

    return SAI_STATUS_SUCCESS;
}
