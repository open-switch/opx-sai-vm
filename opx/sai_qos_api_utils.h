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
* @file sai_qos_api_utils.h
*
* @brief This file contains macro definitions and function prototypes
* used in the SAI Qos API implementation.
*
*************************************************************************/
#ifndef __SAI_QOS_API_UTILS_H__
#define __SAI_QOS_API_UTILS_H__

#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "std_config_node.h"
#include "sai_common_utils.h"

#include "saitypes.h"
#include "saistatus.h"
#include "saiport.h"

#define SAI_QOS_MAPS_MAX_ATTR_COUNT               (2)
#define SAI_QOS_MAX_QUEUE_IDX_SEPARATE            (2)
#define SAI_QOS_MAX_QUEUE_IDX_COMBINED            (1)
#define SAI_QOS_DLFT_SCHED_GROUPS_PER_LEVEL       (1)
#define SAI_QOS_CHILD_COUNT_ONE                   (1)
#define SAI_QOS_POLICER_MAX_ATTR_COUNT            (11)
#define SAI_QOS_POLICER_TYPE_INVALID              (-1)
#define SAI_QOS_WRED_MAX_ATTR_COUNT               (14)

sai_status_t sai_qos_port_all_init (void);

sai_status_t sai_qos_port_all_deinit (void);

sai_status_t sai_qos_port_queue_all_init (sai_object_id_t port_id);

sai_status_t sai_qos_port_queue_all_deinit (sai_object_id_t port_id);

sai_status_t sai_qos_port_queue_list_update (dn_sai_qos_queue_t *p_queue_node,
                                             bool is_add);

sai_status_t sai_qos_port_queue_create(sai_object_id_t port_id,
                                       sai_queue_type_t queue_type,
                                       uint8_t queue_index,
                                       sai_object_id_t parent_sg_id,
                                       sai_object_id_t *queue_id);

sai_status_t sai_qos_port_queue_remove(sai_object_id_t queue_id);

sai_status_t sai_qos_port_sched_group_create (sai_object_id_t port_id,
                                              sai_object_id_t parent_id,
                                              uint_t level,
                                              uint_t max_childs,
                                              sai_object_id_t *sg_id);

sai_status_t sai_qos_port_sched_group_remove(sai_object_id_t sg_id);

sai_status_t sai_qos_port_sched_groups_deinit (sai_object_id_t port_id);

sai_status_t sai_qos_port_sched_group_list_update (
                                    dn_sai_qos_sched_group_t *p_sg_node,
                                    bool is_add);

sai_status_t sai_qos_port_default_hierarchy_init (sai_object_id_t port_id);

sai_status_t sai_qos_port_hierarchy_deinit (sai_object_id_t port_id);

sai_status_t sai_qos_first_free_queue_get (sai_object_id_t port_id,
                                           sai_queue_type_t queue_type,
                                           sai_object_id_t *p_queue_id);

sai_status_t sai_qos_queue_id_list_get (sai_object_id_t port_id,
                                        uint_t queue_id_list_count,
                                        sai_object_id_t *p_queue_id_list);

sai_status_t sai_qos_sched_group_id_list_per_level_get (sai_object_id_t port_id,
                                                        uint_t level,
                                                        uint_t sg_id_list_count,
                                                        sai_object_id_t *p_sg_id_list);
sai_status_t sai_qos_indexed_sched_group_id_get(sai_object_id_t port_id,
                                                uint_t level,
                                                uint_t child_idx,
                                                sai_object_id_t *p_sg_id);

uint_t sai_qos_dlft_sched_groups_per_level_get (sai_object_id_t port_id,
                                                uint_t level);

sai_status_t sai_qos_map_type_attr_set(dn_sai_qos_map_t *p_map_node,
                                       sai_qos_map_type_t map_type);

void sai_qos_map_free_resources(dn_sai_qos_map_t *p_map_node);

sai_status_t sai_qos_map_on_port_set(sai_object_id_t port_id,
                                     const sai_attribute_t *attr,
                                     sai_qos_map_type_t map_type);

sai_status_t sai_qos_map_on_port_set_internal(sai_object_id_t port_id,
                                     const sai_attribute_t *attr,
                                     sai_qos_map_type_t map_type);

sai_status_t sai_qos_map_process_port_set (sai_object_id_t port_id,
                                           sai_object_id_t map_id,
                                           sai_qos_map_type_t map_type);

sai_status_t sai_qos_map_port_list_update(dn_sai_qos_map_t *p_map);

sai_status_t sai_qos_port_scheduler_set (sai_object_id_t port_id,
                                         const sai_attribute_t *p_attr);

sai_status_t sai_qos_queue_scheduler_set (dn_sai_qos_queue_t *p_queue_node,
                                          const sai_attribute_t *p_attr);

