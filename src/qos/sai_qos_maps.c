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
* @file  sai_qos_maps.c
*
* @brief This file contains function definitions for SAI QoS Maps
*        functionality API implementation.
*
*************************************************************************/
#include "sai_npu_qos.h"
#include "sai_npu_switch.h"
#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_api_utils.h"
#include "sai_gen_utils.h"
#include "sai_qos_mem.h"
#include "sai_common_infra.h"

#include "sai.h"
#include "saiqosmap.h"

#include "std_type_defs.h"
#include "std_utils.h"
#include "std_assert.h"
#include <stdlib.h>
#include <inttypes.h>

static inline uint_t sai_qos_maps_get_index(uint_t value, uint_t tc)
{
    return (value * SAI_QOS_MAX_TC) + tc;
}


sai_status_t sai_qos_map_list_alloc(dn_sai_qos_map_t *p_map_node, uint_t count)
{
    STD_ASSERT(p_map_node != NULL);

    p_map_node->map_to_value.list =
        (sai_qos_map_t *)calloc(count, sizeof(sai_qos_map_t));

    if(p_map_node->map_to_value.list == NULL){
        return SAI_STATUS_NO_MEMORY;
    }

    p_map_node->map_to_value.count = count;
    return SAI_STATUS_SUCCESS;
}


static sai_status_t sai_qos_map_dscp_to_tc_and_color_default_set(
dn_sai_qos_map_t *p_map_node)
{
    size_t dscp = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    STD_ASSERT(p_map_node != NULL);

    sai_rc = sai_qos_map_list_alloc(p_map_node, SAI_QOS_MAX_DSCP);

    if(sai_rc != SAI_STATUS_SUCCESS){
        return sai_rc;
    }

    for(dscp = 0; dscp < SAI_QOS_MAX_DSCP; dscp ++){
        p_map_node->map_to_value.list[dscp].key.dscp = dscp;
        p_map_node->map_to_value.list[dscp].value.tc =  SAI_QOS_DEFAULT_TC;
        p_map_node->map_to_value.list[dscp].value.color = SAI_PACKET_COLOR_GREEN;

    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_qos_map_tc_and_color_to_dot1p_default_set(
dn_sai_qos_map_t *p_map_node)
{
    uint_t tc = 0;
    uint_t color = 0;
    uint_t index = 0;
    sai_packet_color_t color_val [] = {SAI_PACKET_COLOR_GREEN,
                                       SAI_PACKET_COLOR_YELLOW,
                                       SAI_PACKET_COLOR_RED };
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_map_node != NULL);

    sai_rc = sai_qos_map_list_alloc(p_map_node, SAI_QOS_MAX_TC * SAI_QOS_MAX_PACKET_COLORS);

    if(sai_rc != SAI_STATUS_SUCCESS){
        return sai_rc;
    }

    for(tc = 0; tc < SAI_QOS_MAX_TC; tc++) {
        for(color = 0; color < SAI_QOS_MAX_PACKET_COLORS; color++) {
            index = sai_qos_maps_get_index(color, tc);
            p_map_node->map_to_value.list[index].key.tc = tc;
            p_map_node->map_to_value.list[index].key.color = color_val[color];
            p_map_node->map_to_value.list[index].value.dot1p =  SAI_QOS_DEFAULT_DOT1P;
        }
    }

    return sai_rc;
}

static sai_status_t sai_qos_map_tc_and_color_to_dscp_default_set(
dn_sai_qos_map_t *p_map_node)
{
    uint_t tc = 0;
    uint_t color = 0;
    uint_t index = 0;
    sai_packet_color_t color_val [] = {SAI_PACKET_COLOR_GREEN,
                                       SAI_PACKET_COLOR_YELLOW,
                                       SAI_PACKET_COLOR_RED };
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_map_node != NULL);

    sai_rc = sai_qos_map_list_alloc(p_map_node, SAI_QOS_MAX_TC * SAI_QOS_MAX_PACKET_COLORS);

    if(sai_rc != SAI_STATUS_SUCCESS){
        return sai_rc;
    }

    for(tc = 0; tc < SAI_QOS_MAX_TC; tc++) {
        for(color = 0; color < SAI_QOS_MAX_PACKET_COLORS; color++) {
            index = sai_qos_maps_get_index(color, tc);
            p_map_node->map_to_value.list[index].key.tc = tc;
            p_map_node->map_to_value.list[index].key.color = color_val[color];
            p_map_node->map_to_value.list[index].value.dscp =  SAI_QOS_DEFAULT_DSCP;
        }
    }

    return sai_rc;
}

