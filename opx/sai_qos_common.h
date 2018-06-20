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
* @file sai_qos_common.h
*
* @brief This file contains the datastructure definitions for SAI QOS objects.
*
*************************************************************************/
#ifndef __SAI_QOS_COMMON_H__
#define __SAI_QOS_COMMON_H__

#include "saipolicer.h"
#include "saiqosmap.h"
#include "saiqueue.h"
#include "saischeduler.h"
#include "saibuffer.h"
#include "saiwred.h"

#include "std_type_defs.h"
#include "std_llist.h"
#include "std_rbtree.h"
/** @defgroup SAIQOS Defaults - SAI QOS Default values to be internally used by SAI
*
* \{
*/
/**
 * @brief MAX packet DSCP supported by SAI spec.
 */
#define SAI_QOS_MAX_DSCP            (64)

/**
 * @brief Default packet DSCP supported by SAI spec.
 */
#define SAI_QOS_DEFAULT_DSCP            (0)

/**
 * @brief MAX packet Dot1p supported by SAI spec.
 */
#define SAI_QOS_MAX_DOT1P           (8)

/**
 * @brief Default packet Dot1p supported by SAI spec.
 */
#define SAI_QOS_DEFAULT_DOT1P           (0)

/**
 * @brief MAX packetTC (internal priority) supported by SAI spec.
 */
#define SAI_QOS_MAX_TC              (16)

/**
 * @brief MAX CFI supported by SAI spec.
 */
#define SAI_QOS_MAX_CFI             (2)

/**
 * @brief Default Traffic Class
 * @TODO: Get the value from config file
 */

#define SAI_QOS_DEFAULT_TC          0

/**
 * @brief MAX packet colors supported by SAI spec.
 */
#define SAI_QOS_MAX_PACKET_COLORS   (3)

/**
 * @brief MAX Qos types supported by SAI spec.
 */
#define SAI_QOS_MAX_QOS_MAPS_TYPES  (14)

/**
 * @brief Default Queue index
 */
#define SAI_QOS_DEFAULT_QUEUE_INDEX (0)

 /**
 * @brief Default PG
 */
#define SAI_QOS_DEFAULT_PG (7)

/**
 * @brief Default PG
 */
#define SAI_QOS_MAX_PFC_PRI (8)

/**
 * @brief Default PG
 */
#define SAI_QOS_DEFAULT_PFC_QUEUE_INDEX (0)

/**
 * @brief Max policer action count
 */
#define SAI_POLICER_MAX_ACTION_COUNT (3)

/**
 * @brief WRED max buffer limit
 */
#define SAI_WRED_MAX_BUFFER_LIMIT       (0xFFFFFFFF)

/**
 * @brief WRED max drop probability
 */
#define SAI_WRED_MAX_DROP_PROBABILITY   (100)

/**
 * @brief WRED max weight
 */
#define SAI_WRED_MAX_WEIGHT             (15)
/**
\}
*/

/** SAI Scheduler default weight */
#define SAI_QOS_SCHEDULER_DEFAULT_WEIGHT (1)

/** SAI Scheduler Maximum weight */
#define SAI_SCHEDULING_MAX_WEIGHT     (100)

/** SAI Scheduler default shapeing type */
#define SAI_QOS_SCHEDULER_DEFAULT_SHAPE_TYPE (SAI_METER_TYPE_BYTES)

/** @defgroup SAIQOS DS - SAI QOS Data structures to be internally used by SAI
*
* \{
*/
/**
 * @brief QOS object types
 *
 */
typedef enum _dn_sai_qos_object_type_t {

    /** Policer object */
    SAI_QOS_OBJECT_TYPE_POLICER  = 0,

    /** WRED object */
    SAI_QOS_OBJECT_TYPE_WRED,

    /** Qos Maps object */
    SAI_QOS_OBJECT_TYPE_MAPS,

    /** Qos Queue object */
    SAI_QOS_OBJECT_TYPE_QUEUE,

    /** Qos Scheduler object */
    SAI_QOS_OBJECT_TYPE_SCHEDULER,

    /** Qos Scheduler Group object */
    SAI_QOS_OBJECT_TYPE_SCHEDULER_GROUP,

    /** Qos Buffer object */
    SAI_QOS_OBJECT_TYPE_BUFFER,

    /** Qos Maximum objects */
    SAI_QOS_OBJECT_TYPE_MAX,

} dn_sai_qos_object_type_t;

/**
 * @brief List of Policer types can be assinged to port/lag/vlan.
 *
 */
