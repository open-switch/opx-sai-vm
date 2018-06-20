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
 * * @file sai_acl_rule_utils.c
 * *
 * * @brief This file contains internal util functions for ACL rule operations.
 * *
 * *************************************************************************/
#include "sai_acl_utils.h"
#include "sai_acl_type_defs.h"
#include "sai_acl_npu_api.h"
#include "sai_acl_rule_utils.h"
#include "sai_common_acl.h"
#include "sai_port_utils.h"
#include "sai_mirror_api.h"
#include "sai_udf_api_utils.h"

#include "std_type_defs.h"
#include "std_rbtree.h"
#include "std_assert.h"
#include "sai_samplepacket_api.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static bool sai_acl_check_filter_change(sai_acl_filter_t *given_filter,
                                        sai_acl_filter_t *scan_filter)
{
    bool field_change = false;

    STD_ASSERT(given_filter != NULL);
    STD_ASSERT(scan_filter != NULL);

    /* Field used as part of setting ACL attribute which is already
     * present in the existing ACL rule. Verify each component of
     * the field. */
    if (scan_filter->enable != given_filter->enable) {
        /* Field's admin state has been changed */
        SAI_ACL_LOG_TRACE ("ACL field %d admin state changed from %d to %d",
                           scan_filter->field, given_filter->enable,
                           scan_filter->enable);
        field_change = true;
    }

    if (sai_acl_rule_udf_field_attr_range(scan_filter->field)) {
        if ((scan_filter->match_data.u8_list.count !=
            given_filter->match_data.u8_list.count) ||
            (scan_filter->match_mask.u8_list.count !=
            given_filter->match_mask.u8_list.count) ||
            (memcmp(scan_filter->match_data.u8_list.list,
            given_filter->match_data.u8_list.list,
            scan_filter->match_data.u8_list.count * sizeof(uint8_t)) != 0) ||
            (memcmp(scan_filter->match_mask.u8_list.list,
            given_filter->match_mask.u8_list.list,
            scan_filter->match_mask.u8_list.count * sizeof(uint8_t)) != 0)) {
            field_change = true;
        }
    } else {
        if (memcmp(&scan_filter->match_data, &given_filter->match_data,
            sizeof(scan_filter->match_data)) != 0) {
           /* Field's data has been changed */
            SAI_ACL_LOG_TRACE ("ACL field %d Match Data changed",
                               scan_filter->field);
            field_change = true;
        } else if (memcmp(&scan_filter->match_mask, &given_filter->match_mask,
                   sizeof(scan_filter->match_mask)) != 0) {
            /* Field's mask has been changed */
            SAI_ACL_LOG_TRACE ("ACL field %d Match Mask changed",
                               scan_filter->field);

            field_change = true;
        }
    }

    if (!field_change) {
        SAI_ACL_LOG_TRACE ("No change required in ACL field %d",
                           scan_filter->field);
    }

    return field_change;
}

static bool sai_acl_check_action_change(sai_acl_action_t *given_action,
                                        sai_acl_action_t *scan_action)
{
    bool action_change = false;
    STD_ASSERT(given_action != NULL);
    STD_ASSERT(scan_action != NULL);

    /* Action used as part of setting ACL attribute which is
     * already present in the existing ACL rule. Verify each
     * component of the action. */
    if (scan_action->enable != given_action->enable) {
        /* Action's admin state has been changed */
        SAI_ACL_LOG_TRACE ("ACL Action %d Admin State Changed "
                           "[Old State = %s, New State = %s]",
                           scan_action->action,
                           (given_action->enable ? "ENABLE" : "DISABLE"),
                           (scan_action->enable ? "ENABLE" : "DISABLE" ));
        action_change = true;
    }

    if (memcmp(&scan_action->parameter, &given_action->parameter,
               sizeof(scan_action->parameter)) != 0) {
        /* Field's data has been changed */
        SAI_ACL_LOG_TRACE ("ACL action %d parameter value "
                           "changed", scan_action->action);
        action_change = true;
    }

    if (!action_change) {
        SAI_ACL_LOG_TRACE ("No change required in ACL action %d",
                            scan_action->action);
    }

    return action_change;
}

sai_acl_rule_t *sai_acl_rule_validate(sai_acl_table_t *acl_table,
                                      sai_object_id_t acl_id)
{
    sai_acl_rule_t *acl_rule = NULL;

    STD_ASSERT(acl_table != NULL);

    /* Search in the ACL Rule DLL */
    acl_rule = (sai_acl_rule_t *) std_dll_getfirst(&acl_table->rule_head);

    while (acl_rule != NULL) {
        if (acl_rule->rule_key.acl_id == acl_id) {
            /* Rule found with same ACL id */
            return acl_rule;
        }
        acl_rule = (sai_acl_rule_t *) std_dll_getnext(&acl_table->rule_head,
                                                      &acl_rule->rule_link);
    }
    return NULL;
}

bool sai_acl_object_list_field_attr(sai_acl_entry_attr_t entry_attr)
{
    if ((entry_attr == SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS) ||
        (entry_attr == SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS)) {
        return true;
    }
    return false;
}

bool sai_acl_object_list_action_attr(sai_acl_entry_attr_t entry_attr)
{
    if ((entry_attr == SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS) ||
        (entry_attr == SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS) ||
        (entry_attr == SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT_LIST)) {
        return true;
    }
    return false;
}
bool sai_acl_range_attr(sai_acl_entry_attr_t entry_attr)
{
    if(entry_attr == SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE)
    {
        return true;
    }
    return false;
}

bool sai_acl_portlist_attr(sai_acl_entry_attr_t entry_attr)
{
    if ((entry_attr == SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS) ||
        (entry_attr == SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS)) {
        return true;
    }
    return false;
}

bool sai_acl_port_attr(sai_acl_entry_attr_t entry_attr)
{
    if ((entry_attr == SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT)  ||
        (entry_attr == SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT)) {
        return true;
    }
    return false;
}

void sai_acl_rule_link(sai_acl_table_t *acl_table,
                       sai_acl_rule_t *acl_rule)
{
    STD_ASSERT(acl_table != NULL);
    STD_ASSERT(acl_rule != NULL);

    /* Add the ACL rule element to the DLL. This will be inserted
     * in a sorting order based on rule priority */
    std_dll_insert(&acl_table->rule_head,
                   (std_dll *)&acl_rule->rule_link);

}

void sai_acl_rule_unlink(sai_acl_table_t *acl_table,
                         sai_acl_rule_t *acl_rule)
{
    STD_ASSERT(acl_table != NULL);
    STD_ASSERT(acl_rule != NULL);

   /* Remove the ACL rule element from the DLL.*/
    if (std_dll_islinked(&acl_rule->rule_link)) {
        std_dll_remove(&acl_table->rule_head,
                       &acl_rule->rule_link);
    }
}

sai_status_t sai_acl_rule_insert(rbtree_handle sai_acl_rule_tree,
                                 sai_acl_rule_t *acl_rule)
{
    STD_ASSERT(sai_acl_rule_tree != NULL);
    STD_ASSERT(acl_rule != NULL);

    return (std_rbtree_insert(sai_acl_rule_tree, acl_rule)
            == STD_ERR_OK ? SAI_STATUS_SUCCESS: SAI_STATUS_FAILURE);
}

sai_acl_rule_t *sai_acl_rule_remove(rbtree_handle sai_acl_rule_tree,
                                    sai_acl_rule_t *acl_rule)
{
    STD_ASSERT(sai_acl_rule_tree != NULL);
    STD_ASSERT(acl_rule != NULL);

    return (sai_acl_rule_t *)std_rbtree_remove(sai_acl_rule_tree, acl_rule);
}

sai_acl_rule_t *sai_acl_rule_find(rbtree_handle sai_acl_rule_tree,
                                  sai_object_id_t acl_id)
{
    sai_acl_rule_t tmp_acl_rule = { .rule_key.acl_id = acl_id };

    STD_ASSERT(sai_acl_rule_tree != NULL);
    return ((sai_acl_rule_t *)std_rbtree_getexact(sai_acl_rule_tree,
                                                  &tmp_acl_rule));
}