sai_status_t sai_qos_sched_group_scheduler_set (dn_sai_qos_sched_group_t *p_sg_node,
                                                const sai_attribute_t *p_attr);

sai_status_t sai_qos_scheduler_reapply (dn_sai_qos_scheduler_t *p_old_sched_node,
                                        dn_sai_qos_scheduler_t *p_new_sched_node);

sai_status_t sai_policer_acl_entries_update(dn_sai_qos_policer_t *p_policer,
                                            dn_sai_qos_policer_t *p_policer_new);

sai_status_t sai_port_attr_storm_control_policer_set(sai_object_id_t port_id,
                                                     const sai_attribute_t *attr);

sai_status_t sai_port_attr_storm_control_policer_set_internal(sai_object_id_t port_id,
                                                     const sai_attribute_t *attr);

sai_status_t sai_qos_wred_link_set(sai_object_id_t wred_link_id,
        sai_object_id_t wred_id, dn_sai_qos_wred_link_t dn_wred_link);

sai_status_t sai_switch_set_qos_default_tc(uint_t default_tc);

sai_status_t sai_qos_hierarchy_handler(std_config_node_t hqos_node,
                                       dn_sai_qos_hierarchy_t *hqos);

sai_status_t sai_qos_port_buffer_profile_set (sai_object_id_t obj_id,
                                              sai_object_id_t profile_id);

sai_status_t sai_qos_obj_update_buffer_profile (sai_object_id_t object_id,
                                               sai_object_id_t profile_id);

sai_status_t sai_buffer_init (void);

sai_status_t sai_qos_port_create_all_pg (sai_object_id_t port_id);

sai_status_t sai_qos_port_destroy_all_pg(sai_object_id_t port_id);

sai_status_t sai_qos_pg_attr_set (sai_object_id_t ingress_pg_id,
                                  const sai_attribute_t *attr);

sai_status_t sai_qos_pg_attr_get (sai_object_id_t ingress_pg_id,
                                  uint32_t attr_count,
                                  sai_attribute_t *attr_list);

sai_status_t sai_qos_pg_stats_get (sai_object_id_t pg_id, uint32_t number_of_counters,
                                   const sai_ingress_priority_group_stat_t *counter_ids,
                                   uint64_t* counters);

sai_status_t sai_qos_pg_stats_clear (sai_object_id_t pg_id, uint32_t number_of_counters,
                                     const sai_ingress_priority_group_stat_t *counter_ids);

sai_status_t sai_qos_pg_node_insert_to_tree (dn_sai_qos_pg_t *p_pg_node);

void sai_qos_pg_node_remove_from_tree (dn_sai_qos_pg_t *p_pg_node);

sai_status_t sai_qos_buffer_pool_node_insert_to_tree (dn_sai_qos_buffer_pool_t *p_buffer_pool_node);

void sai_qos_buffer_pool_node_remove_from_tree (dn_sai_qos_buffer_pool_t
                                                *p_buffer_pool_node);

sai_status_t sai_qos_buffer_profile_node_insert_to_tree (dn_sai_qos_buffer_profile_t
                                                         *p_buffer_profile_node);

sai_status_t sai_qos_buffer_profile_node_remove_from_tree (dn_sai_qos_buffer_profile_t
                                                          *p_buffer_profile_node);

sai_status_t sai_qos_update_buffer_pool_node (dn_sai_qos_buffer_pool_t
                                             *p_buf_pool_node,
                                              uint32_t attr_count,
                                              const sai_attribute_t *attr_list);
sai_status_t sai_qos_validate_shared_buffer_pool_size(
                                  dn_sai_qos_buffer_pool_t  *p_old_buf_pool_node,
                                  dn_sai_qos_buffer_pool_t  *p_buf_pool_node);

sai_status_t sai_qos_update_buffer_profile_node (dn_sai_qos_buffer_profile_t
                                                 *p_buf_profile_node,
                                                 uint32_t attr_count,
                                                 const sai_attribute_t *attr_list);

sai_status_t sai_qos_read_buffer_profile_node (dn_sai_qos_buffer_profile_t
                                               *p_buf_profile_node,
                                               uint32_t attr_count,
                                               sai_attribute_t *attr_list);

void sai_qos_init_buffer_pool_node (dn_sai_qos_buffer_pool_t *p_buf_pool_node);

sai_status_t sai_qos_obj_update_buffer_profile_node (sai_object_id_t obj_id,
                                                     sai_object_id_t buf_profile_id);

void sai_qos_init_buffer_profile_node (dn_sai_qos_buffer_profile_t
                                       *p_buf_profile_node);

bool sai_qos_is_buffer_pool_available (sai_object_id_t pool_id, uint_t size);

void sai_qos_get_default_buffer_profile (dn_sai_qos_buffer_profile_t *p_profile_node);

bool sai_qos_is_object_buffer_profile_compatible (sai_object_id_t object_id,
                                                  dn_sai_qos_buffer_profile_t
                                                  *p_profile_node);