typedef enum _dn_sai_qos_policer_type_t {

    /** Storm Control Flood Policer ID */
    SAI_QOS_POLICER_TYPE_STORM_FLOOD = 0,

    /** Storm Control BCAST Policer ID */
    SAI_QOS_POLICER_TYPE_STORM_BCAST,

    /** Storm Control MCAST Policer ID */
    SAI_QOS_POLICER_TYPE_STORM_MCAST,

    /** All packets Policer ID */
    SAI_QOS_POLICER_TYPE_PACKET_ALL,

    /** Max policer types */
    SAI_QOS_POLICER_TYPE_MAX,

} dn_sai_qos_policer_type_t;

/**
 * @brief List of WRED profile linking types.
 *
 */
typedef enum _dn_sai_qos_wred_link_t {
    DN_SAI_QOS_WRED_LINK_QUEUE,
    DN_SAI_QOS_WRED_LINK_PORT,
    DN_SAI_QOS_WRED_LINK_BUFFER_POOL,
    DN_SAI_QOS_WRED_LINK_MAX,
} dn_sai_qos_wred_link_t;
/**
 *  @brief SAI Policer Key
 */
typedef struct _dn_sai_qos_policer_key_t {

    /** Policer Identifier */
    sai_object_id_t  policer_id;

} dn_sai_qos_policer_key_t;

/**
 * @brief SAI Policer Action Data Structure
 *
 * Contains SAI Policer action specific information.
 **/
typedef struct _dn_sai_policer_action_t {

    /** Action Specifier */
    sai_policer_attr_t action;

    /** Action enable/disable */
    bool               enable;

    /** Action parameter */
    uint_t             value;

} dn_sai_policer_action_t;


/**
 * @brief SAI QOS Policer data structure.
 *
 */
typedef struct _dn_sai_qos_policer_t
{
    /** Key structure for Policer tree*/
    dn_sai_qos_policer_key_t   key;

    /** Mode of the policer */
    sai_policer_mode_t         policer_mode;

    /** Type of the meter. Packets/bytes */
    sai_meter_type_t           meter_type;

    /** Color source: Color blind or color aware */
    sai_policer_color_source_t color_source;

    /** Committed burst size in packets/bytes based on meter type*/
    uint64_t                   cbs;

    /** Peak burst size in packets/bytes based on meter type*/
    uint64_t                   pbs;

    /** Committed info rate in packets/bytes based on meter type*/
    uint64_t                   cir;

    /** Peak info rate in packets/bytes based on meter type*/
    uint64_t                   pir;

    /** Attached policer action count */
    uint_t                     action_count;

    /** Attached Policer actions list */
    dn_sai_policer_action_t    action_list[SAI_POLICER_MAX_ACTION_COUNT];

    /** Stat id associated to the policer */
    uint_t                     policer_stat_id;

    /** Port list head. Nodes of type dn_sai_qos_port_t */
    std_dll_head               port_dll_head[SAI_QOS_POLICER_TYPE_MAX];

    /** Acl rule list head. Nodes of type sai_acl_rule_t*/
    std_dll_head               acl_dll_head;

} dn_sai_qos_policer_t;

/**
 * @brief SAI QOS Map Key
 *
 * Contains the key for SAI QOS map key
 */
typedef struct _dn_sai_qos_map_key_t {

    /** Qos Map Identifier */
    sai_object_id_t  map_id;

} dn_sai_qos_map_key_t;

/**
 * @brief SAI Qos maps Data Structure
 *
 * Contains the SAI Qos maps key and value pair list.
 */

typedef struct _dn_sai_qos_map_t
{
   /** Key structure for Qos maps tree*/
    dn_sai_qos_map_key_t       key;

    /** Type of the map */
    sai_qos_map_type_t         map_type;

    /** Key to value pair list based on map_type */
    sai_qos_map_list_t         map_to_value;

    /** Port list head. Nodes of type dn_sai_qos_port_t */
    std_dll_head               port_dll_head;

} dn_sai_qos_map_t;

/**
 * @brief SAI QOS WRED Thresholds structure
 */
typedef struct _dn_sai_qos_wred_threshold_t {

    /** Enable/Disable wred */
    bool                  enable;

    /** Min threshold limit */
    uint_t              min_limit;

    /** Max threshold limit */
    uint_t              max_limit;

    /** Drop probability */
    uint_t              drop_probability;

    /** ECN Enable/Disable */
    bool                  ecn_enable;

} dn_sai_qos_wred_threshold_t;


