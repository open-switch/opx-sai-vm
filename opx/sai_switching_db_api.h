/*
 * Copyright (c) 2016 Dell Inc.
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
 * @file sai_switching_db_api.h
 *
 * @brief This file contains the function prototypes for updating the
 *        SQL DB tables related to the SAI switching objects in VM
 *        environment.
 */

#ifndef __SAI_SWITCHING_DB_API_H__
#define __SAI_SWITCHING_DB_API_H__

#include "saifdb.h"
#include "saivlan.h"
#include "saiswitch.h"
#include "saitypes.h"
#include "db_sql_ops.h"
#include "std_type_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief Create a new entry in the FDB database table
 * @param fdb_entry - SAI FDB entry info which contains MAC address and VLAN.
 * @param port_id   - SAI port object ID for the FDB entry.
 * @param entry_type - Static or dynamic entry type.
 * @param action     - FDB entry packet action.
 * @return sai status code
 */
sai_status_t sai_fdb_create_db_entry (const sai_fdb_entry_t *fdb_entry,
                                      sai_object_id_t port_id,
                                      sai_fdb_entry_type_t entry_type,
                                      sai_packet_action_t action,
                                      uint_t metadata);

/*
 * @brief Set attributes of an existing entry in the FDB database table
 * @param fdb_entry - SAI FDB entry info which contains MAC address and VLAN.
 * @param port_id   - SAI port object ID for the FDB entry.
 * @param entry_type - Static or dynamic entry type.
 * @param action     - FDB entry packet action.
 * @return sai status code
 */
sai_status_t sai_fdb_set_db_entry (const sai_fdb_entry_t *fdb_entry,
                                   sai_object_id_t port_id,
                                   sai_fdb_entry_type_t entry_type,
                                   sai_packet_action_t action,
                                   uint_t metadata);

/*
 * @brief Delete an entry from the FDB database table
 * @param fdb_entry - SAI FDB entry info which contains MAC address and VLAN.
 * @return sai status code
 */
sai_status_t sai_fdb_delete_db_entry (const sai_fdb_entry_t* fdb_entry);

/*
 * @brief Delete the FDB entries for the given port and vlan from the FDB
 * database table
 * @param port_id - SAI port object ID.
 * @param vlan_id - VLAN ID.
 * @param delete_all - Delete both static and dynamic entry types
 * @param flush_type - Entry type to be flushed either static or dynamic or all
 * @return sai status code
 */
sai_status_t sai_fdb_delete_all_db_entries (sai_object_id_t port_id,
                                            sai_vlan_id_t vlan_id, bool delete_all,
                                            sai_fdb_flush_entry_type_t flush_type);


/*
 * @brief Create a new entry in the VLAN database table
 * @param vlan_id - VLAN ID.
 * @return sai status code
 */
sai_status_t sai_vlan_create_db_entry (sai_vlan_id_t vlan_id);

/*
 * @brief Delete an existing entry from the VLAN database table
 * @param vlan_id - VLAN ID.
 * @return sai status code
 */
sai_status_t sai_vlan_delete_db_entry (sai_vlan_id_t vlan_id);

/*
 * @brief Set attributes of an existing entry in the VLAN database table
 * @param vlan_id - VLAN ID.
 * @param p_attr - pointer to the SAI attribute for a VLAN attribute.
 * @return sai status code
 */
sai_status_t sai_vlan_set_db_entry (sai_vlan_id_t vlan_id,
                                    sai_attribute_t *p_attr);

/*
 * @brief Update the list of ports added to the VLAN on the VLAN PORT LIST
 * Database table.
 * @param vlan_id - VLAN ID.
 * @param port_count - number of ports added to the VLAN ID.
 * @param port_list - List of ports added to the VLAN ID.
 * @return sai status code
 */
sai_status_t sai_vlan_add_port_list_to_db_entry (
sai_vlan_id_t vlan_id, uint_t port_count, const sai_vlan_port_t *port_list);

/*
 * @brief Update the list of ports deleted from the VLAN on the VLAN PORT LIST
 * Database table.
 * @param vlan_id - VLAN ID.
 * @param port_count - number of ports deleted from the VLAN ID.
 * @param port_list - List of ports deleted from the VLAN ID.
 * @return sai status code
 */
sai_status_t sai_vlan_delete_port_list_from_db_entry (
sai_vlan_id_t vlan_id, uint_t port_count, const sai_vlan_port_t *port_list);

#ifdef __cplusplus
}
#endif

#endif /* __SAI_SWITCHING_DB_API_H__ */
