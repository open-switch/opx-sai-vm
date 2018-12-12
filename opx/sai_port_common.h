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
* @file sai_port_common.h
*
* @brief This file contains the data structure definitions for SAI Port objects.
*
*************************************************************************/

/* OPENSOURCELICENSE */

#ifndef __SAI_PORT_COMMON_H__
#define __SAI_PORT_COMMON_H__

#include "std_type_defs.h"
#include "std_rbtree.h"

#include "saitypes.h"
#include "saiswitch.h"
#include "saiport.h"
#include "sai_samplepacket_defs.h"
#include "sai_oid_utils.h"
#include "sai_qos_common.h"

/** \defgroup SAIPORTAPI SAI - Port Structures
 *   SAI Port component Data structures and definition
 *
 *  \ingroup SAIAPI
 *  \{
 */

/**
 * @brief NPU Port Id type
 */
typedef uint32_t sai_npu_port_id_t;

/*
 * @brief SAI Port info table handle
 */
typedef rbtree_handle sai_port_info_table_t;

/**
 * @brief Multiplier to Convert SAI port speed value from Gbps to Mbps
 */
#define SAI_SPEED_MB                  (1000)

/*
 * MAX Speeds supported at SAI. Update this on adding new speed.
 */
#define SAI_MAX_SUPPORTED_SPEEDS      (10)

/**
 * @brief SAI port speeds supported value in Mbps
 */
typedef enum _sai_port_speed_t {
    SAI_PORT_SPEED_TEN_MEG         = 10,
    SAI_PORT_SPEED_HUNDRED_MEG     = 100,
    SAI_PORT_SPEED_GIG             = 1000,
    SAI_PORT_SPEED_TEN_GIG         = 10000,
    SAI_PORT_SPEED_TWENTY_GIG      = 20000,
    SAI_PORT_SPEED_TWENTY_FIVE_GIG = 25000,
    SAI_PORT_SPEED_FORTY_GIG       = 40000,
    SAI_PORT_SPEED_FORTYTWO_GIG    = 42000,
    SAI_PORT_SPEED_FIFTY_GIG       = 50000,
    SAI_PORT_SPEED_HUNDRED_GIG     = 100000,
    SAI_PORT_SPEED_MAX,
    /* Increment the macro SAI_MAX_SUPPORTED_SPEEDS on adding new speed */
} sai_port_speed_t;

/**
 * @brief SAI port supported speed capability
 */
typedef enum _sai_port_speed_capability_t {
    SAI_PORT_CAP_SPEED_TEN_MEG         = (1 << 0),
    SAI_PORT_CAP_SPEED_HUNDRED_MEG     = (1 << 1),
    SAI_PORT_CAP_SPEED_GIG             = (1 << 2),
    SAI_PORT_CAP_SPEED_TEN_GIG         = (1 << 3),
    SAI_PORT_CAP_SPEED_TWENTY_FIVE_GIG = (1 << 4),
    SAI_PORT_CAP_SPEED_FORTY_GIG       = (1 << 5),
    SAI_PORT_CAP_SPEED_FORTY_TWO_GIG   = (1 << 6),
    SAI_PORT_CAP_SPEED_FIFTY_GIG       = (1 << 7),
    SAI_PORT_CAP_SPEED_HUNDRED_GIG     = (1 << 8),
    SAI_PORT_CAP_SPEED_TWENTY_GIG      = (1 << 9),
    SAI_PORT_CAP_SPEED_MAX             = (1 << 10),
} sai_port_speed_capability_t;

/**
 * @brief SAI physical port phy types
 */
typedef enum _sai_port_phy_t {
    /** Internal PHY */
    SAI_PORT_PHY_INTERNAL,

    /** External PHY */
    SAI_PORT_PHY_EXTERNAL,
} sai_port_phy_t;

/**
 * @brief Port forwarding mode
 */
typedef enum _sai_port_fwd_mode_t {
    /** Default forwarding Mode */
    SAI_PORT_FWD_MODE_UNKNOWN,

    /** L2 - Switching */
    SAI_PORT_FWD_MODE_SWITCHING,

    /** L3 - Routing */
    SAI_PORT_FWD_MODE_ROUTING,
} sai_port_fwd_mode_t;
/**
 * @brief Port Add/Delete Event
 */
typedef enum _sai_port_event_t
{
    /** Create a new active port */
    SAI_PORT_EVENT_ADD,

    /** Delete/Invalidate an existing port */
    SAI_PORT_EVENT_DELETE,

} sai_port_event_t;

/**
 * @brief Defines the port event notification
 */
