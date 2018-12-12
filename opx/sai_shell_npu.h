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

/***
 * \file    sai_shell_npu.h
 *
 * \brief Declaration of SAI NPU SHELL APIs
*/

#ifndef __SAI_SHELL_NPU_H
#define __SAI_SHELL_NPU_H

#include "std_error_codes.h"

/**
 * @defgroup SAINPUAPIS SAI - SAI Shell NPU APIS
 *
 *  @{
 */

/**
 * This function call will process the command.
 * @param cmd is npu command to be executed.
 * @return true if command was executed successfully else returns false
 */
typedef bool (*sai_shell_npu_shell_cmd_fn) (const char *cmd);

/**
 * This command initializes the diag/debug shell API.  All initialization or
 * resources needed for the shell are handled by this function.
 * @return SAI_STATUS_SUCCESS if initialization is successful otherwise appropriate
 * error code is returned
 */
typedef sai_status_t (*sai_shell_npu_shell_command_init_fn)(void);

/**
 * This function allows the switch SDK to customize the prompt returned to the
 * shell
 * @return the character string prompt
 */
typedef const char* (*sai_shell_npu_shell_prompt_get_fn)(void) ;

/**
 * @brief Shell NPU API table.
 */
typedef struct _sai_npu_shell_api_t {
    sai_shell_npu_shell_cmd_fn           shell_cmd;
    sai_shell_npu_shell_command_init_fn  shell_cmd_init;
    sai_shell_npu_shell_prompt_get_fn    shell_prompt_get;
} sai_npu_shell_api_t;


/*
 *  @}
 */



#endif