static sai_status_t sai_qos_map_tc_to_queue_default_set(
dn_sai_qos_map_t *p_map_node)
{
    size_t tc = 0;
    uint_t count = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    STD_ASSERT(p_map_node != NULL);

    count = sai_qos_maps_get_max_queue_idx() * SAI_QOS_MAX_TC;

    sai_rc = sai_qos_map_list_alloc(p_map_node, count);

    if(sai_rc != SAI_STATUS_SUCCESS){
        return sai_rc;
    }
    /*
     * Allocating twice max tc entries to populate unicast and multicast
     * queue object ids.
     * List index 0 - 15 will have unicast queue object ids for TC 0 - 15
     * List index 16 - 31 will have multicast queue object ids for TC 0 - 15
     */
    for(tc = 0; tc < SAI_QOS_MAX_TC; tc ++)
    {
        p_map_node->map_to_value.list[tc].key.tc = tc;
        p_map_node->map_to_value.list[tc].value.queue_index = SAI_QOS_DEFAULT_QUEUE_INDEX;

        p_map_node->map_to_value.list[(SAI_QOS_MAX_TC) + tc].key.tc = tc;
        p_map_node->map_to_value.list[(SAI_QOS_MAX_TC) + tc].value.queue_index =
            SAI_QOS_DEFAULT_QUEUE_INDEX;

    }
    return sai_rc;

}
static sai_status_t sai_qos_map_dot1p_to_tc_and_color_default_set(
dn_sai_qos_map_t *p_map_node)
{
    size_t dot1p = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_map_node != NULL);

    sai_rc = sai_qos_map_list_alloc(p_map_node, SAI_QOS_MAX_DOT1P);

    if(sai_rc != SAI_STATUS_SUCCESS){
        return sai_rc;
    }

    for(dot1p = 0; dot1p < SAI_QOS_MAX_DOT1P; dot1p ++){
        p_map_node->map_to_value.list[dot1p].key.dot1p = dot1p;
        p_map_node->map_to_value.list[dot1p].value.tc =  SAI_QOS_DEFAULT_TC;
        p_map_node->map_to_value.list[dot1p].value.color = SAI_PACKET_COLOR_GREEN;

    }

    return sai_rc;
}


static sai_status_t sai_qos_map_dot1p_to_tc_and_color_list_update(
dn_sai_qos_map_t *p_map_node, sai_qos_map_list_t map_list)
{
    size_t loop_idx = 0;
    uint_t dot1p    = 0;
    uint_t tc       = 0;
    uint_t color    = 0;
    uint_t dscp     = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_map_node != NULL);
    STD_ASSERT(map_list.count != 0);
    STD_ASSERT(map_list.list != NULL);
    STD_ASSERT(p_map_node->map_to_value.list != NULL);

    for(loop_idx = 0; loop_idx < map_list.count; loop_idx ++){
        dot1p = map_list.list[loop_idx].key.dot1p;
        tc    = map_list.list[loop_idx].value.tc;
        color = map_list.list[loop_idx].value.color;

        SAI_MAPS_LOG_TRACE("dot1p %d tc %d color %d", dot1p, tc, color);
        sai_rc = sai_qos_maps_attr_value_validate(dot1p, tc, color, dscp);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_MAPS_LOG_ERR("Invalid attribute value");
            break;
        }
        p_map_node->map_to_value.list[dot1p].key.dot1p = dot1p;
        p_map_node->map_to_value.list[dot1p].value.tc = tc;
        p_map_node->map_to_value.list[dot1p].value.color = color;

    }

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_MAPS_LOG_INFO("Map list updated for maptype %d",
                           p_map_node->map_type);
    }

    return sai_rc;
}

