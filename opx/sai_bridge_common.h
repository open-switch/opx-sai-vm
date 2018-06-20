/************************************************************************
* * LEGALESE:   "Copyright (c) 2017, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/***
 * \file    sai_bridge_common.h
 *
 * \brief  Bridge and bridge port Data Structures. To be internally used by SAI
*/


/** \defgroup SAI BRIDGE COMMON - SAI Bridge Data structures and common definitions
* To be internally used by SAI
*
* \{
*/


#if !defined (__SAIBRIDGECOMMON_H_)
#define __SAIBRIDGECOMMON_H_

#include "saitypes.h"
#include "saiswitch.h"
#include "saibridge.h"
#include "sai_event_log.h"
#include "std_rbtree.h"
#include "sai_vlan_common.h"

typedef enum _dn_sai_bridge_flood_type_t {

    /* Flood type unknown unicast */
    SAI_BRIDGE_FLOOD_TYPE_UNKNOWN_UNICAST,

    /* Flood type unknown multicast */
    SAI_BRIDGE_FLOOD_TYPE_UNKNOWN_MULTICAST,

    /* Flood type broadcast */
    SAI_BRIDGE_FLOOD_TYPE_BROADCAST,

    /* Flood type MAX. New flood types must be added above this */
    SAI_BRIDGE_FLOOD_TYPE_MAX

} dn_sai_bridge_flood_type_t;

/**
 * @brief SAI Bridge Data structure
 *
 */
typedef struct _dn_sai_bridge_info_t {

    /* Unique Bridge object ID per Bridge */
    sai_object_id_t                 bridge_id;

    /* Bridge Type - Either 1D or 1Q */
    sai_bridge_type_t               bridge_type;

    /* Maximum number of FDB entries that can be learnt on the bridge */
    sai_uint32_t                    max_learned_address;

    /* Learn disable. By default set to false */
    bool                            learn_disable;

    /* Switch Id that the bridge is part of */
    sai_object_id_t                 switch_obj_id;

    /* Flood control types for individual flood types */
    sai_bridge_flood_control_type_t flood_control[SAI_BRIDGE_FLOOD_TYPE_MAX];

    /* L2MC flood control groups when flood type is SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP */
    sai_object_id_t                 l2mc_flood_group[SAI_BRIDGE_FLOOD_TYPE_MAX];

    /* Number of objects that reference this bridge */
    uint_t                          ref_count;

    /* A void pointer which contains NPU specific hardware info */
    void                           *hw_info;

} dn_sai_bridge_info_t;

/**
 * @brief SAI Bridge port attachment sub port Data structure
 *
 */
typedef struct _dn_sai_bridge_sub_port_t {

    /* VLAN Identifier */
    uint16_t        vlan_id;

    /* SAI port identifier */
    sai_object_id_t port_id;

    /* A pointer to hardware info */
    void *hw_info;

}dn_sai_bridge_sub_port_t;

/**
 * @brief SAI Bridge port attachment port Data structure
 *
 */
typedef struct _dn_sai_bridge_port_t {

    /* SAI port identifier */
    sai_object_id_t port_id;

    /* A pointer to hardware info */
    void *hw_info;

}dn_sai_bridge_port_t;

/**
 * @brief SAI Bridge port attachment RIF Data structure
 *
 */
typedef struct _dn_sai_bridge_router_port_t {

    /* SAI Router interface identifier */
    sai_object_id_t rif_id;

    /* A pointer to hardware info */
    void *hw_info;

}dn_sai_bridge_router_port_t;


/**
 * @brief SAI Bridge port attachment RIF Data structure
 *
 */
typedef struct _dn_sai_bridge_tunnel_port_t {

    /**Tunnel object id*/
    sai_object_id_t  tunnel_oid;

}dn_sai_bridge_tunnel_port_t;

/**
 * @brief SAI Bridge port Data structure
 *
 */
