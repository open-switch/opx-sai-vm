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
* @file sai_tunnel_util.h
*
* @brief This file contains the util macros for SAI Tunnel functionality.
*
*************************************************************************/
#ifndef __SAI_TUNNEL_UTIL_H__
#define __SAI_TUNNEL_UTIL_H__

#include "saitypes.h"
#include "sai_event_log.h"
#include "event_log_types.h"
#include "sai_tunnel.h"

/** Maximum DSCP value used for input validation */
#define SAI_TUNNEL_MAX_DSCP_VAL     (64)
/** Maximum TTL value used for input validation */
#define SAI_TUNNEL_MAX_TTL_VAL      (255)

/** Constant for tunnel object software index */
#define SAI_TUNNEL_OBJ_MAX_ID       (65535)
/** Constant for tunnel map object software index */
#define SAI_TUNNEL_MAP_OBJ_MAX_ID   (65535)
/** Constant for tunnel termination object software index */
#define SAI_TUNNEL_TERM_OBJ_MAX_ID  (65535)
/** Constant for tunnel max vnid value */
#define SAI_TUNNEL_VNID_MAX_VALUE   (0xffffff)

/** Logging utility for SAI Tunnel API */
#define SAI_TUNNEL_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_TUNNEL, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_TUNNEL, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)


/** Per log level based macros for SAI Tunnel API */
#define SAI_TUNNEL_LOG_DEBUG(msg, ...) \
        SAI_TUNNEL_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_TUNNEL_LOG_CRIT(msg, ...) \
        SAI_TUNNEL_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_TUNNEL_LOG_ERR(msg, ...) \
        SAI_TUNNEL_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_TUNNEL_LOG_INFO(msg, ...) \
        SAI_TUNNEL_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_TUNNEL_LOG_WARN(msg, ...) \
        SAI_TUNNEL_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_TUNNEL_LOG_NTC(msg, ...) \
        SAI_TUNNEL_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)


void dn_sai_tunnel_lock (void);
void dn_sai_tunnel_unlock (void);

dn_sai_tunnel_global_t *dn_sai_tunnel_access_global_config (void);

dn_sai_tunnel_t *dn_sai_tunnel_obj_get (sai_object_id_t tunnel_id);

void *dn_sai_tunnel_hw_info_get (sai_object_id_t tunnel_id);

sai_status_t sai_tunnel_object_id_validate (sai_object_id_t tunnel_id);

sai_object_id_t dn_sai_tunnel_underlay_vr_get (sai_object_id_t tunnel_id);

sai_object_id_t dn_sai_tunnel_overlay_vr_get (sai_object_id_t tunnel_id);

dn_sai_tunnel_term_entry_t *dn_sai_tunnel_term_entry_get (
                                                sai_object_id_t tunnel_term_id);

dn_sai_tunnel_map_t *dn_sai_tunnel_map_get (sai_object_id_t tunnel_map_id);

dn_sai_tunnel_map_entry_t *dn_sai_tunnel_map_entry_get (sai_object_id_t tunnel_map_entry_id);

dn_sai_tunnel_map_entry_t *dn_sai_tunnel_find_tunnel_entry_by_bridge(
                                                     dn_sai_tunnel_map_t *p_tunnel_map,
                                                     sai_object_id_t bridge_oid);

dn_sai_tunnel_map_entry_t *dn_sai_tunnel_find_tunnel_entry_by_vnid(
                                                     dn_sai_tunnel_map_t *p_tunnel_map,
                                                     sai_uint32_t vnid);

sai_status_t dn_sai_tunnel_find_bridge_to_vnid_mapping(sai_object_id_t tunnel_oid,
                                                       sai_object_id_t bridge_oid,
                                                       sai_uint32_t *vnid);

sai_status_t dn_sai_tunnel_find_vnid_to_bridge_mapping(sai_object_id_t tunnel_oid,
                                                       sai_uint32_t vnid,
                                                       sai_object_id_t *bridge_oid);

sai_status_t dn_sai_tunnel_decap_tunnel_map_bridge_count_get(sai_object_id_t tunnel_map_id,
                                                             sai_object_id_t bridge_id,
                                                             uint_t *bridge_count);

bool dn_tunnel_map_is_encap_map_type(sai_tunnel_map_type_t type);

sai_status_t dn_sai_tunnel_map_dep_tunnel_add (sai_object_id_t tunnel_map_id,
                                               sai_object_id_t tunnel_id);

