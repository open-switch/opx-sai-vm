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
* @file  sai_qos_wred.c
*
* @brief This file contains function definitions for SAI Wred
*        functionality API implementation.
*
*************************************************************************/
#include "sai_npu_qos.h"
#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_api_utils.h"
#include "sai_gen_utils.h"
#include "sai_qos_mem.h"
#include "sai_common_infra.h"
#include "sai_qos_buffer_util.h"
#include "sai_qos_port_util.h"

#include "sai.h"
#include "saiwred.h"

#include "std_type_defs.h"
#include "std_utils.h"
#include "std_assert.h"
#include <stdlib.h>
#include <inttypes.h>

void sai_qos_wred_free_resources(dn_sai_qos_wred_t *p_wred_node)
{
    SAI_WRED_LOG_TRACE("Freeing the wred node");
    sai_qos_wred_node_free(p_wred_node);
}

static bool sai_qos_wred_is_object_in_use(dn_sai_qos_wred_t *p_wred_node)
{
    dn_sai_qos_wred_link_t wred_link_type = DN_SAI_QOS_WRED_LINK_QUEUE;
    std_dll_head *p_dll_head = NULL;

    STD_ASSERT(p_wred_node != NULL);

    for(wred_link_type = DN_SAI_QOS_WRED_LINK_QUEUE;
                wred_link_type < DN_SAI_QOS_WRED_LINK_MAX;
                wred_link_type++) {
        p_dll_head = sai_qos_wred_link_get_head_ptr(p_wred_node, wred_link_type);

        if((p_dll_head != NULL) && (std_dll_getfirst(p_dll_head) != NULL)) {
            return true ;
        }
    }

    return false;
}

static sai_packet_color_t sai_qos_wred_attr_to_color(uint32_t attr_id)
{
    if(attr_id == SAI_WRED_ATTR_GREEN_ENABLE ||
       attr_id == SAI_WRED_ATTR_GREEN_MIN_THRESHOLD ||
       attr_id == SAI_WRED_ATTR_GREEN_MAX_THRESHOLD ||
       attr_id == SAI_WRED_ATTR_GREEN_DROP_PROBABILITY)
    {
        return SAI_PACKET_COLOR_GREEN;
    }

    if(attr_id == SAI_WRED_ATTR_RED_ENABLE ||
       attr_id == SAI_WRED_ATTR_RED_MIN_THRESHOLD ||
       attr_id == SAI_WRED_ATTR_RED_MAX_THRESHOLD ||
       attr_id == SAI_WRED_ATTR_RED_DROP_PROBABILITY)
    {
        return SAI_PACKET_COLOR_RED;
    }
    if(attr_id == SAI_WRED_ATTR_YELLOW_ENABLE ||
       attr_id == SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD ||
       attr_id == SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD ||
       attr_id == SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY)
    {
        return SAI_PACKET_COLOR_YELLOW;
    }
    return SAI_QOS_MAX_PACKET_COLORS;
}

