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
* @file sai_tunnel_map_obj.c
*
* @brief This file contains implementation for tunnel map and tunnel
*        map entry objects.
*************************************************************************/
#include "sai_tunnel.h"
#include "sai_common_infra.h"
#include "sai_gen_utils.h"
#include "sai_common_utils.h"
#include "sai_oid_utils.h"
#include "sai_bridge_api.h"
#include "sai_tunnel_api_utils.h"
#include "sai_tunnel_util.h"
#include "sai_bridge_common.h"

#include "std_rbtree.h"
#include "std_llist.h"
#include "std_assert.h"

#include "saistatus.h"
#include "saitypes.h"
#include "saitunnel.h"

#include <stdlib.h>
#include <inttypes.h>
static dn_sai_id_gen_info_t tunnel_map_gen_info;
static dn_sai_id_gen_info_t tunnel_map_entry_gen_info;

bool dn_sai_tunnel_map_id_in_use(uint64_t id)
{
    sai_object_id_t tunnel_map_id =
        sai_uoid_create(SAI_OBJECT_TYPE_TUNNEL_MAP, id);

    if(dn_sai_tunnel_map_get(tunnel_map_id) != NULL) {
        return true;
    }

    return false;
}

static sai_object_id_t dn_sai_tunnel_map_id_generate(void)
{
    if(SAI_STATUS_SUCCESS ==
       dn_sai_get_next_free_id(&tunnel_map_gen_info)) {

        return (sai_uoid_create(SAI_OBJECT_TYPE_TUNNEL_MAP,
                                tunnel_map_gen_info.cur_id));
    }

    return SAI_NULL_OBJECT_ID;
}

bool dn_sai_tunnel_map_entry_id_in_use(uint64_t id)
{
    sai_object_id_t tunnel_map_entry_id =
        sai_uoid_create(SAI_OBJECT_TYPE_TUNNEL_MAP_ENTRY, id);

    if(dn_sai_tunnel_map_entry_get(tunnel_map_entry_id) != NULL) {
        return true;
    }

    return false;
}

static sai_object_id_t dn_sai_tunnel_map_entry_id_generate(void)
{
    if(SAI_STATUS_SUCCESS ==
       dn_sai_get_next_free_id(&tunnel_map_entry_gen_info)) {

        return (sai_uoid_create(SAI_OBJECT_TYPE_TUNNEL_MAP_ENTRY,
                                tunnel_map_entry_gen_info.cur_id));
    }

    return SAI_NULL_OBJECT_ID;
}

void dn_sai_tunnel_map_init()
{
    tunnel_map_gen_info.cur_id = 0;
    tunnel_map_gen_info.is_wrappped = false;
    tunnel_map_gen_info.mask = SAI_UOID_NPU_OBJ_ID_MASK;
    tunnel_map_gen_info.is_id_in_use = dn_sai_tunnel_map_id_in_use;
}

void dn_sai_tunnel_map_entry_init()
{
    tunnel_map_entry_gen_info.cur_id = 0;
    tunnel_map_entry_gen_info.is_wrappped = false;
    tunnel_map_entry_gen_info.mask = SAI_UOID_NPU_OBJ_ID_MASK;
    tunnel_map_entry_gen_info.is_id_in_use = dn_sai_tunnel_map_entry_id_in_use;
}

