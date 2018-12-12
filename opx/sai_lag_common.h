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
 * \file    sai_lag_common.h
 *
 * \brief SAI LAG data structures.To be internally used by SAI
*/


/** \defgroup SAILAG Common - SAI LAG Data structures and common definitions
* To be internally used by SAI
*
* \{
*/

#if !defined (__SAILAGCOMMON_H_)
#define __SAILAGCOMMON_H_

#include "saitypes.h"
#include "std_llist.h"
#include "std_struct_utils.h"
#include "sai_event_log.h"
#include "sai_port_common.h"

/*LAG port node: A link list node in port list in a lag*/
typedef struct _sai_lag_port_node_t {

    /* Node: Linked list node. This MUST be the first field in this structure */
    std_dll         node;

    /* Lag_id SAI format port identifier */
    sai_object_id_t lag_id;

    /* Port_id SAI format port identifier */
    sai_object_id_t port_id;

    /* Member_id SAI format port identifier, returned on member add */
    sai_object_id_t member_id;

    /* Disable traffic collection on port */
    bool            ing_disable;

    /* Disable traffic distribution on port */
    bool            egr_disable;

}sai_lag_port_node_t;

/*LAg node: A linked list node in LAG list*/
typedef struct _sai_lag_node_t {

    /*node: Linked list node*/
    std_dll             node;

    /*lag_id: SAI LAG identifier*/
    sai_object_id_t     sai_lag_id;

    /*port_list: List of ports*/
    std_dll_head        port_list;

    /*port_count: Number of ports in the LAG*/
    unsigned int        port_count;

    /* Default vlan id assigned to untagged packets that ingress on the lag */
    uint16_t            pvid;

    /* Default vlan id priority assigned to untagged packets that ingress on the lag */
    uint8_t             default_vlan_priority;

    /** Drop tagged packets that ingress on the lag*/
    bool                drop_tagged;

    /** Drop untagged packets that ingress on the lag*/
    bool                drop_untagged;

    /** Bridge port object ID that is added to 1Q bridge */
    sai_object_id_t     def_bridge_port_id;

    /** Forwarding mode of the LAG */
    sai_port_fwd_mode_t fwd_mode;

    /* Number of objects referencing LAG Object */
    uint_t              ref_count;
}sai_lag_node_t;

#define SAI_LAG_ID_OFFSET STD_STR_OFFSET_OF(sai_lag_node_t,sai_lag_id)
#define SAI_LAG_ID_SIZE STD_STR_SIZE_OF(sai_lag_node_t,sai_lag_id)
#define SAI_LAG_PORT_ID_OFFSET STD_STR_OFFSET_OF(sai_lag_port_node_t, port_id)
#define SAI_LAG_PORT_ID_SIZE STD_STR_SIZE_OF(sai_lag_port_node_t, port_id)

/** Logging utility for SAI LAG API */
#define SAI_LAG_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_LAG, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_LAG, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI LAG API */
#define SAI_LAG_LOG_TRACE(msg, ...) \
        SAI_LAG_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_LAG_LOG_CRIT(msg, ...) \
        SAI_LAG_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_LAG_LOG_ERR(msg, ...) \
        SAI_LAG_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_LAG_LOG_INFO(msg, ...) \
        SAI_LAG_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_LAG_LOG_WARN(msg, ...) \
        SAI_LAG_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_LAG_LOG_NTC(msg, ...) \
        SAI_LAG_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

#endif
/**
\}
*/
