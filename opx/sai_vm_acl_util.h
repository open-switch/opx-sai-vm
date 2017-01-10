
/************************************************************************
 * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
 *
 * This source code is confidential, proprietary, and contains trade
 * secrets that are the sole property of Dell Inc.
 * Copy and/or distribution of this source code or disassembly or reverse
 * engineering of the resultant object code are strictly forbidden without
 * the written consent of Dell Inc.
 *
 ************************************************************************/
/*
 * @file sai_vm_acl_util.h
 *
 * @brief This file contains the function prototypes for SAI acl functions
 *        in VM environment.
 *************************************************************************/

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
