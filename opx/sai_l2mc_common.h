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
 * \file    sai_l2mc_common.h
 *
 * \brief SAI L2MC data structures.To be interally used by SAI
*/


/** \defgroup SAIL2MC Common - SAI L2MC Data structures and common definitions
* To be internally used by SAI
*
* \{
*/

#if !defined (__SAIL2MCCOMMON_H_)
#define __SAIL2MCCOMMON_H_

#include "saitypes.h"
#include "std_llist.h"
#include "std_struct_utils.h"
#include "sai_event_log.h"

#define SAI_INVALID_L2MC_MEMBER_ID 0

/*L2MC member cache node: RB tree of L2MC members */
typedef struct _dn_sai_l2mc_member_node_t {
    /*l2mc_member_id: L2MC member identifier*/
    sai_object_id_t   l2mc_member_id;
    /*switch_id: Switch identifier*/
    sai_object_id_t   switch_id;
    /*l2mc_id: L2MC identifier*/
    sai_object_id_t   l2mc_group_id;
    /*bridge_port_id: sai port identifier*/
    sai_object_id_t   bridge_port_id;
    /*encap_ip: Encap ip address*/
    sai_ip_address_t  encap_ip;
}dn_sai_l2mc_member_node_t;

/*L2MC port node: A node in linked list*/
typedef struct _dn_sai_l2mc_member_dll_node_t{
    /*node: Linked list node*/
    std_dll node;
    /*l2mc_port: Port and tagging mode*/
    dn_sai_l2mc_member_node_t *l2mc_member_info;
}dn_sai_l2mc_member_dll_node_t;

/*L2MC Global cache node: List of ports in the l2mc*/
typedef struct _dn_sai_l2mc_group_node_t {
    /*port_list: List of ports*/
    std_dll_head     member_list;
    /*l2mc_id: L2MC identifier*/
    sai_object_id_t  l2mc_group_id;
    /*port_cout: Number of ports in the L2MC Group*/
    unsigned int     port_count;
    /* Bridge ID that the multicast group is associated to */
    sai_object_id_t  bridge_id;
    /* Number of flood groups that multicast group is attached to in the bridge*/
    uint_t           bridge_ref_count;
    /* Hardware info associated with the l2mc group */
    void            *hw_info;
}dn_sai_l2mc_group_node_t;

/** Logging utility for SAI L2MC API */
#define SAI_L2MC_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_L2MC, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_L2MC, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI L2MC API */
#define SAI_L2MC_LOG_TRACE(msg, ...) \
        SAI_L2MC_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_L2MC_LOG_CRIT(msg, ...) \
        SAI_L2MC_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_L2MC_LOG_ERR(msg, ...) \
        SAI_L2MC_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_L2MC_LOG_INFO(msg, ...) \
        SAI_L2MC_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_L2MC_LOG_WARN(msg, ...) \
        SAI_L2MC_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_L2MC_LOG_NTC(msg, ...) \
        SAI_L2MC_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

#endif

/**
\}
*/