static sai_status_t sai_qos_map_dscp_to_tc_and_color_list_update(
dn_sai_qos_map_t *p_map_node, sai_qos_map_list_t map_list)
{
    size_t loop_idx = 0;
    uint_t dscp     = 0;
    uint_t dot1p    = 0;
    uint_t tc       = 0;
    uint_t color    = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_map_node != NULL);
    STD_ASSERT(map_list.count != 0);
    STD_ASSERT(map_list.list != NULL);
    STD_ASSERT(p_map_node->map_to_value.list != NULL);

    for(loop_idx = 0; loop_idx < map_list.count; loop_idx ++){
        dscp = map_list.list[loop_idx].key.dscp;
        tc   = map_list.list[loop_idx].value.tc;
        color = map_list.list[loop_idx].value.color;

        sai_rc = sai_qos_maps_attr_value_validate(dot1p, tc, color, dscp);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_MAPS_LOG_ERR("Invalid attribute value");
            break;
        }
        p_map_node->map_to_value.list[dscp].key.dscp = dscp;
        p_map_node->map_to_value.list[dscp].value.tc = tc;
        p_map_node->map_to_value.list[dscp].value.color = color;
    }

    if(sai_rc == SAI_STATUS_SUCCESS){
    SAI_MAPS_LOG_INFO("Map list updated for maptype %d",
                       p_map_node->map_type);
    }

    return sai_rc;
}

static sai_status_t sai_qos_map_tc_to_queue_list_update(
dn_sai_qos_map_t *p_map_node, sai_qos_map_list_t map_list)
{
    size_t              loop_idx = 0;
    uint_t              tc = 0;
    uint_t              dot1p = 0;
    uint_t              dscp = 0;
    uint_t              color = 0;
    uint_t              index = 0;
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_map_node != NULL);
    STD_ASSERT(map_list.count != 0);
    STD_ASSERT(map_list.list != NULL);
    STD_ASSERT(p_map_node->map_to_value.list != NULL);

    for(loop_idx = 0; loop_idx < map_list.count; loop_idx ++){
        tc = map_list.list[loop_idx].key.tc;

        sai_rc = sai_qos_maps_attr_value_validate(dot1p, tc, color, dscp);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_MAPS_LOG_ERR("Invalid attribute value");
            break;
        }
        if(map_list.list[loop_idx].value.queue_index >= sai_switch_max_queues_get()){
            SAI_MAPS_LOG_ERR("Invalid queue_index value");
            sai_rc = SAI_STATUS_INVALID_ATTR_VALUE_0;
            break;
        }

        /*
         * Based on the queue type the corresponding index in the map list
         * is updated.
         * Unicast queues are from 0 - 15.
         * Multicast queues are from 16 - 31 for TCs 0 - 15.
         */
        index = tc;

        if(map_list.list[loop_idx].value.queue_index >= sai_switch_max_uc_queues_get()){
            index = (SAI_QOS_MAX_TC) + tc;
            map_list.list[loop_idx].value.queue_index -= sai_switch_max_uc_queues_get();
        }

        SAI_MAPS_LOG_TRACE("queue_index is %d  Index %d\r\n",
                           map_list.list[loop_idx].value.queue_index, index);
        p_map_node->map_to_value.list[index].key.tc = tc;

        p_map_node->map_to_value.list[index].value.queue_index =
            map_list.list[loop_idx].value.queue_index;
    }

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_MAPS_LOG_INFO("Map list updated for maptype %d",
                           p_map_node->map_type);
    }

    return sai_rc;
}

static sai_status_t sai_qos_map_tc_and_color_to_dot1p_list_update(
dn_sai_qos_map_t *p_map_node, sai_qos_map_list_t map_list)
{
    size_t loop_idx = 0;
    uint_t tc       = 0;
    uint_t dot1p    = 0;
    uint_t color    = 0;
    uint_t dscp     = 0;
    size_t index    = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_map_node != NULL);
    STD_ASSERT(map_list.count != 0);
    STD_ASSERT(map_list.list != NULL);
    STD_ASSERT(p_map_node->map_to_value.list != NULL);

    for(loop_idx = 0; loop_idx < map_list.count; loop_idx ++){
        tc = map_list.list[loop_idx].key.tc;
        color = map_list.list[loop_idx].key.color;
        dot1p = map_list.list[loop_idx].value.dot1p;

        sai_rc = sai_qos_maps_attr_value_validate(dot1p, tc, color, dscp);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_MAPS_LOG_ERR("Invalid attribute value");
            break;
        }
        index = sai_qos_maps_get_index(color, tc);
        p_map_node->map_to_value.list[index].key.tc = tc;
        p_map_node->map_to_value.list[index].key.color = color;
        p_map_node->map_to_value.list[index].value.dot1p = dot1p;

    }

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_MAPS_LOG_INFO("Map list updated for maptype %d",
                       p_map_node->map_type);
    }

    return sai_rc;
}

