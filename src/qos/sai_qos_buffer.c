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
* @file  sai_qos_buffer.c
*
* @brief This file contains function definitions for SAI QoS Buffer
*        functionality API implementation.
*
*************************************************************************/
#include "sai_npu_qos.h"
#include "sai_npu_switch.h"
#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_buffer_util.h"
#include "sai_qos_api_utils.h"
#include "sai_gen_utils.h"
#include "sai_qos_mem.h"
#include "sai_common_infra.h"
#include "sai_switch_utils.h"
#include "sai_common_infra.h"

#include "sai.h"
#include "saibuffer.h"

#include "std_type_defs.h"
#include "std_utils.h"
#include "std_assert.h"
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>

static sai_status_t sai_qos_buffer_pool_attributes_validate (
                                     uint_t attr_count,
                                     const sai_attribute_t *attr_list,
                                     dn_sai_operations_t op_type)
{
    sai_status_t                    sai_rc = SAI_STATUS_SUCCESS;
    uint_t                          max_vendor_attr_count = 0;
    const dn_sai_attribute_entry_t *p_vendor_attr = NULL;

    SAI_BUFFER_LOG_TRACE ("Parsing attributes for buffer pool, "
                             "attribute count %d op_type %d.", attr_count,
                             op_type);

    if (attr_count == 0) {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(attr_list);

    sai_buffer_npu_api_get()->buffer_pool_attr_table_get(&p_vendor_attr,
                                                              &max_vendor_attr_count);

    STD_ASSERT(p_vendor_attr != NULL);
    STD_ASSERT(max_vendor_attr_count > 0);

    sai_rc = sai_attribute_validate (attr_count, attr_list, p_vendor_attr,
                                     op_type, max_vendor_attr_count);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BUFFER_LOG_ERR ("Attribute validation failed for %d "
                               "operation", op_type);
    }

    return sai_rc;
}

static sai_status_t sai_qos_create_buffer_pool(sai_object_id_t* pool_id,
                                               _In_ sai_object_id_t switch_id,
                                               uint32_t attr_count,
                                               const sai_attribute_t *attr_list)
{
    sai_status_t sai_rc;
    dn_sai_qos_buffer_pool_t  *p_buf_pool_node = NULL;
    bool npu_conf = false;
    sai_object_id_t wred_id = SAI_NULL_OBJECT_ID;

    STD_ASSERT (attr_list != NULL);
    STD_ASSERT (pool_id != NULL);

    sai_rc = sai_qos_buffer_pool_attributes_validate (attr_count, attr_list,
                                                      SAI_OP_CREATE);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    p_buf_pool_node = sai_qos_buffer_pool_node_alloc ();

    if (NULL == p_buf_pool_node) {
        SAI_BUFFER_LOG_CRIT ("Buffer pool node memory allocation failed.");
        return SAI_STATUS_NO_MEMORY;
    }

    sai_qos_init_buffer_pool_node(p_buf_pool_node);

    sai_qos_lock ();

    do {
        sai_rc = sai_qos_update_buffer_pool_node(p_buf_pool_node, attr_count,
                                                 attr_list);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Error unable to update buffer pool node.");
            break;
        }

        if((SAI_NULL_OBJECT_ID != p_buf_pool_node->wred_id) &&
                (SAI_BUFFER_POOL_TYPE_INGRESS == p_buf_pool_node->pool_type)) {
            SAI_BUFFER_LOG_ERR ("WRED can't be applied on Ingress Buffer pool.");
            sai_rc = SAI_STATUS_INVALID_ATTR_VALUE_0;
            break;
        }

        sai_rc = sai_qos_validate_shared_buffer_pool_size(NULL,
                                                          p_buf_pool_node);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }

        sai_rc = sai_buffer_npu_api_get()->buffer_pool_create (p_buf_pool_node,
                                                               pool_id);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Error unable to create buffer pool node.");
            break;
        }

        npu_conf  = true;
        p_buf_pool_node->key.pool_id = *pool_id;

        sai_rc = sai_qos_buffer_pool_node_insert_to_tree(p_buf_pool_node);

        if(SAI_STATUS_SUCCESS == sai_rc) {
            if(SAI_NULL_OBJECT_ID != p_buf_pool_node->wred_id) {
                wred_id = p_buf_pool_node->wred_id;
                p_buf_pool_node->wred_id = SAI_NULL_OBJECT_ID;

                if((sai_rc = sai_qos_wred_link_set(p_buf_pool_node->key.pool_id,
                                wred_id, DN_SAI_QOS_WRED_LINK_BUFFER_POOL)) != SAI_STATUS_SUCCESS) {
                    SAI_BUFFER_LOG_ERR("Failed to link WRED ID 0x%"PRIx64" for Buffer pool 0x%"PRIx64".",
                            p_buf_pool_node->wred_id, p_buf_pool_node->key.pool_id);
                    sai_qos_buffer_pool_node_remove_from_tree(p_buf_pool_node);
                    break;
                }

                p_buf_pool_node->wred_id = wred_id;
            }
        }
    } while(0);

    if( sai_rc != SAI_STATUS_SUCCESS ) {
        if(npu_conf) {
            sai_buffer_npu_api_get()->buffer_pool_remove (p_buf_pool_node);
        }

        sai_qos_buffer_pool_node_free(p_buf_pool_node);
    }

    sai_qos_unlock ();

    return sai_rc;
}

