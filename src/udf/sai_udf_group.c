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
* @file sai_udf_group.c
*
* @brief This file contains function definitions for the SAI UDF Group
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
#include "std_struct_utils.h"
#include "std_rbtree.h"
#include "std_llist.h"
#include "std_assert.h"
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

static inline dn_sai_udf_group_t *dn_sai_udf_group_node_alloc (void)
{
    return ((dn_sai_udf_group_t *) calloc (1, sizeof (dn_sai_udf_group_t)));
}

static inline void dn_sai_udf_group_node_free (dn_sai_udf_group_t *p_udf_group)
{
    free ((void *) p_udf_group);
}

void sai_udf_deinit (void)
{
    dn_sai_udf_global_t *p_global_param = dn_sai_udf_access_global_param ();

    if (p_global_param->is_init_complete) {
        sai_udf_npu_api_get()->udf_deinit ();
    }

    if (p_global_param->udf_group_tree != NULL) {
        std_rbtree_destroy (p_global_param->udf_group_tree);
    }

    if (p_global_param->udf_obj_tree != NULL) {
        std_rbtree_destroy (p_global_param->udf_obj_tree);
    }

    p_global_param->is_init_complete = false;

    SAI_UDF_LOG_INFO ("SAI UDF module deinitialization done.");
}

sai_status_t sai_udf_init (void)
{
    sai_status_t status = SAI_STATUS_SUCCESS;

    SAI_UDF_LOG_TRACE ("Entering SAI UDF Module Init.");

    dn_sai_udf_global_t *p_global_param = dn_sai_udf_access_global_param ();

    if (p_global_param->is_init_complete) {

        SAI_UDF_LOG_INFO ("SAI UDF Init done already.");

        return status;
    }

    memset (p_global_param, 0, sizeof (dn_sai_udf_global_t));

    do {
        status = SAI_STATUS_UNINITIALIZED;

        p_global_param->udf_group_tree =
                            std_rbtree_create_simple ("udf_group_tree",
                                  STD_STR_OFFSET_OF (dn_sai_udf_group_t, key),
                                  STD_STR_SIZE_OF (dn_sai_udf_group_t, key));

        if (NULL == p_global_param->udf_group_tree) {

            SAI_UDF_LOG_CRIT ("Failed to allocate memory for UDF group database");

            break;
        }

        p_global_param->udf_obj_tree =
                            std_rbtree_create_simple ("udf_obj_tree",
                                  STD_STR_OFFSET_OF (dn_sai_udf_t, key),
                                  STD_STR_SIZE_OF (dn_sai_udf_t, key));

        if (NULL == p_global_param->udf_obj_tree) {

            SAI_UDF_LOG_CRIT ("Failed to allocate memory for UDF obj database");

            break;
        }

        status = sai_udf_npu_api_get()->udf_init ();

        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_CRIT ("SAI UDF NPU initialization failed.");

            break;
        }

        p_global_param->is_init_complete = true;

    } while (0);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_UDF_LOG_TRACE ("Cleaning up SAI UDF global parameters.");

        sai_udf_deinit ();

    } else {

        SAI_UDF_LOG_INFO ("SAI UDF module initialization done.");
    }

    return status;
}

static inline void dn_sai_udf_group_init (dn_sai_udf_group_t *p_udf_group)
{
    STD_ASSERT (p_udf_group != NULL);

    p_udf_group->type = SAI_UDF_GROUP_TYPE_GENERIC;

    std_dll_init (&p_udf_group->udf_list);
}

