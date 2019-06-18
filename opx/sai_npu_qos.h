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
* @file sai_npu_qos.h
*
* @brief This file contains the API signatures defined for SAI NPU QOS
* component.
*
*************************************************************************/

#ifndef __SAI_NPU_QOS_H__
#define __SAI_NPU_QOS_H__

#include "sai_qos_common.h"
#include "sai_common_utils.h"
#include "saitypes.h"
#include "sai_oid_utils.h"
#include "std_type_defs.h"

/** \defgroup SAIQOSNPUAPIs SAI - QOS NPU specific function implementations
 *  NPU specific functions for SAI QOS component
 *
 *  \{
 */
/**
 * @brief Initialization of NPU specific Qos Switch/Global settings.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_global_init_fn) (void);

/**
 * @brief Initialization of NPU specific Qos Port settings.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_port_init_fn) (dn_sai_qos_port_t *p_qos_port_node);

/**
 * @brief Create a Queue in NPU.
 *
 * @param[in] p_queue_node Pointer to the queue node
 * @param[out] p_queue_oid Pointer to Queue OID
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_queue_create_fn) (dn_sai_qos_queue_t *p_queue_node,
                                   sai_object_id_t *p_queue_oid);

/**
 * @brief Remove a Queue in NPU.
 *
 * @param[in] p_queue_node  Pointer to the queue node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_queue_remove_fn) (dn_sai_qos_queue_t *p_queue_node);

/**
 * @brief Attach queue to a parent scheduler group in NPU
 *
 * @param[in] queue_id      Object Identifier of the child queue
 * @param[in] parent_id     Object Identifier of the parent scheduler group
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_queue_attach_to_parent_fn) (
                                             sai_object_id_t queue_id,
                                             sai_object_id_t parent_id);

/**
 * @brief  Detach queue from a parent scheduler group in NPU
 *
 * @param[in] queue_id      Object Identifier of the child queue
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_queue_detach_from_parent_fn) (
                                             sai_object_id_t queue_id);

/**
 * @brief  Modify parent scheduler group of a queue in NPU
 *
 * @param[in] queue_id       Object Identifier of the child queue
 * @param[in] new_parent_id  Object Identifier of the new parent scheduler group
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_queue_modify_parent_fn) (
                                             sai_object_id_t queue_id,
                                             sai_object_id_t new_parent_id);

/**
 * @brief Set an attribute to the queue in NPU.
 *
 * @param[in] p_queue_node Pointer to the queue node
 * @param[in] attr_count Attribute count
 * @param[in] p_attr_list attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_queue_attribute_set_fn) (
                                       dn_sai_qos_queue_t *p_queue_node,
                                       uint_t attr_count,
                                       const sai_attribute_t *p_attr_list);

/**
 * @brief Get attributes from the queue in NPU.
 *
 * @param[in] p_queue_node Pointer to the queue node
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_queue_attribute_get_fn) (
                                                     dn_sai_qos_queue_t *p_queue_node,
                                                     uint_t attr_count,
                                                     sai_attribute_t *attr_list);

/**
 * @brief Get specified counters from the queue in NPU.
 *
 * @param[in] p_queue_node Pointer to the queue node
 * @param[in] counter_ids ID of type sai_queue_stat_t to be retrieved
 * @param[in] number_of_counters Number of counters to be retrieved
 * @param[out] counters Value of retrieved counters
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_queue_stats_get_fn) (
                                                     dn_sai_qos_queue_t *p_queue_node,
                                                     const sai_queue_stat_t *counter_ids,
                                                     uint_t number_of_counters, uint64_t* counters);

/**
 * @brief Clear specified counters from the queue in NPU.
 *
 * @param[in] p_queue_node Pointer to the queue node
 * @param[in] counter_ids ID of type sai_queue_stat_t to be cleared
 * @param[in] number_of_counters Number of counters to be cleared
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_queue_stats_clear_fn) (
                                                     dn_sai_qos_queue_t *p_queue_node,
                                                     const sai_queue_stat_t *counter_ids,
                                                     uint_t number_of_counters);

/**
 * @brief Create a Scheduler group in NPU.
 *
 * @param[in] p_sg_node Pointer to the scheduler group node
 * @param[out] p_sg_oid Pointer to scheduler group OID
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_sched_group_create_fn) (dn_sai_qos_sched_group_t *p_sg_node,
                                   sai_object_id_t *p_sg_oid);

/**
 * @brief Remove a Scheduler group in NPU.
 *
 * @param[in] p_sg_node  Pointer to the scheduler group node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_sched_group_remove_fn) (dn_sai_qos_sched_group_t *p_sg_node);

/**
 * @brief Attach to a parent scheduler group in NPU
 *
 * @param[in] sg_id      Object Identifier of the child scheduler group
 * @param[in] parent_id  Object Identifier of the parent scheduler group
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_sched_group_attach_to_parent_fn) (
                                             sai_object_id_t sg_id,
                                             sai_object_id_t parent_id);

/**
 * @brief  Attach to a parent scheduler group in NPU
 *
 * @param[in] sg_id      Object Identifier of the child scheduler group
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_sched_group_detach_from_parent_fn) (
                                             sai_object_id_t sg_id);

/**
 * @brief  Modify parent scheduler group in NPU
 *
 * @param[in] sg_id          Object Identifier of the child scheduler group
 * @param[in] new_parent_id  Object Identifier of the new parent scheduler group
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_sched_group_modify_parent_fn) (
                                             sai_object_id_t sg_id,
                                             sai_object_id_t new_parent_id);

/**
 * @brief Sets an attribute to the Scheduler group in NPU.
 *
 * @param[in] p_sg_node Pointer to the scheduler group node
 * @param[in] attr_count Attribute count
 * @param[in] p_attr_list attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_sched_group_attribute_set_fn) (
                                       dn_sai_qos_sched_group_t *p_sg_node,
                                       uint_t attr_count,
                                       const sai_attribute_t *p_attr_list);

/**
 * @brief Get attributes from the Scheduler group in NPU.
 *
 * @param[in] p_sg_node Pointer to the scheduler group node
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_sched_group_attribute_get_fn) (
                                                     dn_sai_qos_sched_group_t *p_sg_node,
                                                     uint_t attr_count,
                                                     sai_attribute_t *attr_list);

/**
 *  @brief Get Vendor attribute properties table
 *
 * @param[out] vendor_attr Vendor attribute table
 * @param[out] max_count Maximum attribute count
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef void (*sai_npu_attribute_table_get_fn) (
                                            const dn_sai_attribute_entry_t **p_vendor_attr,
                                            uint_t *p_max_count);


/**
 * @brief Create a Policer in NPU.
 *
 * @param[in] p_policer Pointer to the policer node
 * @param[out] p_pol_id Pointer to the hardware returned policer_id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise
 * a different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_policer_create_fn)(dn_sai_qos_policer_t *p_policer,
                                                      sai_object_id_t  *p_pol_id);

/**
 * @brief Remove a Policer from NPU.
 *
 * @param[in] policer_id policer_id object
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise
 * a different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_policer_remove_fn)(sai_object_id_t policer_id);

/**
 * @brief Set attributes to a policer
 *
 * @param[in] p_policer_node Pointer to the policer node
 * @param[in] attr  Id of the attribute to be set
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise
 * a different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_policer_attribute_set)(dn_sai_qos_policer_t *p_policer_node,
                                                          const sai_attribute_t *pattr);
/**
 * @brief Get attributes for policer
 *
 * @param[in] p_policer_node pointer to the policer node
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise
 * a different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_policer_attribute_get)(dn_sai_qos_policer_t *p_policer_node,
                                                          uint_t attr_count,
                                                          sai_attribute_t *p_attr_list);


/**
 * @brief Function to get storm control policer is supported in hw or not.
 *
 * @return true or false based on storm control policer support in hardware
 */