static sai_status_t sai_qos_map_tc_and_color_to_dscp_list_update(
dn_sai_qos_map_t *p_map_node, sai_qos_map_list_t map_list)
{
    uint_t loop_idx = 0;
    uint_t tc       = 0;
    uint_t dot1p    = 0;
    uint_t color    = 0;
    uint_t dscp     = 0;
    size_t index    = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_map_node != NULL);
    STD_ASSERT(map_list.count != 0);
    STD_ASSERT(map_list.list != NULL);
    STD_ASSERT(p_map_node->map_to_value.list != NULL);

    for(loop_idx = 0; loop_idx < map_list.count; loop_idx ++){
        tc = map_list.list[loop_idx].key.tc;
        color = map_list.list[loop_idx].key.color;
        dscp = map_list.list[loop_idx].value.dscp;

        index = sai_qos_maps_get_index(color, tc);
        sai_rc = sai_qos_maps_attr_value_validate(dot1p, tc, color, dscp);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_MAPS_LOG_TRACE("Invalid attribute value");
            break;
        }

        p_map_node->map_to_value.list[index].key.tc = tc;
        p_map_node->map_to_value.list[index].key.color = color;
        p_map_node->map_to_value.list[index].value.dscp = dscp;
    }

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_MAPS_LOG_INFO("Map list updated for maptype %d",
                       p_map_node->map_type);
    }

    return sai_rc;
}

static sai_status_t sai_qos_map_tc_to_queue_map_value_set(
dn_sai_qos_map_t *p_map_node, sai_qos_map_list_t map_list, dn_sai_operations_t op_type)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    if((op_type == SAI_OP_CREATE) || ((op_type == SAI_OP_SET)
                                      && p_map_node->map_to_value.list == NULL)){
        sai_rc = sai_qos_map_tc_to_queue_default_set(p_map_node);

        if(sai_rc != SAI_STATUS_SUCCESS){
            return sai_rc;
        }
    }
    sai_rc = sai_qos_map_tc_to_queue_list_update(p_map_node, map_list);
    return sai_rc;
}

static sai_status_t sai_qos_map_dot1p_to_tc_and_color_map_value_set(
dn_sai_qos_map_t *p_map_node, sai_qos_map_list_t map_list, dn_sai_operations_t op_type)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    if((op_type == SAI_OP_CREATE) || ((op_type == SAI_OP_SET)
                                      && p_map_node->map_to_value.list == NULL)){
        sai_rc = sai_qos_map_dot1p_to_tc_and_color_default_set(p_map_node);

        if(sai_rc != SAI_STATUS_SUCCESS){
            return sai_rc;
        }
    }
    sai_rc = sai_qos_map_dot1p_to_tc_and_color_list_update(p_map_node, map_list);

    return sai_rc;

}

static sai_status_t sai_qos_map_dscp_to_tc_and_color_map_value_set(
dn_sai_qos_map_t *p_map_node, sai_qos_map_list_t map_list, dn_sai_operations_t op_type)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    if((op_type == SAI_OP_CREATE) || ((op_type == SAI_OP_SET)
                                      && p_map_node->map_to_value.list == NULL)){
        sai_rc = sai_qos_map_dscp_to_tc_and_color_default_set(p_map_node);

        if(sai_rc != SAI_STATUS_SUCCESS){
            return sai_rc;
        }
    }
    sai_rc = sai_qos_map_dscp_to_tc_and_color_list_update(p_map_node, map_list);

    return sai_rc;

}

static sai_status_t sai_qos_map_tc_and_color_to_dscp_map_value_set(
dn_sai_qos_map_t *p_map_node, sai_qos_map_list_t map_list, dn_sai_operations_t op_type)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    if((op_type == SAI_OP_CREATE) || ((op_type == SAI_OP_SET)
                                      && p_map_node->map_to_value.list == NULL)){
        sai_rc = sai_qos_map_tc_and_color_to_dscp_default_set(p_map_node);

        if(sai_rc != SAI_STATUS_SUCCESS){
            return sai_rc;
        }
    }
    sai_rc = sai_qos_map_tc_and_color_to_dscp_list_update(p_map_node, map_list);

    return sai_rc;

}