static sai_status_t dn_sai_udf_group_type_set (
                                         dn_sai_udf_group_t *p_udf_group,
                                         const sai_attribute_value_t *p_value)
{
    if (!((p_value->s32 == SAI_UDF_GROUP_TYPE_GENERIC) ||
          (p_value->s32 == SAI_UDF_GROUP_TYPE_HASH))) {

        SAI_UDF_LOG_ERR ("Invalid UDF Group type value: %d.", p_value->s32);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_udf_group->type = p_value->s32;

    SAI_UDF_LOG_TRACE ("UDF Group type is set to %d.", p_udf_group->type);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_udf_group_length_set (
                                         dn_sai_udf_group_t *p_udf_group,
                                         const sai_attribute_value_t *p_value)
{
    if (p_value->u16 == 0) {

        SAI_UDF_LOG_ERR ("Invalid UDF Group length value: %u.", p_value->u16);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_udf_group->length = p_value->u16;

    SAI_UDF_LOG_TRACE ("UDF Group length is set to %u.", p_udf_group->length);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_udf_group_attr_set (dn_sai_udf_group_t *p_udf_group,
                                               uint32_t attr_count,
                                               const sai_attribute_t *attr_list)
{
    uint_t                  attr_idx;
    sai_status_t            status;
    const sai_attribute_t  *p_attr;

    STD_ASSERT (p_udf_group != NULL);
    STD_ASSERT (attr_list != NULL);

    for (attr_idx = 0, p_attr = attr_list; (attr_idx < attr_count);
         ++attr_idx, ++p_attr) {

        SAI_UDF_LOG_TRACE ("Setting UDF Group attr id: %d at list index: %u.",
                           p_attr->id, attr_idx);

        status = SAI_STATUS_SUCCESS;

        switch (p_attr->id) {

            case SAI_UDF_GROUP_ATTR_TYPE:
                status = dn_sai_udf_group_type_set (p_udf_group, &p_attr->value);
                break;

            case SAI_UDF_GROUP_ATTR_LENGTH:
                status = dn_sai_udf_group_length_set (p_udf_group, &p_attr->value);
                break;

            case SAI_UDF_GROUP_ATTR_UDF_LIST:
                SAI_UDF_LOG_ERR ("UDF list read-only attr is passed in set.");

                status = SAI_STATUS_INVALID_ATTRIBUTE_0;
                break;

            default:
                status = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }

        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("Failure in UDF Group attr list. Index: %d, "
                             "Attribute Id: %d, Error: %d.", attr_idx,
                             p_attr->id, status);

            return (sai_get_indexed_ret_val (status, attr_idx));
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_udf_group_create (sai_object_id_t *udf_group_id,
                                             _In_ sai_object_id_t switch_id,
                                             uint32_t attr_count,
                                             const sai_attribute_t *attr_list)
{
    sai_status_t         status;
    dn_sai_udf_group_t  *p_udf_group = NULL;
    sai_npu_object_id_t  hw_id = 0;

    STD_ASSERT (udf_group_id != NULL);
    STD_ASSERT (attr_list != NULL);

    SAI_UDF_LOG_TRACE ("Entering UDF Group creation.");

    status = dn_sai_udf_attr_list_validate (SAI_OBJECT_TYPE_UDF_GROUP,
                                            attr_count, attr_list,
                                            SAI_OP_CREATE);
    if (status != SAI_STATUS_SUCCESS) {

        SAI_UDF_LOG_ERR ("UDF Group creation attr list validation failed.");

        return status;
    }

    p_udf_group = dn_sai_udf_group_node_alloc ();

    if (p_udf_group == NULL) {

        SAI_UDF_LOG_CRIT ("SAI UDF Group node memory allocation failed.");

        return SAI_STATUS_NO_MEMORY;
    }

    dn_sai_udf_group_init (p_udf_group);

    dn_sai_udf_lock ();

    do {
        status = dn_sai_udf_group_attr_set (p_udf_group, attr_count,
                                            attr_list);
        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("Error in attr list for UDF Group creation");

            break;
        }

        status = sai_udf_npu_api_get()->udf_group_create (p_udf_group, &hw_id);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("NPU UDF Group creation failed.");

            break;
        }

        p_udf_group->key.group_obj_id = sai_uoid_create (SAI_OBJECT_TYPE_UDF_GROUP,
                                                         hw_id);

        t_std_error rc = std_rbtree_insert (dn_sai_udf_group_tree_handle (),
                                            p_udf_group);
        if (rc != STD_ERR_OK) {

            SAI_UDF_LOG_ERR ("Error in inserting UDF Group Id: 0x%"PRIx64" to "
                             "database", p_udf_group->key.group_obj_id);

            break;
        }
    } while (0);

    dn_sai_udf_unlock ();

    if (status != SAI_STATUS_SUCCESS) {

        SAI_UDF_LOG_ERR ("SAI UDF Group creation error: %d.", status);

        if (hw_id != 0) {

            sai_udf_npu_api_get()->udf_group_remove (p_udf_group);
        }

        dn_sai_udf_group_node_free (p_udf_group);

        p_udf_group = NULL;

    } else {

        *udf_group_id = p_udf_group->key.group_obj_id;

        SAI_UDF_LOG_INFO ("Created SAI UDF Group Id: 0x%"PRIx64".", *udf_group_id);
    }

    return status;
}

static sai_status_t dn_sai_udf_group_remove (sai_object_id_t udf_group_id)
{
    sai_status_t  status = SAI_STATUS_FAILURE;

    SAI_UDF_LOG_TRACE ("Entering UDF Group Id: 0x%"PRIx64" remove.", udf_group_id);

    if (!sai_is_obj_id_udf_group (udf_group_id)) {

        SAI_UDF_LOG_ERR ("OID 0x%"PRIx64" is not of UDF Group object type.",
                         udf_group_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    dn_sai_udf_lock ();

    do {
        dn_sai_udf_group_t *p_udf_group = dn_sai_udf_group_node_get (udf_group_id);

        if (p_udf_group == NULL) {

            SAI_UDF_LOG_ERR ("UDF Group node is not found in database for Id: "
                             "0x%"PRIx64"", udf_group_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        /* Check if there are UDF objects in the Group */
        if (p_udf_group->udf_count != 0) {

            SAI_UDF_LOG_ERR ("UDF Group Id: 0x%"PRIx64" contains UDF objects. "
                             "Num UDFs: %u.", udf_group_id, p_udf_group->udf_count);

            status = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        /* Remove from the UDF database view */
        dn_sai_udf_group_t *p_tmp_node = (dn_sai_udf_group_t *)
            std_rbtree_remove (dn_sai_udf_group_tree_handle (), p_udf_group);

        if (p_tmp_node != p_udf_group) {

            SAI_UDF_LOG_ERR ("Error in removing UDF Group Id from database.");

            break;
        }

        /* Remove in NPU */
        status = sai_udf_npu_api_get()->udf_group_remove (p_udf_group);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("UDF Group Id: 0x%"PRIx64" remove from NPU failed.",
                             udf_group_id);
            std_rbtree_insert (dn_sai_udf_group_tree_handle (), p_udf_group);

            break;
        }

        /* Free the UDF Group node */
        SAI_UDF_LOG_INFO ("Removed SAI UDF Group Id: 0x%"PRIx64".",
                          p_udf_group->key.group_obj_id);

        dn_sai_udf_group_node_free (p_udf_group);

        p_udf_group = NULL;
    } while (0);

    dn_sai_udf_unlock ();

    SAI_UDF_LOG_TRACE ("UDF Group Id: 0x%"PRIx64" remove API returning %d.",
                       udf_group_id, status);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_udf_group_set_attribute (sai_object_id_t udf_group_id,
                                                    const sai_attribute_t *attr)
{
    /* @TODO To be updated when settable attribute is added */
    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t dn_sai_udf_group_udf_list_fill (
                                        dn_sai_udf_group_t *p_udf_group,
                                        sai_object_list_t *udf_list)
{
    uint_t         index = 0;
    dn_sai_udf_t  *p_udf;

    if ((p_udf_group->udf_count > udf_list->count) ||
        (udf_list->list == NULL)) {

        SAI_UDF_LOG_ERR ("UDF list size: %u is less than the UDF count: %u in "
                         "UDF Group.", udf_list->count, p_udf_group->udf_count);

        udf_list->count = p_udf_group->udf_count;

        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    /* Fill the UDF id in the list */
    for (p_udf = dn_sai_udf_group_get_first_udf_node (p_udf_group); p_udf != NULL;
         p_udf = dn_sai_udf_group_get_next_udf_node (p_udf_group, p_udf))
    {
        udf_list->list [index] = p_udf->key.udf_obj_id;

        SAI_UDF_LOG_TRACE ("UDF list index: %d, Filled UDF Id: 0x%"PRIx64".",
                           index, udf_list->list [index]);

        index++;
    }

    udf_list->count = p_udf_group->udf_count;

    SAI_UDF_LOG_TRACE ("UDF list count: %u.", udf_list->count);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_udf_group_attr_get (
                                        dn_sai_udf_group_t *p_udf_group,
                                        uint32_t attr_count,
                                        sai_attribute_t *attr_list)
{
    uint_t            attr_idx;
    sai_status_t      status;
    sai_attribute_t  *p_attr;

    STD_ASSERT (p_udf_group != NULL);
    STD_ASSERT (attr_list != NULL);

    for (attr_idx = 0, p_attr = attr_list; (attr_idx < attr_count);
         ++attr_idx, ++p_attr) {

        SAI_UDF_LOG_TRACE ("Get UDF Group attr id: %d at list index: %u.",
                           p_attr->id, attr_idx);

        status = SAI_STATUS_SUCCESS;

        switch (p_attr->id) {

            case SAI_UDF_GROUP_ATTR_TYPE:
                p_attr->value.s32 = p_udf_group->type;
                break;

            case SAI_UDF_GROUP_ATTR_LENGTH:
                p_attr->value.u16 = p_udf_group->length;
                break;

            case SAI_UDF_GROUP_ATTR_UDF_LIST:
                status = dn_sai_udf_group_udf_list_fill (p_udf_group,
                                                         &p_attr->value.objlist);
                break;

            default:
                status = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }

        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("Failure in filling UDF Group attr list for get "
                             "API. Attribute Id: %d, Error: %d.", p_attr->id,
                             status);

            return (sai_get_indexed_ret_val (status, attr_idx));
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_udf_group_get_attribute (sai_object_id_t udf_group_id,
                                                    uint32_t attr_count,
                                                    sai_attribute_t *attr_list)
{
    sai_status_t  status = SAI_STATUS_FAILURE;

    SAI_UDF_LOG_TRACE ("Entering UDF Group Id: 0x%"PRIx64" get.", udf_group_id);

    if (!sai_is_obj_id_udf_group (udf_group_id)) {

        SAI_UDF_LOG_ERR ("OID 0x%"PRIx64" is not of UDF Group object type.",
                         udf_group_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    dn_sai_udf_lock ();

    do {
        dn_sai_udf_group_t *p_udf_group = dn_sai_udf_group_node_get (udf_group_id);

        if (p_udf_group == NULL) {

            SAI_UDF_LOG_ERR ("UDF Group node is not found in database for Id: "
                             "0x%"PRIx64"", udf_group_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        status = dn_sai_udf_group_attr_get (p_udf_group, attr_count, attr_list);

    } while (0);

    dn_sai_udf_unlock ();

    SAI_UDF_LOG_TRACE ("Get attr for UDF Group Id: 0x%"PRIx64" returning %d.",
                       udf_group_id, status);
    return status;
}

sai_status_t dn_sai_udf_group_hw_id_get (sai_object_id_t udf_group_id,
                                         sai_npu_object_id_t *p_hw_id)
{
    sai_status_t  status = SAI_STATUS_FAILURE;

    STD_ASSERT (p_hw_id != NULL);

    if (!sai_is_obj_id_udf_group (udf_group_id)) {

        SAI_UDF_LOG_ERR ("OID 0x%"PRIx64" is not of UDF Group object type.",
                         udf_group_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    dn_sai_udf_lock ();

    do {
        dn_sai_udf_group_t *p_udf_group = dn_sai_udf_group_node_get (udf_group_id);

        if (p_udf_group == NULL) {

            SAI_UDF_LOG_ERR ("UDF Group node is not found in database for Id: "
                             "0x%"PRIx64"", udf_group_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        /* Get the HW Id from NPU */
        status = sai_udf_npu_api_get()->udf_group_hw_id_get (p_udf_group,
                                                             p_hw_id);
        if (status != SAI_STATUS_SUCCESS) {

            SAI_UDF_LOG_ERR ("UDF Group Id: 0x%"PRIx64" get HW Id failed.",
                             udf_group_id);

            break;
        }

    } while (0);

    dn_sai_udf_unlock ();

    SAI_UDF_LOG_TRACE ("UDF Group Id: 0x%"PRIx64", HW Id: 0x%"PRIx64", returning %d.",
                       udf_group_id, *p_hw_id, status);
    return status;
}

sai_status_t dn_sai_udf_group_type_get (sai_object_id_t udf_group_id,
                                        sai_udf_group_type_t *type)
{
    sai_status_t  status = SAI_STATUS_SUCCESS;

    STD_ASSERT (type != NULL);

    if (!sai_is_obj_id_udf_group (udf_group_id)) {

        SAI_UDF_LOG_ERR ("OID 0x%"PRIx64" is not of UDF Group object type.",
                         udf_group_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    dn_sai_udf_lock ();

    do {
        dn_sai_udf_group_t *p_udf_group = dn_sai_udf_group_node_get (udf_group_id);

        if (p_udf_group == NULL) {

            SAI_UDF_LOG_ERR ("UDF Group node is not found in database for Id: "
                             "0x%"PRIx64"", udf_group_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        *type = p_udf_group->type;

    } while (0);

    dn_sai_udf_unlock ();

    SAI_UDF_LOG_TRACE ("UDF Group Id: 0x%"PRIx64", Type: %d, returning %d.",
                       udf_group_id, *type, status);

    return status;
}

void dn_sai_udf_group_api_fill (sai_udf_api_t *p_udf_api_table)
{
    p_udf_api_table->create_udf_group        = dn_sai_udf_group_create;
    p_udf_api_table->remove_udf_group        = dn_sai_udf_group_remove;
    p_udf_api_table->set_udf_group_attribute = dn_sai_udf_group_set_attribute;
    p_udf_api_table->get_udf_group_attribute = dn_sai_udf_group_get_attribute;
}
