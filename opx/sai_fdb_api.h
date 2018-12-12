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
 * \file    sai_fdb_api.h
 *
 * \brief  Declaration of SAI FDB related APIs
*/


#if !defined (__SAIFDBAPI_H_)
#define __SAIFDBAPI_H_
#include "saistatus.h"
#include "sai_fdb_common.h"
#include "saiswitch.h"
#include "saifdb.h"

/** SAI FDB API - Init FDB Tree
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_UNINITIALIZED
*/
sai_status_t sai_init_fdb_tree(void);

/** SAI FDB API - Get FDB entry Node from cache
      \param[in] fdb_entry FDB entry for which node to be get from cache
      \return Success: A valid pointer to FDB entry node
                    Failure: NULL
*/
sai_fdb_entry_node_t* sai_get_fdb_entry_node(const sai_fdb_entry_t *fdb_entry);

/** SAI FDB API - Create and insert FDB entry Node to cache
      \param[in] fdb_entry FDB entry to be cached
      \param[in] fdb_entry_node_data Data to be cached for the FDB entry node
      \return Success: SAI_STATUS_SUCCESS
                    Failure: SAI_STATUS_FAILURE, SAI_STATUS_NO_MEMORY
*/
sai_status_t sai_insert_fdb_entry_node(const sai_fdb_entry_t *fdb_entry,
                                       sai_fdb_entry_node_t *fdb_entry_node_data);

/** SAI FDB API - Update existing FDB entry node
      \param[inout] fdb_entry FDB entry node to be updated
      \param[in] sai_attribute_t attribute that needs to be updated
      \return Success: SAI_STATUS_SUCCESS
                    Failure: SAI_STATUS_FAILURE, SAI_STATUS_NO_MEMORY
*/
void sai_update_fdb_entry_node (sai_fdb_entry_node_t *fdb_entry_node,
                                const sai_attribute_t *attr);

/** SAI FDB API - Check if FDB attribute is valid
      \param[in] sai_attribute_t attribute that needs to be validated
      \return Success: SAI_STATUS_SUCCESS
                    Failure: SAI_STATUS_INVALID_ATTR_VALUE_0,SAI_STATUS_INVALID_ATTRIBUTE_0
*/
sai_status_t sai_is_valid_fdb_attribute_val(const sai_attribute_t *fdb_attr);

/** SAI FDB API - Get port id from fdb entry
      \param[in] fdb_entry FDB entry for which port is required
      \param[out] bridge_port_id Bridge Port id on which entry is installed
      \return Success: SAI_STATUS_SUCCESS
                    Failure: SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_fdb_get_bridge_port_from_cache(const sai_fdb_entry_t *fdb_entry,
                                                sai_object_id_t *bridge_port_id);

/** SAI FDB API - Lock FDB for access
*/
void sai_fdb_lock(void);

/** SAI FDB API - Unlock FDB after access
*/
void sai_fdb_unlock(void);

/** SAI FDB API - Write a registered FDB entry into cache so that an event
                  like insert, delete or move could trigger the notification to subscriber
    \param[in] fdb_entry FDB Entry to register
    \return Success: SAI_STATUS_SUCCESS
            Failure: Appropriate error code will be returned
*/
sai_status_t sai_fdb_write_registered_entry_into_cache (const sai_fdb_entry_t *fdb_entry);

/** SAI FDB API - Remove a registered FDB entry
    \param[in] fdb_entry FDB Entry to register
    \return Success: SAI_STATUS_SUCCESS
            Failure: Appropriate error code will be returned
*/
sai_status_t sai_fdb_remove_registered_entry_from_cache (const sai_fdb_entry_t *fdb_entry);

/** SAI FDB API - Internal callback function pointer declaration
    \param[in] num_notification Number of notifications
    \param[in] data Array of sai_fdb_internal_notification_data_t containing notification information
    \return Success: SAI_STATUS_SUCCESS
            Failure: Appropriate error code will be returned
*/
typedef sai_status_t (*sai_fdb_internal_callback_fn)(uint_t num_notification,
                                                     sai_fdb_internal_notification_data_t *data);


/** SAI FDB API - Register internal callback function
    \param[in] fdb_callback Function pointer to callback function
*/
void sai_fdb_internal_callback_cache_update (sai_fdb_internal_callback_fn
                                                 fdb_callback);

/** SAI FDB API - Send internal notifications to the subscriber
*/
void sai_fdb_send_internal_notifications(void);

/** SAI FDB API - Check if there are any pending notifications to be sent
    \return Success: true
            Failure: false
*/
bool sai_fdb_is_notifications_pending (void);

/** SAI FDB API - Get FDB entry type for flush
      \param[in] flush_entry_type The type of entry that needs to be flushed
      \return: One of entry types in sai_fdb_entry_type_t
*/
static inline sai_fdb_entry_type_t sai_get_sai_fdb_entry_type_for_flush(sai_fdb_flush_entry_type_t
                                                                        flush_entry_type)
{
    sai_fdb_entry_type_t entry_type = SAI_FDB_ENTRY_TYPE_DYNAMIC;

    if (flush_entry_type == SAI_FDB_FLUSH_ENTRY_TYPE_STATIC) {
        entry_type = SAI_FDB_ENTRY_TYPE_STATIC;
    }
    return entry_type;
}

/** SAI FDB API - NPU FDB notification function
      \param[in] num_notification Number of notifications
      \param[in] event_data List of data associated with the FDB event
*/
typedef void (*sai_fdb_npu_event_notification_fn) (uint_t num_notification,
                                                   sai_fdb_event_data_t *event_data);

/** SAI FDB API - Remove FDB entry node from cache
      \param[in] fdb_entry_node Remove FDB entry node from cache

*/
void sai_remove_fdb_entry_node (sai_fdb_entry_node_t *fdb_entry_node);

/** SAI FDB API - Get the next FDB entry node from cache
      \param[in] fdb_key Key of the current FDB entry node

*/
sai_fdb_entry_node_t *sai_get_next_fdb_entry_node (sai_fdb_entry_key_t *fdb_key);

/** SAI FDB API - Get the next FDB registered node from cache
      \param[in] fdb_key Key of the current FDB registered node

*/
sai_fdb_registered_node_t *sai_get_next_fdb_registered_node (sai_fdb_entry_key_t *fdb_key);
#endif
