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
 * @file sai_vm_shell.c
 *
 * @brief This file contains function definitions for SAI SHELL in VM
 *        environment.
 */

#include "sai_vm_defs.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_shell_npu.h"
#include "sai_shell.h"
#include "sai_switch_utils.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include <stdio.h>
#include <stddef.h>

static char sai_vm_prompt [SAI_SHELL_PROMPT_SIZE];

static bool sai_shell_npu_shell_cmd (const char *cmd)
{
    STD_ASSERT(cmd != NULL);

    return true;
}

static sai_status_t sai_shell_npu_shell_command_init (void)
{
    snprintf (sai_vm_prompt, (sizeof (sai_vm_prompt) - 1), SAI_VM_SHELL_PROMPT,
              (sai_switch_id_get ()));

    return SAI_STATUS_SUCCESS;
}

static const char* sai_shell_npu_shell_prompt_get (void)
{
    return sai_vm_prompt;
}

static sai_npu_shell_api_t sai_vm_shell_api_table = {
    sai_shell_npu_shell_cmd,
    sai_shell_npu_shell_command_init,
    sai_shell_npu_shell_prompt_get
};

sai_npu_shell_api_t* sai_vm_shell_api_query (void)
{
    return &sai_vm_shell_api_table;
}