/**
 * @brief SAI QOS WRED profile Key
 */
typedef struct _dn_sai_qos_wred_key_t {

    /** Qos Map Identifier */
    sai_object_id_t  wred_id;

} dn_sai_qos_wred_key_t;

/**
 * @brief SAI Qos WRED Data Structure
 *
 * Contains the SAI Qos wred profile information.
 */

typedef struct _dn_sai_qos_wred_t
{
    /** Key for the wred profile tree */
    dn_sai_qos_wred_key_t key;

    /* WRED Threshold for packet colors supported */
    dn_sai_qos_wred_threshold_t threshold [SAI_QOS_MAX_PACKET_COLORS];

    /** Weight for the wred profile. Independent of color. */
    uint_t                weight;

    /** ECN Marking mode */
    sai_ecn_mark_mode_t   ecn_mark_mode;

    /** Queue list head. Nodes of type dn_sai_qos_queue_t */
    std_dll_head          queue_dll_head;

    /** Port list head. Nodes of type dn_sai_qos_port_t */
    std_dll_head          port_dll_head;

    /** Buffer Pool list head. Nodes of type dn_sai_qos_queue_t */
    std_dll_head          buffer_pool_dll_head;
} dn_sai_qos_wred_t;


/**
 *  @brief SAI Queue DS Key
 */
typedef struct _dn_sai_qos_queue_key_t {

    /** SAI Queue OID */
    sai_object_id_t  queue_id;

} dn_sai_qos_queue_key_t;

/**
 * @brief SAI QOS Queue data structure.
 * Contains the data of attributes associated to queue.
 */
typedef struct _dn_sai_qos_queue_t
{
    /** Key structure for Queue tree */
    dn_sai_qos_queue_key_t  key;

    /** Type of the queue. */
    sai_queue_type_t        queue_type;

    /** SAI Port Identifier */
    sai_object_id_t         port_id;

    /** Queue Index **/
    sai_uint8_t             queue_index;

    /** Parent scheduler group node */
    sai_object_id_t         parent_sched_group_id;

    /** Buffer profile Object ID */
    sai_object_id_t         buffer_profile_id;

    /** WRED profile id attached to queue */
    sai_object_id_t         wred_id;

    /** Scheduler profile id attached to queue */
    sai_object_id_t         scheduler_id;

    /** Child index of queue in parent scheduler group */
    uint_t                  child_offset;

    /** Link to the port queue list */
    std_dll                 port_dll_glue;

    /** Link to the Child list of parent Scheduler group */
    std_dll                 child_queue_dll_glue;

    /** Link to the WRED queue list */
    std_dll                 wred_dll_glue;

    /** Link to the buffer profile associated to the queue */
    std_dll                 buffer_profile_dll_glue;

    /** Link to the Scheduler queue list */
    std_dll                 scheduler_dll_glue;

} dn_sai_qos_queue_t;

/**
 *  @brief SAI Scheduler DS Key
 */
typedef struct _dn_sai_qos_scheduler_key_t {

    /** SAI Scheduler Profile Identifier */
    sai_object_id_t  scheduler_id;

} dn_sai_qos_scheduler_key_t;


/**
 * @brief SAI QOS Scheduler data structure.
 *
 */
typedef struct _dn_sai_qos_scheduler_t
{
    /** Key structure for scheudler tree */
    dn_sai_qos_scheduler_key_t key;

    /** Scheduleing type SP/DWRR */
    sai_scheduling_type_t      sched_algo;

    /** Relative weight for RR */
    uint_t                     weight;

    /** Type of the shaper. Packets/bytes */
    sai_meter_type_t           shape_type;

    /** Maximum bandwidth rate  */
    uint64_t                   max_bandwidth_rate;

    /** Maximum bandwidth burst size */
    uint64_t                   max_bandwidth_burst;

    /** Minimum bandwidth rate  */
    uint64_t                   min_bandwidth_rate;

    /** Minimum bandwidth burst size */
    uint64_t                   min_bandwidth_burst;

    /** Port list head. Nodes of type dn_sai_qos_port_t */
    std_dll_head               port_dll_head;

    /** Queue list head. Nodes of type dn_sai_qos_queue_t */
    std_dll_head               queue_dll_head;

    /** Scheduler group list head. Nodes of type dn_sai_qos_sched_group_t */
    std_dll_head               sched_group_dll_head;

} dn_sai_qos_scheduler_t;


/**
 *  @brief SAI Hierarchy Information
 */
