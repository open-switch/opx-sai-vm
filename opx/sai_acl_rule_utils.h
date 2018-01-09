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
 * * @file sai_acl_rule_utils.h
 * *
 * * @brief This file contains prototype declarations for utility functions
            used in ACL rule operations.
 * *
 * *************************************************************************/
#ifndef _SAI_ACL_RULE_UTILS_H
#define _SAI_ACL_RULE_UTILS_H

#include "sai_acl_type_defs.h"
#include "sai_samplepacket_defs.h"
#include "saistatus.h"
#include "std_type_defs.h"
#include "std_rbtree.h"

static inline sai_samplepacket_direction_t sai_acl_get_sample_direction(sai_attr_id_t action) {
    return (action ==
            SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE)
        ? SAI_SAMPLEPACKET_DIR_INGRESS :
        (action ==
         SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE)
        ? SAI_SAMPLEPACKET_DIR_EGRESS : SAI_SAMPLEPACKET_DIR_MAX;
}

/**************************************************************************
 *                     Function Prototypes
 **************************************************************************/

bool sai_acl_portlist_attr(sai_acl_entry_attr_t entry_attr);

bool sai_acl_object_list_field_attr(sai_acl_entry_attr_t entry_attr);
bool sai_acl_object_list_action_attr(sai_acl_entry_attr_t entry_attr);

sai_object_id_t sai_acl_table_find_udf_group_id(sai_acl_table_t *acl_table,
                                                sai_attr_id_t attribute_id);

bool sai_acl_port_attr(sai_acl_entry_attr_t entry_attr);

sai_acl_rule_t *sai_acl_rule_validate(sai_acl_table_t *acl_table,
                                      sai_object_id_t acl_id);

void sai_acl_rule_link(sai_acl_table_t *acl_table,
                       sai_acl_rule_t *acl_rule);

void sai_acl_rule_unlink(sai_acl_table_t *acl_table,
                         sai_acl_rule_t *acl_rule);

sai_status_t sai_acl_rule_insert(rbtree_handle sai_acl_rule_tree,
                                 sai_acl_rule_t *acl_rule);

sai_acl_rule_t *sai_acl_rule_remove(rbtree_handle sai_acl_rule_tree,
                                    sai_acl_rule_t *acl_rule);

sai_acl_rule_t *sai_acl_rule_find(rbtree_handle sai_acl_rule_tree,
                                  sai_object_id_t acl_id);

sai_status_t sai_acl_rule_copy_filter(sai_acl_filter_t *dst_filter,
                              sai_acl_filter_t *src_filter);

sai_status_t sai_acl_rule_copy_action(sai_acl_action_t *dst_action,
                              sai_acl_action_t *src_action);

void sai_acl_rule_copy_fields(sai_acl_rule_t *dst_rule,
                              sai_acl_rule_t *src_rule);

sai_status_t sai_acl_validate_create_rule(sai_acl_table_t *acl_table,
                                          sai_acl_rule_t *acl_rule);

sai_status_t sai_acl_validate_set_rule(sai_acl_rule_t *modify_rule,
                                       sai_acl_rule_t *given_rule);

void sai_acl_rule_set_field_action(sai_acl_rule_t *rule_scan,
                                   sai_acl_rule_t *given_rule,
                                   uint_t *new_field_cnt,
                                   uint_t *new_action_cnt);

sai_status_t sai_acl_rule_update_field_action(sai_acl_rule_t *rule_scan,
                                              sai_acl_rule_t *given_rule,
                                              uint_t new_fields,
                                              uint_t new_actions);

sai_status_t sai_acl_rule_udf_filter_populate(sai_acl_table_t *acl_table,
                                              sai_attr_id_t attribute_id,
                                              sai_acl_filter_t *acl_filter);

sai_status_t sai_acl_rule_filter_populate_attr_type (sai_attr_id_t attribute_id,
                                             sai_acl_filter_t *acl_filter,
                                             sai_attribute_value_t *attr_value,
                                             bool sai_acl_get_api);

sai_status_t sai_acl_rule_action_populate_attr_type (sai_attr_id_t attribute_id,
                                             sai_acl_action_t *acl_action,
                                             sai_attribute_value_t *attr_value,
                                             bool sai_acl_get_api);

sai_status_t sai_acl_rule_create_samplepacket (sai_acl_rule_t *acl_rule);

sai_status_t sai_acl_rule_remove_samplepacket (sai_acl_rule_t *acl_rule);

sai_status_t sai_acl_rule_validate_update_samplepacket (sai_acl_rule_t *acl_rule,
                                                               sai_acl_rule_t *orig_rule,
                                                               bool validate,
                                                               bool update);
#endif
