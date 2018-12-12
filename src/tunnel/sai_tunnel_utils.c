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
* @file sai_tunnel_utils.c
*
* @brief This file contains the util functions for SAI Tunnel component.
*
*************************************************************************/
#include "saitypes.h"
#include "sai_tunnel.h"
#include "sai_tunnel_util.h"
#include "sai_l3_common.h"
#include "sai_oid_utils.h"
#include "sai_map_utl.h"
#include "std_assert.h"
#include "std_mutex_lock.h"
#include <string.h>
#include <inttypes.h>

/* Simple Mutex lock for accessing Tunnel resources */
static std_mutex_lock_create_static_init_fast (g_sai_tunnel_lock);

static dn_sai_tunnel_global_t g_tunnel_global_info;

void dn_sai_tunnel_lock (void)
{
    std_mutex_lock (&g_sai_tunnel_lock);
}

void dn_sai_tunnel_unlock (void)
{
    std_mutex_unlock (&g_sai_tunnel_lock);
}

dn_sai_tunnel_global_t *dn_sai_tunnel_access_global_config (void)
{
    return &g_tunnel_global_info;
}

dn_sai_tunnel_t *dn_sai_tunnel_obj_get (sai_object_id_t tunnel_id)
{
    dn_sai_tunnel_t  tunnel_obj;

    memset (&tunnel_obj, 0, sizeof (dn_sai_tunnel_t));
    tunnel_obj.tunnel_id = tunnel_id;

    return ((dn_sai_tunnel_t *) std_rbtree_getexact (
            dn_sai_tunnel_tree_handle(), &tunnel_obj));
}

void *dn_sai_tunnel_hw_info_get (sai_object_id_t tunnel_id)
{
    dn_sai_tunnel_t *p_tunnel = dn_sai_tunnel_obj_get(tunnel_id);
    return ((p_tunnel == NULL) ? NULL:p_tunnel->hw_info);
}

dn_sai_tunnel_term_entry_t *dn_sai_tunnel_term_entry_get (
                                                sai_object_id_t tunnel_term_id)
{
    dn_sai_tunnel_term_entry_t  tunnel_term;

    memset (&tunnel_term, 0, sizeof (dn_sai_tunnel_term_entry_t));
    tunnel_term.term_entry_id = tunnel_term_id;

    return ((dn_sai_tunnel_term_entry_t *) std_rbtree_getexact (
            dn_sai_tunnel_term_tree_handle(), &tunnel_term));
}

sai_object_id_t dn_sai_tunnel_underlay_vr_get (sai_object_id_t tunnel_id)
{
    dn_sai_tunnel_t *tunnel_obj = NULL;
    sai_object_id_t  vr_id = SAI_NULL_OBJECT_ID;

    tunnel_obj = dn_sai_tunnel_obj_get (tunnel_id);

    if (tunnel_obj != NULL) {
        vr_id = tunnel_obj->underlay_vrf;
    }

    return vr_id;
}

sai_object_id_t dn_sai_tunnel_overlay_vr_get (sai_object_id_t tunnel_id)
{
    dn_sai_tunnel_t *tunnel_obj = NULL;
    sai_object_id_t  vr_id = SAI_NULL_OBJECT_ID;

    tunnel_obj = dn_sai_tunnel_obj_get (tunnel_id);

    if (tunnel_obj != NULL) {
        vr_id = tunnel_obj->overlay_vrf;
    }

    return vr_id;
}