static sai_status_t sai_qos_map_tc_and_color_to_dot1p_map_value_set(
dn_sai_qos_map_t *p_map_node, sai_qos_map_list_t map_list, dn_sai_operations_t op_type)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    if((op_type == SAI_OP_CREATE) || ((op_type == SAI_OP_SET)
                                      && p_map_node->map_to_value.list == NULL)){
        sai_rc = sai_qos_map_tc_and_color_to_dot1p_default_set(p_map_node);

        if(sai_rc != SAI_STATUS_SUCCESS){
            return sai_rc;
        }
    }
    sai_rc = sai_qos_map_tc_and_color_to_dot1p_list_update(p_map_node, map_list);

    return sai_rc;
}

static sai_status_t sai_qos_map_list_default_set(dn_sai_qos_map_t *p_map_node)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_map_node != NULL);

    if(!(sai_qos_map_npu_api_get()->is_map_supported(p_map_node->map_type))){
        SAI_MAPS_LOG_TRACE("Map type %d is not supported", p_map_node->map_type);
        return SAI_STATUS_NOT_SUPPORTED;
    }

    switch(p_map_node->map_type){

        case SAI_QOS_MAP_TYPE_TC_TO_QUEUE:
            sai_rc = sai_qos_map_tc_to_queue_default_set(p_map_node);
            break;
        /* Fall through  */
        case SAI_QOS_MAP_TYPE_DOT1P_TO_TC:
        case SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR:
            sai_rc = sai_qos_map_dot1p_to_tc_and_color_default_set(p_map_node);
            break;

        /* Fall through  */
        case SAI_QOS_MAP_TYPE_DSCP_TO_TC:
        case SAI_QOS_MAP_TYPE_DSCP_TO_COLOR:
            sai_rc = sai_qos_map_dscp_to_tc_and_color_default_set(p_map_node);
            break;

        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP:
            sai_rc = sai_qos_map_tc_and_color_to_dscp_default_set(p_map_node);
            break;

        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P:
            sai_rc = sai_qos_map_tc_and_color_to_dot1p_default_set(p_map_node);
            break;

        case SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP:
            sai_rc = sai_qos_map_tc_to_pg_default_set (p_map_node);
            break;

        case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE:
            sai_rc = sai_qos_map_pfc_pri_to_queue_default_set (p_map_node);
            break;

        default:
            sai_rc = SAI_STATUS_NOT_SUPPORTED;

    }

    return sai_rc;
}

static sai_status_t sai_qos_map_list_attr_set(
dn_sai_qos_map_t *p_map_node, sai_qos_map_list_t map_list, dn_sai_operations_t op_type)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_map_node != NULL);
    STD_ASSERT(map_list.count != 0);
    STD_ASSERT(map_list.list != NULL);

    if(!(sai_qos_map_npu_api_get()->is_map_supported(p_map_node->map_type))){
        SAI_MAPS_LOG_TRACE("Map type %d is not supported", p_map_node->map_type);
        return SAI_STATUS_NOT_SUPPORTED;
    }

    switch(p_map_node->map_type){

        case SAI_QOS_MAP_TYPE_TC_TO_QUEUE:
            sai_rc = sai_qos_map_tc_to_queue_map_value_set(p_map_node, map_list, op_type);
            break;

        case SAI_QOS_MAP_TYPE_DOT1P_TO_TC:
        case SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR:
            sai_rc = sai_qos_map_dot1p_to_tc_and_color_map_value_set
                (p_map_node, map_list, op_type);
            break;

        case SAI_QOS_MAP_TYPE_DSCP_TO_TC:
        case SAI_QOS_MAP_TYPE_DSCP_TO_COLOR:
            sai_rc = sai_qos_map_dscp_to_tc_and_color_map_value_set
                (p_map_node, map_list, op_type);
            break;

        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP:
            sai_rc = sai_qos_map_tc_and_color_to_dscp_map_value_set
                (p_map_node, map_list, op_type);
            break;

        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P:
            sai_rc = sai_qos_map_tc_and_color_to_dot1p_map_value_set
                (p_map_node, map_list, op_type);
            break;

        case SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP:
            sai_rc = sai_qos_map_tc_to_pg_map_value_set(p_map_node, map_list, op_type);
            break;

        case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE:
            sai_rc = sai_qos_map_pfc_pri_to_queue_map_value_set(p_map_node, map_list, op_type);
            break;

        default:
            sai_rc = SAI_STATUS_NOT_SUPPORTED;

    }

    return sai_rc;
}

