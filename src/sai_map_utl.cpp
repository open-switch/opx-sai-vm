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

/*
 * filename: sai_map_utl.cpp
 */

#include "std_mutex_lock.h"
#include "sai_map_utl.h"
#include <unordered_map>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

struct _sai_map_hash
{
    uint32_t operator()(const sai_map_key_t& key) const {
        uint32_t hash;

        hash  = std::hash<int>()(key.type);
        hash ^= std::hash<uint64_t>()(key.id1);
        hash ^= std::hash<uint64_t>()(key.id2);
        return (hash);
    }
};

struct _sai_map_equal
{
    bool operator()(const sai_map_key_t& key1, const sai_map_key_t& key2) const {
        if ((key1.type == key2.type) &&
            (key1.id1 == key2.id1) && (key1.id2 == key2.id2)) {
            return true;
        }
        else {
            return false;
        }
    }
};

static inline bool operator==(const sai_map_key_t& key1, const sai_map_key_t& key2)
{
    return _sai_map_equal()(key1, key2);
}

std_mutex_lock_create_static_init_fast (sai_map_mutex);

static std::unordered_map<sai_map_key_t, std::vector <sai_map_data_t>, _sai_map_hash, _sai_map_equal> g_sai_map;

static bool sai_map_apply_filter (sai_map_data_t       *arg1,
                                      sai_map_data_t       *arg2,
                                      sai_map_val_filter_t  filter)
{
    if ((filter & SAI_MAP_VAL_FILTER_NONE) == SAI_MAP_VAL_FILTER_NONE) {
        return true;
    }

    if ((filter & SAI_MAP_VAL_FILTER_VAL1) == SAI_MAP_VAL_FILTER_VAL1) {
        if (arg1->val1 != arg2->val1) {
            return false;
        }
    }

    if ((filter & SAI_MAP_VAL_FILTER_VAL2) == SAI_MAP_VAL_FILTER_VAL2) {
        if (arg1->val2 != arg2->val2) {
            return false;
        }
    }

    return true;
}

static void sai_map_copy_value (sai_map_data_t       *dst,
                                    sai_map_data_t       *src,
                                    sai_map_val_filter_t  filter)
{
    if ((filter & SAI_MAP_VAL_FILTER_NONE) == SAI_MAP_VAL_FILTER_NONE) {
        *dst = *src;
        return;
    }

    if ((filter & SAI_MAP_VAL_FILTER_VAL1) == SAI_MAP_VAL_FILTER_VAL1) {
        dst->val1 = src->val1;
    }

    if ((filter & SAI_MAP_VAL_FILTER_VAL2) == SAI_MAP_VAL_FILTER_VAL2) {
        dst->val2 = src->val2;
    }
}

extern "C" {

sai_status_t sai_map_insert (sai_map_key_t *key, sai_map_val_t *value)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    uint32_t     i;

    std_mutex_lock (&sai_map_mutex);

    try {
        auto map_it = g_sai_map.find (*key);

        if (map_it != g_sai_map.end()) {
            std::vector <sai_map_data_t>& list = map_it->second;

            for (i = 0; i < value->count; i++) {
                list.push_back (value->data[i]);
            }
        }
        else {
            std::vector <sai_map_data_t> new_list {};

            for (i = 0; i < value->count; i++) {
                new_list.push_back (value->data[i]);
            }
            g_sai_map.insert (std::make_pair (*key, new_list));
        }
    }
    catch (...) {
        rc = SAI_STATUS_FAILURE;
    }

    std_mutex_unlock (&sai_map_mutex);
    return (rc);
}

sai_status_t sai_map_delete (sai_map_key_t *key)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    std_mutex_lock (&sai_map_mutex);

    try {
        auto map_it = g_sai_map.find (*key);
        if (map_it != g_sai_map.end()) {
            map_it->second.clear();
            g_sai_map.erase (map_it);
        }
    }
    catch (...) {
        rc = SAI_STATUS_FAILURE;
    }

    std_mutex_unlock (&sai_map_mutex);
    return rc;
}