static sai_status_t sai_qos_remove_buffer_pool(sai_object_id_t pool_id)
{
    sai_status_t sai_rc;
    dn_sai_qos_buffer_pool_t  *p_buf_pool_node = NULL;

    sai_qos_lock ();

    do {

        p_buf_pool_node = sai_qos_buffer_pool_node_get (pool_id);

        if (NULL == p_buf_pool_node) {
            SAI_BUFFER_LOG_ERR ("Error buffer pool object not found 0x%"PRIx64"",
                                pool_id);
            sai_rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        if(p_buf_pool_node->num_ref != 0) {
            SAI_BUFFER_LOG_ERR ("Buffer pool in use. Num ref %u",
                                p_buf_pool_node->num_ref);
            sai_rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        sai_rc = sai_buffer_npu_api_get()->buffer_pool_remove (p_buf_pool_node);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Error unable to remove buffer pool .");
            break;
        }

        sai_qos_buffer_pool_node_remove_from_tree(p_buf_pool_node);
        sai_qos_buffer_pool_node_free(p_buf_pool_node);

    } while(0);

    sai_qos_unlock ();

    return sai_rc;
}

static sai_status_t sai_qos_buffer_pool_attr_set (sai_object_id_t pool_id,
                                                  const sai_attribute_t *attr)
{
    dn_sai_qos_buffer_pool_t *p_buf_pool_node = NULL;
    dn_sai_qos_buffer_pool_t  new_buf_pool_node;
    sai_status_t sai_rc = SAI_STATUS_FAILURE;

    STD_ASSERT (attr != NULL);

    sai_rc = sai_qos_buffer_pool_attributes_validate (1, attr, SAI_OP_SET);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    memset(&new_buf_pool_node, 0, sizeof(new_buf_pool_node));
    sai_qos_lock ();

    do {
        p_buf_pool_node = sai_qos_buffer_pool_node_get (pool_id);

        if (p_buf_pool_node == NULL) {
            sai_rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        memcpy(&new_buf_pool_node, p_buf_pool_node, sizeof(dn_sai_qos_buffer_pool_t));

        sai_rc = sai_qos_update_buffer_pool_node (&new_buf_pool_node, 1, attr);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_WARN ("Unable to update buffer pool DB for pool ID"
                                 "0x%"PRIx64"", pool_id);
            break;
        }

        switch (attr->id) {
            case SAI_BUFFER_POOL_ATTR_XOFF_SIZE:
                 /* Validation should be done after releasing/adding xoff_th on buffer profiles.
                    Pool shared buffer size will modify based in xoff_size */
                 if (new_buf_pool_node.pool_type != SAI_BUFFER_POOL_TYPE_INGRESS) {
                     sai_rc = SAI_STATUS_ATTR_NOT_SUPPORTED_0;
                 } else {
                     sai_rc =
                         sai_buffer_npu_api_get()->buffer_pool_attr_set(p_buf_pool_node, attr);
                 }
                 break;
            case SAI_BUFFER_POOL_ATTR_SIZE:
                 sai_rc =
                     sai_qos_validate_shared_buffer_pool_size(p_buf_pool_node, &new_buf_pool_node);
                 if(sai_rc != SAI_STATUS_SUCCESS) {
                     SAI_BUFFER_LOG_WARN ("Unable to update buffer pool ID"
                                          "0x%"PRIx64".", pool_id);
                     break;
                 }

                sai_rc =
                   sai_buffer_npu_api_get()->buffer_pool_attr_set(p_buf_pool_node, attr);
                break;

            case SAI_BUFFER_POOL_ATTR_WRED_PROFILE_ID:
                if(new_buf_pool_node.wred_id != p_buf_pool_node->wred_id) {
                    if(SAI_BUFFER_POOL_TYPE_INGRESS == p_buf_pool_node->pool_type) {
                        SAI_BUFFER_LOG_ERR("WRED can't be applied on Ingress Buffer pool.");
                        sai_rc = SAI_STATUS_INVALID_ATTR_VALUE_0;
                    } else {
                        if((sai_rc = sai_qos_wred_link_set(
                                        p_buf_pool_node->key.pool_id,
                                        new_buf_pool_node.wred_id,
                                        DN_SAI_QOS_WRED_LINK_BUFFER_POOL)) != SAI_STATUS_SUCCESS) {
                            SAI_BUFFER_LOG_ERR("Failed to link WRED ID 0x%"PRIx64" for Buffer pool 0x%"PRIx64".",
                                    new_buf_pool_node.wred_id, p_buf_pool_node->key.pool_id);
                        }
                    }
                }
                break;

            case SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE:
            case SAI_BUFFER_POOL_ATTR_SHARED_SIZE:
            case SAI_BUFFER_POOL_ATTR_TYPE:
                sai_rc = SAI_STATUS_INVALID_ATTRIBUTE_0;
                break;

            default:
                sai_rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }
    } while (0);

    /* Buffer pool node will be modified when handling WRED for dll glue*/
    if((attr->id != SAI_BUFFER_POOL_ATTR_WRED_PROFILE_ID) &&
            (attr->id != SAI_BUFFER_POOL_ATTR_XOFF_SIZE)) {
        if((sai_rc == SAI_STATUS_SUCCESS) && (p_buf_pool_node != NULL)) {
            memcpy(p_buf_pool_node, &new_buf_pool_node, sizeof(dn_sai_qos_buffer_pool_t));
        }
    }

    sai_qos_unlock ();

    return sai_rc;
}

static sai_status_t sai_qos_buffer_pool_attr_get (sai_object_id_t pool_id,
                                                  uint32_t attr_count,
                                                  sai_attribute_t *attr_list)
{
    dn_sai_qos_buffer_pool_t *p_buf_pool_node = NULL;
    sai_status_t sai_rc;

    STD_ASSERT (attr_list != NULL);
    sai_rc = sai_qos_buffer_pool_attributes_validate (attr_count, attr_list,
                                                      SAI_OP_GET);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    sai_qos_lock ();

    do {
        p_buf_pool_node = sai_qos_buffer_pool_node_get (pool_id);

        if (p_buf_pool_node == NULL) {
            SAI_BUFFER_LOG_ERR ("Unknown buffer pool object"
                    "0x%"PRIx64"",pool_id);
            sai_rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }
        if (sai_buffer_npu_api_get()->buffer_pool_attr_get != NULL) {
            sai_rc =
                sai_buffer_npu_api_get()->buffer_pool_attr_get (p_buf_pool_node, attr_count, attr_list);
        } else {
          sai_rc = SAI_STATUS_NOT_SUPPORTED;
        }

    } while (0);

    sai_qos_unlock ();

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BUFFER_LOG_ERR ("Unable to get attributes for buffer pool"
                             "0x%"PRIx64"",pool_id);
    }
    return sai_rc;
}

