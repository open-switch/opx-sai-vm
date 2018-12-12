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
 * \file    sai_npu_fdb.h
 *
 * \brief  Declaration of SAI FDB NPU specific APIs
*/

#if !defined (__SAINPUFDB_H_)
#define __SAINPUFDB_H_

#include "saifdb.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saiswitch.h"
#include "sai_fdb_common.h"
#include "sai_fdb_api.h"

/** SAI NPU FDB -Perform FDB NPU related initialization
    \return Success: SAI_STATUS_SUCCESS
            Failure: One of the failure error codes
*/
typedef sai_status_t (*sai_npu_fdb_init_fn)(void);

/** SAI NPU FDB - Flush all FDB entries based on parameters passed
      \param[in] bridge_port_id Bridge port ID. SAI_NULL_OBJECT_ID if bridge port match is not used
      \param[in] bv_id Specifies Bridge or VLAN Object ID. SAI_NULL_OBJECT_ID if not used
      \param[in] delete_all  Flush all entry types.
      \param[in] flush_type  Specific entry type that needs to be flushed.
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_INVALID_PORT_NUMBER,SAI_STATUS_INVALID_VLAN_ID
*/
typedef sai_status_t (*sai_npu_flush_all_fdb_entries_fn)(sai_object_id_t bridge_port_id,
                                                         sai_object_id_t bv_id, bool delete_all,
                                                         sai_fdb_flush_entry_type_t flush_type);

/** SAI NPU FDB - Flush the specified FDB entry
  \param[in] fdb_entry Which has members MAC address and VLAN
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_flush_fdb_entry_fn)(const sai_fdb_entry_t* fdb_entry, bool validate_port);

/** SAI NPU FDB - Create FDB entry
  \param[in] fdb_entry Which has members MAC address and VLAN
  \param[in] fdb_entry_node_data Data related to FDB entry such as port, type, action, etc.
  \param[in] validate_port If set to true, check if port in hardware and FDB cache are same. If they are not
             same don't remove entry and return error
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE,SAI_STATUS_INVALID_ATTRIBUTE,
SAI_STATUS_INVALID_ATTR_VALUE,SAI_STATUS_NOT_SUPPORTED
 */

typedef sai_status_t (*sai_npu_create_fdb_entry_fn)(const sai_fdb_entry_t *fdb_entry,
                                                    sai_fdb_entry_node_t *fdb_entry_node_data);

