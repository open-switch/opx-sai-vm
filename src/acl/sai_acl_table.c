/************************************************************************
 * * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
 * *
 * * This source code is confidential, proprietary, and contains trade
 * * secrets that are the sole property of Dell Inc.
 * * Copy and/or distribution of this source code or disassembly or reverse
 * * engineering of the resultant object code are strictly forbidden without
 * * the written consent of Dell Inc.
 * *
 * ************************************************************************/
 /**
 * * @file sai_acl_table.c
 * *
 * * @brief This file contains functions for SAI ACL Table operations.
 * *
 * *
 * *************************************************************************/
#include "sai_acl_type_defs.h"
#include "sai_acl_npu_api.h"
#include "sai_common_acl.h"
#include "sai_acl_utils.h"

#include "saitypes.h"
#include "saiacl.h"
#include "saistatus.h"

#include "std_type_defs.h"
#include "std_rbtree.h"
#include "std_llist.h"
#include "std_assert.h"
#include "sai_oid_utils.h"
#include "sai_common_infra.h"
#include "sai_udf_api_utils.h"

#include <stdlib.h>
#include <inttypes.h>

static sai_object_id_t sai_acl_table_id_alloc(void)
{
    uint_t table_idx = 0;
    sai_acl_table_id_node_t *acl_tbl_node = sai_acl_get_table_id_gen();

    STD_ASSERT(acl_tbl_node != NULL);

    for (table_idx = 0; table_idx < SAI_ACL_TABLE_ID_MAX; table_idx++) {
        if (acl_tbl_node[table_idx].table_in_use == false) {
            acl_tbl_node[table_idx].table_in_use = true;
            return (sai_uoid_create (SAI_OBJECT_TYPE_ACL_TABLE,
                    acl_tbl_node[table_idx].table_id));
        }
    }
    return SAI_ACL_INVALID_TABLE_ID;
}


static void sai_acl_table_id_dealloc(sai_object_id_t table_id)
{
    uint_t table_idx = 0;
    sai_acl_table_id_node_t *acl_tbl_node = sai_acl_get_table_id_gen();
    STD_ASSERT(acl_tbl_node != NULL);

    for (table_idx = 0; table_idx < SAI_ACL_TABLE_ID_MAX; table_idx++) {
        if (acl_tbl_node[table_idx].table_id == sai_uoid_npu_obj_id_get(table_id)) {
            acl_tbl_node[table_idx].table_in_use = false;
            return;
        }
    }
}

static int acl_rule_priority_compare(const void *current,
                                     const void *node,
                                     uint_t len)
{
    sai_acl_rule_t *src_acl_rule = (sai_acl_rule_t *)current;
    sai_acl_rule_t *dst_acl_rule = (sai_acl_rule_t *)node;

    STD_ASSERT(src_acl_rule != NULL);
    STD_ASSERT(dst_acl_rule != NULL);

    if (src_acl_rule->acl_rule_priority > dst_acl_rule->acl_rule_priority) {
        return 1;
    } else if (src_acl_rule->acl_rule_priority
               < dst_acl_rule->acl_rule_priority) {
        return -1;
    }
    return 0;
}

static void sai_acl_table_init(sai_acl_table_t *acl_table)
{
    STD_ASSERT(acl_table != NULL);
    std_dll_init_sort (&acl_table->rule_head, acl_rule_priority_compare,
                       SAI_ACL_RULE_DLL_GLUE_OFFSET,
                       SAI_ACL_RULE_DLL_GLUE_SIZE);
}

static bool sai_acl_table_same_priority(uint_t table_priority,
                                        sai_acl_stage_t table_stage)
{
    acl_node_pt acl_node = NULL;
    sai_acl_table_t *acl_table = NULL;

    SAI_ACL_LOG_TRACE ("Searching for table with "
                       "same priority %d", table_priority);
    acl_node = sai_acl_get_acl_node();

    STD_ASSERT(acl_node->sai_acl_table_tree != NULL);

    acl_table = (sai_acl_table_t *)
                 std_rbtree_getfirst(acl_node->sai_acl_table_tree);

    if (acl_table == NULL) {
        SAI_ACL_LOG_TRACE ("No ACL Table present "
                           "in the ACL Table RB tree");
        return false;
    }

    while(acl_table) {
        if ((table_priority == acl_table->acl_table_priority) &&
            (table_stage == acl_table->acl_stage)) {
            SAI_ACL_LOG_TRACE ("Found ACL Table with same priority %d "
                               "Existing Table Id 0x%"PRIx64"", table_priority,
                               acl_table->table_key.acl_table_id);

            return true;
        }

        acl_table =(sai_acl_table_t *)
                   std_rbtree_getnext(acl_node->sai_acl_table_tree, acl_table);
    }

    SAI_ACL_LOG_TRACE ("ACL Table with same priority %d does not "
                       "exist in ACL db", table_priority);

    return false;
}