typedef bool (*sai_npu_policer_is_storm_control_hw)(void);

/**
 * @brief Function to get policer acl reinstallation is needed or not.
 *
 * @return true or false based on whether acl entry gets modified on policer modification.
 */
typedef bool (*sai_npu_is_policer_acl_reinstall_needed) (void);

/**
 * @brief Function to get whether port policer is supported or not
 *
 * @return true or false based on whether port level policer is supported
 */
typedef bool (*sai_npu_is_port_policer_supported) (void);

/**
 * @brief Set the storm control policer on the port.
 *
 * @param[in] port_id Port id on which storm control is to be set
 * @param[in] p_policer Policer node data structure
 * @param[in] type Type of storm control policer.
 * @param[in] is_add Add or remove the sotrm control policer from port
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise appropriate
 * error code is returned.
 */
typedef sai_status_t (*sai_npu_policer_port_set)(sai_object_id_t port_id,
                                                 dn_sai_qos_policer_t *p_policer,
                                                 uint_t type, bool is_add);

/**
 * @brief Create a QoS Map in NPU.
 *
 * @param[in] p_map Pointer to the map node
 * @param[out] p_map_id Pointer to the map_id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise
 * a different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_map_create_fn)(dn_sai_qos_map_t *p_map,
                                sai_npu_object_id_t *p_map_id);



/**
 * @brief Remove a QoS Map in NPU.
 *
 * @param[in] p_map Pointer to the map node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a
 * different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_map_remove_fn)(dn_sai_qos_map_t *p_map);


/**
 * @brief Set attributes to a map
 *
 * @param[in] p_map Pointer to the map node
 * @param[in] attr_flags Bit Mask of the attribute(s) that are to be
 * set
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise
 * a different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_map_attribute_set_fn)(dn_sai_qos_map_t *p_map,
                                             uint_t attr_flags);


/**
 * @brief Get attributes for a map
 *
 * @param[in] p_map Pointer to the map node
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise
 * a different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_map_attribute_get_fn)(dn_sai_qos_map_t *p_map,
                                                      uint_t attr_count,
                                                      sai_attribute_t *p_attr_list);



/**
 * @brief Set the map id to a port
 *
 * @param[in] port_id port object id on which map is applied
 * @param[in] map_id map object id to be applied to port
 * @param[in] map_type Type of the map to be applied on port.
 * @param[in] map_set indicates set or remove of map
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise
 * a different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_port_map_set_fn)(sai_object_id_t port_id,
                                      sai_object_id_t map_id,
                                      sai_qos_map_type_t map_type,
                                      bool map_set);

/**
 * @brief Function to determine whether the map is hwobject or not
 *
 * @param[in] map_type type of the qos map
 * @return true if hwobject false otherwise
 */
