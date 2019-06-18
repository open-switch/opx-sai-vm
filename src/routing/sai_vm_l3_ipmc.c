/************************************************************************
* * LEGALESE:   "Copyright (c) 2019, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/**
* @file sai_vm_l3_ipmc.c
*
* @brief This file contains common L3IPMC implementation APIs for VM.
*************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "std_assert.h"
#include "sail2mc.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_npu_l2mc.h"
#include "sai_npu_ipmc.h"
#include "sai_l2mc_common.h"
#include "sai_l2mc_api.h"
#include "sai_ipmc_common.h"
#include "sai_ipmc_api.h"
#include "sai_bridge_api.h"
#include "sai_switch_utils.h"
#include "sai_oid_utils.h"
#include "sai_l3_util.h"
#include "sai_lag_api.h"

static sai_status_t sai_vm_l3_ipmc_init(void)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l3_ipmc_repl_group_create(dn_sai_ipmc_repl_group_node_t *ipmc_repl_group_node)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l3_ipmc_repl_group_delete(dn_sai_ipmc_repl_group_node_t *ipmc_repl_group_node)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l3_ipmc_rpf_group_create(dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l3_ipmc_rpf_group_delete(dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l3_ipmc_rpf_group_modify(dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l3_ipmc_rpf_group_member_create(
                     dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node,
                     dn_sai_ipmc_rpf_group_member_node_t *ipmc_rpf_group_member_node)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l3_ipmc_rpf_group_member_delete(
                         dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node,
                         dn_sai_ipmc_rpf_group_member_node_t *ipmc_rpf_group_member_node)

{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l3_ipmc_rpf_group_member_port_add(
                     dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node,
                     dn_sai_ipmc_rpf_group_member_node_t *ipmc_rpf_group_member_node,
                     sai_object_id_t attach_port_id)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l3_ipmc_rpf_group_member_port_del(
                     dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node,
                     dn_sai_ipmc_rpf_group_member_node_t *ipmc_rpf_group_member_node,
                     sai_object_id_t attach_port_id)
{
    return SAI_STATUS_SUCCESS;
}

void sai_vm_l3_ipmc_rpf_group_member_dump_hw_info(const void *hw_info)
{
    return;
}

static sai_npu_l3_ipmc_api_t sai_vm_l3_ipmc_api_table = {
    sai_vm_l3_ipmc_init,
    sai_vm_l3_ipmc_repl_group_create,
    sai_vm_l3_ipmc_repl_group_delete,
    sai_vm_l3_ipmc_rpf_group_create,
    sai_vm_l3_ipmc_rpf_group_delete,
    sai_vm_l3_ipmc_rpf_group_modify,
    sai_vm_l3_ipmc_rpf_group_member_create,
    sai_vm_l3_ipmc_rpf_group_member_delete,
    sai_vm_l3_ipmc_rpf_group_member_port_add,
    sai_vm_l3_ipmc_rpf_group_member_port_del,
    sai_vm_l3_ipmc_rpf_group_member_dump_hw_info,
};

sai_npu_l3_ipmc_api_t* sai_vm_l3_ipmc_api_query (void)
{
    return &sai_vm_l3_ipmc_api_table;
}
