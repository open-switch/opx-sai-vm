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
 * @file sai_vm_acl_rule.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI ACL Rule object in VM environment.
 */

#include "sai_vm_defs.h"
#include "sai_acl_db_api.h"
#include "sai_oid_utils.h"
#include "sai_acl_npu_api.h"
#include "sai_acl_type_defs.h"
#include "sai_acl_utils.h"
#include "saiacl.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_bit_masks.h"
#include "std_assert.h"
#include "std_type_defs.h"
#include <inttypes.h>

static bool sai_is_acl_rule_fields_in_table (sai_acl_table_t *acl_table,
                                             sai_acl_rule_t *acl_rule,
                                             bool isCreate)
{
    bool   is_found = true;
    uint_t filter_idx = 0;
    uint_t tbl_fld_idx = 0;
    uint_t rule_filter_map_idx = 0;
    uint_t loop_idx = 0;
    uint_t rule_filter_to_tbl_field_map [] = {
        SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6, SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6,
        SAI_ACL_TABLE_ATTR_FIELD_INNER_SRC_IPV6, SAI_ACL_TABLE_ATTR_FIELD_INNER_DST_IPV6,
        SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC, SAI_ACL_TABLE_ATTR_FIELD_DST_MAC,
        SAI_ACL_TABLE_ATTR_FIELD_SRC_IP, SAI_ACL_TABLE_ATTR_FIELD_DST_IP,
        SAI_ACL_TABLE_ATTR_FIELD_INNER_SRC_IP,SAI_ACL_TABLE_ATTR_FIELD_INNER_DST_IP,
        SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS, SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS,
        SAI_ACL_TABLE_ATTR_FIELD_IN_PORT, SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT,
        SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT,
        SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID,
        SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI,
        SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI,
        SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID,
        SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI,
        SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI,
        SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
        SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT,
        SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
        SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL,
        SAI_ACL_TABLE_ATTR_FIELD_IP_IDENTIFICATION,
        SAI_ACL_TABLE_ATTR_FIELD_DSCP, SAI_ACL_TABLE_ATTR_FIELD_ECN,
        SAI_ACL_TABLE_ATTR_FIELD_TTL,
        SAI_ACL_TABLE_ATTR_FIELD_TOS, SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS,
        SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS, SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE,
        SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG,
        SAI_ACL_TABLE_ATTR_FIELD_IPV6_FLOW_LABEL, SAI_ACL_TABLE_ATTR_FIELD_TC,
        SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE, SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE,
        SAI_ACL_TABLE_ATTR_FIELD_PACKET_VLAN,
        SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_USER_META,
        SAI_ACL_TABLE_ATTR_FIELD_ROUTE_DST_USER_META,
        SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_USER_META,
        SAI_ACL_TABLE_ATTR_FIELD_PORT_USER_META,
        SAI_ACL_TABLE_ATTR_FIELD_VLAN_USER_META,
        SAI_ACL_TABLE_ATTR_FIELD_ACL_USER_META,
        SAI_ACL_TABLE_ATTR_FIELD_FDB_NPU_META_DST_HIT,
        SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT,
        SAI_ACL_TABLE_ATTR_FIELD_ROUTE_NPU_META_DST_HIT,
        SAI_ACL_TABLE_ATTR_FIELD_ACL_RANGE_TYPE,
        SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST,
        SAI_ACL_TABLE_ATTR_FIELD_IPV6_NEXT_HEADER,
        SAI_ACL_TABLE_ATTR_FIELD_BRIDGE_TYPE,
    };

    uint_t table_size = sizeof(rule_filter_to_tbl_field_map)/
        sizeof(rule_filter_to_tbl_field_map[0]);


    STD_ASSERT (acl_table != NULL);
    STD_ASSERT (acl_rule != NULL);

    for (filter_idx = 0; filter_idx < acl_rule->filter_count; filter_idx++) {
        /* Check for only the newly added qualifiers in case of the set API. */
        if ((!isCreate) && (!acl_rule->filter_list [filter_idx].new_field)) {
            continue;
        }

        is_found = false;

        SAI_ACL_LOG_TRACE("Field is %x", acl_rule->filter_list [filter_idx].field);

        /**
         * The DST_PORT qualifier is not part of saiacl qualifier list. Its
         * added internally in sai-common-utils. Hence not adding it part of
         * the filter list above. Converting it back to OUT_PORT for VM alone.
         * */
        if(acl_rule->filter_list [filter_idx].field == SAI_ACL_ENTRY_ATTR_FIELD_DST_PORT){
            acl_rule->filter_list [filter_idx].field = SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT;
        }
        SAI_ACL_LOG_TRACE("Field is %x", acl_rule->filter_list [filter_idx].field);

        /*
         * Get the rule_to_tbl_fields_map [] array idx for the rule's
         * field attribute id.
         */
        rule_filter_map_idx = acl_rule->filter_list [filter_idx].field -
            SAI_ACL_ENTRY_ATTR_FIELD_START;

        SAI_ACL_LOG_TRACE("rule_filter_map_idx is %d",rule_filter_map_idx);

        for (tbl_fld_idx = 0; tbl_fld_idx < acl_table->field_count;
             tbl_fld_idx++) {
            for(loop_idx = 0; loop_idx < table_size;
                loop_idx ++) {
                if (acl_table->field_list [tbl_fld_idx] == rule_filter_to_tbl_field_map[loop_idx]) {
                    is_found = true;
                    break;
                }
            }
        }

        if (!is_found) {
            SAI_ACL_LOG_ERR ("Rule Filter %d is not present in table Obj "
                             "ID: 0x%"PRIx64".",
                             acl_rule->filter_list [filter_idx].field,
                             acl_table->table_key.acl_table_id);

            break;
        }
    }

    return is_found;
}