typedef bool (*sai_npu_qos_map_is_hw_object_fn)(sai_qos_map_type_t map_type);

/**
 * @brief Function to determine whether the map is supported by NPU or not
 *
 * @param[in] map_type type of the qos map
 * @return true if maptype is supported in NPU,false otherwise
 */
typedef bool (*sai_npu_qos_is_map_type_supported_fn)(sai_qos_map_type_t map_type);

/**
 * @brief Create a Scheduler in NPU.
 *
 * @param[in] p_sched_node Pointer to the scheduler node
 * @param[out] p_sched_oid Pointer to Scheduler OID
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_scheduler_create_fn) (dn_sai_qos_scheduler_t *p_sched_node,
                                                     sai_object_id_t *p_sched_oid);

/**
 * @brief Remove a Scheduler in NPU.
 *
 * @param[in] p_sched_node  Pointer to the scheduler node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_scheduler_remove_fn) (dn_sai_qos_scheduler_t *p_sched_node);

/**
 * @brief Set an attribute to the Scheduler in NPU.
 *
 * @param[in] p_sched_node Pointer to the scheduler node
 * @param[in] attr_count Attribute count
 * @param[in] p_attr_list attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_scheduler_attribute_set_fn) (
                                       dn_sai_qos_scheduler_t *p_sched_node,
                                       uint_t attr_count,
                                       const sai_attribute_t *p_attr_list);

/**
 * @brief Get attributes from the Scheduler in NPU.
 *
 * @param[in] p_sched_node Pointer to the scheduler node
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_scheduler_attribute_get_fn) (
                                                     dn_sai_qos_scheduler_t *p_sched_node,
                                                     uint_t attr_count,
                                                     sai_attribute_t *attr_list);
/**
 * @brief Function to determine whether the scheudler is hwobject or not
 *
 * @return true if hwobject false otherwise
 */
typedef bool (*sai_npu_scheduler_is_hw_object_fn)(void);

