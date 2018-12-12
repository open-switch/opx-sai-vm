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
 * * @file sai_acl_rule.c
 * *
 * * @brief This file contains functions for SAI ACL rule operations.
 * *
 * *************************************************************************/
#include "sai_acl_type_defs.h"
#include "sai_acl_npu_api.h"
#include "sai_common_acl.h"
#include "sai_acl_rule_utils.h"
#include "sai_acl_utils.h"
#include "sai_port_utils.h"
#include "sai_qos_util.h"

#include "saitypes.h"
#include "saiacl.h"
#include "saistatus.h"
#include "sai_common_infra.h"

#include "std_type_defs.h"
#include "std_assert.h"
#include "std_rbtree.h"
#include "std_llist.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static bool sai_is_acl_rule_index_used(uint_t acl_rule_index)
{
    sai_object_id_t acl_id = 0;
    acl_node_pt acl_node = NULL;
    sai_acl_rule_t *acl_rule_present = NULL;

    acl_id = sai_uoid_create (SAI_OBJECT_TYPE_ACL_ENTRY,
                              (sai_npu_object_id_t)acl_rule_index);

    acl_node = sai_acl_get_acl_node();

    acl_rule_present = sai_acl_rule_find(acl_node->sai_acl_rule_tree,
                                         acl_id);
    if (acl_rule_present == NULL) {
        return false;
    }
    return true;
}
/**
 * This function allocates the next free index available for rule id creation.
 * Free function is not used intentionally since the acl rule tree is walked to
 * find out the free index available.
 */
static sai_status_t sai_allocate_acl_rule_index(uint_t *alloc_index)
{

    while(sai_is_acl_rule_index_used(*alloc_index)) {
       (*alloc_index) ++;

       /**
        * Wrap around has occured. No more free indices are present.
        *  */
       if(*alloc_index == 0)
       {
           SAI_ACL_LOG_ERR("All entries are exhausted");
           return SAI_STATUS_FAILURE;
       }
    }

    SAI_ACL_LOG_TRACE("Rule index allocated is %d",*alloc_index);
    return SAI_STATUS_SUCCESS;
}

static void sai_acl_rule_free(sai_acl_rule_t *acl_rule)
{
    uint_t filter_count = 0, action_count = 0;

    STD_ASSERT(acl_rule != NULL);

    if (acl_rule->filter_list) {
        for (filter_count = 0; filter_count < acl_rule->filter_count;
             filter_count++) {
             if (sai_acl_object_list_field_attr(acl_rule->filter_list[filter_count].field) &&
                  acl_rule->filter_list[filter_count].match_data.obj_list.list) {
                  free(acl_rule->filter_list[filter_count].match_data.obj_list.list);
             } else if (sai_acl_rule_udf_field_attr_range(acl_rule->filter_list[filter_count].field)) {
                  if (acl_rule->filter_list[filter_count].match_data.u8_list.list) {
                      free(acl_rule->filter_list[filter_count].match_data.u8_list.list);
                  }
                  if (acl_rule->filter_list[filter_count].match_mask.u8_list.list) {
                      free(acl_rule->filter_list[filter_count].match_mask.u8_list.list);
                  }
             }
        }
        free(acl_rule->filter_list);
        acl_rule->filter_list = NULL;
    }
    if (acl_rule->action_list) {
        for (action_count = 0; action_count < acl_rule->action_count;
             action_count++) {
             if (sai_acl_object_list_action_attr(acl_rule->action_list[action_count].action) &&
                  acl_rule->action_list[action_count].parameter.obj_list.list) {
                  free(acl_rule->action_list[action_count].parameter.obj_list.list);
             }
        }
        free(acl_rule->action_list);
        acl_rule->action_list = NULL;
    }
    sai_acl_npu_api_get()->free_acl_rule(acl_rule);
    free (acl_rule);
}