typedef struct _dn_sai_bridge_port_info_t{

    sai_bridge_port_type_t               bridge_port_type;

    /* Unique Bridge port object ID per Bridge port */
    sai_object_id_t                      bridge_port_id;

    /* Unique Bridge port object ID per Bridge port */
    sai_object_id_t                      bridge_id;

    /* Unique Bridge port object ID per Bridge port */
    sai_bridge_port_fdb_learning_mode_t  fdb_learn_mode;

    /* Max learnt FDB address on the FDB port */
    sai_uint32_t                         max_learned_address;

    /* Packet action on learn limit violation */
    sai_packet_action_t                  learn_limit_violation_action;

    /* Admin state of the bridge port */
    bool                                 admin_state;

    /* If ingress filtering is set drop frames with unknown VLANs*/
    bool                                 ingress_filtering;

    /* If egress filtering is set drop frames with unknown VLANs at egress*/
    bool                                 egress_filtering;

    /* Egress tagging mode of the bridge port */
    sai_bridge_port_tagging_mode_t       egr_tagging_mode;

    /* Switch Id that the bridge port is part of */
    sai_object_id_t                      switch_obj_id;

    /* Number of objects that reference this bridge port */
    uint_t                               ref_count;

    /* Number of FDB entries learnt on the port */
    uint_t                               fdb_count;

    union {
        /* Attachment type port */
        dn_sai_bridge_port_t        port;

        /* Attachment type sub port */
        dn_sai_bridge_sub_port_t    sub_port;

        /* Attachment type router port */
        dn_sai_bridge_router_port_t router_port;

        /* Attachment type tunnel port */
        dn_sai_bridge_tunnel_port_t tunnel_port;

    }attachment;

}dn_sai_bridge_port_info_t;

/*Bridge port event: List of events from a bridge port*/
typedef enum _sai_bridge_port_event_t {

    /*Bridge port created during init*/
    SAI_BRIDGE_PORT_EVENT_INIT_CREATE,

    /*Create a bridge port*/
    SAI_BRIDGE_PORT_EVENT_CREATE,

    /*Delete a bridge port*/
    SAI_BRIDGE_PORT_EVENT_REMOVE,

    /*Changes to a lag attached to the bridge port*/
    SAI_BRIDGE_PORT_EVENT_LAG_MODIFY,

} sai_bridge_port_event_t;

/* Bridge port notification data */
typedef struct _sai_bridge_port_notif_t {

    /*Bridge port event type*/
    sai_bridge_port_event_t  event;

    /*LAG object identifier*/
    sai_object_id_t          lag_id;

    /* Add or Remove ports from lag */
    bool                     lag_add_port;

    /* List of ports modified in lag */
    const sai_object_list_t *lag_port_mod_list;

} sai_bridge_port_notif_t;

/**
 * @brief Bridge port event callback
 *
 * @param[in] bridge_port_id Bridge port SAI Object identifier
 * @param[in] data Bridge port notification data
 * @return SAI_STATUS_SUCCESS if successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_bridge_port_event_cb_fn) (
    sai_object_id_t bridge_port_id,
    const sai_bridge_port_notif_t *data
);

#define SAI_BRIDGE_PORT_TYPE_TO_BMP(bridge_port_type)  (1 << bridge_port_type)

#define SAI_IS_BRIDGE_PORT_TYPE_SUBSCRIBED(bridge_port_type, type_bmp) \
        ((1 << bridge_port_type) & type_bmp)

/*Bridge port Callback: List of callbacks registered with BRIDGE port Module*/
typedef struct _sai_bridge_port_cb_t {

    /*List of bridge port types that subscriber is interested in*/
    uint_t     bridge_port_type_bmp;

    /*bridge_port_callback: BRIDGE Event callback*/
    sai_bridge_port_event_cb_fn bridge_port_event_cb;

} sai_bridge_port_cb_t;

/*Bridge types to dump*/
typedef enum _sai_bridge_debug_dump_t {
    /*Dump all VLAN related inforamation */
    SAI_BRIDGE_DEBUG_DUMP_VLAN,

    /*Dump all STP related inforamation */
    SAI_BRIDGE_DEBUG_DUMP_STP,

    /*Dump all Multicast related inforamation */
    SAI_BRIDGE_DEBUG_DUMP_MCAST,
} sai_bridge_debug_dump_t;


/** Logging utility for SAI BRIDGE API */
#define SAI_BRIDGE_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_BRIDGE, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_BRIDGE, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI BRIDGE API */
#define SAI_BRIDGE_LOG_TRACE(msg, ...) \
        SAI_BRIDGE_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_BRIDGE_LOG_CRIT(msg, ...) \
        SAI_BRIDGE_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_BRIDGE_LOG_ERR(msg, ...) \
        SAI_BRIDGE_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_BRIDGE_LOG_INFO(msg, ...) \
        SAI_BRIDGE_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_BRIDGE_LOG_WARN(msg, ...) \
        SAI_BRIDGE_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_BRIDGE_LOG_NTC(msg, ...) \
        SAI_BRIDGE_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

#endif
/**
\}
*/