typedef struct _dn_sai_qos_hierarchy_info_t {

   /** Scheduler group child list head. Nodes of type dn_sai_qos_sched_group_t */
    std_dll_head                  child_sched_group_dll_head;

    /** Scheduler group child list head. Nodes of type dn_sai_qos_queue_t */
    std_dll_head                  child_queue_dll_head;

    /** Attached/Active child node count on group */
    uint_t                        child_count;

    /* Bit set indicates Child index is available for use */
    uint8_t                       *child_index_bitmap;
} dn_sai_qos_hierarchy_info_t;


/**
 *  @brief SAI Scheduler Group DS Key
 */
typedef struct _dn_sai_qos_sched_group_key_t {

    /** SAI Scheduler Group Identifier */
    sai_object_id_t  sched_group_id;

} dn_sai_qos_sched_group_key_t;

/**
 * @brief SAI QOS Scheduler group data structure.
 * Contains the List of nodes grouped at each level per port, and parent node information.
 */
typedef struct _dn_sai_qos_sched_group_t
{
    /** Key structure for Scheduler groups tree */
    dn_sai_qos_sched_group_key_t  key;

    /** SAI Port Identifier */
    sai_object_id_t               port_id;

    /** Parent scheduler group node */
    sai_object_id_t               parent_id;

    /** Hierarchy level */
    uint_t                        hierarchy_level;

    /** Max children at each level */
    uint_t                        max_childs;

    /** Scheduler profile id attached to group */
    sai_object_id_t               scheduler_id;

    /** True if the node contains a dummy child in NPU
        Dummy nodes are needed in the hierarchy due to chip specific restrictions as in
        HSP port in broadcom. These are created in addition to hierarchy present in xml file */
    bool                          dummy_child;

    /** Link to the port scheduler group list */
    std_dll                       port_dll_glue;

    /** Link to the Scheduler scheduler group list */
    std_dll                       scheduler_dll_glue;

    /** Link to the Child list of parent Scheduler group */
    std_dll                       child_sched_group_dll_glue;

    /** Child index of scheduler group  in parent scheduler group */
    uint_t                        child_offset;

    /** Hierarchy Information for group */
    dn_sai_qos_hierarchy_info_t   hqos_info;

} dn_sai_qos_sched_group_t;

/**
 * @brief SAI QOS data structure for the port parameters
 * Contains the List of policers, maps, scheduler and wred assigned to port.
 * And also it holds the information for port attributes
 */

typedef struct _dn_sai_qos_port_t
{
    /** Port Object ID */
    sai_object_id_t               port_id;
    /** Policer list assigned to port */
    sai_object_id_t               policer_id [SAI_QOS_POLICER_TYPE_MAX];

    /** Qos Maps list assigned to port */
    sai_object_id_t               maps_id [SAI_QOS_MAX_QOS_MAPS_TYPES];

    /** Buffer profile Object ID */
    sai_object_id_t               buffer_profile_id;

    /** Link to the Policer port list */
    std_dll                       policer_dll_glue[SAI_QOS_POLICER_TYPE_MAX];

    /** Link to the Scheduler port list */
    std_dll                       scheduler_dll_glue;

    /** Link to the qos maps port list for each map type */
    std_dll                       maps_dll_glue [SAI_QOS_MAX_QOS_MAPS_TYPES];

    /** Link to the buffer profile associated to the port */
    std_dll                       buffer_profile_dll_glue;

    /** Scheduler profile id attached to port */
    sai_object_id_t               scheduler_id;

    /** Queue list head. Nodes of type dn_sai_qos_queue_t */
    std_dll_head                  queue_dll_head;

    /** Scheduler group list head. Nodes of type dn_sai_qos_sched_group_t.
     * Head for each level */
    std_dll_head                 *sched_group_dll_head;

    /** Number of PGs associated to the port */
    unsigned int                  num_pg;

    /** PG list head. Nodes of type dn_sai_qos_pg_t */
    std_dll_head                  pg_dll_head;

    /** Application started creating hierarchy qos */
    bool                          is_app_hqos_init;

    /** NPU specific abstracted information */
    void                         *p_npu_info;

    /** Queue list head. Nodes of type dn_sai_qos_queue_t */
    std_dll_head                  port_pool_dll_head;

} dn_sai_qos_port_t;

/**
 * @brief SAI Port Pool data structure
 */