sai_status_t dn_sai_tunnel_map_dep_tunnel_remove(sai_object_id_t tunnel_map_id,
                                                 sai_object_id_t tunnel_id);

sai_status_t dn_sai_tunnel_map_dep_tunnel_get (sai_object_id_t  tunnel_map_id,
                                               uint_t *count,
                                               sai_object_id_t *tunnel_id_list);

sai_status_t dn_sai_tunnel_map_dep_tunnel_count_get(sai_object_id_t  tunnel_map_id,
                                                    uint_t *p_out_count);

sai_status_t dn_sai_tunnel_map_dep_tunnel_get_at_index (sai_object_id_t  tunnel_map_id,
                                                        uint_t           index,
                                                        sai_object_id_t *tunnel_id);

static inline bool dn_sai_is_ip_tunnel (dn_sai_tunnel_t *p_tunnel_obj)
{
    return ((p_tunnel_obj->tunnel_type == SAI_TUNNEL_TYPE_IPINIP) ||
            (p_tunnel_obj->tunnel_type == SAI_TUNNEL_TYPE_IPINIP_GRE) ? true : false);
}

static inline bool dn_sai_is_vxlan_tunnel (dn_sai_tunnel_t *p_tunnel_obj)
{
    return ((p_tunnel_obj->tunnel_type == SAI_TUNNEL_TYPE_VXLAN) ? true : false);
}

static inline bool dn_sai_is_vxlan_tunnel_map_entry (dn_sai_tunnel_map_entry_t *p_tunnel_map_entry)
{
    if((p_tunnel_map_entry->type == SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF) ||
       (p_tunnel_map_entry->type == SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI)) {
        return true;
    }
    return false;
}

static inline rbtree_handle dn_sai_tunnel_tree_handle (void)
{
    return (dn_sai_tunnel_access_global_config()->tunnel_db);
}

static inline rbtree_handle dn_sai_tunnel_term_tree_handle (void)
{
    return (dn_sai_tunnel_access_global_config()->tunnel_term_table_db);
}

static inline rbtree_handle dn_sai_tunnel_map_tree_handle (void)
{
    return (dn_sai_tunnel_access_global_config()->tunnel_map_db);
}

static inline rbtree_handle dn_sai_tunnel_map_entry_tree_handle (void)
{
    return (dn_sai_tunnel_access_global_config()->tunnel_map_entry_db);
}

static inline dn_sai_tunnel_map_entry_t *sai_tunnel_get_first_tunnel_map_entry(
                                                                std_dll_head *p_dll_head)
{
    uint8_t *p_temp = NULL;

    if((p_temp = (uint8_t *)std_dll_getfirst(p_dll_head))) {

        return ((dn_sai_tunnel_map_entry_t *) (p_temp - SAI_TUNNEL_MAP_ENTRY_DLL_GLUE_OFFSET));
    }

    return NULL;
}

static inline dn_sai_tunnel_map_entry_t *sai_tunnel_get_next_tunnel_map_entry(
                                                   std_dll_head *p_dll_head,
                                                   dn_sai_tunnel_map_entry_t *p_tunnel_map_entry)
{
    uint8_t *p_temp = NULL;

    if((p_temp = (uint8_t *)std_dll_getnext(p_dll_head, &p_tunnel_map_entry->tunnel_map_link))) {

        return ((dn_sai_tunnel_map_entry_t *) (p_temp - SAI_TUNNEL_MAP_ENTRY_DLL_GLUE_OFFSET));
    }

    return NULL;
}

dn_sai_tunnel_map_t *dn_sai_tunnel_find_tunnel_map_by_type(
                                          dn_sai_tunnel_t *p_tunnel,
                                          sai_tunnel_map_type_t type);
/**
 * @brief Increment tunnel reference count
 * @param[in] tunnel_id Tunnel Object identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 **/
sai_status_t sai_tunnel_increment_ref_count (sai_object_id_t tunnel_id);

/**
 * @brief Decrement tunnel reference count
 * @param[in] tunnel_id Tunnel Object identifier
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 **/
sai_status_t sai_tunnel_decrement_ref_count (sai_object_id_t tunnel_id);

/**
 * @brief Check whether tunnel object is in use by another object
 * @param[in] tunnel_id Tunnel Object identifier
 * @return true if tunnel object is used by another object, false otherwise
 **/
bool sai_is_tunnel_in_use (sai_object_id_t tunnel_id);

#endif /* __SAI_TUNNEL_UTIL_H__ */