static bool sai_acl_table_group_id_exist(sai_object_id_t table_group_id,
                                         sai_acl_stage_t table_stage)
{
    acl_node_pt acl_node = NULL;
    sai_acl_table_t *acl_table = NULL;

    SAI_ACL_LOG_TRACE ("Searching for table with same Table Group Id "
                       "0x%"PRIx64"", table_group_id);

    acl_node = sai_acl_get_acl_node();

    STD_ASSERT(acl_node->sai_acl_table_tree != NULL);

    acl_table = (sai_acl_table_t *)
                 std_rbtree_getfirst(acl_node->sai_acl_table_tree);

    if (acl_table == NULL) {
        SAI_ACL_LOG_TRACE ("No ACL Table present "
                           "in the ACL Table RB tree");
        return false;
    }

    while(acl_table) {
        if ((table_group_id == acl_table->table_group_id) &&
           (table_stage == acl_table->acl_stage)) {

            SAI_ACL_LOG_TRACE ("Found ACL Table with same group Id 0x%"PRIx64" "
                               "Existing Table Id 0x%"PRIx64"", table_group_id,
                               acl_table->table_key.acl_table_id);
            return true;
        }

        acl_table =(sai_acl_table_t *)
                   std_rbtree_getnext(acl_node->sai_acl_table_tree, acl_table);
    }

    SAI_ACL_LOG_TRACE ("ACL Table with same group Id 0x%"PRIx64" does not "
                       "exist in ACL db", table_group_id);

    return false;
}

