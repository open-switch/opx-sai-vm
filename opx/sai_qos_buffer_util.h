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
* @file sai_qos_buffer_util.h
*
* @brief This file contains the utility functions for SAI QOS Buffer component.
*
*************************************************************************/
#ifndef __SAI_QOS_BUFFER_UTIL_H__
#define __SAI_QOS_BUFFER_UTIL_H__

#include "saitypes.h"
#include "sai_qos_util.h"


/** \defgroup SAIQOSBUFFERUTILS  SAI - Qos Buffer Utility functions
 *  Util functions in the SAI Qos buffer component
 *
 *  \{
 */

/**
 * @brief Get PG node for the PG ID.
 * @param[in] pg_id  Ingress priority group ID
 *
 * @return A pointer to node of type dn_sai_qos_pg_t if success. Else NULL
 * pointeris returned.
 */
dn_sai_qos_pg_t *sai_qos_pg_node_get (sai_object_id_t pg_id);

/**
 * @brief Get buffer pool node for the buffer pool ID.
 * @param[in] buffer_pool_id  Buffer pool ID
 *
 * @return A pointer to node of type dn_sai_qos_buffer_pool_t if success. Else NULL
 * pointeris returned.
 */
dn_sai_qos_buffer_pool_t *sai_qos_buffer_pool_node_get (sai_object_id_t
                                                        buffer_pool_id);

/**
 * @brief Get buffer profile node for the buffer profile ID.
 * @param[in] buffer_profile_id  Buffer profile ID
 *
 * @return A pointer to node of type dn_sai_qos_buffer_profile_t if success. Else NULL
 * pointeris returned.
 */
dn_sai_qos_buffer_profile_t *sai_qos_buffer_profile_node_get (sai_object_id_t
                                                              buffer_profile_id);

/**
 * @brief Get first queue node from buffer profile node
 * @param[in] p_qos_buffer_profile_node Buffer profile node
 *
 * @return Pointer to first queue node
 */
dn_sai_qos_queue_t *sai_qos_buffer_profile_get_first_queue (dn_sai_qos_buffer_profile_t
                                                            *p_qos_buffer_profile_node);


/**
 * @brief Get Next queue node from buffer profile node
 * @param[in] p_qos_buffer_profile_node Buffer profile node
 * @param[in] p_queue_node Queue node whose next is to be found
 *
 * @return Pointer to next queue node
 */
dn_sai_qos_queue_t *sai_qos_buffer_profile_get_next_queue (dn_sai_qos_buffer_profile_t
                                                           *p_qos_buffer_profile_node,
                                                           dn_sai_qos_queue_t *p_queue_node);


/**
 * @brief Get first port node from buffer profile node
 * @param[in] p_qos_buffer_profile_node Buffer profile node
 *
 * @return Pointer to first port node
 */
dn_sai_qos_port_t *sai_qos_buffer_profile_get_first_port (dn_sai_qos_buffer_profile_t
                                                          *p_qos_buffer_profile_node);


/**
 * @brief Get Next port node from buffer profile node
 * @param[in] p_qos_buffer_profile_node Buffer profile node
 * @param[in] p_port_node port node whose next is to be found
 *
 * @return Pointer to next port node
 */
dn_sai_qos_port_t *sai_qos_buffer_profile_get_next_port (dn_sai_qos_buffer_profile_t
                                                         *p_qos_buffer_profile_node,
                                                         dn_sai_qos_port_t *p_port_node);


/**
 * @brief Get first pg node from buffer profile node
 * @param[in] p_qos_buffer_profile_node Buffer profile node
 *
 * @return Pointer to first pg node
 */
dn_sai_qos_pg_t *sai_qos_buffer_profile_get_first_pg (dn_sai_qos_buffer_profile_t
                                                      *p_qos_buffer_profile_node);


/**
 * @brief Get Next pg node from buffer profile node
 * @param[in] p_qos_buffer_profile_node Buffer profile node
 * @param[in] p_pg_node pg node whose next is to be found
 *
 * @return Pointer to next pg node
 */
dn_sai_qos_pg_t *sai_qos_buffer_profile_get_next_pg (dn_sai_qos_buffer_profile_t
                                                     *p_qos_buffer_profile_node,
                                                      dn_sai_qos_pg_t *p_pg_node);


/**
 * @brief Get first buffer profile node from buffer pool node
 * @param[in] p_qos_buffer_pool_node Buffer pool node
 *
 * @return Pointer to first buffer profile node
 */
dn_sai_qos_buffer_profile_t *sai_qos_buffer_pool_get_first_buffer_profile (dn_sai_qos_buffer_pool_t
                                                      *p_qos_buffer_pool_node);


/**
 * @brief Get Next buffer profile node from buffer pool node
 * @param[in] p_qos_buffer_pool_node Buffer pool node
 * @param[in] p_buffer_profile_node buffer_profile node whose next is to be found
 *
 * @return Pointer to next buffer profile node
 */
dn_sai_qos_buffer_profile_t *sai_qos_buffer_pool_get_next_buffer_profile (dn_sai_qos_buffer_pool_t
                                                                          *p_qos_buffer_pool_node,
                                                                          dn_sai_qos_buffer_profile_t
                                                                          *p_buffer_profile_node);

