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

/***
 * \file    sai_mcast_api.h
 *
 * \brief Declaration of SAI MCAST related APIs
*/

#if !defined (__SAIMCASTAPI_H_)
#define __SAIMCASTAPI_H_
#include "saitypes.h"
#include "saistatus.h"
#include "sai_mcast_common.h"

/** SAI MCAST API - Init MCAST Module data structures
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_UNINITIALIZED
*/
sai_status_t sai_mcast_init(void);

/** SAI MCAST API - Lock for accessing Mcast tree
*/
void sai_mcast_lock(void);
/** SAI MCAST API - API to release the mcast lock
*/
void sai_mcast_unlock(void);
/** SAI MCAST API - Insert multicast entry to the cache
    \param[in] mcast_entry_node MCAST entry info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_ITEM_ALREADY_EXISTS, SAI_STATUS_FAILURE
*/
sai_status_t sai_insert_mcast_entry_node(dn_sai_mcast_entry_node_t *mcast_entry_node);
/** SAI MCAST API - Find multicast entry in the cache
    \param[in] mcast_entry_node MCAST entry info
    \return A Valid pointer to the mcast entry node in the cache else NULL
*/
dn_sai_mcast_entry_node_t * sai_find_mcast_entry(dn_sai_mcast_entry_node_t *entry_node);
/** SAI MCAST API - Remove multicast entry from the cache
    \param[in] A valid mcast_entry_node pointer in the cache
    \return Success: SAI_STATUS_SUCCESS
*/
sai_status_t sai_remove_mcast_entry_node(dn_sai_mcast_entry_node_t *mcast_entry_node);

#endif
