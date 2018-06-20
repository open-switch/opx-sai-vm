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
* @file sai_qos_util.h
*
* @brief This file contains the utility functions for SAI QOS component.
*
*************************************************************************/
#ifndef __SAI_QOS_UTIL_H__
#define __SAI_QOS_UTIL_H__

#include "sai_qos_common.h"
#include "sai_switch_utils.h"
#include "sai_acl_type_defs.h"
#include "sai_port_utils.h"
#include "sai_oid_utils.h"
#include "sai_event_log.h"
#include "std_llist.h"

#include "saitypes.h"


/** \defgroup SAIQOSUTILS SAI - Qos Utility functions
 *  Util functions in the SAI Qos component
 *
 *  \{
 */
#define SAI_POLICER_ATTR_MODE_FLAG    0x1
#define SAI_POLICER_ATTR_PIR_FLAG     0x2

#define SAI_QOS_MAP_TYPE_FLAG           0x1
#define SAI_QOS_MAP_VALUE_TO_LIST_FLAG  0x2

#define SAI_UOID_TYPE_BITPOS     44
#define SAI_UOID_OBJ_ID_BITPOS   0

#define SAI_UOID_TYPE_MASK       0x0000F00000000000
#define SAI_UOID_OBJ_ID_MASK     0x00000FFFFFFFFFFF

#define SAI_QOS_CHILD_INDEX_INVALID  0xFFFFFFFF
#define SAI_QOS_MAP_INVALID_TYPE     (-1)

#define SAI_MAX_QUEUE_TYPE_STRLEN 16
#define SAI_MAX_DROP_TYPE_STRLEN  16
#define SAI_MAX_SCHED_TYPE_STRLEN 32
#define SAI_MAX_METER_TYPE_STRLEN 16
#define SAI_MAX_HIERARCHY_STRLEN  16
#define SAI_HIERARCHY_LEVEL_CHAR  "\t"
#define SAI_HIERARCHY_LEVEL_CHAR_LEN 1


/**
 * @brief Accessor function for SAI Qos global config structure.
 *
 * @return Pointer to the global config structure instance.
 */
dn_sai_qos_global_t *sai_qos_access_global_config (void);

/**
 * @brief Accessor function for SAI Hqos structure.
 *
 * @return Pointer to the global hqos structure instance.
 */
dn_sai_qos_hierarchy_t *sai_qos_default_hqos_get (void);

/**
 * @brief Accessor function for SAI Cpu Hqos structure.
 *
 * @return Pointer to the global cpu hqos structure instance.
 */
dn_sai_qos_hierarchy_t *sai_qos_default_cpu_hqos_get (void);

/**
 * @brief Accessor function for SAI default scheduler id.
 *
 * @return Default sai scheduler object id.
 */
sai_object_id_t sai_qos_default_sched_id_get (void);

/**
 * @brief API to update SAI default scheduler id.
 * @param[in] Created default scheduler id
 */
void sai_qos_default_sched_id_set (sai_object_id_t sched_id);

/**
 * @brief Utility to take simple mutex lock for Qos resources access.
 */
void sai_qos_lock (void);

/**
 * @brief Utility to release simple mutex lock for Qos resources access.
 */
void sai_qos_unlock (void);

/**
 * @brief Utility to get first Qos DLL Node
 * @param[in] p_dll_head DLL head
 * @return First DLL Node
 */
static inline std_dll *sai_qos_dll_get_first (std_dll_head *p_dll_head)
{
    return (std_dll_getfirst (p_dll_head));
}

/**
 * @brief Utility to get Next Qos DLL Node
 * @param[in] p_dll_head DLL head
 * @param[in] p_dll Previous DLL Node
 * @return DLL Node
 */
static inline std_dll *sai_qos_dll_get_next (std_dll_head *p_dll_head,
                                             std_dll *p_dll)
{
    return ((p_dll != NULL) ? std_dll_getnext (p_dll_head, p_dll) : NULL);
}


/**
 * @brief Utility to check is_init_complete flag for SAI Qos component.
 *
 * @return Value of the is_init_complete flag.
 */
static inline bool sai_qos_is_init_complete (void)
{
    return (sai_qos_access_global_config()->is_init_complete);
}

/**
 * @brief Utility to Set is_init_complete flag after SAI Qos component init/deinit.
 */
static inline void sai_qos_init_complete_set (bool is_init_complete)
{
    sai_qos_access_global_config()->is_init_complete = is_init_complete;
}

/**
 * @brief Utility convert SAI queue types to string name.
 *
 * @param[in] queue_type  SAI Queue type.
 * @return Name string of queue type.
 */
static inline const char *sai_qos_queue_type_to_str (sai_queue_type_t queue_type)
{
    if (queue_type == SAI_QUEUE_TYPE_UNICAST) {
        return "Unicast";
    } else if (queue_type == SAI_QUEUE_TYPE_MULTICAST) {
        return "Multicast";
    } else {
        return "All";
    }
}

/**
 * @brief Utility to check seperate unicast and multicast queues supported on port.
 *
 * @param[in] port_id  SAI Port UOID.
 * @return true if suported else false.
 */
static inline bool sai_qos_queue_is_seperate_ucast_and_mcast_supported (sai_object_id_t port_id)
{
    return ((sai_switch_max_uc_queues_per_port_get (port_id) != 0) &&
            (sai_switch_max_mc_queues_per_port_get (port_id) != 0) ? true : false);
}

