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
/**
* @file sai_qos_buffer_common.c
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

sai_status_t sai_qos_buffer_pool_node_insert_to_tree (dn_sai_qos_buffer_pool_t
                                                             *p_buf_pool_node)
{
    rbtree_handle   buf_pool_tree = NULL;
    t_std_error     err_rc = STD_ERR_OK;

    STD_ASSERT (p_buf_pool_node != NULL);

    buf_pool_tree = sai_qos_access_global_config()->buffer_pool_tree;

    STD_ASSERT (buf_pool_tree != NULL);

    err_rc = std_rbtree_insert (buf_pool_tree, p_buf_pool_node);

    if (STD_IS_ERR(err_rc)) {
        SAI_BUFFER_LOG_ERR ("Failed to insert buffer pool node for "
                               " buffer pool ID 0x%"PRIx64" into buffer pool Tree",
                               p_buf_pool_node->key.pool_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}


void sai_qos_buffer_pool_node_remove_from_tree (dn_sai_qos_buffer_pool_t
                                                *p_buf_pool_node)
{
    rbtree_handle   buf_pool_tree = NULL;

    STD_ASSERT (p_buf_pool_node != NULL);

    buf_pool_tree = sai_qos_access_global_config()->buffer_pool_tree;

    STD_ASSERT (buf_pool_tree != NULL);

    std_rbtree_remove (buf_pool_tree, p_buf_pool_node);

}

sai_status_t sai_qos_buffer_profile_node_insert_to_tree (dn_sai_qos_buffer_profile_t
                                                         *p_buf_profile_node)
{
    rbtree_handle   buf_profile_tree = NULL;
    t_std_error     err_rc = STD_ERR_OK;

    STD_ASSERT (p_buf_profile_node != NULL);

    buf_profile_tree = sai_qos_access_global_config()->buffer_profile_tree;

    STD_ASSERT (buf_profile_tree != NULL);

    err_rc = std_rbtree_insert (buf_profile_tree, p_buf_profile_node);

    if (STD_IS_ERR(err_rc)) {
        SAI_BUFFER_LOG_ERR ("Failed to insert buffer profile node for "
                               " buffer profile ID 0x%"PRIx64" into buffer profile Tree",
                               p_buf_profile_node->key.profile_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_buffer_profile_node_remove_from_tree (dn_sai_qos_buffer_profile_t
                                                           *p_buf_profile_node)
{
    rbtree_handle   buf_profile_tree = NULL;
    dn_sai_qos_buffer_pool_t *p_buf_pool_node = NULL;

    STD_ASSERT (p_buf_profile_node != NULL);

    if(p_buf_profile_node->num_ref != 0) {
        SAI_BUFFER_LOG_ERR ("Buffer profile in use. Num ref %u",
                            p_buf_profile_node->num_ref);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    p_buf_pool_node = sai_qos_buffer_pool_node_get (p_buf_profile_node->buffer_pool_id);

    if(p_buf_pool_node != NULL) {
        std_dll_remove(&p_buf_pool_node->buffer_profile_dll_head,
                &p_buf_profile_node->buffer_pool_dll_glue);
        p_buf_pool_node->num_ref--;
    }

    buf_profile_tree = sai_qos_access_global_config()->buffer_profile_tree;

    STD_ASSERT (buf_profile_tree != NULL);

    std_rbtree_remove (buf_profile_tree, p_buf_profile_node);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_validate_shared_buffer_pool_size(
                              dn_sai_qos_buffer_pool_t  *p_old_buf_pool_node,
                              dn_sai_qos_buffer_pool_t  *p_buf_pool_node)
{
    uint_t cur_size = 0;
    uint_t cur_xoff_size = 0;
    int    shared_size = 0;

    STD_ASSERT(p_buf_pool_node != NULL);

    if ((p_buf_pool_node->xoff_size > 0) &&
        (p_buf_pool_node->pool_type != SAI_BUFFER_POOL_TYPE_INGRESS)) {
        SAI_BUFFER_LOG_ERR("SAI_BUFFER_POOL_XOFF_SIZE valid only for pool type %d",
                            p_buf_pool_node->pool_type);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (p_buf_pool_node->xoff_size > p_buf_pool_node->size)
        return SAI_STATUS_INVALID_PARAMETER;

    if (p_old_buf_pool_node != NULL) {
        cur_size = p_old_buf_pool_node->size;
        cur_xoff_size = p_old_buf_pool_node->xoff_size;
    }

    shared_size = p_buf_pool_node->shared_size + p_buf_pool_node->size - cur_size;
    shared_size = shared_size - p_buf_pool_node->xoff_size + cur_xoff_size;

    if (shared_size < 0) {
        SAI_BUFFER_LOG_ERR("Insufficient memory in pool.");
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    p_buf_pool_node->shared_size = shared_size;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_update_buffer_pool_node(dn_sai_qos_buffer_pool_t  *p_buf_pool_node,
                                             uint32_t attr_count,
                                             const sai_attribute_t *attr_list)
{
    uint_t attr_idx;
    bool   is_xoff_size_attr = false;
    uint_t xoff_size_attr_idx = 0;

    STD_ASSERT(p_buf_pool_node != NULL);
    STD_ASSERT(attr_list != NULL);

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        switch(attr_list[attr_idx].id) {
            case SAI_BUFFER_POOL_ATTR_TYPE:
                p_buf_pool_node->pool_type = attr_list[attr_idx].value.s32;
                break;

            case SAI_BUFFER_POOL_ATTR_SIZE:
                p_buf_pool_node->size = attr_list[attr_idx].value.u32;
                break;

            case SAI_BUFFER_POOL_ATTR_XOFF_SIZE:
                p_buf_pool_node->xoff_size = attr_list[attr_idx].value.u32;
                is_xoff_size_attr = true;
                xoff_size_attr_idx = attr_idx;
                break;

            case SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE:
                p_buf_pool_node->threshold_mode = attr_list[attr_idx].value.s32;
                break;

            case SAI_BUFFER_POOL_ATTR_SHARED_SIZE:
                return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0, attr_idx);

            case SAI_BUFFER_POOL_ATTR_WRED_PROFILE_ID:
                p_buf_pool_node->wred_id = attr_list[attr_idx].value.oid;
                break;

            default:
                return sai_get_indexed_ret_val(SAI_STATUS_UNKNOWN_ATTRIBUTE_0, attr_idx);
        }
    }

    if (is_xoff_size_attr && (p_buf_pool_node->pool_type == SAI_BUFFER_POOL_TYPE_EGRESS)) {
        return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0, xoff_size_attr_idx);
    }
    return SAI_STATUS_SUCCESS;
}

void sai_qos_init_buffer_profile_node(dn_sai_qos_buffer_profile_t  *p_buf_profile_node)
{
    STD_ASSERT(p_buf_profile_node != NULL);

    p_buf_profile_node->profile_th_enable = false;
    std_dll_init(&p_buf_profile_node->pg_dll_head);
    std_dll_init(&p_buf_profile_node->port_dll_head);
    std_dll_init(&p_buf_profile_node->queue_dll_head);
}

void sai_qos_init_buffer_pool_node(dn_sai_qos_buffer_pool_t  *p_buf_pool_node)
{
    STD_ASSERT(p_buf_pool_node != NULL);

    p_buf_pool_node->size = 0;
    p_buf_pool_node->shared_size = 0;
    p_buf_pool_node->threshold_mode = SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC;
    p_buf_pool_node->wred_id = SAI_NULL_OBJECT_ID;

    std_dll_init(&p_buf_pool_node->buffer_profile_dll_head);
}

sai_status_t sai_qos_update_buffer_profile_node(dn_sai_qos_buffer_profile_t  *p_buf_profile_node,
                                                uint32_t attr_count,
                                                const sai_attribute_t *attr_list)
{
    uint_t attr_idx;
    dn_sai_qos_buffer_pool_t *p_new_pool_node = NULL;
    dn_sai_qos_buffer_pool_t *p_old_pool_node = NULL;

    STD_ASSERT(p_buf_profile_node != NULL);
    STD_ASSERT(attr_list != NULL);

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        switch(attr_list[attr_idx].id) {
            case SAI_BUFFER_PROFILE_ATTR_POOL_ID:
                p_new_pool_node =
                    sai_qos_buffer_pool_node_get(attr_list[attr_idx].value.oid);

                if (p_new_pool_node == NULL) {
                    SAI_BUFFER_LOG_ERR ("Buffer pool object not found");
                    return SAI_STATUS_ITEM_NOT_FOUND;
                }

                if(p_buf_profile_node->buffer_pool_id != SAI_NULL_OBJECT_ID) {
                    p_old_pool_node = sai_qos_buffer_pool_node_get(
                                            p_buf_profile_node->buffer_pool_id);
                    if(p_old_pool_node != NULL) {
                        std_dll_remove(&p_old_pool_node->buffer_profile_dll_head,
                                       &p_buf_profile_node->buffer_pool_dll_glue);
                        p_old_pool_node->num_ref--;
                    }
                }

                p_buf_profile_node->buffer_pool_id = attr_list[attr_idx].value.oid;
                std_dll_insertatback(&p_new_pool_node->buffer_profile_dll_head,
                                     &p_buf_profile_node->buffer_pool_dll_glue);
                p_new_pool_node->num_ref++;
                break;

            case SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE:
                p_buf_profile_node->size = attr_list[attr_idx].value.u32;
                break;

            case SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE:
                p_buf_profile_node->profile_th_enable = true;
                p_buf_profile_node->threshold_mode =  attr_list[attr_idx].value.s32;
                break;

            case SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH:
                p_buf_profile_node->dynamic_th = attr_list[attr_idx].value.s8;
                break;

            case SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH:
                p_buf_profile_node->static_th = attr_list[attr_idx].value.u32;
                break;

            case SAI_BUFFER_PROFILE_ATTR_XOFF_TH:
                p_buf_profile_node->xoff_th = attr_list[attr_idx].value.u32;
                break;

            case SAI_BUFFER_PROFILE_ATTR_XON_TH:
                p_buf_profile_node->xon_th = attr_list[attr_idx].value.u32;
                break;

            default:
                return sai_get_indexed_ret_val(SAI_STATUS_UNKNOWN_ATTRIBUTE_0, attr_idx);
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_read_buffer_profile_node(dn_sai_qos_buffer_profile_t  *p_buf_profile_node,
                                              uint32_t attr_count,
                                              sai_attribute_t *attr_list)
{
    uint_t attr_idx;
    dn_sai_qos_buffer_pool_t  *p_buf_pool_node = NULL;

    STD_ASSERT(p_buf_profile_node != NULL);
    STD_ASSERT(attr_list != NULL);

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        switch(attr_list[attr_idx].id) {
            case SAI_BUFFER_PROFILE_ATTR_POOL_ID:
                attr_list[attr_idx].value.oid = p_buf_profile_node->buffer_pool_id;
                break;

            case SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE:
                attr_list[attr_idx].value.u32 = p_buf_profile_node->size;
                break;

            case SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE:
                if (p_buf_profile_node->profile_th_enable == true) {
                    attr_list[attr_idx].value.s32 = p_buf_profile_node->threshold_mode;
                } else {
                    p_buf_pool_node = sai_qos_buffer_pool_node_get (p_buf_profile_node->buffer_pool_id);
                    if(p_buf_pool_node == NULL) {
                        SAI_BUFFER_LOG_ERR ("No pool is associated to fetch threshold mode");
                        return SAI_STATUS_FAILURE;
                    }
                    attr_list[attr_idx].value.s32 = p_buf_pool_node->threshold_mode;
                }
                break;

            case SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH:
                attr_list[attr_idx].value.s8 = p_buf_profile_node->dynamic_th;
                break;

            case SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH:
                attr_list[attr_idx].value.u32 = p_buf_profile_node->static_th;
                break;

            case SAI_BUFFER_PROFILE_ATTR_XOFF_TH:
                attr_list[attr_idx].value.u32 = p_buf_profile_node->xoff_th;
                break;

            case SAI_BUFFER_PROFILE_ATTR_XON_TH:
                attr_list[attr_idx].value.u32 = p_buf_profile_node->xon_th;
                break;

            default:
                return sai_get_indexed_ret_val(SAI_STATUS_UNKNOWN_ATTRIBUTE_0, attr_idx);
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_obj_update_buffer_profile_node (sai_object_id_t obj_id,
                                                     sai_object_id_t buf_profile_id)
{
     sai_object_type_t   obj_type = sai_uoid_obj_type_get (obj_id);
     dn_sai_qos_queue_t *queue_node = NULL;
     dn_sai_qos_port_t  *port_node = NULL;
     dn_sai_qos_pg_t    *pg_node = NULL;
     dn_sai_qos_buffer_profile_t  *p_buf_profile_node = NULL;
     dn_sai_qos_buffer_profile_t  *p_old_buf_profile_node = NULL;
     std_dll_head       *dll_head = NULL;
     std_dll_head       *old_dll_head = NULL;
     std_dll            *dll_node = NULL;

     if(buf_profile_id != SAI_NULL_OBJECT_ID) {
         p_buf_profile_node = sai_qos_buffer_profile_node_get(buf_profile_id);

         if(p_buf_profile_node == NULL) {
             SAI_BUFFER_LOG_ERR ("Error - Buffer profile object not found for id 0x%"PRIx64"",
                     buf_profile_id);
             return SAI_STATUS_ITEM_NOT_FOUND;
         }
     }

     switch(obj_type) {
         case SAI_OBJECT_TYPE_PORT:
             port_node = sai_qos_port_node_get(obj_id);
             if(port_node == NULL ) {
                 SAI_BUFFER_LOG_ERR ("Error port obj not found for id 0x%"PRIx64"",
                         obj_id);
                 return SAI_STATUS_ITEM_NOT_FOUND;
             }

             if(port_node->buffer_profile_id != SAI_NULL_OBJECT_ID) {
                 p_old_buf_profile_node = sai_qos_buffer_profile_node_get(
                                                      port_node->buffer_profile_id);
                 old_dll_head = &p_old_buf_profile_node->port_dll_head;
             }

             if(p_buf_profile_node) {
                 dll_head = &p_buf_profile_node->port_dll_head;
             }
             dll_node = &port_node->buffer_profile_dll_glue;
             port_node->buffer_profile_id = buf_profile_id;
             break;

         case SAI_OBJECT_TYPE_QUEUE:
             queue_node = sai_qos_queue_node_get(obj_id);
             if(queue_node == NULL ) {
                 SAI_BUFFER_LOG_ERR ("Error queue obj not found for id 0x%"PRIx64"",
                         obj_id);
                 return SAI_STATUS_ITEM_NOT_FOUND;
             }

             if(queue_node->buffer_profile_id != SAI_NULL_OBJECT_ID) {
                 p_old_buf_profile_node = sai_qos_buffer_profile_node_get(
                                                      queue_node->buffer_profile_id);
                 old_dll_head = &p_old_buf_profile_node->queue_dll_head;
             }

             if(p_buf_profile_node) {
                 dll_head = &p_buf_profile_node->queue_dll_head;
             }
             dll_node = &queue_node->buffer_profile_dll_glue;
             queue_node->buffer_profile_id = buf_profile_id;
             break;

         case SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP:
             pg_node = sai_qos_pg_node_get(obj_id);
             if(pg_node == NULL ) {
                 SAI_BUFFER_LOG_ERR ("Error pg obj not found for id 0x%"PRIx64"",
                         obj_id);
                 return SAI_STATUS_ITEM_NOT_FOUND;
             }

             if(pg_node->buffer_profile_id != SAI_NULL_OBJECT_ID) {
                 p_old_buf_profile_node = sai_qos_buffer_profile_node_get(
                                                      pg_node->buffer_profile_id);
                 old_dll_head = &p_old_buf_profile_node->pg_dll_head;
             }

             if(p_buf_profile_node) {
                 dll_head = &p_buf_profile_node->pg_dll_head;
             }
             dll_node = &pg_node->buffer_profile_dll_glue;
             pg_node->buffer_profile_id = buf_profile_id;
             break;

         default:
             return SAI_STATUS_INVALID_OBJECT_TYPE;

     }

     if(old_dll_head != NULL) {
         std_dll_remove (old_dll_head, dll_node);
         p_old_buf_profile_node->num_ref--;
     }

     if(dll_head != NULL) {
         std_dll_insertatback(dll_head, dll_node);
         p_buf_profile_node->num_ref++;
     }
     return SAI_STATUS_SUCCESS;
}

bool sai_qos_is_buffer_pool_available (sai_object_id_t pool_id, uint_t size)
{
    dn_sai_qos_buffer_pool_t *p_pool_node = NULL;

    p_pool_node = sai_qos_buffer_pool_node_get(pool_id);
    if(p_pool_node == NULL) {
        return false;
    }
    if(p_pool_node->shared_size >= size){
        return true;
    }
    return false;
}

void sai_qos_get_default_buffer_profile (dn_sai_qos_buffer_profile_t *p_profile_node)
{
    STD_ASSERT (p_profile_node != NULL);

    p_profile_node->size = 0;
    p_profile_node->dynamic_th = 0;
    p_profile_node->static_th = 0;
    p_profile_node->xon_th = 0;
    p_profile_node->xoff_th = 0;
}


bool sai_qos_is_object_buffer_profile_compatible (sai_object_id_t object_id,
                                                  dn_sai_qos_buffer_profile_t
                                                  *p_profile_node)
{
    dn_sai_qos_buffer_pool_t *p_pool_node = NULL;
    sai_object_type_t obj_type = sai_uoid_obj_type_get (object_id);

    STD_ASSERT (p_profile_node != NULL);
    p_pool_node = sai_qos_buffer_pool_node_get(p_profile_node->buffer_pool_id);

    if(p_pool_node == NULL) {
        return false;
    }

    if(p_pool_node->pool_type == SAI_BUFFER_POOL_TYPE_INGRESS) {
        if((obj_type == SAI_OBJECT_TYPE_PORT) ||
           (obj_type == SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP)) {
            return true;
        }
    }

    if(p_pool_node->pool_type == SAI_BUFFER_POOL_TYPE_EGRESS) {
        if(obj_type == SAI_OBJECT_TYPE_QUEUE) {
            return true;
        }
    }

    return false;
}

bool sai_qos_is_buffer_pool_compatible (sai_object_id_t pool_id_1, sai_object_id_t pool_id_2,
                                        bool check_th)
{
    dn_sai_qos_buffer_pool_t *p_pool_node_1 = NULL;
    dn_sai_qos_buffer_pool_t *p_pool_node_2 = NULL;

    p_pool_node_1 = sai_qos_buffer_pool_node_get(pool_id_1);
    p_pool_node_2 = sai_qos_buffer_pool_node_get(pool_id_2);

    if ((p_pool_node_1 == NULL) || (p_pool_node_2 == NULL)) {
        return false;
    }

    if(p_pool_node_1->pool_type != p_pool_node_2->pool_type) {
        return false;
    }

    if(check_th) {
        if(p_pool_node_1->threshold_mode != p_pool_node_2->threshold_mode) {
            return false;
        }
    }
    return true;
}

bool sai_qos_buffer_profile_is_valid_threshold_applied (dn_sai_qos_buffer_profile_t
                                                        *p_profile_node,
                                                        const sai_attribute_t *attr)
{
    dn_sai_qos_buffer_pool_t *p_pool_node = NULL;

    STD_ASSERT (p_profile_node != NULL);

    p_pool_node = sai_qos_buffer_pool_node_get(p_profile_node->buffer_pool_id);

    if(p_pool_node == NULL) {
        return false;
    }

    if (p_profile_node->profile_th_enable) {
        if ((p_profile_node->threshold_mode == SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC) &&
                (attr->id == SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH)) {
            return true;

        } else if ((p_profile_node->threshold_mode == SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC) &&
                (attr->id == SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH)) {
            return true;

        }
    } else {
        if ((p_pool_node->threshold_mode == SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC) &&
                (attr->id == SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH)) {
            return true;

        } else if ((p_pool_node->threshold_mode == SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC) &&
                (attr->id == SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH)) {
            return true;

        }
    }

    if ((p_pool_node->pool_type == SAI_BUFFER_POOL_TYPE_INGRESS) &&
            ((attr->id == SAI_BUFFER_PROFILE_ATTR_XOFF_TH) ||
             (attr->id == SAI_BUFFER_PROFILE_ATTR_XON_TH))) {
        return true;
    }
    return false;
}

sai_status_t sai_qos_buffer_profile_is_mandatory_th_attr_present(uint32_t attr_count,
                                                                 const sai_attribute_t *attr_list)
{
    uint_t attr_idx;
    dn_sai_qos_buffer_pool_t *p_pool_node = NULL;
    bool is_dynamic = false;
    bool is_static = false;
    bool profile_th_setting = false;
    sai_buffer_profile_threshold_mode_t buffer_profile_th_mode =
        SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC;
    uint_t pool_idx = 0;

    STD_ASSERT(attr_list != NULL);
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        if (attr_list[attr_idx].id == SAI_BUFFER_PROFILE_ATTR_POOL_ID) {
            p_pool_node = sai_qos_buffer_pool_node_get(attr_list[attr_idx].value.oid);
            pool_idx = attr_idx;
        }

        if (attr_list[attr_idx].id == SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH) {
            is_static = true;
        }

        if (attr_list[attr_idx].id == SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH) {
            is_dynamic = true;
        }

        if (attr_list[attr_idx].id == SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE) {
            profile_th_setting = true;
            buffer_profile_th_mode = attr_list[attr_idx].value.s32;
        }
    }

    if(p_pool_node == NULL) {
        return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0, pool_idx);
    }

    if(is_dynamic && is_static) {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!profile_th_setting) {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    } else {
        if (buffer_profile_th_mode == SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC){
            if(!is_static) {
                return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
            }

        } else if (buffer_profile_th_mode == SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC) {
            if(!is_dynamic) {
                return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
            }
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_buffer_profile_is_pfc_threshold_valid (uint32_t attr_count,
                                                            const sai_attribute_t
                                                            *attr_list)
{
    uint_t attr_idx;
    uint_t pfc_attr_idx = 0;
    dn_sai_qos_buffer_pool_t *p_pool_node = NULL;
    bool pfc_th_present = false;

    STD_ASSERT(attr_list != NULL);
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        if (attr_list[attr_idx].id == SAI_BUFFER_PROFILE_ATTR_POOL_ID) {
            p_pool_node = sai_qos_buffer_pool_node_get(attr_list[attr_idx].value.oid);
        }

        if ((attr_list[attr_idx].id == SAI_BUFFER_PROFILE_ATTR_XOFF_TH) ||
            (attr_list[attr_idx].id == SAI_BUFFER_PROFILE_ATTR_XON_TH)) {
            pfc_th_present = true;;
            pfc_attr_idx = attr_idx;
        }
    }

    if(p_pool_node == NULL) {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((pfc_th_present) &&
        (p_pool_node->pool_type == SAI_BUFFER_POOL_TYPE_EGRESS)) {
        return sai_get_indexed_ret_val (SAI_STATUS_INVALID_ATTRIBUTE_0, pfc_attr_idx);
    }

    return SAI_STATUS_SUCCESS;;
}

sai_status_t sai_qos_pg_node_insert_to_tree (dn_sai_qos_pg_t *p_pg_node)
{
    rbtree_handle   pg_tree = NULL;
    t_std_error     err_rc = STD_ERR_OK;

    STD_ASSERT (p_pg_node != NULL);

    pg_tree = sai_qos_access_global_config()->pg_tree;

    STD_ASSERT (pg_tree != NULL);

    err_rc = std_rbtree_insert (pg_tree, p_pg_node);

    if (STD_IS_ERR(err_rc)) {
        SAI_BUFFER_LOG_ERR ("Failed to insert PG node for "
                               " PG ID 0x%"PRIx64" into PG Tree",
                               p_pg_node->key.pg_id);
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

void sai_qos_pg_node_remove_from_tree (dn_sai_qos_pg_t *p_pg_node)
{
    rbtree_handle   pg_tree = NULL;

    STD_ASSERT (p_pg_node != NULL);

    pg_tree = sai_qos_access_global_config()->pg_tree;

    STD_ASSERT (pg_tree != NULL);

    std_rbtree_remove (pg_tree, p_pg_node);

}

