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