static sai_status_t sai_qos_ecn_mode_set(dn_sai_qos_wred_t *p_wred_node)
{
    switch (p_wred_node->ecn_mark_mode) {
        case SAI_ECN_MARK_MODE_NONE:
            p_wred_node->threshold[SAI_PACKET_COLOR_GREEN].ecn_enable = false;
            p_wred_node->threshold[SAI_PACKET_COLOR_YELLOW].ecn_enable = false;
            p_wred_node->threshold[SAI_PACKET_COLOR_RED].ecn_enable = false;
            break;
        case SAI_ECN_MARK_MODE_GREEN:
            p_wred_node->threshold[SAI_PACKET_COLOR_GREEN].ecn_enable = true;
            p_wred_node->threshold[SAI_PACKET_COLOR_YELLOW].ecn_enable = false;
            p_wred_node->threshold[SAI_PACKET_COLOR_RED].ecn_enable = false;
            break;
        case SAI_ECN_MARK_MODE_YELLOW:
            p_wred_node->threshold[SAI_PACKET_COLOR_GREEN].ecn_enable = false;
            p_wred_node->threshold[SAI_PACKET_COLOR_YELLOW].ecn_enable = true;
            p_wred_node->threshold[SAI_PACKET_COLOR_RED].ecn_enable = false;
            break;
        case SAI_ECN_MARK_MODE_RED:
            p_wred_node->threshold[SAI_PACKET_COLOR_GREEN].ecn_enable = false;
            p_wred_node->threshold[SAI_PACKET_COLOR_YELLOW].ecn_enable = false;
            p_wred_node->threshold[SAI_PACKET_COLOR_RED].ecn_enable = true;
            break;
        case SAI_ECN_MARK_MODE_GREEN_YELLOW:
            p_wred_node->threshold[SAI_PACKET_COLOR_GREEN].ecn_enable = true;
            p_wred_node->threshold[SAI_PACKET_COLOR_YELLOW].ecn_enable = true;
            p_wred_node->threshold[SAI_PACKET_COLOR_RED].ecn_enable = false;
            break;
        case SAI_ECN_MARK_MODE_GREEN_RED:
            p_wred_node->threshold[SAI_PACKET_COLOR_GREEN].ecn_enable = true;
            p_wred_node->threshold[SAI_PACKET_COLOR_YELLOW].ecn_enable = false;
            p_wred_node->threshold[SAI_PACKET_COLOR_RED].ecn_enable = true;
            break;
        case SAI_ECN_MARK_MODE_YELLOW_RED:
            p_wred_node->threshold[SAI_PACKET_COLOR_GREEN].ecn_enable = false;
            p_wred_node->threshold[SAI_PACKET_COLOR_YELLOW].ecn_enable = true;
            p_wred_node->threshold[SAI_PACKET_COLOR_RED].ecn_enable = true;
            break;
        case SAI_ECN_MARK_MODE_ALL:
            p_wred_node->threshold[SAI_PACKET_COLOR_GREEN].ecn_enable = true;
            p_wred_node->threshold[SAI_PACKET_COLOR_YELLOW].ecn_enable = true;
            p_wred_node->threshold[SAI_PACKET_COLOR_RED].ecn_enable = true;
            break;
        default:
            SAI_WRED_LOG_ERR ("Invalid value %d provided for ECN Mode", p_wred_node->ecn_mark_mode);
            return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_qos_wred_attr_set(dn_sai_qos_wred_t *p_wred_node,
                                      const sai_attribute_t *p_attr)
{
    sai_packet_color_t color = SAI_QOS_MAX_PACKET_COLORS;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_wred_node != NULL);

    STD_ASSERT(p_attr != NULL);

    SAI_WRED_LOG_TRACE ("Setting Wred attribute id: %d.",
                        p_attr->id);

    switch (p_attr->id)
    {
        case SAI_WRED_ATTR_GREEN_ENABLE:
        case SAI_WRED_ATTR_YELLOW_ENABLE:
        case SAI_WRED_ATTR_RED_ENABLE:
            color = sai_qos_wred_attr_to_color(p_attr->id);
            p_wred_node->threshold[color].enable = p_attr->value.booldata;
            break;

        case SAI_WRED_ATTR_GREEN_MIN_THRESHOLD:
        case SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD:
        case SAI_WRED_ATTR_RED_MIN_THRESHOLD:
            if(p_attr->value.u32 <= SAI_WRED_MAX_BUFFER_LIMIT) {
                color = sai_qos_wred_attr_to_color(p_attr->id);
                p_wred_node->threshold[color].min_limit = p_attr->value.u32;
            } else {
                sai_rc = SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
            break;


        case SAI_WRED_ATTR_GREEN_MAX_THRESHOLD:
        case SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD:
        case SAI_WRED_ATTR_RED_MAX_THRESHOLD:
            if(p_attr->value.u32 <= SAI_WRED_MAX_BUFFER_LIMIT) {
                color = sai_qos_wred_attr_to_color(p_attr->id);
                p_wred_node->threshold[color].max_limit = p_attr->value.u32;
            } else {
                sai_rc = SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
            break;

        case SAI_WRED_ATTR_GREEN_DROP_PROBABILITY:
        case SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY:
        case SAI_WRED_ATTR_RED_DROP_PROBABILITY:
            if(p_attr->value.u32 <= SAI_WRED_MAX_DROP_PROBABILITY) {
                color = sai_qos_wred_attr_to_color(p_attr->id);
                p_wred_node->threshold[color].drop_probability =
                    p_attr->value.u32;
            } else {
                sai_rc = SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
            break;

        case SAI_WRED_ATTR_WEIGHT:
            if(p_attr->value.u8 <= SAI_WRED_MAX_WEIGHT) {
                p_wred_node->weight = p_attr->value.u8;
            } else {
                sai_rc = SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
            break;

        case SAI_WRED_ATTR_ECN_MARK_MODE:
            if((sai_rc = sai_qos_ecn_mode_set(p_wred_node)) == SAI_STATUS_SUCCESS) {
                p_wred_node->ecn_mark_mode = p_attr->value.s32;
            }
            break;

        default:
            SAI_WRED_LOG_ERR ("Attribute id: %d is an unknown attribute for WRED.", p_attr->id);

            sai_rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            break;
    }

    return sai_rc;
}

static void sai_qos_wred_fill_in_defaults(dn_sai_qos_wred_t *p_wred_node)
{
    STD_ASSERT(p_wred_node != NULL);
    size_t color = 0;

    for(color = 0; color < SAI_QOS_MAX_PACKET_COLORS; color ++)
    {
        p_wred_node->threshold[color].max_limit = SAI_WRED_MAX_BUFFER_LIMIT;
        p_wred_node->threshold[color].min_limit = SAI_WRED_MAX_BUFFER_LIMIT;
        p_wred_node->threshold[color].drop_probability = SAI_WRED_MAX_DROP_PROBABILITY;
    }
}

static sai_status_t sai_wred_parse_update_attributes(dn_sai_qos_wred_t *p_wred_node,
                                                     uint32_t attr_count,
                                                     const sai_attribute_t *attr_list,
                                                     dn_sai_operations_t op_type)
{
    sai_status_t                    sai_rc = SAI_STATUS_SUCCESS;
    uint_t                          list_idx = 0;
    uint_t                          max_vendor_attr_count = 0;
    const dn_sai_attribute_entry_t  *p_vendor_attr = NULL;

    STD_ASSERT(p_wred_node != NULL);

    sai_qos_wred_npu_api_get()->attribute_table_get
        (&p_vendor_attr, &max_vendor_attr_count);

    STD_ASSERT(p_vendor_attr != NULL);
    STD_ASSERT(max_vendor_attr_count != 0);

    sai_rc = sai_attribute_validate(attr_count,attr_list, p_vendor_attr,
                                    op_type, max_vendor_attr_count);

    if(sai_rc != SAI_STATUS_SUCCESS){
        SAI_WRED_LOG_ERR("Attribute validation failed for %d operation",
                            op_type);
        return sai_rc;
    }

    if(op_type != SAI_OP_GET) {
        for (list_idx = 0; list_idx < attr_count; list_idx++)
        {
            sai_rc = sai_qos_wred_attr_set(p_wred_node, &attr_list[list_idx]);
            if(sai_rc != SAI_STATUS_SUCCESS) {
                sai_rc = sai_get_indexed_ret_val(sai_rc, list_idx);
                return sai_rc;
            }
        }
    }

    return sai_rc;
}

static sai_status_t sai_qos_wred_create(sai_object_id_t *wred_id,
                                    sai_object_id_t switch_id,
                                    uint32_t attr_count,
                                    const sai_attribute_t *attr_list)
{
    sai_status_t           sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_wred_t      *p_wred_node = NULL;
    sai_npu_object_id_t    hw_wred_id = 0;

    STD_ASSERT(wred_id != NULL);

    sai_qos_lock();

    do
    {
        if((p_wred_node = sai_qos_wred_node_alloc()) == NULL){
            SAI_WRED_LOG_ERR("Failed to allocate memory for wred node.");
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        sai_qos_wred_fill_in_defaults(p_wred_node);

        sai_rc = sai_wred_parse_update_attributes(p_wred_node, attr_count,
                                                  attr_list, SAI_OP_CREATE);
        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_WRED_LOG_ERR("Failed to parse attributes for wred, Error: %d", sai_rc);
            break;
        }

        sai_rc = sai_qos_wred_npu_api_get()->wred_create(p_wred_node,attr_count,
                                                         attr_list, &hw_wred_id);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_WRED_LOG_ERR("Npu wred create failed for wred");
            break;
        }

        SAI_WRED_LOG_TRACE("Wred created in NPU 0x%"PRIx64"",hw_wred_id);

        *wred_id = sai_uoid_create(SAI_OBJECT_TYPE_WRED,
                                  hw_wred_id);

        SAI_WRED_LOG_TRACE("Wred object id is 0x%"PRIx64"",*wred_id);

        p_wred_node->key.wred_id = *wred_id;

        std_dll_init(&p_wred_node->queue_dll_head);
        std_dll_init(&p_wred_node->port_dll_head);
        std_dll_init(&p_wred_node->buffer_pool_dll_head);

        if (sai_qos_wred_node_insert(p_wred_node) != STD_ERR_OK){
            SAI_WRED_LOG_ERR("Wred id insertion failed in RB tree");
            sai_qos_wred_npu_api_get()->wred_remove(p_wred_node);
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

    }while(0);

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_WRED_LOG_TRACE("Wred created successfully");
    }
    else{
        SAI_WRED_LOG_ERR("Wred create failed");
        sai_qos_wred_free_resources(p_wred_node);
    }

    sai_qos_unlock();
    return sai_rc;
}

static sai_status_t sai_qos_wred_remove(sai_object_id_t wred_id)
{
    dn_sai_qos_wred_t  *p_wred_node = NULL;
    sai_status_t      sai_rc = SAI_STATUS_SUCCESS;

    if(!sai_is_obj_id_wred(wred_id)) {
        SAI_WRED_LOG_ERR("Passed object is not wred object");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    SAI_WRED_LOG_TRACE("Removing wred id 0x%"PRIx64"",wred_id);
    sai_qos_lock();
    do
    {
        p_wred_node = sai_qos_wred_node_get(wred_id);

        if(!p_wred_node){
            SAI_WRED_LOG_ERR("Wred_id 0x%"PRIx64" not present in tree",
                             wred_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if(sai_qos_wred_is_object_in_use(p_wred_node)){
            SAI_WRED_LOG_WARN("Wred_id 0x%"PRIx64" is in use",
                              wred_id);
            sai_rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }
        sai_rc = sai_qos_wred_npu_api_get()->wred_remove(p_wred_node);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_WRED_LOG_ERR("Npu wred remove failed for wredid 0x%"PRIx64"",
                             wred_id);
            break;
        }

        sai_qos_wred_node_remove(wred_id);
        sai_qos_wred_free_resources(p_wred_node);
    }while(0);

    sai_qos_unlock();
    return sai_rc;
}

static void sai_wred_link_error_recovery(dn_sai_qos_wred_t *p_wred_old,
        uint_t count, dn_sai_qos_wred_link_t wred_link_type)
{
    void *p_wred_link_node = NULL;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t wred_link_id = SAI_NULL_OBJECT_ID;

    STD_ASSERT(p_wred_old != NULL);

    if(0 == count)
    {
        return;
    }

    p_wred_link_node = sai_qos_wred_link_node_get_first(p_wred_old, wred_link_type);

    while((p_wred_link_node != NULL) && (count > 0)){
        wred_link_id = sai_qos_wred_link_oid_get(p_wred_link_node,wred_link_type);
        SAI_WRED_LOG_TRACE("Recovering WRED ID 0x%"PRIx64" for %s 0x%"PRIx64"",
                p_wred_old->key.wred_id,sai_qos_wred_link_str(wred_link_type),wred_link_id);
        sai_rc = sai_qos_wred_npu_api_get()->wred_link_set(wred_link_id, p_wred_old, wred_link_type);

        if((SAI_STATUS_SUCCESS != sai_rc) && (SAI_STATUS_NOT_EXECUTED != sai_rc)) {
            SAI_WRED_LOG_ERR("Recovering WRED ID 0x%"PRIx64" for %s 0x%"PRIx64" failed, sai_rc 0x%x.",
                    p_wred_old->key.wred_id,sai_qos_wred_link_str(wred_link_type),wred_link_id,sai_rc);
            break;
        }
        sai_rc = SAI_STATUS_SUCCESS;

        p_wred_link_node = sai_qos_wred_link_node_get_next(p_wred_old, p_wred_link_node, wred_link_type);
        count --;
    }
}

static sai_status_t sai_wred_update_link(dn_sai_qos_wred_t *p_wred_new)
{
    void                    *p_wred_link_node = NULL;
    dn_sai_qos_wred_t       *p_wred_node_old = NULL;
    sai_status_t            sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_wred_link_t  wred_link_type = DN_SAI_QOS_WRED_LINK_QUEUE;
    sai_object_id_t         wred_link_id = SAI_NULL_OBJECT_ID;
    uint_t                  count = 0;

    STD_ASSERT(p_wred_new != NULL);

    p_wred_node_old = sai_qos_wred_node_get(p_wred_new->key.wred_id);

    if(p_wred_node_old == NULL){
        SAI_WRED_LOG_ERR("WRED node not found for WRED ID 0x%"PRIx64"", p_wred_new->key.wred_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for(wred_link_type = DN_SAI_QOS_WRED_LINK_QUEUE;
            wred_link_type < DN_SAI_QOS_WRED_LINK_MAX;
            wred_link_type++) {
        p_wred_link_node = sai_qos_wred_link_node_get_first(p_wred_node_old, wred_link_type);

        while(p_wred_link_node != NULL){
            wred_link_id = sai_qos_wred_link_oid_get(p_wred_link_node,wred_link_type);
            SAI_WRED_LOG_TRACE("Updating WRED ID to 0x%"PRIx64" for %s 0x%"PRIx64"",
                    p_wred_new->key.wred_id,
                    sai_qos_wred_link_str(wred_link_type),
                    wred_link_id);
            sai_rc =
                sai_qos_wred_npu_api_get()->wred_link_set(wred_link_id, p_wred_new, wred_link_type);

            if((SAI_STATUS_SUCCESS != sai_rc) && (SAI_STATUS_NOT_EXECUTED != sai_rc)) {
                SAI_WRED_LOG_TRACE("Updating WRED ID to 0x%"PRIx64" for %s 0x%"PRIx64" failed",
                        p_wred_new->key.wred_id,
                        sai_qos_wred_link_str(wred_link_type),
                        wred_link_id);
                sai_wred_link_error_recovery(p_wred_node_old, count, wred_link_type);
                break;
            }
            sai_rc = SAI_STATUS_SUCCESS;

            p_wred_link_node =
                sai_qos_wred_link_node_get_next(p_wred_node_old,p_wred_link_node,wred_link_type);
            count ++;
        }
    }

    return sai_rc;
}

static sai_status_t sai_qos_wred_attribute_set(sai_object_id_t wred_id,
                                              const sai_attribute_t *p_attr)
{
    dn_sai_qos_wred_t   wred_new_node;
    dn_sai_qos_wred_t   *p_wred_exist_node = NULL;
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    uint_t              attr_count = 1;

    STD_ASSERT (p_attr != NULL);

    SAI_WRED_LOG_TRACE("Setting attribute Id: %d on Wred Id 0x%"PRIx64"",
                          p_attr->id, wred_id);

    if(!sai_is_obj_id_wred(wred_id)) {
        SAI_WRED_LOG_ERR("Passed object is not wred object");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }
    sai_qos_lock();

    do
    {
        p_wred_exist_node = sai_qos_wred_node_get(wred_id);

        if(p_wred_exist_node == NULL){
            SAI_WRED_LOG_ERR("Wred node not found");
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        memset(&wred_new_node, 0, sizeof(dn_sai_qos_wred_t));
        wred_new_node.key.wred_id = wred_id;

        memcpy(&wred_new_node, p_wred_exist_node, sizeof(dn_sai_qos_wred_t));

        sai_rc = sai_wred_parse_update_attributes(&wred_new_node,
                                                     attr_count,
                                                     p_attr, SAI_OP_SET);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_WRED_LOG_ERR("Validation failed for attribute Id %d sai_rc:%d",
                                p_attr->id,sai_rc);
            break;
        }

        sai_rc = sai_qos_wred_npu_api_get()->wred_attr_set(&wred_new_node, attr_count, p_attr);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_WRED_LOG_ERR("NPU set failed for attribute Id %d"
                                "on wredid 0x%"PRIx64"",p_attr->id, wred_id);
            break;
        }
        SAI_WRED_LOG_TRACE("NPU attribute set success for id %d"
                              "for wred 0x%"PRIx64"", p_attr->id, wred_id);

        if(!sai_qos_wred_npu_api_get()->wred_is_hw_object()){
            sai_rc = sai_wred_update_link(&wred_new_node);
            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_WRED_LOG_ERR("Error: Unable to update queue list");
                break;
            }
        }

        memcpy(p_wred_exist_node, &wred_new_node, sizeof(dn_sai_qos_wred_t));
    }while(0);

    sai_qos_unlock();

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_WRED_LOG_INFO("Set attribute success for wred 0x%"PRIx64" for attr %d",
                              p_wred_exist_node->key.wred_id, p_attr->id);
    }

    return sai_rc;
}

static sai_status_t sai_qos_wred_attribute_get(sai_object_id_t wred_id,
                                       uint32_t attr_count,
                                       sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_qos_wred_t  *p_wred_node = NULL;

    STD_ASSERT(attr_list != NULL);

    SAI_WRED_LOG_TRACE("Get wred attributes for id 0x%"PRIx64"",wred_id);

    if ((attr_count == 0) || (attr_count > SAI_QOS_WRED_MAX_ATTR_COUNT)){
        SAI_WRED_LOG_ERR("Invalid number of attributes for qos wred");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(!sai_is_obj_id_wred(wred_id)) {
        SAI_WRED_LOG_ERR("Passed object is not wred object");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }
    sai_qos_lock();

    do
    {
        p_wred_node = sai_qos_wred_node_get(wred_id);

        if(p_wred_node == NULL){
            SAI_WRED_LOG_ERR("Passed wred node is not found in tree");
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_rc = sai_wred_parse_update_attributes(p_wred_node, attr_count,
                                                  attr_list, SAI_OP_GET);

        if (sai_rc != SAI_STATUS_SUCCESS){
            SAI_WRED_LOG_ERR("Validation failed for attribute");
            break;
        }

        sai_rc = sai_qos_wred_npu_api_get()->wred_attr_get(p_wred_node,
                                                      attr_count, attr_list);

        if (sai_rc != SAI_STATUS_SUCCESS){
            SAI_WRED_LOG_ERR("Npu get failed for wredid 0x%"PRIx64"",
                                wred_id);
            break;
        }
    } while (0);

    sai_qos_unlock();

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_WRED_LOG_TRACE("Get attribute success for wred id 0x%"PRIx64"",
                           wred_id);
    }
    else
    {
        SAI_WRED_LOG_ERR("Get attribute failed for wred id 0x%"PRIx64"",
                           wred_id);
    }
    return sai_rc;
}

static void sai_wred_link_node_add(sai_object_id_t wred_link_id,
        dn_sai_qos_wred_t *p_wred_node, dn_sai_qos_wred_link_t wred_link_type)
{
    sai_object_id_t wred_id_old = SAI_NULL_OBJECT_ID;
    sai_status_t sai_rc = SAI_STATUS_FAILURE;

    STD_ASSERT(p_wred_node != NULL);

    SAI_WRED_LOG_TRACE("Add WRED ID 0x%"PRIx64" to %s 0x%"PRIx64".",
            p_wred_node->key.wred_id,
            sai_qos_wred_link_str(wred_link_type),
            wred_link_id);

    wred_id_old = sai_qos_wred_link_wred_id_get(wred_link_id, wred_link_type);

    if(wred_id_old != SAI_NULL_OBJECT_ID){
        SAI_WRED_LOG_TRACE("Removing existing WRED ID 0x%"PRIx64" for %s 0x%"PRIx64".",
                wred_id_old, sai_qos_wred_link_str(wred_link_type), wred_link_id);

        if((sai_rc = sai_qos_wred_link_remove(wred_id_old,
                        wred_link_id,
                        wred_link_type)) != SAI_STATUS_SUCCESS) {
            SAI_WRED_LOG_ERR("Removing existing WRED ID 0x%"PRIx64" for %s 0x%"PRIx64" failed, sai_rc %d.",
                    wred_id_old, sai_qos_wred_link_str(wred_link_type), wred_link_id, sai_rc);
        }
    }

    if((sai_rc = sai_qos_wred_link_insert(p_wred_node->key.wred_id,
                    wred_link_id,
                    wred_link_type)) != SAI_STATUS_SUCCESS) {
        SAI_WRED_LOG_ERR("Insert WRED ID 0x%"PRIx64" for %s 0x%"PRIx64" failed, sai_rc %d.",
                p_wred_node->key.wred_id, sai_qos_wred_link_str(wred_link_type), wred_link_id, sai_rc);
    }
}

static void sai_wred_link_node_delete(sai_object_id_t wred_link_id,
        dn_sai_qos_wred_t *p_wred_node, dn_sai_qos_wred_link_t wred_link_type)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;

    STD_ASSERT(p_wred_node != NULL);

    SAI_WRED_LOG_TRACE("Remove WRED ID 0x%"PRIx64" from %s 0x%"PRIx64".",
            p_wred_node->key.wred_id,
            sai_qos_wred_link_str(wred_link_type),
            wred_link_id);

    if((sai_rc = sai_qos_wred_link_remove(p_wred_node->key.wred_id,
                    wred_link_id,
                    wred_link_type)) != SAI_STATUS_SUCCESS) {
        SAI_WRED_LOG_ERR("Remove WRED 0x%"PRIx64" on %s 0x%"PRIx64" failed, sai_rc 0x%x.",
                p_wred_node->key.wred_id, sai_qos_wred_link_str(wred_link_type), wred_link_id, sai_rc);
    }
}

sai_status_t sai_qos_wred_link_set(sai_object_id_t wred_link_id,
        sai_object_id_t wred_id, dn_sai_qos_wred_link_t wred_link_type)
{
    dn_sai_qos_wred_t *p_wred_node = NULL;
    sai_status_t sai_rc =  SAI_STATUS_SUCCESS;

    SAI_WRED_LOG_TRACE("WRED ID 0x%"PRIx64" SET for %s 0x%"PRIx64".",
            wred_id, sai_qos_wred_link_str(wred_link_type), wred_link_id);

    if((wred_id != SAI_NULL_OBJECT_ID) && (!sai_is_obj_id_wred(wred_id))) {
        SAI_WRED_LOG_ERR("SAI object 0x%"PRIx64" is not of type WRED.", wred_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if(wred_id != SAI_NULL_OBJECT_ID){
        /* WRED ID set */
        p_wred_node = sai_qos_wred_node_get(wred_id);
        if(NULL == p_wred_node) {
            SAI_WRED_LOG_ERR("WRED ID 0x%"PRIx64" not found in DB",wred_id);
            return SAI_STATUS_INVALID_OBJECT_ID;
        }

        sai_rc = sai_qos_wred_npu_api_get()->wred_link_set(wred_link_id, p_wred_node, wred_link_type);
        if((SAI_STATUS_SUCCESS == sai_rc) || (SAI_STATUS_NOT_EXECUTED == sai_rc)) {
            sai_wred_link_node_add(wred_link_id, p_wred_node, wred_link_type);
        }
    } else {
        /* WRED ID reset */
        wred_id = sai_qos_wred_link_wred_id_get(wred_link_id, wred_link_type);
        if(SAI_NULL_OBJECT_ID != wred_id) {
            p_wred_node = sai_qos_wred_node_get(wred_id);
            if(NULL == p_wred_node) {
                SAI_WRED_LOG_ERR("WRED ID 0x%"PRIx64" not found in DB",wred_id);
                return SAI_STATUS_FAILURE;
            }
        }

        sai_rc = sai_qos_wred_npu_api_get()->wred_link_reset(wred_link_id, wred_link_type);
        if((SAI_STATUS_SUCCESS == sai_rc) || (SAI_STATUS_NOT_EXECUTED == sai_rc)) {
            if(p_wred_node != NULL) {
                sai_wred_link_node_delete(wred_link_id, p_wred_node, wred_link_type);
            }
        }
    }

    if(SAI_STATUS_NOT_EXECUTED == sai_rc) {
        sai_qos_wred_link_mark_sw_cache(wred_link_id, wred_link_type);
        SAI_WRED_LOG_INFO("Cached WRED ID 0x%"PRIx64" SET for %s 0x%"PRIx64".",
                wred_id, sai_qos_wred_link_str(wred_link_type), wred_link_id);
        sai_rc = SAI_STATUS_SUCCESS;
    } else if (SAI_STATUS_SUCCESS == sai_rc) {
        sai_qos_wred_link_unmark_sw_cache(wred_link_id, wred_link_type);
    }

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_WRED_LOG_ERR("WRED ID 0x%"PRIx64" SET for %s 0x%"PRIx64" failed, sai_rc 0x%d.",
                wred_id, sai_qos_wred_link_str(wred_link_type), wred_link_id, sai_rc);
    }

    return sai_rc;
}

/* Function to get the Buffer Pool and Port Pool node, if accociated with the given Queue node */
static sai_status_t sai_wred_queue_get_port_pool_and_pool_node(dn_sai_qos_queue_t *p_queue_node,
        dn_sai_qos_buffer_pool_t **p_buffer_pool_node, dn_sai_qos_port_pool_t **p_port_pool_node)
{
    dn_sai_qos_buffer_profile_t *p_buffer_profile_node = NULL;

    if(p_queue_node->buffer_profile_id != SAI_NULL_OBJECT_ID) {
        p_buffer_profile_node = sai_qos_buffer_profile_node_get(p_queue_node->buffer_profile_id);
        if(NULL == p_buffer_profile_node) {
            SAI_WRED_LOG_ERR("Buffer Profile 0x%"PRIx64" not found in DB",p_queue_node->buffer_profile_id);
            return SAI_STATUS_FAILURE;
        }
    } else {
        return SAI_STATUS_NOT_EXECUTED;
    }

    if(p_buffer_profile_node->buffer_pool_id != SAI_NULL_OBJECT_ID) {
        *p_buffer_pool_node = sai_qos_buffer_pool_node_get(p_buffer_profile_node->buffer_pool_id);
        if(NULL == *p_buffer_pool_node) {
            SAI_WRED_LOG_ERR("Buffer Pool 0x%"PRIx64" not found in DB",p_buffer_profile_node->buffer_pool_id);
            return SAI_STATUS_FAILURE;
        }

        if(p_queue_node->port_id != SAI_NULL_OBJECT_ID) {
            *p_port_pool_node = sai_qos_port_pool_node_get(p_queue_node->port_id, p_buffer_profile_node->buffer_pool_id);
            if(NULL == *p_port_pool_node) {
                SAI_WRED_LOG_INFO("Port Pool for Port 0x%"PRIx64" and Buffer Pool 0x%"PRIx64" not found in DB",
                        p_queue_node->port_id, p_buffer_profile_node->buffer_pool_id);
                return SAI_STATUS_NOT_EXECUTED;
            }
        } else {
            return SAI_STATUS_NOT_EXECUTED;
        }
    } else {
        return SAI_STATUS_NOT_EXECUTED;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_wred_link_update_cache(sai_object_id_t queue_id)
{
    dn_sai_qos_queue_t *p_queue_node = NULL;
    dn_sai_qos_buffer_pool_t *p_buffer_pool_node = NULL;
    dn_sai_qos_port_pool_t *p_port_pool_node = NULL;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t queue_id_buf[2] = {0};
    sai_object_list_t queue_list;

    p_queue_node = sai_qos_queue_node_get(queue_id);
    if(NULL == p_queue_node) {
        SAI_WRED_LOG_ERR("Queue 0x%"PRIx64" not found in DB",queue_id);
        return SAI_STATUS_FAILURE;
    }

    /* WRED is valid only for UNICAST Queues */
    if(!(sai_qos_is_queue_type_ucast(p_queue_node->queue_type))) {
        return SAI_STATUS_SUCCESS;
    }

    sai_rc = sai_wred_queue_get_port_pool_and_pool_node(p_queue_node, &p_buffer_pool_node, &p_port_pool_node);
    if((sai_rc != SAI_STATUS_SUCCESS) && (sai_rc != SAI_STATUS_NOT_EXECUTED)) {
        return sai_rc;
    }

    if((p_buffer_pool_node != NULL) && (p_buffer_pool_node->wred_id != SAI_NULL_OBJECT_ID)) {
        memset(queue_id_buf,0,sizeof(queue_id_buf));
        queue_list.list = queue_id_buf;
        queue_list.count = 2;

        /* Getting 2 Queues. To verify that this is not the last queue */
        sai_rc = sai_qos_buffer_pool_get_wred_queue_ids(p_buffer_pool_node->key.pool_id, &queue_list);
        if((sai_rc == SAI_STATUS_SUCCESS) && (queue_list.count < 2)) {
            if(queue_list.count == 1) {
                if(queue_id_buf[0] != queue_id) {
                    /* Last queue associated must be this queue. If not, then something wrong */
                    SAI_WRED_LOG_ERR("Buffer Pool 0x%"PRIx64" WRED cache mismatch Queue Buf 0x%"PRIx64" and Queue 0x%"PRIx64".",
                            p_buffer_pool_node->key.pool_id, queue_id_buf[0], queue_id);
                }

                sai_rc = sai_qos_wred_npu_api_get()->wred_link_reset(
                        p_buffer_pool_node->key.pool_id,
                        DN_SAI_QOS_WRED_LINK_BUFFER_POOL);
                if(SAI_STATUS_SUCCESS != sai_rc) {
                    SAI_WRED_LOG_ERR("Buffer Pool 0x%"PRIx64" WRED reset and cache failed.",p_buffer_pool_node->key.pool_id);
                    return sai_rc;
                }

                sai_qos_wred_link_mark_sw_cache(p_buffer_pool_node->key.pool_id, DN_SAI_QOS_WRED_LINK_BUFFER_POOL);
            } else {
                if(p_buffer_pool_node != NULL) {
                    SAI_WRED_LOG_ERR("Buffer Pool 0x%"PRIx64" WRED queue count is zero",
                                     p_buffer_pool_node->key.pool_id);
                }
            }
        }
    }

    if((p_port_pool_node != NULL) && (p_port_pool_node->wred_id != SAI_NULL_OBJECT_ID)) {
        memset(queue_id_buf,0,sizeof(queue_id_buf));
        queue_list.list = queue_id_buf;
        queue_list.count = 2;

        /* Getting 2 Queues. To verify that this is not the last queue */
        sai_rc = sai_qos_port_pool_get_wred_queue_ids(p_port_pool_node->port_pool_id, &queue_list);
        if((sai_rc == SAI_STATUS_SUCCESS) && (queue_list.count < 2)) {
            if(queue_list.count == 1) {
                if(queue_id_buf[0] != queue_id) {
                    /* Last queue associated must be this queue. If not, then something wrong */
                    SAI_WRED_LOG_ERR("Port Pool 0x%"PRIx64" WRED cache mismatch Queue Buf 0x%"PRIx64" and Queue 0x%"PRIx64".",
                            p_port_pool_node->port_pool_id, queue_id_buf[0], queue_id);
                }

                sai_rc = sai_qos_wred_npu_api_get()->wred_link_reset(
                        p_port_pool_node->port_pool_id,
                        DN_SAI_QOS_WRED_LINK_PORT);
                if(SAI_STATUS_SUCCESS != sai_rc) {
                    SAI_WRED_LOG_ERR("Port Pool 0x%"PRIx64" WRED reset and cache failed.",p_port_pool_node->port_pool_id);
                    return sai_rc;
                }

                sai_qos_wred_link_mark_sw_cache(p_port_pool_node->port_pool_id, DN_SAI_QOS_WRED_LINK_PORT);
            } else {
                if(p_port_pool_node != NULL) {
                    SAI_WRED_LOG_ERR("Port Pool 0x%"PRIx64" WRED queue count is zero",
                                     p_port_pool_node->port_pool_id);
                }
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_wred_link_apply_cache(sai_object_id_t queue_id)
{
    dn_sai_qos_queue_t *p_queue_node = NULL;
    dn_sai_qos_buffer_pool_t *p_buffer_pool_node = NULL;
    dn_sai_qos_port_pool_t *p_port_pool_node = NULL;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    p_queue_node = sai_qos_queue_node_get(queue_id);
    if(NULL == p_queue_node) {
        SAI_WRED_LOG_ERR("Queue 0x%"PRIx64" not found in DB",queue_id);
        return SAI_STATUS_FAILURE;
    }

    /* WRED is valid only for UNICAST Queues */
    if(!(sai_qos_is_queue_type_ucast(p_queue_node->queue_type))) {
        return SAI_STATUS_SUCCESS;
    }

    sai_rc = sai_wred_queue_get_port_pool_and_pool_node(p_queue_node, &p_buffer_pool_node, &p_port_pool_node);
    if((sai_rc != SAI_STATUS_SUCCESS) && (sai_rc != SAI_STATUS_NOT_EXECUTED)) {
        return sai_rc;
    }

    if(p_buffer_pool_node != NULL) {
        if(sai_qos_wred_link_is_sw_cached(p_buffer_pool_node->key.pool_id, DN_SAI_QOS_WRED_LINK_BUFFER_POOL)) {
            /* If still cache is marked then this must be the first Queue associated to the pool */
            if((sai_rc = sai_qos_wred_link_set(p_buffer_pool_node->key.pool_id,
                            p_buffer_pool_node->wred_id,
                            DN_SAI_QOS_WRED_LINK_BUFFER_POOL)) == SAI_STATUS_SUCCESS) {
                SAI_WRED_LOG_INFO("Successfully linked the cached WRED ID 0x%"PRIx64" to Buffer pool 0x%"PRIx64".",
                        p_buffer_pool_node->wred_id, p_buffer_pool_node->key.pool_id);
            } else {
                SAI_WRED_LOG_ERR("Link the cached WRED ID 0x%"PRIx64" to Buffer pool 0x%"PRIx64" failed.",
                        p_buffer_pool_node->wred_id, p_buffer_pool_node->key.pool_id);
                return sai_rc;
            }
        }
    }

    if(p_port_pool_node != NULL) {
        if(sai_qos_wred_link_is_sw_cached(p_port_pool_node->port_pool_id, DN_SAI_QOS_WRED_LINK_PORT)) {
            /* If still cache is marked then this must be the first Queue associated to the pool */
            if((sai_rc = sai_qos_wred_link_set(p_port_pool_node->port_pool_id,
                            p_port_pool_node->wred_id,
                            DN_SAI_QOS_WRED_LINK_PORT)) == SAI_STATUS_SUCCESS) {
                SAI_WRED_LOG_INFO("Successfully linked the cached WRED ID 0x%"PRIx64" to Port pool 0x%"PRIx64".",
                        p_port_pool_node->wred_id, p_port_pool_node->port_pool_id);
            } else {
                SAI_WRED_LOG_ERR("Link the cached WRED ID 0x%"PRIx64" to Port pool 0x%"PRIx64" failed.",
                        p_port_pool_node->wred_id, p_port_pool_node->port_pool_id);
                return sai_rc;
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

/* API method table for Qos wred to be returned during query.
 **/
static sai_wred_api_t sai_qos_wred_method_table = {
    sai_qos_wred_create,
    sai_qos_wred_remove,
    sai_qos_wred_attribute_set,
    sai_qos_wred_attribute_get,
};

sai_wred_api_t *sai_wred_api_query (void)
{
    return &sai_qos_wred_method_table;
}