typedef struct _sai_port_event_notification_t {

    /** Port id */
    sai_object_id_t port_id;

    /** Port event */
    sai_port_event_t port_event;

} sai_port_event_notification_t;

typedef void (*sai_port_event_notification_fn)(uint32_t count,
                                    sai_port_event_notification_t *data);
/**
 * @brief SAI port_event internal notification
 */
typedef struct _saI_port_event_internal_notf_t {

    /** Port event notification handler */
    sai_port_event_notification_fn  port_event;
} sai_port_event_internal_notf_t;

/**
 * @brief SAI PORT attributes default values
 */
#define SAI_DFLT_OPER_STATUS            SAI_PORT_OPER_STATUS_DOWN
#define SAI_DFLT_SPEED                  40000
#define SAI_DFLT_AUTONEG                0
#define SAI_DFLT_FULLDUPLEX             1
#define SAI_DFLT_ADMIN_STATE            false
#define SAI_DFLT_MEDIA_TYPE             SAI_PORT_MEDIA_TYPE_NOT_PRESENT
#define SAI_DFLT_VLAN                   1
#define SAI_DFLT_VLAN_PRIORITY          0
#define SAI_DFLT_ING_FILTERING          false
#define SAI_DFLT_DROP_UNTAGGED          false
#define SAI_DFLT_DROP_TAGGED            false
#define SAI_DFLT_LOOPBACK_MODE          SAI_PORT_INTERNAL_LOOPBACK_MODE_NONE
#define SAI_DFLT_FDB_LEARNING_MODE      SAI_PORT_FDB_LEARNING_MODE_HW
#define SAI_DFLT_UPDATE_DSCP            false
#define SAI_DFLT_MTU                    1514
#define SAI_DFLT_MAX_LEARNED_ADDR       0
#define SAI_DFLT_FDB_LEARNED_LIMIT_VIOL SAI_PACKET_ACTION_DROP
#define SAI_DFLT_FLOW_CONTROL_MODE      SAI_PORT_FLOW_CONTROL_MODE_DISABLE
#define SAI_DFLT_PFC_ENABLED_BITMAP     0
#define SAI_DFLT_OUI_CODE               0x6A737D
#define SAI_DFLT_FEC_MODE               SAI_PORT_FEC_MODE_NONE


/**
 * @brief SAI Lane bitmap based on lane count
 * @todo Remove it once active lane based bitmap is added
 */
#define SAI_ONE_LANE_BITMAP  (0x1)
#define SAI_TWO_LANE_BITMAP  (0x5)
#define SAI_FOUR_LANE_BITMAP (0xF)

/**
 * @brief SAI port attribute information used for caching the
 * default and adapter host configured port attribute values
 */
typedef struct _sai_port_attr_info_t
{
    /** Set to true once default init is done*/
    bool                              default_init;

    /** Link oper status (default to SAI_PORT_OPER_STATUS_DOWN)*/
    sai_port_oper_status_t            oper_status;

    /** Speed in Mbps */
    uint_t                            speed;

    /** Full duplex setting (default to TRUE) */
    bool                              duplex;

    /** Auto Neg setting (default to FALSE) */
    bool                              autoneg;

    /** Admin Mode (default to FALSE) */
    bool                              admin_state;

    /** Media Type (default to SAI_PORT_MEDIA_TYPE_NOT_PRESENT) */
    sai_port_media_type_t             media_type;

    /** Default VLAN Untagged ingress frames are tagged with default VLAN */
    sai_vlan_id_t                     default_vlan;

    /** Default VLAN Priority (default to 0) */
    uint_t                            default_vlan_priority;

    /** Dropping of untagged frames on ingress (default to FALSE) */
    bool                              drop_untagged;

    /** Dropping of tagged frames on ingress (default to FALSE) */
    bool                              drop_tagged;

    /** Internal loopback control (default to SAI_PORT_INTERNAL_LOOPBACK_MODE_NONE) */
    sai_port_internal_loopback_mode_t internal_loopback;

    /** Update DSCP of outgoing packets (default to FALSE) */
    bool                              update_dscp;

    /** MTU (default to 1514 bytes) */
    uint_t                            mtu;

    /** Port related Meta Data */
    uint_t                            meta_data;

     /** Global flow control mode */
    sai_port_flow_control_mode_t      flow_control_mode;

    /** PFC priorities enabled on the port */
    uint8_t                           pfc_enabled_bitmap;

    /** FEC type enabled on the port */
    sai_port_fec_mode_t               fec_mode;

    /** OUI code for 25G/50G port */
    sai_uint32_t                      oui_code;

} sai_port_attr_info_t;

/**
 * @brief SAI port level capabilities and information.
 */
