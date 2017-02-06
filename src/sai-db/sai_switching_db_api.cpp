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
 * @file sai_switching_db_api.cpp
 *
 * @brief This file contains the function definitions for updating the
 *        SQL DB tables related to the SAI switching objects in VM
 *        environment.
 */

#include "sai_vm_db_utils.h"
#include "sai_switching_db_api.h"
#include "db_sql_ops.h"
#include "sai_vm_defs.h"
#include "sai_vm_event_log.h"
#include "sai_oid_utils.h"
#include "saitypes.h"
#include "saifdb.h"
#include "saivlan.h"
#include "saiswitch.h"
#include "saistatus.h"

#include "ds_common_types.h"
#include "std_mac_utils.h"
#include "std_error_codes.h"
#include "std_assert.h"
#include "std_type_defs.h"
#include "sai_vlan_common.h"

#include <map>
#include <string>

using namespace std;

static string sai_fdb_pkt_action_str_get (sai_packet_action_t pkt_action)
{
    std::map<sai_packet_action_t, std::string> pkt_action_str_map =
    {
        {SAI_PACKET_ACTION_DROP, "\"DROP\""},
        {SAI_PACKET_ACTION_FORWARD, "\"FORWARD\""},
        {SAI_PACKET_ACTION_TRAP, "\"TRAP\""},
        {SAI_PACKET_ACTION_LOG, "\"LOG\""}
    };

    if (pkt_action_str_map.find (pkt_action) != pkt_action_str_map.end()) {
        return (pkt_action_str_map [pkt_action]);
    } else {
        return "\"" + string ("INVALID") + "\"";
    }
}

static string sai_vlan_port_tagging_mode_str_get (sai_vlan_tagging_mode_t mode)
{
    std::map<sai_vlan_tagging_mode_t, std::string> tag_mode_str_map =
    {
        {SAI_VLAN_TAGGING_MODE_UNTAGGED, "\"UNTAGGED\""},
        {SAI_VLAN_TAGGING_MODE_TAGGED, "\"TAGGED\""},
        {SAI_VLAN_TAGGING_MODE_PRIORITY_TAGGED, "\"PRIORITY-TAGGED\""}
    };

    if (tag_mode_str_map.find (mode) != tag_mode_str_map.end()) {
        return (tag_mode_str_map [mode]);
    } else {
        return "\"" + string ("INVALID") + "\"";
    }
}

