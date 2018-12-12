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
 * \file   sai_shell_init.c
 * \brief  SAI shell init
 * */
#include "saistatus.h"
#include "sai_shell.h"
#include "sai_shell_common.h"
#include "sai_shell_npu.h"
#include "sai_switch_utils.h"

static sai_shell_init_functions_t sai_shell_init_functions [] = {
    { sai_shell_cmd_init, NULL,NULL},
    { sai_shell_start , NULL, NULL },
};

sai_status_t sai_shell_init() {
    int ix = 0;
    int mx = sizeof (sai_shell_init_functions) / sizeof(*sai_shell_init_functions);
    sai_status_t er = SAI_STATUS_SUCCESS;
    for ( ; ix < mx ; ++ix ) {
        if (sai_shell_init_functions[ix].init_fun_void!=NULL) {
            if ((er=sai_shell_init_functions[ix].init_fun_void())!=SAI_STATUS_SUCCESS) {
                SAI_SWITCH_LOG_ERR ("SAI shell init failed with err %d",er);
                return er;
            }
        }
        if (sai_shell_init_functions[ix].init_fun_param!=NULL) {
            if ((er=sai_shell_init_functions[ix].init_fun_param(
                            sai_shell_init_functions[ix].param))!=SAI_STATUS_SUCCESS) {
                SAI_SWITCH_LOG_ERR ("SAI shell init function param failed with err %d",er);
                return er;
            }
        }
    }
    return er;
}