/**
 * @brief Apply/Reapply Scheduler on Queue/Port/Scheduler group in NPU.
 *
 * @param[in] id  UOID of port/scheduler/scheduler group
 * @param[in] p_old_sched_node  Pointer to the scheduler node with
 *            old attribute values
 * @param[in] p_new_sched_node  Pointer to the scheduler node with updated
 *            attribute values
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_scheduler_set_fn) (sai_object_id_t id,
                                                  dn_sai_qos_scheduler_t *p_old_sched_node,
                                                  dn_sai_qos_scheduler_t *p_new_sched_node);

/**
 * @brief Create a QoS Wred in NPU.
 *
 * @param[in] p_wred Pointer to the wred node
 * @param[in] attr_count Number of attributes
 * @param[in] p_attr Pointer to the attribute list
 * @param[out] p_wred_id Pointer to the wred_id created
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise
 * a different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_wred_create_fn) (dn_sai_qos_wred_t *p_wred,
                                                    uint_t attr_count,
                                                    const sai_attribute_t *p_attr,
                                                    sai_npu_object_id_t *p_wred_id);
/**
 * @brief Remove a QoS Wred in NPU.
 *
 * @param[in] p_wred Pointer to the wred node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a
 * different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_wred_remove_fn)(dn_sai_qos_wred_t *p_wred);


/**
 * @brief Set attributes to a wred
 *
 * @param[in] p_wred Pointer to the wred node
 * @param[in] attr_count Number of attributes
 * @param[in] p_attr pointer to the attribute(s)
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise
 * a different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_wred_attribute_set_fn)(dn_sai_qos_wred_t *p_wred,
                                             uint_t attr_count, const sai_attribute_t *p_attr);


/**
 * @brief Get attributes for a wred
 *
 * @param[in] p_wred Pointer to the wred node
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise
 * a different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_wred_attribute_get_fn)(dn_sai_qos_wred_t *p_wred,
                                                      uint_t attr_count,
                                                      sai_attribute_t *p_attr_list);



/**
 * @brief Apply WRED profile based on the link type
 *
 * @param[in] wred_link_id WRED link object ID to which WRED is applied
 * @param[in] p_wred_node Pointer to WRED node
 * @param[in] wred_link_type Level at which WRED is applied
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise
 * a different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_wred_link_set_fn)(
        sai_object_id_t wred_link_id,
        dn_sai_qos_wred_t *p_wred,
        dn_sai_qos_wred_link_t wred_link_type);

/**
 * @brief Remove applied WRED profile based on the link type
 *
 * @param[in] wred_link_id WRED link object ID to which WRED is applied
 * @param[in] wred_link_type Level at which WRED is applied
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise
 * a different error code is returned.
 */
typedef sai_status_t (*sai_npu_qos_wred_link_reset_fn)(
        sai_object_id_t wred_link_id,
        dn_sai_qos_wred_link_t wred_link_type);

/**
 * @brief Function to determine whether the wred is hwobject or not
 *
 * @return true if hwobject false otherwise
 */
typedef bool (*sai_npu_qos_wred_is_hw_object_fn)(void);

/**
 * @brief Function to return maximum buffer size in NPU.
 *
 * @return Maximum buffer size.
 */
typedef uint_t (*sai_npu_qos_wred_max_buf_size_get_fn)(void);

/**
 * @brief Function to set the switch level default traffic class.
 *
 * @param[in] default_tc value of the default traffic class to updated.
 * @return SAI_STATUS_SUCCESS on success or appropriate error
 * code on failure.
 */
typedef sai_status_t (*sai_npu_qos_switch_default_tc_fn) (uint_t default_tc);

/**
 *  @brief Get Vendor attribute properties table
 *
 * @param[out] p_vendor Vendor attribute table
 * @param[out] p_max_attr_count Maximum attribute count
 */
typedef void (*sai_npu_buffer_pool_attr_table_get_fn) (const dn_sai_attribute_entry_t
                                                       **p_vendor,
                                                       uint_t *p_max_attr_count);

/**
 *  @brief Get Vendor attribute properties table
 *
 * @param[out] p_vendor Vendor attribute table
 * @param[out] p_max_attr_count Maximum attribute count
 */