sai_status_t sai_acl_rule_copy_filter(sai_acl_filter_t *dst_filter,
                              sai_acl_filter_t *src_filter)
{
    uint_t object_count = 0, u8_count = 0;

    STD_ASSERT(src_filter != NULL);
    STD_ASSERT(dst_filter != NULL);

    dst_filter->field_change = src_filter->field_change;
    dst_filter->new_field = src_filter->new_field;
    dst_filter->field = src_filter->field;
    dst_filter->enable = src_filter->enable;

    if (sai_acl_object_list_field_attr(src_filter->field)) {
        if ((src_filter->match_data.obj_list.list == NULL) &&
            (src_filter->match_data.obj_list.count != 0)) {
            SAI_ACL_LOG_ERR ("Source Object list empty");
            return SAI_STATUS_FAILURE;
        }

        dst_filter->match_data.obj_list.count =
                                    src_filter->match_data.obj_list.count;
        dst_filter->match_data.obj_list.list  = (sai_object_id_t *)calloc(
                                        src_filter->match_data.obj_list.count,
                                        sizeof(sai_object_id_t));

        if (!dst_filter->match_data.obj_list.list) {
            return SAI_STATUS_NO_MEMORY;
        }

        for (object_count = 0; object_count <
             src_filter->match_data.obj_list.count; object_count++) {
             dst_filter->match_data.obj_list.list[object_count] =
                 src_filter->match_data.obj_list.list[object_count];
        }
    } else if (sai_acl_rule_udf_field_attr_range(src_filter->field)) {
        if (((src_filter->match_data.u8_list.list == NULL) &&
            (src_filter->match_data.u8_list.count != 0)) ||
            ((src_filter->match_mask.u8_list.list == NULL) &&
            (src_filter->match_mask.u8_list.count != 0))) {
            SAI_ACL_LOG_ERR ("Source Byte list empty");
            return SAI_STATUS_FAILURE;
        }

        dst_filter->udf_field.udf_group_id = src_filter->udf_field.udf_group_id;
        dst_filter->udf_field.udf_attr_index = src_filter->udf_field.udf_attr_index;

        dst_filter->match_data.u8_list.count =
                            src_filter->match_data.u8_list.count;
        dst_filter->match_mask.u8_list.count =
                            src_filter->match_mask.u8_list.count;
        dst_filter->match_data.u8_list.list = (uint8_t *)calloc(
                                        src_filter->match_data.u8_list.count,
                                        sizeof(uint8_t));

        if (!dst_filter->match_data.u8_list.list) {
            return SAI_STATUS_NO_MEMORY;
        }

        dst_filter->match_mask.u8_list.list = (uint8_t *)calloc(
                                        src_filter->match_mask.u8_list.count,
                                        sizeof(uint8_t));

        if (!dst_filter->match_mask.u8_list.list) {
            free (dst_filter->match_data.u8_list.list);
            return SAI_STATUS_NO_MEMORY;
        }

        for (u8_count = 0; u8_count < src_filter->match_data.u8_list.count; u8_count++) {
             dst_filter->match_data.u8_list.list[u8_count] =
                 src_filter->match_data.u8_list.list[u8_count];
        }

        for (u8_count = 0; u8_count < src_filter->match_mask.u8_list.count; u8_count++) {
             dst_filter->match_mask.u8_list.list[u8_count] =
                 src_filter->match_mask.u8_list.list[u8_count];
        }

    } else {

        memcpy((uint8_t*)&dst_filter->match_data,
               (uint8_t*)&src_filter->match_data,
               sizeof(dst_filter->match_data));

        memcpy((uint8_t*)&dst_filter->match_mask,
               (uint8_t*)&src_filter->match_mask,
               sizeof(dst_filter->match_mask));
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_rule_copy_action(sai_acl_action_t *dst_action,
                                      sai_acl_action_t *src_action)
{
    uint_t object_count = 0;

    STD_ASSERT(src_action != NULL);
    STD_ASSERT(dst_action != NULL);

    dst_action->action_change = src_action->action_change;
    dst_action->new_action = src_action->new_action;
    dst_action->action = src_action->action;
    dst_action->enable = src_action->enable;

    if (sai_acl_object_list_action_attr(src_action->action)) {
        if ((src_action->parameter.obj_list.list == NULL) &&
            (src_action->parameter.obj_list.count != 0)) {
            SAI_ACL_LOG_ERR ("Source Object list empty");
            return SAI_STATUS_FAILURE;
        }

        dst_action->parameter.obj_list.count =
                                    src_action->parameter.obj_list.count;
        dst_action->parameter.obj_list.list  = (sai_object_id_t *)calloc(
                                    src_action->parameter.obj_list.count,
                                        sizeof(sai_object_id_t));

        if (!dst_action->parameter.obj_list.list) {
            return SAI_STATUS_NO_MEMORY;
        }

        for (object_count = 0; object_count <
             src_action->parameter.obj_list.count; object_count++) {
             dst_action->parameter.obj_list.list[object_count] =
                 src_action->parameter.obj_list.list[object_count];
        }
    } else {

        memcpy((uint8_t*)&dst_action->parameter,
               (uint8_t*)&src_action->parameter,
               sizeof(dst_action->parameter));
    }

    return SAI_STATUS_SUCCESS;

}

void sai_acl_rule_copy_fields(sai_acl_rule_t *dst_rule,
                              sai_acl_rule_t *src_rule)
{
    STD_ASSERT(src_rule != NULL);
    STD_ASSERT(dst_rule != NULL);

    dst_rule->rule_key.acl_id = src_rule->rule_key.acl_id;
    dst_rule->acl_rule_priority = src_rule->acl_rule_priority;
    dst_rule->table_id = src_rule->table_id;
    dst_rule->acl_rule_state = src_rule->acl_rule_state;
    dst_rule->counter_id = src_rule->counter_id;
    dst_rule->filter_count = src_rule->filter_count;
    dst_rule->action_count = src_rule->action_count;
}

sai_status_t sai_acl_validate_create_rule(sai_acl_table_t *acl_table,
                                          sai_acl_rule_t *acl_rule)
{
    acl_node_pt acl_node = NULL;
    sai_acl_counter_t *acl_counter = NULL;

    STD_ASSERT(acl_table != NULL);
    STD_ASSERT(acl_rule != NULL);

    acl_node = sai_acl_get_acl_node();

    if (acl_rule->table_id == SAI_ACL_INVALID_TABLE_ID) {
        SAI_ACL_LOG_ERR ("Mandatory attribute table id not "
                         "present for ACL rule 0x%"PRIx64"",
                         acl_rule->rule_key.acl_id);
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    if (acl_rule->counter_id) {
        acl_counter = sai_acl_cntr_find(acl_node->sai_acl_counter_tree,
                                        acl_rule->counter_id);
        if (acl_counter == NULL) {
            SAI_ACL_LOG_ERR ("ACL Counter not present "
                             "for Counter ID 0x%"PRIx64"",
                             acl_rule->counter_id);
            return SAI_STATUS_INVALID_OBJECT_ID;
        }

        if (acl_counter->table_id != acl_rule->table_id) {
            SAI_ACL_LOG_ERR ("ACL Counter table Id 0x%"PRIx64" does "
                             "not match with Rule Table Id 0x%"PRIx64"",
                             acl_counter->table_id,
                             acl_rule->table_id);
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_validate_set_rule(sai_acl_rule_t *modify_rule,
                                       sai_acl_rule_t *given_rule)
{
    acl_node_pt acl_node = NULL;
    sai_acl_counter_t *acl_counter = NULL;

    STD_ASSERT(modify_rule != NULL);
    STD_ASSERT(given_rule != NULL);

    acl_node = sai_acl_get_acl_node();

    /* Set rule attribute has a table id, check if it matches
     * the existing rule's table id, else there is no change*/
    if (modify_rule->table_id) {
        if (modify_rule->table_id != given_rule->table_id) {
            SAI_ACL_LOG_ERR ("Cannot modify table id during "
                             "set rule attribute, modified table id = 0x%"PRIx64" "
                             "existing table id = 0x%"PRIx64"",
                             modify_rule->table_id,
                             given_rule->table_id);

            return SAI_STATUS_ATTR_NOT_SUPPORTED_0;
        }
    }

    if (modify_rule->counter_id) {
        if (modify_rule->counter_id != given_rule->counter_id)
        {
            /* Set rule attribute has a counter, check if counter exists*/
            acl_counter = sai_acl_cntr_find(acl_node->sai_acl_counter_tree,
                                            modify_rule->counter_id);

            if (acl_counter == NULL) {
                SAI_ACL_LOG_ERR ("ACL Counter not present "
                                 "for Counter ID 0x%"PRIx64"",modify_rule->counter_id);
                return SAI_STATUS_INVALID_OBJECT_ID;
            }

            if (acl_counter->table_id != given_rule->table_id) {
                SAI_ACL_LOG_ERR ("ACL Counter table Id 0x%"PRIx64" does "
                                 "not match with Rule Table Id 0x%"PRIx64"",
                                 acl_counter->table_id,
                                 given_rule->table_id);
                return SAI_STATUS_INVALID_OBJECT_ID;
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

void sai_acl_rule_set_field_action(sai_acl_rule_t *rule_scan,
                                   sai_acl_rule_t *given_rule,
                                   uint_t *new_field_cnt,
                                   uint_t *new_action_cnt)
{
    uint_t scan_filter_count = 0, scan_action_count = 0;
    uint_t given_filter_count = 0, given_action_count = 0;
    uint_t new_field_count = 0, new_action_count = 0;
    sai_acl_filter_t *given_filter = NULL, *scan_filter = NULL;
    sai_acl_action_t *given_action = NULL, *scan_action = NULL;

    STD_ASSERT(rule_scan != NULL);
    STD_ASSERT(given_rule != NULL);
    STD_ASSERT(new_field_cnt != NULL);
    STD_ASSERT(new_action_cnt != NULL);

    for (scan_filter_count = 0;
         scan_filter_count < rule_scan->filter_count;
         scan_filter_count++) {

         scan_filter = &rule_scan->filter_list[scan_filter_count];

        for (given_filter_count = 0;
             given_filter_count < given_rule->filter_count;
             given_filter_count++) {

             given_filter = &given_rule->filter_list[given_filter_count];

            if (scan_filter->field == given_filter->field) {

                scan_filter->field_change = sai_acl_check_filter_change(
                                                   given_filter, scan_filter);
                break;
            }
        }

        if (given_filter_count == given_rule->filter_count) {
            /* Reached the end of inner loop and still the field didn't
             * match with the existing filter set. */
            new_field_count++;
            scan_filter->new_field = true;
            SAI_ACL_LOG_TRACE ("New ACL field %d needs to "
                               "be added", scan_filter->field);
        }
    }

    for (scan_action_count = 0;
         scan_action_count < rule_scan->action_count;
         scan_action_count++) {

        scan_action = &rule_scan->action_list[scan_action_count];

        for (given_action_count = 0;
             given_action_count < given_rule->action_count;
             given_action_count++) {

            given_action = &given_rule->action_list[given_action_count];

            if (scan_action->action == given_action->action) {
                scan_action->action_change = sai_acl_check_action_change(
                                                   given_action, scan_action);
                break;

            }
        }

        if (given_action_count == given_rule->action_count) {
            /* Reached the end of inner loop and still the action
             * didn't match with the existing action set. */
            new_action_count++;
            scan_action->new_action = true;
            SAI_ACL_LOG_TRACE ("New ACL action %d needs "
                               "to be added", scan_action->action);
        }
    }

    *new_field_cnt  = new_field_count;
    *new_action_cnt = new_action_count;
}

static sai_status_t sai_acl_rule_update_byte_list(sai_u8_list_t *src_data_list,
                                                  sai_u8_list_t *src_mask_list,
                                                  sai_u8_list_t *dst_data_list,
                                                  sai_u8_list_t *dst_mask_list,
                                                  bool src_action)
{
    uint_t           byte_cnt = 0;
    uint8_t         *byte_data_list = NULL, *byte_mask_list = NULL;

    if ((dst_data_list->count != 0) || (dst_mask_list->count != 0)) {
        /*
         * Following scenarios possible in Set operation
         * - Set to Disable
         * - Set to different count of bytes
         * - Set to same count but different byte data
         */
        /* Validation of byte list in existing rule */
        if ((dst_data_list->list == NULL) || (dst_mask_list->list == NULL)) {
            SAI_ACL_LOG_ERR ("Byte List empty for a valid byte count");
            return SAI_STATUS_FAILURE;
        }

        if (!src_action) {
            /* Free all the memory in the given rule */
            dst_data_list->count = 0;
            dst_mask_list->count = 0;

            if (dst_data_list->list) {
                free (dst_data_list->list);
                dst_data_list->list = NULL;
            }
            if (dst_mask_list->list) {
                free (dst_mask_list->list);
                dst_mask_list->list = NULL;
            }
        } else if ((src_data_list->count != 0) &&
                   (src_data_list->count != dst_data_list->count)) {
            /* Object List needs to be re-assigned */
            byte_cnt = src_data_list->count;

            byte_data_list = (uint8_t *)calloc(byte_cnt, sizeof(uint8_t));
            if (byte_data_list == NULL) {
                SAI_ACL_LOG_ERR ("System out of memory for byte data list");
                return SAI_STATUS_NO_MEMORY;
            }
            byte_mask_list = (uint8_t *)calloc(byte_cnt, sizeof(uint8_t));
            if (byte_mask_list == NULL) {
                SAI_ACL_LOG_ERR ("System out of memory for byte mask list");
                free (byte_data_list);
                return SAI_STATUS_NO_MEMORY;
            }

            free (dst_data_list->list);
            free (dst_mask_list->list);

            memcpy(byte_data_list, src_data_list->list, (byte_cnt * sizeof(uint8_t)));
            memcpy(byte_mask_list, src_mask_list->list, (byte_cnt * sizeof(uint8_t)));

            dst_data_list->count = byte_cnt;
            dst_data_list->list = byte_data_list;
            dst_mask_list->count = byte_cnt;
            dst_mask_list->list = byte_mask_list;

        } else if ((src_data_list->count == dst_data_list->count) &&
            (src_data_list->count == dst_mask_list->count)) {

            /* Just copy the object list to the rule */
            byte_cnt = src_data_list->count;
            byte_data_list = src_data_list->list;
            byte_mask_list = src_mask_list->list;

            memcpy(dst_data_list->list, byte_data_list, (byte_cnt * sizeof(uint8_t)));
            memcpy(dst_mask_list->list, byte_mask_list, (byte_cnt * sizeof(uint8_t)));
        }
    } else {
        /*
         * Existing Rule doesn't have byte list. Following conditions
         * need to be handled.
         * - Set to new byte list
         * - Set to Disable (No Operation)
         */
        /* Validation for any potential memory leak */
        if (dst_data_list->list) {
            SAI_ACL_LOG_ERR ("Byte Data list still exist with byte count as 0");
            free (dst_data_list->list);
            dst_data_list->list = NULL;
        }

        if (dst_mask_list->list) {
            SAI_ACL_LOG_ERR ("Byte Mask list still exist with byte count as 0");
            free (dst_mask_list->list);
            dst_mask_list->list = NULL;
        }

        if (src_action && (src_data_list->count != 0)) {
            byte_cnt = src_data_list->count;

            byte_data_list = (uint8_t *)calloc(byte_cnt, sizeof(uint8_t));
            if (byte_data_list == NULL) {
                SAI_ACL_LOG_ERR ("System out of memory for byte data list");
                return SAI_STATUS_NO_MEMORY;
            }

            byte_mask_list = (uint8_t *)calloc(byte_cnt, sizeof(uint8_t));
            if (byte_mask_list == NULL) {
                SAI_ACL_LOG_ERR ("System out of memory for byte mask list");
                free (byte_data_list);
                return SAI_STATUS_NO_MEMORY;
            }

            memcpy(byte_data_list, src_data_list->list, (byte_cnt * sizeof(uint8_t)));
            memcpy(byte_mask_list, src_mask_list->list, (byte_cnt * sizeof(uint8_t)));

            dst_data_list->count = byte_cnt;
            dst_data_list->list = byte_data_list;
            dst_mask_list->count = byte_cnt;
            dst_mask_list->list = byte_mask_list;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_rule_update_object_list(sai_object_list_t *src_obj_list,
                                                    sai_object_list_t *dst_obj_list,
                                                    bool src_action)
{
    uint_t           object_cnt = 0;
    sai_object_id_t *object_list  = NULL;

    if (dst_obj_list->count != 0) {
        /*
         * Following scenarios possible in Set operation
         * - Set to Disable
         * - Set to different count of objects
         * - Set to same count but different object ids
         */
        /* Validation of object list in existing rule */
        if (dst_obj_list->list == NULL) {
            SAI_ACL_LOG_ERR ("Object List empty for a valid object count");
            return SAI_STATUS_FAILURE;
        }

        if (!src_action) {
            /* Free all the memory in the given rule */
            dst_obj_list->count = 0;
            if (dst_obj_list->list) {
                free (dst_obj_list->list);
                dst_obj_list->list = NULL;
            }
        } else if ((src_obj_list->count != 0) &&
                   (src_obj_list->count != dst_obj_list->count)) {
            /* Object List needs to be re-assigned */
            object_cnt = src_obj_list->count;

            object_list = (sai_object_id_t *)calloc(object_cnt, sizeof(sai_object_id_t));
            if (object_list == NULL) {
                SAI_ACL_LOG_ERR ("System out of memory for object list");
                return SAI_STATUS_NO_MEMORY;
            }

            free (dst_obj_list->list);

            memcpy(object_list, src_obj_list->list,
                   (object_cnt * sizeof(sai_object_id_t)));

            dst_obj_list->count = object_cnt;
            dst_obj_list->list = object_list;
        } else if (src_obj_list->count == dst_obj_list->count) {

            /* Just copy the object list to the rule */
            object_cnt = src_obj_list->count;
            object_list = src_obj_list->list;

            memcpy(dst_obj_list->list, object_list,
                   (object_cnt * sizeof(sai_object_id_t)));
        }
    } else {
        /*
         * Existing Rule doesn't have object list. Following conditions
         * need to be handled.
         * - Set to new object list
         * - Set to Disable (No Operation)
         */
        /* Validation for any potential memory leak */
        if (dst_obj_list->list) {
            SAI_ACL_LOG_ERR ("Object list still exist with object count as 0");
            free (dst_obj_list->list);
            dst_obj_list->list = NULL;
        }

        if (src_action && (src_obj_list->count != 0)) {
            object_cnt = src_obj_list->count;
            object_list = (sai_object_id_t *)calloc(object_cnt, sizeof(sai_object_id_t));
            if (object_list == NULL) {
                SAI_ACL_LOG_ERR ("System out of memory for object list");
                return SAI_STATUS_NO_MEMORY;
            }

            memcpy(object_list, src_obj_list->list,
                   (object_cnt * sizeof(sai_object_id_t)));

            dst_obj_list->count = object_cnt;
            dst_obj_list->list = object_list;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_rule_update_new_fields(
                                            sai_acl_rule_t *rule_scan,
                                            sai_acl_rule_t *given_rule,
                                            uint_t total_fields)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    uint_t scan_filter_count = 0, given_filter_count = 0;
    sai_acl_filter_t *filter_list = NULL;

    STD_ASSERT(rule_scan != NULL);
    STD_ASSERT(given_rule != NULL);
    STD_ASSERT(total_fields > 0);

    filter_list = (sai_acl_filter_t *)calloc(total_fields,
                                             sizeof(sai_acl_filter_t));

    if (filter_list == NULL) {
        SAI_ACL_LOG_ERR ("Memory allocation failed for "
                         "filter_list");
        return  SAI_STATUS_NO_MEMORY;
    }

    /* First copy the old set of fields  */
    for (given_filter_count = 0; given_filter_count < given_rule->filter_count;
         given_filter_count++) {

         memcpy(&filter_list[given_filter_count],
                &given_rule->filter_list[given_filter_count],
                sizeof(sai_acl_filter_t));
    }

    /* Now copy the new fields */
    given_filter_count = given_rule->filter_count;
    for (scan_filter_count = 0; scan_filter_count < rule_scan->filter_count;
         scan_filter_count++) {

        if (rule_scan->filter_list[scan_filter_count].new_field) {

                filter_list[given_filter_count].enable =
                                rule_scan->filter_list[scan_filter_count].enable;
                filter_list[given_filter_count].field =
                                rule_scan->filter_list[scan_filter_count].field;

            if (sai_acl_object_list_field_attr(rule_scan->filter_list[
                scan_filter_count].field)) {
                /* Update the object list */
                rc = sai_acl_rule_update_object_list(&rule_scan->filter_list[
                                scan_filter_count].match_data.obj_list,
                                &filter_list[given_filter_count].match_data.obj_list,
                                rule_scan->filter_list[scan_filter_count].enable);
                if (rc != SAI_STATUS_SUCCESS) {
                    SAI_ACL_LOG_ERR ("Object List field update failed");
                    free (filter_list);
                    return rc;
                }
            } else if (sai_acl_rule_udf_field_attr_range(rule_scan->filter_list[
                scan_filter_count].field)) {
                /* Update the byte list */
                rc = sai_acl_rule_update_byte_list(&rule_scan->filter_list[scan_filter_count].
                                match_data.u8_list, &rule_scan->filter_list[scan_filter_count].
                                match_mask.u8_list, &filter_list[given_filter_count].
                                match_data.u8_list, &filter_list[given_filter_count].
                                match_mask.u8_list, rule_scan->filter_list[scan_filter_count].enable);
                if (rc != SAI_STATUS_SUCCESS) {
                    SAI_ACL_LOG_ERR ("Byte List field update failed");
                    free (filter_list);
                    return rc;
                }
            } else {
                memcpy(&filter_list[given_filter_count],
                   &rule_scan->filter_list[scan_filter_count],
                   sizeof(sai_acl_filter_t));
            }
            filter_list[given_filter_count].new_field = false;
            given_filter_count++;
        }
    }

    /* Free the old filter list memory in the given entry */
    free(given_rule->filter_list);
    given_rule->filter_list = filter_list;
    given_rule->filter_count = total_fields;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_rule_update_counter_action(
                                          sai_acl_rule_t *rule_scan,
                                          sai_acl_rule_t *given_rule,
                                          bool counter_action)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(rule_scan != NULL);
    STD_ASSERT(given_rule != NULL);

    if (!counter_action) {
        if (given_rule->counter_id != 0) {
            /* Just detach the old counter */

            rc = sai_detach_cntr_from_acl_rule(given_rule);

            if (rc != SAI_STATUS_SUCCESS) {
                return rc;
            }
            SAI_ACL_LOG_TRACE ("Counter 0x%"PRIx64" detached from ACL rule id 0x%"PRIx64"",
                               given_rule->counter_id,
                               given_rule->rule_key.acl_id);
            given_rule->counter_id = 0;
        }
    } else if (given_rule->counter_id != 0) {
        /* Case of detach and re-attach */
        if (rule_scan->counter_id != given_rule->counter_id) {

            SAI_ACL_LOG_TRACE ("Counter 0x%"PRIx64" being detached and Counter 0x%"PRIx64" "
                               "being attached to ACL Rule 0x%"PRIx64"",
                               given_rule->counter_id, rule_scan->counter_id,
                               given_rule->rule_key.acl_id);

            rc = sai_detach_cntr_from_acl_rule(given_rule);

            if (rc != SAI_STATUS_SUCCESS) {
                SAI_ACL_LOG_ERR ("ACL Counter 0x%"PRIx64" failed to detach from ACL "
                                 "Rule Id 0x%"PRIx64"", given_rule->counter_id,
                                 given_rule->rule_key.acl_id);
                return rc;
            }
            given_rule->counter_id = rule_scan->counter_id;

            rc = sai_attach_cntr_to_acl_rule(given_rule);
            if (rc != SAI_STATUS_SUCCESS) {
                SAI_ACL_LOG_ERR ("ACL Counter 0x%"PRIx64" failed to attach with ACL "
                                 "Rule Id 0x%"PRIx64"", given_rule->counter_id,
                                 given_rule->rule_key.acl_id);
                return rc;
            }

            SAI_ACL_LOG_TRACE ("New Counter id 0x%"PRIx64" attached to ACL rule id 0x%"PRIx64"",
                               given_rule->counter_id,
                               given_rule->rule_key.acl_id);
        }
    } else {
        if (given_rule->counter_id == 0) {
            /* Just attach the new counter */
            given_rule->counter_id = rule_scan->counter_id;

            rc = sai_attach_cntr_to_acl_rule(given_rule);
            if (rc != SAI_STATUS_SUCCESS) {
                SAI_ACL_LOG_ERR ("ACL Counter 0x%"PRIx64" failed to attach with ACL "
                                 "Rule Id 0x%"PRIx64"", given_rule->counter_id,
                                 given_rule->rule_key.acl_id);
                return rc;
            }

            SAI_ACL_LOG_TRACE ("New Counter id 0x%"PRIx64" attached to ACL rule id 0x%"PRIx64"",
                               given_rule->counter_id,
                               given_rule->rule_key.acl_id);
        }
    }

    return rc;
}

static sai_status_t sai_acl_rule_update_new_actions(
                                             sai_acl_rule_t *rule_scan,
                                             sai_acl_rule_t *given_rule,
                                             uint_t total_actions)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    sai_acl_action_t *action_list = NULL;
    uint_t given_action_count = 0, scan_action_count = 0;

    STD_ASSERT(rule_scan != NULL);
    STD_ASSERT(given_rule != NULL);
    STD_ASSERT(total_actions > 0);

    action_list = (sai_acl_action_t *)
                  calloc(total_actions,sizeof(sai_acl_action_t));

    if (action_list == NULL) {
        SAI_ACL_LOG_ERR ("Memory allocation failed for "
                         "filter_list");
        return  SAI_STATUS_NO_MEMORY;
    }

    /* First copy the old set of actions */
    for (given_action_count = 0; given_action_count < given_rule->action_count;
         given_action_count++) {

         memcpy(&action_list[given_action_count],
               &given_rule->action_list[given_action_count],
               sizeof(sai_acl_action_t));
    }

    /* Now copy the new actions */
    given_action_count = given_rule->action_count;
    for (scan_action_count = 0;
         scan_action_count < rule_scan->action_count; scan_action_count++) {

         if (rule_scan->action_list[scan_action_count].new_action) {
             if (sai_acl_object_list_action_attr(rule_scan->action_list[
                scan_action_count].action)) {
                action_list[given_action_count].enable =
                                rule_scan->action_list[scan_action_count].enable;
                action_list[given_action_count].action =
                                rule_scan->action_list[scan_action_count].action;
                /* Update the object list */
                rc = sai_acl_rule_update_object_list(&rule_scan->action_list[
                                scan_action_count].parameter.obj_list,
                                &action_list[given_action_count].parameter.obj_list,
                                rule_scan->action_list[scan_action_count].enable);
                if (rc != SAI_STATUS_SUCCESS) {
                    SAI_ACL_LOG_ERR ("Object List action update failed");
                    free (action_list);
                    return rc;
                }
             } else {
                memcpy(&action_list[given_action_count],
                       &rule_scan->action_list[scan_action_count],
                       sizeof(sai_acl_action_t));
                if (rule_scan->action_list[scan_action_count].action ==
                    SAI_ACL_ENTRY_ATTR_ACTION_COUNTER) {
                    rc = sai_acl_rule_update_counter_action(rule_scan, given_rule,
                                 rule_scan->action_list[scan_action_count].enable);
                    if (rc != SAI_STATUS_SUCCESS) {
                        SAI_ACL_LOG_ERR ("Counter Action Update failed");
                        free (action_list);
                        return rc;
                    }
                 }
             }
             action_list[given_action_count].new_action = false;
             given_action_count++;
         }
    }

    /* Free the old filter list memory in the given entry */
    free(given_rule->action_list);
    given_rule->action_list = action_list;
    given_rule->action_count = total_actions;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_rule_update_field_action(sai_acl_rule_t *rule_scan,
                                              sai_acl_rule_t *given_rule,
                                              uint_t new_fields,
                                              uint_t new_actions)
{
    uint_t total_fields = 0, total_actions = 0;
    uint_t scan_filter_count = 0, scan_action_count = 0;
    uint_t given_filter_count = 0, given_action_count = 0;
    sai_acl_filter_t *given_filter = NULL, *scan_filter = NULL;
    sai_acl_action_t *given_action = NULL, *scan_action = NULL;
    sai_status_t rc = SAI_STATUS_FAILURE;

    STD_ASSERT(rule_scan != NULL);
    STD_ASSERT(given_rule != NULL);

    /* All the field and actions were successfully modified or created
     * in hardware. They need to be updated in ACL Rule data structures too.
     * First Field/Action changes will be applied and then new Field/Action(s)
     * will be appended to the existing list.*/
    /* Starting with Field change */
    for (scan_filter_count = 0;
         scan_filter_count < rule_scan->filter_count;
         scan_filter_count++) {

         scan_filter = &rule_scan->filter_list[scan_filter_count];

         for (given_filter_count = 0;
              given_filter_count < given_rule->filter_count;
              given_filter_count++) {

              given_filter = &given_rule->filter_list[given_filter_count];

              if ((scan_filter->field_change == true) &&
                  (scan_filter->field == given_filter->field)) {

                  given_filter->enable = scan_filter->enable;

                  if (sai_acl_object_list_field_attr(given_filter->field)) {
                      /* Update the Field Object List */
                      rc = sai_acl_rule_update_object_list(
                                    &scan_filter->match_data.obj_list,
                                    &given_filter->match_data.obj_list,
                                    scan_filter->enable);
                      if (rc != SAI_STATUS_SUCCESS) {
                          SAI_ACL_LOG_ERR ("Object List field update failed");
                          return rc;
                      }
                  } else if (sai_acl_rule_udf_field_attr_range(given_filter->field)) {
                      /* Update the Byte List */
                      rc = sai_acl_rule_update_byte_list(
                                    &scan_filter->match_data.u8_list,
                                    &scan_filter->match_mask.u8_list,
                                    &given_filter->match_data.u8_list,
                                    &given_filter->match_mask.u8_list,
                                    scan_filter->enable);
                      if (rc != SAI_STATUS_SUCCESS) {
                          SAI_ACL_LOG_ERR ("Byte List field update failed");
                          return rc;
                      }
                  } else {
                    memcpy(given_filter, scan_filter, sizeof(sai_acl_filter_t));
                  }
                  given_filter->field_change = false;
                  break;
              }
         }
    }

    /* Action Change */
    for (scan_action_count = 0;
         scan_action_count < rule_scan->action_count;
         scan_action_count++) {

         scan_action = &rule_scan->action_list[scan_action_count];

         for (given_action_count = 0;
              given_action_count < given_rule->action_count;
              given_action_count++) {

              given_action = &given_rule->action_list[given_action_count];

              if ((scan_action->action_change == true) &&
                 (scan_action->action == given_action->action)) {
                  if (sai_acl_object_list_action_attr(given_action->action)) {
                      given_action->enable = scan_action->enable;
                      /* Update the Action Object List */
                      rc = sai_acl_rule_update_object_list(
                                    &scan_action->parameter.obj_list,
                                    &given_action->parameter.obj_list,
                                    scan_action->enable);
                      if (rc != SAI_STATUS_SUCCESS) {
                          SAI_ACL_LOG_ERR ("Object List action update failed");
                          return rc;
                      }
                  } else {
                      if (scan_action->action == SAI_ACL_ENTRY_ATTR_ACTION_COUNTER) {
                          rc = sai_acl_rule_update_counter_action(rule_scan, given_rule,
                                                              scan_action->enable);
                          if (rc != SAI_STATUS_SUCCESS) {
                              SAI_ACL_LOG_ERR ("Counter Action update failed");
                              return rc;
                          }
                      }
                      memcpy(given_action, scan_action, sizeof(sai_acl_action_t));
                  }
                  given_action->action_change = false;

                  break;
              }
         }
    }

    /* New Field */
    if (new_fields) {
        total_fields = new_fields + given_rule->filter_count;
        rc = sai_acl_rule_update_new_fields(rule_scan, given_rule,
                                            total_fields);

        if(rc != SAI_STATUS_SUCCESS) {
            return rc;
        }
    }

    /* New Action */
    if (new_actions) {
        total_actions = new_actions + given_rule->action_count;
        rc = sai_acl_rule_update_new_actions(rule_scan, given_rule,
                                            total_actions);

        if(rc != SAI_STATUS_SUCCESS) {
            if (new_fields) {
                free (given_rule->filter_list);
            }
            return rc;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static inline sai_status_t sai_acl_validate_object_type(
                                            sai_attr_id_t attribute_id,
                                            sai_object_id_t oid)
{
    bool invalid_object_type = false;

    if ((attribute_id == SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT) ||
        (attribute_id == SAI_ACL_ENTRY_ATTR_FIELD_DST_PORT)) {
        if (!sai_is_obj_id_port(oid) && !sai_is_obj_id_lag(oid)) {
            SAI_ACL_LOG_ERR ("Invalid In Port Object Type");
            invalid_object_type = true;
        }
    } else if ((attribute_id == SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT) ||
        (attribute_id == SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT) ||
        (attribute_id == SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS) ||
        (attribute_id == SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS)) {
        if (!sai_is_obj_id_port(oid)) {
            SAI_ACL_LOG_ERR ("Invalid Port Object Type");
            invalid_object_type = true;
        }
    } else if (attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_COUNTER) {
        if (!sai_is_obj_id_acl_counter(oid)) {
            SAI_ACL_LOG_ERR ("Invalid Counter Object Type");
            invalid_object_type = true;
        }
    } else if (attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER) {
        if (!sai_is_obj_id_policer(oid)) {
            SAI_ACL_LOG_ERR ("Invalid Policer Object Type");
            invalid_object_type = true;
        }
    } else if ((attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE) ||
               (attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE)) {
        if (!sai_is_obj_id_samplepkt_session(oid)) {
            SAI_ACL_LOG_ERR ("Invalid Sample Packet Object Type");
            invalid_object_type = true;
        }
    } else if (attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_SET_CPU_QUEUE) {
        if (!sai_is_obj_id_queue(oid)) {
            SAI_ACL_LOG_ERR ("Invalid Queue Object Type");
            invalid_object_type = true;
        }
    } else if ((attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS) ||
               (attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS)) {
        if (!sai_is_obj_id_mirror_session(oid)) {
            SAI_ACL_LOG_ERR ("Invalid Mirror Session Object Type");
            invalid_object_type = true;
        }
    } else if(attribute_id == SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE) {
        if(!sai_is_obj_id_acl_range(oid)) {
            SAI_ACL_LOG_ERR ("Invalid range Object type");
            invalid_object_type = true;
        }
    }


    if (invalid_object_type) {
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    return SAI_STATUS_SUCCESS;
}
static sai_status_t sai_acl_rule_filter_populate_range(
                                    sai_attr_id_t attribute_id,
                                    sai_acl_filter_t *acl_filter,
                                    sai_attribute_value_t *attr_value,
                                    bool sai_acl_get_api)
{
    uint_t range_cnt = 0;
    sai_object_id_t *range_list = NULL;
    sai_object_list_t *range_info = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    if (!sai_acl_get_api) {
        /* CREATE OP */
        range_info = &attr_value->aclfield.data.objlist;

        if (((range_info->count == 0) || (range_info->list == NULL)) &&
            attr_value->aclfield.enable) {
            SAI_ACL_LOG_ERR ("Invalid range list");
            return SAI_STATUS_INVALID_ATTR_VALUE_0;
        }

        if (attr_value->aclfield.enable) {
            /* Validate the object type for each object id present in the list */
            for (range_cnt = 0; range_cnt < range_info->count; range_cnt++) {
                if ((rc = sai_acl_validate_object_type(attribute_id,
                                range_info->list[range_cnt])) != SAI_STATUS_SUCCESS) {
                    SAI_ACL_LOG_ERR ("Invalid Object Type ");
                    return rc;
                }
            }

            range_list = (sai_object_id_t *)calloc(range_info->count,
                                                  sizeof(sai_object_id_t));

            if (range_list == NULL) {
                SAI_ACL_LOG_ERR ("System out of memory for range list");
                return SAI_STATUS_NO_MEMORY;
            }

            memcpy(range_list, range_info->list,
                   range_info->count * sizeof(sai_object_id_t));

            acl_filter->match_data.obj_list.count = range_info->count;
            acl_filter->match_data.obj_list.list = range_list;
            acl_filter->enable = true;
        }
    } else {

        range_info = &acl_filter->match_data.obj_list;

        if (attr_value->aclfield.data.objlist.count < range_info->count) {
            SAI_ACL_LOG_ERR ("Get Field range List failed, "
                             "Actual range = %d, Available range = %d",
                             range_info->count,
                             attr_value->aclfield.data.objlist.count);
            attr_value->aclfield.data.objlist.count = range_info->count;
            return SAI_STATUS_BUFFER_OVERFLOW;
        } else {
            attr_value->aclfield.data.objlist.count = range_info->count;
            memcpy (attr_value->aclfield.data.objlist.list, range_info->list,
                    (range_info->count * sizeof(sai_object_id_t)));
        }
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_rule_filter_populate_object_list(
                                    sai_attr_id_t attribute_id,
                                    sai_acl_filter_t *acl_filter,
                                    sai_attribute_value_t *attr_value,
                                    bool sai_acl_get_api)
{
    uint_t port_cnt = 0;
    sai_object_id_t *port_list  = NULL;
    sai_object_list_t *port_info = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /*
     * Check whether Object List belongs to Port List or range as ACL
     * expects only port list and range to be part of object list.
     */

    if(sai_acl_range_attr(attribute_id)) {
        sai_acl_rule_filter_populate_range(attribute_id,
                                           acl_filter, attr_value, sai_acl_get_api);
    }
    if (sai_acl_portlist_attr(attribute_id)) {
        /* Verify the operation being performed (CREATE OR GET) */
        if (!sai_acl_get_api) {
            /* CREATE OP */
            port_info = &attr_value->aclfield.data.objlist;

            if (((port_info->count == 0) || (port_info->list == NULL)) &&
                attr_value->aclfield.enable) {
                SAI_ACL_LOG_ERR ("Invalid port list");
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }

            if (attr_value->aclfield.enable) {
                /* Validate the object type for each object id present in the list */
                for (port_cnt = 0; port_cnt < port_info->count; port_cnt++) {
                     if ((rc = sai_acl_validate_object_type(attribute_id,
                                port_info->list[port_cnt])) != SAI_STATUS_SUCCESS) {
                         SAI_ACL_LOG_ERR ("Invalid Object Type ");
                         return rc;
                     }
                }

                for (port_cnt = 0; port_cnt < port_info->count; port_cnt++) {
                     if (!sai_is_port_valid(port_info->list[port_cnt])) {
                         SAI_ACL_LOG_ERR ("Invalid port number");
                         return SAI_STATUS_INVALID_OBJECT_ID;
                     }
                }

                port_list = (sai_object_id_t *)calloc(port_info->count,
                                                sizeof(sai_object_id_t));

                if (port_list == NULL) {
                    SAI_ACL_LOG_ERR ("System out of memory for port list");
                    return SAI_STATUS_NO_MEMORY;
                }

                memcpy(port_list, port_info->list,
                       port_info->count * sizeof(sai_object_id_t));

                acl_filter->match_data.obj_list.count = port_info->count;
                acl_filter->match_data.obj_list.list = port_list;
                acl_filter->enable = true;
            }
        } else {
            /* GET OP */
            port_info = &acl_filter->match_data.obj_list;

            if (attr_value->aclfield.data.objlist.count < port_info->count) {
                /* Port List cannot accomodate all the ports as the port
                 * count in Get API is less than existing ports.
                 *
                 * Set the correct port count and return error to the
                 * caller
                 */
                SAI_ACL_LOG_ERR ("Get Field Port List failed, "
                                 "Actual Port = %d, Available Port = %d",
                                 port_info->count,
                                 attr_value->aclfield.data.objlist.count);
                attr_value->aclfield.data.objlist.count = port_info->count;
                return SAI_STATUS_BUFFER_OVERFLOW;
            } else {
                attr_value->aclfield.data.objlist.count = port_info->count;
                memcpy (attr_value->aclfield.data.objlist.list, port_info->list,
                        (port_info->count * sizeof(sai_object_id_t)));
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_acl_rule_filter_populate_u8_list(
                                    sai_acl_filter_t *acl_filter,
                                    sai_attribute_value_t *attr_value,
                                    bool sai_acl_get_api)
{
    uint8_t  *u8_data_list = NULL, *u8_mask_list = NULL;
    sai_u8_list_t *u8_data_info = NULL, *u8_mask_info = NULL;

    /* Verify the operation being performed (CREATE OR GET) */
    if (!sai_acl_get_api) {
        /* CREATE OP */
        u8_data_info = &attr_value->aclfield.data.u8list;
        u8_mask_info = &attr_value->aclfield.mask.u8list;

        if (((u8_data_info->count == 0) || (u8_data_info->list == NULL) ||
             (u8_mask_info->count == 0) || (u8_mask_info->list == NULL)) &&
             attr_value->aclfield.enable) {
             SAI_ACL_LOG_ERR ("Invalid u8 list");
             return SAI_STATUS_INVALID_ATTR_VALUE_0;
        }

        if (attr_value->aclfield.enable) {

            u8_data_list = (uint8_t *)calloc(u8_data_info->count, sizeof(uint8_t));

            if (u8_data_list == NULL) {
                SAI_ACL_LOG_ERR ("System out of memory for u8 data list");
                return SAI_STATUS_NO_MEMORY;
            }

            u8_mask_list = (uint8_t *)calloc(u8_mask_info->count, sizeof(uint8_t));

            if (u8_mask_list == NULL) {
                SAI_ACL_LOG_ERR ("System out of memory for u8 mask list");
                free (u8_data_list);
                return SAI_STATUS_NO_MEMORY;
            }

            memcpy(u8_data_list, u8_data_info->list, u8_data_info->count * sizeof(uint8_t));
            memcpy(u8_mask_list, u8_mask_info->list, u8_mask_info->count * sizeof(uint8_t));

            acl_filter->match_data.u8_list.count = u8_data_info->count;
            acl_filter->match_data.u8_list.list = u8_data_list;
            acl_filter->match_mask.u8_list.count = u8_mask_info->count;
            acl_filter->match_mask.u8_list.list = u8_mask_list;
            acl_filter->enable = true;
        }
    } else {
        /* GET OP */
        u8_data_info = &acl_filter->match_data.u8_list;
        u8_mask_info = &acl_filter->match_mask.u8_list;

        if ((attr_value->aclfield.data.u8list.count < u8_data_info->count) ||
            (attr_value->aclfield.mask.u8list.count < u8_mask_info->count)) {
            /* Byte List cannot accomodate all the bytes as the byte
             * count in Get API is less than existing bytes.
             *
             * Set the correct byte count and return error to the
             * caller
             */
            SAI_ACL_LOG_ERR ("Get Field Byte List failed, "
                             "Actual Data Byte Count = %d, Available Data Byte Count = %d \n"
                             "Actual Mask Byte Count = %d, Available Mask Byte Count = %d",
                              u8_data_info->count, attr_value->aclfield.data.u8list.count,
                              u8_mask_info->count, attr_value->aclfield.mask.u8list.count);
            attr_value->aclfield.data.u8list.count = u8_data_info->count;
            attr_value->aclfield.mask.u8list.count = u8_mask_info->count;
            return SAI_STATUS_BUFFER_OVERFLOW;
        } else {
            attr_value->aclfield.data.u8list.count = u8_data_info->count;
            attr_value->aclfield.mask.u8list.count = u8_mask_info->count;
            memcpy (attr_value->aclfield.data.u8list.list, u8_data_info->list,
                   (u8_data_info->count * sizeof(uint8_t)));
            memcpy (attr_value->aclfield.mask.u8list.list, u8_mask_info->list,
                   (u8_mask_info->count * sizeof(uint8_t)));
        }
    }

    return SAI_STATUS_SUCCESS;
}

static inline void sai_acl_rule_filter_populate_attr_mac(
                                            sai_acl_filter_t *acl_filter,
                                            sai_attribute_value_t *attr_value,
                                            bool sai_acl_get_api)
{
    /* Create Operation */
    if (!sai_acl_get_api) {
        memcpy(acl_filter->match_data.mac,
               attr_value->aclfield.data.mac,
               sizeof(sai_mac_t));
        memcpy(acl_filter->match_mask.mac,
               attr_value->aclfield.mask.mac,
               sizeof(sai_mac_t));
    } else {
        /* Get Operation */
        memcpy(attr_value->aclfield.data.mac,
               acl_filter->match_data.mac, sizeof(sai_mac_t));
        memcpy(attr_value->aclfield.mask.mac,
               acl_filter->match_mask.mac, sizeof(sai_mac_t));
    }
}

static inline void sai_acl_rule_filter_populate_attr_ipv4(
                                            sai_acl_filter_t *acl_filter,
                                            sai_attribute_value_t *attr_value,
                                            bool sai_acl_get_api)
{
    /* Create Operation */
    if (!sai_acl_get_api) {
        memcpy((uint8_t *)&acl_filter->match_data.ip4,
               (uint8_t *)&attr_value->aclfield.data.ip4,
               sizeof(sai_ip4_t));
        memcpy((uint8_t *)&acl_filter->match_mask.ip4,
               (uint8_t *)&attr_value->aclfield.mask.ip4,
               sizeof(sai_ip4_t));
    } else {
        /* Get Operation */
        memcpy((uint8_t *)&attr_value->aclfield.data.ip4,
               (uint8_t *)&acl_filter->match_data.ip4,
               sizeof(sai_ip4_t));
        memcpy((uint8_t *)&attr_value->aclfield.mask.ip4,
               (uint8_t *)&acl_filter->match_mask.ip4,
               sizeof(sai_ip4_t));
    }
}

static inline void sai_acl_rule_filter_populate_attr_ipv6(
                                            sai_acl_filter_t *acl_filter,
                                            sai_attribute_value_t *attr_value,
                                            bool sai_acl_get_api)
{
    /* Create Operation */
    if (!sai_acl_get_api) {
        memcpy((uint8_t *)&acl_filter->match_data.ip6,
               (uint8_t *)&attr_value->aclfield.data.ip6,
               sizeof(sai_ip6_t));
        memcpy((uint8_t *)&acl_filter->match_mask.ip6,
               (uint8_t *)&attr_value->aclfield.mask.ip6,
               sizeof(sai_ip6_t));
    } else {
        /* Get Operation */
        memcpy((uint8_t *)&attr_value->aclfield.data.ip6,
               (uint8_t *)&acl_filter->match_data.ip6,
               sizeof(sai_ip6_t));
        memcpy((uint8_t *)&attr_value->aclfield.mask.ip6,
               (uint8_t *)&acl_filter->match_mask.ip6,
               sizeof(sai_ip6_t));
    }
}

static sai_status_t sai_acl_rule_filter_populate_attr_obj_id(
                                            sai_attr_id_t attribute_id,
                                            sai_acl_filter_t *acl_filter,
                                            sai_attribute_value_t *attr_value,
                                            bool sai_acl_get_api)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    sai_object_type_t object_type = SAI_OBJECT_TYPE_NULL;

    /* Create Operation */
    if (!sai_acl_get_api) {
        if (attr_value->aclfield.enable) {
            /* Validate the object type */
            if ((rc = sai_acl_validate_object_type(attribute_id,
                attr_value->aclfield.data.oid)) != SAI_STATUS_SUCCESS) {
                SAI_ACL_LOG_ERR ("Invalid Object Type ");
                return rc;
            }

            object_type = sai_uoid_obj_type_get(attr_value->aclfield.data.oid);

            switch (object_type) {
                case SAI_OBJECT_TYPE_LAG:
                    break;
                case SAI_OBJECT_TYPE_PORT:
                    if (sai_acl_port_attr(attribute_id)) {
                        if (!sai_is_port_valid(attr_value->aclfield.data.oid)) {
                        SAI_ACL_LOG_ERR ("Invalid port number");
                        return SAI_STATUS_INVALID_OBJECT_ID;
                        }
                    }
                    break;
                default:
                    if (!sai_is_obj_type_valid(object_type)) {
                        SAI_ACL_LOG_ERR ("Invalid Object Type %d in ACL Filter list",
                                         object_type);
                        rc = SAI_STATUS_INVALID_OBJECT_TYPE;
                    } else {
                        SAI_ACL_LOG_ERR ("Object Type %d not supported in ACL Filter list",
                                         object_type);
                        rc = SAI_STATUS_NOT_SUPPORTED;
                    }
                    return rc;
            }
        }
        acl_filter->match_data.oid = attr_value->aclfield.data.oid;
    } else {
        /* Get Operation */
        attr_value->aclfield.data.oid = acl_filter->match_data.oid;
    }

    return rc;
}

sai_status_t sai_acl_rule_udf_filter_populate (sai_acl_table_t *acl_table,
                                             sai_attr_id_t attribute_id,
                                             sai_acl_filter_t *acl_filter)
{
    sai_attr_id_t       group_attr_id = 0;
    sai_object_id_t     udf_group_id = SAI_NULL_OBJECT_ID;
    sai_npu_object_id_t udf_npu_id = 0;
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(acl_table != NULL);
    STD_ASSERT(acl_filter != NULL);

    /*
     * UDF attribute id passed in ACL Rule should be present in ACL Table.
     * ACL Rule requires UDF group id which will be retrieved from ACL Table.
     * Rule creation would fail if the UDF attribute id passed is not present
     * ACL Table.
     */
    group_attr_id = SAI_ACL_TABLE_ATTR_USER_DEFINED_FIELD_GROUP_MIN + (
                    attribute_id - SAI_ACL_ENTRY_ATTR_USER_DEFINED_FIELD_GROUP_MIN);

    if ((udf_group_id = sai_acl_table_find_udf_group_id(
        acl_table, group_attr_id)) == SAI_NULL_OBJECT_ID) {
        /* UDF attribute id not present in ACL Table */
        SAI_ACL_LOG_ERR ("ACL Rule UDF attribute id %d not present in ACL Table",
                         attribute_id);
        return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    acl_filter->udf_field.udf_group_id = udf_group_id;
    acl_filter->udf_field.udf_attr_index = attribute_id;

    if ((sai_rc = dn_sai_udf_group_hw_id_get(udf_group_id, &udf_npu_id))
        != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Failed to fetch NPU object id for UDF Group Id: "
                         "0x%"PRIx64"", udf_group_id);
        return sai_rc;
    }

    acl_filter->udf_field.udf_npu_id = udf_npu_id;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_rule_filter_populate_attr_type (sai_attr_id_t attribute_id,
                                             sai_acl_filter_t *acl_filter,
                                             sai_attribute_value_t *attr_value,
                                             bool sai_acl_get_api)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    SAI_ACL_LOG_TRACE ("ACL Rule attribute %d belongs to ACL field",
                        attribute_id);

    switch (sai_acl_rule_get_attr_type(attribute_id)) {
        case SAI_ACL_ENTRY_ATTR_BOOL:
            if (!sai_acl_get_api) {
                acl_filter->match_data.booldata = attr_value->aclfield.data.booldata;
            } else {
                attr_value->aclfield.data.booldata = acl_filter->match_data.booldata;
            }
            break;
        case SAI_ACL_ENTRY_ATTR_ONE_BYTE:
            if (!sai_acl_get_api) {
                acl_filter->match_data.u8 = attr_value->aclfield.data.u8;
                acl_filter->match_mask.u8 = attr_value->aclfield.mask.u8;
            } else {
                attr_value->aclfield.data.u8 = acl_filter->match_data.u8;
                attr_value->aclfield.mask.u8 = acl_filter->match_mask.u8;
            }
            break;
        case SAI_ACL_ENTRY_ATTR_TWO_BYTES:
            if (!sai_acl_get_api) {
                acl_filter->match_data.u16 = attr_value->aclfield.data.u16;
                acl_filter->match_mask.u16 = attr_value->aclfield.mask.u16;
            } else {
                attr_value->aclfield.data.u16 = acl_filter->match_data.u16;
                attr_value->aclfield.mask.u16 = acl_filter->match_mask.u16;
            }
            break;
        case SAI_ACL_ENTRY_ATTR_FOUR_BYTES:
            if (!sai_acl_get_api) {
                acl_filter->match_data.u32 = attr_value->aclfield.data.u32;
                acl_filter->match_mask.u32 = attr_value->aclfield.mask.u32;
            } else {
                attr_value->aclfield.data.u32 = acl_filter->match_data.u32;
                attr_value->aclfield.mask.u32 = acl_filter->match_mask.u32;
            }
            break;
        case SAI_ACL_ENTRY_ATTR_ENUM:
            if (!sai_acl_get_api) {
                acl_filter->match_data.s32 = attr_value->aclfield.data.s32;
                acl_filter->match_mask.s32 = attr_value->aclfield.mask.s32;
            } else {
                attr_value->aclfield.data.s32 = acl_filter->match_data.s32;
                attr_value->aclfield.mask.s32 = acl_filter->match_mask.s32;
            }
            break;
        case SAI_ACL_ENTRY_ATTR_MAC:
            sai_acl_rule_filter_populate_attr_mac(acl_filter, attr_value,
                                                  sai_acl_get_api);
            break;
        case SAI_ACL_ENTRY_ATTR_IPv4:
            sai_acl_rule_filter_populate_attr_ipv4(acl_filter, attr_value,
                                                   sai_acl_get_api);

            break;
        case SAI_ACL_ENTRY_ATTR_IPv6:
            sai_acl_rule_filter_populate_attr_ipv6(acl_filter, attr_value,
                                                   sai_acl_get_api);
            break;
        case SAI_ACL_ENTRY_ATTR_OBJECT_ID:
            rc = sai_acl_rule_filter_populate_attr_obj_id(attribute_id,
                               acl_filter, attr_value, sai_acl_get_api);
            break;
        case SAI_ACL_ENTRY_ATTR_OBJECT_LIST:
            rc = sai_acl_rule_filter_populate_object_list(attribute_id,
                               acl_filter, attr_value, sai_acl_get_api);
            break;
        case SAI_ACL_ENTRY_ATTR_ONE_BYTE_LIST:
            rc = sai_acl_rule_filter_populate_u8_list(acl_filter, attr_value,
                                                      sai_acl_get_api);
            break;
        default:
             /* SAI_ACL_ENTRY_ATTR_INVALID - Attribute Value not required */
            break;
    }

    return rc;
}

static sai_status_t sai_acl_rule_action_populate_object_list(
                                    sai_attr_id_t attribute_id,
                                    sai_acl_action_t *acl_action,
                                    sai_attribute_value_t *attr_value,
                                    bool sai_acl_get_api)
{
    uint_t obj_cnt = 0;
    sai_object_id_t *objlist  = NULL;
    sai_object_list_t *obj_info = NULL;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* Verify the operation being performed (CREATE OR GET) */
    if (!sai_acl_get_api) {
        /* CREATE OP */
        obj_info = &attr_value->aclaction.parameter.objlist;

        if (((obj_info->count == 0) || (obj_info->list == NULL)) &&
            acl_action->enable) {
            SAI_ACL_LOG_ERR ("Invalid object list action");
            return SAI_STATUS_INVALID_ATTR_VALUE_0;
        }

        if (acl_action->enable) {
            /* Validate the object type for each object id present in the list */
            for (obj_cnt = 0; obj_cnt < obj_info->count; obj_cnt++) {
                 if ((rc = sai_acl_validate_object_type(attribute_id,
                           obj_info->list[obj_cnt])) != SAI_STATUS_SUCCESS) {
                     SAI_ACL_LOG_ERR ("Invalid Object Type ");
                     return rc;
                 }
            }
            for (obj_cnt = 0; obj_cnt < obj_info->count; obj_cnt++) {

                 if ((attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS) ||
                     (attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS)) {

                     if (!sai_is_obj_id_mirror_session (obj_info->list[obj_cnt])) {
                         SAI_ACL_LOG_ERR ("0x%"PRIx64" is not a valid Mirror obj",
                                          obj_info->list[obj_cnt]);
                         return SAI_STATUS_INVALID_ATTR_VALUE_0;
                     }

                     if (!(sai_mirror_is_valid_mirror_session (
                          (sai_object_id_t)obj_info->list[obj_cnt]))) {
                         SAI_ACL_LOG_ERR ("Invalid mirror attribute");
                         return SAI_STATUS_INVALID_ATTR_VALUE_0;
                     }
                 } else if (attribute_id == SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT_LIST) {

                     if (!sai_is_port_valid(obj_info->list[obj_cnt])) {
                         SAI_ACL_LOG_ERR ("Invalid port number");
                         return SAI_STATUS_INVALID_ATTR_VALUE_0;
                     }
                 }

            }

            objlist = (sai_object_id_t *)calloc(obj_info->count,
                                            sizeof(sai_object_id_t));

            if (objlist == NULL) {
                SAI_ACL_LOG_ERR ("System out of memory for object list");
                return SAI_STATUS_NO_MEMORY;
            }

            memcpy(objlist, obj_info->list,
                       obj_info->count * sizeof(sai_object_id_t));

            acl_action->parameter.obj_list.count = obj_info->count;
            acl_action->parameter.obj_list.list = objlist;
        }
    } else {
        /* GET OP */
        obj_info = &acl_action->parameter.obj_list;

        if (attr_value->aclaction.parameter.objlist.count < obj_info->count) {
            /* Object List cannot accomodate all the objects as the object
             * count in Get API is less than existing ports.
             *
             * Set the correct object count and return error to the
             * caller
             */
            SAI_ACL_LOG_ERR ("Get Action Object List failed, "
                             "Actual Object = %d, Available Object = %d",
                             obj_info->count,
                             attr_value->aclaction.parameter.objlist.count);
            attr_value->aclaction.parameter.objlist.count = obj_info->count;
            return SAI_STATUS_BUFFER_OVERFLOW;
        } else {
            attr_value->aclaction.parameter.objlist.count = obj_info->count;
            memcpy (attr_value->aclaction.parameter.objlist.list, obj_info->list,
                    (obj_info->count * sizeof(sai_object_id_t)));
        }
    }

    return SAI_STATUS_SUCCESS;
}

static inline sai_status_t sai_acl_rule_action_populate_attr_obj_id (
                                            sai_attr_id_t attribute_id,
                                            sai_acl_action_t *acl_action,
                                            sai_attribute_value_t *attr_value,
                                            bool sai_acl_get_api)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* Create Operation */
    if (!sai_acl_get_api) {
        if (attr_value->aclaction.enable) {
            /* Validate the object type */
            if ((rc = sai_acl_validate_object_type(attribute_id,
                        attr_value->aclaction.parameter.oid)) != SAI_STATUS_SUCCESS) {
                SAI_ACL_LOG_ERR ("Invalid Object Type ");
                return rc;
            }

            if (sai_acl_port_attr(attribute_id)) {
                if (!sai_is_port_valid(attr_value->aclaction.parameter.oid)) {
                    SAI_ACL_LOG_ERR ("Invalid port number");
                    return SAI_STATUS_INVALID_OBJECT_ID;
                }
            }
        }

        acl_action->parameter.oid = attr_value->aclaction.parameter.oid;
    } else {
        /* Get Operation */
        attr_value->aclaction.parameter.oid = acl_action->parameter.oid;
    }

    return rc;
}

sai_status_t sai_acl_rule_action_populate_attr_type (sai_attr_id_t attribute_id,
                                             sai_acl_action_t *acl_action,
                                             sai_attribute_value_t *attr_value,
                                             bool sai_acl_get_api)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    SAI_ACL_LOG_TRACE ("ACL Rule attribute %d belongs to ACL Action",
                        attribute_id);

    switch (sai_acl_rule_get_attr_type(attribute_id)) {
        case SAI_ACL_ENTRY_ATTR_ONE_BYTE:
            if (!sai_acl_get_api) {
                acl_action->parameter.u8 = attr_value->aclaction.parameter.u8;
            } else {
                attr_value->aclaction.parameter.u8 = acl_action->parameter.u8;
            }
            break;
        case SAI_ACL_ENTRY_ATTR_TWO_BYTES:
            if (!sai_acl_get_api) {
                acl_action->parameter.u16 = attr_value->aclaction.parameter.u16;
            } else {
                attr_value->aclaction.parameter.u16 = acl_action->parameter.u16;
            }
            break;
        case SAI_ACL_ENTRY_ATTR_FOUR_BYTES:
            if (!sai_acl_get_api) {
                acl_action->parameter.u32 = attr_value->aclaction.parameter.u32;
            } else {
                attr_value->aclaction.parameter.u32 = acl_action->parameter.u32;
            }
            break;
        case SAI_ACL_ENTRY_ATTR_ENUM:
            /*
             * Packet Action case which uses enum sai_packet_action_t
             */
            if (!sai_acl_get_api) {
                acl_action->parameter.s32 = attr_value->aclaction.parameter.s32;
            } else {
                attr_value->aclaction.parameter.s32 = acl_action->parameter.s32;
            }
            break;
        case SAI_ACL_ENTRY_ATTR_MAC:
            if (!sai_acl_get_api) {
                memcpy(acl_action->parameter.mac,
                       attr_value->aclaction.parameter.mac,
                       sizeof(sai_mac_t));
            } else {
                 memcpy(attr_value->aclaction.parameter.mac,
                        acl_action->parameter.mac,
                        sizeof(sai_mac_t));
            }
            break;
        case SAI_ACL_ENTRY_ATTR_IPv4:
            if (!sai_acl_get_api) {
                memcpy((uint8_t *)&acl_action->parameter.ip4,
                       (uint8_t *)&attr_value->aclaction.parameter.ip4,
                       sizeof(sai_ip4_t));
            } else {
                memcpy((uint8_t *)&attr_value->aclaction.parameter.ip4,
                       (uint8_t *)&acl_action->parameter.ip4,
                       sizeof(sai_ip4_t));
            }
            break;
        case SAI_ACL_ENTRY_ATTR_IPv6:
            if (!sai_acl_get_api) {
                memcpy((uint8_t *)&acl_action->parameter.ip6,
                       (uint8_t *)&attr_value->aclaction.parameter.ip6,
                       sizeof(sai_ip6_t));
            } else {
                memcpy((uint8_t *)&attr_value->aclaction.parameter.ip6,
                       (uint8_t *)&acl_action->parameter.ip6,
                       sizeof(sai_ip6_t));
            }
            break;
        case SAI_ACL_ENTRY_ATTR_OBJECT_ID:
            rc = sai_acl_rule_action_populate_attr_obj_id(attribute_id,
                                    acl_action, attr_value, sai_acl_get_api);
            break;
        case SAI_ACL_ENTRY_ATTR_OBJECT_LIST:
            rc = sai_acl_rule_action_populate_object_list(attribute_id,
                                    acl_action, attr_value, sai_acl_get_api);
            break;
        default:
             /* SAI_ACL_ENTRY_ATTR_INVALID - Attribute Value not required */
            break;
    }

    return rc;
}

sai_status_t sai_acl_rule_create_samplepacket(sai_acl_rule_t *acl_rule)
{
    uint_t action_index = 0;
    uint_t filter_index = 0;
    bool         is_sample_enable = false;
    bool         port_set = false;
    sai_object_id_t sample_object = SAI_NULL_OBJECT_ID;
    sai_status_t    rc = SAI_STATUS_SUCCESS;
    sai_samplepacket_direction_t sample_direction = SAI_SAMPLEPACKET_DIR_MAX;
    sai_object_list_t portlist;

    memset (&portlist, 0, sizeof(portlist));

    for (action_index = 0; action_index < acl_rule->action_count; action_index++)
    {
        port_set = false;
        if ((acl_rule->action_list[action_index].action ==
                    SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE) ||
                (acl_rule->action_list[action_index].action ==
                 SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE)) {

            /* ACL Rule has Sample Packet set as an action */
            sample_object = acl_rule->action_list[action_index].parameter.oid;
            /* Set the direction */
            sample_direction = (acl_rule->action_list[action_index].action ==
                    SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE)
                ? SAI_SAMPLEPACKET_DIR_INGRESS :
                (acl_rule->action_list[action_index].action ==
                 SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE)
                ? SAI_SAMPLEPACKET_DIR_EGRESS : SAI_SAMPLEPACKET_DIR_MAX;

            if (sample_direction == SAI_SAMPLEPACKET_DIR_MAX) {
                return SAI_STATUS_FAILURE;
            }
            is_sample_enable = acl_rule->action_list[action_index].enable;
        }

        if (is_sample_enable) {
            if (sample_direction == SAI_SAMPLEPACKET_DIR_INGRESS) {
                for (filter_index = 0; filter_index < acl_rule->filter_count; filter_index++)
                {
                    if (sai_acl_portlist_attr(acl_rule->filter_list[filter_index].field)) {
                        rc = sai_samplepacket_validate_object (
                                &acl_rule->filter_list[filter_index].match_data.obj_list,
                                sample_object, sample_direction, true, true);

                        port_set = true;
                        break;
                    } else if (sai_acl_port_attr(acl_rule->filter_list[filter_index].field)) {
                        portlist.count = 1;
                        portlist.list = &acl_rule->filter_list[filter_index].match_data.oid;
                        rc = sai_samplepacket_validate_object (
                                &portlist,
                                sample_object, sample_direction, true, true);

                        port_set = true;
                        break;
                    }
                }
            }

            if (!(port_set)) {
                rc = sai_samplepacket_validate_object ((sai_object_list_t *)NULL,
                        sample_object, sample_direction, true, true);
            }
        }
    }

    return rc;
}

sai_status_t sai_acl_rule_remove_samplepacket (sai_acl_rule_t *acl_rule)
{
    bool   port_set = false;
    uint_t action_index = 0;
    uint_t filter_index = 0;
    sai_object_id_t sample_object = SAI_NULL_OBJECT_ID;
    sai_samplepacket_direction_t sample_direction = SAI_SAMPLEPACKET_DIR_MAX;
    sai_object_list_t portlist;

    memset (&portlist, 0, sizeof(portlist));

    for (action_index = 0; action_index < acl_rule->action_count; action_index++)
    {
        port_set = false;
        if ((acl_rule->action_list[action_index].action ==
            SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE) ||
            (acl_rule->action_list[action_index].action ==
             SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE)) {
            /* ACL Rule has Sample Packet set as an action */
            sample_object = acl_rule->action_list[action_index].parameter.oid;
            /* Set the direction */
            sample_direction = (acl_rule->action_list[action_index].action ==
                    SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE)
                ? SAI_SAMPLEPACKET_DIR_INGRESS :
                (acl_rule->action_list[action_index].action ==
                 SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE)
                ? SAI_SAMPLEPACKET_DIR_EGRESS : SAI_SAMPLEPACKET_DIR_MAX;

            if (sample_direction == SAI_SAMPLEPACKET_DIR_MAX) {
                return SAI_STATUS_FAILURE;
            }

            if (sample_direction == SAI_SAMPLEPACKET_DIR_INGRESS) {
                for (filter_index = 0; filter_index < acl_rule->filter_count; filter_index++)
                {
                    if (sai_acl_portlist_attr(acl_rule->filter_list[filter_index].field)) {
                        sai_samplepacket_remove_object (
                                &acl_rule->filter_list[filter_index].match_data.obj_list,
                                sample_object, sample_direction);
                        port_set = true;
                        break;
                    } else if (sai_acl_port_attr(acl_rule->filter_list[filter_index].field)) {
                        portlist.count = 1;
                        portlist.list = &acl_rule->filter_list[filter_index].match_data.oid;
                        sai_samplepacket_remove_object (
                                &portlist,
                                sample_object, sample_direction);

                        port_set = true;
                        break;
                    }
                }
            }
            if (!(port_set)) {
                sai_samplepacket_remove_object ((sai_object_list_t *)NULL,
                        sample_object, sample_direction);
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_acl_rule_validate_update_samplepacket (sai_acl_rule_t *acl_rule,
                                                               sai_acl_rule_t *orig_acl_rule,
                                                               bool validate,
                                                               bool update)
{
    uint_t action_index = 0;
    uint_t filter_index = 0;
    bool         is_sample_enable = false;
    bool         is_sample_set = false;
    bool         port_set = false;
    bool         new_port_set = false;
    sai_object_id_t sample_object = SAI_NULL_OBJECT_ID;
    sai_status_t    rc = SAI_STATUS_SUCCESS;
    sai_samplepacket_direction_t sample_direction = SAI_SAMPLEPACKET_DIR_MAX;
    sai_object_list_t portlist;

    memset (&portlist, 0, sizeof(portlist));

    /* Walk through the new/changed action as part of set and check whether it is samplepacket
     * action, if yes get the portlist from the original acl rule (new set can have only one attribute,
     * since that attribute is already a samplepacket action, portlist can be retrieved from original
     * acl rule only) and associate it to the samplepacket session
     */
    for (action_index = 0; action_index < acl_rule->action_count; action_index++)
    {
        if ((acl_rule->action_list[action_index].action ==
                    SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE) ||
                (acl_rule->action_list[action_index].action ==
                 SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE)) {

            is_sample_set = true;
            /* ACL Rule has Sample Packet set as an action */
            sample_object = acl_rule->action_list[action_index].parameter.oid;
            /* Set the direction */
            sample_direction = (acl_rule->action_list[action_index].action ==
                    SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE)
                ? SAI_SAMPLEPACKET_DIR_INGRESS :
                (acl_rule->action_list[action_index].action ==
                 SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE)
                ? SAI_SAMPLEPACKET_DIR_EGRESS : SAI_SAMPLEPACKET_DIR_MAX;

            if (sample_direction == SAI_SAMPLEPACKET_DIR_MAX) {
                return SAI_STATUS_FAILURE;
            }
            is_sample_enable = acl_rule->action_list[action_index].enable;
        }
    }

    if (is_sample_set) {
        if (sample_direction == SAI_SAMPLEPACKET_DIR_INGRESS) {

            for (filter_index = 0; filter_index < orig_acl_rule->filter_count; filter_index++)
            {
                if (sai_acl_portlist_attr(orig_acl_rule->filter_list[filter_index].field)) {
                    /* Remove the previous samplepacket configuration for the changed action
                     * alone (INGRESS or EGRESS) before validation for it to be successful */
                    if (validate) {
                        rc = sai_samplepacket_remove_object (
                                &orig_acl_rule->filter_list[filter_index].match_data.obj_list,
                                orig_acl_rule->samplepacket_id[sample_direction], sample_direction);
                    }
                    if (is_sample_enable) {
                        rc = sai_samplepacket_validate_object (
                                &orig_acl_rule->filter_list[filter_index].match_data.obj_list,
                                sample_object, sample_direction, validate, update);
                        if ((validate) && (rc != SAI_STATUS_SUCCESS)) {
                                sai_samplepacket_validate_object (
                                &orig_acl_rule->filter_list[filter_index].match_data.obj_list,
                                orig_acl_rule->samplepacket_id[sample_direction], sample_direction, true, true);

                        }

                        port_set = true;
                        break;
                    }
                }
            }
        }

        if (!(port_set)) {
            if (validate) {
                rc = sai_samplepacket_remove_object (
                        (sai_object_list_t *)NULL,
                        orig_acl_rule->samplepacket_id[sample_direction] , sample_direction);
            }
            if (is_sample_enable) {
                rc = sai_samplepacket_validate_object ((sai_object_list_t *)NULL,
                        sample_object, sample_direction, validate, update);
            }
        }
    } else {

    /* If the new set is not samplepacket action, check whether the new/changed attribute is
     * inports/inport field. If yes again get the samplepacket action from the original acl rule
     * and associate it with the new set of ports
     */
        for (filter_index = 0; filter_index < acl_rule->filter_count; filter_index++)
        {
            if (sai_acl_portlist_attr(acl_rule->filter_list[filter_index].field)) {
                new_port_set = true;
                break;
            }
        }

        if (new_port_set) {
            for (action_index = 0; action_index < orig_acl_rule->action_count; action_index++)
            {
                /* If there is a change in IN_PORTS qualifier ingress samplepacket should alone
                 * be changed
                 */
                if (orig_acl_rule->action_list[action_index].action ==
                        SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE) {

                    is_sample_set = true;
                    /* ACL Rule has Sample Packet set as an action */
                    sample_object = orig_acl_rule->action_list[action_index].parameter.oid;
                    /* Set the direction */
                    sample_direction = (orig_acl_rule->action_list[action_index].action ==
                            SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE)
                        ? SAI_SAMPLEPACKET_DIR_INGRESS :
                        (orig_acl_rule->action_list[action_index].action ==
                         SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE)
                        ? SAI_SAMPLEPACKET_DIR_EGRESS : SAI_SAMPLEPACKET_DIR_MAX;

                    if (sample_direction == SAI_SAMPLEPACKET_DIR_MAX) {
                        return SAI_STATUS_FAILURE;
                    }
                    is_sample_enable = orig_acl_rule->action_list[action_index].enable;
                }

                if (is_sample_set) {

                    if (sai_acl_portlist_attr(acl_rule->filter_list[filter_index].field)) {

                        if (validate) {
                            rc = sai_samplepacket_remove_object (
                                    &orig_acl_rule->filter_list[filter_index].match_data.obj_list,
                                    orig_acl_rule->samplepacket_id[sample_direction], sample_direction);
                            if (rc != SAI_STATUS_SUCCESS) {
                                return rc;
                            }
                        }
                        if (is_sample_enable) {
                            rc = sai_samplepacket_validate_object (
                                    &acl_rule->filter_list[filter_index].match_data.obj_list,
                                    sample_object, sample_direction, validate, update);
                            if ((validate) && (rc != SAI_STATUS_SUCCESS)) {
                                    sai_samplepacket_validate_object (
                                        &orig_acl_rule->filter_list[filter_index].match_data.obj_list,
                                        orig_acl_rule->samplepacket_id[sample_direction], sample_direction, true, true);

                            }
                        }

                        break;

                    }
                }

            }

        }
    }

    return rc;
}
