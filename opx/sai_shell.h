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
 * \file    sai_shell.h
 *
 * \brief Declaration of SAI SHELL APIs
*/

#ifndef __SAI_SHELL_H_
#define __SAI_SHELL_H_

#include "saitypes.h"

#include "std_utils.h"
#include "std_type_defs.h"

#include <stdbool.h>
#include <stddef.h>

/**
 * @defgroup SAISHELLAPIS SAI - shell APIS
 * Functions used to enable and define a debug shell for SAI
 * Provides a shell-interface to manage the NPU using SAI API
 * Provide access to the vendor provided shell.
 * @{
 */

/**
 * size of the SAI shell prompt
 */
#define SAI_SHELL_PROMPT_SIZE 1024

/**
 * This function call checks the whether the command is already registered or not.
 * If the command is already registered the callback function for the command would be
 * called.
 * @param param is the parameter that the function call is registered with
 * @param cmd_string the command to be checked and executed.
 * @return true if the cmd_string is meant to be processed by this function, otherwise false
 */
typedef bool (*sai_shell_check_run_function)(void * param, const char *cmd_string);

/**
 * This function will just execute a specific shell command. When registering this
 * callback function the caller would have already specified the actual name of the
 * shell command.Therefore this function takes the command line arguments of
 * shell command is input parameter.
 * eg.
 *      shell to print a route table
 *      ::print-routes all
 *      this function would receive the string " all"
 * (everything after the "print-routes")
 * @param handle is a parsed string handle to process command line arguments
 */
typedef void (*sai_shell_function)(std_parsed_string_t handle);

/**
 * This function initializes the sai shell commands. After this function call,
 * application can register shell commands to be processed by the sai shell
 * @return sai return code
 */
sai_status_t sai_shell_cmd_init(void);

/**
 * This will add a function call to the shell command list.  Any function added
 * here can be called from the shell by prefixing the command with a ::
 * @param param the context to the function call
 * @param fun the function to register
 * @return true command was added, false otherwise
 */
bool sai_shell_cmd_add_flexible(void * param, sai_shell_check_run_function fun);

/**
 * This will register a callback function against a specific shell command.
 * @param cmd_name the string name of the shell command to be added
 * @param fun the function call to be executed
 * @param description the description of the command. Make sure to pass a const
 * char string only.
 * @return true command was added, false otherwise
 */
bool sai_shell_cmd_add(const char *cmd_name,sai_shell_function fun,const char *description);

/**
 * This will start the actual shell which will then begin accepting commands.
 * @return standard error code
 */
sai_status_t sai_shell_start(void);

/**
 * This will enable/ disable shell redirect logs.
 */
sai_status_t sai_shell_set(bool status);


/**
 * Run a shell command - this allows the use of any created shell command incuding
 * ::commands and default commands.
 * @param cmd command to run
 * @return sai return code
 */
sai_status_t sai_shell_run_command(const char *cmd);

/**
 * The initializaiton function for the shell.
 * @return  Error code indicating that a problem has occured.  SAI SHELL will not
 *          start if there is an error at initializaiton time.
 */
sai_status_t sai_shell_init(void);
/**
 * @}
 */

#endif /* __SAI_SHELL_H_ */
