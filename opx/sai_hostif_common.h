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
* @file sai_hostif_common.h
*
* @brief This file contains the datastructure definitions for SAI HOSTIF
*        module.
*
*************************************************************************/
#ifndef _SAI_HOSTIF_COMMON_H_
#define _SAI_HOSTIF_COMMON_H_

#include "sai.h"
#include "saitypes.h"
#include "saiswitch.h"
#include "saihostif.h"

#include "std_type_defs.h"
#include "std_llist.h"
#include "sai_event_log.h"

/** Default CPU queue*/
#define DN_SAI_HOSTIF_DEFAULT_QUEUE (0)

/** Logging utility for SAI Host Interface API */
#define SAI_HOSTIF_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_HOSTIF, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_HOSTIF, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI Host Interface API */
#define SAI_HOSTIF_LOG_TRACE(msg, ...) \
            SAI_HOSTIF_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_HOSTIF_LOG_INFO(msg, ...) \
                SAI_HOSTIF_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_HOSTIF_LOG_NTC(msg, ...) \
                SAI_HOSTIF_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

#define SAI_HOSTIF_LOG_WARN(msg, ...) \
                SAI_HOSTIF_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_HOSTIF_LOG_ERR(msg, ...) \
            SAI_HOSTIF_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_HOSTIF_LOG_CRIT(msg, ...) \
                SAI_HOSTIF_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

/**
 * @brief Trap node key
 *
 * Holds the key field for trap node. The information in the trap node would
 * be used to configure NPU registers to trap packets to CPU.
 */
typedef struct _dn_sai_trap_key_t {
    /** Trap id acts as the key*/
    sai_object_id_t trap_id;
} dn_sai_trap_key_t;

/**
 * @brief Trap node datastructure
 *
 * Contains the information related to a SAI trap
 */
typedef struct _dn_sai_trap_node_t {
    /** Link to the next trap having the same trap group */
    std_dll             trap_link;
    /** Key for the trap node */
    dn_sai_trap_key_t   key;
    /** Packet action to be enforced for the trap */
    sai_packet_action_t trap_action;
    /** Priority for the trap */
    uint_t              trap_prio;
    /** The list ports on which the trap needs to be applied */
    sai_object_list_t   port_list;
    /** The trap group associated to the trap */
    sai_object_id_t     trap_group;
    /** NPU specific information for the trap */
    void               *npu_trap_info;
} dn_sai_trap_node_t;

/**
 * @brief Trap Group Key
 *
 * Holds the key for trap group
 */
typedef struct _dn_sai_trap_group_key_t {
    /** Trap group object */
    sai_object_id_t trap_group_id;
} dn_sai_trap_group_key_t;

/**
 * @brief Trap group node structure
 *
 * Contains the information related to SAI trap group
 */
typedef struct _dn_sai_trap_group_node_t {
    /** Trap group key */
    dn_sai_trap_group_key_t   key;
    /** Admin state for trap */
    bool                      admin_state;
    /** Trap group priority */
    uint_t                    group_prio;
    /** Cpu Queue associated to the trap group */
    uint_t                    cpu_queue;
    /** Policer object */
    sai_object_id_t           policer_id;
    /** List of traps associated to the trap group */
    std_dll_head              trap_list;
    /** Count of traps associated to the trap group */
    uint_t                    trap_count;
    /** List of traps associated to the trap group */
    std_dll_head              user_def_trap_list;
    /** Count of traps associated to the trap group */
    uint_t                    user_def_trap_count;
} dn_sai_trap_group_node_t;

/**
 * @brief User Defined Trap node key
 *
 * Holds the key field for user defined trap node. The information in the trap
 * node would be used to configure NPU registers to trap packets to CPU.
 */
typedef struct _dn_sai_user_def_trap_key_t {
    /** ((User defined trap type << DN_HOSTIF_USER_DEF_TRAP_TYPE_SHIFT)
     *  | NPU trap id) acts as the key
     */
    sai_object_id_t user_def_trap_id;
} dn_sai_user_def_trap_key_t;

/**
 * @brief User Defined Trap node datastructure
 *
 * Contains the information related to a SAI User Defined trap
 */
typedef struct _dn_sai_user_def_trap_node_t {
    /** Link to the next trap having the same trap group */
    std_dll             trap_link;
    /** Key for the trap node */
    dn_sai_user_def_trap_key_t   key;
    /** Priority for the trap */
    uint_t              trap_prio;
    /** The trap group associated to the trap */
    sai_object_id_t     trap_group;
} dn_sai_user_def_trap_node_t;

/**
 * @brief HostIf Operations
 *
 * Represents the general hostif operations
 */
typedef enum _dn_sai_hostif_op_t {
    /** Create operation*/
    DN_SAI_HOSTIF_CREATE = 1,
    /** Remove operation*/
    DN_SAI_HOSTIF_REMOVE = 2,
    /** Set operation*/
    DN_SAI_HOSTIF_SET = 3,
    /** Get operation*/
    DN_SAI_HOSTIF_GET = 4
} dn_sai_hostif_op_t;

/**
 * @brief HostIf debug direction in logging
 *
 * Represents the direction to log in hostif
 */
typedef enum _sai_hostif_debug_log_dir_t {
    SAI_HOSTIF_DEBUG_LOG_DIR_RX,
    SAI_HOSTIF_DEBUG_LOG_DIR_TX,
    SAI_HOSTIF_DEBUG_LOG_DIR_BOTH,
} sai_hostif_debug_log_dir_t;


/**
 * @brief HostIf debug attribute to be set
 *
 * Debug attributes in hostif
 */
typedef enum _sai_hostif_debug_attr_t {
    SAI_HOSTIF_DEBUG_ATTR_LOG,
    SAI_HOSTIF_DEBUG_ATTR_DIR,
    SAI_HOSTIF_DEBUG_ATTR_PORT,
} sai_hostif_debug_attr_t;

#define SAI_HOSTIF_DEBUG_PORT_ALL  (-1)
#endif /*_SAI_HOSTIF_COMMON_H_*/