static sai_status_t sai_acl_table_udf_field_validate(sai_object_id_t udf_group_id)
{
    dn_sai_udf_group_t *p_udf_group = NULL;

    /*
     * TODO: New API will be added to obtain the group type. Below code
     * should be replaced with the API once available.
     */
    p_udf_group = dn_sai_udf_group_node_get (udf_group_id);

    if (p_udf_group == NULL) {
        SAI_ACL_LOG_ERR ("UDF Group node missing for UDF Group Id: "
                         "0x%"PRIx64"", udf_group_id);
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    if (p_udf_group->type != SAI_UDF_GROUP_TYPE_GENERIC) {
        SAI_ACL_LOG_ERR ("UDF Group Type not generic for UDF Group Id: "
                         "0x%"PRIx64"", udf_group_id);
        return SAI_STATUS_ATTR_NOT_SUPPORTED_0;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_table_insert(rbtree_handle sai_acl_table_tree,
                                         sai_acl_table_t *acl_table)
{
    STD_ASSERT(sai_acl_table_tree != NULL);
    STD_ASSERT(acl_table != NULL);

    return (std_rbtree_insert(sai_acl_table_tree, acl_table)
            == STD_ERR_OK ? SAI_STATUS_SUCCESS: SAI_STATUS_FAILURE);
}

static sai_acl_table_t *sai_acl_table_remove(rbtree_handle sai_acl_table_tree,
                                             sai_acl_table_t *acl_table)
{
    STD_ASSERT(sai_acl_table_tree != NULL);
    STD_ASSERT(acl_table != NULL);

    return ((sai_acl_table_t *)std_rbtree_remove(
            sai_acl_table_tree, acl_table));
}

static void sai_acl_table_free(sai_acl_table_t *acl_table)
{
    STD_ASSERT(acl_table != NULL);

    if (acl_table->udf_field_count != 0) {
        free (acl_table->udf_field_list);
    }

    free(acl_table->field_list);
    free(acl_table->action_list);
    free (acl_table);
}

static sai_status_t sai_acl_table_validate_attributes(uint_t attr_count,
                                               const sai_attribute_t *attr_list,
                                               uint_t *num_fields,
                                               uint_t *num_udf_fields,
                                               uint_t *num_actions)
{
    uint_t field_count = 0, attribute_count = 0;
    uint_t dup_index = 0, udf_field_count = 0;
    sai_attr_id_t attribute_id = 0;
    sai_attribute_value_t attribute_value = {0};
    bool attr_stage_present = false, attr_prio_present = false;
    sai_acl_stage_t table_stage = SAI_ACL_STAGE_INGRESS;
    sai_status_t rc = SAI_STATUS_SUCCESS;
    uint_t action_count = 0;

    STD_ASSERT(attr_count > 0);
    STD_ASSERT(attr_list != NULL);
    STD_ASSERT(num_fields != NULL);
    STD_ASSERT(num_udf_fields != NULL);
    STD_ASSERT(num_actions != NULL);

    SAI_ACL_LOG_TRACE ("Validating table attributes");

    /* ACL Table Stage is required prior to perform Table validations */
    for (attribute_count = 0; attribute_count < attr_count;
         attribute_count++) {

         attribute_id = attr_list[attribute_count].id;
         attribute_value = attr_list[attribute_count].value;

         if (attribute_id == SAI_ACL_TABLE_ATTR_ACL_STAGE) {
             if(sai_acl_valid_stage(attribute_value.s32) == false) {
                return sai_get_indexed_ret_val (SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                attribute_count);
             }
             table_stage = attribute_value.s32;
         }
    }

    /* ACL Table Validations */
    for (attribute_count = 0; attribute_count < attr_count;
         attribute_count++) {
        attribute_id = attr_list[attribute_count].id;

        if (!sai_acl_table_valid_attr_range(attribute_id)) {
            SAI_ACL_LOG_ERR ("Attribute ID %d not "
                             "in ACL Table Attribute range",
                             attribute_id);
            return sai_get_indexed_ret_val (SAI_STATUS_INVALID_ATTRIBUTE_0,
                                            attribute_count);
        }

        /* Basic Validation passed, get the attribute value */
        attribute_value = attr_list[attribute_count].value;

        switch (attribute_id) {
            /* Mandatory Attributes */
            case SAI_ACL_TABLE_ATTR_ACL_STAGE:
                attr_stage_present = true;
                break;
            case SAI_ACL_TABLE_ATTR_PRIORITY:
                if (sai_acl_table_same_priority(attribute_value.u32, table_stage)) {
                    SAI_ACL_LOG_ERR ("Found ACL Table with same priority %d",
                                     attribute_value.u32);
                    return sai_get_indexed_ret_val (SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                    attribute_count);
                }
                attr_prio_present = true;
                break;
            /* Optional Attributes */
            case SAI_ACL_TABLE_ATTR_SIZE:
                /* NPU specific validations only */
                break;
            case SAI_ACL_TABLE_ATTR_GROUP_ID:
                if (!sai_is_obj_id_acl_table_group(attribute_value.oid)) {
                    SAI_ACL_LOG_ERR ("Invalid ACL Table Group Object Type");
                    return sai_get_indexed_ret_val (SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                    attribute_count);
                }
                if (!sai_acl_table_group_id_exist(attribute_value.oid, table_stage))  {
                    SAI_ACL_LOG_ERR ("Table Group Id 0x%"PRIx64" does not exist",
                                     attribute_value.oid);
                    return sai_get_indexed_ret_val (SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                    attribute_count);
                }
                break;
            case SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST:
                /* ACL Table Actions */
                action_count = attribute_value.s32list.count;
                SAI_ACL_LOG_TRACE ("Actions count : %d", action_count);
                break;
            default:
                if (sai_acl_table_field_attr_range(attribute_id)) {
                    if (sai_acl_table_udf_field_attr_range(attribute_id)) {
                        /* Validate the UDF Group Type, ACL does not
                         * supports Hash type UDF group */
                        if (!sai_is_obj_id_udf_group(attribute_value.oid)) {
                            SAI_ACL_LOG_ERR ("Invalid ACL Table UDF Group Id");
                            return sai_get_indexed_ret_val (SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                            attribute_count);
                        }
                        if ((rc = sai_acl_table_udf_field_validate(attribute_value.oid))
                            != SAI_STATUS_SUCCESS) {
                            return sai_get_indexed_ret_val (rc, attribute_count);
                        }
                        udf_field_count++;
                    }
                    /*ACL Table Fields*/
                    field_count++;
                } else {
                    return sai_get_indexed_ret_val (SAI_STATUS_UNKNOWN_ATTRIBUTE_0,
                                                    attribute_count);
                }
                break;

        }

        for (dup_index = attribute_count + 1; dup_index < attr_count;
             dup_index++) {
             if (attribute_id == attr_list[dup_index].id) {
                 SAI_ACL_LOG_ERR ("Duplicate attributes in indices %d and %d",
                                  attribute_count,dup_index);
                 return sai_get_indexed_ret_val (SAI_STATUS_INVALID_ATTRIBUTE_0,
                                                 dup_index);
             }
        }
    }

    /* Validate the mandatory attributes required for table creation*/
    if (attr_stage_present == false) {
        SAI_ACL_LOG_ERR ("Table creation failed as "
                         "stage is not specified");
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    if (attr_prio_present == false) {
        SAI_ACL_LOG_ERR ("Table creation failed as "
                         "table priority is not specified");
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    if (field_count == 0) {
        SAI_ACL_LOG_ERR ("Table creation failed as "
                         "fields are not specified");
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }


    *num_fields = field_count;
    *num_udf_fields = udf_field_count;
    *num_actions = action_count;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_table_udf_field_populate(sai_acl_table_t *acl_table,
                                             sai_attr_id_t attribute_id,
                                             sai_object_id_t udf_group_id,
                                             uint_t udf_field_idx)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_acl_udf_field_t *udf_field_list = NULL;
    sai_npu_object_id_t  udf_npu_id = 0;

    STD_ASSERT(acl_table != NULL);

    udf_field_list = acl_table->udf_field_list;

    STD_ASSERT(udf_field_list != NULL);

    udf_field_list[udf_field_idx].udf_attr_index = attribute_id;
    udf_field_list[udf_field_idx].udf_group_id = udf_group_id;

    if ((sai_rc = dn_sai_udf_group_hw_id_get(udf_group_id,
        &udf_npu_id)) != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Failed to get UDF hardware id for UDF Object Id: 0x%"PRIx64"",
                          udf_group_id);
        return sai_rc;
    }

    udf_field_list[udf_field_idx].udf_npu_id = udf_npu_id;

    return sai_rc;
}

static sai_status_t sai_acl_table_populate(sai_acl_table_t *acl_table,
                                           uint_t attr_count,
                                           const sai_attribute_t *attr_list,
                                           uint_t field_count,
                                           uint_t udf_field_count,
                                           uint_t action_count)
{
    uint_t field_count_idx = 0, attribute_count = 0;
    uint_t udf_field_count_idx = 0;
    sai_attr_id_t attribute_id = 0;
    sai_attribute_value_t attribute_value = {0};
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    uint_t action_count_idx = 0, count = 0;

    STD_ASSERT(acl_table != NULL);
    STD_ASSERT(attr_list != NULL);
    STD_ASSERT(attr_count > 0);
    STD_ASSERT(field_count > 0);

    SAI_ACL_LOG_TRACE ("Populating table attributes");

    do {
        /* Allocate memory for field list in ACL table structure*/
        acl_table->field_list = (sai_acl_table_attr_t *)
            calloc(field_count, sizeof(sai_acl_table_attr_t));
        if (acl_table->field_list == NULL) {
            SAI_ACL_LOG_ERR ("Allocation of Memory "
                    "failed for ACL Table Field List");
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        /* Allocate memory for field list in ACL table structure*/
        acl_table->action_list = (sai_acl_action_type_t *)
            calloc(action_count, sizeof(sai_acl_action_type_t));
        if (acl_table->action_list == NULL) {
            SAI_ACL_LOG_ERR ("Allocation of Memory "
                    "failed for ACL Table Action List");
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        if (udf_field_count != 0) {
            SAI_ACL_LOG_TRACE ("ACL Table has %d UDF Fields", udf_field_count);
            acl_table->udf_field_count = udf_field_count;
            acl_table->udf_field_list = (sai_acl_udf_field_t *)
                calloc(udf_field_count, sizeof(sai_acl_udf_field_t));
            if (acl_table->udf_field_list == NULL) {
                SAI_ACL_LOG_ERR ("Allocation of Memory "
                        "failed for ACL Table Field List");
                sai_rc = SAI_STATUS_NO_MEMORY;
                break;
            }
        }
    } while (0);

    if (sai_rc == SAI_STATUS_NO_MEMORY) {
        if (acl_table->field_list) {
            free (acl_table->field_list);
        }

        if (acl_table->action_list) {
            free (acl_table->action_list);
        }
        return sai_rc;
    }

    acl_table->field_count = field_count;

    for (attribute_count = 0; attribute_count < attr_count;
         attribute_count++) {
        attribute_id = attr_list[attribute_count].id;
        attribute_value = attr_list[attribute_count].value;

        switch (attribute_id) {
            case SAI_ACL_TABLE_ATTR_ACL_STAGE:
                acl_table->acl_stage = attribute_value.s32;
                break;
            case SAI_ACL_TABLE_ATTR_PRIORITY:
                acl_table->acl_table_priority = attribute_value.u32;
                break;
            case SAI_ACL_TABLE_ATTR_SIZE:
                acl_table->table_size = attribute_value.u32;
                break;
            case SAI_ACL_TABLE_ATTR_GROUP_ID:
                acl_table->table_group_id = attribute_value.oid;
                acl_table->virtual_group_create = true;
                break;
            case SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST:
                if (attribute_value.s32list.count > 0) {
                    for (count = 0; count <
                         attribute_value.s32list.count; count++) {
                        if (sai_acl_table_action_attr_range
                            (attribute_value.s32list.list[count])) {
                            acl_table->action_list[action_count_idx] =
                                  attribute_value.s32list.list[count];
                            action_count_idx++;
                        }
                    }
                    acl_table->action_count = action_count_idx;
                }
                break;
            default:
                if (sai_acl_table_field_attr_range(attribute_id)) {
                    if (sai_acl_table_udf_field_attr_range(attribute_id)) {

                        if (udf_field_count_idx >= udf_field_count) {
                            SAI_ACL_LOG_ERR ("UDF Field index exceeds UDF field count");
                            return sai_get_indexed_ret_val (SAI_STATUS_INVALID_ATTRIBUTE_0,
                                                            attribute_count);
                        }
                        if ((sai_rc = sai_acl_table_udf_field_populate(acl_table, attribute_id,
                            attribute_value.oid, udf_field_count_idx)) != SAI_STATUS_SUCCESS) {
                            SAI_ACL_LOG_ERR ("Failed to populate UDF field");
                            return sai_rc;
                        }

                        udf_field_count_idx++;
                    }
                    if (field_count_idx > field_count) {
                        SAI_ACL_LOG_ERR ("Field index exceeds field count");
                        return sai_get_indexed_ret_val (SAI_STATUS_INVALID_ATTRIBUTE_0,
                                                        attribute_count);
                    }
                    acl_table->field_list[field_count_idx] = attribute_id;
                    field_count_idx++;
                }
                break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_create_acl_table_id (
                        sai_acl_table_t *acl_table)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    acl_table->table_key.acl_table_id = sai_acl_table_id_alloc();

    if (acl_table->table_key.acl_table_id == SAI_ACL_INVALID_TABLE_ID) {
        return SAI_STATUS_TABLE_FULL;
    }

    if (!acl_table->virtual_group_create) {
        acl_table->table_group_id = sai_uoid_create (SAI_OBJECT_TYPE_ACL_TABLE_GROUP,
                                        sai_uoid_npu_obj_id_get(acl_table->table_key.acl_table_id));
    }

    if ((acl_table->table_size != 0) ||
        (acl_table->table_group_id != 0 && acl_table->virtual_group_create)) {
        /* Fixed size table or virtually grouped table needs to be created */
        if (acl_table->npu_table_info == NULL) {
            if ((sai_rc = sai_acl_npu_api_get()->create_acl_table(acl_table))
                != SAI_STATUS_SUCCESS) {
                SAI_ACL_LOG_ERR ("Fixed Size Table Creation failed");
                return sai_rc;
            }
        }
    }

    return sai_rc;
}

sai_status_t sai_create_acl_table(sai_object_id_t *acl_table_id,
                                  sai_object_id_t switch_id,
                                  uint32_t attr_count,
                                  const sai_attribute_t *attr_list)
{
    sai_status_t rc    = SAI_STATUS_FAILURE;
    sai_acl_table_t *acl_table = NULL;
    acl_node_pt acl_node = NULL;
    uint_t field_count = 0, udf_field_count = 0;
    uint_t action_count = 0;

    if (attr_count == 0) {
        SAI_ACL_LOG_ERR ("Attr count is zero");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(attr_list != NULL);
    STD_ASSERT(acl_table_id != NULL);

    rc = sai_acl_table_validate_attributes(attr_count, attr_list,
                                           &field_count, &udf_field_count,
                                           &action_count);

    if (rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("ACL Table attribute "
                   "validations failed");
        return rc;
    }

    acl_table = (sai_acl_table_t *)calloc(1, sizeof(sai_acl_table_t));

    if (acl_table == NULL) {
        SAI_ACL_LOG_ERR ("Allocation of Memory failed "
                         "for ACL Table");
        return SAI_STATUS_NO_MEMORY;
    }

    do {
        sai_acl_lock();
        acl_node = sai_acl_get_acl_node();

        sai_acl_table_init(acl_table);

        rc = sai_acl_table_populate(acl_table, attr_count, attr_list,
                                    field_count, udf_field_count,
                                    action_count);

        if (rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("ACL Table populate function"
                             "failed");
            break;
        }

        /* Qualifiers passed needs to be validated as to whether
         * they are supported in the stage (Pre-Ingress/Ingress/Egress) provided */
        if ((rc = sai_acl_npu_api_get()->validate_acl_table_field(acl_table))
            != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("ACL NPU Table validation failed");
            break;
        }

        if ((rc = sai_create_acl_table_id (acl_table)) != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("Failed to create ACL Table Id");
            break;
        }

        /* Insert the ACL table node in the RB Tree*/
        rc = sai_acl_table_insert(acl_node->sai_acl_table_tree, acl_table);

        if (rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("Failed Insertion of ACL "
                             "Table in ACL db with priority %lu and Table Id 0x%"PRIx64"",
                             acl_table->acl_table_priority,
                             acl_table->table_key.acl_table_id);

            break;
        }
        *acl_table_id = acl_table->table_key.acl_table_id;

    } while(0);

    if (rc != SAI_STATUS_SUCCESS) {

        sai_acl_table_id_dealloc(acl_table->table_key.acl_table_id);
        sai_acl_table_free(acl_table);
    } else {
        SAI_ACL_LOG_INFO ("ACL Table Id 0x%"PRIx64" successfully created ",
                          *acl_table_id);
    }

    sai_acl_unlock();
    return rc;
}

sai_status_t sai_delete_acl_table(sai_object_id_t table_id)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_acl_table_t *acl_table = NULL;
    acl_node_pt acl_node = NULL;
    bool table_removed = false;

    if (!sai_is_obj_id_acl_table(table_id)) {
        SAI_ACL_LOG_ERR ("Table Id 0x%"PRIx64" is not a ACL Table Object", table_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    do {
        sai_acl_lock();

        acl_node = sai_acl_get_acl_node();
        acl_table = sai_acl_table_find(acl_node->sai_acl_table_tree,
                                       table_id);
        if (acl_table == NULL) {
            SAI_ACL_LOG_ERR ("Table deletion failed: "
                             "ACL Table 0x%"PRIx64" not found", table_id);

            rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if (acl_table->rule_count > 0) {
            SAI_ACL_LOG_ERR ("Table deletion failed: "
                             "Rules %d exist in the ACL Table id 0x%"PRIx64"",
                             acl_table->rule_count, table_id);
            rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        if (acl_table->num_counters > 0) {
            SAI_ACL_LOG_ERR ("Table deletion failed: "
                             "%d counters associated with ACL Table id 0x%"PRIx64"",
                             acl_table->num_counters, table_id);
            rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        acl_table = sai_acl_table_remove(acl_node->sai_acl_table_tree,
                                         acl_table);
        if (acl_table == NULL) {
            /* Some internal error in RB tree, log an error */
            SAI_ACL_LOG_ERR ("Table deletion failed: "
                             "Failure removing ACL Table Id 0x%"PRIx64" from ACL db",
                             table_id);

            rc = SAI_STATUS_FAILURE;
            break;
        }
        table_removed = true;

        /* Table removal needs to be propogated to NPU for the removal of
         * any table equivalent entity in the hardware*/
        if ((rc = sai_acl_npu_api_get()->delete_acl_table(acl_table)) !=
             SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("Table deletion failed: "
                             "Failure deleting ACL Table Id 0x%"PRIx64" from hardware",
                             table_id);
            break;
        }

        sai_acl_table_id_dealloc(table_id);

        /* Free the allocated memory for acl_table */
        sai_acl_table_free(acl_table);

    } while(0);

    if (rc != SAI_STATUS_SUCCESS) {
        if (table_removed) {
            sai_acl_table_insert(acl_node->sai_acl_table_tree, acl_table);
        }
    } else {
        SAI_ACL_LOG_INFO ("ACL Table Id 0x%"PRIx64" successfully removed",
                          table_id);
    }

    sai_acl_unlock();
    return rc;
}

sai_object_id_t sai_acl_table_find_udf_group_id(sai_acl_table_t *acl_table,
                                                sai_attr_id_t attribute_id)
{
    uint_t udf_field_idx = 0;

    for (udf_field_idx = 0; udf_field_idx < acl_table->udf_field_count;
         udf_field_idx++) {
         if (acl_table->udf_field_list[udf_field_idx].udf_attr_index == attribute_id) {
             return acl_table->udf_field_list[udf_field_idx].udf_group_id;
         }
    }

    return SAI_NULL_OBJECT_ID;
}

sai_status_t sai_get_acl_table(sai_object_id_t table_id,
                               uint32_t attr_count,
                               sai_attribute_t *attr_list)
{
    uint_t attribute_count = 0;
    sai_attr_id_t attribute_id = 0;
    sai_acl_table_t *acl_table = NULL;
    acl_node_pt acl_node = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    if (attr_count == 0) {
        SAI_ACL_LOG_ERR ("Attr count is zero");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(attr_list != NULL);

    if (!sai_is_obj_id_acl_table(table_id)) {
        SAI_ACL_LOG_ERR ("Table Id 0x%"PRIx64" is not a ACL Table Object", table_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    do {
        sai_acl_lock();

        acl_node = sai_acl_get_acl_node();
        acl_table = sai_acl_table_find(acl_node->sai_acl_table_tree,
                                       table_id);
        if (acl_table == NULL) {
            SAI_ACL_LOG_ERR ("ACL Table not present "
                             "for table ID 0x%"PRIx64"", table_id);

            rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        for (attribute_count = 0; attribute_count < attr_count;
             attribute_count++) {
             attribute_id = attr_list[attribute_count].id;

             if (!sai_acl_table_valid_attr_range(attribute_id)) {
                SAI_ACL_LOG_ERR ("Attribute ID %d not in ACL "
                                 "Table Attribute range", attribute_id);
                rc = sai_get_indexed_ret_val (SAI_STATUS_INVALID_ATTRIBUTE_0,
                                              attribute_count);
                break;
             }
             switch (attribute_id) {
                 case SAI_ACL_TABLE_ATTR_ACL_STAGE:
                     attr_list[attribute_count].value.s32 = acl_table->acl_stage;
                     break;
                 case SAI_ACL_TABLE_ATTR_PRIORITY:
                     attr_list[attribute_count].value.u32 = acl_table->acl_table_priority;
                     break;
                 case SAI_ACL_TABLE_ATTR_SIZE:
                     attr_list[attribute_count].value.u32 = acl_table->table_size;
                     break;
                 case SAI_ACL_TABLE_ATTR_GROUP_ID:
                     attr_list[attribute_count].value.oid = acl_table->table_group_id;
                     break;
                 case SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST:
                     SAI_ACL_LOG_TRACE ("Nothing to get for ACL Table Actions");
                     break;
                 default:
                     if (sai_acl_table_field_attr_range(attribute_id)) {
                         if (sai_acl_table_udf_field_attr_range(attribute_id)) {
                             attr_list[attribute_count].value.oid =
                                    sai_acl_table_find_udf_group_id(acl_table,
                                                                    attribute_id);
                         } else {
                            SAI_ACL_LOG_TRACE ("Nothing to get for ACL Table Fields");
                         }
                     } else {
                         rc = sai_get_indexed_ret_val (SAI_STATUS_UNKNOWN_ATTRIBUTE_0,
                                                       attribute_count);
                         break;
                     }
             }
        }
    } while(0);

    sai_acl_unlock();

    if (rc == SAI_STATUS_SUCCESS) {
         SAI_ACL_LOG_INFO ("Get Attribute success for ACL Table Id 0x%"PRIx64"",
                           table_id);
    }

    return rc;
}

sai_status_t sai_set_acl_table(sai_object_id_t table_id,
                               const sai_attribute_t *attr)
{
    SAI_ACL_LOG_ERR ("Set Attribute not supported for ACL table");
    return SAI_STATUS_NOT_SUPPORTED;
}