/**
 * @brief Utility to check seperate unicast or multicast queues supported on port.
 *
 * @param[in] port_id  SAI Port UOID.
 * @return true if suported else false.
 */
static inline bool sai_qos_queue_is_seperate_ucast_or_mcast_supported (sai_object_id_t port_id)
{
    return ((sai_switch_max_uc_queues_per_port_get (port_id) != 0) ||
            (sai_switch_max_mc_queues_per_port_get (port_id) != 0) ? true : false);
}

/**
 * @brief Utility to check queue type is unicast.
 *
 * @param[in] queue_type  Queue Type.
 * @return true if type is unicast else false.
 */
static inline bool sai_qos_is_queue_type_ucast (sai_queue_type_t queue_type)
{
    return ((queue_type == SAI_QUEUE_TYPE_UNICAST) ? true : false);
}

/**
 * @brief Utility to check queue type is multicast.
 *
 * @param[in] queue_type  Queue Type.
 * @return true if type is multicast else false.
 */
static inline bool sai_qos_is_queue_type_mcast (sai_queue_type_t queue_type)
{
    return ((queue_type == SAI_QUEUE_TYPE_MULTICAST) ? true : false);
}

/**
 * @brief Utility to check queue type is unicast or multicast.
 *
 * @param[in] queue_type  Queue Type.
 * @return true if type is unicast/multicast else false.
 */
static inline bool sai_qos_is_queue_type_ucast_or_mcast (sai_queue_type_t queue_type)
{
    return (((queue_type == SAI_QUEUE_TYPE_UNICAST) ||
             (queue_type == SAI_QUEUE_TYPE_MULTICAST)) ? true : false);
}

/**
 * @brief Utility to check hierarchy supported in switch.
 *
 * @return true if supported else false.
 */
static inline bool sai_qos_is_hierarchy_qos_supported (void)
{
    return ((sai_switch_max_hierarchy_levels_get () != 0) ? true : false);
}

/**
 * @brief Utility to check hierarchy supported is flexibile to modify in switch.
 *
 * @return true if supported else false.
 */
static inline bool sai_qos_is_fixed_hierarchy_qos (void)
{
    return ((sai_switch_info_get()->hierarchy_fixed) ? true : false);
}

/**
 * @brief Get SAI QOS Port node.
 *
 * @param[in] port_id    port id
 * @return Pointer to the Qos Port node if qos port id is found otherwise NULL
 */
dn_sai_qos_port_t* sai_qos_port_node_get (sai_object_id_t port_id);

/**
 * @brief Get first qos port node from port tree.
 *
 * @return Pointer to the first qos port node in tree.
 */
dn_sai_qos_port_t *sai_qos_port_node_get_first (void);

/**
 * @brief Get next qos port node from port tree.
 * Helper to scan QOS PORT nodes.
 *
 * @param[in] p_qos_port_node   Pointer to the Qos port node
 * @return Pointer to the next qos port node in tree
 */
dn_sai_qos_port_t *sai_qos_port_node_get_next (dn_sai_qos_port_t *p_qos_port_node);

/**
 * @brief Get first Queue node from PORT's Queue list.
 * Helper to scan PORT Queue list.
 *
 * @param[in] p_qos_port_node   Pointer to the Qos port node
 * @return Pointer to the first Queue node in DLL
 */
dn_sai_qos_queue_t *sai_qos_port_get_first_queue (
                                      dn_sai_qos_port_t *p_qos_port_node);

/**
 * @brief Get next Queue node from PORT's Queue list.
 * Helper to scan PORT Queue list.
 *
 * @param[in] p_qos_port_node   Pointer to the QOS Port node
 * @param[in] p_queue_node  Pointer to the Queue node to which next node is got
 * @return Pointer to the next Queue node in DLL
 */
dn_sai_qos_queue_t *sai_qos_port_get_next_queue (
                                            dn_sai_qos_port_t *p_qos_port_node,
                                            dn_sai_qos_queue_t *p_queue_node);

/**
 * @brief Get first mcast Queue node from PORT's Queue list.
 * Helper to scan PORT Queue list.
 *
 * @param[in] p_qos_port_node   Pointer to the Qos port node
 * @return Pointer to the first mcast Queue node in DLL
 */
dn_sai_qos_queue_t *sai_qos_port_get_first_mcast_queue(
                                            dn_sai_qos_port_t *p_qos_port_node);

/**
 * @brief Remove Policer node from tree
 *
 * @param[in] Policer id to remove
 */
void sai_qos_policer_node_remove(sai_object_id_t policer_id);

/**
 * @brief Insert Policer node into tree
 *
 * @param[in] p_policer_node policer node to insert
 * @return STD_ERR_OK on success or error otherwise
 */
t_std_error sai_qos_policer_node_insert(dn_sai_qos_policer_t *p_policer_node);

/**
 * @brief Get SAI QOS Policer node.
 *
 * @param[in] policer_id policer_id
 * @return Pointer to the policer node if policer id is found otherwise NULL
 */
dn_sai_qos_policer_t *sai_qos_policer_node_get(sai_object_id_t policer_id);
/**
 * @brief Get SAI QOS Queue node.
 *
 * @param[in] queue_id    Queue id
 * @return Pointer to the Queue node if queue id is found otherwise NULL
 */
