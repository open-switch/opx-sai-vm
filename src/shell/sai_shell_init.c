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
