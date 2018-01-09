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
* @file sai_udf_common.h
*
* @brief This file contains the datastructure definitions for SAI
* UDF objects.
*
*************************************************************************/
#ifndef __SAI_UDF_COMMON_H__
#define __SAI_UDF_COMMON_H__

#include "sai.h"
#include "saitypes.h"
#include "saiudf.h"

#include "sai_event_log.h"
#include "event_log_types.h"
#include "std_rbtree.h"
#include "std_llist.h"

/** @defgroup SAIUDF DS - SAI UDF Data structures to be internally used by SAI
*
* \{
*/

/**
 * @brief SAI UDF Group key field data structure.
 *
 */
typedef struct _dn_sai_udf_group_key_t {
    /** UDF Group object id */
    sai_object_id_t          group_obj_id;
} dn_sai_udf_group_key_t;

/**
 * @brief SAI UDF Group data structure.
 *
 */
typedef struct _dn_sai_udf_group_t {
    /** Key structure for UDF Group tree */
    dn_sai_udf_group_key_t   key;

    /** UDF Group type */
    sai_udf_group_type_t     type;

    /** UDF length for the UDF objects in the group */
    uint_t                   length;

    /** Number of UDF objects in the group */
    uint_t                   udf_count;

    /** UDF list head. Nodes of type dn_sai_udf_t */
    std_dll_head             udf_list;

    /** Place holder for NPU-specific data */
    void                    *npu_info;
} dn_sai_udf_group_t;

/**
 * @brief SAI UDF key field data structure.
 *
 */
typedef struct _dn_sai_udf_key_t {
    /** UDF object id */
    sai_object_id_t        udf_obj_id;
} dn_sai_udf_key_t;

/**
 * @brief SAI UDF data structure.
 *
 */
typedef struct _dn_sai_udf_t {
    /** Link to the next node in UDF Group's UDF list */
    std_dll                node_link;

    /** Key structure for UDF object tree */
    dn_sai_udf_key_t       key;

    /** UDF Match object id for the UDF object */
    sai_object_id_t        match_obj_id;

    /** Pointer to the UDF Group node this UDF object belongs to */
    dn_sai_udf_group_t    *p_udf_group;

    /** Base for the UDF */
    sai_udf_base_t         base;

    /** Offset from the base for the UDF */
    uint_t                 offset;

    /** Hash mask bytes to be used for the UDF Hash field */
    sai_u8_list_t          hash_mask;

    /** Place holder for NPU-specific data */
    void                  *npu_info;
} dn_sai_udf_t;


/** Default UDF Hash mask byte count */
#define SAI_UDF_DFLT_HASH_MASK_COUNT (2)

/** Logging utility for SAI UDF API */
#define SAI_UDF_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_UDF, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_UDF, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)


/** Per log level based macros for SAI UDF API */
#define SAI_UDF_LOG_TRACE(msg, ...) \
        SAI_UDF_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_UDF_LOG_CRIT(msg, ...) \
        SAI_UDF_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_UDF_LOG_ERR(msg, ...) \
        SAI_UDF_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_UDF_LOG_INFO(msg, ...) \
        SAI_UDF_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_UDF_LOG_WARN(msg, ...) \
        SAI_UDF_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_UDF_LOG_NTC(msg, ...) \
        SAI_UDF_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)
/**
\}
*/


#endif /*__SAI_UDF_COMMON_H__*/