static sai_status_t sai_acl_get_attr_from_rule (sai_acl_table_t *acl_table,
                                         sai_acl_rule_t *acl_rule,
                                         uint_t attr_count,
                                         sai_attribute_t *attr_list)
{
    uint_t attribute_count = 0;
    sai_attr_id_t attribute_id = 0;
    sai_attribute_value_t *attr_value = NULL;
    uint_t filter_count = 0, action_count = 0;
    bool field_found = false, action_found = false;
    sai_acl_filter_t *acl_filter = NULL;
    sai_acl_action_t *acl_action = NULL;
    sai_status_t rc = SAI_STATUS_FAILURE;

    if (attr_count == 0) {
        SAI_ACL_LOG_ERR ("Parameter attr_count is 0");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(acl_rule != NULL);
    STD_ASSERT(attr_list != NULL);

    for (attribute_count = 0; attribute_count < attr_count; attribute_count++) {

        attribute_id = attr_list[attribute_count].id;
        attr_value = &attr_list[attribute_count].value;

        if (!sai_acl_rule_valid_attr_range(attribute_id)) {
            SAI_ACL_LOG_ERR ("Attribute ID %d not in ACL Rule "
                             "attribute range", attribute_id);
            return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                           attribute_count);
        }

        if (attribute_id == SAI_ACL_ENTRY_ATTR_TABLE_ID) {
            attr_list[attribute_count].value.oid = acl_rule->table_id;
        } else if (attribute_id == SAI_ACL_ENTRY_ATTR_PRIORITY) {
            attr_list[attribute_count].value.u32 = acl_rule->acl_rule_priority;
        } else if (attribute_id == SAI_ACL_ENTRY_ATTR_ADMIN_STATE) {
            attr_list[attribute_count].value.booldata =
                                  acl_rule->acl_rule_state ? true:false;
        } else if (sai_acl_rule_field_attr_range(attribute_id)) {

            if ((attribute_id == SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT) &&
                (acl_table->acl_stage == SAI_ACL_STAGE_INGRESS)) {
                attribute_id = SAI_ACL_ENTRY_ATTR_FIELD_DST_PORT;
            }

            for (filter_count = 0, acl_filter = &acl_rule->filter_list[0];
                 filter_count < acl_rule->filter_count;
                 ++filter_count, ++acl_filter) {

                 if (acl_filter->field == attribute_id) {
                     if ((rc = sai_acl_rule_filter_populate_attr_type(
                         attribute_id, acl_filter, attr_value, true)) !=
                         SAI_STATUS_SUCCESS) {
                         if (rc == SAI_STATUS_INVALID_ATTR_VALUE_0) {
                             rc = sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                          attribute_count);
                         }
                         return rc;
                     }
                     field_found = true;
                     attr_value->aclfield.enable = acl_filter->enable;
                 }

            }

            if (field_found == false) {
                SAI_ACL_LOG_ERR ("Field Attr not found %d", attribute_id);
                return SAI_STATUS_ITEM_NOT_FOUND;
            }

        } else if (sai_acl_rule_action_attr_range(attribute_id)) {

            for (action_count = 0, acl_action = &acl_rule->action_list[0];
                 action_count < acl_rule->action_count;
                 ++action_count, ++acl_action) {

                 if (acl_action->action == attribute_id) {
                     if ((rc = sai_acl_rule_action_populate_attr_type(
                         attribute_id, acl_action, attr_value, true)) !=
                         SAI_STATUS_SUCCESS) {
                         if (rc == SAI_STATUS_INVALID_ATTR_VALUE_0) {
                             rc = sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                          attribute_count);
                         } else if (rc == SAI_STATUS_INVALID_ATTRIBUTE_0) {
                             rc = sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                                          attribute_count);
                         }
                         return rc;
                     }
                     action_found = true;
                     attr_value->aclaction.enable = acl_action->enable;
                 }
            }

            if (attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_COUNTER) {
                attr_list[attribute_count].value.aclaction.parameter.oid  =
                                            acl_rule->counter_id;
            }
            if (attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER) {
                attr_list[attribute_count].value.aclaction.parameter.oid  =
                                            acl_rule->policer_id;
            }

            if (action_found == false) {
                SAI_ACL_LOG_ERR ("Action Attr %d not found ", attribute_id);
                return SAI_STATUS_ITEM_NOT_FOUND;
            }

        } else {
            SAI_ACL_LOG_ERR ("Unknown Attribute ID %d",
                             attr_list[attribute_count].id);
            return sai_get_indexed_ret_val(SAI_STATUS_UNKNOWN_ATTRIBUTE_0,
                                           attribute_count);
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_rule_populate(sai_acl_table_t *acl_table,
                                   sai_acl_rule_t *acl_rule,
                                   uint_t attr_count,
                                   const sai_attribute_t *attr_list,
                                   uint_t filter_count,
                                   uint_t action_count)
{
    bool rule_admin_state = false;
    sai_attr_id_t attribute_id = 0;
    uint_t num_filter = 0, num_action = 0, attribute_count = 0;
    uint_t policer_type = 0;
    sai_attribute_value_t attr_value;
    sai_status_t rc = SAI_STATUS_FAILURE;

    STD_ASSERT(acl_table != NULL);
    STD_ASSERT(acl_rule != NULL);
    STD_ASSERT(attr_list != NULL);

    memset(&attr_value, 0, sizeof(sai_attribute_value_t));

    acl_rule->filter_list = (sai_acl_filter_t *)calloc(filter_count, sizeof(sai_acl_filter_t));

    if (acl_rule->filter_list == NULL) {
        SAI_ACL_LOG_ERR ("Allocation of Memory failed "
                         "for ACL Rule Filter List");
        return SAI_STATUS_NO_MEMORY;
    }

    acl_rule->action_list = (sai_acl_action_t *)calloc(action_count, sizeof(sai_acl_action_t));

    if (acl_rule->action_list == NULL) {
        SAI_ACL_LOG_ERR ("Allocation of Memory failed for "
                         "ACL Rule Action List");
        return SAI_STATUS_NO_MEMORY;
    }

    for (attribute_count = 0; attribute_count < attr_count; attribute_count++) {
        attribute_id = attr_list[attribute_count].id;
        attr_value = attr_list[attribute_count].value;
        if (sai_acl_rule_field_attr_range(attribute_id)) {

            if ((attribute_id == SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT) &&
                (acl_table->acl_stage == SAI_ACL_STAGE_INGRESS)) {
                attribute_id = SAI_ACL_ENTRY_ATTR_FIELD_DST_PORT;
            }

            acl_rule->filter_list[num_filter].field = attribute_id;
            acl_rule->filter_list[num_filter].enable =
                                        attr_value.aclfield.enable;

            if ((rc = sai_acl_rule_filter_populate_attr_type(attribute_id,
                      &acl_rule->filter_list[num_filter], &attr_value, false))
                      != SAI_STATUS_SUCCESS) {
                if (rc == SAI_STATUS_INVALID_ATTR_VALUE_0) {
                    rc = sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                 attribute_count);
                }
                return rc;
            }

            if (sai_acl_rule_udf_field_attr_range(attribute_id)) {
                /* Need to validate and populate UDF specific data
                 * from ACL Table to ACL Rule */
                if ((rc = sai_acl_rule_udf_filter_populate(acl_table, attribute_id,
                    &acl_rule->filter_list[num_filter])) != SAI_STATUS_SUCCESS) {
                    return sai_get_indexed_ret_val(rc, attribute_count);
                }
            }
            num_filter++;
        } else if (sai_acl_rule_action_attr_range(attribute_id)) {

            acl_rule->action_list[num_action].action = attribute_id;
            acl_rule->action_list[num_action].enable =
                                        attr_value.aclaction.enable;

            if ((rc = sai_acl_rule_action_populate_attr_type(attribute_id,
                      &acl_rule->action_list[num_action], &attr_value, false))
                      != SAI_STATUS_SUCCESS) {
                if (rc == SAI_STATUS_INVALID_ATTR_VALUE_0) {
                    rc = sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                 attribute_count);
                } else if (rc == SAI_STATUS_INVALID_ATTRIBUTE_0) {
                    rc = sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                                 attribute_count);
                }
                return rc;
            }

            if (attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_COUNTER) {
                if ((attr_value.aclaction.parameter.oid == SAI_NULL_OBJECT_ID) &&
                    acl_rule->action_list[num_action].enable) {
                    return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                   attribute_count);
                }
                if (attr_value.aclaction.enable) {
                    acl_rule->counter_id = attr_value.aclaction.parameter.oid;
                }
            }

            if ((attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE) ||
                    (attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE)) {
                /* Verify the Sample Object Id */
                if ((attr_value.aclaction.parameter.oid == SAI_NULL_OBJECT_ID) &&
                        acl_rule->action_list[num_action].enable) {
                    return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                   attribute_count);
                }
                if (attr_value.aclaction.enable) {
                    acl_rule->samplepacket_id[sai_acl_get_sample_direction(attribute_id)] =
                                                                attr_value.aclaction.parameter.oid;
                }
            }

            if (attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER) {
                if(attr_value.aclaction.enable &&
                   attr_value.aclaction.parameter.oid == SAI_NULL_OBJECT_ID) {
                   return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                  attribute_count);
                }

                if(attr_value.aclaction.enable){
                    policer_type = sai_get_type_from_npu_object(attr_value.aclaction.parameter.oid);
                    if((policer_type != SAI_POLICER_MODE_SR_TCM) &&
                       (policer_type != SAI_POLICER_MODE_TR_TCM)){
                       return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                      attribute_count);
                    }
                    acl_rule->policer_id = attr_value.aclaction.parameter.oid;
                }
            }

            num_action++;

        } else if (attribute_id == SAI_ACL_ENTRY_ATTR_TABLE_ID) {
            acl_rule->table_id = attr_value.oid;
        } else if (attribute_id == SAI_ACL_ENTRY_ATTR_PRIORITY) {
            acl_rule->acl_rule_priority = attr_value.u32;
        } else if (attribute_id == SAI_ACL_ENTRY_ATTR_ADMIN_STATE) {
            rule_admin_state = true;
            acl_rule->acl_rule_state = attr_value.booldata;
        } else {
            SAI_ACL_LOG_TRACE ("Unknown Attribute ID %d", attribute_id);
            return sai_get_indexed_ret_val(SAI_STATUS_UNKNOWN_ATTRIBUTE_0,
                                           attribute_count);
        }
    }

    /* Set the defaults */
    if (rule_admin_state != true) {
        acl_rule->acl_rule_state = SAI_ACL_RULE_DEFAULT_ADMIN_STATE;
    }

    acl_rule->filter_count = filter_count;
    acl_rule->action_count = action_count;
    acl_rule->npu_rule_info = NULL;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_check_rule_attributes(uint_t attr_count,
                                           const sai_attribute_t *attr_list,
                                           uint_t *field_count,
                                           uint_t *action_count,
                                           sai_object_id_t *acl_table_id)
{
    uint_t attribute_count = 0, dup_index = 0;
    uint_t num_fields = 0, num_actions = 0;
    sai_attr_id_t attribute_id = 0;

    STD_ASSERT(attr_count > 0);
    STD_ASSERT(attr_list != NULL);
    STD_ASSERT(field_count != NULL);
    STD_ASSERT(action_count != NULL);
    STD_ASSERT(acl_table_id != NULL);

    for (attribute_count = 0; attribute_count < attr_count;
         attribute_count++) {
        attribute_id = attr_list[attribute_count].id;

        if (!sai_acl_rule_valid_attr_range(attribute_id)) {
            SAI_ACL_LOG_ERR ("Attribute ID %d not in "
                             "ACL Rule attribute range",attribute_id);

            return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                           attribute_count);
        } else if (sai_acl_rule_field_attr_range(attribute_id)) {
            SAI_ACL_LOG_TRACE ("Attribute ID %d belongs "
                               "to ACL rule field",attribute_id);
            num_fields++;
        } else if (sai_acl_rule_action_attr_range(attribute_id)) {
            SAI_ACL_LOG_TRACE ("Attribute ID %d belongs "
                               "to ACL action",attribute_id);
            num_actions++;
        }

        for (dup_index = attribute_count + 1; dup_index < attr_count;
             dup_index++) {
            if (attribute_id == attr_list[dup_index].id) {
                SAI_ACL_LOG_ERR ("Duplicate Attribute ID in "
                                 "indices %d and %d",
                                 attribute_count, dup_index);
                return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                               dup_index);
            }
        }

        if (attribute_id == SAI_ACL_ENTRY_ATTR_TABLE_ID) {
            *acl_table_id = attr_list[attribute_count].value.oid;
        }
    }

    *field_count = num_fields;
    *action_count = num_actions;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_install_acl_rule(sai_acl_table_t *acl_table,
                                         sai_acl_rule_t *acl_rule)
{
    sai_status_t rc = SAI_STATUS_FAILURE;

    STD_ASSERT(acl_table != NULL);
    STD_ASSERT(acl_rule != NULL);

    /*Create table in NPU if this is the first rule*/
    if (acl_table->npu_table_info == NULL) {
        /* Table needs to be created in hardware */
        rc = sai_acl_npu_api_get()->create_acl_table(acl_table);
        if (rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("Table Creation failed "
                       "for ACL Rule with priority %d in Table Id 0x%"PRIx64"",
                       acl_rule->acl_rule_priority, acl_rule->table_id);
            return rc;
        }
    }
    /* Table programmed in hardware, now create the rule in
     * hardware. */
    rc = sai_acl_npu_api_get()->create_acl_rule(acl_table, acl_rule);

    if (rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("ACL Rule Creation failed "
                   "in hardware with priority %d in Table Id 0x%"PRIx64"",
                   acl_rule->acl_rule_priority, acl_rule->table_id);
        return rc;
    }
    return rc;
}