static sai_status_t sai_qos_buffer_profile_attributes_validate (
                                     uint_t attr_count,
                                     const sai_attribute_t *attr_list,
                                     dn_sai_operations_t op_type)
{
    sai_status_t                    sai_rc = SAI_STATUS_SUCCESS;
    uint_t                          max_vendor_attr_count = 0;
    const dn_sai_attribute_entry_t *p_vendor_attr = NULL;

    SAI_BUFFER_LOG_TRACE ("Parsing attributes for buffer profile, "
                             "attribute count %d op_type %d.", attr_count,
                             op_type);

    if (attr_count == 0) {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(attr_list == NULL) {
        SAI_BUFFER_LOG_TRACE ("attribute list passed is NULL");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    sai_buffer_npu_api_get()->buffer_profile_attr_table_get(&p_vendor_attr,
                                                              &max_vendor_attr_count);

    STD_ASSERT(p_vendor_attr != NULL);
    STD_ASSERT(max_vendor_attr_count > 0);

    sai_rc = sai_attribute_validate (attr_count, attr_list, p_vendor_attr,
                                     op_type, max_vendor_attr_count);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BUFFER_LOG_ERR ("Attribute validation failed for %d "
                               "operation", op_type);
        return sai_rc;
    }

    if(op_type == SAI_OP_CREATE ) {
        sai_rc = sai_qos_buffer_profile_is_mandatory_th_attr_present(attr_count,
                                                                     attr_list);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Mandatory th missing");
            return sai_rc;
        }

        sai_rc = sai_qos_buffer_profile_is_pfc_threshold_valid(attr_count,
                                                               attr_list);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Invalid PFC threshold applied");
            return sai_rc;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_qos_create_buffer_profile (sai_object_id_t* profile_id,
                                                   _In_ sai_object_id_t switch_id,
                                                   uint32_t attr_count,
                                                   const sai_attribute_t *attr_list)
{
    sai_status_t sai_rc;
    dn_sai_qos_buffer_profile_t  *p_buf_profile_node = NULL;
    bool npu_conf = false;

    STD_ASSERT (attr_list != NULL);
    STD_ASSERT (profile_id != NULL);
    sai_rc = sai_qos_buffer_profile_attributes_validate (attr_count, attr_list,
                                                         SAI_OP_CREATE);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }
    p_buf_profile_node = sai_qos_buffer_profile_node_alloc ();

    if (NULL == p_buf_profile_node) {
        SAI_BUFFER_LOG_ERR ("Buffer profile node memory allocation failed.");
        return SAI_STATUS_NO_MEMORY;
    }
    sai_qos_init_buffer_profile_node (p_buf_profile_node);

    sai_qos_lock ();

    do {
        sai_rc = sai_qos_update_buffer_profile_node(p_buf_profile_node, attr_count,
                                                    attr_list);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Error unable to update buffer profile node.");
            break;
        }

        sai_rc = sai_buffer_npu_api_get()->buffer_profile_create (p_buf_profile_node,
                                                                  profile_id);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Error unable to create buffer profile node.");
            break;
        }

        npu_conf  = true;
        p_buf_profile_node->key.profile_id = *profile_id;

        sai_rc = sai_qos_buffer_profile_node_insert_to_tree(p_buf_profile_node);

    } while(0);

    if( sai_rc != SAI_STATUS_SUCCESS ) {
        if(npu_conf) {
            sai_buffer_npu_api_get()->buffer_profile_remove (p_buf_profile_node);
        }

        sai_qos_buffer_profile_node_free(p_buf_profile_node);
    }

    sai_qos_unlock ();

    return sai_rc;
}

