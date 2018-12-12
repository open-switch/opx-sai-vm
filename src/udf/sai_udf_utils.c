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
* @file sai_udf_utils.c
*
* @brief This file contains utility function definitions for the SAI UDF
*        APIs.
*
*************************************************************************/
#include "saiudf.h"

#include "sai_udf_api_utils.h"
#include "sai_udf_common.h"
#include "sai_common_utils.h"

#include "std_mutex_lock.h"
#include "std_llist.h"
#include "std_assert.h"
#include <string.h>

/*
 * Global variable.
 */
static dn_sai_udf_global_t  g_sai_udf_param = {
    is_init_complete: false,
};

static sai_udf_api_t sai_udf_api_method_table;

/*
 * Accessor Function for g_sai_udf_param
 */
dn_sai_udf_global_t *dn_sai_udf_access_global_param (void)
{
    return (&g_sai_udf_param);
}

/*
 * Lock for UDF API functions
 */
static std_mutex_lock_create_static_init_fast (sai_udf_lock);

void dn_sai_udf_lock ()
{
    std_mutex_lock (&sai_udf_lock);
}

void dn_sai_udf_unlock ()
{
    std_mutex_unlock (&sai_udf_lock);
}

/**
 * Attribute Id array for UDF Group object containing the
 * properties for create, set and get API.
 */
static const dn_sai_attribute_entry_t dn_sai_udf_group_attr[] =  {
    { SAI_UDF_GROUP_ATTR_TYPE,        false, true, false, true, true, true },
    { SAI_UDF_GROUP_ATTR_LENGTH,            true, true, false, true, true, true },
    { SAI_UDF_GROUP_ATTR_UDF_LIST,    false, false, false, true, true, true },
};

/**
 * Attribute Id array for UDF object containing the properties for
 * create, set and get API.
 */
static const dn_sai_attribute_entry_t dn_sai_udf_attr[] =  {
    { SAI_UDF_ATTR_MATCH_ID,    true, true, false, true, true, true },
    { SAI_UDF_ATTR_GROUP_ID,    true, true, false, true, true, true },
    { SAI_UDF_ATTR_BASE,        false, true, true, true, true, true },
    { SAI_UDF_ATTR_OFFSET,      true, true, true, true, true, true },
    { SAI_UDF_ATTR_HASH_MASK,   false, true, true, true, true, true },
};

/**
 * Attribute Id array for UDF Match containing the properties for
 * create, set and get API.
 */
static const dn_sai_attribute_entry_t dn_sai_udf_match_attr[] =  {
    { SAI_UDF_MATCH_ATTR_L2_TYPE,      false, true, false, true, true,  true },
    { SAI_UDF_MATCH_ATTR_L3_TYPE,      false, true, false, true, true,  true },
    { SAI_UDF_MATCH_ATTR_GRE_TYPE,     false, true, false, true, true,  true },
    { SAI_UDF_MATCH_ATTR_PRIORITY,     false, true, false, true, true,  true },
};

/**
 * Function returning the attribute array for UDF Group object and
 * count of the total number of attributes.
 */
static inline void dn_sai_udf_group_attr_table_get (
                                 const dn_sai_attribute_entry_t **p_attr_table,
                                 uint_t *p_attr_count)
{
    *p_attr_table = &dn_sai_udf_group_attr [0];

    *p_attr_count = (sizeof(dn_sai_udf_group_attr)) /
                     (sizeof(dn_sai_udf_group_attr[0]));
}

/**
 * Function returning the attribute array for UDF object and
 * count of the total number of attributes.
 */
static inline void dn_sai_udf_obj_attr_table_get (
                                const dn_sai_attribute_entry_t **p_attr_table,
                                uint_t *p_attr_count)
{
    *p_attr_table = &dn_sai_udf_attr [0];

    *p_attr_count = (sizeof(dn_sai_udf_attr)) / (sizeof(dn_sai_udf_attr[0]));
}

/**
 * Function returning the attribute array for UDF Match object and
 * count of the total number of attributes.
 */
static inline void dn_sai_udf_match_attr_table_get (
                                 const dn_sai_attribute_entry_t **p_attr_table,
                                 uint_t *p_attr_count)
{
    *p_attr_table = &dn_sai_udf_match_attr[0];

    *p_attr_count = (sizeof(dn_sai_udf_match_attr)) /
                     (sizeof(dn_sai_udf_match_attr[0]));
}