sai_status_t sai_create_acl_rule(sai_object_id_t *acl_rule_id,
                                 sai_object_id_t switch_id,
                                 uint32_t attr_count,
                                 const sai_attribute_t *attr_list)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_acl_table_t *acl_table = NULL;
    sai_acl_rule_t *acl_rule = NULL;
    acl_node_pt acl_node = NULL;
    uint_t field_count = 0, action_count = 0;
    bool rule_installed = false, samplepacket_installed = false;
    sai_object_id_t acl_table_id = 0;
    uint_t acl_rule_index = 0;

    if (attr_count == 0) {
        SAI_ACL_LOG_ERR ("Parameter attr_count is 0");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(acl_rule_id != NULL);
    STD_ASSERT(attr_list != NULL);

    rc = sai_acl_check_rule_attributes(attr_count, attr_list,
                                       &field_count,
                                       &action_count,
                                       &acl_table_id);
    if (rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Validation of ACL rule "
                   "attributes failed");
        return rc;
    }

    if (field_count == 0) {
        SAI_ACL_LOG_ERR ("Field Count is 0");
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    acl_node = sai_acl_get_acl_node();
    acl_table = sai_acl_table_find(acl_node->sai_acl_table_tree,
                                   acl_table_id);
    if (acl_table == NULL) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    acl_rule = (sai_acl_rule_t *)calloc(1,sizeof(sai_acl_rule_t));
    if (acl_rule == NULL) {
        SAI_ACL_LOG_ERR ("Allocation of Memory failed for "
                         "ACL Rule");
        return SAI_STATUS_NO_MEMORY;
    }

    sai_acl_lock();
    do {
        rc = sai_acl_rule_populate(acl_table, acl_rule, attr_count, attr_list,
                                   field_count, action_count);
        if (rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("ACL Rule populate failed");
            break;
        }

        if ((rc = sai_acl_validate_create_rule(acl_table, acl_rule)) != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("ACL Rule validate failed");
            break;
        }

        rc = sai_allocate_acl_rule_index(&acl_rule_index);
        if (rc != SAI_STATUS_SUCCESS) {
            break;
        }

        acl_rule->rule_key.acl_id = sai_uoid_create (SAI_OBJECT_TYPE_ACL_ENTRY,
                                                     (sai_npu_object_id_t)acl_rule_index);

        rc = sai_install_acl_rule(acl_table, acl_rule);
        if (rc != SAI_STATUS_SUCCESS) {
            break;
        }
        rule_installed = true;

        /* Check whether samplepacket needs to be created */
        if ((acl_rule->samplepacket_id[SAI_SAMPLEPACKET_DIR_INGRESS] != 0) ||
           (acl_rule->samplepacket_id[SAI_SAMPLEPACKET_DIR_EGRESS]) != 0) {
            if ((rc = sai_acl_rule_create_samplepacket(acl_rule)) !=
                      SAI_STATUS_SUCCESS) {
                SAI_ACL_LOG_ERR ("SamplePacket config is not applied");
                break;
            }
            samplepacket_installed = true;
        }

        /* Check whether counter needs to be attached to the ACL Rule */
        if (acl_rule->counter_id != 0) {
            if ((rc = sai_attach_cntr_to_acl_rule(acl_rule)) !=
                        SAI_STATUS_SUCCESS) {
                SAI_ACL_LOG_ERR (" ACL Counter 0x%"PRIx64" failed to attach with ACL"
                                 "Rule Id 0x%"PRIx64" in Table Id 0x%"PRIx64"",
                                 acl_rule->counter_id, acl_rule->rule_key.acl_id,
                                 acl_rule->table_id);
                break;
            }
        }

        if(acl_rule->policer_id != 0) {
            SAI_ACL_LOG_TRACE("Policer id is %"PRIx64"",acl_rule->policer_id);
            if ((rc = sai_attach_policer_to_acl_rule(acl_rule)) != SAI_STATUS_SUCCESS){
                SAI_ACL_LOG_ERR (" ACL Policer 0x%"PRIx64" failed to attach with ACL"
                                 "Rule Id 0x%"PRIx64" in Table Id 0x%"PRIx64"",
                                 acl_rule->policer_id,
                                 acl_rule->rule_key.acl_id,
                                 acl_rule->table_id);
                break;
            }
        }
        /* Insert the ACL rule node in the RB Tree. */
        rc = sai_acl_rule_insert(acl_node->sai_acl_rule_tree, acl_rule);
        if (rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("Failed to insert ACL Rule "
                             "Id 0x%"PRIx64" in software Table Id 0x%"PRIx64" rc %d",
                             acl_rule->rule_key.acl_id,
                             acl_rule->table_id, rc);
            break;
        }


    } while(0);

    if (rc != SAI_STATUS_SUCCESS) {
       if (rule_installed) {
           sai_acl_npu_api_get()->delete_acl_rule(acl_table, acl_rule);
       }
       if (samplepacket_installed) {
           sai_acl_rule_remove_samplepacket(acl_rule);
       }
       sai_acl_rule_free(acl_rule);
    } else {
       sai_acl_rule_link(acl_table, acl_rule);
       acl_table->rule_count++;
       *acl_rule_id = acl_rule->rule_key.acl_id;
       SAI_ACL_LOG_INFO ("ACL entry 0x%"PRIx64" successfully programmed "
                         "in hardware", *acl_rule_id);
    }


    sai_acl_unlock();
    return rc;
}