/**
 * @brief Get buffer profile id from the object
 * @param[in] obj_id object ID to get buffer profile
 * @param[out] profile_id pointer to Buffer profile object ID
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 * error code is returned.
 */
sai_status_t sai_qos_get_buffer_profile_id (sai_object_id_t obj_id,
                                            sai_object_id_t *profile_id);

/**
 * @brief Get buffer pool id from the object
 * @param[in] obj_id object ID to get buffer pool
 * @param[out] pool_id pointer to Buffer pool object ID
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 * error code is returned.
 */
sai_status_t sai_qos_get_buffer_pool_id (sai_object_id_t obj_id,
                                         sai_object_id_t *pool_id);

/**
 * @brief Get shared threshold mode
 * @param[in] p_profile_node Buffer profile node to be used
 * @param[out] th_mode Threshold mode for the profile
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 * error code is returned.
 */
sai_status_t sai_qos_get_th_mode (dn_sai_qos_buffer_profile_t *p_profile_node,
                                  sai_buffer_profile_threshold_mode_t *th_mode);


/**
 * @brief Get first PG node from a port node
 * @param[in] p_qos_port_node Port node to be used
 *
 * @return Pointer to first PG node if present. NULL otherwise
 */
dn_sai_qos_pg_t *sai_qos_port_get_first_pg (dn_sai_qos_port_t  *p_qos_port_node);

/**
 * @brief Get Next PG node from a port node
 * @param[in] p_qos_port_node Port node to be used
 * @param[in] p_pg_node Current PG node
 *
 * @return Pointer to Next PG node if present. NULL otherwise
 */
dn_sai_qos_pg_t *sai_qos_port_get_next_pg (dn_sai_qos_port_t  *p_qos_port_node,
                                           dn_sai_qos_pg_t *p_pg_node);

/**
 * @brief Get First PG that belong to the pool
 * @param[in] pool_id Buffer pool ID
 * @param[out] pg_id First PG ID that belong to the pool
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 * error code is returned.
 */
sai_status_t sai_qos_buffer_pool_get_first_pg_id (sai_object_id_t pool_id,
                                                  sai_object_id_t *pg_id);

/**
 * @brief Get First Queue that belong to the pool
 * @param[in] pool_id Buffer pool ID
 * @param[out] queue_id First Queue ID that belong to the pool
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 * error code is returned.
 */
sai_status_t sai_qos_buffer_pool_get_first_queue_id (sai_object_id_t pool_id,
                                                     sai_object_id_t *queue_id);

/**
 * @brief Get Reserved XOFF threshold from buffer profile
 *
 *  Buffer profile xoff_th reserved or upper limit dependent on buffer pool XOFF_SIZE.
 *  If the user has set XOFF_SIZE = 0, the PG headroom buffer is equal to XOFF_TH
 *  and it is not shared. If the user has set XOFF_SIZE > 0, the
 *  total headroom pool buffer for all PGs is equal to XOFF_SIZE
 *  and XOFF_TH specifies the maximum amount of headroom pool.
 *  buffer one PG can use.
 *
 * @param[in] p_qos_buffer_profile_node Buffer profile node
 *
 * @return Size of reserved xoff size
 */

uint_t sai_qos_buffer_profile_get_reserved_xoff_th_get (const dn_sai_qos_buffer_profile_t
                                                        *p_qos_buffer_profile_node);

/**
 * @brief Get Buffer pool xoff size configuration status
 *
 * @param[in] pool_id Buffer pool id
 *
 * @return Size of reserved xoff size
 */
bool sai_qos_is_buffer_pool_xoff_size_configured (sai_object_id_t pool_id);

/**
 * @brief Get unicast front end port queues that belong to the pool based on count
 * @param[in] pool_id Buffer pool ID
 * @param[out] p_queue_list Pointer to queue list buffer. Modifies the list count
 *                          based on the number of queues updated in the list
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 * error code is returned.
 */
sai_status_t sai_qos_buffer_pool_get_wred_queue_ids(sai_object_id_t pool_id,
        sai_object_list_t *p_queue_list);

/** Logging utility for SAI Buffer API */
#define SAI_BUFFER_LOG(level, msg, ...) \
        if (sai_is_log_enabled (SAI_API_BUFFER, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_BUFFER, level, msg, ##__VA_ARGS__); \
        }

/**
 * @brief   SAI Qos Initialization specific trace logging function
 */

/** Per log level based macros for SAI Buffer API */
#define SAI_BUFFER_LOG_TRACE(msg, ...) \
        SAI_BUFFER_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_BUFFER_LOG_CRIT(msg, ...) \
        SAI_BUFFER_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_BUFFER_LOG_ERR(msg, ...) \
        SAI_BUFFER_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_BUFFER_LOG_INFO(msg, ...) \
        SAI_BUFFER_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_BUFFER_LOG_WARN(msg, ...) \
        SAI_BUFFER_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_BUFFER_LOG_NTC(msg, ...) \
        SAI_BUFFER_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/**
 * \}
 */

#endif /* __SAI_QOS_BUFFER_UTIL_H__ */
