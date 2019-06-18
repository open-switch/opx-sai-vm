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
 * \file    sai_fdb_common.h
 *
 * \brief  FDB Data Structures. To be internally used by SAI
*/


/** \defgroup SAIFDB COMMON - SAI FDB Data structures and common definitions
* To be internally used by SAI
*
* \{
*/


#if !defined (__SAIFDBCOMMON_H_)
#define __SAIFDBCOMMON_H_

#include "saitypes.h"
#include "saiswitch.h"
#include "saifdb.h"
#include "std_radix.h"
#include "std_radical.h"
#include "sai_event_log.h"

/** FDB Entry key: Key used to save FDB entry in cache*/
typedef struct _sai_fdb_entry_key_t {

    /*bv_id: represents the bridge or VLAN identifier of the entry*/
    sai_object_id_t bv_id;

    /*mac_address: SAI Format MAC Address*/
    sai_mac_t       mac_address;

}sai_fdb_entry_key_t;

/** FDB Entry Node: The full FDB node structure*/
typedef struct _sai_fdb_entry_node_t {

    /*fdb_rt_head: Radix tree head*/
    std_rt_head          fdb_rt_head;

    /*fdb_key: Key for the FDB node*/
    sai_fdb_entry_key_t  fdb_key;

    /*bridge_port_id: Bridge Port on which FDB entry is learnt*/
    sai_object_id_t      bridge_port_id;

    /*entry_type: Type of the entry either static or dynamic*/
    sai_fdb_entry_type_t entry_type;

    /*action: FDB action - Forward/Trap/Log/Drop*/
    sai_packet_action_t  action;

    /* end_point_ip: End point IP address in case of bridge port type tunnel */
    sai_ip_address_t     end_point_ip;

    /* metadata: FDB Meta Data */
    uint_t               metadata;

    /* is_pending_entry: True if the entry is pending*/
    bool                 is_pending_entry;

}sai_fdb_entry_node_t;

/** FDB Registered Node: The full FDB registered node structure*/
typedef struct _sai_fdb_registered_node_t {

    /*fdb_radical_head: Radical tree head*/
    std_radical_head_t  fdb_radical_head;

    /*fdb_key: Key for the FDB node*/
    sai_fdb_entry_key_t fdb_key;

    /*bridge_port_id: Bridge Port on which FDB entry is learnt*/
    sai_object_id_t     bridge_port_id;

    /*node_in_cl: If the node is currently in the changelist*/
    bool                node_in_cl;

    /*fdb_event: FDB event associated with the node*/
    sai_fdb_event_t     fdb_event;

} sai_fdb_registered_node_t;

/** FDB Internal notification data: Data passed in notifications internal to other SAI modules*/
typedef struct _sai_fdb_internal_notification_data_t {

    /*fdb_entry: FDB entry*/
    sai_fdb_entry_t     fdb_entry;

    /*bridge_port_id: Bridge Port on which FDB entry is learnt*/
    sai_object_id_t     bridge_port_id;

    /*fdb_event: FDB event associated with the data*/
    sai_fdb_event_t     fdb_event;

} sai_fdb_internal_notification_data_t;

/** FDB event data: Data generated as part of a FDB event*/
typedef struct _sai_fdb_event_data_t {

    /* Data for notifying to registered modules */
    sai_fdb_event_notification_data_t *notification_data;

    /* is_pending_entry: True if the entry is pending*/
    bool                               is_pending_entry;

} sai_fdb_event_data_t;

typedef struct _sai_fdb_global_data_t {

    /*sai_global_fdb_tree: FDB entry global tree*/
    std_rt_table       *sai_global_fdb_tree;

    /*sai_registered_fdb_entry_tree: Tree containing registered FDB entries*/
    std_rt_table       *sai_registered_fdb_entry_tree;

    /*fdb_notification_marker: Marker node for changelist in registered FDB entry tree*/
    std_radical_ref_t  fdb_marker;

    /*num_notifications: Number of notifications pending to be sent*/
    uint_t             num_notifications;

    /*cur_notification_idx: Current number of notification be added to array by notification thread*/
    uint_t             cur_notification_idx;

} sai_fdb_global_data_t;

static inline bool sai_fdb_entry_is_forwarding_action(sai_packet_action_t action)
{
    return ((action == SAI_PACKET_ACTION_FORWARD) ||
            (action == SAI_PACKET_ACTION_COPY)    ||
            (action == SAI_PACKET_ACTION_LOG)     ||
            (action == SAI_PACKET_ACTION_TRANSIT));
}

#define SAI_FDB_ENTRY_KEY_SIZE (sizeof(sai_fdb_entry_key_t)*8)

#define SAI_MAX_FDB_ATTRIBUTES 5
#define SAI_MAC_NUM_BYTES 6
#define SAI_MAC_NUM_CHAR_PER_BYTE 3
#define SAI_MAC_STR_LEN (SAI_MAC_NUM_CHAR_PER_BYTE*SAI_MAC_NUM_BYTES)
#define SAI_FDB_LEARN_LIMIT_DISABLE 0
#define SAI_FDB_MAX_NOTIFICATION_NODES 50
#define SAI_FDB_MAX_MACS_PER_CALLBACK 1000

/** Logging utility for SAI FDB API */
#define SAI_FDB_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_FDB, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_FDB, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI FDB API */
#define SAI_FDB_LOG_TRACE(msg, ...) \
        SAI_FDB_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_FDB_LOG_CRIT(msg, ...) \
        SAI_FDB_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_FDB_LOG_ERR(msg, ...) \
        SAI_FDB_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_FDB_LOG_INFO(msg, ...) \
        SAI_FDB_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_FDB_LOG_WARN(msg, ...) \
        SAI_FDB_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_FDB_LOG_NTC(msg, ...) \
        SAI_FDB_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

#endif
/**
\}
*/
