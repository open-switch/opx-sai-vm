/************************************************************************
* * LEGALESE:   "Copyright (c) 2017, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/**
* @file sai_vm_l3_mcast.c
*
* @brief This file contains L3 Multicast implementation APIs for VM.
*************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "std_assert.h"
#include "sail2mc.h"
#include "sail2mcgroup.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saiswitch.h"
#include "sai_npu_l2mc.h"
#include "sai_npu_mcast.h"
#include "sai_l2mc_common.h"
#include "sai_mcast_common.h"
#include "sai_switch_utils.h"
#include "sai_l3_util.h"
#include "sai_oid_utils.h"
#include <string.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include "sai_l2mc_api.h"
#include "sai_ipmc_api.h"
#include "sai_vlan_api.h"
#include "sai_mcast_api.h"
#include "saiipmc.h"
#include "sai_npu_l3_mcast.h"

static sai_status_t sai_vm_l3_mcast_init(void)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l3_mcast_entry_create(const dn_sai_l3_mcast_entry_node_t *l3_mcast_entry_node)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l3_mcast_entry_remove(const dn_sai_l3_mcast_entry_node_t *l3_mcast_entry_node)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l3_mcast_entry_update(const dn_sai_l3_mcast_entry_node_t *l3_mcast_entry_node)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l3_mcast_entry_get(const dn_sai_l3_mcast_entry_node_t *l3_mcast_entry_node)
{
    return SAI_STATUS_SUCCESS;
}

static sai_npu_l3_mcast_api_t sai_vm_l3_mcast_api_table = {
    sai_vm_l3_mcast_init,
    sai_vm_l3_mcast_entry_create,
    sai_vm_l3_mcast_entry_remove,
    sai_vm_l3_mcast_entry_update,
    sai_vm_l3_mcast_entry_get
};

sai_npu_l3_mcast_api_t* sai_vm_l3_mcast_api_query (void)
{
    return &sai_vm_l3_mcast_api_table;
}