sai_status_t sai_delete_acl_rule(sai_object_id_t acl_id)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_acl_rule_t *acl_rule = NULL;
    sai_acl_table_t *acl_table = NULL;
    acl_node_pt acl_node = NULL;
    bool cntr_detached = false, samplepacket_removed = false, policer_detached = false;

    if (!sai_is_obj_id_acl_entry(acl_id)) {
        SAI_ACL_LOG_ERR ("ACL Id 0x%"PRIx64" is not a ACL Entry Object", acl_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_acl_lock();
    do {
        acl_node = sai_acl_get_acl_node();
        acl_rule = sai_acl_rule_find(acl_node->sai_acl_rule_tree, acl_id);
        if (acl_rule == NULL) {
            SAI_ACL_LOG_ERR ("ACL Rule not present for "
                             "Rule ID 0x%"PRIx64"", acl_id);
            rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        /* Retrieve the ACL table node in the RB tree from the
         * table id present in the ACL rule structure. */
        acl_table = sai_acl_table_find(acl_node->sai_acl_table_tree,
                                               acl_rule->table_id);
        if (acl_table == NULL) {
            /* This is a fatal error as the table for the provided acl_id
             * is not present in ACL database. */
            SAI_ACL_LOG_ERR ("ACL Table  0x%"PRIx64" not present for "
                             "Rule ID  0x%"PRIx64"",acl_rule->table_id, acl_id);
            rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        /* Both rule and table are present in there respective RB trees.
         * Proceed to validate rule DLL in ACL table node to avoid any
         * ACL data-structures inconsistency. */
        if (acl_rule != (sai_acl_rule_validate(acl_table, acl_rule->rule_key.acl_id))) {
            SAI_ACL_LOG_ERR ("ACL Rule absent in the Rule DLL "
                             "stored in table Id  0x%"PRIx64" for ACL rule ID 0x%"PRIx64"",
                             acl_table->table_key.acl_table_id, acl_id);
            rc = SAI_STATUS_FAILURE;
            break;
        }

        if ((acl_rule->samplepacket_id[SAI_SAMPLEPACKET_DIR_INGRESS] != 0) ||
           (acl_rule->samplepacket_id[SAI_SAMPLEPACKET_DIR_EGRESS]) != 0) {
            rc = sai_acl_rule_remove_samplepacket(acl_rule);
            if (rc != SAI_STATUS_SUCCESS) {
                 SAI_ACL_LOG_ERR ("Unable to remove SamplePacket Config");
                 break;
            }
            samplepacket_removed = true;
        }

        /* Check whether counter needs to be detached */
        if (acl_rule->counter_id != 0) {
            rc = sai_detach_cntr_from_acl_rule(acl_rule);
            if (rc != SAI_STATUS_SUCCESS) {
                SAI_ACL_LOG_ERR ("ACL Counter 0x%"PRIx64" failed to detach with "
                                 "ACL rule ID 0x%"PRIx64" in Table Id 0x%"PRIx64"",
                                 acl_rule->counter_id,
                                 acl_rule->rule_key.acl_id,
                                 acl_rule->table_id);
                break;
            }
            cntr_detached = true;
        }
        if (acl_rule->policer_id != 0) {
            rc = sai_detach_policer_from_acl_rule(acl_rule);
            if (rc != SAI_STATUS_SUCCESS) {
                SAI_ACL_LOG_ERR ("Policer 0x%"PRIx64" failed to "
                                 "detach with ACL rule ID 0x%"PRIx64" in Table Id 0x%"PRIx64"",
                                 acl_rule->policer_id,
                                 acl_rule->rule_key.acl_id,
                                 acl_rule->table_id);
                break;
            }
            policer_detached = true;
        }

        /* Delete the entry in the hardware*/
        rc = sai_acl_npu_api_get()->delete_acl_rule(acl_table, acl_rule);
        if (rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("Failure deleting ACL Rule Id 0x%"PRIx64" "
                             "from hardware", acl_id);
            break;
        }

        /* Update all the SAI ACL data structures */
        acl_rule = sai_acl_rule_remove(acl_node->sai_acl_rule_tree, acl_rule);
        if (acl_rule == NULL) {
            /* Some internal error in RB tree, log an error */
            SAI_ACL_LOG_ERR ("Failure removing ACL Rule Id 0x%"PRIx64" "
                             "from Rule Database", acl_id);
            break;
        }
    } while(0);

    if (rc != SAI_STATUS_SUCCESS) {
        if (samplepacket_removed) {
            sai_acl_rule_create_samplepacket(acl_rule);
        }
        if (cntr_detached) {
            sai_attach_cntr_to_acl_rule(acl_rule);
        }
        if (policer_detached) {
            sai_attach_policer_to_acl_rule(acl_rule);
        }
    } else {
        sai_acl_rule_unlink(acl_table, acl_rule);
        acl_table->rule_count--;
        sai_acl_rule_free(acl_rule);
        SAI_ACL_LOG_INFO ("ACL Rule Id 0x%"PRIx64" successfully deleted "
                          "from hardware", acl_id);
    }
    sai_acl_unlock();
    return rc;
}

static void sai_acl_rule_update(sai_acl_rule_t *rule_scan,
                                sai_acl_rule_t *given_rule,
                                uint_t new_fields, uint_t new_actions,
                                bool rule_priority_change,
                                bool rule_state_change)
{
    sai_status_t rc = SAI_STATUS_FAILURE;

    STD_ASSERT(rule_scan != NULL);
    STD_ASSERT(given_rule != NULL);

    if (rule_priority_change) {
        /* Update the rule priority */
        given_rule->acl_rule_priority = rule_scan->acl_rule_priority;
    }

    if (rule_state_change) {
       /* Update the rule admin state */
       given_rule->acl_rule_state = rule_scan->acl_rule_state;
    }

    /* Update the fields and actions */
    rc = sai_acl_rule_update_field_action(rule_scan, given_rule,
                                          new_fields, new_actions);
    if (rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("ACL rule field/action set failed");
    }
}

static sai_status_t sai_acl_rule_copy(sai_acl_rule_t *dst_rule,
                               sai_acl_rule_t *src_rule)
{
    uint_t filter_count = 0, action_count = 0;
    sai_status_t rc = SAI_STATUS_FAILURE;

    STD_ASSERT(dst_rule != NULL);
    STD_ASSERT(src_rule != NULL);

    if (dst_rule->filter_list == NULL) {
        dst_rule->filter_list = (sai_acl_filter_t *)
                                     calloc(src_rule->filter_count,
                                            sizeof(sai_acl_filter_t));

        if (dst_rule->filter_list == NULL) {
            return SAI_STATUS_NO_MEMORY;
        }
    } else {
        if (dst_rule->filter_count != src_rule->filter_count) {
            return SAI_STATUS_FAILURE;
        }
        memset((uint8_t *)dst_rule->filter_list, 0,
               dst_rule->filter_count * sizeof(sai_acl_filter_t));
    }

    if (dst_rule->action_list == NULL) {
        dst_rule->action_list = (sai_acl_action_t *)
                                 calloc(src_rule->action_count,
                                        sizeof(sai_acl_action_t));

        if (dst_rule->action_list == NULL) {
            return SAI_STATUS_NO_MEMORY;
        }
    } else {
        if (dst_rule->action_count != src_rule->action_count) {
            return SAI_STATUS_FAILURE;
        }
        memset((uint8_t *)dst_rule->action_list, 0,
               dst_rule->action_count * sizeof(sai_acl_action_t));
    }

    for (filter_count = 0; filter_count < src_rule->filter_count;
         filter_count++) {
         if ((rc = sai_acl_rule_copy_filter(&dst_rule->filter_list[filter_count],
                   &src_rule->filter_list[filter_count])) != SAI_STATUS_SUCCESS) {
             return rc;
         }
    }

    for (action_count = 0; action_count < src_rule->action_count;
         action_count++) {
         if ((rc = sai_acl_rule_copy_action(&dst_rule->action_list[action_count],
                   &src_rule->action_list[action_count])) != SAI_STATUS_SUCCESS) {
             return rc;
         }
    }

    sai_acl_rule_copy_fields(dst_rule, src_rule);

    rc = sai_acl_npu_api_get()->copy_acl_rule(dst_rule, src_rule);

    if (rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("ACL BCM Rule Copy failed");
    }

    return rc;
}

static sai_status_t sai_acl_rule_scan_and_mod(sai_acl_rule_t *rule_scan,
                                             sai_acl_rule_t *given_rule)
{
    sai_acl_table_t *acl_table = NULL;
    uint_t new_fields = 0, new_actions = 0;
    sai_acl_rule_t *compare_rule = NULL;
    sai_status_t rc = SAI_STATUS_FAILURE;
    acl_node_pt acl_node = NULL;
    bool rule_priority_change = false, rule_state_change = false;

    STD_ASSERT(rule_scan != NULL);
    STD_ASSERT(given_rule != NULL);

    acl_node = sai_acl_get_acl_node();

    acl_table = sai_acl_table_find(acl_node->sai_acl_table_tree,
                                           given_rule->table_id);
    STD_ASSERT(acl_table != NULL);

    compare_rule = (sai_acl_rule_t *)calloc(1, sizeof(sai_acl_rule_t));
    if (compare_rule == NULL) {
        SAI_ACL_LOG_ERR ("Allocation of Memory "
                         "failed for ACL Rule");
        return SAI_STATUS_NO_MEMORY;
    }

    do {
        /* Copy the contents of given rule to fallback rule. This is
         * required as a fallback mechanism in case rule modification fails */
        rc = sai_acl_rule_copy(compare_rule, given_rule);
        if (rc != SAI_STATUS_SUCCESS) {
           break;
        }

        if (rule_scan->acl_rule_priority != given_rule->acl_rule_priority) {
            SAI_ACL_LOG_TRACE ("Set ACL Rule Priority Attribute "
                               "[Old Pri = %d, New Pri = %d]",
                               given_rule->acl_rule_priority,
                               rule_scan->acl_rule_priority);
            rule_priority_change = true;
        }

        if ((rule_scan->acl_rule_state != SAI_ACL_RULE_DEFAULT_ADMIN_STATE) &&
            rule_scan->acl_rule_state != given_rule->acl_rule_state) {
            SAI_ACL_LOG_TRACE ("Set ACL Rule Admin State "
                               "Attribute [Old State = %s, New State = %s]",
                               (given_rule->acl_rule_state ?
                               "ENABLE" : "DISABLE"),
                               (rule_scan->acl_rule_state ?
                               "ENABLE" : "DISABLE" ));
            rule_state_change = true;
        }

        /* Both filter and action list present in the ACL rule contains
         * double booleans.
         *
         * First boolean (filter/action)_change indicates that as part of
         * set rule attribute there is a change required in the existing
         * field/action already present in the given rule.
         *
         * Second boolean new_(filter/action) indicates that a new field
         * or action needs to be added to the existing set of filters/actions.
         *
         * Following function would set these booleans */
         sai_acl_rule_set_field_action(rule_scan, compare_rule, &new_fields,
                                           &new_actions);

        /* Rule scan has been relevantly marked with the field/action change or
         * new field/action which needs to be added. Now modify ACL BCM
         * structures and update in NPU */
        rc = sai_acl_npu_api_get()->set_acl_rule(acl_table, rule_scan,
                                                 compare_rule, given_rule);

        if (rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("ACL rule set attribute "
                             "failed in NPU");
            break;
        }

        /* Rule was successfully modified in NPU. ACL rule now needs to
         * be modified */
        sai_acl_rule_update(rule_scan, given_rule, new_fields, new_actions,
                            rule_priority_change, rule_state_change);
    } while(0);

    sai_acl_rule_free(compare_rule);

    return rc;
}

sai_status_t sai_set_acl_rule(sai_object_id_t acl_id,
                              const sai_attribute_t *attr)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_acl_rule_t *acl_rule_present = NULL;
    sai_acl_rule_t *acl_rule_modify = NULL;
    uint_t field_count = 0, action_count = 0;
    acl_node_pt acl_node = NULL;
    sai_acl_table_t *acl_table = NULL;
    sai_object_id_t acl_table_id = 0;

    STD_ASSERT(attr != NULL);

    if (!sai_is_obj_id_acl_entry(acl_id)) {
        SAI_ACL_LOG_ERR ("ACL Id 0x%"PRIx64" is not a ACL Entry Object",
                         acl_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    rc = sai_acl_check_rule_attributes(SAI_ACL_DEFAULT_SET_ATTR_COUNT, attr,
                                       &field_count, &action_count, &acl_table_id);
    if (rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Validation of ACL rule attributes failed");
        return rc;
    }

    sai_acl_lock();
    acl_node = sai_acl_get_acl_node();

    acl_rule_present = sai_acl_rule_find(acl_node->sai_acl_rule_tree,
                                         acl_id);
    if (acl_rule_present == NULL) {
        /*ACL Rule not present */
        SAI_ACL_LOG_ERR (" ACL Rule not present for Rule ID 0x%"PRIx64","
                         "rule set attribute failed", acl_id);

        sai_acl_unlock();
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    acl_table = sai_acl_table_find(acl_node->sai_acl_table_tree,
                                   acl_rule_present->table_id);

    STD_ASSERT(acl_table != NULL);

    acl_rule_modify = (sai_acl_rule_t *)calloc(1, sizeof(sai_acl_rule_t));

    if (acl_rule_modify == NULL) {
        SAI_ACL_LOG_ERR ("Allocation of Memory failed for ACL Rule");
        sai_acl_unlock();
        return SAI_STATUS_NO_MEMORY;
    }

    acl_rule_modify->rule_key.acl_id = acl_rule_present->rule_key.acl_id;

    do {
        rc = sai_acl_rule_populate(acl_table, acl_rule_modify,
                        SAI_ACL_DEFAULT_SET_ATTR_COUNT, attr,
                        field_count, action_count);

        if (rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("ACL Rule populate function failed");
            break;
        }

        if (attr->id != SAI_ACL_ENTRY_ATTR_PRIORITY) {
            acl_rule_modify->acl_rule_priority = acl_rule_present->acl_rule_priority;
        }

        rc = sai_acl_validate_set_rule(acl_rule_modify, acl_rule_present);

        if (rc != SAI_STATUS_SUCCESS) {
            break;
        }

        if ((rc = sai_acl_rule_validate_update_samplepacket(acl_rule_modify, acl_rule_present,
                  true, false)) != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("SamplePacket id could not be updated");
            break;
        }

        rc = sai_acl_rule_scan_and_mod(acl_rule_modify, acl_rule_present);

        if (rc != SAI_STATUS_SUCCESS) {
            break;
        }

        if ((rc = sai_acl_rule_validate_update_samplepacket(acl_rule_modify, acl_rule_present,
                  false, true)) != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("SamplePacket id could not be updated");
            break;
        }

        rc = sai_acl_rule_policer_update(acl_rule_modify, acl_rule_present);
        if (rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR("Policer update failed");
            break;
        }

    } while(0);

    sai_acl_rule_free(acl_rule_modify);

    if (rc == SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_INFO ("Set Attribute successful for ACL Rule ID 0x%"PRIx64"", acl_id);
    }

    sai_acl_unlock();
    return rc;
}

sai_status_t sai_get_acl_rule(sai_object_id_t acl_id, uint32_t attr_count,
                              sai_attribute_t *attr_list)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_acl_rule_t *acl_rule = NULL;
    acl_node_pt acl_node = NULL;
    sai_acl_table_t *acl_table = NULL;

    if (!sai_is_obj_id_acl_entry(acl_id)) {
        SAI_ACL_LOG_ERR ("ACL Id 0x%"PRIx64" is not a ACL Entry Object",
                         acl_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if (attr_count == 0) {
        SAI_ACL_LOG_ERR ("Invalid attr_count");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(attr_list != NULL);

    sai_acl_lock();
    acl_node = sai_acl_get_acl_node();

    acl_rule = sai_acl_rule_find(acl_node->sai_acl_rule_tree,
                                 acl_id);

    if (acl_rule == NULL) {
        /* ACL Rule set attribute is invalid as the rule id
         * does not exist in SAI ACL db. */
        SAI_ACL_LOG_ERR ("ACL Rule not present for Rule ID 0x%"PRIx64", "
                         "rule get attribute failed", acl_id);
        sai_acl_unlock();
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    acl_table = sai_acl_table_find(acl_node->sai_acl_table_tree,
                                   acl_rule->table_id);
    STD_ASSERT(acl_table != NULL);

    rc = sai_acl_get_attr_from_rule(acl_table, acl_rule, attr_count, attr_list);
    if (rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Get ACL Rule Attribute failed");
    } else {
        SAI_ACL_LOG_INFO ("Get Attribute successful for ACL Rule ID 0x%"PRIx64"", acl_id);
    }

    sai_acl_unlock();
    return rc;
}