typedef struct _dn_sai_qos_port_pool_t {
    /** Link to the WRED port pool list */
    std_dll                         port_dll_glue;

    /** Qos Port Pool SAI object Identifier */
    sai_object_id_t                 port_pool_id;

    /** Port SAI object Identifier */
    sai_object_id_t                 port_id;

    /** Qos Buffer profile Identifier */
    sai_object_id_t                 pool_id;

    /** WRED profile id attached to port pool */
    sai_object_id_t                 wred_id;

    /** Link to the WRED port pool list */
    std_dll                         wred_dll_glue;

    /** To indicate if WRED is not set in HW */
    bool                            wred_sw_cached;

} dn_sai_qos_port_pool_t;

/**
 * @brief SAI QOS Buffer pool Key
 *
 * Contains the key for SAI QOS buffer pool
 */
typedef struct _dn_sai_qos_buffer_pool_key_t {

    /** Qos Buffer profile Identifier */
    sai_object_id_t  pool_id;

} dn_sai_qos_buffer_pool_key_t;

/**
 * @brief SAI Qos Buffer pool Data Structure
 *
 * Contains the SAI Qos buffer pool key and value pait list.
 */

typedef struct _dn_sai_qos_buffer_pool_t
{
   /** Key structure for Qos Buffer pool tree*/
    dn_sai_qos_buffer_pool_key_t   key;

    /** Type of the Buffer pool */
    sai_buffer_pool_type_t         pool_type;

    /** Buffer pool threshold mode */
    sai_buffer_pool_threshold_mode_t    threshold_mode;

    /** Size of the buffer pool */
    uint32_t                       size;

    /** Shared Size of the buffer pool */
    uint32_t                       shared_size;

    /** No of buffer profiles associated with this pool */
    uint_t                         num_ref;

    /** Buffer profile list head. Nodes of type dn_sai_qos_buffer_profile_t */
    std_dll_head                   buffer_profile_dll_head;

    /** Place holder for NPU-specific data */
    void                           *hw_info;

    /** Shared heardroom Size of the buffer pool */
    uint32_t                       xoff_size;

    /** WRED profile id attached to queue */
    sai_object_id_t         wred_id;

    /** Link to the WRED queue list */
    std_dll                 wred_dll_glue;

    /** To indicate if WRED is not set in HW */
    bool                    wred_sw_cached;

} dn_sai_qos_buffer_pool_t;

/**
 * @brief SAI QOS Buffer profile Key
 *
 * Contains the key for SAI QOS buffer profile
 */
typedef struct _dn_sai_qos_buffer_profile_key_t {

    /** Qos Buffer Profile Identifier */
    sai_object_id_t  profile_id;

} dn_sai_qos_buffer_profile_key_t;

/**
 * @brief SAI Qos Buffer profile Data Structure
 *
 * Contains the SAI Qos buffer profile key and value pait list.
 */

typedef struct _dn_sai_qos_buffer_profile_t
{
   /** Key structure for Qos Buffer profile tree*/
    dn_sai_qos_buffer_profile_key_t   key;

    /** Buffer Pool object */
    sai_object_id_t                   buffer_pool_id;

    /** Size of the buffer profile */
    uint32_t                          size;

    /** Enable buffer profile level threshold mode */
    bool                              profile_th_enable;

    /** Buffer profile threshold mode */
    sai_buffer_profile_threshold_mode_t       threshold_mode;

    /** Shared dynamic threshold of the buffer profile */
    int8_t                            dynamic_th;

    /** Shared static size of the buffer profile */
    uint32_t                          static_th;

    /** XOFF Threshold */
    uint32_t                          xoff_th;

    /** XON Threshold */
    uint32_t                          xon_th;

    /** Link to list in the buffer pool the profile is assocated to */
    std_dll                           buffer_pool_dll_glue;

    /** Number of Ports, PG and Queue where this buffer profile is applied*/
    unsigned int                      num_ref;

    /** List of PG set with this buffer profile */
    std_dll_head                      pg_dll_head;

    /** List of Ports set with this buffer profile */
    std_dll_head                      port_dll_head;

    /** List of queues set with this buffer profile */
    std_dll_head                      queue_dll_head;

    /** Place holder for NPU-specific data */
    void                              *hw_info;

} dn_sai_qos_buffer_profile_t;

/**
 *  @brief SAI pg DS Key
 */
typedef struct _dn_sai_qos_pg_key_t {

    /** SAI PG OID */
    sai_object_id_t  pg_id;

} dn_sai_qos_pg_key_t;

/**
 * @brief SAI QOS PG data structure.
 * Contains the data of attributes associated to Priority Group.
 */