static sai_status_t sai_qos_maps_attr_set(
dn_sai_qos_map_t *p_map_node, const sai_attribute_t *p_attr,
uint_t *p_attr_flags, dn_sai_operations_t op_type)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_map_node != NULL);
    STD_ASSERT(p_attr != NULL);
    STD_ASSERT(p_attr_flags != NULL);

    SAI_MAPS_LOG_TRACE ("Setting Maps attribute id: %d.",
                       p_attr->id);

    switch (p_attr->id){
        case SAI_QOS_MAP_ATTR_TYPE:
            sai_rc = sai_qos_map_type_attr_set (p_map_node, p_attr->value.s32);
            *p_attr_flags |= SAI_QOS_MAP_TYPE_FLAG;
            break;

        case SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST:
            sai_rc = sai_qos_map_list_attr_set (p_map_node, p_attr->value.qosmap, op_type);
            *p_attr_flags |= SAI_QOS_MAP_VALUE_TO_LIST_FLAG;
            break;

        default:
            SAI_MAPS_LOG_ERR ("Attribute id: %d is not a known attribute "
                              "for Maps.", p_attr->id);

            return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
    }

    return sai_rc;
}

static sai_status_t sai_qos_parse_update_attributes(
                                     dn_sai_qos_map_t *pmap_node,
                                     uint32_t attr_count,
                                     const sai_attribute_t *attr_list,
                                     dn_sai_operations_t op_type,
                                     uint_t *p_attr_flags)
{
    sai_status_t               sai_rc = SAI_STATUS_SUCCESS;
    uint_t                     list_idx = 0;
    uint_t                     max_vendor_attr_count = 0;
    const dn_sai_attribute_entry_t   *p_vendor_attr = NULL;

    STD_ASSERT(pmap_node != NULL);
    STD_ASSERT(attr_list != NULL);
    STD_ASSERT(p_attr_flags != NULL);

    sai_qos_map_npu_api_get()->attribute_table_get
        (&p_vendor_attr, &max_vendor_attr_count);

    if((p_vendor_attr == NULL) || (max_vendor_attr_count == 0)){
        SAI_MAPS_LOG_ERR("Vendor attribute table get failed");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_rc = sai_attribute_validate(attr_count,attr_list, p_vendor_attr,
                                    op_type, max_vendor_attr_count);

    if(sai_rc != SAI_STATUS_SUCCESS){
        SAI_MAPS_LOG_ERR("Attribute validation failed for %d operation",
                         op_type);
        return sai_rc;
    }

    if(op_type != SAI_OP_GET){
        /*
         * Map type is must for filling in the default values based on
         * type. The attr_list can have the maptype at any location.
         * So skipping through all other attributes to set the map type
         * and then set other attributes.
         */
        for (list_idx = 0; list_idx < attr_count; list_idx++){

            if(attr_list[list_idx].id != SAI_QOS_MAP_ATTR_TYPE){
                continue;
            }
            sai_rc = sai_qos_maps_attr_set(pmap_node, &attr_list[list_idx], p_attr_flags, op_type);
            if(sai_rc != SAI_STATUS_SUCCESS)
            {
                return sai_get_indexed_ret_val(sai_rc, list_idx);
            }
            break;
        }

        for (list_idx = 0; list_idx < attr_count; list_idx++){

            if(attr_list[list_idx].id == SAI_QOS_MAP_ATTR_TYPE){
                continue;
            }
            sai_rc = sai_qos_maps_attr_set(pmap_node, &attr_list[list_idx], p_attr_flags, op_type);
            if(sai_rc != SAI_STATUS_SUCCESS)
            {
                sai_rc = sai_get_indexed_ret_val(sai_rc, list_idx);
                break;
            }
        }
    }
    return sai_rc;
}


/* SAI QOS Maps API Implementations. */

static sai_status_t sai_qos_map_create(sai_object_id_t *map_id,
                          _In_ sai_object_id_t switch_id,
                          uint32_t attr_count,
                          const sai_attribute_t *attr_list)
{
    sai_status_t           sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_map_t       *p_map_node = NULL;
    sai_npu_object_id_t    hw_map_id = 0;
    uint_t                 attr_flags = 0;

    STD_ASSERT(map_id != NULL);
    STD_ASSERT(attr_list != NULL);

    sai_qos_lock();

    do{
        if((p_map_node = sai_qos_maps_node_alloc()) == NULL){
            SAI_MAPS_LOG_ERR("Failed to allocate memory for mapnode.");
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        sai_rc = sai_qos_parse_update_attributes(p_map_node, attr_count,
                                                 attr_list, SAI_OP_CREATE,
                                                 &attr_flags);
        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_MAPS_LOG_ERR("Failed to parse maps attributes for maptype %d",
                             p_map_node->map_type);
            break;
        }

        if(p_map_node->map_to_value.list == NULL){
            sai_rc = sai_qos_map_list_default_set(p_map_node);
            if(sai_rc != SAI_STATUS_SUCCESS){
                SAI_MAPS_LOG_ERR("Default map list update failed for maptype %d",
                                 p_map_node->map_type);
                break;
            }
        }

        sai_rc = sai_qos_map_npu_api_get()->map_create(p_map_node, &hw_map_id);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_MAPS_LOG_ERR("Npu map create failed for maptype %d",
                             p_map_node->map_type);
            break;
        }

        SAI_MAPS_LOG_TRACE("Mapid created in NPU 0x%"PRIx64"",hw_map_id);

        /* Append the maptype to the NPU returned map id.*/

        hw_map_id = sai_add_type_to_object(hw_map_id,
                                                 p_map_node->map_type);

        *map_id = sai_uoid_create(SAI_OBJECT_TYPE_QOS_MAP,
                                  hw_map_id);

        SAI_MAPS_LOG_TRACE("Map object id is 0x%"PRIx64"",*map_id);

        p_map_node->key.map_id = *map_id;

        std_dll_init(&p_map_node->port_dll_head);

        if (sai_qos_map_node_insert(p_map_node) != STD_ERR_OK){
            sai_qos_map_npu_api_get()->map_remove(p_map_node);

            SAI_MAPS_LOG_ERR("Map id insertion failed in RB tree");

            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

    }while(0);


    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_MAPS_LOG_INFO("Map created successfully for maptype %d",
                           p_map_node->map_type);
    }
    else if (p_map_node != NULL)
    {

        SAI_MAPS_LOG_ERR("Map create failed for maptype %d",
                           p_map_node->map_type);
        sai_qos_map_free_resources(p_map_node);
    }
    sai_qos_unlock();
    return sai_rc;

}