bool sai_qos_is_buffer_pool_compatible (sai_object_id_t pool_id_1,
                                        sai_object_id_t pool_id_2, bool check_th);

bool sai_qos_buffer_profile_is_valid_threshold_applied (dn_sai_qos_buffer_profile_t
                                                        *p_profile_node,
                                                        const sai_attribute_t *attr);

sai_status_t sai_qos_buffer_profile_is_mandatory_th_attr_present (uint32_t attr_count,
                                                                  const sai_attribute_t
                                                                  *attr_list);

sai_status_t sai_qos_buffer_profile_is_pfc_threshold_valid (uint32_t attr_count,
                                                            const sai_attribute_t
                                                            *attr_list);

sai_status_t sai_qos_map_pfc_pri_to_queue_default_set (dn_sai_qos_map_t *p_map_node);

sai_status_t sai_qos_map_pfc_pri_to_queue_map_value_set (dn_sai_qos_map_t *p_map_node,
                                                         sai_qos_map_list_t map_list,
                                                         dn_sai_operations_t op_type);

sai_status_t sai_qos_map_tc_to_pg_default_set (dn_sai_qos_map_t *p_map_node);

sai_status_t sai_qos_map_tc_to_pg_map_value_set (dn_sai_qos_map_t *p_map_node,
                                                 sai_qos_map_list_t map_list,
                                                 dn_sai_operations_t op_type);

sai_status_t sai_qos_map_list_alloc(dn_sai_qos_map_t *p_map_node, uint_t count);

sai_status_t sai_qos_create_default_scheduler(sai_object_id_t *default_sched_id);

sai_status_t sai_qos_remove_default_scheduler();

sai_status_t sai_qos_port_deinit (sai_object_id_t port_id);

sai_status_t sai_qos_port_init (sai_object_id_t port_id);

static inline uint_t sai_qos_maps_get_max_queue_idx()
{
    /** @TODO: Value to be obtained from config file */
    return SAI_QOS_MAX_QUEUE_IDX_SEPARATE;
}

sai_status_t sai_get_port_attr_from_storm_control_type(dn_sai_qos_policer_type_t type,
                                                       sai_port_attr_t *attr_id);

sai_status_t sai_qos_sched_group_remove_configs (dn_sai_qos_sched_group_t *p_sg_node);

sai_status_t sai_qos_queue_remove_configs(dn_sai_qos_queue_t *p_queue_node);

/**
 * @brief Resets the Egress Pool based WRED and caches the current value,
 * if the input queue is the last unicast queue to be removed from the buffer pool.
 *
 * @param[in] queue_id SAI object id of type queue
 * @return SAI_STATUS_SUCCESS or appropriate SAI error
 */
sai_status_t sai_qos_wred_link_update_cache(sai_object_id_t queue_id);

/**
 * @brief Sets the Egress Pool based WRED to the cached value,
 * if the input queue is an unicast queue added to the buffer pool.
 *
 * @param[in] queue_id SAI object id of type queue
 * @return SAI_STATUS_SUCCESS or appropriate SAI error
 */
sai_status_t sai_qos_wred_link_apply_cache(sai_object_id_t queue_id);

/**
 * @brief Function init SAI OID gen utility
 */
void sai_qos_port_pool_oid_gen_init(void);

/**
 * @brief SAI CREATE function for Port Pool Object
 */
sai_status_t sai_qos_port_create_port_pool(
        sai_object_id_t *port_pool_id,
        sai_object_id_t switch_id,
        uint32_t attr_count,
        const sai_attribute_t *attr_list);

/**
 * @brief SAI REMOVE function for Port Pool Object
 */
sai_status_t sai_qos_port_remove_port_pool(sai_object_id_t port_pool_id);

/**
 * @brief SAI SET function for Port Pool Object
 */
sai_status_t sai_qos_port_set_port_pool_attribute(sai_object_id_t port_pool_id, const sai_attribute_t *attr);

/**
 * @brief SAI GET function for Port Pool Object
 */
sai_status_t sai_qos_port_get_port_pool_attribute(
        sai_object_id_t port_pool_id,
        uint32_t attr_count,
        sai_attribute_t *attr_list);

/**
 * @brief SAI GET STATS function for Port Pool Object
 */
sai_status_t sai_qos_port_get_port_pool_stats(
        sai_object_id_t port_pool_id,
        uint32_t number_of_counters,
        const sai_port_pool_stat_t *counter_ids,
        uint64_t *counters);

/**
 * @brief SAI CLEAR STATS function for Port Pool Object
 */
sai_status_t sai_qos_port_clear_port_pool_stats(
        sai_object_id_t port_pool_id,
        uint32_t number_of_counters,
        const sai_port_pool_stat_t *counter_ids);
#endif /* __SAI_QOS_API_UTILS_H__ */
