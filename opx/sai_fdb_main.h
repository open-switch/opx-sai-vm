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
* @file sai_fdb_main.h
*
* @brief Private header file contains the declarations for APIs used in
*        sai_fdb.c
*
*************************************************************************/


#ifndef __SAI_FDB_MAIN_H__
#define __SAI_FDB_MAIN_H__
#include "saistatus.h"
#include "saitypes.h"
#include "saifdb.h"
#include "sai_fdb_api.h"

#define SAI_FDB_MAX_FD 2
#define SAI_FDB_READ_FD 0
#define SAI_FDB_WRITE_FD 1

sai_status_t sai_l2_fdb_set_aging_time(uint32_t value);

sai_status_t sai_l2_fdb_get_aging_time(uint32_t *value);

sai_status_t sai_l2_fdb_register_callback(sai_fdb_event_notification_fn
                                                         fdb_notification_fn);

sai_status_t sai_l2_fdb_set_switch_max_learned_address(uint32_t value);

sai_status_t sai_l2_fdb_get_switch_max_learned_address(uint32_t *value);

sai_status_t sai_l2_fdb_ucast_miss_action_set(const sai_attribute_t *attr);

sai_status_t sai_l2_fdb_ucast_miss_action_get(sai_attribute_t *attr);

sai_status_t sai_l2_fdb_mcast_miss_action_set(const sai_attribute_t *attr);

sai_status_t sai_l2_fdb_mcast_miss_action_get(sai_attribute_t *attr);

sai_status_t sai_l2_fdb_bcast_miss_action_set(const sai_attribute_t *attr);

sai_status_t sai_l2_fdb_bcast_miss_action_get(sai_attribute_t *attr);

sai_status_t sai_l2_mcast_cpu_flood_enable_set(bool enable);

sai_status_t sai_l2_mcast_cpu_flood_enable_get(bool *enable);

sai_status_t sai_l2_bcast_cpu_flood_enable_set(bool enable);

sai_status_t sai_l2_bcast_cpu_flood_enable_get(bool *enable);

sai_status_t sai_l2_get_fdb_table_size(sai_attribute_t *attr);

sai_status_t sai_l2_register_fdb_entry(const sai_fdb_entry_t *fdb_entry);

sai_status_t sai_l2_deregister_fdb_entry(const sai_fdb_entry_t *fdb_entry);

void sai_l2_fdb_register_internal_callback (sai_fdb_internal_callback_fn
                                                       fdb_internal_callback);

void sai_dump_all_fdb_entry_nodes (void);

void sai_dump_all_fdb_entry_count (void);

void sai_dump_all_fdb_registered_nodes (void);

void sai_dump_fdb_entry_nodes_per_bridge_port (sai_object_id_t bridge_port_id);

void sai_dump_fdb_entry_nodes_per_vlan (sai_vlan_id_t vlan_id);

void sai_dump_fdb_entry_nodes_per_bridge_port_vlan (sai_object_id_t bridge_port_id,
                                             sai_vlan_id_t vlan_id);
#endif
