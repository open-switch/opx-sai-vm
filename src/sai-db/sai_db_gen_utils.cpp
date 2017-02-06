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
 * @file sai_db_gen_utils.cpp
 *
 * @brief This file contains the function definitions for the utilities
 *        generic to SAI objects and used to update SQL DB tables.
 */

#include "sai_db_gen_utils.h"
#include "sai_oid_utils.h"
#include "saitypes.h"

#include "std_assert.h"
#include "std_type_defs.h"

#include <string>

std::string sai_vm_byte_list_str_get (const sai_u8_list_t *p_byte_list)
{
    size_t           idx = 0;
    std::string      byte_list_str;
    uint8_t         *p_list = NULL;

    STD_ASSERT (p_byte_list != NULL);

    if (p_byte_list->count == 0) {
        return "\"-\"";
    }

    STD_ASSERT (p_byte_list->list != NULL);

    for (p_list = p_byte_list->list; idx < p_byte_list->count; p_list++) {

        byte_list_str += std::to_string (*p_list);
        idx++;

        if (idx != p_byte_list->count) {
            byte_list_str += ", ";
        }
    }

    byte_list_str = "\"" + byte_list_str + "\"";

    return byte_list_str;
}

std::string sai_vm_obj_list_str_get (const sai_object_list_t *p_obj_list)
{
    size_t           idx = 0;
    uint_t           npu_obj_id = 0;
    std::string      obj_list_str;
    sai_object_id_t *p_list = NULL;

    STD_ASSERT (p_obj_list != NULL);

    if (p_obj_list->count == 0) {
        return "\"-\"";
    }

    STD_ASSERT (p_obj_list->list != NULL);

    for (p_list = p_obj_list->list; idx < p_obj_list->count; p_list++) {
        npu_obj_id = (uint_t) sai_uoid_npu_obj_id_get (*p_list);

        obj_list_str += std::to_string (npu_obj_id);
        idx++;

        if (idx != p_obj_list->count) {
            obj_list_str += ", ";
        }
    }

    obj_list_str = "\"" + obj_list_str + "\"";

    return obj_list_str;
}
