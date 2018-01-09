/************************************************************************
* LEGALESE:   "Copyright (c) 2016, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file sai_hash_object.h
*
* @brief This file contains the datastructure definitions for SAI Hash object.
*
*************************************************************************/
#ifndef __SAI_HASH_OBJECT_H__
#define __SAI_HASH_OBJECT_H__

#include "std_type_defs.h"
#include "sai_event_log.h"
#include "event_log_types.h"
#include "std_rbtree.h"

#include "sai.h"
#include "saitypes.h"
#include "saihash.h"

/** @defgroup DNSAIHASH DS - Hash Data structures internally used by SAI
*    implementation.
*
* \{
*/

/**
 * @brief SAI Hash object data structure.
 *
 */
typedef struct _dn_sai_hash_object_t {
    /** Hash object Id */
    sai_object_id_t          obj_id;

    /** List of Hash native fields set for this object. */
    sai_s32_list_t           native_fields_list;

    /** List of Hash HASH fields set for this object. */
    sai_object_list_t        udf_group_list;

    /** Ref count for this hash object. */
    uint_t                   ref_count;

    /** Place holder for NPU-specific data */
    void                    *npu_info;
} dn_sai_hash_object_t;

typedef enum _dn_sai_hash_object_type {

    DN_SAI_ECMP_NON_IP_HASH = 0,
    DN_SAI_LAG_NON_IP_HASH = 1,
    DN_SAI_ECMP_IPV4_HASH = 2,
    DN_SAI_ECMP_IPV4_IN_IPV4_HASH = 3,
    DN_SAI_ECMP_IPV6_HASH = 4,
    DN_SAI_LAG_IPV4_HASH = 5,
    DN_SAI_LAG_IPV4_IN_IPV4_HASH = 6,
    DN_SAI_LAG_IPV6_HASH = 7,
    DN_SAI_MAX_HASH_OBJ_TYPES

} dn_sai_hash_object_type;

/**
 * @brief SAI Hash object table for global parameters
 */
typedef struct _dn_sai_hash_obj_db_t {

    /** SW Db of Hash objects. Nodes of type dn_sai_hash_object_t */
    rbtree_handle           hash_obj_tree;

    /** Traffic type based view of hash objects */
    dn_sai_hash_object_t   *hash_obj_types [DN_SAI_MAX_HASH_OBJ_TYPES];

    /** Bitmap for Hash object index usage */
    uint8_t                *hash_index_bitmap;

    /** Flag to indicate the initialization complete */
    bool                    is_init_complete;
} dn_sai_hash_obj_db_t;


/** Logging utility for SAI HASH API */
#define SAI_HASH_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_HASH, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_HASH, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)


/** Per log level based macros for SAI HASH API */
#define SAI_HASH_LOG_TRACE(msg, ...) \
        SAI_HASH_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_HASH_LOG_CRIT(msg, ...) \
        SAI_HASH_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_HASH_LOG_ERR(msg, ...) \
        SAI_HASH_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_HASH_LOG_INFO(msg, ...) \
        SAI_HASH_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_HASH_LOG_WARN(msg, ...) \
        SAI_HASH_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_HASH_LOG_NTC(msg, ...) \
        SAI_HASH_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)
/**
\}
*/


#endif /*__SAI_HASH_COMMON_H__*/

