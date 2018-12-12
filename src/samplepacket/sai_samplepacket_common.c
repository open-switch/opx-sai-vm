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
* @file sai_samplepacket_common.c
*
* @brief This file contains public API handling for the SAI samplepacket
*        functionality
*
*************************************************************************/
#include "saitypes.h"
#include "saistatus.h"

#include "sai_samplepacket_defs.h"
#include "sai_samplepacket_api.h"
#include "sai_samplepacket_util.h"
#include "sai_npu_samplepacket.h"
#include "sai_samplepacket_api.h"
#include "sai_port_common.h"
#include "sai_port_utils.h"
#include "sai_port_main.h"
#include "sai_switch_utils.h"
#include "sai_common_utils.h"
#include "sai_common_infra.h"

#include "std_rbtree.h"
#include "std_assert.h"
#include "std_type_defs.h"
#include "stdbool.h"
#include "std_struct_utils.h"

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

static rbtree_handle samplepacket_sessions_tree = NULL;

rbtree_handle sai_samplepacket_sessions_db_get (void)
{
    return samplepacket_sessions_tree;
}

static inline uint64_t sai_samplepacket_bitmap (void)
{
    return (sai_attribute_bit_set(SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE));
}


static inline bool sai_samplepacket_is_mandat_attribute_missing (uint32_t req_attributes)
{
    bool result = false;
    result = ((req_attributes & (sai_samplepacket_bitmap())) ^ (sai_samplepacket_bitmap()));

    return result;
}


static sai_status_t sai_samplepacket_fill_attribute_params (
            dn_sai_samplepacket_session_info_t *p_session_info,
            const sai_attribute_t *attr_list,
            uint32_t attr_index)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    switch (attr_list->id) {
        case SAI_SAMPLEPACKET_ATTR_TYPE:
            if (attr_list->value.s32 != SAI_SAMPLEPACKET_TYPE_SLOW_PATH) {
                SAI_SAMPLEPACKET_LOG_ERR ("Invalid samplepacket type");
                rc = SAI_STATUS_CODE((abs)SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index);
                break;
            }
            p_session_info->sampling_type = attr_list->value.s32;
            break;

        case SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE:
            p_session_info->sample_rate = attr_list->value.u32;
            break;

        default:
            rc = SAI_STATUS_CODE((abs)SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + attr_index);
            SAI_SAMPLEPACKET_LOG_ERR("Attribute id %d is not valid", attr_list->id);
            break;
    }

    return rc;
}

