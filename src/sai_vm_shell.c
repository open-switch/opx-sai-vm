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
 * @file sai_vm_shell.c
 *
 * @brief This file contains function definitions for SAI SHELL in VM
 *        environment.
 *************************************************************************/
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

