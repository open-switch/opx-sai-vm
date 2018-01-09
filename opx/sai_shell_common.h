/************************************************************************
* * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
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