typedef void (*sai_npu_buffer_profile_attr_table_get_fn) (const dn_sai_attribute_entry_t
                                                          **p_vendor,
                                                          uint_t *p_max_attr_count);

/**
 *  @brief Init NPU buffer parameters
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_buffer_init_fn) (void);

/**
 *  @brief Create Buffer pool
 *
 * @param[in] p_buf_pool_node Buffer pool node
 * @param[out] p_buf_pool_oid Buffer pool Object ID created
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_buffer_pool_create_fn) (dn_sai_qos_buffer_pool_t
                                                       *p_buf_pool_node,
                                                       sai_object_id_t *p_buf_pool_oid);

/**
 *  @brief Remove Buffer pool
 *
 * @param[in] p_buf_pool_node Buffer pool node
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_buffer_pool_remove_fn) (dn_sai_qos_buffer_pool_t
                                                       *p_buf_pool_node);

/**
 *  @brief Set buffer pool attribute
 *
 * @param[in] p_buf_pool_node Buffer pool node
 * @param[in] attr attribute to be set
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_buffer_pool_attr_set_fn) (dn_sai_qos_buffer_pool_t
                                                         *p_buf_pool_node,
                                                         const sai_attribute_t *attr);

/**
 *  @brief Get buffer pool attributes
 *
 * @param[in] p_buf_pool_node Buffer pool node
 * @param[in] attr_count attribute count
 * @param[out] attr_list attribute list to get
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_buffer_pool_attr_get_fn) (const dn_sai_qos_buffer_pool_t *p_buf_pool_node,
                                                         uint32_t attr_count, sai_attribute_t *attr_list);
/**
 *  @brief Get buffer pool stats
 *
 * @param[in] pool_id Buffer Pool ID
 * @param[in] counter_ids List of counter IDs to obtain
 * @param[in] number_of_counters Number of counters to obtain
 * @param[out] counters List of counter obtained
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t(*sai_npu_buffer_pool_stats_get_fn) (sai_object_id_t pool_id,
                                                         const sai_buffer_pool_stat_t
                                                         *counter_ids,
                                                         uint32_t number_of_counters,
                                                         uint64_t* counters);

/**
 *  @brief Create Buffer profile
 *
 * @param[in] p_bufffer_profile_node Buffer profile node
 * @param[out] bufffer_profile_id Buffer profile Object ID created
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_buffer_profile_create_fn) (dn_sai_qos_buffer_profile_t
                                                          *p_buffer_profile_node,
                                                          sai_object_id_t *buffer_profile_id);


/**
 *  @brief Remove Buffer profile
 *
 * @param[in] p_buffer_profile_node Buffer profile node
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_buffer_profile_remove_fn) (dn_sai_qos_buffer_profile_t
                                                          *p_buffer_profile_node);

/**
 *  @brief Set buffer profile attribute
 *
 * @param[in] p_buf_profile_node Buffer profile node
 * @param[in] old_attr attribute to be set
 * @param[in] attr attribute to be set
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_buffer_profile_attr_set_fn) (sai_object_id_t obj_id,
                                                            dn_sai_qos_buffer_profile_t
                                                            *p_buf_profile_node,
                                                            const sai_attribute_t *old_attr,
                                                            const sai_attribute_t *attr);

/**
 *  @brief Apply buffer profile on an object
 *
 * @param[in] obj_id Object ID on which buffer profile to be applied
 * @param[in] p_old_buf_profile_node Buffer profile node
 * @param[in] p_buf_profile_node Buffer profile node
 * @param[in] is_retry is retry
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_apply_buffer_profile_fn) (sai_object_id_t obj_id,
                      dn_sai_qos_buffer_profile_t *p_old_buf_profile_node,
                      dn_sai_qos_buffer_profile_t *p_buf_profile_node, bool is_retry);

/**
 *  @brief Apply buffer profile on an object
 *
 * @param[in] buf_pool_id Buffer pool ID
 * @param[in] p_buf_profile_node Buffer profile node
 * @param[in] add_size Increase is reserved buffer size due to buffer profile modification
 *
 * @return SAI_STATUS_SUCCESS if operation is successful and buffer is available otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_check_buffer_size_fn) (sai_object_id_t buf_pool_id,
                      const dn_sai_qos_buffer_profile_t *p_buf_profile_node,
                      uint_t add_size);

/**
 *  @brief Create ingress priority group
 *
 * @param[in] p_pg_node Priority group to be created
 * @param[in] index Index of Priority Group in port
 * @param[out] pg_id Priority Group ID created
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_pg_create_fn) (dn_sai_qos_pg_t *p_pg_node, uint_t index,
                                              sai_object_id_t *pg_id);

/**
 *  @brief Destroy ingress priority group
 *
 * @param[in] p_pg_node Priority group to be destroyed
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_pg_destroy_fn) (dn_sai_qos_pg_t *p_pg_node);

 /**
 *  @brief Get ingress priority group attributes
 *
 * @param[in] p_pg_node Priority group node
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_pg_attr_get_fn) (dn_sai_qos_pg_t *p_pg_node,
                                                uint32_t attr_count,
                                                sai_attribute_t *attr_list);
/**
 *  @brief Get Ingress priority group stats
 *
 * @param[in] pg_id Priority group ID
 * @param[in] counter_ids List of counter IDs to obtain
 * @param[in] number_of_counters Number of counters to obtain
 * @param[out] counters List of counter obtained
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t(*sai_npu_pg_stats_get_fn) (sai_object_id_t pg_id,
                                                const sai_ingress_priority_group_stat_t
                                                *counter_ids,  uint32_t number_of_counters,
                                                 uint64_t* counters);


/**
 *  @brief Clear Ingress priority group stats
 *
 * @param[in] pg_id Priority group ID
 * @param[in] counter_ids List of counter IDs to obtain
 * @param[in] number_of_counters Number of counters to obtain
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t(*sai_npu_pg_stats_clear_fn) (sai_object_id_t pg_id,
                                                const sai_ingress_priority_group_stat_t
                                                *counter_ids,  uint32_t number_of_counters);

/**
 * @brief Qos NPU common API table.
 */