sai_status_t sai_fdb_create_db_entry (const sai_fdb_entry_t *fdb_entry,
                                      sai_object_id_t port_id,
                                      sai_fdb_entry_type_t entry_type,
                                      sai_packet_action_t action,
                                      uint_t metadata)
{
    char   mac_addr [SAI_VM_MAX_BUFSZ];
    uint_t npu_port_id = (uint_t) sai_uoid_npu_obj_id_get (port_id);

    std_mac_to_string ((const hal_mac_addr_t *)fdb_entry->mac_address,
                       mac_addr, sizeof (mac_addr));

    string mac_addr_str = "\"" + string (mac_addr) + "\"";
    string vlan_id_str = std::to_string (fdb_entry->vlan_id);
    string port_id_str = std::to_string (npu_port_id);
    string is_static_str = (entry_type == SAI_FDB_ENTRY_TYPE_STATIC)? "1" : "0";
    string pkt_action_str = sai_fdb_pkt_action_str_get (action);
    string metadata_str = std::to_string (metadata);

    string insert_str = string ("( ") + mac_addr_str + ", " + vlan_id_str +
        ", " + is_static_str + ", " + port_id_str + ", " + pkt_action_str +
        ", " + metadata_str + string(")");

    if (db_sql_insert (sai_vm_get_db_handle(), "SAI_FDB", insert_str.c_str())
        != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting FDB entry with MAC address: %s and"
                           " vlan: %s.", mac_addr_str.c_str(),
                           vlan_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_fdb_set_db_entry (const sai_fdb_entry_t *fdb_entry,
                                   sai_object_id_t port_id,
                                   sai_fdb_entry_type_t entry_type,
                                   sai_packet_action_t action,
                                   uint_t metadata)
{
    char   mac_addr [SAI_VM_MAX_BUFSZ];
    string attr_str;
    string value_str;
    uint_t attr_list [] = {SAI_FDB_ENTRY_ATTR_TYPE, SAI_FDB_ENTRY_ATTR_PORT_ID,
        SAI_FDB_ENTRY_ATTR_PACKET_ACTION};
    uint_t num_attr = sizeof (attr_list) / sizeof (attr_list [0]);
    uint_t idx = 0;
    uint_t npu_port_id = (uint_t) sai_uoid_npu_obj_id_get (port_id);

    std_mac_to_string ((const hal_mac_addr_t *)fdb_entry->mac_address,
                       mac_addr, sizeof (mac_addr));

    string mac_addr_str = "\"" + string (mac_addr) + "\"";
    string vlan_id_str = std::to_string (fdb_entry->vlan_id);
    string port_id_str = std::to_string (npu_port_id);
    string is_static_str = (entry_type == SAI_FDB_ENTRY_TYPE_STATIC)? "1" : "0";
    string pkt_action_str = sai_fdb_pkt_action_str_get (action);
    string metadata_str = std::to_string (metadata);

    string cond_str = string ("( mac_address=") + mac_addr_str +
        " AND vlan_id =" + vlan_id_str + string(")");

    for (idx = 0; idx < num_attr; idx++) {
        if (attr_list [idx] == SAI_FDB_ENTRY_ATTR_TYPE) {
            attr_str = "IS_STATIC";
            value_str = is_static_str;
        } else if (attr_list [idx] == SAI_FDB_ENTRY_ATTR_PORT_ID) {
            attr_str = "PORT_ID";
            value_str = port_id_str;
        } else if (attr_list [idx] == SAI_FDB_ENTRY_ATTR_PACKET_ACTION) {
            attr_str = "PACKET_ACTION";
            value_str = pkt_action_str;
        } else if (attr_list [idx] == SAI_FDB_ENTRY_ATTR_META_DATA) {
            attr_str = "FDB_METADATA";
            value_str = metadata_str;
        } else {
            SAI_VM_DB_LOG_ERR ("Attribute ID %d is not valid for FDB "
                               "object.", attr_list [idx]);

            continue;
        }

        if (db_sql_set_attribute (sai_vm_get_db_handle(), "SAI_FDB",
                                  attr_str.c_str(), value_str.c_str(),
                                  cond_str.c_str()) != STD_ERR_OK) {
            SAI_VM_DB_LOG_ERR ("Error setting FDB entry with MAC address: %s "
                               "and vlan: %s, attr: %s, value: %s.",
                               mac_addr_str.c_str(), vlan_id_str.c_str(),
                               attr_str.c_str(), value_str.c_str());

            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_fdb_delete_db_entry (const sai_fdb_entry_t* fdb_entry)
{
    char mac_addr [SAI_VM_MAX_BUFSZ];

    std_mac_to_string ((const hal_mac_addr_t *)fdb_entry->mac_address,
                       mac_addr, sizeof (mac_addr));

    string mac_addr_str = "\"" + string (mac_addr) + "\"";
    string vlan_id_str = std::to_string (fdb_entry->vlan_id);

    string delete_str = string ("( mac_address=") + mac_addr_str +
        " AND vlan_id =" + vlan_id_str + string(")");

    if (db_sql_delete (sai_vm_get_db_handle(), "SAI_FDB", delete_str.c_str())
        != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error deleting FDB entry with MAC address: %s and"
                           " vlan: %s.", mac_addr_str.c_str(),
                           vlan_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_fdb_delete_all_db_entries (sai_object_id_t port_id,
                                            sai_vlan_id_t vlan_id, bool flush_all,
                                            sai_fdb_flush_entry_type_t flush_type)
{
    string vlan_id_str = std::to_string (vlan_id);
    uint_t npu_port_id = (uint_t) sai_uoid_npu_obj_id_get (port_id);
    string port_id_str = std::to_string (npu_port_id);
    string is_static_str;
    string delete_str;
    bool system_flush = false;

    if (flush_type == SAI_FDB_FLUSH_ENTRY_TYPE_DYNAMIC) {
        is_static_str = "0";
    } else if (flush_type == SAI_FDB_FLUSH_ENTRY_TYPE_STATIC) {
        is_static_str = "1";
    } else {
        SAI_VM_DB_LOG_ERR ("Invalid entry type :%d",flush_type);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (port_id !=0 ) {
        delete_str += string ("( PORT_ID=") + port_id_str;
        if (vlan_id != VLAN_UNDEF) {
            delete_str += string(" AND vlan_id=") + vlan_id_str;
        }
    } else {
        if (vlan_id != VLAN_UNDEF) {
            delete_str += string("(vlan_id =") + vlan_id_str;
        } else {
            system_flush = true;
        }
    }

    if (!flush_all) {
        if (!system_flush) {
            delete_str += string(" AND");
        } else {
            delete_str += string(" (");
        }
        delete_str += string(" IS_STATIC=") + is_static_str;
    }

    if ((flush_all) && (system_flush)) {
        if (db_sql_delete_all_records (sai_vm_get_db_handle(), "SAI_FDB")
            != STD_ERR_OK) {
            SAI_VM_DB_LOG_ERR ("Error deleting all the FDB entries in the DB.");
            return SAI_STATUS_FAILURE;
        }
    } else {
        delete_str += string(")");
        if (db_sql_delete (sai_vm_get_db_handle(), "SAI_FDB", delete_str.c_str())
            != STD_ERR_OK) {
            SAI_VM_DB_LOG_ERR ("Error deleting FDB entries with port: %s and vlan:"
                               " %s.", port_id_str.c_str(), vlan_id_str.c_str());
            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_vlan_create_db_entry (sai_vlan_id_t vlan_id)
{
    string vlan_id_str = std::to_string (vlan_id);
    string stp_id_str = std::to_string (SAI_VM_DFLT_STP_INSTANCE_ID);
    string max_learn_limit_str = std::to_string (SAI_VM_DFLT_VLAN_MAX_LEARN_LIMIT);
    string learn_disable_str = std::to_string (SAI_VM_DFLT_VLAN_LEARN_DISABLE);
    string vlan_meta_data = std::to_string (SAI_VM_DFLT_VLAN_META_DATA);

    string insert_str = string ("( ") + vlan_id_str + ", " + stp_id_str + ", " +
        max_learn_limit_str + ", " + learn_disable_str + ", " +
        vlan_meta_data + string(")");

    if (db_sql_insert (sai_vm_get_db_handle(), "SAI_VLAN", insert_str.c_str())
        != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting VLAN entry with vlan: %s.",
                           vlan_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_vlan_delete_db_entry (sai_vlan_id_t vlan_id)
{
    string vlan_id_str = std::to_string (vlan_id);

    string delete_str = string ("( vlan_id=") + vlan_id_str + string(")");

    if (db_sql_delete (sai_vm_get_db_handle(), "SAI_VLAN", delete_str.c_str())
        != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error deleting VLAN entry with vlan: %s.",
                           vlan_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_vlan_set_db_entry (sai_vlan_id_t vlan_id,
                                    sai_attribute_t *p_attr)
{
    uint_t stp_instance_id = 0;
    string attr_str;
    string value_str;
    string vlan_id_str = std::to_string (vlan_id);

    STD_ASSERT (p_attr != NULL);

    string cond_str = string ("( vlan_id=") + vlan_id_str + string(")");

    if (p_attr->id == SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES) {
        attr_str = "MAX_LEARN_LIMIT";
        value_str = std::to_string (p_attr->value.u32);
    } else if (p_attr->id == SAI_VLAN_ATTR_STP_INSTANCE) {
        attr_str = "STP_INSTANCE";

        stp_instance_id = (uint_t) sai_uoid_npu_obj_id_get (p_attr->value.oid);

        value_str = std::to_string (stp_instance_id);
    } else if (p_attr->id == SAI_VLAN_ATTR_LEARN_DISABLE) {
        attr_str = "DISABLE_LEARN";
        value_str = std::to_string (p_attr->value.booldata);
    } else if (p_attr->id == SAI_VLAN_ATTR_META_DATA) {
        attr_str = "VLAN_METADATA";
        value_str = std::to_string (p_attr->value.u32);
    } else {
        SAI_VM_DB_LOG_ERR ("Attribute ID %d is not valid for VLAN Object.",
                           p_attr->id);

        return SAI_STATUS_FAILURE;
    }

    if (db_sql_set_attribute (sai_vm_get_db_handle(), "SAI_VLAN",
                              attr_str.c_str(), value_str.c_str(),
                              cond_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error setting VLAN entry with vlan: %s, attr: %s, "
                           "value: %s.", vlan_id_str.c_str(),
                           attr_str.c_str(), value_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_vlan_add_port_list_to_db_entry (
sai_vlan_id_t vlan_id, uint_t port_count, const sai_vlan_port_t *port_list)
{
    uint_t idx = 0;
    uint_t npu_port_id = 0;
    string port_id_str;
    string tag_mode_str;
    string insert_str;

    STD_ASSERT (port_list != NULL);

    string vlan_id_str = std::to_string (vlan_id);

    for (idx = 0; idx < port_count; idx++) {
        npu_port_id =
            (uint_t) sai_uoid_npu_obj_id_get (port_list [idx].port_id);

        port_id_str = std::to_string (npu_port_id);

        tag_mode_str =
            sai_vlan_port_tagging_mode_str_get (port_list [idx].tagging_mode);

        insert_str = string ("( ") + vlan_id_str + ", " + port_id_str + ", " +
            tag_mode_str + string(")");

        if (db_sql_insert (sai_vm_get_db_handle(), "SAI_VLAN_PORT_LIST",
                           insert_str.c_str()) != STD_ERR_OK) {
            SAI_VM_DB_LOG_ERR ("Error inserting VLAN_PORT_LIST entry with "
                               "vlan: %s and port: %s.",
                               vlan_id_str.c_str(), port_id_str.c_str());

            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_vlan_delete_port_list_from_db_entry (
sai_vlan_id_t vlan_id, uint_t port_count, const sai_vlan_port_t *port_list)
{
    uint_t idx = 0;
    uint_t npu_port_id = 0;
    string port_id_str;
    string delete_str;

    STD_ASSERT (port_list != NULL);

    string vlan_id_str = std::to_string (vlan_id);

    for (idx = 0; idx < port_count; idx++) {
        npu_port_id =
            (uint_t) sai_uoid_npu_obj_id_get (port_list [idx].port_id);

        port_id_str = std::to_string (npu_port_id);

        delete_str = string ("( vlan_id=") + vlan_id_str + " AND PORT_ID =" +
            port_id_str + string(")");

        if (db_sql_delete (sai_vm_get_db_handle(), "SAI_VLAN_PORT_LIST",
                           delete_str.c_str()) != STD_ERR_OK) {
            SAI_VM_DB_LOG_ERR ("Error deleting VLAN_PORT_LIST entry with "
                               "vlan: %s and port: %s.",
                               vlan_id_str.c_str(), port_id_str.c_str());

            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}
