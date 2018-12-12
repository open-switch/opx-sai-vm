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
* @file sai_hash_obj.c
*
* @brief This file contains function definitions for the SAI Hash object
*        functionality API.
*
*************************************************************************/

#include "saihash.h"
#include "saistatus.h"
#include "saitypes.h"
#include "saiswitch.h"
#include "saiudf.h"

#include "sai_hash_object.h"
#include "sai_oid_utils.h"
#include "sai_modules_init.h"
#include "sai_hash_api_utils.h"
#include "sai_common_infra.h"
#include "sai_common_utils.h"
#include "sai_switch_utils.h"
#include "sai_udf_api_utils.h"
#include "sai_npu_switch.h"

#include "std_mutex_lock.h"
#include "std_type_defs.h"
#include "std_struct_utils.h"
#include "std_rbtree.h"
#include "std_assert.h"
#include "std_bit_masks.h"
#include <string.h>
#include <inttypes.h>

/*
 * Global params for Hash obj db.
 */
static dn_sai_hash_obj_db_t g_hash_obj_db = {
    is_init_complete: false,
};

/**
 * Attribute Id array for SAI Hash object containing the
 * properties for create, set and get API.
 */
static const dn_sai_attribute_entry_t dn_sai_hash_attr[] =  {
    { SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST,        false, true, true, true, true, true },
    { SAI_HASH_ATTR_UDF_GROUP_LIST,           false, true, true, true, true, true },
};

static sai_attr_id_t dn_switch_hash_attr_id [] = {
    SAI_SWITCH_ATTR_ECMP_HASH,
    SAI_SWITCH_ATTR_LAG_HASH,
    SAI_SWITCH_ATTR_ECMP_HASH_IPV4,
    SAI_SWITCH_ATTR_ECMP_HASH_IPV4_IN_IPV4,
    SAI_SWITCH_ATTR_ECMP_HASH_IPV6,
    SAI_SWITCH_ATTR_LAG_HASH_IPV4,
    SAI_SWITCH_ATTR_LAG_HASH_IPV4_IN_IPV4,
    SAI_SWITCH_ATTR_LAG_HASH_IPV6,
};

static uint_t dn_sai_hash_obj_types [] = {
    DN_SAI_ECMP_NON_IP_HASH,
    DN_SAI_LAG_NON_IP_HASH,
    DN_SAI_ECMP_IPV4_HASH,
    DN_SAI_ECMP_IPV4_IN_IPV4_HASH,
    DN_SAI_ECMP_IPV6_HASH,
    DN_SAI_LAG_IPV4_HASH,
    DN_SAI_LAG_IPV4_IN_IPV4_HASH,
    DN_SAI_LAG_IPV6_HASH,
};

static void dn_sai_switch_attr_id_to_hash_type (sai_attr_id_t attr_id,
                                                uint_t *p_type)
{
    STD_ASSERT (p_type != NULL);

    size_t len = (sizeof (dn_switch_hash_attr_id)) / (sizeof (sai_attr_id_t));

    (*p_type) = translate (dn_switch_hash_attr_id, dn_sai_hash_obj_types,
                           len, attr_id, DN_SAI_ECMP_NON_IP_HASH);
}

/*
 * Lock for Hash API functions.
 */
static std_mutex_lock_create_static_init_fast (sai_hash_lock);


static inline dn_sai_hash_obj_db_t *dn_sai_access_hash_obj_db (void)
{
    return (&g_hash_obj_db);
}

static inline rbtree_handle dn_sai_hash_obj_tree_handle (void)
{
    return (dn_sai_access_hash_obj_db()->hash_obj_tree);
}

static inline uint8_t *dn_sai_hash_access_index_bitmap (void)
{
    return (dn_sai_access_hash_obj_db()->hash_index_bitmap);
}

static inline void dn_sai_hash_lock (void)
{
    std_mutex_lock (&sai_hash_lock);
}

static inline void dn_sai_hash_unlock (void)
{
    std_mutex_unlock (&sai_hash_lock);
}

static dn_sai_hash_object_t *dn_sai_switch_attr_hash_obj_get (sai_attr_id_t attr_id)
{
    uint_t type;

    dn_sai_switch_attr_id_to_hash_type (attr_id, &type);

    if (type < DN_SAI_MAX_HASH_OBJ_TYPES) {
        return (dn_sai_access_hash_obj_db()->hash_obj_types [type]);
    }

    return NULL;
}

