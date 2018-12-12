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
* @file sai_gen_utils.c
*
* @brief This file contains utility APIs for SAI module
*************************************************************************/

#include <stdio.h>
#include "saistatus.h"
#include "sai.h"
#include "sai_gen_utils.h"
#include "sai_common_utils.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include "sai_port_utils.h"
#include "sai_switch_utils.h"

#define SAI_SWITCH_ID_STR_LEN 50

sai_log_level_t g_sai_api_log_level [SAI_NUM_API_ID];
sai_log_level_t g_sai_api_custom_log_level [SAI_NUM_API_CUSTOM_ID];

char g_sai_switch_id_str [SAI_SWITCH_ID_STR_LEN] = "INVALID";

static inline sai_status_t get_indexed_ret_val(sai_status_t ret_val,
                                            unsigned int attr_index,
                                            unsigned int min_val,
                                            unsigned int max_val)
{
    if(attr_index < (max_val - min_val)) {
        return (sai_status_t)(ret_val + SAI_STATUS_CODE(attr_index));
    }
    return SAI_STATUS_INVALID_PARAMETER;
}

sai_status_t sai_get_indexed_ret_val(sai_status_t ret_val,unsigned int attr_index)
{
    switch(ret_val) {
        case SAI_STATUS_INVALID_ATTRIBUTE_0:
            return get_indexed_ret_val(ret_val, attr_index,
                                       SAI_STATUS_INVALID_ATTRIBUTE_0,
                                       SAI_STATUS_INVALID_ATTRIBUTE_MAX);
        case SAI_STATUS_INVALID_ATTR_VALUE_0:
            return get_indexed_ret_val(ret_val, attr_index,
                                       SAI_STATUS_INVALID_ATTR_VALUE_0,
                                       SAI_STATUS_INVALID_ATTR_VALUE_MAX);
        case SAI_STATUS_ATTR_NOT_IMPLEMENTED_0:
            return get_indexed_ret_val(ret_val, attr_index,
                                       SAI_STATUS_ATTR_NOT_IMPLEMENTED_0,
                                       SAI_STATUS_ATTR_NOT_IMPLEMENTED_MAX);
        case SAI_STATUS_UNKNOWN_ATTRIBUTE_0:
            return get_indexed_ret_val(ret_val, attr_index,
                                       SAI_STATUS_UNKNOWN_ATTRIBUTE_0,
                                       SAI_STATUS_UNKNOWN_ATTRIBUTE_MAX);
        case SAI_STATUS_ATTR_NOT_SUPPORTED_0:
            return get_indexed_ret_val(ret_val, attr_index,
                                       SAI_STATUS_ATTR_NOT_SUPPORTED_0,
                                       SAI_STATUS_ATTR_NOT_SUPPORTED_MAX);
        default:
            break;
    }
    return ret_val;
}

