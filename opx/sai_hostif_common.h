/************************************************************************
* LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
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
#include "saihostintf.h"

#include "std_type_defs.h"
#include "std_llist.h"
#include "sai_event_log.h"

/** Default CPU queue*/
#define DN_SAI_HOSTIF_DEFAULT_QUEUE (0)

/** Logging utility for SAI Host Interface API */
#define SAI_HOSTIF_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_HOST_INTERFACE, level)) { \
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
    sai_hostif_trap_type_t trap_id;
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
} dn_sai_trap_group_node_t;

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

#endif /*_SAI_HOSTIF_COMMON_H_*/