static void dn_sai_switch_attr_hash_obj_set (sai_attr_id_t attr_id,
                                             dn_sai_hash_object_t *p_hash_obj)
{
    uint_t type;

    dn_sai_switch_attr_id_to_hash_type (attr_id, &type);

    if (type < DN_SAI_MAX_HASH_OBJ_TYPES) {
        dn_sai_access_hash_obj_db()->hash_obj_types [type] = p_hash_obj;
    }
}

static inline dn_sai_hash_object_t *dn_sai_hash_obj_node_get (
                                                sai_object_id_t obj_id)
{
    return ((dn_sai_hash_object_t *) std_rbtree_getexact (
                         dn_sai_hash_obj_tree_handle(), &obj_id));
}
/*
 * Functions used for Hash API methods
 */
static sai_status_t dn_sai_hash_attr_list_validate (
                                               uint32_t attr_count,
                                               const sai_attribute_t *attr_list,
                                               dn_sai_operations_t op_type)
{
    const dn_sai_attribute_entry_t  *p_attr_table = &dn_sai_hash_attr[0];
    uint_t max_attr_count =
                ((sizeof(dn_sai_hash_attr)) / (sizeof(dn_sai_hash_attr[0])));
    sai_status_t status;

    status = sai_attribute_validate (attr_count, attr_list, p_attr_table,
                                     SAI_OP_CREATE, max_attr_count);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_HASH_LOG_TRACE ("Attr list validation failed. op-type: %d, Error: %d.",
                             op_type, status);
    } else {

        SAI_HASH_LOG_TRACE ("Attr list validation passed. op-type: %d, Error: %d.",
                            op_type, status);
    }

    return status;
}

static sai_status_t dn_sai_hash_native_field_list_set (
                                           dn_sai_hash_object_t *p_hash_obj,
                                           const sai_s32_list_t *field_list)
{
    int32_t *new_list = NULL;

    STD_ASSERT (field_list != NULL);

    if (field_list->count) {

        STD_ASSERT (field_list->list != NULL);

        new_list = dn_sai_hash_native_field_list_alloc (field_list->count);

        if (new_list == NULL) {

            SAI_HASH_LOG_ERR ("Failure to alloc memory for Native field list");

            return SAI_STATUS_NO_MEMORY;
        }

        memcpy (new_list, field_list->list,
                (field_list->count * sizeof (int32_t)));
    }

    if (p_hash_obj->native_fields_list.list != NULL) {

        dn_sai_hash_native_field_list_free (p_hash_obj);
    }

    p_hash_obj->native_fields_list.count = field_list->count;

    p_hash_obj->native_fields_list.list = new_list;

    return SAI_STATUS_SUCCESS;
}

static bool dn_sai_hash_udf_group_list_validate (const sai_object_list_t *obj_list)
{
    STD_ASSERT (obj_list != NULL);

    uint_t                count;
    sai_status_t          status;
    sai_udf_group_type_t  type;

    for (count = 0; count < obj_list->count; count++)
    {
         if ((!sai_is_obj_id_udf_group (obj_list->list[count]))) {

             SAI_HASH_LOG_ERR ("OID 0x%"PRIx64" is not UDF Group object type.",
                               obj_list->list[count]);

             return false;
         }

         status = dn_sai_udf_group_type_get (obj_list->list[count], &type);

         if ((status != SAI_STATUS_SUCCESS) || (type != SAI_UDF_GROUP_TYPE_HASH)) {

             SAI_HASH_LOG_ERR ("Invalid UDF Group object Id 0x%"PRIx64".",
                               obj_list->list[count]);

             return false;
         }
    }

    return true;
}