static sai_status_t sai_qos_map_remove(sai_object_id_t map_id)
{
    dn_sai_qos_map_t  *p_map_node = NULL;
    sai_status_t      sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_port_t *p_qos_port_node = NULL;

    if(!sai_is_obj_id_qos_map(map_id)){
        SAI_MAPS_LOG_ERR("mapid 0x%"PRIx64" is not of type qosmap",map_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_qos_lock();

    do{
        p_map_node = sai_qos_map_node_get(map_id);

        if(p_map_node == NULL){
            SAI_MAPS_LOG_ERR("Map_id 0x%"PRIx64" not present in tree",
                             map_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        p_qos_port_node  = sai_qos_maps_get_port_node_from_map(p_map_node);

        if(p_qos_port_node != NULL){
            SAI_MAPS_LOG_ERR("Mapid 0x%"PRIx64" is in use", map_id);
            sai_rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        sai_rc = sai_qos_map_npu_api_get()->map_remove(p_map_node);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_MAPS_LOG_ERR("Npu map remove failed for mapid 0x%"PRIx64"",
                             map_id);
            break;
        }

        sai_qos_map_node_remove(map_id);

        sai_qos_map_free_resources(p_map_node);
    }while(0);

    sai_qos_unlock();

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_MAPS_LOG_INFO("Map id 0x%"PRIx64" successfully removed ",map_id);
    }
    else
    {
        SAI_MAPS_LOG_ERR("Map id 0x%"PRIx64" removal failed",map_id);
    }

    return sai_rc;
}


static sai_status_t sai_qos_map_attribute_set(
sai_object_id_t map_id, const sai_attribute_t *p_attr)
{
    dn_sai_qos_map_t   map_new_node;
    dn_sai_qos_map_t  *p_map_exist_node = NULL;
    sai_status_t      sai_rc = SAI_STATUS_SUCCESS;
    uint_t            attr_flags = 0;
    uint_t            attr_count = 1;

    STD_ASSERT (p_attr != NULL);

    if(!sai_is_obj_id_qos_map(map_id)){
        SAI_MAPS_LOG_ERR("mapid 0x%"PRIx64" is not of type qosmap",map_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    SAI_MAPS_LOG_TRACE("Setting attribute Id: %d on Map Id 0x%"PRIx64"",
           p_attr->id, map_id);

    sai_qos_lock();

    do{

        p_map_exist_node = sai_qos_map_node_get(map_id);

        if(p_map_exist_node == NULL){
            SAI_MAPS_LOG_ERR("Mapid 0x%"PRIx64" not found in tree",map_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        memset(&map_new_node, 0, sizeof(dn_sai_qos_map_t));

        map_new_node.map_type = p_map_exist_node->map_type;
        map_new_node.key.map_id = map_id;

        if(p_map_exist_node->map_to_value.list != NULL){

            memcpy(&map_new_node.map_to_value,
                   &p_map_exist_node->map_to_value, sizeof(p_map_exist_node->map_to_value));
        }

        sai_rc = sai_qos_parse_update_attributes(&map_new_node, attr_count,
                                                 p_attr, SAI_OP_SET,
                                                 &attr_flags);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_MAPS_LOG_ERR("Validation failed for attribute Id %d",
                             p_attr->id);
            break;
        }

        SAI_MAPS_LOG_TRACE("Map list count in set is %d",
                               map_new_node.map_to_value.count);
        sai_rc = sai_qos_map_npu_api_get()->map_attr_set
            (&map_new_node, attr_flags);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_MAPS_LOG_ERR("NPU set failed for attribute Id %d"
                             "on mapid 0x%"PRIx64"",p_attr->id, map_id);
            break;
        }

    }while(0);


    if(sai_rc == SAI_STATUS_SUCCESS){
        memcpy(&p_map_exist_node->map_to_value,
               &map_new_node.map_to_value, sizeof(p_map_exist_node->map_to_value));

        /*TC to Queue map alone is not hardware generated id. So when the
         * map values are modified update the associated ports on which the
         * map is applied.
         */

        if(!sai_qos_map_npu_api_get()->map_is_hw_object(p_map_exist_node->map_type)){
            sai_qos_map_port_list_update(p_map_exist_node);
        }
    }

    sai_qos_unlock();

    return sai_rc;
}


static sai_status_t sai_qos_map_attribute_get(sai_object_id_t map_id,
                                              uint32_t attr_count,
                                              sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_qos_map_t     *p_map_node = NULL;
    uint_t               attr_flags = 0;

    STD_ASSERT(attr_list != NULL);

    if(!sai_is_obj_id_qos_map(map_id)){
        SAI_MAPS_LOG_ERR("mapid 0x%"PRIx64" is not of type qosmap",map_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if ((attr_count == 0) || (attr_count > SAI_QOS_MAPS_MAX_ATTR_COUNT)){
        SAI_MAPS_LOG_ERR("Invalid number of attributes for qosmaps");
        return SAI_STATUS_INVALID_PARAMETER;
    }


    sai_qos_lock();

    do{
        p_map_node = sai_qos_map_node_get(map_id);

        if(p_map_node == NULL){
            SAI_MAPS_LOG_ERR("Mapid 0x%"PRIx64" not found in tree",map_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }
        sai_rc = sai_qos_parse_update_attributes(p_map_node, attr_count,
                                                 attr_list, SAI_OP_GET,
                                                 &attr_flags);

        if (sai_rc != SAI_STATUS_SUCCESS){
            SAI_MAPS_LOG_ERR("Validation failed for attribute");
            break;
        }

        sai_rc = sai_qos_map_npu_api_get()->map_attr_get(p_map_node,
                                                         attr_count, attr_list);

        if (sai_rc != SAI_STATUS_SUCCESS){
            SAI_MAPS_LOG_ERR("Npu get failed for mapid 0x%"PRIx64"",map_id);
            break;
        }
    } while (0);

    sai_qos_unlock();

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_MAPS_LOG_INFO("Get attribute success for mapid 0x%"PRIx64"",map_id);
    }
    else
    {
        SAI_MAPS_LOG_ERR("Get attribute failed for mapid 0x%"PRIx64"",map_id);
    }

    return sai_rc;

}

/* API method table for Qos maps to be returned during query.
 */
static sai_qos_map_api_t sai_qos_maps_method_table =
{
    sai_qos_map_create,
    sai_qos_map_remove,
    sai_qos_map_attribute_set,
    sai_qos_map_attribute_get
};

sai_qos_map_api_t *sai_qosmaps_api_query (void)
{
    return &sai_qos_maps_method_table;
}
