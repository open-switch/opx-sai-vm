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
 * @file sai_db_gen_utils.h
 *
 * @brief This file contains the function prototypes for the general
 *        utilities used to update the SQL DB tables.
 */

#ifndef __SAI_DB_GEN_UTILS_H__
#define __SAI_DB_GEN_UTILS_H__

#include "saitypes.h"
#include <string>

/*
 * @brief Get the string for the SAI byte list.
 * @return comma seperated string for the given SAI byte list.
 */
std::string sai_vm_byte_list_str_get (const sai_u8_list_t *p_byte_list);

/*
 * @brief Get the string for the SAI object list.
 * @return comma seperated string for the given SAI object list.
 */
std::string sai_vm_obj_list_str_get (const sai_object_list_t *p_obj_list);

#endif /* __SAI_DB_GEN_UTILS_H__ */
