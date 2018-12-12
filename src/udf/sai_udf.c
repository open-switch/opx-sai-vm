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
* @file sai_udf.c
*
* @brief This file contains function definitions for the SAI UDF
*        functionality API.
*
*************************************************************************/

#include "saiudf.h"
#include "saistatus.h"
#include "saitypes.h"

#include "sai_common_infra.h"
#include "sai_udf_api_utils.h"
#include "sai_udf_common.h"
#include "sai_oid_utils.h"

#include "std_type_defs.h"
#include "std_rbtree.h"
#include "std_llist.h"
#include "std_assert.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static inline dn_sai_udf_t *dn_sai_udf_node_alloc (void)
{
    return ((dn_sai_udf_t *) calloc (1, sizeof (dn_sai_udf_t)));
}

static inline uint8_t *dn_sai_udf_hash_mask_alloc (uint32_t count)
{
    return ((uint8_t *) calloc (count, sizeof (uint8_t)));
}

static inline void dn_sai_udf_node_free (dn_sai_udf_t *p_udf_node)
{
    free ((void *) p_udf_node->hash_mask.list);

    p_udf_node->hash_mask.list = NULL;

    free ((void *) p_udf_node);
}

static void dn_sai_udf_hash_mask_log_trace (dn_sai_udf_t *p_udf)
{
    uint_t  index;

    STD_ASSERT (p_udf != NULL);

    SAI_UDF_LOG_TRACE ("UDF hash mask count is set to %u.",
                       p_udf->hash_mask.count);

    SAI_UDF_LOG_TRACE ("UDF hash mask byte is set to ");

    for (index = 0; index < p_udf->hash_mask.count; index++) {

        SAI_UDF_LOG_TRACE ("0x%x ", p_udf->hash_mask.list [index]);
    }
}