sai_status_t sai_map_delete_elements (sai_map_key_t        *key,
                                      sai_map_val_t        *value,
                                      sai_map_val_filter_t  filter)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    uint32_t     i;
    uint32_t     position;

    std_mutex_lock (&sai_map_mutex);

    try {
        auto map_it = g_sai_map.find (*key);
        if (map_it != g_sai_map.end()) {
            std::vector <sai_map_data_t>& list = map_it->second;

            for (i = 0; i < value->count; i++) {
                position = 0;
                for (auto data: list) {
                    if (sai_map_apply_filter (&value->data[i],
                                                  &data, filter)) {
                        list.erase (list.begin() + position);
                        break;
                    }
                    position++;
                }
            }
        }
    }
    catch (...) {
        rc = SAI_STATUS_FAILURE;
    }

    std_mutex_unlock (&sai_map_mutex);

    return rc;
}

sai_status_t sai_map_get (sai_map_key_t *key, sai_map_val_t *value)
{
    uint32_t     count;
    uint32_t     i;
    sai_status_t rc = SAI_STATUS_SUCCESS;

    std_mutex_lock (&sai_map_mutex);

    try {
        auto map_it = g_sai_map.find (*key);
        if (map_it != g_sai_map.end()) {
            std::vector <sai_map_data_t>& list = map_it->second;

            count = list.size();

            if (count > value->count) {
                /*
                 * The passed buffer is insufficient. So fill
                 * the count with the actual number of elements,
                 * so that the caller will call with sufficient
                 * memory again.
                 */
                rc = SAI_STATUS_BUFFER_OVERFLOW;
            }
            else {
                for (i = 0; i < count; i++) {
                    value->data[i] = list.at(i);
                }
            }

            value->count = count;
        }
        else {
            rc = SAI_STATUS_ITEM_NOT_FOUND;
        }
    }
    catch (...) {
        rc = SAI_STATUS_FAILURE;
    }

    std_mutex_unlock (&sai_map_mutex);
    return rc;
}

sai_status_t sai_map_get_element_at_index (const sai_map_key_t *key,
                                           uint32_t index,
                                           sai_map_val_t *value)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    if((value == NULL) || (key == NULL)) {
       return SAI_STATUS_INVALID_PARAMETER;
    }

    std_mutex_lock (&sai_map_mutex);

    try {
        auto map_it = g_sai_map.find (*key);
        if (map_it != g_sai_map.end()) {
            std::vector <sai_map_data_t>& list = map_it->second;

            if (index >= list.size()) {
                rc = SAI_STATUS_INVALID_PARAMETER;
            }
            else {
                value->data[0] = list.at(index);
            }

        }
        else {
            rc = SAI_STATUS_ITEM_NOT_FOUND;
        }
    }
    catch (...) {
        rc = SAI_STATUS_FAILURE;
    }

    std_mutex_unlock (&sai_map_mutex);
    return rc;
}

sai_status_t sai_map_get_elements (sai_map_key_t        *key,
                                   sai_map_val_t        *value,
                                   sai_map_val_filter_t  filter)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    uint32_t     i;

    std_mutex_lock (&sai_map_mutex);

    try {
        auto map_it = g_sai_map.find (*key);
        if (map_it != g_sai_map.end()) {
            std::vector <sai_map_data_t>& list = map_it->second;

            for (i = 0; i < value->count; i++) {
                for (auto data: list) {
                    if (sai_map_apply_filter (&value->data[i],
                                                  &data, filter)) {
                        sai_map_copy_value (&value->data[i],
                                                &data, filter);
                        break;
                    }
                }
            }
        }
    }
    catch (...) {
        rc = SAI_STATUS_FAILURE;
    }

    std_mutex_unlock (&sai_map_mutex);

    return rc;
}

sai_status_t sai_map_get_val_count (sai_map_key_t *key, uint32_t *p_out_count)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    std_mutex_lock (&sai_map_mutex);

    try {
        auto map_it = g_sai_map.find (*key);
        if (map_it != g_sai_map.end()) {
            std::vector <sai_map_data_t>& list = map_it->second;

            *p_out_count = list.size();
        }
        else {
            rc = SAI_STATUS_ITEM_NOT_FOUND;
        }
    }
    catch (...) {
        rc = SAI_STATUS_FAILURE;
    }

    std_mutex_unlock (&sai_map_mutex);
    return rc;
}
}
