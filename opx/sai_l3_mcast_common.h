/************************************************************************
* * LEGALESE:   "Copyright (c) 2019, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/***
 * \file    sai_l3_mcast_common.h
 *
 * \brief  MCAST Data Structures. To be interally used by SAI(for IPMC entry's DB management)
*/


/** \defgroup SAIL3MCAST COMMON - SAI MCAST Data structures and common definitions
* To be internally used by SAI, for housekeeping of IPMC entry DB
*
* \{
*/


#if !defined (__SAIL3MCASTCOMMON_H_)
#define __SAIL3MCASTCOMMON_H_

#include "saitypes.h"
#include "std_radix.h"
#include "std_radical.h"
#include "saiswitch.h"
#include "saiipmc.h"

/** Multicast entry type*/
typedef enum _dn_sai_l3_mcast_entry_type {
    /* S,G Entry */
    SAI_L3_MCAST_ENTRY_TYPE_SG,
    /* X,G Entry */
    SAI_L3_MCAST_ENTRY_TYPE_XG,
}dn_sai_l3_mcast_entry_type;

/** MCAST Entry key: Key used to save MCAST entry in local DB*/
typedef struct _dn_sai_l3_mcast_entry_key_t {
    /*NPU id*/
    sai_object_id_t             switch_id;
    /*VRF Id*/
    sai_object_id_t              vrf_id;
    /*Type: Multiast entry type*/
    dn_sai_l3_mcast_entry_type        entry_type;
    /*Destination Group address*/
    sai_ip_address_t             grp_addr;
    /*Source: Mulitcast source id*/
    sai_ip_address_t             src_addr;
}dn_sai_l3_mcast_entry_key_t;

/** Layer 3 MCAST Entry Node: The full MCAST node structure*/
typedef struct _dn_sai_l3_mcast_entry_node_t {
    /*mcast_rt_head: Radix tree head*/
    std_rt_head                 mcast_rt_head;
    /*mcast_key: Key for the MCAST node*/
    dn_sai_l3_mcast_entry_key_t       mcast_key;
    /*group_id: IPMC group id*/
    sai_object_id_t             mcast_ipmc_group_id;
    /*group_id: IPMC group id*/
    sai_object_id_t             mcast_rpf_group_id;
    /*action: MCAST action - Forward/Trap/Log/Drop*/
    sai_packet_action_t         action;
    /*toggle cpu port : Flag to indicate inclusion/exclusion of CPU Port*/
    bool   toggle_cpu_port;
}dn_sai_l3_mcast_entry_node_t;

typedef struct _dn_sai_l3_mcast_global_data_t {
    /*sai_global_mcast_tree: Multicast entry global tree*/
    std_rt_table       *sai_global_mcast_tree;
    /*mcast_marker: Dummy*/
    std_radical_ref_t  mcast_marker;
} dn_sai_l3_mcast_global_data_t;

#define SAI_L3_MCAST_ENTRY_KEY_SIZE (sizeof(dn_sai_l3_mcast_entry_key_t)*8)

/** Logging utility for SAI MCAST API */
#define SAI_L3_MCAST_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_IPMC, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_IPMC, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI MCAST API */
#define SAI_L3_MCAST_LOG_TRACE(msg, ...) \
        SAI_L3_MCAST_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_L3_MCAST_LOG_CRIT(msg, ...) \
        SAI_L3_MCAST_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_L3_MCAST_LOG_ERR(msg, ...) \
        SAI_L3_MCAST_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_L3_MCAST_LOG_INFO(msg, ...) \
        SAI_L3_MCAST_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_L3_MCAST_LOG_WARN(msg, ...) \
        SAI_L3_MCAST_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_L3_MCAST_LOG_NTC(msg, ...) \
        SAI_L3_MCAST_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

#define SAI_L3_MCAST_LOG_LVL(lvl, msg, ...) { \
        if (lvl == SAI_ERR_SUBLVL) SAI_L3_MCAST_LOG_ERR ( msg, ##__VA_ARGS__); \
        if (lvl == SAI_TRACE_SUBLVL) SAI_L3_MCAST_LOG_TRACE ( msg, ##__VA_ARGS__); \
        }
#endif
/**
\}
*/
