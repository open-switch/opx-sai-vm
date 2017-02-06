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
 * @file sai_acl_db_utils.h
 *
 * @brief This file contains the function prototypes for the utilities
 *        to update the SAI ACL objects related DB tables in
 *        VM environment.
 */

#ifndef __SAI_ACL_DB_UTILS_H__
#define __SAI_ACL_DB_UTILS_H__

#include "sai_acl_type_defs.h"
#include "saiacl.h"

#include <string>

/* SAI ACL Table object related DB utilities */
std::string sai_acl_table_stage_str_get (sai_acl_stage_t stage);
std::string sai_acl_table_field_attr_str_get (sai_acl_table_attr_t field);

/* SAI ACL Rule object related DB utilities */
std::string sai_acl_rule_filter_attr_str_get (sai_acl_entry_attr_t filter);
std::string sai_acl_rule_action_attr_str_get (sai_acl_entry_attr_t action);
void sai_acl_rule_filter_match_info_str_get (sai_acl_filter_t *p_filter,
                                             std::string *match_data_str,
                                             std::string *match_mask_str);
std::string sai_acl_rule_action_parameter_str_get (sai_acl_action_t *p_action);

/* SAI ACL Counter object related DB utilities */
std::string sai_acl_cntr_type_str_get (sai_acl_counter_type_t cntr_type);

#endif /* __SAI_ACL_DB_UTILS_H__ */