static sai_status_t sai_qos_remove_buffer_profile (sai_object_id_t profile_id)
{
    sai_status_t sai_rc;
    dn_sai_qos_buffer_profile_t  *p_buf_profile_node = NULL;

    sai_qos_lock ();

    do {

        p_buf_profile_node = sai_qos_buffer_profile_node_get (profile_id);

        if (NULL == p_buf_profile_node) {
            SAI_BUFFER_LOG_ERR ("Error buffer profile object not found 0x%"PRIx64"",
                                profile_id);
            sai_rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        if(p_buf_profile_node->num_ref != 0) {
            SAI_BUFFER_LOG_ERR ("Buffer profile in use. Num ref %u",
                                p_buf_profile_node->num_ref);
            sai_rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        sai_rc = sai_buffer_npu_api_get()->buffer_profile_remove (p_buf_profile_node);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Error unable to remove buffer profile .");
            break;
        }

        sai_qos_buffer_profile_node_remove_from_tree (p_buf_profile_node);

        sai_qos_buffer_profile_node_free (p_buf_profile_node);

    } while(0);

    sai_qos_unlock ();
    return sai_rc;
}

static sai_status_t sai_qos_buffer_profile_attr_get_internal (sai_object_id_t profile_id,
                                                     uint32_t attr_count,
                                                     sai_attribute_t *attr_list)
{
    sai_status_t sai_rc;
    dn_sai_qos_buffer_profile_t  *p_buf_profile_node = NULL;

    STD_ASSERT(attr_list != NULL);

    sai_rc = sai_qos_buffer_profile_attributes_validate (attr_count, attr_list,
                                                         SAI_OP_GET);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    p_buf_profile_node = sai_qos_buffer_profile_node_get (profile_id);

    if (NULL == p_buf_profile_node) {
        SAI_BUFFER_LOG_ERR ("Error buffer profile object not found 0x%"PRIx64"",
                             profile_id);
        sai_qos_unlock();
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    sai_rc = sai_qos_read_buffer_profile_node (p_buf_profile_node,
                                               attr_count, attr_list);

    return sai_rc;
}

static sai_status_t sai_qos_buffer_profile_attr_get (sai_object_id_t profile_id,
                                                     uint32_t attr_count,
                                                     sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    sai_qos_lock();

    sai_rc = sai_qos_buffer_profile_attr_get_internal (profile_id, attr_count, attr_list);

    sai_qos_unlock();

    return sai_rc;
}

static sai_status_t sai_qos_buffer_pool_size_recalc (sai_object_id_t pool_id,
                                                     uint_t size,
                                                     bool is_add)
{
    dn_sai_qos_buffer_pool_t *p_pool_node = NULL;
    sai_attribute_t attr;
    sai_status_t sai_rc;

    p_pool_node = sai_qos_buffer_pool_node_get (pool_id);

    if(p_pool_node == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* is_add = true means some object is added to pool and hence shared size
       is to be reduced */

    if(is_add) {
        if(p_pool_node->shared_size < size) {
            SAI_BUFFER_LOG_ERR("Insufficient memory in pool. Available:%u"
                               "Required:%u",p_pool_node->shared_size, size);
            return SAI_STATUS_INSUFFICIENT_RESOURCES;
        }
        p_pool_node->shared_size -= size;
    } else {
        p_pool_node->shared_size += size;
    }

    memset(&attr, 0, sizeof(attr));
    attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    attr.value.u32 = p_pool_node->size;

    sai_rc = sai_buffer_npu_api_get()->buffer_pool_attr_set (p_pool_node, &attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BUFFER_LOG_WARN ("Unable to update buffer pool DB for pool ID"
                "0x%"PRIx64"", pool_id);
        if(is_add) {
            p_pool_node->shared_size += size;
        } else {
            p_pool_node->shared_size -= size;
        }
        return sai_rc;
    }

    return sai_rc;
}

static sai_status_t sai_qos_update_buffer_profile_ports (dn_sai_qos_buffer_profile_t
                                                         *p_buf_profile_node,
                                                         const sai_attribute_t *old_attr,
                                                         const sai_attribute_t *attr,
                                                         sai_object_id_t *port_id)
{
    dn_sai_qos_port_t *port_node = NULL;
    sai_status_t sai_rc;

    for (port_node = sai_qos_buffer_profile_get_first_port (p_buf_profile_node);
         port_node != NULL; port_node =
         sai_qos_buffer_profile_get_next_port(p_buf_profile_node,port_node)) {

        if (port_node->port_id == *port_id) {
            break;
        }

        sai_rc = sai_buffer_npu_api_get()->buffer_profile_attr_set (
                port_node->port_id, p_buf_profile_node,
                old_attr, attr);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            *port_id = port_node->port_id;
            return sai_rc;
        }
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_qos_update_buffer_profile_queues (dn_sai_qos_buffer_profile_t
                                                          *p_buf_profile_node,
                                                          const sai_attribute_t *old_attr,
                                                          const sai_attribute_t *attr,
                                                          sai_object_id_t *queue_id)
{
    dn_sai_qos_queue_t *queue_node = NULL;
    sai_status_t sai_rc;

    for (queue_node = sai_qos_buffer_profile_get_first_queue (p_buf_profile_node);
         queue_node != NULL; queue_node =
         sai_qos_buffer_profile_get_next_queue (p_buf_profile_node,queue_node)) {

        if(queue_node->key.queue_id == *queue_id) {
            break;
        }
        sai_rc = sai_buffer_npu_api_get()->buffer_profile_attr_set (
                queue_node->key.queue_id, p_buf_profile_node,
                old_attr, attr);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            *queue_id = queue_node->key.queue_id;
            return sai_rc;
        }
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_qos_update_buffer_profile_pgs (dn_sai_qos_buffer_profile_t
                                                       *p_buf_profile_node,
                                                       const sai_attribute_t *old_attr,
                                                       const sai_attribute_t *attr,
                                                       sai_object_id_t *pg_id)
{
    dn_sai_qos_pg_t *pg_node = NULL;
    sai_status_t sai_rc;

    for(pg_node = sai_qos_buffer_profile_get_first_pg (p_buf_profile_node);
        pg_node != NULL; pg_node =
        sai_qos_buffer_profile_get_next_pg (p_buf_profile_node,pg_node)) {

        if(pg_node->key.pg_id == *pg_id ) {
            break;
        }
        sai_rc = sai_buffer_npu_api_get()->buffer_profile_attr_set (
                                      pg_node->key.pg_id, p_buf_profile_node,
                                      old_attr, attr);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            *pg_id = pg_node->key.pg_id;
            return sai_rc;
       }
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_qos_update_buffer_profile_objects (dn_sai_qos_buffer_profile_t
                                                           *p_buf_profile_node,
                                                           const sai_attribute_t *old_attr,
                                                           const sai_attribute_t *attr)
{
    sai_object_id_t port_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t queue_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t pg_id = SAI_NULL_OBJECT_ID;
    bool port_update = false;
    bool queue_update = false;
    bool pg_update = false;
    sai_status_t sai_rc;

    do {
        if (attr->id == SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE) {
            sai_rc = sai_qos_update_buffer_profile_node (p_buf_profile_node, 1, attr);
            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_BUFFER_LOG_ERR ("Failed to update the cache for profile");
            }
            break;
        }

        if ((attr->id != SAI_BUFFER_PROFILE_ATTR_XOFF_TH) &&
            (attr->id != SAI_BUFFER_PROFILE_ATTR_XON_TH)) {

            sai_rc = sai_qos_update_buffer_profile_ports(p_buf_profile_node, old_attr, attr, &port_id);
            port_update = true;
            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_BUFFER_LOG_ERR ("Object update failed for port:0x%"PRIx64"",port_id);
                break;
            }

            sai_rc = sai_qos_update_buffer_profile_queues(p_buf_profile_node,old_attr, attr, &queue_id);
            queue_update = true;
            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_BUFFER_LOG_ERR ("Object update failed for Queue:0x%"PRIx64"",queue_id);
                break;
            }
        }

        sai_rc = sai_qos_update_buffer_profile_pgs(p_buf_profile_node, old_attr, attr, &pg_id);
        pg_update = true;
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Object update failed for pg:0x%"PRIx64"",pg_id);
            break;
        }

        sai_rc = sai_qos_update_buffer_profile_node (p_buf_profile_node, 1, attr);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Failed to update the cache for profile");
            break;
        }
    } while (0);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        if(port_update) {
            sai_qos_update_buffer_profile_ports(p_buf_profile_node, attr, old_attr, &port_id);
        }

        if(queue_update) {
            sai_qos_update_buffer_profile_queues(p_buf_profile_node, attr, old_attr, &queue_id);
        }

        if(pg_update) {
            sai_qos_update_buffer_profile_pgs(p_buf_profile_node, attr, old_attr, &pg_id);
        }
    }
    return sai_rc;
}