static inline uint_t sai_vm_acl_bmp_idx_to_entry_id_get (uint_t table_id,
                                                         uint_t bmp_idx)
{
    return ((SAI_VM_ACL_TABLE_MAX_ENTRIES * table_id) + bmp_idx);
}

static inline uint_t sai_vm_acl_entry_id_to_bmp_idx_get (uint_t entry_id)
{
    return (entry_id % SAI_VM_ACL_TABLE_MAX_ENTRIES);
}

sai_status_t sai_npu_create_acl_rule (sai_acl_table_t *acl_table,
                                      sai_acl_rule_t *acl_rule)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    sai_npu_object_id_t table_id = 0;
    bool                is_fields_in_tbl = false;

    SAI_ACL_LOG_TRACE ("NPU ACL Rule Creation API.");

    STD_ASSERT (acl_table != NULL);
    STD_ASSERT (acl_rule != NULL);

    is_fields_in_tbl = sai_is_acl_rule_fields_in_table (acl_table, acl_rule,
                                                        true /* isCreate */);

    if (is_fields_in_tbl == false) {
        SAI_ACL_LOG_ERR ("All Rule filters not present in table Obj ID: "
                         "0x%"PRIx64".", acl_table->table_key.acl_table_id);

        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    table_id = sai_uoid_npu_obj_id_get (acl_table->table_key.acl_table_id);

    /* Insert ACL Entry object record to DB. */
    sai_rc = sai_acl_rule_create_db_entry (acl_rule);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Error inserting entry to DB for ACL Entry: 0x%"PRIx64", "
                         "OBJ ID: %"PRIx64".", sai_uoid_npu_obj_id_get(acl_rule->rule_key.acl_id)
                         , acl_rule->rule_key.acl_id);

        return sai_rc;
    }

    SAI_ACL_LOG_TRACE ("ACL Entry Creation success, Entry ID: 0x%"PRIx64", Obj Id: "
                       " 0x%"PRIx64" on Table Id: 0x%"PRIx64", Obj Id: 0x%"PRIx64".",
                       sai_uoid_npu_obj_id_get(acl_rule->rule_key.acl_id),
                       acl_rule->rule_key.acl_id, table_id,
                       acl_table->table_key.acl_table_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_delete_acl_rule (sai_acl_table_t *acl_table,
                                      sai_acl_rule_t *acl_rule)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    sai_npu_object_id_t table_id = 0;
    sai_npu_object_id_t entry_id = 0;

    SAI_ACL_LOG_TRACE ("NPU ACL Rule deletion API.");

    STD_ASSERT (acl_table != NULL);
    STD_ASSERT (acl_rule != NULL);

    table_id = sai_uoid_npu_obj_id_get (acl_table->table_key.acl_table_id);
    entry_id = sai_uoid_npu_obj_id_get (acl_rule->rule_key.acl_id);

    /* Remove ACL Entry object record from DB. */
    sai_rc = sai_acl_rule_delete_db_entry (acl_rule);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Error removing entry from DB for ACL entry 0x%"PRIx64", "
                         "Object ID: 0x%" PRIx64 ".", entry_id,
                         acl_rule->rule_key.acl_id);

        return sai_rc;
    }

    SAI_ACL_LOG_TRACE ("ACL Entry deletion success, Entry ID: 0x%"PRIx64", Obj Id: "
                       " 0x%"PRIx64" from Table Id: 0x%"PRIx64" Obj Id: 0x%"PRIx64".",
                       entry_id, acl_rule->rule_key.acl_id, table_id,
                       acl_table->table_key.acl_table_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_set_acl_rule (sai_acl_table_t *acl_table,
                                   sai_acl_rule_t *set_rule,
                                   sai_acl_rule_t *compare_rule,
                                   sai_acl_rule_t *given_rule)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    bool         is_fields_in_tbl = false;

    STD_ASSERT (acl_table != NULL);
    STD_ASSERT (set_rule != NULL);
    STD_ASSERT (compare_rule != NULL);
    STD_ASSERT (given_rule != NULL);

    SAI_ACL_LOG_TRACE ("NPU ACL Rule Set API.");

    is_fields_in_tbl = sai_is_acl_rule_fields_in_table (acl_table, set_rule,
                                                        false);

    if (is_fields_in_tbl == false) {
        SAI_ACL_LOG_ERR ("All Rule filters not present in table Obj ID: "
                         "0x%"PRIx64".", acl_table->table_key.acl_table_id);

        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /*
     * TODO Check for counter id is existing in the counter tree
     * maintained on the acl table node.
     *
     * Currently sai_acl_get_acl_node, sai_acl_table_find and sai_acl_cntr_find
     * are defined in sai-common and hence not accessible from sai-bcm/sai-vm.
     * To move these util functions to sai-common-utils.
     */

    /* Update ACL Entry object record in DB. */
    sai_rc = sai_acl_rule_set_db_entry (set_rule, given_rule);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Error updating entry to DB for ACL Entry Object "
                         "ID: %"PRIx64".", set_rule->rule_key.acl_id);

        return sai_rc;
    }

    SAI_ACL_LOG_TRACE ("ACL Entry set API successful for ACL Entry Object "
                       "ID: %"PRIx64".", set_rule->rule_key.acl_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_copy_acl_rule (sai_acl_rule_t *src_rule,
                                    sai_acl_rule_t *dst_rule)
{
    STD_ASSERT (src_rule != NULL);
    STD_ASSERT (dst_rule != NULL);

    SAI_ACL_LOG_TRACE ("ACL Entry copy API.");

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_free_acl_rule (sai_acl_rule_t *acl_rule)
{
    STD_ASSERT (acl_rule != NULL);

    return SAI_STATUS_SUCCESS;
}

void sai_npu_acl_dump_rule(const sai_acl_rule_t *acl_rule)
{
    STD_ASSERT (acl_rule != NULL);

    return;
}