typedef struct _sai_port_info_t
{
    /** SAI port UOID - key to port info tree */
    sai_object_id_t       sai_port_id;

    /** Port active or inactive */
    bool                  port_valid;

    /** Logical/Local port id to index NPU API's */
    sai_npu_port_id_t     local_port_id;

    /** Physical Port Number */
    sai_npu_port_id_t     phy_port_id;

    /** Group number it is part of in a multi-lane port */
    uint_t                port_group;

    /** PHY used Internal or External */
    sai_port_phy_t        phy_type;

    /** Forwarding mode - Switching or Routing */
    sai_port_fwd_mode_t   fwd_mode;

    /** External PHY address */
    sai_npu_port_id_t     ext_phy_addr;

    /** Port supported capability flags [sai_port_capability_t] */
    uint64_t              port_supported_capb;

    /** Port capabilities enabled [sai_port_capability_t] */
    uint64_t              port_enabled_capb;

    /** Maximum  Lanes support per port */
    uint_t                max_lanes_per_port;

    /** Bitmap of the actives lanes in the port */
    uint64_t              port_lane_bmap;

    /** Port speed in Gbps */
    sai_port_speed_t      port_speed;

    /** Port supported speed [sai_port_speed_capability_t]  */
    uint_t                port_speed_capb;

    /** Cache Current optics type based on adapter host input */
    sai_port_media_type_t media_type;

    /** Cache all port attributes default/configured value */
    sai_port_attr_info_t  port_attr_info;

    /** Map dport number <dport_id> to internal port number <phy_port_id> */
    sai_npu_port_id_t     dport_id;

    /** Bridge port object ID that is added to 1Q bridge */
    sai_object_id_t       def_bridge_port_id;

    /** LAG ID that the port is part of. SAI_NULL_OBJECT_ID if the port doesn't belong to any LAG */
    sai_object_id_t       lag_id;

    /** Number of objects referencing this port */
    uint_t                ref_count;

    /** Number of VLANs that the port is part of */
    uint_t                num_vlans;
    /*EEE support on port */
    bool                  eee_support;
} sai_port_info_t;

/**
 * @brief SAI port level capabilities Flags
 */
typedef enum _sai_port_capability_t
{
    /** Breakout Mode support on the port */
    SAI_PORT_CAP_BREAKOUT_MODE = (1 << 0),

    /** 1 lane break out/in mode */
    SAI_PORT_CAP_BREAKOUT_MODE_1X = (1 << 1),

    /** 2 lanes break out/in mode */
    SAI_PORT_CAP_BREAKOUT_MODE_2X = (1 << 2),

    /** 4 lanes break out/in mode */
    SAI_PORT_CAP_BREAKOUT_MODE_4X = (1 << 3),

    /** Add new capabilities above this. Increment SAI_PORT_MAX_BREAKOUT_MODE_CAP on adding */
    SAI_PORT_CAP_MAX = (1 << 4)
} sai_port_capability_t;

/* Increment this on adding new modes. This is macro does not include SAI_PORT_CAP_BREAKOUT_MODE
 * itself and its just a count of breakout modes
 */
#define SAI_PORT_MAX_BREAKOUT_MODE_CAP 3
/**
 * @brief Number of hardware lane in a physical port
 */
typedef enum _sai_port_lane_count_t
{
    SAI_PORT_LANE_COUNT_ONE = 1,

    SAI_PORT_LANE_COUNT_TWO = 2,

    SAI_PORT_LANE_COUNT_FOUR = 4,
} sai_port_lane_count_t;

/**
 * @brief Data structure of port table for attached applications on the port
 */
typedef struct _sai_port_application_info_t {

    /** port Id */
    sai_object_id_t port_id;

    /** Mirror sessions per port tree */
    void *mirror_sessions_tree;

    /** Qos DB per port tree */
    dn_sai_qos_port_t *qos_port_db;
    /** Sampling object */
    sai_object_id_t sample_object[SAI_SAMPLEPACKET_DIR_MAX];

} sai_port_application_info_t;

/**
 * @brief Structure to be used by APIs to configure breakout
 */
