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
 * \file    sai_mcast_common.h
 *
 * \brief  MCAST Data Structures. To be interally used by SAI
*/


/** \defgroup SAIMCAST COMMON - SAI MCAST Data structures and common definitions
* To be internally used by SAI
*
* \{
*/


#if !defined (__SAIMCASTCOMMON_H_)
#define __SAIMCASTCOMMON_H_

#include "saitypes.h"
#include "std_radix.h"
#include "std_radical.h"
#include "saiswitch.h"

/** Owner flag for mcast entry*/
typedef enum _dn_sai_mcast_entry_owner_flag {
    SAI_MCAST_OWNER_L2MC,
    SAI_MCAST_OWNER_IPMC,
}dn_sai_mcast_entry_owner_flag;

/** Multicast entry type*/
typedef enum _dn_sai_mcast_entry_type {
    /* S,G Entry */
    SAI_MCAST_ENTRY_TYPE_SG,
    /* X,G Entry */
    SAI_MCAST_ENTRY_TYPE_XG,
}dn_sai_mcast_entry_type;

/** MCAST Entry key: Key used to save MCAST entry in cache*/
typedef struct _dn_sai_mcast_entry_key_t {
    /*VRF Id*/
    sai_object_id_t              vrf_id;
    /*Destination Group address*/
    sai_ip_address_t             grp_addr;
    /*bv_id: Bridge/Vlan Object incase of L2mc*/
    sai_object_id_t              bv_id;
    /*Source: Mulitcast source id*/
    sai_ip_address_t             src_addr;
}dn_sai_mcast_entry_key_t;

/** MCAST Entry Node: The full MCAST node structure*/
typedef struct _dn_sai_mcast_entry_node_t {
    /*mcast_rt_head: Radix tree head*/
    std_rt_head                 mcast_rt_head;
    /*mcast_key: Key for the MCAST node*/
    dn_sai_mcast_entry_key_t       mcast_key;
    /*group_id: L2MC/IPMC group id*/
    sai_object_id_t             mcast_group_id;
    /*action: MCAST action - Forward/Trap/Log/Drop*/
    sai_packet_action_t         action;
    /*Owner: Owner for the mulitcast entry*/
    dn_sai_mcast_entry_owner_flag  owner;
    /*Type: Multiast entry type*/
    dn_sai_mcast_entry_type        entry_type;
    /*NPU id*/
    sai_object_id_t             switch_id;
}dn_sai_mcast_entry_node_t;

typedef struct _dn_sai_mcast_global_data_t {
    /*sai_global_mcast_tree: Multicast entry global tree*/
    std_rt_table       *sai_global_mcast_tree;
    /*mcast_marker: Dummy*/
    std_radical_ref_t  mcast_marker;
} dn_sai_mcast_global_data_t;

#define SAI_MCAST_ENTRY_KEY_SIZE (sizeof(dn_sai_mcast_entry_key_t)*8)

/** Logging utility for SAI MCAST API */
#define SAI_MCAST_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_L2MC, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_MCAST, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI MCAST API */
#define SAI_MCAST_LOG_TRACE(msg, ...) \
        SAI_MCAST_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_MCAST_LOG_CRIT(msg, ...) \
        SAI_MCAST_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_MCAST_LOG_ERR(msg, ...) \
        SAI_MCAST_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_MCAST_LOG_INFO(msg, ...) \
        SAI_MCAST_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_MCAST_LOG_WARN(msg, ...) \
        SAI_MCAST_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_MCAST_LOG_NTC(msg, ...) \
        SAI_MCAST_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

#endif
/**
\}
*/
