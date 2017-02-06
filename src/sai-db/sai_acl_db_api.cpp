/*
 * Copyright (c) 2016 Dell Inc.
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
 * @file sai_acl_db_api.cpp
 *
 * @brief This file contains the function definitions for updating the
 *        SQL DB tables related to the SAI ACL objects - table, entry and
 *        counter objects in VM environment.
 */

#include "sai_vm_db_utils.h"
#include "sai_vm_defs.h"
#include "sai_vm_event_log.h"
#include "sai_acl_db_api.h"
#include "sai_acl_db_utils.h"
#include "db_sql_ops.h"

#include "sai_acl_type_defs.h"
#include "sai_oid_utils.h"

#include "saiacl.h"
#include "saitypes.h"
#include "saistatus.h"

#include "std_error_codes.h"
#include "std_assert.h"
#include <string.h>
#include <inttypes.h>

#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdint.h>

static sai_status_t sai_acl_table_qualifier_list_db_populate (
uint_t table_id, uint_t qual_count, sai_acl_table_attr_t *qual_list)
{
    size_t      idx = 0;
    std::string insert_str;
    std::string qual_str;
    std::string table_id_str = std::to_string (table_id);

    STD_ASSERT (qual_list != NULL);

    for (idx = 0; idx < qual_count; idx++) {
        qual_str = sai_acl_table_field_attr_str_get (qual_list [idx]);

        insert_str = std::string ("( ") + table_id_str + ", " + qual_str +
            std::string (")");

        if (db_sql_insert (sai_vm_get_db_handle(),
                           "SAI_ACL_TABLE_QUALIFIER_LIST",
                           insert_str.c_str()) != STD_ERR_OK) {
            SAI_VM_DB_LOG_ERR ("Error inserting entry for table ID: %s, "
                               "qualifier: %s to SAI_ACL_TABLE_QUALIFIER_LIST.",
                               table_id_str.c_str(), qual_str.c_str());

            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_table_db_entry_update_usage_count_field (
sai_object_id_t acl_tbl_obj_id, std::string *p_field_str, bool is_add)
{
    uint_t      acl_table_id = 0;
    uint_t      num_counters_in_use = 0;
    char        num_counters_str [SAI_VM_MAX_BUFSZ];
    std::string value_str;

    STD_ASSERT (p_field_str);

    std::string attr_str = *p_field_str;

    acl_table_id = (uint_t) sai_uoid_npu_obj_id_get (acl_tbl_obj_id);

    std::string table_id_str = std::to_string (acl_table_id);
    std::string cond_str = std::string ("( table_id=") + table_id_str +
        std::string (")");

    /* Get the current number of counters in use for the table ID */
    if (db_sql_get_attribute (sai_vm_get_db_handle(), "SAI_ACL_TABLE",
                              attr_str.c_str(), cond_str.c_str(),
                              num_counters_str) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error getting attr: %s on DB entry for table ID: "
                           "%s.", attr_str.c_str(), table_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    /* Update the number of counters in use for the table ID */
    num_counters_in_use = std::atoi (num_counters_str);
    (is_add)? num_counters_in_use++ : num_counters_in_use--;
    value_str = std::to_string (num_counters_in_use);

    if (db_sql_set_attribute (sai_vm_get_db_handle(), "SAI_ACL_TABLE",
                              attr_str.c_str(), value_str.c_str(),
                              cond_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error setting attr: %s value: %s on DB entry for "
                           "table ID: %s.", attr_str.c_str(), value_str.c_str(),
                           table_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_table_db_entry_update_total_counters (
sai_object_id_t acl_tbl_obj_id, bool is_add)
{
    std::string field_str = "num_counters_in_use";

    return (sai_acl_table_db_entry_update_usage_count_field (acl_tbl_obj_id,
                                                             &field_str,
                                                             is_add));
}

static sai_status_t sai_acl_table_db_entry_update_total_rules (
sai_object_id_t acl_tbl_obj_id, bool is_add)
{
    std::string field_str = "num_entries_in_use";

    return (sai_acl_table_db_entry_update_usage_count_field (acl_tbl_obj_id,
                                                             &field_str,
                                                             is_add));
}

static sai_status_t sai_acl_rule_filter_list_add_db_entry (
uint_t rule_id, uint_t filter_count, sai_acl_filter_t *p_list)
{
    size_t      idx = 0;
    std::string insert_str;
    std::string filter_str;
    std::string admin_state_str;
    std::string match_data_str;
    std::string match_mask_str;

    STD_ASSERT (p_list != NULL);

    std::string rule_id_str = std::to_string (rule_id);

    for (idx = 0; idx < filter_count; idx++) {
        filter_str = sai_acl_rule_filter_attr_str_get (p_list [idx].field);

        admin_state_str = (p_list [idx].enable)? "1" : "0";

        sai_acl_rule_filter_match_info_str_get (&p_list [idx], &match_data_str,
                                                &match_mask_str);

        insert_str = std::string ("( ") + rule_id_str + ", " + filter_str +
            ", " + admin_state_str + ", " + match_data_str + ", " +
            match_mask_str + std::string (")");

        if (db_sql_insert (sai_vm_get_db_handle(), "SAI_ACL_ENTRY_FILTER_LIST",
                           insert_str.c_str()) != STD_ERR_OK) {
            SAI_VM_DB_LOG_ERR ("Error inserting Rule Filter List DB entry for "
                               "Rule ID: %s, filter: %s.", rule_id_str.c_str(),
                               filter_str.c_str());

            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_rule_filter_list_set_db_entry (
uint_t rule_id, sai_acl_filter_t *p_filter)
{
    size_t      idx = 0;
    std::string match_data_str;
    std::string match_mask_str;
    std::string attr_str [] = {"admin_state", "match_data", "match_mask"};
    uint_t num_attr = sizeof (attr_str) / sizeof (*attr_str);

    STD_ASSERT (p_filter != NULL);

    std::string rule_id_str = std::to_string (rule_id);

    std::string filter_str = sai_acl_rule_filter_attr_str_get (p_filter->field);

    std::string admin_state_str = (p_filter->enable)? "1" : "0";

    sai_acl_rule_filter_match_info_str_get (p_filter,
                                            &match_data_str, &match_mask_str);

    std::string value_str [] = {admin_state_str, match_data_str,
        match_mask_str};

    std::string cond_str = std::string ("( entry_id=") + rule_id_str +
        std::string (" AND filter=") + filter_str + std::string (")");

    for (idx = 0; idx < num_attr; idx++) {
        if (db_sql_set_attribute (sai_vm_get_db_handle(),
                                  "SAI_ACL_ENTRY_FILTER_LIST",
                                  attr_str [idx].c_str(),
                                  value_str [idx].c_str(), cond_str.c_str())
            != STD_ERR_OK) {
            SAI_VM_DB_LOG_ERR ("Error updating Rule Filter List DB entry for "
                               "Rule ID: %s, attribute: %s, value: %s.",
                               rule_id_str.c_str(), attr_str [idx].c_str(),
                               value_str [idx].c_str());

            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_rule_filter_list_db_update (
uint_t acl_rule_id, uint_t filter_count, sai_acl_filter_t *p_filter_list,
uint_t *p_new_filter_count)
{
    size_t            idx = 0;
    uint_t            new_filters = 0;
    sai_acl_filter_t *p_filter = NULL;

    STD_ASSERT (p_filter_list != NULL);
    STD_ASSERT (p_new_filter_count != NULL);

    std::string rule_id_str = std::to_string (acl_rule_id);

    for (idx = 0; idx < filter_count; idx++) {
        p_filter = &p_filter_list [idx];

        if (p_filter->field_change) {
            if ((sai_acl_rule_filter_list_set_db_entry (acl_rule_id, p_filter))
                != SAI_STATUS_SUCCESS) {
                SAI_VM_DB_LOG_ERR ("Error updating ACL Rule filter DB entry "
                                   "for Rule ID: %s, filter attribute ID: %d.",
                                   rule_id_str.c_str(), p_filter->field);

                return SAI_STATUS_FAILURE;
            }
        }

        if (p_filter->new_field) {
            if ((sai_acl_rule_filter_list_add_db_entry (acl_rule_id,
                                                        1, /* count */
                                                        p_filter)) !=
                SAI_STATUS_SUCCESS) {
                SAI_VM_DB_LOG_ERR ("Error adding ACL Rule filter DB entry for "
                                   "Rule ID: %s, filter attribute ID: %d.",
                                   rule_id_str.c_str(), p_filter->field);

                return SAI_STATUS_FAILURE;
            }

            new_filters++;
        }
    }

    *p_new_filter_count = new_filters;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_rule_action_list_add_db_entry (
uint_t rule_id, uint_t action_count, sai_acl_action_t *p_list)
{
    size_t      idx = 0;
    std::string insert_str;
    std::string action_str;
    std::string admin_state_str;
    std::string param_str;

    STD_ASSERT (p_list != NULL);

    std::string rule_id_str = std::to_string (rule_id);

    for (idx = 0; idx < action_count; idx++) {
        action_str = sai_acl_rule_action_attr_str_get (p_list [idx].action);

        admin_state_str = (p_list [idx].enable)? "1" : "0";

        param_str = sai_acl_rule_action_parameter_str_get (&p_list [idx]);

        insert_str = std::string ("( ") + rule_id_str + ", " + action_str +
            ", " + admin_state_str + ", " + param_str + std::string (")");

        if (db_sql_insert (sai_vm_get_db_handle(), "SAI_ACL_ENTRY_ACTION_LIST",
                           insert_str.c_str()) != STD_ERR_OK) {
            SAI_VM_DB_LOG_ERR ("Error inserting Rule Action List DB entry for "
                               "Rule ID: %s, Action: %s.", rule_id_str.c_str(),
                               action_str.c_str());

            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_rule_action_list_set_db_entry (
uint_t rule_id, sai_acl_action_t *p_action)
{
    size_t idx = 0;
    std::string attr_str [] = {"admin_state", "parameter"};
    uint_t num_attr = sizeof (attr_str) / sizeof (*attr_str);

    STD_ASSERT (p_action != NULL);

    std::string rule_id_str = std::to_string (rule_id);

    std::string action_str =
        sai_acl_rule_action_attr_str_get (p_action->action);

    std::string admin_state_str = (p_action->enable)? "1" : "0";

    std::string param_str = sai_acl_rule_action_parameter_str_get (p_action);

    std::string value_str [] = {admin_state_str, param_str};

    std::string cond_str = std::string ("( entry_id=") + rule_id_str +
        std::string (" AND action=") + action_str + std::string (")");

    for (idx = 0; idx < num_attr; idx++) {
        if (db_sql_set_attribute (sai_vm_get_db_handle(),
                                  "SAI_ACL_ENTRY_ACTION_LIST",
                                  attr_str [idx].c_str(),
                                  value_str [idx].c_str(), cond_str.c_str())
            != STD_ERR_OK) {
            SAI_VM_DB_LOG_ERR ("Error updating Rule Action List DB entry for "
                               "Rule ID: %s, attribute: %s, value: %s.",
                               rule_id_str.c_str(), attr_str [idx].c_str(),
                               value_str [idx].c_str());

            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_rule_action_list_db_update (
uint_t acl_rule_id, uint_t action_count, sai_acl_action_t *p_action_list,
uint_t *p_new_action_count)
{
    size_t            idx = 0;
    uint_t            new_actions = 0;
    sai_acl_action_t *p_action = NULL;

    STD_ASSERT (p_action_list != NULL);
    STD_ASSERT (p_new_action_count != NULL);

    std::string rule_id_str = std::to_string (acl_rule_id);

    for (idx = 0; idx < action_count; idx++) {
        p_action = &p_action_list [idx];

        if (p_action->action_change) {
            if ((sai_acl_rule_action_list_set_db_entry (acl_rule_id, p_action))
                != SAI_STATUS_SUCCESS) {
                SAI_VM_DB_LOG_ERR ("Error updating ACL Rule action DB entry "
                                   "for Rule ID: %s, action attribute ID: %d.",
                                   rule_id_str.c_str(), p_action->action);

                return SAI_STATUS_FAILURE;
            }
        }

        if (p_action->new_action) {
            if ((sai_acl_rule_action_list_add_db_entry (acl_rule_id,
                                                        1, /* count */
                                                        p_action)) !=
                SAI_STATUS_SUCCESS) {
                SAI_VM_DB_LOG_ERR ("Error adding ACL Rule action DB entry for "
                                   "Rule ID: %s, action attribute ID: %d.",
                                   rule_id_str.c_str(), p_action->action);

                return SAI_STATUS_FAILURE;
            }

            new_actions++;
        }
    }

    *p_new_action_count = new_actions;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_table_create_db_entry (sai_acl_table_t *p_acl_table)
{
    uint_t acl_table_id = 0;

    STD_ASSERT (p_acl_table != NULL);

    acl_table_id =
        (uint_t) sai_uoid_npu_obj_id_get (p_acl_table->table_key.acl_table_id);

    std::string table_id_str = std::to_string (acl_table_id);

    /* Check and return if ACL table entry is already existing */
    std::string cond_str =
        std::string ("( table_id=") + table_id_str + std::string (")");
    std::string attr_str = "table_id";
    char   out_str [SAI_VM_MAX_BUFSZ];

    if (db_sql_get_attribute (sai_vm_get_db_handle(), "SAI_ACL_TABLE",
                              attr_str.c_str(), cond_str.c_str(), out_str)
        == STD_ERR_OK) {
        if (table_id_str == std::string (out_str)) {
            SAI_VM_DB_LOG_ERR ("DB entry is already existing for table ID: %s.",
                               table_id_str.c_str());

            return SAI_STATUS_SUCCESS;
        }
    }

    std::string stage_str =
        sai_acl_table_stage_str_get (p_acl_table->acl_stage);
    std::string prio_str = std::to_string (p_acl_table->acl_table_priority);
    std::string table_size = std::to_string (p_acl_table->table_size);
    std::string table_group_id = std::to_string (p_acl_table->table_group_id);
    std::string num_qual_str = std::to_string (p_acl_table->field_count);
    std::string num_entries_str = std::to_string (p_acl_table->rule_count);
    std::string num_counters_str = std::to_string (p_acl_table->num_counters);
    std::string num_udf_quals_str = std::to_string (p_acl_table->udf_field_count);

    /* Insert ACL table entry to DB */
    std::string insert_str = std::string ("( ") + table_id_str + ", " +
        stage_str + ", " + prio_str + ", " + table_size + ", " + table_group_id +
        ", " + num_qual_str + ", " + num_entries_str + ", " + num_counters_str +
        ", " + num_udf_quals_str + std::string (")");

    if (db_sql_insert (sai_vm_get_db_handle(), "SAI_ACL_TABLE",
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting ACL Table entry with table ID: %s,"
                           " Obj ID: 0x%" PRIx64 ".", table_id_str.c_str(),
                           p_acl_table->table_key.acl_table_id);

        return SAI_STATUS_FAILURE;
    }

    if ((sai_acl_table_qualifier_list_db_populate (acl_table_id,
                                                   p_acl_table->field_count,
                                                   p_acl_table->field_list)) !=
        SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error adding qualifiers to "
                           "SAI_ACL_TABLE_QUALIFIER_LIST for table ID: %s, "
                           "Obj ID: 0x%" PRIx64 ".", table_id_str.c_str(),
                           p_acl_table->table_key.acl_table_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_table_delete_db_entry (sai_acl_table_t *p_acl_table)
{
    uint_t acl_table_id = 0;

    STD_ASSERT (p_acl_table != NULL);

    acl_table_id =
        (uint_t) sai_uoid_npu_obj_id_get (p_acl_table->table_key.acl_table_id);

    std::string table_id_str = std::to_string (acl_table_id);

    std::string delete_str =
        std::string ("( table_id=") + table_id_str + std::string (")");

    if (db_sql_delete (sai_vm_get_db_handle(), "SAI_ACL_TABLE",
                       delete_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error deleting ACL Table entry with table ID: %s, "
                           "Obj ID: 0x%" PRIx64 ".", table_id_str.c_str(),
                           p_acl_table->table_key.acl_table_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_rule_create_db_entry (sai_acl_rule_t *p_acl_rule)
{
    uint_t      acl_rule_id = 0;
    uint_t      acl_table_id = 0;
    uint_t      acl_cntr_id = 0;
    std::string cntr_id_str = "\"-\"";

    STD_ASSERT (p_acl_rule != NULL);

    acl_rule_id =
        (uint_t) sai_uoid_npu_obj_id_get (p_acl_rule->rule_key.acl_id);

    acl_table_id = (uint_t) sai_uoid_npu_obj_id_get (p_acl_rule->table_id);

    std::string rule_id_str = std::to_string (acl_rule_id);
    std::string table_id_str = std::to_string (acl_table_id);
    std::string prio_str = std::to_string (p_acl_rule->acl_rule_priority);
    std::string admin_state_str = (p_acl_rule->acl_rule_state)? "1" : "0";
    std::string num_filter_str = std::to_string (p_acl_rule->filter_count);
    std::string num_action_str = std::to_string (p_acl_rule->action_count);

    if (p_acl_rule->counter_id != 0) {
        acl_cntr_id = (uint_t) sai_uoid_npu_obj_id_get (p_acl_rule->counter_id);

        cntr_id_str = std::to_string (acl_cntr_id);
    }

    std::string insert_str = std::string ("( ") + rule_id_str + ", " +
        table_id_str + ", " +  prio_str + ", " + admin_state_str + ", " +
        num_filter_str + ", " + num_action_str + ", " + cntr_id_str +
        std::string (")");

    if (db_sql_insert (sai_vm_get_db_handle(), "SAI_ACL_ENTRY",
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting ACL RULE entry with Rule ID: %s, "
                           " Rule obj ID: 0x%" PRIx64 ".",
                           rule_id_str.c_str(), p_acl_rule->rule_key.acl_id);

        return SAI_STATUS_FAILURE;
    }

    if ((sai_acl_rule_filter_list_add_db_entry (acl_rule_id,
                                                p_acl_rule->filter_count,
                                                p_acl_rule->filter_list)) !=
        SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error adding ACL Rule filter DB entries for Table "
                           "ID: %s, Rule ID: %s.", table_id_str.c_str(),
                           rule_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    if ((sai_acl_rule_action_list_add_db_entry (acl_rule_id,
                                                p_acl_rule->action_count,
                                                p_acl_rule->action_list)) !=
        SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error adding ACL Rule action DB entries for Table "
                           "ID: %s, Rule ID: %s.", table_id_str.c_str(),
                           rule_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    /* Increment total rules in use for the ACL TABLE DB entry */
    if (sai_acl_table_db_entry_update_total_rules (p_acl_rule->table_id,
                                                   true /* add */)
        != SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error updating the total rules in use on "
                           "ACL TABLE for obj ID: 0x%" PRIx64 ".",
                           p_acl_rule->table_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_rule_delete_db_entry (sai_acl_rule_t *p_acl_rule)
{
    uint_t acl_rule_id = 0;

    STD_ASSERT (p_acl_rule != NULL);

    acl_rule_id =
        (uint_t) sai_uoid_npu_obj_id_get (p_acl_rule->rule_key.acl_id);

    std::string rule_id_str = std::to_string (acl_rule_id);

    std::string delete_str =
        std::string ("( entry_id=") + rule_id_str + std::string (")");

    if (db_sql_delete (sai_vm_get_db_handle(), "SAI_ACL_ENTRY",
                       delete_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error deleting ACL Rule entry with Rule ID: %s, "
                           " Rule obj ID: 0x%" PRIx64 ".",
                           rule_id_str.c_str(), p_acl_rule->rule_key.acl_id);

        return SAI_STATUS_FAILURE;
    }

    /* Decrement total rules in use for the ACL TABLE DB entry */
    if (sai_acl_table_db_entry_update_total_rules (p_acl_rule->table_id,
                                                   false /* remove */)
        != SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error updating the total rules in use on "
                           "ACL TABLE for obj ID: 0x%" PRIx64 ".",
                           p_acl_rule->table_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_rule_set_db_entry (sai_acl_rule_t *p_new_acl_rule,
                                        sai_acl_rule_t *p_existing_acl_rule)
{
    size_t      idx = 0;
    uint_t      acl_rule_id = 0;
    uint_t      cntr_id = 0;
    uint_t      new_filter_count = 0;
    uint_t      new_action_count = 0;
    std::string cntr_id_str = "\"-\"";

    STD_ASSERT (p_new_acl_rule != NULL);
    STD_ASSERT (p_existing_acl_rule != NULL);

    acl_rule_id =
        (uint_t) sai_uoid_npu_obj_id_get (p_new_acl_rule->rule_key.acl_id);

    std::string rule_id_str = std::to_string (acl_rule_id);

    if ((sai_acl_rule_filter_list_db_update (acl_rule_id,
                                             p_new_acl_rule->filter_count,
                                             p_new_acl_rule->filter_list,
                                             &new_filter_count))
        != SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error updating ACL Rule filter list DB entries for "
                           "Rule ID: %s.", rule_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    if ((sai_acl_rule_action_list_db_update (acl_rule_id,
                                             p_new_acl_rule->action_count,
                                             p_new_acl_rule->action_list,
                                             &new_action_count))
        != SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error updating ACL Rule action list DB entries for "
                           "Rule ID: %s.", rule_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    /* Update the DB attribute fields in SAI_ACL_ENTRY table. */
    std::string prio_str = std::to_string (p_new_acl_rule->acl_rule_priority);
    std::string admin_state_str = (p_new_acl_rule->acl_rule_state)? "1" : "0";
    std::string num_filter_str =
        std::to_string ((new_filter_count + p_existing_acl_rule->filter_count));
    std::string num_action_str =
        std::to_string (new_action_count + p_existing_acl_rule->action_count);

    if (p_new_acl_rule->counter_id != 0) {
        cntr_id = (uint_t) sai_uoid_npu_obj_id_get (p_new_acl_rule->counter_id);
        cntr_id_str = std::to_string (cntr_id);
    }

    std::string attr_str [] = {
        "priority", "admin_state", "filter_count", "action_count",
        "counter_id"};
    std::string value_str [] = {
        prio_str, admin_state_str, num_filter_str, num_action_str, cntr_id_str};
    uint_t num_attr = sizeof (attr_str) / sizeof (*attr_str);
    std::string cond_str =
        std::string ("( entry_id=") + rule_id_str + std::string (")");

    for (idx = 0; idx < num_attr; idx++) {
        if (db_sql_set_attribute (sai_vm_get_db_handle(), "SAI_ACL_ENTRY",
                                  attr_str [idx].c_str(),
                                  value_str [idx].c_str(), cond_str.c_str())
            != STD_ERR_OK) {
            SAI_VM_DB_LOG_ERR ("Error seting ACL Rule entry with Rule ID: %s, "
                               " Rule obj ID: 0x%" PRIx64 ", attr: %s, "
                               "value: %s.", rule_id_str.c_str(),
                               p_new_acl_rule->rule_key.acl_id,
                               attr_str [idx].c_str(), value_str [idx].c_str());

            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_rule_db_entry_update_counter_id (sai_object_id_t rule_id,
                                                      sai_object_id_t cntr_id,
                                                      bool attach_cntr)
{
    uint_t      npu_rule_id = 0;
    uint_t      npu_cntr_id = 0;
    std::string value_str = "\"-\"";

    npu_rule_id = (uint_t) sai_uoid_npu_obj_id_get (rule_id);
    std::string rule_id_str = std::to_string (npu_rule_id);
    std::string attr_str = "counter_id";

    if (attach_cntr) {
        npu_cntr_id = (uint_t) sai_uoid_npu_obj_id_get (cntr_id);
        value_str = std::to_string (npu_cntr_id);
    }

    std::string cond_str =
        std::string ("( entry_id=") + rule_id_str + std::string (")");

    if (db_sql_set_attribute (sai_vm_get_db_handle(), "SAI_ACL_ENTRY",
                              attr_str.c_str(), value_str.c_str(),
                              cond_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error seting ACL Rule entry with Rule ID: %s, "
                           " Rule obj ID: 0x%" PRIx64 ", attr: %s, "
                           "value: %s.", rule_id_str.c_str(), rule_id,
                           attr_str.c_str(), value_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_counter_create_db_entry (sai_acl_counter_t *p_acl_cntr)
{
    uint_t acl_cntr_id = 0;
    uint_t acl_table_id = 0;

    STD_ASSERT (p_acl_cntr != NULL);

    acl_cntr_id =
        (uint_t) sai_uoid_npu_obj_id_get (p_acl_cntr->counter_key.counter_id);

    acl_table_id = (uint_t) sai_uoid_npu_obj_id_get (p_acl_cntr->table_id);

    std::string cntr_id_str = std::to_string (acl_cntr_id);
    std::string table_id_str = std::to_string (acl_table_id);
    std::string type_str = sai_acl_cntr_type_str_get (p_acl_cntr->counter_type);
    std::string num_ref_str = std::to_string (p_acl_cntr->shared_count);
    std::string byte_count_str = "0";
    std::string pkt_count_str = "0";

    std::string insert_str = std::string ("( ") + cntr_id_str + ", " +
        table_id_str + ", " + type_str + ", " + num_ref_str + ", " +
        byte_count_str + ", " + pkt_count_str + std::string (")");

    if (db_sql_insert (sai_vm_get_db_handle(), "SAI_ACL_COUNTER",
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting ACL Counter entry with Counter ID: "
                           "%s, Obj ID: 0x%" PRIx64 ".", cntr_id_str.c_str(),
                           p_acl_cntr->counter_key.counter_id);

        return SAI_STATUS_FAILURE;
    }

    /* Increment total counters in use for the ACL TABLE DB entry */
    if (sai_acl_table_db_entry_update_total_counters (p_acl_cntr->table_id,
                                                      true /* add */)
        != SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error updating the total counters in use on "
                           "ACL TABLE for obj ID: 0x%" PRIx64 ".",
                           p_acl_cntr->table_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_counter_delete_db_entry (sai_acl_counter_t *p_acl_cntr)
{
    uint_t acl_cntr_id = 0;

    STD_ASSERT (p_acl_cntr != NULL);

    acl_cntr_id =
        (uint_t) sai_uoid_npu_obj_id_get (p_acl_cntr->counter_key.counter_id);

    std::string cntr_id_str = std::to_string (acl_cntr_id);

    std::string delete_str =
        std::string ("( counter_id=") + cntr_id_str + std::string (")");

    if (db_sql_delete (sai_vm_get_db_handle(), "SAI_ACL_COUNTER",
                       delete_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error deleting ACL Counter entry with Counter ID: "
                           "%s, Obj ID: 0x%" PRIx64 ".", cntr_id_str.c_str(),
                           p_acl_cntr->counter_key.counter_id);

        return SAI_STATUS_FAILURE;
    }

    /* Decrement total counters in use for the ACL TABLE DB entry */
    if (sai_acl_table_db_entry_update_total_counters (p_acl_cntr->table_id,
                                                      false /* remove */)
        != SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error updating the total counters in use on "
                           "ACL TABLE for obj ID: 0x%" PRIx64 ".",
                           p_acl_cntr->table_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_counter_db_entry_set_cntrs (sai_acl_counter_t *p_acl_cntr,
                                                 uint_t count_value)
{
    uint_t      acl_cntr_id = 0;
    uint_t      num_counters = 1;
    size_t      idx = 0;
    std::string attr_str;
    std::string value_str;
    std::string cntr_type_str [] = {"byte_count", "packet_count"};

    STD_ASSERT (p_acl_cntr != NULL);

    acl_cntr_id =
        (uint_t) sai_uoid_npu_obj_id_get (p_acl_cntr->counter_key.counter_id);

    std::string cntr_id_str = std::to_string (acl_cntr_id);

    std::string cond_str =
        std::string ("( counter_id=") + cntr_id_str + std::string (")");

    if (p_acl_cntr->counter_type == SAI_ACL_COUNTER_BYTES_PACKETS) {
        num_counters = sizeof (cntr_type_str) / sizeof (*cntr_type_str);
    }

    for (idx = 0; idx < num_counters; idx++) {
        if (p_acl_cntr->counter_type == SAI_ACL_COUNTER_BYTES_PACKETS) {
            attr_str = cntr_type_str [idx];
        } else if (p_acl_cntr->counter_type == SAI_ACL_COUNTER_BYTES) {
            attr_str = "byte_count";
        } else if (p_acl_cntr->counter_type == SAI_ACL_COUNTER_PACKETS) {
            attr_str = "packet_count";
        }else {
            SAI_VM_DB_LOG_ERR ("Counter type %d is not valid.",
                               p_acl_cntr->counter_type);

            continue;
        }

        value_str = std::to_string (count_value);

        if (db_sql_set_attribute (sai_vm_get_db_handle(), "SAI_ACL_COUNTER",
                                  attr_str.c_str(), value_str.c_str(),
                                  cond_str.c_str()) != STD_ERR_OK) {
            SAI_VM_DB_LOG_ERR ("Error setting ACL Counter entry for counter ID:"
                               " %s, attr: %s, value: %s.", cntr_id_str.c_str(),
                               attr_str.c_str(), value_str.c_str());

            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_counter_db_entry_set_ref_count (
sai_acl_counter_t *p_acl_cntr)
{
    uint_t acl_cntr_id = 0;

    STD_ASSERT (p_acl_cntr != NULL);

    acl_cntr_id =
        (uint_t) sai_uoid_npu_obj_id_get (p_acl_cntr->counter_key.counter_id);

    std::string cntr_id_str = std::to_string (acl_cntr_id);

    std::string cond_str =
        std::string ("( counter_id=") + cntr_id_str + std::string (")");
    std::string attr_str = "reference_count";
    std::string value_str = std::to_string (p_acl_cntr->shared_count);

    if (db_sql_set_attribute (sai_vm_get_db_handle(), "SAI_ACL_COUNTER",
                              attr_str.c_str(), value_str.c_str(),
                              cond_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error setting ACL Counter entry for counter ID:"
                           " %s, attr: %s, value: %s.", cntr_id_str.c_str(),
                           attr_str.c_str(), value_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}