typedef struct _sai_port_breakout_config_t {

    /** Current breakout mode */
    sai_port_breakout_mode_type_t curr_mode;

    /** New breakout mode */
    sai_port_breakout_mode_type_t new_mode;

    /** Current speed of the port */
    sai_port_speed_t curr_speed;

    /** New speed of the port after breakout */
    sai_port_speed_t new_speed;

} sai_port_breakout_config_t;
/*
 * SAI Port Object Id (sai_npu_object_id_t) 48 bit encoding scheme
 -------------------------------------------------------------------------
 |  RESV  |      Port Type       |      Fabric ID        |  Port Number  |
 -------------------------------------------------------------------------
 |   (4)  |          (4)         |         (8)           |      (32)     |
 -------------------------------------------------------------------------

 Port Number - Encoded as 32-bits value, represents the port logical port number.
 Logical Port numbers can be same as HW Port number or it can be mapped
 to a HW port depending on the NPU

 Fabric ID   - Encoded as 12-bits value, represents the distinct NPU/Fabric id
 of a specific NPU in a Multi NPU system.

 Port Type   - Represents the SAI port type, encoded as 4-bits value. The different
 SAI port object types are defined as in sai_port_type_t.

 RESV        - Reserved 4-bits for future use, will be set to all 0.

*/

#define SAI_PORT_NUMBER_BITPOS      0
#define SAI_FABRIC_ID_BITPOS        32
#define SAI_PORT_TYPE_BITPOS        40

#define SAI_PORT_NUMBER_MASK        0X000FFFFFFFF
#define SAI_FABRIC_ID_MASK          0X0FF00000000
#define SAI_PORT_TYPE_MASK          0xF0000000000

/**
 * @brief Create SAI port object id.
 *
 * @param[in] type SAI port type
 * @param[in] fab_id fabric id
 * @param[in] port_id logical port number
 * @return SAI unified object id encoding the object type and object id.
 */
static inline sai_object_id_t sai_port_id_create(sai_port_type_t type,
                                                 sai_switch_id_t fab_id,
                                                 sai_npu_port_id_t port_id)
{
    sai_npu_object_id_t port_npu_oid = 0;

    port_npu_oid = (((( (sai_npu_object_id_t) port_id) << SAI_PORT_NUMBER_BITPOS) & SAI_PORT_NUMBER_MASK) |
                    ((( (sai_npu_object_id_t) fab_id) << SAI_FABRIC_ID_BITPOS) & SAI_FABRIC_ID_MASK) |
                    ((( (sai_npu_object_id_t) type) << SAI_PORT_TYPE_BITPOS) & SAI_PORT_TYPE_MASK));

    return sai_uoid_create(SAI_OBJECT_TYPE_PORT, port_npu_oid);
}

/**
 * @brief Get SAI Port number from the unified object id. Based on port_type,
 *        SAI_PORT_TYPE_LOGICAL - This port id is used for indexing the NPU SDK API's.
 *        SAI_PORT_TYPE_CPU - This port id is used in all CPU related NPU SDK API's
 *
 * @param[in] uoid SAI unified object id.
 * @return SAI npu port number part from the unified object id is returned.
 */
static inline sai_npu_port_id_t sai_port_number_get(sai_object_id_t port_id)
{
    return (sai_npu_port_id_t)((sai_uoid_npu_obj_id_get(port_id) &
                                SAI_PORT_NUMBER_MASK) >> SAI_PORT_NUMBER_BITPOS);
}

/**
 * @brief Get SAI port type from the unified object id.
 *
 * @param[in] uoid SAI unified object id.
 * @return SAI port type from the unified object id is returned.
 */
static inline sai_port_type_t sai_port_type_get(sai_object_id_t port_id)
{
    return (sai_port_type_t)((sai_uoid_npu_obj_id_get(port_id) &
                              SAI_PORT_TYPE_MASK) >> SAI_PORT_TYPE_BITPOS);
}

/**
 * @brief Get SAI Fabric id from the unified object id
 *
 * @param[in] uoid SAI unified object id.
 * @return SAI Fabric id part from the unified object id is returned.
 */
static inline sai_switch_id_t sai_port_fabric_id_get(sai_object_id_t port_id)
{
    return (sai_switch_id_t)((sai_uoid_npu_obj_id_get(port_id)
                              & SAI_PORT_TYPE_MASK) >> SAI_PORT_TYPE_BITPOS);
}

/**
 * @brief Check if the SAI object id is logical port object id.
 *
 * @param[in] uoid SAI unified object id.
 * @return true if logical port object id else false is returned.
 */
static inline bool sai_is_obj_id_logical_port(sai_object_id_t port_id)
{
    return (sai_port_type_get(port_id) == SAI_PORT_TYPE_LOGICAL);
}

/**
 * @brief Check if the SAI object id is cpu port object id.
 *
 * @param[in] uoid SAI unified object id.
 * @return true if cpu port object id else false is returned.
 */
static inline bool sai_is_obj_id_cpu_port(sai_object_id_t port_id)
{
    return (sai_port_type_get(port_id) == SAI_PORT_TYPE_CPU);
}

/**
 * \}
 */

#endif /* __SAI_PORT_COMMON_H__ */