static sai_status_t dn_sai_hash_udf_group_list_set (
                                           dn_sai_hash_object_t *p_hash_obj,
                                           const sai_object_list_t *obj_list)
{
    sai_object_id_t  *new_list = NULL;

    STD_ASSERT (obj_list != NULL);

    if (obj_list->count) {

        STD_ASSERT (obj_list->list != NULL);

        if (!dn_sai_hash_udf_group_list_validate (obj_list)) {

            SAI_HASH_LOG_ERR ("Invalid value passed in UDF Group list.");

            return SAI_STATUS_INVALID_ATTR_VALUE_0;
        }

        new_list = dn_sai_hash_udf_group_list_alloc (obj_list->count);

        if (new_list == NULL) {

            SAI_HASH_LOG_ERR ("Failure to alloc memory for UDF Group list");

            return SAI_STATUS_NO_MEMORY;
        }

        memcpy (new_list, obj_list->list,
                (obj_list->count * sizeof (sai_object_id_t)));
    }

    if (p_hash_obj->udf_group_list.list != NULL) {

        dn_sai_hash_udf_group_list_free (p_hash_obj);
    }

    p_hash_obj->udf_group_list.count = obj_list->count;

    p_hash_obj->udf_group_list.list = new_list;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_hash_attr_set (dn_sai_hash_object_t *p_hash_obj,
                                          uint32_t attr_count,
                                          const sai_attribute_t *attr_list,
                                          dn_sai_operations_t op_type)
{
    uint_t                  attr_idx;
    sai_status_t            status = SAI_STATUS_SUCCESS;
    const sai_attribute_t  *p_attr;

    STD_ASSERT (p_hash_obj != NULL);

    for (attr_idx = 0, p_attr = attr_list; (attr_idx < attr_count);
         ++attr_idx, ++p_attr) {

        SAI_HASH_LOG_TRACE ("Setting Hash attr id: %d at list index: %u.",
                            p_attr->id, attr_idx);

        status = SAI_STATUS_SUCCESS;

        switch (p_attr->id) {

            case SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST:
                status = dn_sai_hash_native_field_list_set (p_hash_obj,
                                                       &p_attr->value.s32list);
                break;

            case SAI_HASH_ATTR_UDF_GROUP_LIST:
                status = dn_sai_hash_udf_group_list_set (p_hash_obj,
                                                       &p_attr->value.objlist);
                break;
        }

        if (status != SAI_STATUS_SUCCESS) {

            SAI_HASH_LOG_ERR ("Failure in Hash attr list. Index: %d, Attr Id: %d"
                              ", Error: %d.", attr_idx, p_attr->id, status);

            return (sai_get_indexed_ret_val (status, attr_idx));
        }
    }

    return status;
}

static sai_status_t dn_sai_hash_obj_create (sai_object_id_t *hash_id,
                                            _In_ sai_object_id_t switch_id,
                                            uint32_t attr_count,
                                            const sai_attribute_t *attr_list)
{
    sai_status_t          status;
    int                   hash_index;
    dn_sai_hash_object_t *p_hash_obj = NULL;

    STD_ASSERT (hash_id != NULL);
    STD_ASSERT (attr_list != NULL);

    SAI_HASH_LOG_TRACE ("Entering Hash object creation.");

    status = dn_sai_hash_attr_list_validate (attr_count, attr_list,
                                             SAI_OP_CREATE);
    if (status != SAI_STATUS_SUCCESS) {

        SAI_HASH_LOG_ERR ("Hash object creation attr list validation failed.");

        return status;
    }

    hash_index = std_find_first_bit (dn_sai_hash_access_index_bitmap (),
                                     DN_SAI_MAX_HASH_INDEX, 0);

    if (hash_index < 0) {

        SAI_HASH_LOG_ERR ("Free Hash object ID not available. Max index "
                          "value initialized to %d.", DN_SAI_MAX_HASH_INDEX);

        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    p_hash_obj = dn_sai_hash_obj_node_alloc ();

    if (p_hash_obj == NULL) {

        SAI_HASH_LOG_CRIT ("Hash node memory allocation failed.");

        return SAI_STATUS_NO_MEMORY;
    }

    dn_sai_hash_lock ();

    do {
        status = dn_sai_hash_attr_set (p_hash_obj, attr_count, attr_list,
                                       SAI_OP_CREATE);
        if (status != SAI_STATUS_SUCCESS) {

            SAI_HASH_LOG_ERR ("Error in attr list for Hash object creation");

            break;
        }

        p_hash_obj->obj_id = sai_uoid_create (SAI_OBJECT_TYPE_HASH, hash_index);

        t_std_error rc = std_rbtree_insert (dn_sai_hash_obj_tree_handle (),
                                            p_hash_obj);

        if (rc != STD_ERR_OK) {

            SAI_HASH_LOG_ERR ("Error in inserting Hash Id: 0x%"PRIx64" to "
                              "database", p_hash_obj->obj_id);

            break;
        }

    } while (0);

    dn_sai_hash_unlock ();

    if (status != SAI_STATUS_SUCCESS) {

        SAI_HASH_LOG_ERR ("SAI Hash object creation error: %d.", status);

        dn_sai_hash_obj_node_free (p_hash_obj);
    } else {

        *hash_id = p_hash_obj->obj_id;

        /* Use the hash object index */
        STD_BIT_ARRAY_CLR (dn_sai_hash_access_index_bitmap (), hash_index);

        SAI_HASH_LOG_INFO ("Created SAI Hash Id: 0x%"PRIx64".", *hash_id);
    }

    return status;
}

static sai_status_t dn_sai_hash_obj_remove (sai_object_id_t hash_obj_id)
{
    sai_status_t  status = SAI_STATUS_FAILURE;
    dn_sai_hash_object_t *p_hash_obj = NULL;
    uint_t hash_index;

    SAI_HASH_LOG_TRACE ("Entering Hash Id: 0x%"PRIx64" remove.", hash_obj_id);

    if (!sai_is_obj_id_hash (hash_obj_id)) {

        SAI_HASH_LOG_ERR ("OID 0x%"PRIx64" is not of UDF object type.",
                          hash_obj_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    dn_sai_hash_lock ();

    do {
        p_hash_obj = dn_sai_hash_obj_node_get (hash_obj_id);

        if (p_hash_obj == NULL) {

            SAI_HASH_LOG_ERR ("Hash node is not found in database for Id: "
                              "0x%"PRIx64"", hash_obj_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if (dn_sai_hash_is_obj_used (p_hash_obj)) {

            SAI_HASH_LOG_ERR ("Hash object Id 0x%"PRIx64" is being used.",
                              hash_obj_id);

            status = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        dn_sai_hash_object_t *p_tmp_node = (dn_sai_hash_object_t *)
            std_rbtree_remove (dn_sai_hash_obj_tree_handle (), p_hash_obj);

        if (p_tmp_node != p_hash_obj) {

            SAI_HASH_LOG_ERR ("Error in removing Hash Id: 0x%"PRIx64" from "
                              "database.", hash_obj_id);

            break;
        }

        hash_index = (uint_t) sai_uoid_npu_obj_id_get (hash_obj_id);

        STD_BIT_ARRAY_SET (dn_sai_hash_access_index_bitmap (), hash_index);

        SAI_HASH_LOG_TRACE ("Released Hash Id: %d to free pool", hash_index);

        /* Free the Hash node */
        dn_sai_hash_obj_node_free (p_hash_obj);

        SAI_HASH_LOG_INFO ("Removed SAI Hash Id: 0x%"PRIx64".", hash_obj_id);

        status = SAI_STATUS_SUCCESS;

    } while (0);

    dn_sai_hash_unlock ();

    SAI_HASH_LOG_TRACE ("Hash Id: 0x%"PRIx64" remove API returning %d.",
                        hash_obj_id, status);
    return status;
}

static sai_status_t dn_sai_hash_obj_set_in_hw (dn_sai_hash_object_t *p_hash_obj,
                                               const sai_attribute_t *attr)
{
    const sai_s32_list_t    *p_native_fields_list = NULL;
    const sai_object_list_t *p_udf_group_list = NULL;;
    sai_attr_id_t            attr_id;
    size_t                   index;
    size_t                   clnup_index;
    sai_status_t             status;
    bool                     is_set = false;

    size_t attr_count = (sizeof (dn_switch_hash_attr_id)) / (sizeof (sai_attr_id_t));

    if (attr->id == SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST) {

        p_native_fields_list = &attr->value.s32list;
        p_udf_group_list     = &p_hash_obj->udf_group_list;

    } else if (attr->id == SAI_HASH_ATTR_UDF_GROUP_LIST) {

        p_udf_group_list     = &attr->value.objlist;
        p_native_fields_list = &p_hash_obj->native_fields_list;

    } else {

        return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
    }

    /* Apply the Hash object for all types that it is set */
    for (index = 0; index < attr_count; index++)
    {
        attr_id = dn_switch_hash_attr_id [index];

        if (dn_sai_switch_attr_hash_obj_get (attr_id) == p_hash_obj) {

            /* Set the switch hash fields in NPU */
            status = sai_switch_npu_api_get()->switch_hash_fields_set (attr_id,
                                     p_native_fields_list, p_udf_group_list);

            if (status != SAI_STATUS_SUCCESS) {

                SAI_HASH_LOG_ERR ("Hash attr set with Id: 0x%"PRIx64" failed "
                                  "for settting hash fields in NPU.",
                                  p_hash_obj->obj_id);

                break;
            }

            is_set = true;

            SAI_HASH_LOG_TRACE ("Attr Id: %d set with Hash Obj Id: 0x%"PRIx64" "
                                "fields in NPU.", attr_id, p_hash_obj->obj_id);
        }
    }

    if (status == SAI_STATUS_SUCCESS) {

        SAI_HASH_LOG_TRACE ("Set Hash Obj Id: 0x%"PRIx64" fields in NPU.",
                            p_hash_obj->obj_id);

        return status;
    }

    /* Reset the Hash object with the old fields */
    for (clnup_index = 0; is_set && clnup_index < index; clnup_index++)
    {
        SAI_HASH_LOG_TRACE ("Cleaning up Hash Obj Id: 0x%"PRIx64" in NPU.",
                            p_hash_obj->obj_id);

        attr_id = dn_switch_hash_attr_id [clnup_index];

        if (dn_sai_switch_attr_hash_obj_get (attr_id) == p_hash_obj) {

            SAI_HASH_LOG_TRACE ("Resetting Hash Obj: 0x%"PRIx64" fields for "
                                "Attr Id: %d.", p_hash_obj->obj_id, attr_id);

            /* Reset the switch hash fields in NPU */
            sai_switch_npu_api_get()->switch_hash_fields_set (attr_id,
                                     &p_hash_obj->native_fields_list,
                                     &p_hash_obj->udf_group_list);
        }
    }

    return status;
}

static sai_status_t dn_sai_hash_obj_set_attr (sai_object_id_t hash_obj_id,
                                              const sai_attribute_t *attr)
{
    sai_status_t          status = SAI_STATUS_FAILURE;
    dn_sai_hash_object_t *p_hash_obj = NULL;
    const uint_t          count = 1;

    SAI_HASH_LOG_TRACE ("Entering Hash Id: 0x%"PRIx64" set attr.", hash_obj_id);

    if (!sai_is_obj_id_hash (hash_obj_id)) {

        SAI_HASH_LOG_ERR ("OID 0x%"PRIx64" is not of UDF object type.",
                          hash_obj_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    status = dn_sai_hash_attr_list_validate (count, attr, SAI_OP_SET);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_HASH_LOG_ERR ("Hash object set attr list validation failed.");

        return status;
    }

    dn_sai_hash_lock ();

    do {
        p_hash_obj = dn_sai_hash_obj_node_get (hash_obj_id);

        if (p_hash_obj == NULL) {

            SAI_HASH_LOG_ERR ("Hash node is not found in database for Id: "
                              "0x%"PRIx64"", hash_obj_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        /* Validate the UDF Group object id */
        if (attr->id == SAI_HASH_ATTR_UDF_GROUP_LIST) {

            STD_ASSERT (attr->value.objlist.list != NULL);

            if (!dn_sai_hash_udf_group_list_validate (&attr->value.objlist)) {

                SAI_HASH_LOG_ERR ("Invalid value passed in UDF Group list.");

                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
        }

        /* Set the hash fields in NPU if the hash object is used already */
        if (dn_sai_hash_is_obj_used (p_hash_obj)) {

            status = dn_sai_hash_obj_set_in_hw (p_hash_obj, attr);

            if (status != SAI_STATUS_SUCCESS) {

                SAI_HASH_LOG_ERR ("Failed to apply the hash obj fields in HW.");

                break;
            }
        }

        status = dn_sai_hash_attr_set (p_hash_obj, count, attr, SAI_OP_SET);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_HASH_LOG_ERR ("Error in settting the attr in Hash object node.");

            break;
        }

    } while (0);

    dn_sai_hash_unlock ();

    if (status != SAI_STATUS_SUCCESS) {

        SAI_HASH_LOG_ERR ("SAI Hash Id: 0x%"PRIx64" set error: %d.",
                          hash_obj_id, status);
    } else {

        SAI_HASH_LOG_INFO ("SAI Hash Id: 0x%"PRIx64" attr set success.",
                           hash_obj_id);
    }

    return status;
}

static sai_status_t dn_sai_hash_attr_get (dn_sai_hash_object_t *p_hash_obj,
                                          uint32_t attr_count,
                                          sai_attribute_t *attr_list)
{
    uint_t            attr_idx;
    sai_status_t      status = SAI_STATUS_SUCCESS;
    sai_attribute_t  *p_attr;

    STD_ASSERT (p_hash_obj != NULL);

    for (attr_idx = 0, p_attr = attr_list; (attr_idx < attr_count);
         ++attr_idx, ++p_attr) {

        SAI_HASH_LOG_TRACE ("Getting UDF attr id: %d at list index: %u.",
                            p_attr->id, attr_idx);

        status = SAI_STATUS_SUCCESS;

        switch (p_attr->id) {

            case SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST:
                if ((p_attr->value.s32list.count) <
                    (p_hash_obj->native_fields_list.count)) {

                    SAI_HASH_LOG_ERR ("Not enough space in list to fill native"
                                      " hash fields.");

                    status = SAI_STATUS_BUFFER_OVERFLOW;
                    p_attr->value.s32list.count = p_hash_obj->native_fields_list.count;
                    break;
                }

                p_attr->value.s32list.count = p_hash_obj->native_fields_list.count;
                memcpy (p_attr->value.s32list.list, p_hash_obj->native_fields_list.list,
                       (p_hash_obj->native_fields_list.count * sizeof (int32_t)));
                break;

            case SAI_HASH_ATTR_UDF_GROUP_LIST:
                if ((p_attr->value.objlist.count) <
                    (p_hash_obj->udf_group_list.count)) {

                    SAI_HASH_LOG_ERR ("Not enough space in list to fill udf"
                                      " group hash fields.");

                    p_attr->value.objlist.count = p_hash_obj->udf_group_list.count;
                    status = SAI_STATUS_BUFFER_OVERFLOW;
                    break;
                }

                p_attr->value.objlist.count = p_hash_obj->udf_group_list.count;
                memcpy (p_attr->value.objlist.list, p_hash_obj->udf_group_list.list,
                       (p_hash_obj->udf_group_list.count * sizeof (sai_object_id_t)));
                break;
        }

        if (status != SAI_STATUS_SUCCESS) {

            SAI_HASH_LOG_ERR ("Failure in Hash attr list. Index: %d, Attr Id: %d"
                              ", Error: %d.", attr_idx, p_attr->id, status);

            return (sai_get_indexed_ret_val (status, attr_idx));
        }
    }

    return status;
}

static sai_status_t dn_sai_hash_obj_get_attr (sai_object_id_t hash_obj_id,
                                              uint32_t attr_count,
                                              sai_attribute_t *attr_list)
{
    sai_status_t  status;

    SAI_HASH_LOG_TRACE ("Entering Hash Id: 0x%"PRIx64" set attr.", hash_obj_id);

    if (!sai_is_obj_id_hash (hash_obj_id)) {

        SAI_HASH_LOG_ERR ("OID 0x%"PRIx64" is not of UDF object type.",
                          hash_obj_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    status = dn_sai_hash_attr_list_validate (attr_count, attr_list, SAI_OP_GET);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_HASH_LOG_ERR ("Hash object get attr list validation failed.");

        return status;
    }

    dn_sai_hash_lock ();

    do {
        dn_sai_hash_object_t *p_hash_obj = dn_sai_hash_obj_node_get (hash_obj_id);

        if (p_hash_obj == NULL) {

            SAI_HASH_LOG_ERR ("Hash node is not found in database for Id: "
                              "0x%"PRIx64"", hash_obj_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        status = dn_sai_hash_attr_get (p_hash_obj, attr_count, attr_list);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_HASH_LOG_ERR ("Error in filling Hash object attributes.");

            break;
        }

    } while (0);

    dn_sai_hash_unlock ();

    return status;
}

/*
 * Functions used for Initialization
 */
static sai_status_t dn_sai_hash_default_hash_obj_remove (sai_attr_id_t attr_id)
{
    sai_status_t  status = SAI_STATUS_FAILURE;
    dn_sai_hash_object_t *p_hash_obj = NULL;

    p_hash_obj = dn_sai_switch_attr_hash_obj_get (attr_id);

    if (p_hash_obj == NULL) {

       SAI_HASH_LOG_TRACE ("Hash object not set for attr id: %d", attr_id);

       return SAI_STATUS_SUCCESS;
    }

    /* Reset the default hash object in Switch attr to NULL */
    status = dn_sai_switch_hash_attr_set (attr_id, SAI_NULL_OBJECT_ID);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_HASH_LOG_ERR ("Failure in removing default hash object for attr Id"
                          " :%d.", attr_id);

        return status;
    }

    /* Remove the Hash object */
    status = dn_sai_hash_obj_remove (p_hash_obj->obj_id);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_HASH_LOG_ERR ("Failure in removing default hash object.");

        return status;
    }

    return status;
}

void sai_hash_obj_deinit (void)
{
    dn_sai_hash_obj_db_t *p_hash_obj_db = dn_sai_access_hash_obj_db ();

    if (p_hash_obj_db->is_init_complete) {

        /* Remove the Default ECMP Hash object */
        dn_sai_hash_default_hash_obj_remove (SAI_SWITCH_ATTR_ECMP_HASH);

        /* Remove the Default LAG Hash object */
        dn_sai_hash_default_hash_obj_remove (SAI_SWITCH_ATTR_LAG_HASH);
    }

    if (p_hash_obj_db->hash_obj_tree != NULL) {
        std_rbtree_destroy (p_hash_obj_db->hash_obj_tree);
    }

    std_bitmaparray_free_data (p_hash_obj_db->hash_index_bitmap);

    p_hash_obj_db->is_init_complete = false;

    memset (p_hash_obj_db, 0, sizeof (dn_sai_hash_obj_db_t));

    SAI_HASH_LOG_INFO ("SAI Hash module deinitialization done.");
}

static sai_status_t dn_sai_hash_default_hash_obj_create (sai_attr_id_t attr_id)
{
    sai_s32_list_t   hash_field_list;
    uint_t           max_field_count = sai_switch_max_native_hash_fields ();
    int32_t          default_fields [max_field_count];
    const uint32_t   count = 1;
    sai_attribute_t  attr;
    sai_object_id_t  hash_obj_id;
    sai_status_t     status;

    hash_field_list.count = SAI_SWITCH_DEFAULT_HASH_FIELDS_COUNT;
    hash_field_list.list  = default_fields;

    status = sai_switch_default_native_hash_fields_get (&hash_field_list);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_HASH_LOG_ERR ("Failure to get default native hash fields.");

        return status;
    }

    attr.value.s32list = hash_field_list;
    attr.id            = SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST;

    status = dn_sai_hash_obj_create (&hash_obj_id, sai_switch_id_get(),
                                     count, &attr);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_HASH_LOG_ERR ("Failure in creating default hash object.");

        return status;
    }

    /* Set the default hash object in Switch attr */
    status = dn_sai_switch_hash_attr_set (attr_id, hash_obj_id);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_HASH_LOG_ERR ("Failure in setting default hash object for attr id: "
                          "%d.", attr_id);

        dn_sai_hash_obj_remove (hash_obj_id);
    }

    return status;
}

sai_status_t sai_hash_obj_init (void)
{
    sai_status_t status = SAI_STATUS_SUCCESS;

    dn_sai_hash_obj_db_t *p_hash_obj_db = dn_sai_access_hash_obj_db ();

    if (p_hash_obj_db->is_init_complete) {

        SAI_HASH_LOG_INFO ("SAI Hash object Init done already.");

        return status;
    }

    memset (p_hash_obj_db, 0, sizeof (dn_sai_hash_obj_db_t));

    do {
        status = SAI_STATUS_UNINITIALIZED;

        p_hash_obj_db->hash_obj_tree =
                  std_rbtree_create_simple ("hash_obj_tree",
                      STD_STR_OFFSET_OF (dn_sai_hash_object_t, obj_id),
                      STD_STR_SIZE_OF (dn_sai_hash_object_t, obj_id));

        if (NULL == p_hash_obj_db->hash_obj_tree) {

            SAI_HASH_LOG_CRIT ("Failed to allocate memory for Hash object tree");

            break;
        }

        p_hash_obj_db->hash_index_bitmap =
                             std_bitmap_create_array (DN_SAI_MAX_HASH_INDEX);

        if (NULL == p_hash_obj_db->hash_index_bitmap) {

            SAI_HASH_LOG_CRIT ("Failed to initialize Hash object Id bitmap");

            break;
        }

        /* Create the default ECMP Hash object */
        status = dn_sai_hash_default_hash_obj_create (SAI_SWITCH_ATTR_ECMP_HASH);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_HASH_LOG_CRIT ("Failure to create default ECMP Hash object");

            break;
        }

        /* Create the default LAG Hash object */
        status = dn_sai_hash_default_hash_obj_create (SAI_SWITCH_ATTR_LAG_HASH);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_HASH_LOG_CRIT ("Failure to create default LAG Hash object");

            break;
        }

        p_hash_obj_db->is_init_complete = true;

    } while (0);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_HASH_LOG_TRACE ("Cleaning up SAI Hash global parameters.");

        sai_hash_obj_deinit ();

    } else {

        SAI_HASH_LOG_INFO ("SAI Hash module initialization done.");
    }

    return status;
}

/*
 * Functions used for Switch Hash Attributes
 */
sai_status_t dn_sai_switch_hash_attr_set (sai_attr_id_t attr_id,
                                          sai_object_id_t hash_obj_id)
{
    dn_sai_hash_object_t *p_hash_obj = NULL;
    dn_sai_hash_object_t *p_old_hash_obj = NULL;
    sai_s32_list_t        native_fields_list;
    sai_object_list_t     udf_group_list;
    sai_status_t          status;

    SAI_HASH_LOG_TRACE ("Entering to set Hash Id: 0x%"PRIx64" for attr Id: %d.",
                        hash_obj_id, attr_id);

    memset (&native_fields_list, 0, sizeof (sai_s32_list_t));
    memset (&udf_group_list, 0, sizeof (sai_object_list_t));

    dn_sai_hash_lock ();

    do {
        if (hash_obj_id != SAI_NULL_OBJECT_ID) {

            if (!sai_is_obj_id_hash (hash_obj_id)) {

                SAI_HASH_LOG_ERR ("OID 0x%"PRIx64" is not of UDF object type.",
                                  hash_obj_id);

                return SAI_STATUS_INVALID_OBJECT_TYPE;
            }

            p_hash_obj = dn_sai_hash_obj_node_get (hash_obj_id);

            if (p_hash_obj == NULL) {

                SAI_HASH_LOG_ERR ("Hash node is not found in database for Id: "
                                  "0x%"PRIx64"", hash_obj_id);

                status = SAI_STATUS_INVALID_OBJECT_ID;
                break;
            }

            native_fields_list = p_hash_obj->native_fields_list;
            udf_group_list     = p_hash_obj->udf_group_list;
        }

        /* Set the switch hash fields in NPU */
        status = sai_switch_npu_api_get()->switch_hash_fields_set (attr_id,
                                       &native_fields_list, &udf_group_list);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_HASH_LOG_TRACE ("Switch Hash attr set with Id: 0x%"PRIx64" failed "
                                "for settting hash fields in NPU.", hash_obj_id);

            break;
        }

        p_old_hash_obj = dn_sai_switch_attr_hash_obj_get (attr_id);

        if (p_old_hash_obj) {

            p_old_hash_obj->ref_count--;
        }

        dn_sai_switch_attr_hash_obj_set (attr_id, p_hash_obj);

        if (p_hash_obj) {

            p_hash_obj->ref_count++;
        }

    } while (0);

    dn_sai_hash_unlock ();

    if (status != SAI_STATUS_SUCCESS) {

        SAI_HASH_LOG_ERR ("Switch Hash object set returning error: %d.", status);

    } else {

        SAI_HASH_LOG_INFO ("Switch Hash attr is set to Hash object Id: "
                           "0x%"PRIx64".", hash_obj_id);
    }

    return status;
}

sai_status_t dn_sai_switch_hash_attr_get (sai_attribute_t *attr)
{
    STD_ASSERT (attr != NULL);

    dn_sai_hash_object_t *p_hash_obj = NULL;

    SAI_HASH_LOG_TRACE ("Entering to get switch attr Id: %d.", attr->id);

    attr->value.oid = SAI_NULL_OBJECT_ID;

    dn_sai_hash_lock ();

    do {
        p_hash_obj = dn_sai_switch_attr_hash_obj_get (attr->id);

        if (p_hash_obj) {

            attr->value.oid = p_hash_obj->obj_id;
        }

    } while (0);

    dn_sai_hash_unlock ();

    SAI_HASH_LOG_TRACE ("Switch attr Id: %d get returning 0x%"PRIx64".",
                        attr->id, attr->value.oid);

    return SAI_STATUS_SUCCESS;
}

/*
 * API method table
 */
static sai_hash_api_t sai_hash_api_method_table = {
    dn_sai_hash_obj_create,
    dn_sai_hash_obj_remove,
    dn_sai_hash_obj_set_attr,
    dn_sai_hash_obj_get_attr,
};

sai_hash_api_t *sai_hash_api_query (void)
{
    return &sai_hash_api_method_table;
}

