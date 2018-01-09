/*
 * Copyright (c) 2016 Dell Inc.
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

/*
 * filename: sai_bridge_db.cpp
 */

#include "std_mutex_lock.h"
#include "sai_map_utl.h"
#include <unordered_map>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "saibridge.h"
#include "sai_bridge_api.h"
#include "sai_bridge_common.h"


static std::unordered_map<sai_object_id_t, dn_sai_bridge_info_t> bridge_db;
static std::unordered_map<sai_object_id_t, dn_sai_bridge_port_info_t> bridge_port_db;

extern "C" {

sai_status_t sai_bridge_cache_write (sai_object_id_t bridge_id, const dn_sai_bridge_info_t *bridge_info)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    if(bridge_info == NULL) {
        SAI_BRIDGE_LOG_TRACE("Bridge info is NULL for bridge id 0x%" PRIx64 " in bridge cache write",
                             bridge_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    try {
        auto map_it = bridge_db.find (bridge_id);

        if (map_it != bridge_db.end()) {
            dn_sai_bridge_info_t &cached_bridge_info = map_it->second;
            cached_bridge_info = *bridge_info;
        }
        else {
            bridge_db.insert (std::make_pair (bridge_id, *bridge_info));
        }
    }
    catch (...) {
        SAI_BRIDGE_LOG_WARN("Error condition encountered in bridge cache write");
        rc = SAI_STATUS_FAILURE;
    }

    return (rc);
}

sai_status_t sai_bridge_cache_delete (sai_object_id_t bridge_id)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    try {
        auto map_it = bridge_db.find (bridge_id);
        if (map_it != bridge_db.end()) {
            bridge_db.erase (map_it);
        }
    }
    catch (...) {
        SAI_BRIDGE_LOG_WARN("Error condition encountered in bridge cache delete");
        rc = SAI_STATUS_FAILURE;
    }

    return rc;
}

sai_status_t sai_bridge_cache_read (sai_object_id_t bridge_id, dn_sai_bridge_info_t **bridge_info)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    if(bridge_info == NULL) {
        SAI_BRIDGE_LOG_TRACE("NULL *bridge_port_info passed in bridge cache read");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    try {
        auto map_it = bridge_db.find (bridge_id);
        if (map_it != bridge_db.end()) {
            *bridge_info =(dn_sai_bridge_info_t *)&map_it->second;
        }
        else {
        SAI_BRIDGE_LOG_WARN("Error condition encountered in bridge cache read");
            rc = SAI_STATUS_INVALID_OBJECT_ID;
        }
    }
    catch (...) {
        rc = SAI_STATUS_FAILURE;
    }

    return rc;
}

bool sai_is_bridge_created (sai_object_id_t bridge_id)
{

    auto map_it = bridge_db.find (bridge_id);

    try {
        if(map_it != bridge_db.end()) {
            return true;
        }
    }
    catch (...) {
        SAI_BRIDGE_LOG_WARN("Error condition encountered in is bridge created");
    }
    return false;
}

sai_status_t sai_bridge_port_cache_write (sai_object_id_t bridge_port_id,
                                          const dn_sai_bridge_port_info_t *bridge_port_info)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    if(bridge_port_info == NULL) {
        SAI_BRIDGE_LOG_TRACE("Bridge port info is NULL for bridge port id 0x%" PRIx64 ""
                             " in bridge port cache write", bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    try {
        rc = sai_bridge_map_insert(bridge_port_info->bridge_id,
                                   bridge_port_info->bridge_port_id);

        if(rc != SAI_STATUS_SUCCESS) {
            SAI_BRIDGE_LOG_ERR("Error %d in bridge to bridge port map insert", rc);
            return rc;
        }

        auto map_it = bridge_port_db.find (bridge_port_id);

        if (map_it != bridge_port_db.end()) {
            dn_sai_bridge_port_info_t &cached_bridge_port_info = map_it->second;
            cached_bridge_port_info = *bridge_port_info;
        }
        else {
            bridge_port_db.insert (std::make_pair (bridge_port_id, *bridge_port_info));
        }
    }
    catch (...) {
        SAI_BRIDGE_LOG_WARN("Error condition encountered in bridge port cache write");
        sai_bridge_map_remove(bridge_port_info->bridge_id, bridge_port_info->bridge_port_id);
        rc = SAI_STATUS_FAILURE;
    }

    return (rc);
}

sai_status_t sai_bridge_port_cache_delete (sai_object_id_t bridge_port_id)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    dn_sai_bridge_port_info_t cached_bridge_port_info;

    try {
        auto map_it = bridge_port_db.find (bridge_port_id);
        if (map_it != bridge_port_db.end()) {
            cached_bridge_port_info = map_it->second;
            rc = sai_bridge_map_remove(cached_bridge_port_info.bridge_id, bridge_port_id);
            if(rc != SAI_STATUS_SUCCESS) {
                SAI_BRIDGE_LOG_ERR("Error %d in bridge to bridge port map remove", rc);
                return rc;
            }
            bridge_port_db.erase (map_it);
        }
    }
    catch (...) {
        SAI_BRIDGE_LOG_WARN("Error condition encountered in bridge port cache delete");
        sai_bridge_map_insert(cached_bridge_port_info.bridge_id, bridge_port_id);
        rc = SAI_STATUS_FAILURE;
    }

    return rc;
}

sai_status_t sai_bridge_port_cache_read (sai_object_id_t bridge_port_id,
                                         dn_sai_bridge_port_info_t **bridge_port_info)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    if(bridge_port_info == NULL) {
        SAI_BRIDGE_LOG_TRACE("NULL *bridge_port_info passed in bridge port cache read");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    try {
        auto map_it = bridge_port_db.find (bridge_port_id);
        if (map_it != bridge_port_db.end()) {
            *bridge_port_info =(dn_sai_bridge_port_info_t *) &map_it->second;
        }
        else {
            rc = SAI_STATUS_INVALID_OBJECT_ID;
        }
    }
    catch (...) {
        SAI_BRIDGE_LOG_WARN("Error condition encountered in bridge port cache read");
        rc = SAI_STATUS_FAILURE;
    }

    return rc;
}

bool sai_is_bridge_port_created (sai_object_id_t bridge_port_id)
{

    auto map_it = bridge_port_db.find (bridge_port_id);

    try {
        if(map_it != bridge_port_db.end()) {
            return true;
        }
    }
    catch (...) {
        SAI_BRIDGE_LOG_WARN("Error condition encountered in is bridge port created");
    }
    return false;
}

uint_t sai_bridge_total_count(void)
{
    return bridge_db.size();
}

uint_t sai_bridge_port_total_count(void)
{
    return bridge_port_db.size();
}

sai_status_t sai_bridge_list_get(uint_t *count, sai_object_id_t *bridge_list)
{
    uint_t idx = 0;

    if((count == NULL) || (bridge_list == NULL)) {
        SAI_BRIDGE_LOG_TRACE("count is %p bridge list is %p in bridge list get",
                             count, bridge_list);
    }
    if(*count < sai_bridge_total_count()) {
        SAI_BRIDGE_LOG_ERR("Expected %d count but actual count is %d",
                           sai_bridge_total_count(),*count);
        return SAI_STATUS_BUFFER_OVERFLOW;
    }
    try {
        for(auto map_it = bridge_db.begin(); map_it != bridge_db.end();map_it++) {
            bridge_list[idx] = map_it->first;
            idx++;
        }
    }
    catch (...) {
        SAI_BRIDGE_LOG_WARN("Error condition encountered in is bridge list get");
        return SAI_STATUS_FAILURE;
    }
    *count = idx;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_list_get(uint_t *count, sai_object_id_t *bridge_port_list)
{
    uint_t idx = 0;

    if((count == NULL) || (bridge_port_list == NULL)) {
        SAI_BRIDGE_LOG_TRACE("count is %p bridge port list is %p in bridge port list get",
                             count, bridge_port_list);
    }
    if(*count < sai_bridge_port_total_count()) {
        SAI_BRIDGE_LOG_ERR("Expected %d count but actual count is %d",
                           sai_bridge_port_total_count(),*count);
        *count = sai_bridge_port_total_count();
        return SAI_STATUS_BUFFER_OVERFLOW;
    }
    try {
        for(auto map_it = bridge_port_db.begin(); map_it != bridge_port_db.end();map_it++) {
            bridge_port_list[idx] = map_it->first;
            idx++;
        }
    }
    catch (...) {
        SAI_BRIDGE_LOG_WARN("Error condition encountered in is bridge port list get");
        return SAI_STATUS_FAILURE;
    }
    *count = idx;
    return SAI_STATUS_SUCCESS;
}

}