static sai_status_t sai_samplepacket_session_create (sai_object_id_t *session_id,
                                              _In_ sai_object_id_t switch_id,
                                              uint32_t attr_count,
                                              const sai_attribute_t *attr_list)
{
    dn_sai_samplepacket_session_info_t *p_session_info          = NULL;
    uint_t                   validate_req_attributes = 0;
    uint_t                   attr_index              = 0;
    sai_status_t               rc                   = SAI_STATUS_SUCCESS;
    sai_npu_object_id_t        npu_object_id           = 0;

    STD_ASSERT (attr_list != NULL);
    STD_ASSERT(attr_count > 0);

    if (sai_samplepacket_sessions_db_get() == NULL) {
        SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket initialization not done");
        return SAI_STATUS_FAILURE;
    }

    sai_samplepacket_lock();

    do {
        p_session_info = sai_samplepacket_session_node_alloc ();
        if (!(p_session_info)) {
            SAI_SAMPLEPACKET_LOG_ERR ("Memory allocation failed");
            rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        for (attr_index = 0; attr_index < attr_count; ++attr_index) {
            /* This is done to check if all the mandatory arguments are set */
            validate_req_attributes = validate_req_attributes |
                                      sai_attribute_bit_set(attr_list->id);

            rc = sai_samplepacket_fill_attribute_params (p_session_info, attr_list, attr_index);

            /* Skip from NPU create if some attributes are invalid or has invalid values */
            if (rc != SAI_STATUS_SUCCESS) {
                break;
            }

            attr_list++;
        }

        if (rc != SAI_STATUS_SUCCESS) {
            SAI_SAMPLEPACKET_LOG_ERR ("Invalid attributes are set");
            break;
        }

        /* Check if all the mandatory attributes are set */
        if (sai_samplepacket_is_mandat_attribute_missing(validate_req_attributes)) {
            SAI_SAMPLEPACKET_LOG_ERR ("Required attributes are not set");
            rc = SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
            break;
        }

        if ((rc = sai_samplepacket_npu_api_get()->session_create(p_session_info, &npu_object_id)) !=
            SAI_STATUS_SUCCESS) {
            SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket session create failed");
            break;
        }

        p_session_info->session_id = sai_uoid_create (SAI_OBJECT_TYPE_SAMPLEPACKET, npu_object_id);

        if (std_rbtree_insert (sai_samplepacket_sessions_db_get(), (void *) p_session_info) != STD_ERR_OK) {
            SAI_SAMPLEPACKET_LOG_ERR ("Node insertion failed for session 0x%"PRIx64"",
                                                                p_session_info->session_id);
            sai_samplepacket_npu_api_get()->session_destroy (p_session_info->session_id);
            rc = SAI_STATUS_FAILURE;
            break;
        }

        p_session_info->port_tree = std_rbtree_create_simple ("port_tree",
                                    STD_STR_OFFSET_OF(dn_sai_samplepacket_port_info_t, key),
                                    STD_STR_SIZE_OF(dn_sai_samplepacket_port_info_t, key));

        if (p_session_info->port_tree == NULL) {
            SAI_SAMPLEPACKET_LOG_ERR ("Source port tree creations failed for session 0x%"PRIx64"",
                                                                p_session_info->session_id);
            std_rbtree_remove (sai_samplepacket_sessions_db_get(), (void *) p_session_info);
            sai_samplepacket_npu_api_get()->session_destroy (p_session_info->session_id);
            rc = SAI_STATUS_FAILURE;
            break;
        }

        *session_id = p_session_info->session_id;
    } while (0);

    if ((p_session_info) && (rc != SAI_STATUS_SUCCESS)) {
        sai_samplepacket_session_node_free ((dn_sai_samplepacket_session_info_t *)p_session_info);
    }
    sai_samplepacket_unlock();

    return rc;
}

static sai_status_t sai_samplepacket_session_remove (sai_object_id_t session_id)
{
    dn_sai_samplepacket_session_info_t *p_session_info = NULL;
    sai_status_t               rc          = SAI_STATUS_SUCCESS;

    SAI_SAMPLEPACKET_LOG_TRACE ("Destroy samplepacket session 0x%"PRIx64"", session_id);

    if (sai_samplepacket_sessions_db_get() == NULL) {
        SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket initialization not done");
        return SAI_STATUS_FAILURE;
    }

    if (!(sai_is_obj_id_samplepkt_session (session_id))) {
        SAI_SAMPLEPACKET_LOG_ERR ("0x%"PRIx64" does not have a valid object type", session_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_samplepacket_lock();
    do {
        p_session_info = (dn_sai_samplepacket_session_info_t *) std_rbtree_getexact (
                                     sai_samplepacket_sessions_db_get(), (void *)&session_id);
        if (!(p_session_info)) {
            SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket session not found 0x%"PRIx64"", session_id);
            rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        /* Check if any source port is still attached to the session */

        if (std_rbtree_getfirst(p_session_info->port_tree) != NULL) {
            SAI_SAMPLEPACKET_LOG_ERR ("Ports are still attached to samplepacket session"
                                "0x%"PRIx64"", session_id);
            rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        if ((rc = sai_samplepacket_npu_api_get()->session_destroy (p_session_info->session_id)) !=
            SAI_STATUS_SUCCESS) {
            SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket session destroy failed 0x%"PRIx64"", session_id);
            break;
        }

        if (p_session_info->port_tree) {
            std_rbtree_destroy (p_session_info->port_tree);
        }

        if (std_rbtree_remove (sai_samplepacket_sessions_db_get() , (void *)p_session_info) !=
                               p_session_info) {
            SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket session node remove failed 0x%"PRIx64"",
                                       session_id);
            rc = SAI_STATUS_FAILURE;
            break;
        }

        sai_samplepacket_session_node_free (p_session_info);

    } while (0);

    sai_samplepacket_unlock();

    return rc;
}

sai_status_t sai_samplepacket_session_port_add (sai_object_id_t session_id,
                                          sai_object_id_t samplepacket_port_id,
                                          sai_samplepacket_direction_t samplepacket_direction,
                                          dn_sai_samplepacket_mode_t sample_mode)
{
    dn_sai_samplepacket_session_info_t *p_session_info = NULL;
    dn_sai_samplepacket_port_info_t    *p_port_node  = NULL;
    sai_status_t               rc          = SAI_STATUS_SUCCESS;
    dn_sai_samplepacket_port_info_t     tmp_port_node;

    memset (&tmp_port_node, 0, sizeof(tmp_port_node));

    if (!(sai_is_obj_id_samplepkt_session (session_id))) {
        SAI_SAMPLEPACKET_LOG_ERR ("0x%"PRIx64" does not have a valid object type", session_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if (sai_samplepacket_sessions_db_get() == NULL) {
        SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket initialization not done");
        return SAI_STATUS_FAILURE;
    }

    SAI_SAMPLEPACKET_LOG_TRACE ("Add port 0x%"PRIx64" to samplepacket session 0x%"PRIx64" in direction %d",
                           samplepacket_port_id, session_id, samplepacket_direction);

    sai_samplepacket_lock();

    do {
        p_session_info = (dn_sai_samplepacket_session_info_t *) std_rbtree_getexact (
                                        sai_samplepacket_sessions_db_get(), (void *)&session_id);
        if (p_session_info == NULL) {
            SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket Session not found 0x%"PRIx64"", session_id);
            rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if (!p_session_info->port_tree) {
            SAI_SAMPLEPACKET_LOG_ERR ("Source ports tree initialization not done");
            rc = SAI_STATUS_FAILURE;
            break;
        }

        tmp_port_node.key.samplepacket_port = samplepacket_port_id;
        tmp_port_node.key.samplepacket_direction   = samplepacket_direction;

        p_port_node = std_rbtree_getexact (p_session_info->port_tree,
                                            (void *) &tmp_port_node);
        if (p_port_node == NULL) {
            p_port_node = sai_samplepacket_port_node_alloc();
            if (p_port_node == NULL) {
                SAI_SAMPLEPACKET_LOG_ERR ("Memory allocation failed for source port 0x%"PRIx64"",
                        samplepacket_port_id);
                rc = SAI_STATUS_NO_MEMORY;
                break;
            }
            p_port_node->key.samplepacket_direction = samplepacket_direction;
            p_port_node->key.samplepacket_port = samplepacket_port_id;
            if (std_rbtree_insert(p_session_info->port_tree, (void *)p_port_node) !=
                    STD_ERR_OK) {
                rc = SAI_STATUS_FAILURE;
                break;
            }
        } else {
            if ((sample_mode == SAI_SAMPLEPACKET_MODE_PORT_BASED) &&
                (p_port_node->sample_mode & SAI_SAMPLEPACKET_MODE_PORT_BASED)) {
                SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket port 0x%"PRIx64" already exists for direction %d",
                        samplepacket_port_id, samplepacket_direction);
                rc = SAI_STATUS_ITEM_ALREADY_EXISTS;
                p_port_node = NULL;
                break;
            } else if ((sample_mode == SAI_SAMPLEPACKET_MODE_FLOW_BASED) &&
                    (p_port_node->sample_mode & SAI_SAMPLEPACKET_MODE_FLOW_BASED)) {
                rc = SAI_STATUS_SUCCESS;
                p_port_node->ref_count++;
                break;
            }
        }

        if (sample_mode == SAI_SAMPLEPACKET_MODE_PORT_BASED) {
            if ((rc = sai_samplepacket_npu_api_get()->session_port_add (p_session_info,
                            samplepacket_port_id, samplepacket_direction)) != SAI_STATUS_SUCCESS) {
                SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket Port addition failed for port 0x%"PRIx64"",
                        samplepacket_port_id);
                if (p_port_node->ref_count == 0) {
                    std_rbtree_remove (p_session_info->port_tree, (void *)p_port_node);
                }
                break;
            }
        } else if (sample_mode == SAI_SAMPLEPACKET_MODE_FLOW_BASED) {
            if ((rc = sai_samplepacket_npu_api_get()->session_acl_port_add (p_session_info,
                            samplepacket_port_id, samplepacket_direction)) != SAI_STATUS_SUCCESS) {
                SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket Acl In Port addition failed for port 0x%"PRIx64"",
                        samplepacket_port_id);
                if (!(p_port_node->sample_mode & SAI_SAMPLEPACKET_MODE_PORT_BASED) &&
                    (p_port_node->ref_count == 0)) {
                    std_rbtree_remove (p_session_info->port_tree, (void *)p_port_node);
                }
                break;
            }
            p_port_node->ref_count++;
        } else {
            rc = SAI_STATUS_FAILURE;
            break;
        }

        p_port_node->sample_mode |= sample_mode;

    } while (0);

    if ((p_port_node) && (rc != SAI_STATUS_SUCCESS)) {
        sai_samplepacket_port_node_free (p_port_node);
    }

    sai_samplepacket_unlock();
    return rc;
}

sai_status_t sai_samplepacket_session_port_remove  (sai_object_id_t session_id,
                                              sai_object_id_t samplepacket_port_id,
                                              sai_samplepacket_direction_t samplepacket_direction,
                                              dn_sai_samplepacket_mode_t sample_mode)
{
    dn_sai_samplepacket_session_info_t *p_session_info = NULL;
    dn_sai_samplepacket_port_info_t    *p_port_node  = NULL;
    sai_status_t               rc          = SAI_STATUS_SUCCESS;
    dn_sai_samplepacket_port_info_t    tmp_port_node;

    memset (&tmp_port_node, 0, sizeof(tmp_port_node));

    if (!(sai_is_obj_id_samplepkt_session (session_id))) {
        SAI_SAMPLEPACKET_LOG_ERR ("0x%"PRIx64" does not have a valid object type", session_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if (sai_samplepacket_sessions_db_get() == NULL) {
        SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket initialization not done");
        return SAI_STATUS_FAILURE;
    }

    sai_samplepacket_lock();
    SAI_SAMPLEPACKET_LOG_TRACE ("Remove port 0x%"PRIx64" from samplepacket session 0x%"PRIx64""
                                "for direction %d",
                                samplepacket_port_id, session_id, samplepacket_direction);

    do {
        p_session_info = (dn_sai_samplepacket_session_info_t *) std_rbtree_getexact (
                                  sai_samplepacket_sessions_db_get(), (void *)&session_id);
        if (p_session_info == NULL) {
            SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket session not found 0x%"PRIx64"", session_id);
            rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if (p_session_info->port_tree == NULL) {
            SAI_SAMPLEPACKET_LOG_ERR ("Source ports tree initialization not done");
            rc = SAI_STATUS_FAILURE;
            break;
        }

        tmp_port_node.key.samplepacket_port = samplepacket_port_id;
        tmp_port_node.key.samplepacket_direction   = samplepacket_direction;

        p_port_node = std_rbtree_getexact (p_session_info->port_tree,
                                            (void *) &tmp_port_node);
        if (p_port_node == NULL) {
            SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket port not found 0x%"PRIx64"", samplepacket_port_id);
            rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }


        if (sample_mode == SAI_SAMPLEPACKET_MODE_PORT_BASED) {
            if ((rc = sai_samplepacket_npu_api_get()->session_port_remove (session_id,
                            samplepacket_port_id,
                            samplepacket_direction)) != SAI_STATUS_SUCCESS) {
                SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket port 0x%"PRIx64" removal from session"
                        "0x%"PRIx64" failed",
                        samplepacket_port_id, session_id);
                break;
            }
        } else if (sample_mode == SAI_SAMPLEPACKET_MODE_FLOW_BASED)  {

            p_port_node->ref_count--;

            if (p_port_node->ref_count == 0) {
                if ((rc = sai_samplepacket_npu_api_get()->session_acl_port_remove (
                                session_id, samplepacket_port_id,
                                samplepacket_direction)) != SAI_STATUS_SUCCESS) {
                    SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket port 0x%"PRIx64" removal from session"
                            "0x%"PRIx64" failed", samplepacket_port_id, session_id);
                    break;
                }
                p_port_node->sample_mode &= ~sample_mode;
            } else {
                rc = SAI_STATUS_OBJECT_IN_USE;
                break;
            }
        }


        if (p_port_node->ref_count == 0) {
            if (std_rbtree_remove (p_session_info->port_tree, (void *)p_port_node) !=
                    p_port_node) {
                SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket port node remove failed 0x%"PRIx64"", samplepacket_port_id);
                rc = SAI_STATUS_FAILURE;
                break;
            }
        }

        sai_samplepacket_port_node_free (p_port_node);

    } while (0);

    sai_samplepacket_unlock();

    return rc;
}

static sai_status_t sai_samplepacket_session_attribute_set (sai_object_id_t session_id,
                                               const sai_attribute_t *attr)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    uint_t attr_index = 0 ;
    dn_sai_samplepacket_session_info_t *p_session_info = NULL;

    STD_ASSERT (attr != NULL);

    if (!(sai_is_obj_id_samplepkt_session (session_id))) {
        SAI_SAMPLEPACKET_LOG_ERR ("0x%"PRIx64" does not have a valid object type", session_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_samplepacket_lock();

    SAI_SAMPLEPACKET_LOG_TRACE ("Attribute set for session_id 0x%"PRIx64"", session_id);

    do {

        p_session_info = (dn_sai_samplepacket_session_info_t *) std_rbtree_getexact (
                                  sai_samplepacket_sessions_db_get(), (void *)&session_id);
        if (p_session_info == NULL) {
            SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket session not found 0x%"PRIx64"", session_id);
            rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        rc = sai_samplepacket_npu_api_get()->session_set (p_session_info, attr);

        if (rc != SAI_STATUS_SUCCESS) {
            SAI_SAMPLEPACKET_LOG_ERR ("NPU Attribute set failed %d", rc);
            break;
        }

        if ((rc = sai_samplepacket_fill_attribute_params (p_session_info, attr, attr_index)) !=
             SAI_STATUS_SUCCESS) {
            SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket fill attribute parameters failed %d", rc);
            break;
        }

    } while (0);

    sai_samplepacket_unlock();

    return rc;
}

static sai_status_t sai_samplepacket_session_attribute_get (sai_object_id_t session_id,
                                               uint32_t attr_count,
                                               sai_attribute_t *attr_list)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    dn_sai_samplepacket_session_info_t *p_session_info = NULL;

    STD_ASSERT(attr_list != NULL);
    STD_ASSERT(attr_count > 0);

    if (!(sai_is_obj_id_samplepkt_session (session_id))) {
        SAI_SAMPLEPACKET_LOG_ERR ("0x%"PRIx64" does not have a valid object type", session_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_samplepacket_lock();
    SAI_SAMPLEPACKET_LOG_TRACE ("Attribute get for session_id 0x%"PRIx64"", session_id);

    do {

        p_session_info = (dn_sai_samplepacket_session_info_t *) std_rbtree_getexact (
                          sai_samplepacket_sessions_db_get(), (void *)&session_id);

        if (p_session_info == NULL) {
            SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket session not found 0x%"PRIx64"", session_id);
            rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        rc = sai_samplepacket_npu_api_get()->session_get (p_session_info, attr_count, attr_list);

    } while (0);

    sai_samplepacket_unlock();
    return rc;
}

sai_status_t sai_samplepacket_init (void)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    sai_samplepacket_mutex_lock_init ();

    samplepacket_sessions_tree = std_rbtree_create_simple ("samplepacket_sessions_tree",
                           STD_STR_OFFSET_OF(dn_sai_samplepacket_session_info_t, session_id),
                           STD_STR_SIZE_OF(dn_sai_samplepacket_session_info_t, session_id));

    if (samplepacket_sessions_tree == NULL) {
        SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket sessions tree create failed");
        return SAI_STATUS_FAILURE;
    }

    ret = sai_acl_samplepacket_init ();
    if (ret != SAI_STATUS_SUCCESS) {
        SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket ACL Initialization failed %d", ret);
    }

    ret = sai_samplepacket_npu_api_get()->samplepacket_init();

    if (ret != SAI_STATUS_SUCCESS) {
        SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket NPU Initialization failed %d", ret);
    }

    return ret;
}

static sai_samplepacket_api_t sai_samplepacket_method_table =
{
    sai_samplepacket_session_create,
    sai_samplepacket_session_remove,
    sai_samplepacket_session_attribute_set,
    sai_samplepacket_session_attribute_get,
};

sai_samplepacket_api_t* sai_samplepacket_api_query(void)
{
    return (&sai_samplepacket_method_table);
}
