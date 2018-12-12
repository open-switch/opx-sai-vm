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
 * \file   sai_shell.c
 * \brief  shell handlers
 */

#define _GNU_SOURCE

#include "saistatus.h"
#include "sai_shell.h"
#include "sai_shell_common.h"
#include "sai_shell_npu.h"
#include "sai_switch_utils.h"

#include "std_llist.h"
#include "std_cmd_redir.h"
#include "std_mutex_lock.h"
#include "std_type_defs.h"
#include "std_error_codes.h"
#include "std_utils.h"
#include "sai_common_infra.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#define SAI_SHELL_CMD_SOCK "/var/run/ar.npu.shell"
#define SAI_PROMPT "SAI.%d> "

static std_cmd_redir_t shell;
static bool _prompt=false;
static bool npu_shell_enabled=false;
static char sai_prompt[SAI_SHELL_PROMPT_SIZE];

static std_dll_head cmd_list;

static std_mutex_lock_create_static_init_rec(sai_shell_lock);

bool sai_shell_check_cmd(const char *cmd, const char *buff) {
    return strncmp(cmd,buff,strlen(cmd))==0;
}

const char* sai_shell_prompt_get(void)
{
    snprintf (sai_prompt, sizeof(sai_prompt), SAI_PROMPT, sai_switch_id_get());
    return sai_prompt;
}

static void sai_shell_show_prompt() {
    const char *prompt;
    if (npu_shell_enabled) {
        prompt = sai_shell_npu_api_get()->shell_prompt_get();
    }
    else {
        prompt = sai_shell_prompt_get();
    }
    printf ("%s",prompt);
}

static void sai_shell_process_cmd(const char * msg) {
    if (!sai_shell_check_cmd("::",msg)) {
        std_mutex_lock(&sai_shell_lock);
        sai_shell_npu_api_get()->shell_cmd(msg);
        std_mutex_unlock(&sai_shell_lock);
        return ;
    }
    else if (!sai_shell_check_cmd("::",msg)) {
        printf ("Not a SAI shell command\n");
        return;
    }
    msg+=2;

    std_mutex_lock(&sai_shell_lock);
    std_dll *ptr = std_dll_getfirst(&cmd_list);
    do {
        cmd_entry_t *p = (cmd_entry_t*)ptr;
        if (p->fun(p->param,msg)) break;
    } while ( (ptr=std_dll_getnext(&cmd_list,ptr))!=NULL);
    std_mutex_unlock(&sai_shell_lock);
}


void *npu_shell_proxy(void *param) {
    static char buff[1024] = {0};
    while (1) {
        if (_prompt) sai_shell_show_prompt();
        fflush(stdout);

        if (fgets(buff,sizeof(buff),stdin)==NULL) continue;
        if (buff[0] == 0) continue;
        if (buff[0] == '\n') continue;

        std_remove_trailing_whitespace(buff,"\n \t");

        sai_shell_process_cmd(buff);
        fflush(stdout);
    }
    return NULL;
}

sai_status_t sai_shell_run_command(const char *cmd) {
    sai_shell_process_cmd(cmd);
    return SAI_STATUS_SUCCESS;
}

bool sai_shell_cmd_add_flexible(void * param, sai_shell_check_run_function fun) {

    cmd_entry_t *p = (cmd_entry_t*)calloc(1,sizeof(cmd_entry_t));
    if(p==NULL) return false;

    std_mutex_lock(&sai_shell_lock);
    p->fun = fun;
    p->param = param;
    std_dll_insert(&cmd_list,(std_dll*)p);
    std_mutex_unlock(&sai_shell_lock);
    return true;
}

static bool sai_shell_check_run_match_function(void * param, const char *cmd_string) {
    cmd_simple_entry_t *p = (cmd_simple_entry_t*) param;
    if (!sai_shell_check_cmd(p->cmd,cmd_string)) return false;

    std_parsed_string_t handle;
    const char *args = cmd_string+strlen(p->cmd);
    while(isspace(*args)) args++;
    if (!std_parse_string(&handle,args," ")) return false;

    p->fun(handle);
    std_parse_string_free(handle);
    return true;
}

bool sai_shell_cmd_add(const char *cmd_name,sai_shell_function fun,const char *description) {
    cmd_simple_entry_t *p = (cmd_simple_entry_t*)calloc(1,sizeof(cmd_simple_entry_t));
    if (p==NULL)  return false;

    p->cmd = cmd_name;
    p->fun =fun;
    p->description = description;
    if (!sai_shell_cmd_add_flexible(p,sai_shell_check_run_match_function)) {
        free(p); return false;
    }
    return true;
}

static void sai_npu_shell(std_parsed_string_t handle) {
    npu_shell_enabled = true;
}


static void sai_shell_prompt_change(std_parsed_string_t handle) {
    if(std_parse_string_num_tokens(handle)==0) return;
    size_t ix = 0;
    _prompt=strstr(std_parse_string_next(handle,&ix),"off")!=NULL ? false : true;
}

static void sai_shell_help(std_parsed_string_t handle) {
    printf("SAI Shell Command Help\n");
    std_mutex_lock(&sai_shell_lock);
    std_dll *ptr = std_dll_getfirst(&cmd_list);
    do {
        cmd_entry_t *p = (cmd_entry_t*)ptr;
        if ( p->fun == sai_shell_check_run_match_function) {
            cmd_simple_entry_t *cmd = (cmd_simple_entry_t*)p->param;
            printf("::%s - %s\n",cmd->cmd, cmd->description ? cmd->description : "No information");
        }
    } while ( (ptr=std_dll_getnext(&cmd_list,ptr))!=NULL);
    std_mutex_unlock(&sai_shell_lock);
}

static void sai_shell_cmd_exit(std_parsed_string_t handle) {
    printf("\n");
    if (npu_shell_enabled) {
        npu_shell_enabled = false;
    } else
    {
        std_cmd_redir_term_conn(&shell);
    }
}

sai_status_t sai_shell_cmd_init(void) {
    std_dll_init(&cmd_list);

    sai_shell_cmd_add("help",sai_shell_help,"Displays List of commands and its description");
    sai_shell_cmd_add("exit",sai_shell_cmd_exit,"Exit the SAI Shell");
    sai_shell_cmd_add("prompt",sai_shell_prompt_change,"[on|off] - To Enable/Disable Prompt");
    sai_shell_cmd_add("npu",sai_npu_shell,"To Enter NPU shell");
    sai_shell_add_debug_commands();

    return (sai_shell_npu_api_get()->shell_cmd_init());
}

sai_status_t sai_shell_start(void) {

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_shell_set(bool status)
{
    t_std_error er = STD_ERR_OK;

    SAI_SWITCH_LOG_TRACE("SAI Shell re-initialized");

    if (status == false) {
        if((er = std_cmd_redir_term_conn(&shell)) != STD_ERR_OK) {
            SAI_SWITCH_LOG_ERR ("SAI tern redirect failed with err %d",er);
            return SAI_STATUS_FAILURE;
        }
    }else {
        strncpy(shell.path,SAI_SHELL_CMD_SOCK,sizeof(shell.path)-1);
        shell.param = NULL;
        shell.func = npu_shell_proxy;
        if ((er = std_cmd_redir_init(&shell)) != STD_ERR_OK) {
            SAI_SWITCH_LOG_ERR ("SAI redirect init failed with err %d",er);
            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}