typedef struct _sai_npu_qos_api_t {

    sai_npu_qos_global_init_fn            qos_global_init;
    sai_npu_qos_port_init_fn              qos_port_init;
    sai_npu_qos_switch_default_tc_fn      qos_switch_default_tc;
} sai_npu_qos_api_t;


/**
 * @brief Qos NPU Scheduler group API table.
 */
typedef struct _sai_npu_sched_group_api_t {

    sai_npu_sched_group_create_fn              sched_group_create;
    sai_npu_sched_group_remove_fn              sched_group_remove;
    sai_npu_sched_group_attribute_set_fn       sched_group_attribute_set;
    sai_npu_sched_group_attribute_get_fn       sched_group_attribute_get;
    sai_npu_sched_group_attach_to_parent_fn    sched_group_attach_to_parent;
    sai_npu_sched_group_detach_from_parent_fn  sched_group_detach_from_parent;
    sai_npu_sched_group_modify_parent_fn       sched_group_modify_parent;
    sai_npu_attribute_table_get_fn             attribute_table_get;
} sai_npu_sched_group_api_t;

/**
 * @brief Qos NPU Queue API table.
 */
typedef struct _sai_npu_queue_api_t {

    sai_npu_queue_create_fn              queue_create;
    sai_npu_queue_remove_fn              queue_remove;
    sai_npu_queue_attach_to_parent_fn    queue_attach_to_parent;
    sai_npu_queue_detach_from_parent_fn  queue_detach_from_parent;
    sai_npu_queue_modify_parent_fn       queue_modify_parent;
    sai_npu_queue_attribute_set_fn       queue_attribute_set;
    sai_npu_queue_attribute_get_fn       queue_attribute_get;
    sai_npu_attribute_table_get_fn       attribute_table_get;
    sai_npu_queue_stats_get_fn           queue_stats_get;
    sai_npu_queue_stats_clear_fn         queue_stats_clear;

} sai_npu_queue_api_t;

/**
 * @brief Qos NPU Maps API table.
 */
typedef struct _sai_npu_qos_map_api_t {

    sai_npu_qos_map_create_fn              map_create;
    sai_npu_qos_map_remove_fn              map_remove;
    sai_npu_qos_map_attribute_set_fn       map_attr_set;
    sai_npu_qos_map_attribute_get_fn       map_attr_get;
    sai_npu_qos_port_map_set_fn            port_map_set;
    sai_npu_qos_map_is_hw_object_fn        map_is_hw_object;
    sai_npu_qos_is_map_type_supported_fn   is_map_supported;
    sai_npu_attribute_table_get_fn         attribute_table_get;

} sai_npu_qos_map_api_t;

