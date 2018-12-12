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

/*
 * \file    sai_shell_common.h
 *
 * \brief Init datastructures and logging functions
*/

#ifndef __SAI_SHELL_COMMON_H
#define __SAI_SHELL_COMMON_H

#include "std_llist.h"
#include "std_error_codes.h"
#include "event_log_types.h"

typedef struct _sai_shell_init_functions_t {
    t_std_error (*init_fun_void)(void); /*init functions without any parameters*/
    t_std_error (*init_fun_param)(void *param); /*init functions with parameters*/
    void *param; /*init parameters*/
} sai_shell_init_functions_t;

typedef struct {
    std_dll entry;
    sai_shell_check_run_function fun;
    void * param;
} cmd_entry_t;

typedef struct {
    const char * cmd;
    const char * description;
    sai_shell_function fun;
}cmd_simple_entry_t;

void sai_shell_add_debug_commands(void);

#endif
