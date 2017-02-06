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
 * @file sai_switch_db_api.cpp
 *
 * @brief This file contains the function definitions for updating the
 *        SQL DB tables related to the SAI switch object attributes in VM
 *        environment.
 */

extern "C" {
#include "sai_oid_utils.h"
#include "sai_switch_utils.h"
#include "sai_port_utils.h"
#include "sai_l3_util.h"
#include "sai_vlan_api.h"
#include "sai_vm_acl_util.h"
}

#include "sai_vm_db_utils.h"
#include "sai_db_gen_utils.h"
#include "sai_switch_db_api.h"
#include "db_sql_ops.h"
#include "sai_vm_defs.h"
#include "sai_vm_event_log.h"
#include "saitypes.h"
#include "saiswitch.h"
#include "saistatus.h"

#include "ds_common_types.h"
#include "std_mac_utils.h"
#include "std_error_codes.h"
#include "std_assert.h"
#include "std_type_defs.h"
#include <string.h>

#include <map>
#include <vector>
#include <string>

static std::map<uint_t, std::string> switch_attr_db_str_map =
{
    {SAI_SWITCH_ATTR_PORT_NUMBER, "\"total_ports\""},
    {SAI_SWITCH_ATTR_PORT_LIST, "\"port_list\""},
    {SAI_SWITCH_ATTR_CPU_PORT, "\"cpu_port_id\""},
    {SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS, "\"max_vrf\""},
    {SAI_SWITCH_ATTR_FDB_TABLE_SIZE, "\"fdb_table_size\""},
    {SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED, "\"on_link_route_support\""},
    {SAI_SWITCH_ATTR_OPER_STATUS, "\"oper_status\""},
    {SAI_SWITCH_ATTR_MAX_TEMP, "\"max_temperature\""},
    {SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY, "\"table_min_prio\""},
    {SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY, "\"table_max_prio\""},
    {SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY, "\"entry_min_prio\""},
    {SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY, "\"entry_max_prio\""},
    {SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID, "\"stp_instance_id\""},
    {SAI_SWITCH_ATTR_SWITCHING_MODE, "\"switching_mode\""},
    {SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE, "\"bcast_cpu_flood_enable\""},
    {SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE, "\"mcast_cpu_flood_enable\""},
    {SAI_SWITCH_ATTR_SRC_MAC_ADDRESS, "\"src_mac\""},
    {SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES, "\"max_learn_limit\""},
    {SAI_SWITCH_ATTR_FDB_AGING_TIME, "\"aging_time\""},
    {SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION, "\"ucast_miss_pkt_action\""},
    {SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION,
        "\"bcast_miss_pkt_action\""},
    {SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION,
        "\"mcast_miss_pkt_action\""},
    {SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM, "\"algorithm\""},
    {SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED, "\"seed\""},
    {SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM, "\"algorithm\""},
    {SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED, "\"seed\""},
    {SAI_SWITCH_ATTR_ECMP_MEMBERS, "\"max_ecmp_paths\""},
    {SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL, "\"cntr_refresh_interval\""},
};

static std::map<sai_switch_oper_status_t, std::string>
switch_oper_status_str_map =
{
    {SAI_SWITCH_OPER_STATUS_UNKNOWN, "\"UNKNOWN\""},
    {SAI_SWITCH_OPER_STATUS_UP, "\"UP\""},
    {SAI_SWITCH_OPER_STATUS_DOWN, "\"DOWN\""},
    {SAI_SWITCH_OPER_STATUS_FAILED, "\"FAILED\""}
};

static std::map<sai_switch_switching_mode_t, std::string>
switching_mode_str_map =
{
    {SAI_SWITCHING_MODE_CUT_THROUGH, "\"CUT_THROUGH\""},
    {SAI_SWITCHING_MODE_STORE_AND_FORWARD, "\"STORE_AND_FORWARD\""}
};

static std::string sai_switch_oper_status_str_get (
sai_switch_oper_status_t status)
{
    if (switch_oper_status_str_map.find (status) !=
        switch_oper_status_str_map.end()) {
        return (switch_oper_status_str_map [status]);
    }

    return "\"" + std::string ("INVALID") + "\"";
}

static std::string sai_switch_mode_str_get (sai_switch_switching_mode_t mode)
{
    if (switching_mode_str_map.find (mode) != switching_mode_str_map.end()) {
        return (switching_mode_str_map [mode]);
    }

    return "\"" + std::string ("INVALID") + "\"";
}