static sai_status_t dn_sai_tunnel_map_validate_type(sai_tunnel_map_type_t type)
{
    if((type == SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI) ||
       (type == SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF)) {

        return SAI_STATUS_SUCCESS;
    }

    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t dn_sai_tunnel_map_fill_attr(dn_sai_tunnel_map_t *p_tunnel_map,
                                                uint32_t attr_count,
                                                const sai_attribute_t *attr_list)
{
    uint_t idx = 0;
    sai_status_t sai_rc = SAI_STATUS_FAILURE;

    STD_ASSERT(p_tunnel_map != NULL);
    STD_ASSERT(attr_list != NULL);

    for(idx = 0; idx < attr_count; idx++) {

        switch(attr_list[idx].id) {

            case SAI_TUNNEL_MAP_ATTR_TYPE:

                sai_rc = dn_sai_tunnel_map_validate_type(attr_list[idx].value.s32);
                if(sai_rc != SAI_STATUS_SUCCESS) {

                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + idx;
                }

                p_tunnel_map->type = attr_list[idx].value.s32;
                break;

            default:

                SAI_TUNNEL_LOG_ERR("Tunnel map unknown attribute id %d at idx %u",
                                   attr_list[idx].id, idx);

                return (SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + idx);

        }
    }

    return sai_rc;
}

static sai_status_t dn_sai_create_tunnel_map(sai_object_id_t *tunnel_map_id,
                                             sai_object_id_t switch_id,
                                             uint32_t attr_count,
                                             const sai_attribute_t *attr_list)
{
    t_std_error rc = 0;
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_tunnel_map_t *p_tunnel_map = NULL;

    STD_ASSERT(tunnel_map_id != NULL);
    STD_ASSERT(attr_list != NULL);

    sai_rc = dn_sai_tunnel_attr_list_validate (SAI_OBJECT_TYPE_TUNNEL_MAP,
                                               attr_count, attr_list,
                                               SAI_OP_CREATE);
    if(sai_rc != SAI_STATUS_SUCCESS){

        SAI_TUNNEL_LOG_ERR("Tunnel map attribute validation failed for "
                           "create operation");
        return sai_rc;
    }

    p_tunnel_map = calloc(1, sizeof(dn_sai_tunnel_map_t));

    if(NULL == p_tunnel_map) {

        SAI_TUNNEL_LOG_ERR("Failed to allocate memory for tunnel map node");
        return SAI_STATUS_NO_MEMORY;
    }

    do {

        sai_rc = dn_sai_tunnel_map_fill_attr(p_tunnel_map, attr_count, attr_list);

        if(sai_rc != SAI_STATUS_SUCCESS) {

            SAI_TUNNEL_LOG_ERR("Failed to fill tunnel map attributes");
            break;
        }

        p_tunnel_map->map_id = dn_sai_tunnel_map_id_generate();

        if(SAI_NULL_OBJECT_ID == p_tunnel_map->map_id) {

            SAI_TUNNEL_LOG_ERR("Failed to generate object id for tunnel map");
            sai_rc = SAI_STATUS_TABLE_FULL;
            break;
        }

        rc = std_rbtree_insert(dn_sai_tunnel_map_tree_handle(), p_tunnel_map);

        if (rc != STD_ERR_OK) {

            SAI_TUNNEL_LOG_ERR ("Error in inserting tunnel map id: 0x%"PRIx64" "
                                "to database", p_tunnel_map->map_id);

            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

        std_dll_init(&p_tunnel_map->tunnel_map_entry_list);
        *tunnel_map_id = p_tunnel_map->map_id;

    }while(0);

    if(sai_rc != SAI_STATUS_SUCCESS) {

        free(p_tunnel_map);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_remove_tunnel_map(sai_object_id_t tunnel_map_id)
{
    dn_sai_tunnel_map_t *p_tunnel_map = NULL;

    p_tunnel_map = dn_sai_tunnel_map_get(tunnel_map_id);

    if(NULL == p_tunnel_map) {

        SAI_TUNNEL_LOG_ERR("Failed to remove tunnel map id: 0x%"PRIx64".Object not found",
                           tunnel_map_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if(p_tunnel_map->ref_count > 0){

        SAI_TUNNEL_LOG_ERR("Tunnel map is in use (ref count=%u)",
                           p_tunnel_map->ref_count);

        return SAI_STATUS_OBJECT_IN_USE;
    }

    std_rbtree_remove(dn_sai_tunnel_map_tree_handle(), p_tunnel_map);

    free(p_tunnel_map);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_set_tunnel_map_attr(sai_object_id_t tunnel_map_id,
                                               const sai_attribute_t *attr)
{
    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t dn_sai_get_tunnel_map_attr(sai_object_id_t tunnel_map_id,
                                               uint32_t attr_count,
                                               sai_attribute_t *attr_list)
{
    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t dn_sai_tunnel_map_entry_validate(dn_sai_tunnel_map_entry_t *p_tunnel_map_entry)
{
    dn_sai_tunnel_map_t *p_tunnel_map = NULL;

    STD_ASSERT(p_tunnel_map_entry != NULL);

    p_tunnel_map = dn_sai_tunnel_map_get(p_tunnel_map_entry->tunnel_map_id);

    if(p_tunnel_map_entry->type == SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI) {

        if(dn_sai_tunnel_find_tunnel_entry_by_bridge(p_tunnel_map,
                                                     p_tunnel_map_entry->key.bridge_oid))
        {
            SAI_TUNNEL_LOG_ERR("Bridge to VNID mapping already exists in tunnel map");
            return SAI_STATUS_FAILURE;
        }

    } else {

        if(dn_sai_tunnel_find_tunnel_entry_by_vnid(p_tunnel_map,
                                                   p_tunnel_map_entry->key.vnid))
        {
            SAI_TUNNEL_LOG_ERR("VNID to Bridge mapping already exists in tunnel map");
            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_tunnel_map_entry_fill_attr(dn_sai_tunnel_map_entry_t *p_tunnel_map_entry,
                                                      uint32_t attr_count,
                                                      const sai_attribute_t *attr_list)
{
    uint_t idx = 0;
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_tunnel_map_t *p_tunnel_map = NULL;
    dn_sai_bridge_info_t *p_bridge_info = NULL;

    STD_ASSERT(p_tunnel_map_entry != NULL);
    STD_ASSERT(attr_list != NULL);

    for(idx = 0; idx < attr_count; idx++) {

        switch(attr_list[idx].id) {

            case SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE:

                sai_rc = dn_sai_tunnel_map_validate_type(attr_list[idx].value.s32);
                if(sai_rc != SAI_STATUS_SUCCESS) {

                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + idx;
                }
                p_tunnel_map_entry->type = attr_list[idx].value.s32;
                break;

            case SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP:

                 p_tunnel_map = dn_sai_tunnel_map_get(attr_list[idx].value.oid);
                 if(NULL == p_tunnel_map) {

                     return SAI_STATUS_INVALID_ATTR_VALUE_0 + idx;
                 }
                 p_tunnel_map_entry->tunnel_map_id = attr_list[idx].value.oid;
                 break;

            case SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_KEY:
            case SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE:

                 sai_rc = sai_bridge_cache_read(attr_list[idx].value.oid,
                                                 &p_bridge_info);
                 if(sai_rc != SAI_STATUS_SUCCESS) {

                     SAI_TUNNEL_LOG_ERR("Failed to get bridge info for bridge"
                                        "oid = %"PRIx64"",attr_list[idx].value.oid);
                     return SAI_STATUS_INVALID_ATTR_VALUE_0 + idx;
                 }

                 if(attr_list[idx].id == SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_KEY) {

                     p_tunnel_map_entry->key.bridge_oid = attr_list[idx].value.oid;

                 } else {

                     p_tunnel_map_entry->value.bridge_oid = attr_list[idx].value.oid;
                 }
                 break;

            case SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_KEY:
            case SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE:

                 if(attr_list[idx].value.u32 > SAI_TUNNEL_VNID_MAX_VALUE) {

                     SAI_TUNNEL_LOG_ERR("VNID value %u is greater than the maximum "
                                        "allowed VNID %u", attr_list[idx].value.u32,
                                        SAI_TUNNEL_VNID_MAX_VALUE);

                     return SAI_STATUS_INVALID_ATTR_VALUE_0 + idx;
                 }

                 if(attr_list[idx].id == SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_KEY) {
                     p_tunnel_map_entry->key.vnid = attr_list[idx].value.u32;
                 } else {
                     p_tunnel_map_entry->value.vnid = attr_list[idx].value.u32;
                 }
                 break;

            default:

                SAI_TUNNEL_LOG_ERR("Tunnel map unknown attribute id %d at idx %u",
                                   attr_list[idx].id, idx);

                return (SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + idx);
        }
    }

    if(p_tunnel_map->type != p_tunnel_map_entry->type) {

        SAI_TUNNEL_LOG_ERR("Tunnel map type %d does not match with tunnel map "
                           "entry type %d", p_tunnel_map->type, p_tunnel_map_entry->type);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    return dn_sai_tunnel_map_entry_validate(p_tunnel_map_entry);
}

static sai_status_t dn_sai_create_tunnel_map_entry(sai_object_id_t *tunnel_map_entry_id,
                                                   sai_object_id_t switch_id,
                                                   uint32_t attr_count,
                                                   const sai_attribute_t *attr_list)
{
    t_std_error rc = 0;
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    sai_object_id_t bridge_oid = SAI_NULL_OBJECT_ID;
    dn_sai_tunnel_map_entry_t *p_tunnel_map_entry = NULL;
    dn_sai_tunnel_map_t *p_tunnel_map = NULL;
    dn_sai_bridge_info_t *p_bridge_info = NULL;

    STD_ASSERT(tunnel_map_entry_id != NULL);
    STD_ASSERT(attr_list != NULL);

    sai_rc = dn_sai_tunnel_attr_list_validate (SAI_OBJECT_TYPE_TUNNEL_MAP_ENTRY,
                                               attr_count, attr_list,
                                               SAI_OP_CREATE);
    if(sai_rc != SAI_STATUS_SUCCESS){

        SAI_TUNNEL_LOG_ERR("Tunnel map entry attribute validation failed for create operation");
        return sai_rc;
    }

    p_tunnel_map_entry = calloc(1, sizeof(dn_sai_tunnel_map_entry_t));

    if(NULL == p_tunnel_map_entry){

        SAI_TUNNEL_LOG_ERR("Failed to allocate memory for tunnel map entry node");
        return SAI_STATUS_NO_MEMORY;
    }

    dn_sai_tunnel_lock();
    sai_bridge_lock();
    do {

        sai_rc = dn_sai_tunnel_map_entry_fill_attr(p_tunnel_map_entry, attr_count,
                                                   attr_list);
        if(sai_rc != SAI_STATUS_SUCCESS){
            break;
        }

        p_tunnel_map_entry->tunnel_map_entry_id = dn_sai_tunnel_map_entry_id_generate();

        if(SAI_NULL_OBJECT_ID == p_tunnel_map_entry->tunnel_map_entry_id) {

            SAI_TUNNEL_LOG_ERR("Failed to generate object id for tunnel map entry");
            sai_rc = SAI_STATUS_TABLE_FULL;
            break;
        }

        sai_rc = sai_tunnel_npu_api_get()->tunnel_map_entry_create(p_tunnel_map_entry);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_TUNNEL_LOG_ERR("Failed to create tunnel map entry in the NPU");
            break;
        }


        rc = std_rbtree_insert(dn_sai_tunnel_map_entry_tree_handle(), p_tunnel_map_entry);

        if (rc != STD_ERR_OK) {

            SAI_TUNNEL_LOG_ERR ("Error in inserting tunnel map entry id: 0x%"PRIx64" "
                                "to database", p_tunnel_map_entry->tunnel_map_entry_id);

            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

        p_tunnel_map = dn_sai_tunnel_map_get(p_tunnel_map_entry->tunnel_map_id);

        std_dll_insertatback(&p_tunnel_map->tunnel_map_entry_list,
                             &p_tunnel_map_entry->tunnel_map_link);

        if(p_tunnel_map_entry->type == SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI) {

            bridge_oid = p_tunnel_map_entry->key.bridge_oid;

        } else {

            bridge_oid = p_tunnel_map_entry->value.bridge_oid;
        }

        sai_bridge_cache_read(bridge_oid, &p_bridge_info);

        p_tunnel_map->ref_count++;
        p_bridge_info->ref_count++;

        *tunnel_map_entry_id = p_tunnel_map_entry->tunnel_map_entry_id;

    } while(0);

    sai_bridge_unlock();
    dn_sai_tunnel_unlock();

    if(sai_rc != SAI_STATUS_SUCCESS) {

        free(p_tunnel_map_entry);
    }

    return sai_rc;
}

static bool dn_sai_is_tunnel_map_entry_in_use(dn_sai_tunnel_map_entry_t *p_tunnel_map_entry)
{
    uint_t tunnel_idx = 0;
    uint_t tunnel_count = 0;
    uint_t bridge_count = 0;
    bool is_connected = false;
    bool is_encap_map = false;
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    sai_object_id_t tunnel_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t bridge_oid = SAI_NULL_OBJECT_ID;

    sai_rc = dn_sai_tunnel_map_dep_tunnel_count_get(p_tunnel_map_entry->tunnel_map_id,
                                                    &tunnel_count);
    if(sai_rc != SAI_STATUS_SUCCESS) {

        SAI_TUNNEL_LOG_ERR("Failed to get the dependent tunnel count for the "
                           "tunnel map entry 0x%"PRIx64"",
                           p_tunnel_map_entry->tunnel_map_entry_id);
        return false;
    }

    if(p_tunnel_map_entry->type == SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI) {
        bridge_oid = p_tunnel_map_entry->key.bridge_oid;
        is_encap_map = true;
    } else {
        bridge_oid = p_tunnel_map_entry->value.bridge_oid;
        /* Get the count of decap tunnel map entries which are mapping to
         * the bridge*/
        sai_rc = dn_sai_tunnel_decap_tunnel_map_bridge_count_get(
                                              p_tunnel_map_entry->tunnel_map_id,
                                              bridge_oid, &bridge_count);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_TUNNEL_LOG_ERR("Failed to get bridge count for bridge 0x%"PRIx64
                               " in tunnel map 0x%"PRIx64"",bridge_oid,
                               p_tunnel_map_entry->tunnel_map_id);
            return true;
        }
    }

    for(tunnel_idx = 0; tunnel_idx < tunnel_count; tunnel_idx++) {
        sai_rc = dn_sai_tunnel_map_dep_tunnel_get_at_index(p_tunnel_map_entry->tunnel_map_id,
                                                           tunnel_idx, &tunnel_id);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_TUNNEL_LOG_ERR("Failed to get the dependent tunnel for the "
                               "tunnel map entry 0x%"PRIx64"",
                               p_tunnel_map_entry->tunnel_map_entry_id);
            continue;
        }

        is_connected = sai_bridge_is_bridge_connected_to_tunnel(bridge_oid, tunnel_id);
        if(is_connected) {

            if(!is_encap_map) {
                /* Alternative decap entry exists to map a vnid to the bridge,
                 * so this decap entry can be changed or removed*/
                if(bridge_count > 1) {
                    continue;
                }
            }
            SAI_TUNNEL_LOG_ERR("Cannot change/remove tunnel map entry 0x%"PRIx64
                               " as bridge port exists between bridge 0x%"PRIx64
                               " and  tunnel 0x%"PRIx64"",
                               p_tunnel_map_entry->tunnel_map_entry_id,
                               bridge_oid, tunnel_id);
            return true;
        }
    }

    return false;
}

static sai_status_t dn_sai_remove_tunnel_map_entry(sai_object_id_t tunnel_map_entry_id)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    sai_object_id_t bridge_oid = SAI_NULL_OBJECT_ID;
    dn_sai_tunnel_map_t *p_tunnel_map = NULL;
    dn_sai_bridge_info_t *p_bridge_info = NULL;
    dn_sai_tunnel_map_entry_t *p_tunnel_map_entry = NULL;

    dn_sai_tunnel_lock();
    sai_bridge_lock();
    do {
        p_tunnel_map_entry = dn_sai_tunnel_map_entry_get(tunnel_map_entry_id);

        if(NULL == p_tunnel_map_entry) {

            SAI_TUNNEL_LOG_ERR("Failed to remove tunnel map entry id: 0x%"PRIx64
                               ".Object not found", tunnel_map_entry_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if(dn_sai_is_tunnel_map_entry_in_use(p_tunnel_map_entry)) {
            sai_rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        sai_rc = sai_tunnel_npu_api_get()->tunnel_map_entry_remove(p_tunnel_map_entry);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_TUNNEL_LOG_ERR("Failed to remove tunnel map entry id: 0x%"PRIx64
                               " in the NPU", tunnel_map_entry_id);
            break;
        }

        p_tunnel_map = dn_sai_tunnel_map_get(p_tunnel_map_entry->tunnel_map_id);

        std_dll_remove(&p_tunnel_map->tunnel_map_entry_list,
                       &p_tunnel_map_entry->tunnel_map_link);

        std_rbtree_remove(dn_sai_tunnel_map_entry_tree_handle(), p_tunnel_map_entry);

        if(p_tunnel_map_entry->type == SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI) {
            bridge_oid = p_tunnel_map_entry->key.bridge_oid;
        } else {
            bridge_oid = p_tunnel_map_entry->value.bridge_oid;
        }

        sai_bridge_cache_read(bridge_oid, &p_bridge_info);

        p_tunnel_map->ref_count--;
        p_bridge_info->ref_count--;

        free(p_tunnel_map_entry);

        sai_rc = SAI_STATUS_SUCCESS;

    } while(0);

    sai_bridge_unlock();
    dn_sai_tunnel_unlock();

    return sai_rc;
}

static sai_status_t dn_sai_validate_and_fill_tunnel_map_entry_set(
                              dn_sai_tunnel_map_entry_t *p_old_tunnel_map_entry,
                              dn_sai_tunnel_map_entry_t *p_new_tunnel_map_entry,
                              const sai_attribute_t *attr)
{
    sai_uint32_t vnid = 0;
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_bridge_info_t *p_bridge_info = NULL;
    sai_object_id_t bridge_oid = SAI_NULL_OBJECT_ID;

    if(attr->id == SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE) {

        if(p_old_tunnel_map_entry->type != SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF) {

            SAI_TUNNEL_LOG_ERR("Bridge id can be provided as value only if "
                               "the tunnel map entry is of type vni to bridge map");
            return SAI_STATUS_INVALID_ATTRIBUTE_0;
        }

        bridge_oid =  attr->value.oid;

        sai_rc = sai_bridge_cache_read(bridge_oid, &p_bridge_info);

        if(sai_rc != SAI_STATUS_SUCCESS) {

            SAI_TUNNEL_LOG_ERR("Failed to get bridge info for bridge"
                               "oid = %"PRIx64"",bridge_oid);
            return SAI_STATUS_INVALID_ATTR_VALUE_0;
        }

        if(dn_sai_is_tunnel_map_entry_in_use(p_old_tunnel_map_entry)) {
            return SAI_STATUS_OBJECT_IN_USE;
        }

        p_new_tunnel_map_entry->value.bridge_oid = bridge_oid;

    } else if(attr->id == SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE) {

        if(p_old_tunnel_map_entry->type != SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI) {

            SAI_TUNNEL_LOG_ERR("VNID can be provided as value only if "
                               "the tunnel map entry is of type bridge to vni map");
            return SAI_STATUS_INVALID_ATTRIBUTE_0;
        }

        vnid = attr->value.u32;

        if(vnid > SAI_TUNNEL_VNID_MAX_VALUE) {
            SAI_TUNNEL_LOG_ERR("VNID value %u is greater than the maximum "
                               "allowed VNID %u", vnid,
                               SAI_TUNNEL_VNID_MAX_VALUE);

            return SAI_STATUS_INVALID_ATTR_VALUE_0;
        }
        p_new_tunnel_map_entry->value.vnid = vnid;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_set_tunnel_map_entry_attr(sai_object_id_t tunnel_map_entry_id,
                                                     const sai_attribute_t *attr)
{
    uint32_t attr_count = 1;
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_tunnel_map_entry_t *p_tunnel_map_entry = NULL;
    dn_sai_tunnel_map_entry_t new_tunnel_map_entry;
    sai_object_id_t old_bridge_oid = SAI_NULL_OBJECT_ID;
    dn_sai_bridge_info_t *p_bridge_info = NULL;
    dn_sai_bridge_info_t *p_old_bridge_info = NULL;

    STD_ASSERT(attr != NULL);

    sai_rc = dn_sai_tunnel_attr_list_validate (SAI_OBJECT_TYPE_TUNNEL_MAP_ENTRY,
                                               attr_count, attr, SAI_OP_SET);
    if(sai_rc != SAI_STATUS_SUCCESS){

        SAI_TUNNEL_LOG_ERR("Tunnel map entry attribute validation failed for set operation");
        return sai_rc;
    }

    dn_sai_tunnel_lock();
    sai_bridge_lock();
    do {
        p_tunnel_map_entry = dn_sai_tunnel_map_entry_get(tunnel_map_entry_id);

        if(NULL == p_tunnel_map_entry) {

            SAI_TUNNEL_LOG_ERR("Failed to set attribute on tunnel map entry id: 0x%"PRIx64
                               ".Object not found", tunnel_map_entry_id);

            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        new_tunnel_map_entry = *p_tunnel_map_entry;

        sai_rc = dn_sai_validate_and_fill_tunnel_map_entry_set(p_tunnel_map_entry,
                                                               &new_tunnel_map_entry,
                                                               attr);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_TUNNEL_LOG_ERR("Failed to validate attribute id %u "
                               "for tunnel map entry id 0x%"PRIx64"",
                               attr->id, tunnel_map_entry_id);
            break;
        }

        if(attr->id == SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE) {

            old_bridge_oid = p_tunnel_map_entry->value.bridge_oid;

            sai_rc = sai_bridge_cache_read(old_bridge_oid, &p_old_bridge_info);

            if(sai_rc != SAI_STATUS_SUCCESS) {

                SAI_TUNNEL_LOG_ERR("Failed to get bridge info for bridge"
                                   "oid = %"PRIx64"",old_bridge_oid);
                sai_rc =SAI_STATUS_ITEM_NOT_FOUND;
                break;
            }

            sai_rc = sai_bridge_cache_read(attr->value.oid, &p_bridge_info);

            if(sai_rc != SAI_STATUS_SUCCESS) {

                SAI_TUNNEL_LOG_ERR("Failed to get bridge info for bridge"
                                   "oid = %"PRIx64"",attr->value.oid);
                sai_rc =SAI_STATUS_ITEM_NOT_FOUND;
                break;
            }
        }

        sai_rc = sai_tunnel_npu_api_get()->tunnel_map_entry_set(&new_tunnel_map_entry);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_TUNNEL_LOG_ERR("Failed to set attribute id %u on tunnel map entry id: 0x%"
                               PRIx64" in the NPU", attr->id, tunnel_map_entry_id);
            break;
        }

        /*Update the new values*/
        if(attr->id == SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE) {

            p_old_bridge_info->ref_count--;
            p_tunnel_map_entry->value.bridge_oid = attr->value.oid;
            p_bridge_info->ref_count++;

        } else if(attr->id == SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE) {
            p_tunnel_map_entry->value.vnid = attr->value.u32;
        }

    } while(0);
    sai_bridge_unlock();
    dn_sai_tunnel_unlock();

    return sai_rc;
}

static sai_status_t dn_sai_get_tunnel_map_entry_attr(sai_object_id_t tunnel_map_entry_id,
                                                     uint32_t attr_count,
                                                     sai_attribute_t *attr_list)
{
    uint_t tunnel_map_entry_idx = 0;
    sai_status_t sai_rc =  SAI_STATUS_FAILURE;
    dn_sai_tunnel_map_entry_t *p_tunnel_map_entry = NULL;

    STD_ASSERT(attr_list != NULL);

    dn_sai_tunnel_lock();

    do {
        p_tunnel_map_entry =  dn_sai_tunnel_map_entry_get(tunnel_map_entry_id);
        if(NULL == p_tunnel_map_entry) {

            SAI_TUNNEL_LOG_ERR("Failed to set attribute on tunnel map entry id: 0x%"PRIx64
                               ".Object not found", tunnel_map_entry_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_rc = SAI_STATUS_SUCCESS;

        for(tunnel_map_entry_idx = 0; tunnel_map_entry_idx < attr_count; tunnel_map_entry_idx++)
        {
            switch(attr_list[tunnel_map_entry_idx].id) {
                case SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE:
                    attr_list[tunnel_map_entry_idx].value.s32 = p_tunnel_map_entry->type;
                    break;
                case SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP :
                    attr_list[tunnel_map_entry_idx].value.oid = p_tunnel_map_entry->tunnel_map_id;
                    break;
                case SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_KEY:
                    if(p_tunnel_map_entry->type != SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI)
                    {
                        sai_rc = SAI_STATUS_INVALID_ATTRIBUTE_0 + tunnel_map_entry_idx;
                        break;
                    }
                    attr_list[tunnel_map_entry_idx].value.oid = p_tunnel_map_entry->key.bridge_oid;
                    break;
                case SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE:
                    if(p_tunnel_map_entry->type != SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF)
                    {
                        sai_rc = SAI_STATUS_INVALID_ATTRIBUTE_0 + tunnel_map_entry_idx;
                        break;
                    }
                    attr_list[tunnel_map_entry_idx].value.oid = p_tunnel_map_entry->value.bridge_oid;
                    break;
                case SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_KEY:
                    if(p_tunnel_map_entry->type != SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF)
                    {
                        sai_rc = SAI_STATUS_INVALID_ATTRIBUTE_0 + tunnel_map_entry_idx;
                        break;
                    }
                    attr_list[tunnel_map_entry_idx].value.u32 = p_tunnel_map_entry->key.vnid;
                    break;
                case SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE:
                    if(p_tunnel_map_entry->type != SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI)
                    {
                        sai_rc = SAI_STATUS_INVALID_ATTRIBUTE_0 + tunnel_map_entry_idx;
                        break;
                    }
                    attr_list[tunnel_map_entry_idx].value.u32 = p_tunnel_map_entry->value.vnid;
                    break;
                default:
                    SAI_TUNNEL_LOG_ERR("Unknown attr id %u in tunnel map entry %"PRIx64"get",
                                       attr_list[tunnel_map_entry_idx].id,
                                       p_tunnel_map_entry->tunnel_map_entry_id);
                    sai_rc = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + tunnel_map_entry_idx;
            }

            if(sai_rc != SAI_STATUS_SUCCESS) {
                break;
            }
        }
    } while(0);

    dn_sai_tunnel_unlock();
    return sai_rc;
}

void dn_sai_tunnel_map_obj_api_fill (sai_tunnel_api_t *api_table)
{
    api_table->create_tunnel_map        = dn_sai_create_tunnel_map;
    api_table->remove_tunnel_map        = dn_sai_remove_tunnel_map;
    api_table->set_tunnel_map_attribute = dn_sai_set_tunnel_map_attr;
    api_table->get_tunnel_map_attribute = dn_sai_get_tunnel_map_attr;
}

void dn_sai_tunnel_map_entry_obj_api_fill (sai_tunnel_api_t *api_table)
{
    api_table->create_tunnel_map_entry        = dn_sai_create_tunnel_map_entry;
    api_table->remove_tunnel_map_entry        = dn_sai_remove_tunnel_map_entry;
    api_table->set_tunnel_map_entry_attribute = dn_sai_set_tunnel_map_entry_attr;
    api_table->get_tunnel_map_entry_attribute = dn_sai_get_tunnel_map_entry_attr;
}
