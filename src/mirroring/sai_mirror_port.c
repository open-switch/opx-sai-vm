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
/*
* @file sai_mirror_port.c
*
* @brief This file contains SAI Mirror for port API's.
*
*************************************************************************/
#include "saiswitch.h"
#include "saitypes.h"
#include "saistatus.h"

#include "sai_port_common.h"
#include "sai_port_utils.h"
#include "sai_mirror_defs.h"
#include "sai_mirror_api.h"
#include "sai_mirror_util.h"

#include "std_assert.h"
#include "std_rbtree.h"

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

static sai_status_t sai_mirror_remove_mirror_per_port (sai_object_id_t port_id,
                                     const sai_attribute_t *attr,
                                     sai_mirror_direction_t mirror_direction)
{
    sai_status_t                    ret                   = SAI_STATUS_SUCCESS;
    uint32_t                        session_idx           = 0;
    rbtree_handle                   mirror_per_port_tree  = NULL;
    bool                            is_remove             = false;
    sai_object_id_t                 mirror_session_id     = 0;
    sai_mirror_session_per_port_t  *mirror_session_node   = NULL;
    sai_mirror_session_per_port_t   temp_mirror_session_node;
    sai_port_application_info_t    *port_node             = NULL;

    memset (&temp_mirror_session_node, 0, sizeof (temp_mirror_session_node));

    SAI_MIRROR_LOG_TRACE ("Mirror is enabled in port 0x%"PRIx64" for direction %d", port_id,
                            mirror_direction);

    port_node = sai_port_application_info_create_and_get (port_id);
    if (port_node == NULL) {
        SAI_MIRROR_LOG_ERR ("Port Info is NULL for port 0x%"PRIx64"", port_id);
        return SAI_STATUS_FAILURE;
    } else {
        mirror_per_port_tree = (rbtree_handle) port_node->mirror_sessions_tree;
        if (mirror_per_port_tree == NULL) {
            SAI_MIRROR_LOG_ERR ("Mirror sessions tree is NULL for port 0x%"PRIx64"", port_id);
            return SAI_STATUS_FAILURE;
        }
    }

    mirror_session_node = std_rbtree_getfirst (mirror_per_port_tree);
    while (mirror_session_node)
    {
        temp_mirror_session_node.session_id = mirror_session_node->session_id;
        temp_mirror_session_node.mirror_direction = mirror_session_node->mirror_direction;

        if (mirror_session_node->mirror_direction != mirror_direction)  {
            mirror_session_node = std_rbtree_getnext (mirror_per_port_tree,
                                                     &temp_mirror_session_node);
            continue;
        }
        is_remove = true;

        for (session_idx = 0; session_idx < attr->value.objlist.count;
             session_idx++)
        {
            mirror_session_id = attr->value.objlist.list[session_idx];
            if (mirror_session_node->session_id == mirror_session_id) {
                is_remove = false;
                break;
            }
        }

        if (is_remove) {
            ret = sai_mirror_session_port_remove (mirror_session_node->session_id,
                    port_id, mirror_direction);
            if (ret != SAI_STATUS_SUCCESS) {
                SAI_MIRROR_LOG_ERR("Could not remove port 0x%"PRIx64" from the mirror session 0x%"PRIx64"",
                        port_id, mirror_session_id);
                break;
            } else {
                if (std_rbtree_remove(mirror_per_port_tree,
                            (void *) mirror_session_node) != mirror_session_node) {

                    ret = SAI_STATUS_FAILURE;
                    SAI_MIRROR_LOG_ERR ("Node remove failed 0x%"PRIx64"", mirror_session_node->session_id);
                    break;
                }
                free (mirror_session_node);
            }
        }

        mirror_session_node = std_rbtree_getnext (mirror_per_port_tree, &temp_mirror_session_node);
    }
    if (std_rbtree_getfirst(mirror_per_port_tree) == NULL)
    {
         std_rbtree_destroy (mirror_per_port_tree);
         port_node->mirror_sessions_tree = NULL;
         sai_port_application_info_remove(port_node);
    }

    return ret;
}