static void dn_sai_udf_attr_table_get (sai_object_type_t obj_type,
                                  const dn_sai_attribute_entry_t **p_attr_table,
                                  uint_t *p_attr_count)
{
    if (obj_type == SAI_OBJECT_TYPE_UDF) {

        dn_sai_udf_obj_attr_table_get (p_attr_table, p_attr_count);

    } else if (obj_type == SAI_OBJECT_TYPE_UDF_GROUP) {

        dn_sai_udf_group_attr_table_get (p_attr_table, p_attr_count);

    } else if (obj_type == SAI_OBJECT_TYPE_UDF_MATCH) {

        dn_sai_udf_match_attr_table_get (p_attr_table, p_attr_count);
    }
}

sai_status_t dn_sai_udf_attr_list_validate (sai_object_type_t obj_type,
                                            uint32_t attr_count,
                                            const sai_attribute_t *attr_list,
                                            dn_sai_operations_t op_type)
{
    const dn_sai_attribute_entry_t  *p_attr_table = NULL;
    uint_t                           max_attr_count = 0;
    sai_status_t                     status = SAI_STATUS_FAILURE;

    if (!((obj_type == SAI_OBJECT_TYPE_UDF) ||
          (obj_type == SAI_OBJECT_TYPE_UDF_MATCH) ||
          (obj_type == SAI_OBJECT_TYPE_UDF_GROUP))) {

        SAI_UDF_LOG_ERR ("Invalid UDF obj type %d passed.", obj_type);

        return status;
    }

    if (!((op_type == SAI_OP_CREATE) || (op_type == SAI_OP_SET) ||
          (op_type == SAI_OP_GET))) {

        SAI_UDF_LOG_ERR ("Invalid optype %d passed for validation.", op_type);

        return status;
    }

    dn_sai_udf_attr_table_get (obj_type, &p_attr_table, &max_attr_count);

    if ((p_attr_table == NULL) || (max_attr_count == 0)) {
        /* Not expected */
        SAI_UDF_LOG_ERR ("Unable to find attribute id table.");

        return status;
    }

    status = sai_attribute_validate (attr_count, attr_list, p_attr_table,
                                     op_type, max_attr_count);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_UDF_LOG_ERR ("Attr list validation failed. op-type: %d, Error: %d.",
                         op_type, status);
    } else {

        SAI_UDF_LOG_TRACE ("Attr list validation passed. op-type: %d, Error: %d.",
                           op_type, status);
    }

    return status;
}

/*
 * UDF API method table query function
 */
sai_udf_api_t *sai_udf_api_query (void)
{
    dn_sai_udf_group_api_fill (&sai_udf_api_method_table);
    dn_sai_udf_match_api_fill (&sai_udf_api_method_table);
    dn_sai_udf_api_fill (&sai_udf_api_method_table);

    return (&sai_udf_api_method_table);
}

dn_sai_udf_t *dn_sai_udf_node_get (sai_object_id_t udf_id)
{
    dn_sai_udf_t    udf_node;

    memset (&udf_node, 0, sizeof (dn_sai_udf_t));

    udf_node.key.udf_obj_id = udf_id;

    return ((dn_sai_udf_t *) std_rbtree_getexact (dn_sai_udf_tree_handle (),
                                                  &udf_node));
}

dn_sai_udf_group_t *dn_sai_udf_group_node_get (sai_object_id_t udf_group_id)
{
    dn_sai_udf_group_t udf_group_node;

    memset (&udf_group_node, 0, sizeof (dn_sai_udf_group_t));

    udf_group_node.key.group_obj_id = udf_group_id;

    return ((dn_sai_udf_group_t *) std_rbtree_getexact (
                                               dn_sai_udf_group_tree_handle (),
                                               &udf_group_node));
}

dn_sai_udf_t *dn_sai_udf_group_get_first_udf_node (dn_sai_udf_group_t *p_udf_group)
{
    STD_ASSERT (p_udf_group != NULL);

    return ((dn_sai_udf_t *) std_dll_getfirst (&p_udf_group->udf_list));
}

dn_sai_udf_t *dn_sai_udf_group_get_next_udf_node (dn_sai_udf_group_t *p_udf_group,
                                               dn_sai_udf_t *p_udf_node)
{
    STD_ASSERT (p_udf_group != NULL);
    STD_ASSERT (p_udf_node != NULL);

    return ((dn_sai_udf_t *) std_dll_getnext (&p_udf_group->udf_list,
                                              &p_udf_node->node_link));
}