static sai_status_t sai_switch_attr_db_table_str_get (uint_t attr_id,
                                                      std::string *p_table_str)
{
    STD_ASSERT (p_table_str != NULL);

    switch (attr_id) {
        case SAI_SWITCH_ATTR_PORT_NUMBER:
        case SAI_SWITCH_ATTR_PORT_LIST:
        case SAI_SWITCH_ATTR_CPU_PORT:
            *p_table_str = "SAI_SWITCH_PORT_DFLT_INFO_TABLE";
            break;

        case SAI_SWITCH_ATTR_FDB_TABLE_SIZE:
        case SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID:
            *p_table_str = "SAI_SWITCH_L2_DFLT_INFO_TABLE";
            break;

        case SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS:
        case SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED:
        case SAI_SWITCH_ATTR_ECMP_MEMBERS:
            *p_table_str = "SAI_SWITCH_L3_DFLT_INFO_TABLE";
            break;

        case SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY:
        case SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY:
        case SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY:
        case SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY:
            *p_table_str = "SAI_SWITCH_ACL_DFLT_INFO_TABLE";
            break;

        case SAI_SWITCH_ATTR_OPER_STATUS:
        case SAI_SWITCH_ATTR_MAX_TEMP:
        case SAI_SWITCH_ATTR_SWITCHING_MODE:
        case SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL:
            *p_table_str = "SAI_SWITCH_MISC_INFO_TABLE";
            break;

        case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
        case SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES:
        case SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE:
        case SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE:
            *p_table_str = "SAI_SWITCH_L2_PARAMS_TABLE";
            break;

        case SAI_SWITCH_ATTR_FDB_AGING_TIME:
        case SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION:
        case SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION:
        case SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION:
            *p_table_str = "SAI_SWITCH_FDB_PARAMS_TABLE";
            break;

        default:
            return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_switch_attr_db_field_str_get (uint_t attr_id,
                                                      std::string *p_field_str)
{
    STD_ASSERT (p_field_str != NULL);

    if (switch_attr_db_str_map.find (attr_id) != switch_attr_db_str_map.end()) {
        *p_field_str = switch_attr_db_str_map [attr_id];

        return SAI_STATUS_SUCCESS;
    }

    return SAI_STATUS_FAILURE;
}

static sai_status_t sai_switch_attr_db_value_str_get (
const sai_attribute_t *p_attr, std::string *p_value_str)
{
    char   buf [SAI_VM_MAX_BUFSZ];
    uint_t npu_obj_id = 0;

    STD_ASSERT (p_attr != NULL);
    STD_ASSERT (p_value_str != NULL);

    memset (buf, 0, sizeof (buf));

    switch (p_attr->id) {
        case SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED:
        case SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE:
        case SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE:
            *p_value_str = ((p_attr->value.booldata)? "1" : "0");
            break;

        case SAI_SWITCH_ATTR_PORT_NUMBER:
        case SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS:
        case SAI_SWITCH_ATTR_FDB_TABLE_SIZE:
        case SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY:
        case SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY:
        case SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY:
        case SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY:
        case SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES:
        case SAI_SWITCH_ATTR_FDB_AGING_TIME:
        case SAI_SWITCH_ATTR_ECMP_MEMBERS:
        case SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL:
            *p_value_str = std::to_string (p_attr->value.u32);
            break;

        case SAI_SWITCH_ATTR_MAX_TEMP:
            *p_value_str = std::to_string (p_attr->value.s32);
            break;

        case SAI_SWITCH_ATTR_OPER_STATUS:
            *p_value_str = sai_switch_oper_status_str_get
                ((sai_switch_oper_status_t)p_attr->value.s32);
            break;

        case SAI_SWITCH_ATTR_SWITCHING_MODE:
            *p_value_str = sai_switch_mode_str_get
                ((sai_switch_switching_mode_t)p_attr->value.s32);
            break;

        case SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION:
        case SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION:
        case SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION:
            *p_value_str =
                sai_packet_action_str ((sai_packet_action_t)p_attr->value.s32);

            (*p_value_str) = "\"" + *p_value_str + "\"";
            break;

        case SAI_SWITCH_ATTR_CPU_PORT:
        case SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID:
            npu_obj_id = (uint_t) sai_uoid_npu_obj_id_get (p_attr->value.oid);
            *p_value_str = std::to_string (npu_obj_id);
            break;

        case SAI_SWITCH_ATTR_PORT_LIST:
            *p_value_str = sai_vm_obj_list_str_get (&p_attr->value.objlist);
            break;

        case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
            std_mac_to_string ((const hal_mac_addr_t *) p_attr->value.mac,
                               buf, sizeof (buf));

             (*p_value_str) = "\"" + std::string (buf) + "\"";
            break;

        default:
            return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_switch_port_dflt_info_table_init (
sai_switch_id_t switch_id)
{
    std::string switch_id_str = std::to_string(switch_id);
    std::string table_str = "SAI_SWITCH_PORT_DFLT_INFO_TABLE";

    uint_t cpu_port_id = sai_switch_get_cpu_port ();
    std::string cpu_port_id_str = std::to_string (cpu_port_id);

    uint_t total_ports = sai_switch_get_max_lport ();
    std::string total_ports_str = std::to_string (total_ports);

    /* Fill in the port objects in the list and get the port list string */
    sai_object_list_t port_obj_list;
    sai_object_id_t port_objects [total_ports];
    port_obj_list.list = port_objects;

    sai_port_logical_list_get (&port_obj_list);

    std::string port_list_str = sai_vm_obj_list_str_get (&port_obj_list);

    std::string insert_str =
        std::string ("( ") + switch_id_str + ", " + total_ports_str + ", " +
        cpu_port_id_str + ", " + port_list_str + std::string(")");

    if (db_sql_insert (sai_vm_get_db_handle(), table_str.c_str(),
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting switch DB entry for switch ID: %s,"
                           " table: %s, insert string: %s.",
                           switch_id_str.c_str(), table_str.c_str(),
                           insert_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_switch_l2_dflt_info_table_init (
sai_switch_id_t switch_id)
{
    std::string switch_id_str = std::to_string(switch_id);
    std::string table_str = "SAI_SWITCH_L2_DFLT_INFO_TABLE";

    /* TODO: Get using sai-common-utils API once available. */
    uint_t fdb_table_size = SAI_VM_FDB_TABLE_SIZE;
    std::string fdb_table_size_str = std::to_string (fdb_table_size);

    /* TODO: Get using sai-common-utils API once available. */
    uint_t stp_instance_id = SAI_VM_DFLT_STP_INSTANCE_ID;
    std::string stp_instance_id_str = std::to_string (stp_instance_id);

    std::string insert_str = std::string ("( ") + switch_id_str + ", " +
        fdb_table_size_str + ", " + stp_instance_id_str + std::string(")");

    if (db_sql_insert (sai_vm_get_db_handle(), table_str.c_str(),
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting switch DB entry for switch ID: %s,"
                           " table: %s, insert string: %s.",
                           switch_id_str.c_str(), table_str.c_str(),
                           insert_str.c_str());

        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_switch_l3_dflt_info_table_init (
sai_switch_id_t switch_id)
{
    uint_t max_vrf = 0;
    uint_t max_ecmp_paths = 0;

    std::string switch_id_str = std::to_string(switch_id);
    std::string table_str = "SAI_SWITCH_L3_DFLT_INFO_TABLE";

    sai_switch_max_virtual_routers_get (&max_vrf);
    std::string max_vrf_str = std::to_string (max_vrf);

    sai_switch_max_ecmp_paths_get (&max_ecmp_paths);
    std::string max_ecmp_paths_str = std::to_string (max_ecmp_paths);

    /* TODO: Get using sai-common-utils API once available. */
    std::string on_link_rt_support_str = "0";

    std::string insert_str = std::string ("( ") + switch_id_str + ", " +
        max_vrf_str + ", " + max_ecmp_paths_str + ", " +
        on_link_rt_support_str + std::string(")");

    if (db_sql_insert (sai_vm_get_db_handle(), table_str.c_str(),
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting switch DB entry for switch ID: %s,"
                           " table: %s, insert string: %s.",
                           switch_id_str.c_str(), table_str.c_str(),
                           insert_str.c_str());

        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_switch_acl_dflt_info_table_init (
sai_switch_id_t switch_id)
{
    std::string switch_id_str = std::to_string(switch_id);
    std::string table_str = "SAI_SWITCH_ACL_DFLT_INFO_TABLE";

    sai_attribute_t attr;

    attr.id = SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY;
    sai_npu_get_acl_attribute(&attr);
    std::string table_min_prio_str = std::to_string(attr.value.u32);

    attr.id = SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY;
    sai_npu_get_acl_attribute(&attr);
    std::string table_max_prio_str = std::to_string(attr.value.u32);

    attr.id = SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY;
    sai_npu_get_acl_attribute(&attr);
    std::string entry_min_prio_str = std::to_string(attr.value.u32);

    attr.id = SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY;
    sai_npu_get_acl_attribute(&attr);
    std::string entry_max_prio_str = std::to_string(attr.value.u32);

    std::string insert_str = std::string ("( ") + switch_id_str + ", " +
        table_min_prio_str + ", " + table_max_prio_str + ", " +
        entry_min_prio_str + ", " + entry_max_prio_str + std::string(")");

    if (db_sql_insert (sai_vm_get_db_handle(), table_str.c_str(),
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting switch DB entry for switch ID: %s,"
                           " table: %s, insert string: %s.",
                           switch_id_str.c_str(), table_str.c_str(),
                           insert_str.c_str());

        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_switch_misc_info_table_init (
sai_switch_id_t switch_id)
{
    sai_status_t                sai_rc = SAI_STATUS_SUCCESS;
    sai_switch_oper_status_t    oper_state = SAI_SWITCH_OPER_STATUS_UNKNOWN;
    sai_switch_switching_mode_t mode = SAI_SWITCHING_MODE_STORE_AND_FORWARD;

    std::string switch_id_str = std::to_string(switch_id);
    std::string table_str = "SAI_SWITCH_MISC_INFO_TABLE";

    sai_rc = sai_npu_switching_mode_get (&mode);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error getting switching mode for switch ID: %d.",
                           switch_id);

        return sai_rc;
    }

    std::string mode_str = sai_switch_mode_str_get (mode);

    sai_rc = sai_switch_oper_status_get (&oper_state);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error getting oper status for switch ID: %d.",
                           switch_id);

        return sai_rc;
    }

    std::string oper_status_str = sai_switch_oper_status_str_get (oper_state);

    uint_t cntr_interval = sai_switch_counter_refresh_interval_get ();
    std::string cntr_refresh_interval_str = std::to_string (cntr_interval);

    std::string max_temp_str = std::to_string (SAI_VM_SWITCH_TEMP);

    std::string insert_str = std::string ("( ") + switch_id_str + ", " +
        oper_status_str + ", " + mode_str + ", " + cntr_refresh_interval_str +
        ", " + max_temp_str + std::string(")");

    if (db_sql_insert (sai_vm_get_db_handle(), table_str.c_str(),
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting switch DB entry for switch ID: %s,"
                           " table: %s, insert string: %s.",
                           switch_id_str.c_str(), table_str.c_str(),
                           insert_str.c_str());

        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_switch_l2_params_table_init (
sai_switch_id_t switch_id)
{
    char               buf [SAI_VM_MAX_BUFSZ];
    std::string        switch_id_str = std::to_string(switch_id);
    std::string        table_str = "SAI_SWITCH_L2_PARAMS_TABLE";
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get ();

    memset (buf, 0, sizeof (buf));

    std::string max_learn_limit_str =
        std::to_string (sai_switch_info_ptr->max_mac_learn_limit);
    std::string bcast_cpu_flood_enable_str =
        std::to_string (sai_switch_info_ptr->isBcastCpuFloodEnable);
    std::string mcast_cpu_flood_enable_str =
        std::to_string (sai_switch_info_ptr->isMcastCpuFloodEnable);

    std_mac_to_string ((const hal_mac_addr_t *)
                       sai_switch_info_ptr->switch_mac_addr, buf, sizeof (buf));

    std::string src_mac_str = "\"" + std::string (buf) + "\"";

    std::string insert_str = std::string ("( ") + switch_id_str + ", " +
        src_mac_str + ", " + max_learn_limit_str +
        ", " + bcast_cpu_flood_enable_str + ", " + mcast_cpu_flood_enable_str +
        std::string(")");

    if (db_sql_insert (sai_vm_get_db_handle(), table_str.c_str(),
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting switch DB entry for switch ID: %s,"
                           " table: %s, insert string: %s.",
                           switch_id_str.c_str(), table_str.c_str(),
                           insert_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_switch_fdb_params_table_init (
sai_switch_id_t switch_id)
{
    std::string        pkt_action_str;
    std::string        switch_id_str = std::to_string(switch_id);
    std::string        table_str = "SAI_SWITCH_FDB_PARAMS_TABLE";
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get ();

    std::string aging_time_str =
        std::to_string (sai_switch_info_ptr->fdb_aging_time);

    std::string ucast_miss_pkt_action_str =
        sai_packet_action_str (sai_switch_info_ptr->fdbUcastMissPktAction);
    ucast_miss_pkt_action_str = "\"" + ucast_miss_pkt_action_str + "\"";

    std::string bcast_miss_pkt_action_str =
        sai_packet_action_str (sai_switch_info_ptr->fdbBcastMissPktAction);
    bcast_miss_pkt_action_str = "\"" + bcast_miss_pkt_action_str + "\"";

    std::string mcast_miss_pkt_action_str =
        sai_packet_action_str (sai_switch_info_ptr->fdbMcastMissPktAction);
    mcast_miss_pkt_action_str = "\"" + mcast_miss_pkt_action_str + "\"";

    std::string insert_str = std::string ("( ") + switch_id_str + ", " +
        aging_time_str + ", " + ucast_miss_pkt_action_str + ", " +
        bcast_miss_pkt_action_str + ", " + mcast_miss_pkt_action_str +
        std::string(")");

    if (db_sql_insert (sai_vm_get_db_handle(), table_str.c_str(),
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting switch DB entry for switch ID: %s,"
                           " table: %s, insert string: %s.",
                           switch_id_str.c_str(), table_str.c_str(),
                           insert_str.c_str());

        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

static std::vector<sai_vm_switch_db_table_init> switch_db_table_init_functions =
{
    sai_vm_switch_port_dflt_info_table_init,
    sai_vm_switch_l2_dflt_info_table_init,
    sai_vm_switch_l3_dflt_info_table_init,
    sai_vm_switch_acl_dflt_info_table_init,
    sai_vm_switch_misc_info_table_init,
    sai_vm_switch_l2_params_table_init,
    sai_vm_switch_fdb_params_table_init
};

sai_status_t sai_vm_switch_db_init (sai_switch_id_t switch_id)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    size_t       ix = 0;
    size_t       mx = switch_db_table_init_functions.size();

    for (ix = 0; ix < mx; ix++) {
        sai_rc = (switch_db_table_init_functions [ix])(switch_id);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }
    }

    return sai_rc;
}

sai_status_t sai_switch_attribute_set_db_entry (sai_switch_id_t switch_id,
                                                const sai_attribute_t *p_attr)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    std::string  table_str;
    std::string  attr_str;
    std::string  value_str;

    STD_ASSERT (p_attr != NULL);

    std::string switch_id_str = std::to_string(switch_id);
    std::string cond_str =
        std::string ("( switch_id=") + switch_id_str + std::string (")");;

    /* Get the switch DB table name for the given attribute */
    sai_rc = sai_switch_attr_db_table_str_get (p_attr->id, &table_str);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error getting the switch DB table name for "
                           "switch ID: %d, attribute ID: %d.", switch_id,
                           p_attr->id);

        return sai_rc;
    }

    /* Get the switch DB field name for the given attribute */
    sai_rc = sai_switch_attr_db_field_str_get (p_attr->id, &attr_str);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error getting the switch DB field name for "
                           "switch ID: %d, attribute ID: %d on table: %s.",
                           switch_id, p_attr->id, table_str.c_str());

        return sai_rc;
    }

    /* Get the value string to be inserted into DB for the given attribute */
    sai_rc = sai_switch_attr_db_value_str_get (p_attr, &value_str);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error getting the switch DB value string for switch"
                           "ID: %d, attribute ID: %d, attribute name: %s on "
                           "table: %s.", switch_id, p_attr->id,
                           attr_str.c_str(), table_str.c_str());

        return sai_rc;
    }

    if (db_sql_set_attribute (sai_vm_get_db_handle(), table_str.c_str(),
                              attr_str.c_str(), value_str.c_str(),
                              cond_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error setting switch DB entry for switch ID: %s on"
                           " table: %s with attr: %s, value: %s.",
                           switch_id_str.c_str(), table_str.c_str(),
                           attr_str.c_str(), value_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return sai_rc;
}