dn_sai_qos_queue_t  *sai_qos_queue_node_get (sai_object_id_t queue_id);

/**
 * @brief Utility to get queue type
 *
 * @param[in] queue_id Queue ID.
 * @return SAI_QUEUE_TYPE_UNICAST or SAI_QUEUE_TYPE_MULTICAST
 */
sai_queue_type_t sai_qos_get_queue_type (sai_object_id_t queue_id);

/**
 * @brief Get SAI QOS Scheduler Group node.
 *
 * @param[in] sg_id    Scheduler Group id
 * @return Pointer to the Scheduler group node if scheduler group id
 * is found otherwise NULL
 */
dn_sai_qos_sched_group_t *sai_qos_sched_group_node_get (sai_object_id_t sg_id);


/**
 * @brief Get first Scheduler group node from PORT's Scheduler group
 * list of each hierarchy level. Helper to scan PORT Scheduler group list.
 *
 * @param[in] p_qos_port_node   Pointer to the Qos port node
 * @param[in] level             Hierarchy Level
 * @return Pointer to the first Scheduler group node in DLL
 */
dn_sai_qos_sched_group_t *sai_qos_port_get_first_sched_group (
                                  dn_sai_qos_port_t *p_qos_port_node,
                                  uint_t level);

/**
 * @brief Get next Scheduler group node from PORT's Scheduler group list.
 * Helper to scan PORT Scheduler group list.
 *
 * @param[in] p_qos_port_node    Pointer to the QOS Port node
 * @param[in] p_sg_node Pointer to the scheduler group node to
 * which next node is got
 * @return Pointer to the next Scheduler group node in DLL
 */
dn_sai_qos_sched_group_t *sai_qos_port_get_next_sched_group (
                                   dn_sai_qos_port_t *p_qos_port_node,
                                   dn_sai_qos_sched_group_t *p_sg_node);

/**
 * @brief Get first Child Scheduler group node from
 * scheduler groups child list. Helper to scan scheduler group's child list.
 *
 * @param[in] p_sg_node   Pointer to the Scheduler group node
 * @return Pointer to the first child scheduler group node in DLL
 */
dn_sai_qos_sched_group_t *sai_qos_sched_group_get_first_child_sched_group (
                                                     dn_sai_qos_sched_group_t *p_sg_node);

/**
 * @brief Get next child scheduler group node from scheduler groups child list.
 * Helper to scan scheduler group's child list list.
 *
 * @param[in] p_sg_node Pointer to the Scheduler group node
 * @param[in] p_child_sg_node Pointer to the child scheduler group node to
 * which next node is got
 * @return Pointer to the next child scheduler group node in DLL
 */
dn_sai_qos_sched_group_t *sai_qos_sched_group_get_next_child_sched_group (
                                   dn_sai_qos_sched_group_t *p_sg_node,
                                   dn_sai_qos_sched_group_t *p_child_sg_node);
/**
 * @brief Get first Child queue node from scheduler group child list.
 * Helper to scan scheduler group hierarchy child queue list.
 *
 * @param[in] p_sg_node   Pointer to the scheduler group node
 * @return Pointer to the first child queue node in DLL
 */
dn_sai_qos_queue_t *sai_qos_sched_group_get_first_child_queue (
                                         dn_sai_qos_sched_group_t *p_sg_node);


/**
 * @brief Get next child queue node from scheduler group child list.
 * Helper to scan scheduler group hierarchy child queue list.
 *
 * @param[in] p_sg_node    Pointer to the Scheduler group node
 * @param[in] p_queue_node Pointer to the queue node to
 * which next node is got
 * @return Pointer to the next child queue node in DLL
 */
dn_sai_qos_queue_t *sai_qos_sched_group_get_next_child_queue (
                                   dn_sai_qos_sched_group_t *p_sg_node,
                                   dn_sai_qos_queue_t *p_queue_node);

/**
 * @brief Utility to get status of applications started initialization of hierarchy qos.
 *
 * @param[in] port_id   Port UOID.
 * @param[in/out] is_app_hqos_init_started  Value of the is_init_complete flag.
 *
 * @return  SAI_STATUS_SUCCESS if operation is successful otherwise a different
 * error code is returned.
 */
static inline sai_status_t sai_qos_port_hierarchy_init_from_application_status_get (
                                                       sai_object_id_t port_id,
                                                       bool *is_app_hqos_init_started)
{
    dn_sai_qos_port_t *p_qos_port_node = NULL;

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *is_app_hqos_init_started = p_qos_port_node->is_app_hqos_init;

    return SAI_STATUS_SUCCESS;
}