typedef struct _dn_sai_qos_pg_t
{
    /** Key structure for pg tree */
    dn_sai_qos_pg_key_t  key;

    /** SAI Port Identifier */
    sai_object_id_t         port_id;

    /** Buffer profile Object ID */
    sai_object_id_t         buffer_profile_id;

    /** Link to the port pg list */
    std_dll                 port_dll_glue;

    /** Link to the buffer profile associated to the pg */
    std_dll                 buffer_profile_dll_glue;

} dn_sai_qos_pg_t;
/**
 * @brief SAI QOS data structure for the global parameters
 * Contains the pointers for policer, policer counter, wred, maps,
 * queue, scheudler, scheduler groups and switch attribute information.*
 */
typedef struct _dn_sai_qos_global_t {

    /** Nodes of type dn_sai_qos_policer_t */
    rbtree_handle    policer_tree;

    /** Nodes of type dn_sai_qos_wred_t */
    rbtree_handle    wred_tree;

    /** Nodes of type dn_sai_qos_map_t */
    rbtree_handle    map_tree;

    /** Queue tree. Nodes of type dn_sai_qos_queue_t */
    rbtree_handle    queue_tree;

    /** Scheduler tree. Nodes of type dn_sai_qos_scheduler_t */
    rbtree_handle    scheduler_tree;

    /** Scheduler group tree. Nodes of type dn_sai_qos_sched_group_t */
    rbtree_handle    scheduler_group_tree;

    /** PG tree. Nodes of type dn_sai_qos_pg_t */
    rbtree_handle    pg_tree;

    /** Buffer pool tree. Nodes of type dn_sai_qos_buffer_pool_t */
    rbtree_handle    buffer_pool_tree;

    /** Buffer profile tree. Nodes of type dn_sai_qos_buffer_profile_t */
    rbtree_handle    buffer_profile_tree;

    /** flag to indicate if global params are initialized */
    bool             is_init_complete;

} dn_sai_qos_global_t;


/**
 * @brief Structure defining the map directions ingress or egress.
 */
typedef enum _dn_sai_qos_map_direction_t
{
    /** Map direction ingress */
    SAI_MAP_DIR_INGRESS,

    /** Map direction egress */
    SAI_MAP_DIR_EGRESS,
}dn_sai_qos_map_direction_t;

/**
 * @brief Structure definig the type of child in the qos hierarchy.
 */
typedef enum _dn_sai_qos_hierarchy_child_type_t
{
    /** SAI scheduler group hierarchy child type queue */
    CHILD_TYPE_QUEUE,
    /** SAI scheduler group hierarchy child type scheduler */
    CHILD_TYPE_SCHEDULER,
}dn_sai_qos_hierarchy_child_type_t;

/**
 * @brief Structure containing information about child nodes in hierarchy.
 */
typedef struct _dn_sai_qos_child_info_t
{
    /** Type of child Scheduler group or queue */
    dn_sai_qos_hierarchy_child_type_t type;
    /** Level in the hierarchy */
    uint_t level;
    /** Index of this node in the hierarchy. */
    uint_t child_index;
    /** Type of queue if child type is queue */
    uint_t queue_type;
}dn_sai_qos_child_info_t;

/**
 * @brief Structure containing information about nodes in hierarchy.
 */
typedef struct _dn_sai_qos_sched_grp_info_t
{
    /**Index of the node in hierarchy */
    uint_t   node_id;
    /**Number of children attached to this node. */
    uint_t    num_children;
    /**Scheduling algorithm type*/
    uint_t    sched_mode;
    /**Array having child info */
    dn_sai_qos_child_info_t *child_info;
}dn_sai_qos_sched_grp_info_t;

/**
 * @brief Structure containing information about levels in hierarchy.
 */
typedef struct _dn_sai_qos_level_info_t
{
    /**Level of the node in hierarchy */
    uint_t    level;

    /** Number of scheduler groups at this level */
    uint_t num_sg_groups;

    /**Array containing node info at each level in hierarchy */
    dn_sai_qos_sched_grp_info_t *sg_info;
}dn_sai_qos_level_info_t;

/**
 * @brief Structure containing information about default hierarchy for each NPU.
 */
typedef struct _dn_sai_qos_hierarchy_t
{
    /**Array having level info in the hierarchy */
    dn_sai_qos_level_info_t *level_info;
}dn_sai_qos_hierarchy_t;

/**
\}
*/
#endif /*__SAI_QOS_COMMON_H__*/