/** SAI NPU FDB - Get FDB Entry from hardware
  \param[in] fdb_entry Which has members MAC address and VLAN
  \param[out] fdb_entry_node which has FDB key, type, action and port
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE,SAI_STATUS_INVALID_ATTRIBUTE,
SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_get_fdb_entry_from_hardware_fn)(const sai_fdb_entry_t *fdb_entry,
                                                               sai_fdb_entry_node_t *fdb_entry_node);

/** SAI NPU FDB - Write FDB Entry to hardware
  \param[in] fdb_entry_node which has FDB key, type, action and port
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE,SAI_STATUS_INVALID_ATTRIBUTE,
SAI_STATUS_INVALID_ATTR_VALUE,SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_write_fdb_entry_to_hardware_fn)(sai_fdb_entry_node_t
                                                               *fdb_entry_node);

/** SAI NPU FDB - Set MAC Learning limit per switch
  \param[in] value Number of MACs to be limited
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_set_switch_max_learned_address_fn)(uint32_t value);


/** SAI NPU FDB - Get MAC Learning limit per switch
  \param[in] value Number of MACs to be limited
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_get_switch_max_learned_address_fn)(uint32_t *value);

/** SAI NPU FDB - Set FDB Unicast Miss action
  \param[in] attr Value should be of type sai_switch_fdb_miss_action_t
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_fdb_ucast_miss_action_set_fn)(const sai_attribute_t *attr);

/** SAI NPU FDB - Get FDB Unicast Miss action
  \param[out] attr Value would be of type sai_switch_fdb_miss_action_t
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_fdb_ucast_miss_action_get_fn)(sai_attribute_t *attr);

/** SAI NPU FDB - Set FDB Multicast Miss action
  \param[in] attr Value should be of type sai_switch_fdb_miss_action_t
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_fdb_mcast_miss_action_set_fn)(const sai_attribute_t *attr);

/** SAI NPU FDB - Get FDB Multicast Miss action
  \param[out] attr Value would be of type sai_switch_fdb_miss_action_t
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_fdb_mcast_miss_action_get_fn)(sai_attribute_t *attr);

/** SAI NPU FDB - Set FDB Broadcast Miss action
  \param[in] attr Value should be of type sai_switch_fdb_miss_action_t
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_fdb_bcast_miss_action_set_fn)(const sai_attribute_t *attr);

/** SAI NPU FDB - Get FDB Broadcast Miss action
  \param[out] attr Value would be of type sai_switch_fdb_miss_action_t
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_fdb_bcast_miss_action_get_fn)(sai_attribute_t *attr);
/** SAI NPU FDB - Set Aging time for entries in FDB table
  \param[in] value Aging time in seconds
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_set_aging_time_fn)(uint32_t value);

/** SAI NPU FDB - Get Aging time for entries in FDB table
  \param[out] value Aging time in seconds
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_get_aging_time_fn)(uint32_t *value);

/** SAI NPU FDB - Register Callback for FDB notification from Application to SAI
  \param[in] fdb_notification_fn Callback function pointer
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_register_callback_fn)(sai_fdb_npu_event_notification_fn
                                                     fdb_npu_notification_fn);

/** SAI NPU FDB - Get FDB table size
  \param[out] attr Attribute containing table size in value.u32
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_get_fdb_table_size_fn)(sai_attribute_t *attr);

/** SAI NPU FDB - Enable/Disable flooding of broadcast packets to cpu
  \param[in] enable To Enable or disable
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_bcast_cpu_flood_enable_set_fn)(bool enable);

/** SAI NPU FDB - Enable/Disable flooding of multicast packets to cpu
  \param[in] enable To Enable or disable
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_mcast_cpu_flood_enable_set_fn)(bool enable);

/** SAI NPU FDB - Get flooding setting of broadcast packets to cpu
  \param[out] enable Get whether enabled or disabled
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_bcast_cpu_flood_enable_get_fn)(bool *enable);

/** SAI NPU FDB - Get flooding flooding of multicast packets to cpu
  \param[out] enable Get whether enabled or disabled
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_mcast_cpu_flood_enable_get_fn)(bool *enable);

/** SAI FDB API - Register internal flush callback
    \param[in] flush_fdb_entry Function pointer to flush callback function
*/
void sai_fdb_npu_flush_callback_cache_update (sai_npu_flush_fdb_entry_fn flush_fdb_entry);

/**
 * @brief FDB NPU API table.
 */
typedef struct _sai_npu_fdb_api_t {
    sai_npu_fdb_init_fn                            fdb_init;
    sai_npu_flush_all_fdb_entries_fn               flush_all_fdb_entries;
    sai_npu_flush_fdb_entry_fn                     flush_fdb_entry;
    sai_npu_create_fdb_entry_fn                    create_fdb_entry;
    sai_npu_get_fdb_entry_from_hardware_fn         get_fdb_entry_from_hardware;
    sai_npu_write_fdb_entry_to_hardware_fn         write_fdb_entry_to_hardware;
    sai_npu_set_aging_time_fn                      set_aging_time;
    sai_npu_get_aging_time_fn                      get_aging_time;
    sai_npu_get_fdb_table_size_fn                  get_fdb_table_size;
    sai_npu_register_callback_fn                   register_callback;
    sai_npu_set_switch_max_learned_address_fn      set_switch_max_learned_address;
    sai_npu_get_switch_max_learned_address_fn      get_switch_max_learned_address;
    sai_npu_fdb_ucast_miss_action_set_fn           fdb_ucast_miss_action_set;
    sai_npu_fdb_ucast_miss_action_get_fn           fdb_ucast_miss_action_get;
    sai_npu_fdb_mcast_miss_action_set_fn           fdb_mcast_miss_action_set;
    sai_npu_fdb_mcast_miss_action_get_fn           fdb_mcast_miss_action_get;
    sai_npu_fdb_bcast_miss_action_set_fn           fdb_bcast_miss_action_set;
    sai_npu_fdb_bcast_miss_action_get_fn           fdb_bcast_miss_action_get;
    sai_npu_bcast_cpu_flood_enable_set_fn          bcast_cpu_flood_enable_set;
    sai_npu_mcast_cpu_flood_enable_set_fn          mcast_cpu_flood_enable_set;
    sai_npu_bcast_cpu_flood_enable_get_fn          bcast_cpu_flood_enable_get;
    sai_npu_mcast_cpu_flood_enable_get_fn          mcast_cpu_flood_enable_get;

} sai_npu_fdb_api_t;

#endif