sai_status_t sai_tunnel_object_id_validate (sai_object_id_t tunnel_id)
{
    sai_status_t  status = SAI_STATUS_SUCCESS;

    if (!sai_is_obj_id_tunnel (tunnel_id)) {
        SAI_TUNNEL_LOG_ERR ("0x%"PRIx64" is not a valid Tunnel obj Id.",
                            tunnel_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    dn_sai_tunnel_lock();

    if (dn_sai_tunnel_obj_get (tunnel_id) == NULL) {
        SAI_TUNNEL_LOG_ERR ("0x%"PRIx64" is not a valid Tunnel obj Id.",
                             tunnel_id);

        status = SAI_STATUS_INVALID_OBJECT_ID;
    }

    dn_sai_tunnel_unlock();

    return status;
}

sai_status_t sai_tunnel_increment_ref_count (sai_object_id_t tunnel_id)
{
    dn_sai_tunnel_t *tunnel_obj = dn_sai_tunnel_obj_get(tunnel_id);

    if(tunnel_obj == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    tunnel_obj->ref_count++;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_tunnel_decrement_ref_count (sai_object_id_t tunnel_id)
{
    dn_sai_tunnel_t *tunnel_obj = dn_sai_tunnel_obj_get(tunnel_id);

    if(tunnel_obj == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    if(tunnel_obj->ref_count == 0) {
        return SAI_STATUS_FAILURE;
    }
    tunnel_obj->ref_count--;
    return SAI_STATUS_SUCCESS;
}


bool sai_is_tunnel_in_use (sai_object_id_t tunnel_id)
{
    dn_sai_tunnel_t *tunnel_obj = dn_sai_tunnel_obj_get(tunnel_id);

    if(tunnel_obj == NULL) {
        return false;
    }

    if(tunnel_obj->ref_count > 0) {
        return true;
    }
    return false;
}

dn_sai_tunnel_map_t *dn_sai_tunnel_map_get (sai_object_id_t tunnel_map_id)
{
    dn_sai_tunnel_map_t  tunnel_map;

    memset (&tunnel_map, 0, sizeof (dn_sai_tunnel_map_t));
    tunnel_map.map_id = tunnel_map_id;

    return ((dn_sai_tunnel_map_t *) std_rbtree_getexact (dn_sai_tunnel_map_tree_handle(),
                                                         &tunnel_map));
}

dn_sai_tunnel_map_entry_t *dn_sai_tunnel_map_entry_get (sai_object_id_t tunnel_map_entry_id)
{
    dn_sai_tunnel_map_entry_t  tunnel_map_entry;

    memset (&tunnel_map_entry, 0, sizeof (dn_sai_tunnel_map_entry_t));
    tunnel_map_entry.tunnel_map_entry_id = tunnel_map_entry_id;

    return ((dn_sai_tunnel_map_entry_t *) std_rbtree_getexact (
                                                        dn_sai_tunnel_map_entry_tree_handle(),
                                                        &tunnel_map_entry));
}

bool dn_tunnel_map_is_encap_map_type(sai_tunnel_map_type_t type)
{
    if((type == SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI)||
       (type == SAI_TUNNEL_MAP_TYPE_OECN_TO_UECN) ||
       (type == SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI)) {
        return true;
    }

    return false;
}

dn_sai_tunnel_map_t *dn_sai_tunnel_find_tunnel_map_by_type(
                                              dn_sai_tunnel_t *p_tunnel,
                                              sai_tunnel_map_type_t type)
{
    uint_t idx = 0;
    dn_sai_tunnel_map_t *p_tunnel_map = NULL;
    sai_object_list_t *p_objlist = NULL;

    STD_ASSERT(p_tunnel != NULL);

    p_objlist =  dn_tunnel_map_is_encap_map_type(type) ?
                 &p_tunnel->tunnel_encap_mapper_list :
                 &p_tunnel->tunnel_decap_mapper_list;

    for(idx = 0; idx < p_objlist->count; idx++) {

        p_tunnel_map = dn_sai_tunnel_map_get(p_objlist->list[idx]);
        if((p_tunnel_map != NULL) && (p_tunnel_map->type == type)) {
            return p_tunnel_map;
        }
    }

    return NULL;
}

dn_sai_tunnel_map_entry_t *dn_sai_tunnel_find_tunnel_entry_by_bridge(
                                                       dn_sai_tunnel_map_t *p_tunnel_map,
                                                       sai_object_id_t bridge_oid)
{
    dn_sai_tunnel_map_entry_t *p_tunnel_map_entry = NULL;

    STD_ASSERT(p_tunnel_map != NULL);

    if(p_tunnel_map->type != SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI) {
        SAI_TUNNEL_LOG_ERR("Tunnel map 0x%"PRIx64" is not of type bridge to vni",
                           p_tunnel_map->map_id);
        return NULL;
    }

    p_tunnel_map_entry = sai_tunnel_get_first_tunnel_map_entry(
                                              &p_tunnel_map->tunnel_map_entry_list);

    while(p_tunnel_map_entry != NULL) {

        if(p_tunnel_map_entry->key.bridge_oid == bridge_oid) {
            return p_tunnel_map_entry;
        }

        p_tunnel_map_entry = sai_tunnel_get_next_tunnel_map_entry(
                                              &p_tunnel_map->tunnel_map_entry_list,
                                              p_tunnel_map_entry);
    }

    return NULL;
}

dn_sai_tunnel_map_entry_t *dn_sai_tunnel_find_tunnel_entry_by_vnid(
                                                          dn_sai_tunnel_map_t *p_tunnel_map,
                                                          sai_uint32_t vnid)
{
    dn_sai_tunnel_map_entry_t *p_tunnel_map_entry = NULL;

    STD_ASSERT(p_tunnel_map != NULL);

    if(p_tunnel_map->type != SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF) {
        SAI_TUNNEL_LOG_ERR("Tunnel map 0x%"PRIx64" is not of type vni to bridge",
                           p_tunnel_map->map_id);
        return NULL;
    }

    p_tunnel_map_entry = sai_tunnel_get_first_tunnel_map_entry(
                                                     &p_tunnel_map->tunnel_map_entry_list);

    while(p_tunnel_map_entry != NULL) {

        if(p_tunnel_map_entry->key.vnid == vnid) {
            return p_tunnel_map_entry;
        }

        p_tunnel_map_entry = sai_tunnel_get_next_tunnel_map_entry(
                                                      &p_tunnel_map->tunnel_map_entry_list,
                                                       p_tunnel_map_entry);
    }

    return NULL;
}

sai_status_t dn_sai_tunnel_find_bridge_to_vnid_mapping(sai_object_id_t tunnel_oid,
                                                       sai_object_id_t bridge_oid,
                                                       sai_uint32_t *vnid)
{
    dn_sai_tunnel_t *p_tunnel = NULL;
    dn_sai_tunnel_map_t *p_tunnel_map = NULL;
    dn_sai_tunnel_map_entry_t *p_tunnel_map_entry = NULL;

    p_tunnel = dn_sai_tunnel_obj_get(tunnel_oid);

    if(NULL == p_tunnel) {
        SAI_TUNNEL_LOG_ERR("Tunnel 0x%"PRIx64"not found",tunnel_oid);
        return SAI_STATUS_FAILURE;
    }

    p_tunnel_map = dn_sai_tunnel_find_tunnel_map_by_type(p_tunnel,
                                                         SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI);
    if(NULL == p_tunnel_map) {
        SAI_TUNNEL_LOG_ERR("Tunnel map of type bridge to vni not found in tunnel 0x%"
                           PRIx64"",tunnel_oid);
        return SAI_STATUS_FAILURE;
    }

    p_tunnel_map_entry = dn_sai_tunnel_find_tunnel_entry_by_bridge(p_tunnel_map, bridge_oid);

    if(NULL == p_tunnel_map_entry) {
        SAI_TUNNEL_LOG_ERR("Encap Tunnel map entry for bridge 0x%"PRIx64" not found in"
                           " tunnel map 0x%"PRIx64"",bridge_oid, p_tunnel_map->map_id);
        return SAI_STATUS_FAILURE;
    }

    *vnid = p_tunnel_map_entry->value.vnid;

    return SAI_STATUS_SUCCESS;
}

sai_status_t dn_sai_tunnel_find_vnid_to_bridge_mapping(sai_object_id_t tunnel_oid,
                                                       sai_uint32_t vnid,
                                                       sai_object_id_t *bridge_oid)
{
    dn_sai_tunnel_t *p_tunnel = NULL;
    dn_sai_tunnel_map_t *p_tunnel_map = NULL;
    dn_sai_tunnel_map_entry_t *p_tunnel_map_entry = NULL;

    p_tunnel = dn_sai_tunnel_obj_get(tunnel_oid);

    if(NULL == p_tunnel) {
        SAI_TUNNEL_LOG_ERR("Tunnel 0x%"PRIx64"not found",tunnel_oid);
        return SAI_STATUS_FAILURE;
    }

    p_tunnel_map = dn_sai_tunnel_find_tunnel_map_by_type(p_tunnel,
                                                         SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF);
    if(NULL == p_tunnel_map) {

        SAI_TUNNEL_LOG_ERR("Tunnel map of type vni to bridge found in tunnel 0x%"
                           PRIx64"",tunnel_oid);
        return SAI_STATUS_FAILURE;
    }

    p_tunnel_map_entry = dn_sai_tunnel_find_tunnel_entry_by_vnid(p_tunnel_map, vnid);

    if(NULL == p_tunnel_map_entry) {
        SAI_TUNNEL_LOG_ERR("Decap Tunnel map entry for vnid %u not found in"
                           " tunnel map 0x%"PRIx64"", vnid, p_tunnel_map->map_id);
        return SAI_STATUS_FAILURE;
    }

    *bridge_oid = p_tunnel_map_entry->value.bridge_oid;

    return SAI_STATUS_SUCCESS;
}

sai_status_t dn_sai_tunnel_decap_tunnel_map_bridge_count_get(sai_object_id_t tunnel_map_id,
                                                             sai_object_id_t bridge_id,
                                                             uint_t *bridge_count)
{
    uint_t count = 0;
    dn_sai_tunnel_map_t *p_tunnel_map = NULL;
    dn_sai_tunnel_map_entry_t *p_tunnel_map_entry = NULL;

    STD_ASSERT(bridge_count != NULL);

    p_tunnel_map = dn_sai_tunnel_map_get(tunnel_map_id);

    if(NULL == p_tunnel_map) {
        SAI_TUNNEL_LOG_ERR("Tunnel map id: 0x%"PRIx64" not found",
                           tunnel_map_id);
        return SAI_STATUS_FAILURE;
    }

    if(p_tunnel_map->type != SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF) {
        SAI_TUNNEL_LOG_ERR("Tunnel map %"PRIx64" is not decap tunnel map",
                           tunnel_map_id);
        return SAI_STATUS_FAILURE;
    }

    p_tunnel_map_entry = sai_tunnel_get_first_tunnel_map_entry(
                                       &p_tunnel_map->tunnel_map_entry_list);

    while(p_tunnel_map_entry != NULL) {

        if(p_tunnel_map_entry->value.bridge_oid == bridge_id) {
            count++;
        }

        p_tunnel_map_entry = sai_tunnel_get_next_tunnel_map_entry(
                                                      &p_tunnel_map->tunnel_map_entry_list,
                                                      p_tunnel_map_entry);
    }

    *bridge_count = count;

    return SAI_STATUS_SUCCESS;
}

sai_status_t dn_sai_tunnel_map_dep_tunnel_add (sai_object_id_t tunnel_map_id,
                                               sai_object_id_t tunnel_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_TUNNEL_MAP_TO_TUNNEL_LIST;
    key.id1  = tunnel_map_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = tunnel_id;

    return sai_map_insert (&key, &value);
}

sai_status_t dn_sai_tunnel_map_dep_tunnel_remove(sai_object_id_t tunnel_map_id,
                                                 sai_object_id_t tunnel_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    uint_t         count = 0;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_TUNNEL_MAP_TO_TUNNEL_LIST;
    key.id1  = tunnel_map_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = tunnel_id;

    sai_map_delete_elements (&key, &value, SAI_MAP_VAL_FILTER_VAL1);

    if (sai_map_get_val_count (&key, &count) == SAI_STATUS_SUCCESS) {
        if (count == 0) {
            sai_map_delete (&key);
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t dn_sai_tunnel_map_dep_tunnel_get (sai_object_id_t  tunnel_map_id,
                                               uint_t          *count,
                                               sai_object_id_t *tunnel_id_list)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_status_t   rc;
    uint_t       index;

    if((count == NULL) || (tunnel_id_list == NULL)) {
        SAI_TUNNEL_LOG_ERR ("Invalid inputs given for getting tunnels from "
                            "tunnel map 0x%"PRIx64"",tunnel_map_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    sai_map_data_t data [*count];

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data [0], 0, (*count) * sizeof (sai_map_data_t));

    key.type = SAI_MAP_TYPE_TUNNEL_MAP_TO_TUNNEL_LIST;
    key.id1  = tunnel_map_id;

    value.count = *count;
    value.data  = data;

    rc = sai_map_get (&key, &value);

    if(rc == SAI_STATUS_ITEM_NOT_FOUND) {
        *count = 0;
        return SAI_STATUS_SUCCESS;
    }

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }

    for (index = 0; index < value.count; index++) {
        tunnel_id_list [index] = data [index].val1;
    }
    *count = value.count;

    return SAI_STATUS_SUCCESS;
}

sai_status_t dn_sai_tunnel_map_dep_tunnel_count_get(sai_object_id_t  tunnel_map_id,
                                                    uint_t          *p_out_count)
{
    sai_map_key_t  key;
    sai_status_t   rc;

    if(p_out_count == NULL) {
        SAI_TUNNEL_LOG_ERR("Invalid output parament to get dependent tunnel "
                           "count for tunnel map 0x%"PRIx64"", tunnel_map_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset (&key, 0, sizeof (key));

    key.type = SAI_MAP_TYPE_TUNNEL_MAP_TO_TUNNEL_LIST;
    key.id1  = tunnel_map_id;

    rc = sai_map_get_val_count (&key, p_out_count);

    if(rc == SAI_STATUS_ITEM_NOT_FOUND) {
        *p_out_count = 0;
        return SAI_STATUS_SUCCESS;
    }

    return rc;
}

sai_status_t dn_sai_tunnel_map_dep_tunnel_get_at_index (sai_object_id_t  tunnel_map_id,
                                                        uint_t           index,
                                                        sai_object_id_t *tunnel_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_status_t   rc = SAI_STATUS_FAILURE;
    sai_map_data_t data;

    if(tunnel_id == NULL) {
        SAI_TUNNEL_LOG_ERR("Tunnel is NULL for tunnel map 0x%"PRIx64" in "
                           "tunnel to bridge port get at index", tunnel_map_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (sai_map_data_t));

    key.type = SAI_MAP_TYPE_TUNNEL_MAP_TO_TUNNEL_LIST;
    key.id1 = tunnel_map_id;
    value.data = &data;

    rc = sai_map_get_element_at_index(&key, index, &value);
    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }

    *tunnel_id = data.val1;
    return SAI_STATUS_SUCCESS;
}
