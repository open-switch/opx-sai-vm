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
 * @file sai_vm_acl_util.h
 *
 * @brief This file contains the function prototypes for SAI acl functions
 *        in VM environment.
 */

#ifndef __SAI_VM_ACL_UTIL_H__
#define __SAI_VM_ACL_UTIL_H__

#include "saitypes.h"
#include "sai_acl_type_defs.h"
#include "saiacl.h"

sai_status_t sai_npu_create_acl_rule(sai_acl_table_t *acl_table,
                                     sai_acl_rule_t *acl_rule);
sai_status_t sai_npu_delete_acl_rule(sai_acl_table_t *acl_table,
                                     sai_acl_rule_t *acl_rule);
sai_status_t sai_npu_free_acl_rule(sai_acl_rule_t *acl_rule);
sai_status_t sai_npu_copy_acl_rule(sai_acl_rule_t *dst_rule,
                                    sai_acl_rule_t *src_rule);
sai_status_t sai_npu_set_acl_rule(sai_acl_table_t *acl_table,
                                  sai_acl_rule_t *set_rule,
                                  sai_acl_rule_t *compare_rule,
                                  sai_acl_rule_t *given_rule);
sai_status_t sai_npu_create_acl_cntr(sai_acl_table_t *acl_table,
                                     sai_acl_counter_t *acl_cntr);
sai_status_t sai_npu_delete_acl_cntr(sai_acl_counter_t *acl_cntr);
sai_status_t sai_npu_set_acl_cntr(sai_acl_counter_t *acl_cntr,
                                  uint64_t count_value, bool byte_set);
sai_status_t sai_npu_get_acl_cntr(sai_acl_counter_t *acl_cntr,
                                  uint_t num_count, uint64_t *count_value);
sai_status_t sai_npu_attach_cntr_to_acl_rule(sai_acl_rule_t *acl_rule,
                                             sai_acl_counter_t *acl_cntr);
sai_status_t sai_npu_detach_cntr_from_acl_rule(sai_acl_rule_t *acl_rule,
                                               sai_acl_counter_t *acl_cntr);
sai_status_t sai_npu_get_acl_attribute(sai_attribute_t *attr);

void sai_npu_acl_dump_table(const sai_acl_table_t *acl_table);
void sai_npu_acl_dump_rule(const sai_acl_rule_t *acl_rule);
void sai_npu_acl_dump_counter(const sai_acl_counter_t *acl_counter);


#endif /* __SAI_VM_ACL_UTIL_H__ */