static sai_status_t  sai_qos_check_buffer_size (const sai_attribute_t *attr,
                                                dn_sai_qos_buffer_profile_t
                                                *p_buf_profile_node,
                                                uint_t *buf_size, bool *is_add)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT (attr != NULL);
    STD_ASSERT (p_buf_profile_node != NULL);
    STD_ASSERT (buf_size != NULL);
    STD_ASSERT (is_add != NULL);

    if(attr->id == SAI_BUFFER_PROFILE_ATTR_POOL_ID) {
        *buf_size = (p_buf_profile_node->num_ref*(p_buf_profile_node->size +
                    sai_qos_buffer_profile_get_reserved_xoff_th_get(p_buf_profile_node)));
        if(!sai_qos_is_buffer_pool_available (attr->value.oid, *buf_size)) {
            SAI_BUFFER_LOG_WARN ("Error not enough memory in buffer pool 0x%"PRIx64"",
                                  p_buf_profile_node->buffer_pool_id);
            return SAI_STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if(attr->id == SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE) {
        if(attr->value.u32 > p_buf_profile_node->size) {
            *buf_size = ((p_buf_profile_node->num_ref)*
                    (attr->value.u32 - p_buf_profile_node->size));

            if(!sai_qos_is_buffer_pool_available(p_buf_profile_node->buffer_pool_id,
                                                 *buf_size)) {
                SAI_BUFFER_LOG_WARN ("Error not enough memory in buffer pool 0x%"PRIx64"",
                                     p_buf_profile_node->buffer_pool_id);
                return SAI_STATUS_INSUFFICIENT_RESOURCES;
            }

            sai_rc = sai_buffer_npu_api_get()->check_buffer_size(p_buf_profile_node->buffer_pool_id,
                                               p_buf_profile_node, (attr->value.u32 - p_buf_profile_node->size));
            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_BUFFER_LOG_WARN ("Error not enough memory in buffer pool 0x%"PRIx64"",
                                      p_buf_profile_node->buffer_pool_id);
                return SAI_STATUS_INSUFFICIENT_RESOURCES;
            }

            *is_add = true;
        } else {
            *is_add = false;
            *buf_size = ((p_buf_profile_node->num_ref)*
                    (p_buf_profile_node->size - attr->value.u32));
        }
    }

    if (attr->id == SAI_BUFFER_PROFILE_ATTR_XOFF_TH) {
        if (sai_qos_is_buffer_pool_xoff_size_configured(p_buf_profile_node->buffer_pool_id) == true) {
           *is_add = true;
           *buf_size = 0;
           SAI_BUFFER_LOG_TRACE ("Buffer pool XOFF_SIZE is confiured, "
                                 "Skip checking buffer availbality for pool 0x%"PRIx64"",
                                 p_buf_profile_node->buffer_pool_id);
           return SAI_STATUS_SUCCESS;
        }
        if(attr->value.u32 > p_buf_profile_node->xoff_th) {
            *buf_size = ((p_buf_profile_node->num_ref)*
                    (attr->value.u32 - p_buf_profile_node->xoff_th));

            if(!sai_qos_is_buffer_pool_available(p_buf_profile_node->buffer_pool_id,
                                                 *buf_size)) {
                SAI_BUFFER_LOG_WARN ("Error not enough memory in buffer pool 0x%"PRIx64"",
                                      p_buf_profile_node->buffer_pool_id);
                return SAI_STATUS_INSUFFICIENT_RESOURCES;
            }

            sai_rc = sai_buffer_npu_api_get()->check_buffer_size(p_buf_profile_node->buffer_pool_id,
                                                                 p_buf_profile_node,
                                                    (attr->value.u32 - p_buf_profile_node->xoff_th));
            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_BUFFER_LOG_WARN ("Error not enough memory in buffer pool 0x%"PRIx64"",
                                      p_buf_profile_node->buffer_pool_id);
                return SAI_STATUS_INSUFFICIENT_RESOURCES;
            }
            *is_add = true;
        } else {
            *is_add = false;
            *buf_size = ((p_buf_profile_node->num_ref)*
                    (p_buf_profile_node->xoff_th - attr->value.u32));
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t  sai_qos_check_buffer_profile_attr_value (const sai_attribute_t *attr,
                                                             dn_sai_qos_buffer_profile_t
                                                             *p_buf_profile_node)
{
    switch (attr->id) {
        case SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH:
        case SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH:
        case SAI_BUFFER_PROFILE_ATTR_XOFF_TH:
        case SAI_BUFFER_PROFILE_ATTR_XON_TH:
            if(!sai_qos_buffer_profile_is_valid_threshold_applied(p_buf_profile_node,
                                                                  attr)) {
                SAI_BUFFER_LOG_ERR ("Error invalid threshold applied");
                return SAI_STATUS_INVALID_ATTRIBUTE_0;
            }
            break;

        case SAI_BUFFER_PROFILE_ATTR_POOL_ID:
            if (!sai_qos_is_buffer_pool_compatible (attr->value.oid,
                                                    p_buf_profile_node->buffer_pool_id,
                                                    !p_buf_profile_node->profile_th_enable)) {
                SAI_BUFFER_LOG_WARN ("New buffer pool is not of same type");
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
            break;

        default:
            break;

    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_qos_buffer_profile_attr_set (sai_object_id_t profile_id,
                                                     const sai_attribute_t *attr)
{
    sai_status_t sai_rc;
    dn_sai_qos_buffer_profile_t  *p_buf_profile_node = NULL;
    uint_t buf_size = 0;
    sai_object_id_t old_pool_id;
    bool is_add = false;
    sai_attribute_t old_attr;

    STD_ASSERT(attr != NULL);

    sai_rc = sai_qos_buffer_profile_attributes_validate (1, attr, SAI_OP_SET);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    memset(&old_attr, 0, sizeof(old_attr));
    old_attr.id = attr->id;

    sai_qos_lock ();

    do {
        sai_rc = sai_qos_buffer_profile_attr_get_internal (profile_id, 1, &old_attr);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Unable to get old buffer profile attribute value");
            break;
        }

        p_buf_profile_node = sai_qos_buffer_profile_node_get (profile_id);

        if (NULL == p_buf_profile_node) {
            SAI_BUFFER_LOG_ERR ("Error buffer profile object not found 0x%"PRIx64"",
                                profile_id);
            sai_rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        old_pool_id = p_buf_profile_node->buffer_pool_id;

        sai_rc = sai_qos_check_buffer_profile_attr_value (attr, p_buf_profile_node);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }

        sai_rc = sai_qos_check_buffer_size (attr, p_buf_profile_node, &buf_size, &is_add);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }

        sai_rc = sai_qos_update_buffer_profile_objects (p_buf_profile_node, &old_attr, attr);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Unable to update buffer profile objects");
            break;
        }

        if(attr->id == SAI_BUFFER_PROFILE_ATTR_POOL_ID) {
            sai_qos_buffer_pool_size_recalc (old_pool_id, buf_size, false);
            sai_qos_buffer_pool_size_recalc (attr->value.oid, buf_size, true);
        }

        if((attr->id == SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE) ||
           (attr->id == SAI_BUFFER_PROFILE_ATTR_XOFF_TH)) {
            sai_qos_buffer_pool_size_recalc(p_buf_profile_node->buffer_pool_id, buf_size, is_add);
        }

    } while (0);


    sai_qos_unlock ();
    return sai_rc;
}

static sai_status_t sai_qos_calc_pool_size (sai_object_id_t old_pool_id, sai_object_id_t new_pool_id,
                                            bool check_th, uint_t old_size, uint_t new_size,
                                            uint_t *tmp_size, bool *is_add)
{
    if(old_pool_id == new_pool_id) {
        if(new_size > old_size) {
            *tmp_size = new_size - old_size;
            *is_add = true;
        } else {
            *tmp_size = old_size - new_size;
        }
    } else {
        if (new_pool_id != SAI_NULL_OBJECT_ID) {
            if (old_pool_id != SAI_NULL_OBJECT_ID) {
                if (!sai_qos_is_buffer_pool_compatible (old_pool_id, new_pool_id, check_th)) {
                    SAI_BUFFER_LOG_WARN ("New buffer pool is not of same type");
                    return SAI_STATUS_INVALID_PARAMETER;
                }
            }
            *tmp_size = new_size;
        }

    }

    if((new_pool_id != SAI_NULL_OBJECT_ID) &&
       (!sai_qos_is_buffer_pool_available(new_pool_id, *tmp_size))) {
        SAI_BUFFER_LOG_WARN ("Error not enough memory in buffer pool");
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_obj_update_buffer_profile (sai_object_id_t object_id,
                                                sai_object_id_t profile_id)
{
    sai_status_t sai_rc;
    dn_sai_qos_buffer_profile_t  *p_buf_profile_node = NULL;
    dn_sai_qos_buffer_profile_t  *p_old_buf_profile_node = NULL;
    dn_sai_qos_buffer_profile_t   def_buf_profile;
    sai_object_id_t old_profile_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t old_pool_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t new_pool_id = SAI_NULL_OBJECT_ID;
    uint_t new_size = 0;
    uint_t old_size = 0;
    uint_t tmp_size = 0;
    bool is_add = false;
    bool check_th = true;

    sai_rc = sai_qos_get_buffer_profile_id (object_id, &old_profile_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BUFFER_LOG_ERR ("Error unable to get old buffer profile");
        return sai_rc;
    }

    if(old_profile_id == profile_id) {
        SAI_BUFFER_LOG_INFO ("Item already exists");
        return SAI_STATUS_SUCCESS;
    }

    if (profile_id != SAI_NULL_OBJECT_ID) {
        p_buf_profile_node = sai_qos_buffer_profile_node_get (profile_id);
        if (NULL == p_buf_profile_node) {
            SAI_BUFFER_LOG_ERR ("Error buffer profile object not found 0x%"PRIx64"",
                                profile_id);
            return SAI_STATUS_ITEM_NOT_FOUND;
        }

        if(!sai_qos_is_object_buffer_profile_compatible (object_id,
                                                         p_buf_profile_node)) {
            SAI_BUFFER_LOG_ERR ("Error buffer profile object incompatible 0x%"PRIx64"",
                                profile_id);
            return SAI_STATUS_INVALID_PARAMETER;
        }

        new_size = p_buf_profile_node->size + sai_qos_buffer_profile_get_reserved_xoff_th_get(p_buf_profile_node);
        new_pool_id = p_buf_profile_node->buffer_pool_id;
    } else {
        memset(&def_buf_profile, 0, sizeof(dn_sai_qos_buffer_profile_t));
        sai_qos_get_default_buffer_profile (&def_buf_profile);
        p_buf_profile_node = &def_buf_profile;
    }

    if(old_profile_id != SAI_NULL_OBJECT_ID) {
        p_old_buf_profile_node = sai_qos_buffer_profile_node_get (old_profile_id);
        if (NULL == p_old_buf_profile_node) {
            SAI_BUFFER_LOG_ERR ("Error old buffer profile object not found 0x%"PRIx64"",
                                old_profile_id);
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
        old_size = p_old_buf_profile_node->size + sai_qos_buffer_profile_get_reserved_xoff_th_get(p_old_buf_profile_node);
        old_pool_id = p_old_buf_profile_node->buffer_pool_id;
        if(p_old_buf_profile_node->profile_th_enable) {
            check_th = false;
        }
    } else {
        memset(&def_buf_profile, 0, sizeof(dn_sai_qos_buffer_profile_t));
        sai_qos_get_default_buffer_profile (&def_buf_profile);
        p_old_buf_profile_node = &def_buf_profile;
    }

    sai_rc = sai_qos_calc_pool_size (old_pool_id, new_pool_id, check_th,
                                     old_size, new_size, &tmp_size, &is_add);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    sai_rc = sai_buffer_npu_api_get()->buffer_profile_apply (object_id, p_old_buf_profile_node,
                                                             p_buf_profile_node, false);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BUFFER_LOG_ERR ("Error unable to set buffer profile object");
        return sai_rc;
    }

    sai_rc = sai_qos_obj_update_buffer_profile_node (object_id, profile_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BUFFER_LOG_ERR ("Error unable to update buffer profile node");
        if(old_profile_id != SAI_NULL_OBJECT_ID) {
            sai_buffer_npu_api_get()->buffer_profile_apply (object_id, p_buf_profile_node,
                                                            p_old_buf_profile_node, false);
        }
        return sai_rc;
    }

    if(old_pool_id == new_pool_id) {
        sai_qos_buffer_pool_size_recalc (old_pool_id, tmp_size, is_add);
    } else {
        if(old_pool_id != SAI_NULL_OBJECT_ID) {
            sai_qos_buffer_pool_size_recalc (old_pool_id, old_size, false);
        }
        if(new_pool_id != SAI_NULL_OBJECT_ID) {
            sai_qos_buffer_pool_size_recalc (new_pool_id, new_size, true);
        }
    }
    return sai_rc;
}

sai_status_t sai_buffer_init (void)
{
    sai_status_t sai_rc;

    sai_rc = sai_buffer_npu_api_get()->buffer_init();
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BUFFER_LOG_CRIT ("Error NPU init failed");
        return sai_rc;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_buffer_pool_stats_get (sai_object_id_t pool_id, uint32_t number_of_counters,
                                            const sai_buffer_pool_stat_t *counter_ids,
                                            uint64_t* counters)
{
    return sai_buffer_npu_api_get()->buffer_pool_stats_get(pool_id, counter_ids,
                                                           number_of_counters, counters);
}

sai_status_t sai_qos_buffer_pool_stats_clear (sai_object_id_t pool_id,
                                              _In_ uint32_t number_of_counters,
                                              _In_ const sai_buffer_pool_stat_t *counter_ids)
{
    /** TODO - Implement it */
    return SAI_STATUS_NOT_IMPLEMENTED;
}

static sai_buffer_api_t sai_qos_buffer_method_table = {
    sai_qos_create_buffer_pool,
    sai_qos_remove_buffer_pool,
    sai_qos_buffer_pool_attr_set,
    sai_qos_buffer_pool_attr_get,
    sai_qos_buffer_pool_stats_get,
    sai_qos_buffer_pool_stats_clear,
    NULL,
    NULL,
    sai_qos_pg_attr_set,
    sai_qos_pg_attr_get,
    sai_qos_pg_stats_get,
    sai_qos_pg_stats_clear,
    sai_qos_create_buffer_profile,
    sai_qos_remove_buffer_profile,
    sai_qos_buffer_profile_attr_set,
    sai_qos_buffer_profile_attr_get,
};

sai_buffer_api_t *sai_qos_buffer_api_query (void)
{
    return (&sai_qos_buffer_method_table);
}
