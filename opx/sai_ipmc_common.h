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
 * \file    sai_ipmc_common.h
 *
 * \brief SAI IPMC data structures.To be interally used by SAI
*/


/** \defgroup SAIIPMC Common - SAI IPMC Data structures and common definitions
* To be internally used by SAI, to be used for housekeep of internal DB of IPMC Group
* RPF Group , IPMC Group Members and RPF Group Members
*
* \{
*/

#if !defined (__SAIIPMCCOMMON_H_)
#define __SAIIPMCCOMMON_H_

#include "saitypes.h"
#include "std_llist.h"
#include "std_struct_utils.h"
#include "sai_event_log.h"
#include "saiipmcextensions.h"

#define SAI_INVALID_IPMC_MEMBER_ID 0

/*IPMC RPF Group member cache node: RB tree of IPMC RPF Group members */
typedef struct _dn_sai_ipmc_rpf_group_member_node_t {
    /*ipmc_rpf_member_id: IPMC RPF Group member identifier*/
    sai_object_id_t   ipmc_rpf_grp_member_id;
    /*switch_id: Switch identifier*/
    sai_object_id_t   switch_id;
    /*ipmc_rpf_group_id: IPMC RPF Group identifier*/
    sai_object_id_t   ipmc_rpf_group_id;
    /*router_intf_id: sai Router Interface Object identifier*/
    sai_object_id_t   router_intf_id;

    uint32_t          bridge_port_count;

    sai_object_id_t   *bridge_port;

    /* Hardware info associated with the ipmc group */
    void            *hw_info;
}dn_sai_ipmc_rpf_group_member_node_t;

/*IPMC port node: A node in linked list*/
typedef struct _dn_sai_ipmc_rpf_group_member_dll_node_t{
    /*node: Linked list node*/
    std_dll node;
    /*l3 ipmc OIF router port: Just the L3 interfaces */
    dn_sai_ipmc_rpf_group_member_node_t *ipmc_rpf_group_member_info;
}dn_sai_ipmc_rpf_group_member_dll_node_t;

/*IPMC RPF Group Global cache node: List of L3 IIFs in the ipmc rpf group */
typedef struct _dn_sai_ipmc_rpf_group_node_t {
    /*member_list: List of IIFs*/
    std_dll_head     member_list;
    /*ipmc_rpf_group_id: IPMC RPF Group identifier*/
    sai_object_id_t  ipmc_rpf_group_id;
    /*ipmc_repl_group_id: IPMC Replication Group identifier*/
    sai_object_id_t  ipmc_repl_group_id;
    /*iif_cout: Number of iifs in the IPMC RPF Group*/
    unsigned int     l3_iif_count;
    /* Hardware info associated with the ipmc RPF group */
    void            *hw_info;
}dn_sai_ipmc_rpf_group_node_t;

/*IPMC member cache node: RB tree of IPMC Group members */
typedef struct _dn_sai_ipmc_group_member_node_t {
    /*ipmc_grp_member_id: IPMC member identifier*/
    sai_object_id_t   ipmc_grp_member_id;
    /*switch_id: Switch identifier*/
    sai_object_id_t   switch_id;
    /*ipmc_group id: IPMC Group Obj identifier*/
    sai_object_id_t   ipmc_group_id;
    /*router_intf_id: sai Router Interface Object identifier*/
    sai_object_id_t   router_intf_id;

    uint32_t          bridge_port_count;

    sai_object_id_t   *bridge_port;

    /* Hardware info associated with the ipmc group */
    void            *hw_info;
}dn_sai_ipmc_group_member_node_t;

/*IPMC port node: A node in linked list*/
typedef struct _dn_sai_ipmc_group_member_dll_node_t{
    /*node: Linked list node*/
    std_dll node;
    /*l3 ipmc OIF router port: Just the L3 interfaces */
    dn_sai_ipmc_group_member_node_t *ipmc_group_member_info;
}dn_sai_ipmc_group_member_dll_node_t;

/*IPMC Global cache node: List of L3 OIFs in the ipmc*/
typedef struct _dn_sai_ipmc_group_node_t {
    /*l3_oif_list: List of OIFs*/
    std_dll_head     member_list;
    /*ipmc_group_id: IPMC Group Obj identifier*/
    sai_object_id_t  ipmc_group_id;
    /*ipmc_repl_group_id: IPMC Replication Group identifier*/
    sai_object_id_t  ipmc_repl_group_id;
    /*oif_cout: Number of oifs in the IPMC Group*/
    unsigned int     l3_oif_count;
    /* Hardware info associated with the ipmc group */
    void            *hw_info;
}dn_sai_ipmc_group_node_t;

/*IPMC Global cache node: List of Sai Replication Group Objects */
typedef struct _dn_sai_ipmc_repl_group_node_t {
    /*ipmc_repl_group_id: IPMC Replication Group Obj identifier*/
    sai_object_id_t  ipmc_repl_group_id;

    /*ipmc_repl_group_owner: IPMC Replication Group Obj owner */
    sai_ipmc_repl_group_owner_t ipmc_repl_group_owner;

    /*ipmc_hw_repl_index : The H/W index which will be used for replication
     *                     in the received VLAN and other L3 Interface */
     int32_t        ipmc_hw_repl_index;

    /*ref_count : The count of ipmcGroup/rpfGroup objects pointing to this
     *            replication object  */
     uint32_t        ref_count;

     uint32_t        owner;
}dn_sai_ipmc_repl_group_node_t;

/** Logging utility for SAI IPMC API */
#define SAI_IPMC_LOG(module, level, msg, ...) \
    do { \
        if (sai_is_log_enabled (module, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_IPMC, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI IPMC API */
#define SAI_IPMC_LOG_TRACE(module, msg, ...) \
        SAI_IPMC_LOG (module, SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_IPMC_LOG_CRIT(module, msg, ...) \
        SAI_IPMC_LOG (module, SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_IPMC_LOG_ERR(module, msg, ...) \
        SAI_IPMC_LOG (module, SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_IPMC_LOG_INFO(module, msg, ...) \
        SAI_IPMC_LOG (module, SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_IPMC_LOG_WARN(module, msg, ...) \
        SAI_IPMC_LOG (module, SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_IPMC_LOG_NTC(module, msg, ...) \
        SAI_IPMC_LOG (module, SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

#define SAI_IPMC_LOG_LVL(module, lvl, msg, ...) { \
        if (lvl == SAI_ERR_SUBLVL) SAI_IPMC_LOG_ERR ( module, msg, ##__VA_ARGS__); \
        if (lvl == SAI_TRACE_SUBLVL) SAI_IPMC_LOG_TRACE ( module, msg, ##__VA_ARGS__); \
        }
#endif

/**
\}
*/