static sai_status_t dn_sai_udf_default_hash_mask_fill (dn_sai_udf_t *p_udf)
{
    STD_ASSERT (p_udf != NULL);

    uint_t  udf_length = p_udf->p_udf_group->length;
    uint8_t hash_mask = 0xFF;
    uint_t  count;

    if (p_udf->p_udf_group->type != SAI_UDF_GROUP_TYPE_HASH) {

        return SAI_STATUS_SUCCESS;
    }

    p_udf->hash_mask.list = dn_sai_udf_hash_mask_alloc (udf_length);

    if (p_udf->hash_mask.list == NULL) {

        SAI_UDF_LOG_ERR ("Failed to allocate memory for default UDF Hash mask.");

        return SAI_STATUS_NO_MEMORY;
    }

    p_udf->hash_mask.count = udf_length;

    for (count = 0; count < udf_length; count++) {
        p_udf->hash_mask.list [count] = hash_mask;
    }

    dn_sai_udf_hash_mask_log_trace (p_udf);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_udf_hash_mask_set (dn_sai_udf_t *p_udf,
                                              sai_u8_list_t *hash_mask)
{
    STD_ASSERT (p_udf != NULL);
    STD_ASSERT (hash_mask != NULL);
    STD_ASSERT (p_udf->p_udf_group != NULL);

    if (p_udf->p_udf_group->type != SAI_UDF_GROUP_TYPE_HASH) {

        SAI_UDF_LOG_ERR ("UDF Hash mask attribute Id is passed for UDF Group "
                         "of type: %d.", p_udf->p_udf_group->type);

        return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    if ((hash_mask->count == 0) || (hash_mask->list == NULL)) {

        SAI_UDF_LOG_ERR ("Invalid UDF Hash mask attribute value passed. count"
                         ": %u, list: %p.", hash_mask->count, hash_mask->list);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    if (p_udf->p_udf_group->length != hash_mask->count) {

        SAI_UDF_LOG_ERR ("UDF Hash mask count: %u not same as UDF length: %u.",
                         hash_mask->count, p_udf->p_udf_group->length);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    if (p_udf->hash_mask.list == NULL) {

        p_udf->hash_mask.list  = dn_sai_udf_hash_mask_alloc (hash_mask->count);

        if (p_udf->hash_mask.list == NULL) {

            SAI_UDF_LOG_ERR ("Failed to allocate memory for UDF Hash mask.");

            return SAI_STATUS_NO_MEMORY;
        }
    }

    p_udf->hash_mask.count = hash_mask->count;

    memcpy (p_udf->hash_mask.list, hash_mask->list, hash_mask->count);

    dn_sai_udf_hash_mask_log_trace (p_udf);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_udf_match_id_set (
                                         dn_sai_udf_t *p_udf,
                                         const sai_attribute_value_t *p_value)
{
    if (!sai_is_obj_id_udf_match (p_value->oid)) {

        SAI_UDF_LOG_ERR ("OID 0x%"PRIx64" is not of UDF Match object type.",
                         p_value->oid);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_udf->match_obj_id = p_value->oid;

    SAI_UDF_LOG_TRACE ("UDF Match Id is set to 0x%"PRIx64".", p_udf->match_obj_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_udf_group_id_set (
                                         dn_sai_udf_t *p_udf,
                                         const sai_attribute_value_t *p_value)
{
    if (!sai_is_obj_id_udf_group (p_value->oid)) {

        SAI_UDF_LOG_ERR ("OID 0x%"PRIx64" is not of UDF Group object type.",
                         p_value->oid);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    dn_sai_udf_group_t *p_udf_group = dn_sai_udf_group_node_get (p_value->oid);

    if (p_udf_group == NULL) {

        SAI_UDF_LOG_ERR ("UDF Group node is not found in database for Id: "
                         "0x%"PRIx64"", p_value->oid);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    p_udf->p_udf_group = p_udf_group;

    SAI_UDF_LOG_TRACE ("UDF Group Id is set to 0x%"PRIx64".",
                       p_udf->p_udf_group->key.group_obj_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_udf_base_set (dn_sai_udf_t *p_udf,
                                         const sai_attribute_value_t *p_value)
{
    if (!((p_value->s32 == SAI_UDF_BASE_L2) ||
         (p_value->s32 == SAI_UDF_BASE_L3) ||
         (p_value->s32 == SAI_UDF_BASE_L4))) {

        SAI_UDF_LOG_ERR ("Invalid UDF Base id: %d passed", p_value->s32);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_udf->base = p_value->s32;

    SAI_UDF_LOG_TRACE ("UDF Base is set to %d.", p_udf->base);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_udf_attr_set (dn_sai_udf_t *p_udf,
                                         uint32_t attr_count,
                                         const sai_attribute_t *attr_list,
                                         dn_sai_operations_t op_type)
{
    uint_t                  attr_idx;
    sai_status_t            status = SAI_STATUS_SUCCESS;
    sai_u8_list_t           hash_mask_list = {0, NULL};
    uint_t                  hash_mask_attr_index = 0;
    bool                    hash_mask_set = false;
    const sai_attribute_t  *p_attr;

    STD_ASSERT (p_udf != NULL);
    STD_ASSERT (attr_list != NULL);

    for (attr_idx = 0, p_attr = attr_list; (attr_idx < attr_count);
         ++attr_idx, ++p_attr) {

        SAI_UDF_LOG_TRACE ("Setting UDF attr id: %d at list index: %u.",
                           p_attr->id, attr_idx);

        status = SAI_STATUS_SUCCESS;

        switch (p_attr->id) {

            case SAI_UDF_ATTR_MATCH_ID:
                status = dn_sai_udf_match_id_set (p_udf, &p_attr->value);
                break;

            case SAI_UDF_ATTR_GROUP_ID:
                status = dn_sai_udf_group_id_set (p_udf, &p_attr->value);
                break;

            case SAI_UDF_ATTR_BASE:
                status = dn_sai_udf_base_set (p_udf, &p_attr->value);
                break;

            case SAI_UDF_ATTR_OFFSET:
                p_udf->offset = p_attr->value.u16;
                break;

            case SAI_UDF_ATTR_HASH_MASK:
                hash_mask_set        = true;
                hash_mask_attr_index = attr_idx;
                hash_mask_list       = p_attr->value.u8list;
                break;

            default:
                status = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }

        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("Failure in UDF attr list. Index: %d, Attr Id: %d"
                             ", Error: %d.", attr_idx, p_attr->id, status);

            return (sai_get_indexed_ret_val (status, attr_idx));
        }
    }

    if (hash_mask_set) {

        status = dn_sai_udf_hash_mask_set (p_udf, &hash_mask_list);

        return (sai_get_indexed_ret_val (status, hash_mask_attr_index));

    } else if (op_type == SAI_OP_CREATE) {

        /* Set the default Hash mask */
        status = dn_sai_udf_default_hash_mask_fill (p_udf);
    }

    return status;
}

static void dn_sai_udf_group_udf_list_update (dn_sai_udf_t *p_udf, bool is_add)
{
    STD_ASSERT (p_udf != NULL);

    dn_sai_udf_group_t *p_udf_group = p_udf->p_udf_group;

    if (is_add) {

        std_dll_insertatback (&p_udf_group->udf_list, &p_udf->node_link);

        p_udf_group->udf_count++;

    } else {
        std_dll_remove (&p_udf_group->udf_list, &p_udf->node_link);

        p_udf_group->udf_count--;
    }
}

static void dn_sai_udf_node_init (dn_sai_udf_t *p_udf_obj)
{
    STD_ASSERT (p_udf_obj != NULL);

    p_udf_obj->base        = SAI_UDF_BASE_L2;
    p_udf_obj->p_udf_group = NULL;
}

static sai_status_t dn_sai_udf_create (sai_object_id_t *udf_id,
                                       _In_ sai_object_id_t switch_id,
                                       uint32_t attr_count,
                                       const sai_attribute_t *attr_list)
{
    sai_status_t         status;
    dn_sai_udf_t        *p_udf = NULL;
    sai_npu_object_id_t  hw_id = 0;

    STD_ASSERT (udf_id != NULL);
    STD_ASSERT (attr_list != NULL);

    SAI_UDF_LOG_TRACE ("Entering UDF creation.");

    status = dn_sai_udf_attr_list_validate (SAI_OBJECT_TYPE_UDF,
                                            attr_count, attr_list,
                                            SAI_OP_CREATE);
    if (status != SAI_STATUS_SUCCESS) {

        SAI_UDF_LOG_ERR ("UDF creation attr list validation failed.");

        return status;
    }

    p_udf = dn_sai_udf_node_alloc ();

    if (p_udf == NULL) {

        SAI_UDF_LOG_CRIT ("SAI UDF node memory allocation failed.");

        return SAI_STATUS_NO_MEMORY;
    }

    dn_sai_udf_node_init (p_udf);

    dn_sai_udf_lock ();

    do {
        status = dn_sai_udf_attr_set (p_udf, attr_count, attr_list,
                                      SAI_OP_CREATE);
        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("Error in attr list for UDF creation");

            break;
        }

        status = sai_udf_npu_api_get()->udf_create (p_udf, &hw_id);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("NPU UDF object creation failed.");

            break;
        }

        p_udf->key.udf_obj_id = sai_uoid_create (SAI_OBJECT_TYPE_UDF, hw_id);

        t_std_error rc = std_rbtree_insert (dn_sai_udf_tree_handle (), p_udf);

        if (rc != STD_ERR_OK) {

            SAI_UDF_LOG_ERR ("Error in inserting UDF Id: 0x%"PRIx64" to "
                             "database", p_udf->key.udf_obj_id);

            break;
        }

        /* Insert into the UDF Group's UDF list */
        dn_sai_udf_group_udf_list_update (p_udf, true);

    } while (0);

    dn_sai_udf_unlock ();

    if (status != SAI_STATUS_SUCCESS) {

        SAI_UDF_LOG_ERR ("SAI UDF creation error: %d.", status);

        if (hw_id != 0) {

            sai_udf_npu_api_get()->udf_remove (p_udf);
        }

        dn_sai_udf_node_free (p_udf);

        p_udf = NULL;
    } else {

        *udf_id = p_udf->key.udf_obj_id;

        SAI_UDF_LOG_INFO ("Created SAI UDF Id: 0x%"PRIx64".", *udf_id);
    }

    return status;
}

static sai_status_t dn_sai_udf_remove (sai_object_id_t udf_id)
{
    sai_status_t  status = SAI_STATUS_FAILURE;

    SAI_UDF_LOG_TRACE ("Entering UDF Id: 0x%"PRIx64" remove.", udf_id);

    if (!sai_is_obj_id_udf (udf_id)) {

        SAI_UDF_LOG_ERR ("OID 0x%"PRIx64" is not of UDF object type.",
                         udf_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    dn_sai_udf_lock ();

    do {
        dn_sai_udf_t *p_udf_node = dn_sai_udf_node_get (udf_id);

        if (p_udf_node == NULL) {

            SAI_UDF_LOG_ERR ("UDF node is not found in database for Id: "
                             "0x%"PRIx64"", udf_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        /* Remove from the UDF database view */
        dn_sai_udf_t *p_tmp_node = (dn_sai_udf_t *)
            std_rbtree_remove (dn_sai_udf_tree_handle (), p_udf_node);

        if (p_tmp_node != p_udf_node) {

            SAI_UDF_LOG_ERR ("Error in removing UDF Id: 0x%"PRIx64" from "
                             "database.", udf_id);

            break;
        }

        /* Remove in NPU */
        status = sai_udf_npu_api_get()->udf_remove (p_udf_node);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("UDF Id: 0x%"PRIx64" remove from NPU failed.",
                             udf_id);
            std_rbtree_insert (dn_sai_udf_tree_handle (), p_udf_node);

            break;
        }

        /* Remove from the UDF Group's UDF list view */
        dn_sai_udf_group_udf_list_update (p_udf_node, false);

        SAI_UDF_LOG_INFO ("Removed SAI UDF Id: 0x%"PRIx64".",
                          p_udf_node->key.udf_obj_id);

        /* Free the UDF node */
        dn_sai_udf_node_free (p_udf_node);

        p_udf_node = NULL;
    } while (0);

    dn_sai_udf_unlock ();

    SAI_UDF_LOG_TRACE ("UDF Id: 0x%"PRIx64" remove API returning %d.",
                       udf_id, status);
    return status;
}

static sai_status_t dn_sai_udf_set_attribute (sai_object_id_t udf_id,
                                              const sai_attribute_t *attr)
{
    sai_status_t    status;
    dn_sai_udf_t    udf_obj_info;
    const uint_t    attr_count = 1;

    STD_ASSERT (attr != NULL);

    SAI_UDF_LOG_TRACE ("Entering UDF Id: 0x%"PRIx64" set attr Id: %d.",
                       udf_id, attr->id);

    if (!sai_is_obj_id_udf (udf_id)) {

        SAI_UDF_LOG_ERR ("OID 0x%"PRIx64" is not of UDF object type.",
                         udf_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    status = dn_sai_udf_attr_list_validate (SAI_OBJECT_TYPE_UDF,
                                            attr_count, attr, SAI_OP_SET);
    if (status != SAI_STATUS_SUCCESS) {

        SAI_UDF_LOG_ERR ("UDF set API attr list validation failed.");

        return status;
    }

    dn_sai_udf_lock ();

    do {
        dn_sai_udf_t *p_udf_node = dn_sai_udf_node_get (udf_id);

        if (p_udf_node == NULL) {

            SAI_UDF_LOG_ERR ("UDF node is not found in database for Id: "
                             "0x%"PRIx64"", udf_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        /* Validate and set the attribute in temporary node */
        memcpy (&udf_obj_info, p_udf_node, sizeof (udf_obj_info));

        status = dn_sai_udf_attr_set (&udf_obj_info, attr_count, attr,
                                      SAI_OP_SET);
        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("Error in attr value for UDF set attribute");

            break;
        }

        status = sai_udf_npu_api_get()->udf_attribute_set (&udf_obj_info, attr);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("UDF object attribute set in NPU failed.");

            break;
        }

        memcpy (p_udf_node, &udf_obj_info, sizeof (udf_obj_info));

        SAI_UDF_LOG_INFO ("SAI UDF Id: 0x%"PRIx64" set Attr Id: %d success.",
                          udf_id, attr->id);
    } while (0);

    dn_sai_udf_unlock ();

    SAI_UDF_LOG_TRACE ("Set attr for UDF Id: 0x%"PRIx64" returning %d.",
                       udf_id, status);

    return status;
}

static sai_status_t dn_sai_udf_hash_mask_fill (const dn_sai_udf_t *p_udf,
                                               sai_u8_list_t *p_list_attr)
{
    uint8_t        hash_mask [SAI_UDF_DFLT_HASH_MASK_COUNT] = {0xFF, 0xFF};
    sai_u8_list_t  hash_mask_value = {SAI_UDF_DFLT_HASH_MASK_COUNT, hash_mask};

    if (p_udf->hash_mask.count != 0) {

        hash_mask_value = p_udf->hash_mask;
    }

    if ((hash_mask_value.count > p_list_attr->count) ||
        (p_list_attr->list == NULL)) {

        SAI_UDF_LOG_ERR ("Attr list size: %u is less than the UDF hash mask "
                         "len: %u.", p_list_attr->count, hash_mask_value.count);

        p_list_attr->count = hash_mask_value.count;

        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    p_list_attr->count = hash_mask_value.count;

    memcpy (p_list_attr->list, hash_mask_value.list,
            sizeof (uint8_t) * hash_mask_value.count);

    SAI_UDF_LOG_TRACE ("UDF Hash mask length: %u.", p_list_attr->count);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_udf_attr_get (const dn_sai_udf_t *p_udf,
                                         uint32_t attr_count,
                                         sai_attribute_t *attr_list)
{
    sai_attribute_t *p_attr;
    uint_t           attr_idx;
    sai_status_t     status;

    STD_ASSERT (p_udf != NULL);
    STD_ASSERT (attr_list != NULL);

    for (attr_idx = 0, p_attr = attr_list; (attr_idx < attr_count);
         ++attr_idx, ++p_attr) {

        SAI_UDF_LOG_TRACE ("Get UDF attr id: %d at list index: %u.",
                           p_attr->id, attr_idx);

        status = SAI_STATUS_SUCCESS;

        switch (p_attr->id) {

            case SAI_UDF_ATTR_MATCH_ID:
                p_attr->value.oid = p_udf->match_obj_id;
                break;

            case SAI_UDF_ATTR_GROUP_ID:
                p_attr->value.oid = p_udf->p_udf_group->key.group_obj_id;
                break;

            case SAI_UDF_ATTR_BASE:
                p_attr->value.s32 = p_udf->base;
                break;

            case SAI_UDF_ATTR_OFFSET:
                p_attr->value.u16 = p_udf->offset;
                break;

            case SAI_UDF_ATTR_HASH_MASK:
                status = dn_sai_udf_hash_mask_fill (p_udf, &p_attr->value.u8list);
                break;

            default:
                status = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }

        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("Failure in filling UDF attr list for get API."
                             "Attribute Id: %d, Error: %d.", p_attr->id,
                             status);

            return (sai_get_indexed_ret_val (status, attr_idx));
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_udf_get_attribute (sai_object_id_t udf_id,
                                              uint32_t attr_count,
                                              sai_attribute_t *attr_list)
{
    sai_status_t    status;

    SAI_UDF_LOG_TRACE ("Entering UDF Id: 0x%"PRIx64" get API.", udf_id);

    if (!sai_is_obj_id_udf (udf_id)) {

        SAI_UDF_LOG_ERR ("OID 0x%"PRIx64" is not of UDF object type.",
                         udf_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    dn_sai_udf_lock ();

    do {
        dn_sai_udf_t *p_udf_node = dn_sai_udf_node_get (udf_id);

        if (p_udf_node == NULL) {

            SAI_UDF_LOG_ERR ("UDF node is not found in database for Id: "
                             "0x%"PRIx64"", udf_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        status = dn_sai_udf_attr_get (p_udf_node, attr_count, attr_list);
    } while (0);

    dn_sai_udf_unlock ();

    SAI_UDF_LOG_TRACE ("Get attr for UDF Id: 0x%"PRIx64" returning %d.",
                       udf_id, status);

    return status;
}

void dn_sai_udf_api_fill (sai_udf_api_t *p_udf_api_table)
{
    p_udf_api_table->create_udf        = dn_sai_udf_create;
    p_udf_api_table->remove_udf        = dn_sai_udf_remove;
    p_udf_api_table->set_udf_attribute = dn_sai_udf_set_attribute;
    p_udf_api_table->get_udf_attribute = dn_sai_udf_get_attribute;
}

static sai_status_t dn_sai_udf_match_create (sai_object_id_t *udf_match_id,
                                             _In_ sai_object_id_t switch_id,
                                             uint32_t attr_count,
                                             const sai_attribute_t *attr_list)
{
    sai_status_t         status;
    sai_npu_object_id_t  hw_id = 0;

    STD_ASSERT (udf_match_id != NULL);
    STD_ASSERT (attr_list != NULL);

    SAI_UDF_LOG_TRACE ("Entering UDF Match Id creation.");

    status = dn_sai_udf_attr_list_validate (SAI_OBJECT_TYPE_UDF_MATCH,
                                            attr_count, attr_list,
                                            SAI_OP_CREATE);
    if (status != SAI_STATUS_SUCCESS) {

        SAI_UDF_LOG_ERR ("UDF Match creation attr list validation failed.");

        return status;
    }

    dn_sai_udf_lock ();

    do {
        status = sai_udf_npu_api_get()->udf_match_create (attr_count, attr_list,
                                                          &hw_id);
        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("NPU UDF Match object creation failed.");

            break;
        }
    } while (0);

    dn_sai_udf_unlock ();

    if (status != SAI_STATUS_SUCCESS) {

        SAI_UDF_LOG_ERR ("SAI UDF Match creation error: %d.", status);

    } else {

        *udf_match_id = sai_uoid_create (SAI_OBJECT_TYPE_UDF_MATCH, hw_id);

        SAI_UDF_LOG_INFO ("Created SAI UDF Match Id: 0x%"PRIx64".",
                          *udf_match_id);
    }

    return status;
}

static sai_status_t dn_sai_udf_match_remove (sai_object_id_t udf_match_id)
{
    sai_status_t  status = SAI_STATUS_FAILURE;

    SAI_UDF_LOG_TRACE ("Entering UDF Match Id: 0x%"PRIx64" remove.",
                       udf_match_id);

    if (!sai_is_obj_id_udf_match (udf_match_id)) {

        SAI_UDF_LOG_ERR ("OID 0x%"PRIx64" is not of UDF Match object type.",
                         udf_match_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    dn_sai_udf_lock ();

    do {
        /* Remove in NPU */
        status = sai_udf_npu_api_get()->udf_match_remove (udf_match_id);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("UDF Match Id: 0x%"PRIx64" remove from NPU failed.",
                             udf_match_id);

            break;
        }
    } while (0);

    dn_sai_udf_unlock ();

    SAI_UDF_LOG_TRACE ("UDF Match Id: 0x%"PRIx64" remove API returning %d.",
                       udf_match_id, status);
    return status;
}

static sai_status_t dn_sai_udf_match_set_attribute (sai_object_id_t udf_match_id,
                                                    const sai_attribute_t *attr)
{
    /* @TODO To be updated when settable attribute is added */
    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t dn_sai_udf_match_get_attribute (sai_object_id_t udf_match_id,
                                                    uint32_t attr_count,
                                                    sai_attribute_t *attr_list)
{
    sai_status_t  status = SAI_STATUS_FAILURE;

    SAI_UDF_LOG_TRACE ("Entering UDF Match Id: 0x%"PRIx64" get.", udf_match_id);

    if (!sai_is_obj_id_udf_match (udf_match_id)) {

        SAI_UDF_LOG_ERR ("OID 0x%"PRIx64" is not of UDF Match object type.",
                         udf_match_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    dn_sai_udf_lock ();

    do {
        /* Remove in NPU */
        status = sai_udf_npu_api_get()->udf_match_attribute_get (udf_match_id,
                                        attr_count, attr_list);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("UDF Match Id: 0x%"PRIx64" get from NPU failed.",
                             udf_match_id);

            break;
        }
    } while (0);

    dn_sai_udf_unlock ();

    SAI_UDF_LOG_TRACE ("Get attr for UDF Match Id: 0x%"PRIx64" returning %d.",
                       udf_match_id, status);
    return status;
}

void dn_sai_udf_match_api_fill (sai_udf_api_t *p_udf_api_table)
{
    p_udf_api_table->create_udf_match        = dn_sai_udf_match_create;
    p_udf_api_table->remove_udf_match        = dn_sai_udf_match_remove;
    p_udf_api_table->set_udf_match_attribute = dn_sai_udf_match_set_attribute;
    p_udf_api_table->get_udf_match_attribute = dn_sai_udf_match_get_attribute;
}