sai_status_t sai_mirror_handle_per_port (sai_object_id_t port_id,
                                     const sai_attribute_t *attr,
                                     sai_mirror_direction_t mirror_direction)
{
    sai_status_t                       ret                 = SAI_STATUS_SUCCESS;
    uint32_t                           session_idx         = 0;
    rbtree_handle                      mirror_per_port_tree         = NULL;
    sai_object_id_t                    mirror_session_id   = 0;
    sai_mirror_session_per_port_t     *mirror_session_node = NULL;
    sai_mirror_session_per_port_t      temp_mirror_session_node;
    sai_port_application_info_t       *port_node = NULL;

    memset (&temp_mirror_session_node, 0, sizeof (temp_mirror_session_node));

    SAI_MIRROR_LOG_TRACE ("Mirror is enabled in port 0x%"PRIx64" for direction %d", port_id,
                            mirror_direction);

    port_node = sai_port_application_info_create_and_get (port_id);
    if (port_node == NULL) {
        SAI_MIRROR_LOG_ERR ("Port Info is NULL for port 0x%"PRIx64"", port_id);
        return SAI_STATUS_FAILURE;
    } else {
        if (port_node->mirror_sessions_tree == NULL) {
            SAI_MIRROR_LOG_INFO ("Mirror sessions tree is NULL for port 0x%"PRIx64"", port_id);
            port_node->mirror_sessions_tree = std_rbtree_create_simple (
                    "mirror_sessions_tree_per_port", 0, sizeof(sai_mirror_session_per_port_t));
            if (port_node->mirror_sessions_tree == NULL) {
                SAI_MIRROR_LOG_ERR ("Mirror port tree is NULL for port 0x%"PRIx64"", port_id);
                return SAI_STATUS_FAILURE;
            }
        }
    }
    mirror_per_port_tree = (rbtree_handle) port_node->mirror_sessions_tree;

    /*
     * Walk through the attribute and check whether it is a new session add or
     * update of the sessions and process accordingly.
     */
    for (session_idx = 0; session_idx < attr->value.objlist.count; session_idx++)
    {
        mirror_session_id = attr->value.objlist.list[session_idx];
        temp_mirror_session_node.session_id = mirror_session_id;
        temp_mirror_session_node.mirror_direction = mirror_direction;
        mirror_session_node = std_rbtree_getexact (mirror_per_port_tree,
                (void *) &temp_mirror_session_node);
        if ((mirror_session_node == NULL) || ((mirror_session_node) &&
            (mirror_session_node->mirror_direction != mirror_direction))) {

            /*
             * New session add for the given direction.
             */

            ret = sai_mirror_session_port_add(mirror_session_id,
                    port_id, mirror_direction);
            if (ret != SAI_STATUS_SUCCESS) {
                SAI_MIRROR_LOG_ERR("Could not add port 0x%"PRIx64" to the mirror session 0x%"PRIx64"",
                                    port_id, mirror_session_id);
                break;
            } else {
                mirror_session_node = (sai_mirror_session_per_port_t *)
                                      calloc (1, sizeof (sai_mirror_session_per_port_t));
                if (!mirror_session_node) {
                    SAI_MIRROR_LOG_ERR("Memory allocation failed for mirror info node"
                                       "for port 0x%"PRIx64"", port_id);
                    ret = SAI_STATUS_NO_MEMORY;
                    break;
                }
                mirror_session_node->session_id = mirror_session_id;
                mirror_session_node->mirror_direction = mirror_direction;
                if ((std_rbtree_insert(mirror_per_port_tree,
                                      (void *) mirror_session_node)) != STD_ERR_OK) {

                    ret = SAI_STATUS_FAILURE;
                    SAI_MIRROR_LOG_ERR ("Node insertion failed for session 0x%"PRIx64" direction %d",
                                         mirror_session_node->session_id,
                                         mirror_session_node->mirror_direction);
                    sai_mirror_session_port_remove (mirror_session_node->session_id,
                            port_id, mirror_direction);
                    free (mirror_session_node);
                    break;
                }
            }
        }
    }

    /*
     * Remove the sessions which is not present in the updated list.
     */
    if (ret == SAI_STATUS_SUCCESS) {
        ret = sai_mirror_remove_mirror_per_port (port_id, attr, mirror_direction);
    }

    return ret;
}