/**
 * @brief Get first free child index in scheduler group node.
 *
 * @param[in] sg_id  Parent scheduler group id.
 * @param[out] child_index First free position in the bitmap.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_sched_group_first_free_child_index_get (sai_object_id_t sg_id,
                                                             uint_t *child_index);

/**
 * @brief Free child index in scheduler group node.
 *
 * @param[in] sg_id Parent scheduler group id.
 * @param[in] child_index Child index to be release.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_sched_group_child_index_free (sai_object_id_t sg_id,
                                                   uint_t child_index);

/**
 * @brief Utility to get the parent of the child queue/scheduler group oid.
 *
 * @param[in] child_id UOID of child queue/scheduler group.
 * @param[out] p_child_parent_id UOID of parent.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_child_parent_id_get (sai_object_id_t child_id,
                                          sai_object_id_t *p_child_parent_id);
/**
 * @brief Update index in hierarchy child node.
 *
 * @param[in] child_id UOID of child hierarchy node.
 * @param[in] child_index Index to be updated in child node.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_child_index_update (sai_object_id_t child_id, uint_t child_index);

/**
 * @brief Get index in hierarchy child node.
 *
 * @param[in] child_id UOID of child hierarchy node.
 * @param[out] child_index Child index in node.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_child_index_get (sai_object_id_t child_id, uint_t *child_index);

/**
 * @brief Get child list of scheduler group.
 *
 * @param[in] p_sg_node        Pointer to the Scheduler group node
 * @param[out] p_child_objlist Pointer to child object id's list.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_sched_group_child_id_list_get (dn_sai_qos_sched_group_t *p_sg_node,
                                                    sai_object_list_t *p_child_objlist);

/**
 * @brief Get scheduler group id list on port.
 *
 * @param[in] port_id       Port UOID.
 * @param[out] p_sg_objlist Pointer to scheduler group object id's list.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_port_sched_group_id_list_get (sai_object_id_t port_id,
                                             sai_object_list_t *p_sg_objlist);

/**
 * @brief Get queue list on port.
 *
 * @param[in] port_id          Port UOID.
 * @param[out] p_queue_objlist Pointer to queue object id's list.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_port_queue_id_list_get (sai_object_id_t port_id,
                                             sai_object_list_t *p_queue_objlist);
/**
 * @brief Get scheduler group count on port.
 *
 * @param[in] port_id          Port UOID.
 * @param[out] count           Number of scheduler groups in the port
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_port_sched_group_count_get (sai_object_id_t port_id, uint32_t *count);

/**
 * @brief Get queue count on port.
 *
 * @param[in] port_id          Port UOID.
 * @param[out] count           Number of queues in the port
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_port_queue_count_get (sai_object_id_t port_id, uint32_t *count);

/**
 * @brief Validate the incoming maps attribute values.
 *
 * @param[in]  dot1p Incoming dot1p value
 * @param[in]  tc    Incoming tc value
 * @param[in]  color Incoming color value
 * @param[in]  dscp  Incoming dscp value
 * @return SAI_STATUS_SUCCESS on success or failure otherwise
 */
