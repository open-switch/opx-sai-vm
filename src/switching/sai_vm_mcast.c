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
* @file sai_vm_l2mc.c
*
* @brief This file contains L2MC implementation APIs for sai-vm.
*************************************************************************/

#include "std_assert.h"
#include "saistatus.h"
#include "sai_npu_mcast.h"

static sai_status_t sai_vm_mcast_init(void)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_mcast_entry_create(const dn_sai_mcast_entry_node_t *mcast_entry_node)
{
    STD_ASSERT(mcast_entry_node != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_mcast_entry_remove(const dn_sai_mcast_entry_node_t *mcast_entry_node)
{
    STD_ASSERT(mcast_entry_node != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_npu_mcast_api_t sai_vm_mcast_api_table = {
    sai_vm_mcast_init,
    sai_vm_mcast_entry_create,
    sai_vm_mcast_entry_remove,
};

sai_npu_mcast_api_t* sai_vm_mcast_api_query (void)
{
    return &sai_vm_mcast_api_table;
}

