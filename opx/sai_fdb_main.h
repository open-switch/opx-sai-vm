/************************************************************************
* * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
*************************************************************************/
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
