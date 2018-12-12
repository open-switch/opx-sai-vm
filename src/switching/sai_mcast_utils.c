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
* @file sai_mcast_utils.c
*
* @brief This file contains utility APIs for SAI L2MC module
*************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "saitypes.h"
#include "saistatus.h"
#include "saitypes.h"
#include "sai_gen_utils.h"
#include "sai_l2mc_common.h"
#include "sai_mcast_common.h"
#include "std_mutex_lock.h"
#include "std_assert.h"
#include "sai_gen_utils.h"

static dn_sai_mcast_global_data_t sai_mcast_global_cache;
static std_mutex_lock_create_static_init_fast(mcast_lock);

std_rt_table *sai_mcast_global_cache_get(void)
{
    return sai_mcast_global_cache.sai_global_mcast_tree;
}

void sai_mcast_lock(void)
{
    std_mutex_lock(&mcast_lock);
}

void sai_mcast_unlock(void)
{
    std_mutex_unlock(&mcast_lock);
}

sai_status_t sai_mcast_init(void)
{

    SAI_MCAST_LOG_TRACE("Performing MCAST Cache Init");

    sai_mcast_global_cache.sai_global_mcast_tree = std_radix_create("MCASTCache",
            SAI_MCAST_ENTRY_KEY_SIZE, NULL, NULL, 0);
    if(sai_mcast_global_cache.sai_global_mcast_tree == NULL) {
        SAI_MCAST_LOG_CRIT("Unable to create MCAST Cache");
        return SAI_STATUS_UNINITIALIZED;
    }
    return SAI_STATUS_SUCCESS;
}

dn_sai_mcast_entry_node_t * sai_find_mcast_entry(dn_sai_mcast_entry_node_t *entry_node)
{
    dn_sai_mcast_entry_node_t *mcast_entry_node = NULL;

    STD_ASSERT(entry_node != NULL);

    mcast_entry_node = (dn_sai_mcast_entry_node_t *)std_radix_getexact(
            sai_mcast_global_cache.sai_global_mcast_tree,
            (u_char *)&entry_node->mcast_key,
            SAI_MCAST_ENTRY_KEY_SIZE);
    return mcast_entry_node;
}

sai_status_t sai_insert_mcast_entry_node(dn_sai_mcast_entry_node_t *mcast_entry_node)
{
    dn_sai_mcast_entry_node_t *tmp_node = NULL;
    std_rt_head *mcast_rt_head = NULL;

    STD_ASSERT(mcast_entry_node != NULL);

    tmp_node = sai_find_mcast_entry(mcast_entry_node);
    if (tmp_node != NULL) {
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    if ((tmp_node = (dn_sai_mcast_entry_node_t *)
            calloc (1, sizeof(dn_sai_mcast_entry_node_t))) == NULL) {
        SAI_L2MC_LOG_ERR("Failed to allocate memory for MCAST Entry node");
        return SAI_STATUS_NO_MEMORY;
    }

    memcpy(tmp_node, mcast_entry_node, sizeof(dn_sai_mcast_entry_node_t));
    tmp_node->mcast_rt_head.rth_addr = (unsigned char *) &tmp_node->mcast_key;
    mcast_rt_head = std_radix_insert(sai_mcast_global_cache.sai_global_mcast_tree,
            &(tmp_node->mcast_rt_head),
            SAI_MCAST_ENTRY_KEY_SIZE);
    if(mcast_rt_head == NULL) {
        SAI_MCAST_LOG_CRIT("Unable to add MCAST Node TBD");
        free(tmp_node);
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_remove_mcast_entry_node(dn_sai_mcast_entry_node_t *mcast_entry_node)
{

    STD_ASSERT(mcast_entry_node != NULL);
    std_radix_remove(sai_mcast_global_cache.sai_global_mcast_tree,&(mcast_entry_node->mcast_rt_head));
    free(mcast_entry_node);
    return SAI_STATUS_SUCCESS;
}