static inline sai_status_t sai_qos_maps_attr_value_validate(uint_t dot1p,
                                     uint_t tc, uint_t color, uint_t dscp)
{
    if((dot1p >= SAI_QOS_MAX_DOT1P) ||
       (dscp >= SAI_QOS_MAX_DSCP) ||
       (color >= SAI_QOS_MAX_PACKET_COLORS) ||
       (tc >= SAI_QOS_MAX_TC)) {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    else{
        return SAI_STATUS_SUCCESS;
    }
}

/**
 * @brief Utility function to embed the type into the id from hardware.
 * @param[in] id hardwareid returned by npu
 * @param[in] type type of the policer or maptype
 *
 * @return qos objectid embedded with type.
 */
sai_npu_object_id_t sai_add_type_to_object(sai_npu_object_id_t id,
                                           uint_t type);

/**
 * @brief Utility function to get the type from npu object id.
 * @param[in] id npu object id
 *
 * @return type embedded in npu map object. Policer or maptype
 */
uint_t sai_get_type_from_npu_object(sai_npu_object_id_t id);

/**
 * @brief Utility function to get the id from objectid
 *        after stripping out the type.
 * @param[in] id  objectid with type embedded
 *
 * @return object id  with type removed
 */
sai_npu_object_id_t sai_get_id_from_npu_object(sai_npu_object_id_t id);

/**
 * @brief Utility to get a node from the qos maps tree.
 *
 * @param[in] map_id Map object id to be searched in the tree.
 * @return Node of type dn_sai_qos_map_t * or NULL on error.
 */
dn_sai_qos_map_t *sai_qos_map_node_get(const sai_object_id_t map_id);


/**
 * @brief Utility to insert nodes in qos maps tree.
 *
 * @param[in] p_map_node Node to be inserted.
 * @return STD_ERR_OK on success or failure otherwise.
 */
t_std_error sai_qos_map_node_insert(const dn_sai_qos_map_t *p_map_node);


/**
 * @brief Utility to remove nodes in qos maps   tree.
 *
 * @param[in] map_id Map object id to be removed from tree.
 */
void sai_qos_map_node_remove(sai_object_id_t map_id);

/**
 * @brief Get first port node from map DLL
 *
 * @param[in] p_map_node   Pointer to the Qos Map Node
 * @return Pointer to the first Port node in DLL
 */
dn_sai_qos_port_t *sai_qos_maps_get_port_node_from_map (dn_sai_qos_map_t *p_map_node);

/**
 * @brief Get next port node from map DLL
 * @param[in] p_map_node   Pointer to the Qos Map Node
 * @param[in] p_port_node   Pointer to the Port Node
 *
 * @return Pointer to the next node in  Port node in DLL
 */

dn_sai_qos_port_t *sai_qos_maps_next_port_node_from_map_get(dn_sai_qos_map_t *p_map_node,
                                                            dn_sai_qos_port_t *p_port_node);

/**
 * @brief Get next port node from map DLL
 * @param[in] p_map_node   Pointer to the Qos Map Node
 * @param[in] p_port_node   Pointer to the Port Node
 *
 * @return Pointer to the next node in  Port node in DLL
 */

dn_sai_qos_port_t *sai_qos_maps_next_port_node_from_map_get(dn_sai_qos_map_t *p_map_node,
                                                            dn_sai_qos_port_t *p_port_node);
/**
 * @brief Get the multicast qobject id of the passed index of a port
 * @param[in] p_qos_port_node Pointer to the Port node
 * @param[in] queue_index Index of the queue node to get
 *
 * @return Pointer to the queue node or NULL
 */
dn_sai_qos_queue_t *sai_qos_port_get_indexed_mc_queue_object(dn_sai_qos_port_t *p_qos_port_node,
                                                             uint_t queue_index);
/**
 * @brief Get the unicast qobject id of the passed index of a port
 * @param[in] p_qos_port_node Pointer to the Port node
 * @param[in] queue_index Index of the queue node to get
 *
 * @return Pointer to the queue node or NULL
 */
dn_sai_qos_queue_t *sai_qos_port_get_indexed_uc_queue_object(dn_sai_qos_port_t *p_qos_port_node,
                                                             uint_t queue_index);

/**
 * @brief Function to get the first acl entry from the policer node.
 * @param[in] p_policer Pointer to policer node
 *
 * @return Pointer to acl_rule or NULL
 */
sai_acl_rule_t *sai_qos_first_acl_rule_node_from_policer_get(dn_sai_qos_policer_t *p_policer);

/**
 * @brief Function to get the next acl entry from the policer.
 * @param[in] p_policer Pointer to policer node
 * @param[in] p_acl_node Pointer to acl node to get the next node
 *
 * @return Pointer to acl_rule or NULL
 */
sai_acl_rule_t *sai_qos_next_acl_rule_node_from_policer_get(dn_sai_qos_policer_t *p_policer,
                                                            sai_acl_rule_t *p_acl_node);

/**
 * @brief Get next port node from policer port list.
 *
 * @param[in] p_policer Pointer to the Policer node
 * @param[in] p_port_node Pointer to the port node
 * @param[in] type type of storm control policer
 * @return Pointer to the next port node in DLL or NULL
 */
dn_sai_qos_port_t *sai_qos_next_port_node_from_policer_get(dn_sai_qos_policer_t *p_policer,
                                                           dn_sai_qos_port_t *p_port_node,
                                                           uint_t type);

/**
 * @brief Get next port node from policer port list.
 *
 * @param[in] p_policer Pointer to the Policer node
 * @param[in] type type of storm control policer
 * @return Pointer to the first port node in DLL or NULL
 */
dn_sai_qos_port_t *sai_qos_port_node_from_policer_get(dn_sai_qos_policer_t *p_policer,
                                                      uint_t type);

/**
 * @brief Get SAI QOS Scheduler node.
 *
 * @param[in] sched_id    Scheduler id
 * @return Pointer to the Qos Scheduler node if qos scheduler id is found otherwise NULL
 */
dn_sai_qos_scheduler_t *sai_qos_scheduler_node_get (sai_object_id_t sched_id);

/**
 * @brief Get first Queue node from Scheduler's queue list.
 * Helper to scan Scheduler applied queue list.
 *
 * @param[in] p_sched_node  Pointer to the Scheduler node
 * @return Pointer to the first Queue node in DLL
 */
dn_sai_qos_queue_t *sai_qos_scheduler_get_first_queue (dn_sai_qos_scheduler_t *p_sched_node);

/**
 * @brief Get next Queue node from Scheduler's queue list.
 * Helper to scan Scheduler applied queue list.
 *
 * @param[in] p_sched_node   Pointer to the Scheduler node
 * @param[in] p_queue_node   Pointer to the queue node to
 * which next node is got
 * @return Pointer to the next Queue node in DLL
 */
dn_sai_qos_queue_t *sai_qos_scheduler_get_next_queue (dn_sai_qos_scheduler_t *p_sched_node,
                                                      dn_sai_qos_queue_t *p_queue_node);

/**
 * @brief Get first Scheduler group from Scheduler's scheduler group list.
 * Helper to scan Scheduler applied scheduler group list.
 *
 * @param[in] p_sched_node  Pointer to the Scheduler node
 * @return Pointer to the first Scheduler group node in DLL
 */
dn_sai_qos_sched_group_t *sai_qos_scheduler_get_first_sched_group (
                                      dn_sai_qos_scheduler_t *p_sched_node);

/**
 * @brief Get next Scheduler group node from Scheduler's scheduler group list.
 * Helper to scan Scheduler applied scheduler group list.
 *
 * @param[in] p_sched_node   Pointer to the Scheduler node
 * @param[in] p_sg_node      Pointer to the scheduler group node to
 * which next node is got
 * @return Pointer to the next scheduler group node in DLL
 */
dn_sai_qos_sched_group_t *sai_qos_scheduler_get_next_sched_group (
                                      dn_sai_qos_scheduler_t *p_sched_node,
                                      dn_sai_qos_sched_group_t *p_sg_node);

/**
 * @brief Get first Qos port node from Scheduler's port list.
 * Helper to scan Scheduler applied port list.
 *
 * @param[in] p_sched_node  Pointer to the Scheduler node
 * @return Pointer to the first port node in DLL
 */
dn_sai_qos_port_t *sai_qos_scheduler_get_first_port (dn_sai_qos_scheduler_t *p_sched_node);

/**
 * @brief Get next port node from Scheduler's port list.
 * Helper to scan Scheduler applied port list.
 *
 * @param[in] p_sched_node   Pointer to the Scheduler node
 * @param[in] p_qos_port_node    Pointer to the port node to
 * which next node is got
 * @return Pointer to the next port node in DLL
 */
dn_sai_qos_port_t *sai_qos_scheduler_get_next_port (dn_sai_qos_scheduler_t *p_sched_node,
                                                    dn_sai_qos_port_t *p_qos_port_node);

/**
 * @brief Utility to get a node from the wred tree.
 *
 * @param[in] wred_id Wred object id to be searched in the tree.
 * @return Node of type dn_sai_qos_wred_t * or NULL on error.
 */
dn_sai_qos_wred_t *sai_qos_wred_node_get(const sai_object_id_t wred_id);


/**
 * @brief Utility to insert nodes in wred tree.
 *
 * @param[in] p_wred_node Node to be inserted.
 * @return STD_ERR_OK on success or failure otherwise.
 */
t_std_error sai_qos_wred_node_insert(const dn_sai_qos_wred_t *p_wred_node);

/**
 * @brief Utility to remove nodes in wred tree.
 *
 * @param[in] wred_id Wred object id to be removed from tree.
 */
void sai_qos_wred_node_remove(sai_object_id_t wred_id);

/**
 * @brief Utility to get WRED link text given link type
 *
 * @param[in] wred_link_type Type of the WRED link
 * @return Pointer to link text string or NULL
 */
const char *sai_qos_wred_link_str(dn_sai_qos_wred_link_t wred_link_type);

/**
 * @brief Utility to get WRED link ID given link node and type
 *
 * @param[in] p_wred_link_node Pointer to the WRED link node
 * @param[in] wred_link_type Type of the WRED link
 * @return Valid WRED link SAI object ID or SAI_NULL_OBJECT_ID
 */
sai_object_id_t sai_qos_wred_link_oid_get(void *p_wred_link_node,
        dn_sai_qos_wred_link_t wred_link_type);

/**
 * @brief Utility to WRED link dll head given WRED node and link type
 *
 * @param[in] p_wred_node Pointer to the WRED node
 * @param[in] wred_link_type Type of the WRED link
 * @return Pointer to valid dll head or NULL
 */
std_dll_head *sai_qos_wred_link_get_head_ptr(
        dn_sai_qos_wred_t *p_wred_node,
        dn_sai_qos_wred_link_t wred_link_type);

/**
 * @brief Utility to get the WRED ID given the WRED link ID and link type.
 *
 * @param[in] wred_link_id SAI object ID of the WRED link
 * @param[in] wred_link_type Type of the WRED link
 * @return Valid SAI WRED ID or SAI_NULL_OBJECT_ID
 */
sai_object_id_t sai_qos_wred_link_wred_id_get(sai_object_id_t wred_link_id,
        dn_sai_qos_wred_link_t dn_wred_link);

/**
 * @brief Utility to check if the WRED cache is marked or not for
 * given WRED link ID and link type.
 *
 * @param[in] wred_link_id SAI object ID of the WRED link
 * @param[in] wred_link_type Type of the WRED link
 * @return true if marked; false otherwise
 */
bool sai_qos_wred_link_is_sw_cached(sai_object_id_t wred_link_id,
        dn_sai_qos_wred_link_t wred_link_type);

/**
 * @brief Utility to mark the cache for given WRED link ID and link type.
 *
 * @param[in] wred_link_id SAI object ID of the WRED link
 * @param[in] wred_link_type Type of the WRED link
 * @return SAI_STATUS_SUCCESS or appropriate SAI error
 */
sai_status_t sai_qos_wred_link_mark_sw_cache(sai_object_id_t wred_link_id,
        dn_sai_qos_wred_link_t wred_link_type);

/**
 * @brief Utility to unmark the cache for given WRED link ID and link type.
 *
 * @param[in] wred_link_id SAI object ID of the WRED link
 * @param[in] wred_link_type Type of the WRED link
 * @return SAI_STATUS_SUCCESS or appropriate SAI error
 */
sai_status_t sai_qos_wred_link_unmark_sw_cache(sai_object_id_t wred_link_id,
        dn_sai_qos_wred_link_t wred_link_type);

/**
 * @brief Utility to get the first link node based on the WRED link type.
 * This is used for walking the WRED link types attached to this WRED profile.
 *
 * @param[in] p_wred_node Pointer to wred node which has WRED link list
 * @param[in] wred_link_type Type of the WRED link
 * @return void pointer to the link node or NULL
 */
void *sai_qos_wred_link_node_get_first(dn_sai_qos_wred_t *p_wred_node,
        dn_sai_qos_wred_link_t wred_link_type);

/**
 * @brief Utility to get the next link node based on the WRED link type.
 * This is used for walking the WRED link types attached to this WRED profile.
 *
 * @param[in] p_wred_node Pointer to wred node which has queue list
 * @param[in] p_wred_link_node Pointer to current link node to return next link node
 * @param[in] wred_link_type Type of the WRED link
 * @return void pointer to the link node or NULL
 */
void *sai_qos_wred_link_node_get_next(dn_sai_qos_wred_t *p_wred_node,
        void* p_wred_link_node, dn_sai_qos_wred_link_t wred_link_type);

/**
 * @brief Utility to insert the link glue node to WRED
 * link list based on the WRED link type.
 *
 * @param[in] wred_id SAI WRED object ID
 * @param[in] wred_link_id SAI WRED link object ID
 * @param[in] dn_wred_link Type of the WRED link
 * @return SAI_STATUS_SUCCESS or appropriate SAI error
 */
sai_status_t sai_qos_wred_link_insert(sai_object_id_t wred_id,
        sai_object_id_t wred_link_id, dn_sai_qos_wred_link_t dn_wred_link);

/**
 * @brief Utility to remove the link glue node from WRED
 * link list based on the WRED link type.
 *
 * @param[in] wred_id SAI WRED object ID
 * @param[in] wred_link_id SAI WRED link object ID
 * @param[in] dn_wred_link Type of the WRED link
 * @return SAI_STATUS_SUCCESS or appropriate SAI error
 */
sai_status_t sai_qos_wred_link_remove(sai_object_id_t wred_id,
        sai_object_id_t wred_link_id, dn_sai_qos_wred_link_t dn_wred_link);

/**
 * @brief Utility to map the map port attr to map type.
 *
 * @param[in] port_attr Port attribute to set or get
 * @return Map type corresponding to the port map attribute
 */
sai_qos_map_type_t sai_get_map_type_from_port_attr(sai_attr_id_t port_attr);

/**
 * @brief Get the qobject id of the passed index of a port
 * @param[in] p_qos_port_node Pointer to the Port node
 * @param[in] queue_index Index of the queue node to get
 *
 * @return Pointer to the queue node or NULL
 */
dn_sai_qos_queue_t *sai_qos_port_get_indexed_queue_object(dn_sai_qos_port_t *p_qos_port_node,
                                                          uint_t queue_index);

/**
 * @brief Get the number of priority groups in the port
 * @param[in] port_id Port Object ID
 * @param[out] num_pg Number of Priority groups in the port
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_port_get_num_pg (sai_object_id_t port_id, uint_t *num_pg);

/**
 * @brief Get the List of priority groups in the port
 * @param[in] port_id Port Object ID
 * @param[out] pg_list List of Priority groups in the port
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_port_get_pg_list (sai_object_id_t port_id,
                                       sai_object_list_t *pg_list);


/**
 * @brief Get the tc for pg programming
 * @param[in] port_id Port Object ID
 * @param[in] pg_id Priority Group indentifier
 * @param[out] tc Traffic class that maps to the PG
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_get_tc_from_pg (sai_object_id_t port_id, uint_t pg_id, uint_t *tc);

/**
 * @brief Validate whether the child sg can be attached to the parent
 * @param[in] p_child_node Child scheduler group node
 * @param[in] p_parent_node Parent scheduler group node
 *
 * @return SAI_STATUS_SUCCESS if validation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_sched_group_validate_child_parent(
                                     dn_sai_qos_sched_group_t *p_child_node,
                                     dn_sai_qos_sched_group_t *p_parent_node);

/**
 * @brief Validate whether the child queue can be attached to the parent
 * @param[in] p_child_node Child queue node
 * @param[in] p_parent_node Parent scheduler group node
 *
 * @return SAI_STATUS_SUCCESS if validation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_qos_queue_validate_child_parent(
                                     dn_sai_qos_queue_t *p_queue_node,
                                     dn_sai_qos_sched_group_t *p_parent_node);

sai_status_t sai_qos_sched_group_and_child_nodes_update (
                                          sai_object_id_t sg_id,
                                          sai_object_id_t child_id,
                                          bool is_add);

/** Logging utility for SAI POLICER API */
#define SAI_POLICER_LOG(level, msg, ...) \
        if (sai_is_log_enabled (SAI_API_POLICER, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_POLICER, level, msg, ##__VA_ARGS__); \
       }

/** Logging utility for SAI Scheduler Group API */
#define SAI_SCHED_GRP_LOG(level, msg, ...) \
        if (sai_is_log_enabled (SAI_API_SCHEDULER_GROUP, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_SCHEDULER_GRP, level, msg, ##__VA_ARGS__); \
        }

/** Logging utility for SAI Queue API */
#define SAI_QUEUE_LOG(level, msg, ...) \
        if (sai_is_log_enabled (SAI_API_QUEUE, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_QUEUE, level, msg, ##__VA_ARGS__); \
        }

/** Logging utility for SAI Scheduler API */
#define SAI_SCHED_LOG(level, msg, ...) \
        if (sai_is_log_enabled (SAI_API_SCHEDULER, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_SCHEDULER, level, msg, ##__VA_ARGS__); \
        }

/** Logging utility for SAI WRED API */
#define SAI_WRED_LOG(level, msg, ...) \
        if (sai_is_log_enabled (SAI_API_WRED, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_WRED, level, msg, ##__VA_ARGS__); \
       }

/**
 * @brief   SAI maps specific trace logging function
 */
#define SAI_MAPS_LOG(level, msg, ...) \
        if (sai_is_log_enabled (SAI_API_QOS_MAP, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_MAPS, level, msg, ##__VA_ARGS__); \
       }

/**
 * @brief   SAI Qos Initialization specific trace logging function
 */
#define SAI_QOS_LOG(level, msg, ...) \
        if (sai_is_log_enabled (SAI_API_QOS_MAP, level) || \
            (sai_is_log_enabled (SAI_API_QUEUE, level)) || \
            (sai_is_log_enabled (SAI_API_WRED, level)) || \
            (sai_is_log_enabled (SAI_API_POLICER, level))|| \
            (sai_is_log_enabled (SAI_API_SCHEDULER, level))|| \
            (sai_is_log_enabled (SAI_API_SCHEDULER_GROUP, level)) || \
            (sai_is_log_enabled (SAI_API_BUFFER, level))) { \
            SAI_LOG_UTIL(ev_log_t_SAI_QOS, level, msg, ##__VA_ARGS__); \
       }

/** Per log level based macros for SAI Scheduler API */
#define SAI_SCHED_LOG_TRACE(msg, ...) \
        SAI_SCHED_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_SCHED_LOG_CRIT(msg, ...) \
        SAI_SCHED_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_SCHED_LOG_ERR(msg, ...) \
        SAI_SCHED_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_SCHED_LOG_INFO(msg, ...) \
        SAI_SCHED_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_SCHED_LOG_WARN(msg, ...) \
        SAI_SCHED_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_SCHED_LOG_NTC(msg, ...) \
        SAI_SCHED_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/** Per log level based macros for SAI Scheduler group API */
#define SAI_SCHED_GRP_LOG_TRACE(msg, ...) \
        SAI_SCHED_GRP_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_SCHED_GRP_LOG_CRIT(msg, ...) \
        SAI_SCHED_GRP_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_SCHED_GRP_LOG_ERR(msg, ...) \
        SAI_SCHED_GRP_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_SCHED_GRP_LOG_INFO(msg, ...) \
        SAI_SCHED_GRP_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_SCHED_GRP_LOG_WARN(msg, ...) \
        SAI_SCHED_GRP_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_SCHED_GRP_LOG_NTC(msg, ...) \
        SAI_SCHED_GRP_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/** Per log level based macros for SAI Queue API */
#define SAI_QUEUE_LOG_TRACE(msg, ...) \
        SAI_QUEUE_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_QUEUE_LOG_CRIT(msg, ...) \
        SAI_QUEUE_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_QUEUE_LOG_ERR(msg, ...) \
        SAI_QUEUE_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_QUEUE_LOG_INFO(msg, ...) \
        SAI_QUEUE_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_QUEUE_LOG_WARN(msg, ...) \
        SAI_QUEUE_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_QUEUE_LOG_NTC(msg, ...) \
        SAI_QUEUE_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/** Per log level based macros for SAI POLICER API */
#define SAI_POLICER_LOG_TRACE(msg, ...) \
        SAI_POLICER_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_POLICER_LOG_CRIT(msg, ...) \
        SAI_POLICER_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_POLICER_LOG_ERR(msg, ...) \
        SAI_POLICER_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_POLICER_LOG_INFO(msg, ...) \
        SAI_POLICER_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_POLICER_LOG_WARN(msg, ...) \
        SAI_POLICER_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_POLICER_LOG_NTC(msg, ...) \
        SAI_POLICER_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/** Per log level based macros for SAI WRED API */
#define SAI_WRED_LOG_TRACE(msg, ...) \
        SAI_WRED_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_WRED_LOG_CRIT(msg, ...) \
        SAI_WRED_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_WRED_LOG_ERR(msg, ...) \
        SAI_WRED_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_WRED_LOG_INFO(msg, ...) \
        SAI_WRED_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_WRED_LOG_WARN(msg, ...) \
        SAI_WRED_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_WRED_LOG_NTC(msg, ...) \
        SAI_WRED_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)


/** Per log level based macros for SAI QOS MAPS API */
#define SAI_MAPS_LOG_TRACE(msg, ...) \
        SAI_MAPS_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_MAPS_LOG_CRIT(msg, ...) \
        SAI_MAPS_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_MAPS_LOG_ERR(msg, ...) \
        SAI_MAPS_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_MAPS_LOG_INFO(msg, ...) \
        SAI_MAPS_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_MAPS_LOG_WARN(msg, ...) \
        SAI_MAPS_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_MAPS_LOG_NTC(msg, ...) \
        SAI_MAPS_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/** Per log level based macros for SAI QOS INIT API */
#define SAI_QOS_LOG_TRACE(msg, ...) \
        SAI_QOS_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_QOS_LOG_CRIT(msg, ...) \
        SAI_QOS_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_QOS_LOG_ERR(msg, ...) \
        SAI_QOS_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_QOS_LOG_INFO(msg, ...) \
        SAI_QOS_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_QOS_LOG_WARN(msg, ...) \
        SAI_QOS_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_QOS_LOG_NTC(msg, ...) \
        SAI_QOS_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)


/**
 * \}
 */

#endif /* __SAI_QOS_UTIL_H__ */