sai_status_t sai_attribute_validate (uint_t attr_count,
                                     const sai_attribute_t *p_attr_list,
                                     const dn_sai_attribute_entry_t *p_v_attr,
                                     dn_sai_operations_t op_type,
                                     uint_t max_vendor_attr_count)
{
    sai_status_t           sai_rc = SAI_STATUS_SUCCESS;
    size_t                 vendor_list_index = 0;
    size_t                 list_index = 0;
    const sai_attribute_t *p_attr = NULL;
    const dn_sai_attribute_entry_t *p_vendor_attr = NULL;
    uint_t                 total_mandatory_attribs = 0;
    uint_t                 create_mandatory_attribs = 0;

    if(op_type != SAI_OP_CREATE){
        /*
         * For set and get both count and list should not be 0.
         */
        if((attr_count == 0) || (p_attr_list == NULL)){
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    else {
        /*
         * For create case both the attr_count and p_attr_list can be 0.
         * So testing the other two cases here.
         */
        if ((attr_count != 0) && (p_attr_list == NULL)) {
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    if(attr_count  > max_vendor_attr_count){
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (vendor_list_index = 0, p_vendor_attr = p_v_attr;
         vendor_list_index < max_vendor_attr_count;
         ++vendor_list_index, ++p_vendor_attr) {

        if(p_vendor_attr->mandatory_on_create)
            total_mandatory_attribs ++;
    }

    for (list_index = 0, p_attr = p_attr_list;
         list_index < attr_count; ++list_index, ++p_attr) {

        for(vendor_list_index = 0, p_vendor_attr = p_v_attr;
            vendor_list_index < max_vendor_attr_count;
            ++vendor_list_index, ++p_vendor_attr) {

            sai_rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;

            if (p_vendor_attr->id == p_attr->id) {

                if (! p_vendor_attr->is_implemented) {
                    sai_rc = SAI_STATUS_ATTR_NOT_IMPLEMENTED_0;
                    break;
                } else if (! p_vendor_attr->is_supported){
                    sai_rc = SAI_STATUS_ATTR_NOT_SUPPORTED_0;
                    break;
                }

                if (op_type == SAI_OP_CREATE) {
                    if (p_vendor_attr->valid_for_create) {

                        if (p_vendor_attr->mandatory_on_create)
                            create_mandatory_attribs++;

                        sai_rc = SAI_STATUS_SUCCESS;
                        break;
                    }
                } else if (op_type == SAI_OP_SET) {
                    if (p_vendor_attr->valid_for_set) {
                        sai_rc = SAI_STATUS_SUCCESS;
                        break;
                    }
                } else if (op_type == SAI_OP_GET) {
                    if (p_vendor_attr->valid_for_get) {
                        sai_rc = SAI_STATUS_SUCCESS;
                        break;
                    }
                }
                sai_rc = SAI_STATUS_INVALID_ATTRIBUTE_0;
                break;
            }
        }

        if(sai_rc != SAI_STATUS_SUCCESS)
            return sai_get_indexed_ret_val(sai_rc, list_index);
    }

    if((op_type == SAI_OP_CREATE) &&
       (create_mandatory_attribs != total_mandatory_attribs))
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;

    return sai_rc;
}

int sai_port_node_compare(const void *current,
                                     const void *node,
                                     uint_t len)
{
    sai_object_id_t *src_port = (sai_object_id_t *)current;
    sai_object_id_t *dst_port = (sai_object_id_t *)node;

    STD_ASSERT(src_port != NULL);
    STD_ASSERT(dst_port != NULL);

    if ((*src_port) > (*dst_port)) {
        return 1;
    } else if ((*src_port) < (*dst_port)) {
        return -1;
    }
    return 0;
}

bool sai_is_port_list_valid(const sai_object_list_t *sai_port_list, unsigned int *invalid_idx)
{
    unsigned int num_port = 0;

    STD_ASSERT(sai_port_list != NULL);
    if(sai_port_list->count == 0) {
        return false;
    }
    for(num_port = 0; num_port < sai_port_list->count; num_port++) {
        if(!sai_is_port_valid(sai_port_list->list[num_port])){
            *invalid_idx = num_port;
            return false;
        }
    }
    return true;
}

char* sai_switch_id_strify (void)
{
    snprintf (g_sai_switch_id_str, SAI_SWITCH_ID_STR_LEN, "Switch Id: %u",
              sai_switch_id_get());

    return g_sai_switch_id_str;
}

const char* sai_switch_id_str_get (void)
{
    return ((const char *) g_sai_switch_id_str);
}

static inline sai_log_level_t sai_log_level_get (sai_api_t api_id)
{
    return ((api_id < SAI_NUM_API_ID) ? g_sai_api_log_level [api_id] :
    SAI_API_CUSTOM_CHECK(api_id)?g_sai_api_custom_log_level[SAI_API_CUSTOM_INDEX(api_id)]:SAI_LOG_LEVEL_WARN);
}

void sai_log_level_set (sai_api_t api_id, sai_log_level_t level)
{
    if (api_id < SAI_NUM_API_ID) {
        g_sai_api_log_level [api_id] = level;
    } else if (SAI_API_CUSTOM_CHECK(api_id)) {
        g_sai_api_custom_log_level [SAI_API_CUSTOM_INDEX(api_id)] = level;
    }

}

bool sai_is_log_enabled (sai_api_t api_id, sai_log_level_t level)
{
    return (level >= sai_log_level_get (api_id));
}

void sai_log_init (void)
{
    uint_t api_id;

    sai_switch_id_strify ();

    /* Set the log level to LOG_WARN for all SAI API modules */
    for (api_id = 0; api_id < SAI_NUM_API_ID; api_id++)
    {
        sai_log_level_set (api_id, SAI_LOG_LEVEL_WARN);
    }
    for (api_id = SAI_API_CUSTOM_RANGE_START;
            api_id < SAI_API_CUSTOM_RANGE_END; api_id++)
    {
        sai_log_level_set (api_id, SAI_LOG_LEVEL_WARN);
    }
}

sai_status_t sai_find_attr_in_attrlist(sai_attr_id_t attr_id,
                                       uint_t attr_count,
                                       const sai_attribute_t *attr_list,
                                       uint_t *attr_index,
                                       const sai_attribute_value_t **attr_val)
{
    uint_t index = 0;

    if ((NULL == attr_val) || (NULL == attr_list) || (NULL == attr_index)) {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (index = 0; index < attr_count; index++) {
        if (attr_list[index].id == attr_id) {
            *attr_val = &(attr_list[index].value);
            *attr_index = index;
            return SAI_STATUS_SUCCESS;
        }
    }

    return SAI_STATUS_ITEM_NOT_FOUND;
}

bool dn_sai_check_duplicate_attr(uint32_t attr_count, const sai_attribute_t *attr_list,
                                 uint_t *dup_index)
{
    uint_t index = 0, next_index = 0;

    if ((NULL == attr_list) || (dup_index == NULL)) {
        return false;
    }

    for(index = 0; index < attr_count; index++) {
        for(next_index = index+1; next_index < attr_count; next_index++) {
            if (attr_list[index].id == attr_list[next_index].id) {
                *dup_index = next_index;
                return true;
            }
        }
    }
    return false;
}

sai_status_t dn_sai_get_next_free_id(dn_sai_id_gen_info_t *info)
{
    uint64_t tmp_id = 0;

    info->cur_id = (info->cur_id + 1) & info->mask;

    if(!info->cur_id) {
        info->is_wrappped = true;
    }

    if(info->is_wrappped) {
        tmp_id = info->cur_id;

        while(info->is_id_in_use(info->cur_id)) {
            info->cur_id = (info->cur_id + 1) & info->mask;
            /* Exit criteria: ID search has wrapped around,
               no free ID to be allocated */
            if(tmp_id == info->cur_id)
                return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}
