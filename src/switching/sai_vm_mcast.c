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