/**
 * @brief Qos NPU POlicer API table.
 */
typedef struct _sai_npu_qos_policer_api_t {

    sai_npu_qos_policer_create_fn           policer_create;
    sai_npu_qos_policer_remove_fn           policer_remove;
    sai_npu_qos_policer_attribute_set       policer_set;
    sai_npu_qos_policer_attribute_get       policer_get;
    sai_npu_attribute_table_get_fn          attribute_table_get;
    sai_npu_policer_is_storm_control_hw     is_storm_control_hw_port_list_supported;
    sai_npu_is_policer_acl_reinstall_needed is_acl_reinstall_needed;
    sai_npu_policer_port_set                policer_port_set;
    sai_npu_is_port_policer_supported       port_policer_supported;
} sai_npu_policer_api_t;

/**
 *  @brief Qos NPU Scheduler API table.
 */
typedef struct _sai_npu_scheudler_api_t {

    sai_npu_scheduler_create_fn           scheduler_create;
    sai_npu_scheduler_remove_fn           scheduler_remove;
    sai_npu_scheduler_attribute_set_fn    scheduler_attribute_set;
    sai_npu_scheduler_attribute_get_fn    scheduler_attribute_get;
    sai_npu_scheduler_is_hw_object_fn     scheduler_is_hw_object;
    sai_npu_attribute_table_get_fn        attribute_table_get;
    sai_npu_scheduler_set_fn              scheduler_set;
} sai_npu_scheduler_api_t;

/**
 * @brief Qos NPU Wred  API table.
 */
typedef struct _sai_npu_wred_api_t {

    sai_npu_qos_wred_create_fn             wred_create;
    sai_npu_qos_wred_remove_fn             wred_remove;
    sai_npu_qos_wred_attribute_set_fn      wred_attr_set;
    sai_npu_qos_wred_attribute_get_fn      wred_attr_get;
    sai_npu_qos_wred_link_set_fn           wred_link_set;
    sai_npu_qos_wred_link_reset_fn         wred_link_reset;
    sai_npu_qos_wred_is_hw_object_fn       wred_is_hw_object;
    sai_npu_qos_wred_max_buf_size_get_fn   wred_max_buf_size_get;
    sai_npu_attribute_table_get_fn         attribute_table_get;

} sai_npu_wred_api_t;

/**
 * @brief Qos NPU Buffer  API table.
 */
typedef struct _sai_npu_buffer_api_t {

    sai_npu_buffer_init_fn                   buffer_init;
    sai_npu_buffer_pool_create_fn            buffer_pool_create;
    sai_npu_buffer_pool_remove_fn            buffer_pool_remove;
    sai_npu_buffer_pool_attr_set_fn          buffer_pool_attr_set;
    sai_npu_buffer_pool_attr_get_fn          buffer_pool_attr_get;
    sai_npu_buffer_pool_stats_get_fn         buffer_pool_stats_get;
    sai_npu_buffer_profile_create_fn         buffer_profile_create;
    sai_npu_buffer_profile_remove_fn         buffer_profile_remove;
    sai_npu_buffer_profile_attr_set_fn       buffer_profile_attr_set;
    sai_npu_apply_buffer_profile_fn          buffer_profile_apply;
    sai_npu_pg_create_fn                     pg_create;
    sai_npu_pg_destroy_fn                    pg_destroy;
    sai_npu_pg_attr_get_fn                   pg_attr_get;
    sai_npu_pg_stats_get_fn                  pg_stats_get;
    sai_npu_pg_stats_clear_fn                pg_stats_clear;
    sai_npu_buffer_pool_attr_table_get_fn    buffer_pool_attr_table_get;
    sai_npu_buffer_profile_attr_table_get_fn buffer_profile_attr_table_get;
    sai_npu_check_buffer_size_fn             check_buffer_size;
} sai_npu_buffer_api_t;

/**
 * \}
 */
#endif /* __SAI_NPU_QOS_H__ */
