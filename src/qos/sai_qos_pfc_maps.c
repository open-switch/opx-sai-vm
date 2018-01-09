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
* @file  sai_qos_pfc_maps.c
*
* @brief This file contains function definitions for SAI QoS PFC Maps
*        functionality API implementation.
*
*************************************************************************/
#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_api_utils.h"
#include "sai_qos_mem.h"
#include "sai_switch_utils.h"

#include "sai.h"
#include "saitypes.h"
#include "saiqosmaps.h"

#include "std_type_defs.h"
#include "std_utils.h"
#include "std_assert.h"
#include <stdlib.h>
#include <inttypes.h>

sai_status_t sai_qos_map_tc_to_pg_default_set (dn_sai_qos_map_t *p_map_node)
{
    size_t tc = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    STD_ASSERT(p_map_node != NULL);

    sai_rc = sai_qos_map_list_alloc(p_map_node, SAI_QOS_MAX_TC);

    if(sai_rc != SAI_STATUS_SUCCESS){
        return sai_rc;
    }

    for(tc = 0; tc < SAI_QOS_MAX_TC; tc ++)
    {
        p_map_node->map_to_value.list[tc].key.tc = tc;
        p_map_node->map_to_value.list[tc].value.pg = SAI_QOS_DEFAULT_PG;

    }
    return sai_rc;

}

static sai_status_t sai_qos_map_tc_to_pg_list_update (dn_sai_qos_map_t *p_map_node,
                                                      sai_qos_map_list_t map_list)
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

        sai_rc = sai_qos_maps_attr_value_validate (dot1p, tc, color, dscp);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_MAPS_LOG_ERR ("Invalid attribute value");
            break;
        }
        if(map_list.list[loop_idx].value.pg >= sai_switch_num_pg_get()) {
            SAI_MAPS_LOG_ERR ("Invalid attribute value");
            sai_rc = SAI_STATUS_INVALID_ATTR_VALUE_0;
            break;
        }

        index = tc;


        SAI_MAPS_LOG_TRACE("pg is %d  Index %d\r\n",
                           map_list.list[loop_idx].value.pg, index);
        p_map_node->map_to_value.list[index].key.tc = tc;

        p_map_node->map_to_value.list[index].value.pg =
            map_list.list[loop_idx].value.pg;
    }

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_MAPS_LOG_INFO("Map list updated for maptype %d",
                           p_map_node->map_type);
    }

    return sai_rc;
}

sai_status_t sai_qos_map_tc_to_pg_map_value_set (dn_sai_qos_map_t *p_map_node,
                                                 sai_qos_map_list_t map_list,
                                                 dn_sai_operations_t op_type)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    if((op_type == SAI_OP_CREATE) || ((op_type == SAI_OP_SET)
                                      && p_map_node->map_to_value.list == NULL)){

        sai_rc = sai_qos_map_tc_to_pg_default_set (p_map_node);

        if(sai_rc != SAI_STATUS_SUCCESS){
            return sai_rc;
        }
    }
    sai_rc = sai_qos_map_tc_to_pg_list_update (p_map_node, map_list);
    return sai_rc;
}

static sai_status_t sai_qos_map_pfc_pri_to_queue_list_update (dn_sai_qos_map_t *p_map_node,
                                                              sai_qos_map_list_t map_list)
{
    size_t              loop_idx = 0;
    uint_t              pfc_pri = 0;
    uint_t              index = 0;
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_map_node != NULL);
    STD_ASSERT(map_list.count != 0);
    STD_ASSERT(map_list.list != NULL);
    STD_ASSERT(p_map_node->map_to_value.list != NULL);

    for(loop_idx = 0; loop_idx < map_list.count; loop_idx ++){
        pfc_pri = map_list.list[loop_idx].key.prio;

        if(map_list.list[loop_idx].value.queue_index > sai_switch_max_queues_get()){
            SAI_MAPS_LOG_ERR("Invalid queue_index value");
            sai_rc = SAI_STATUS_INVALID_ATTR_VALUE_0;
            break;
        }
        index = pfc_pri;

        if(map_list.list[loop_idx].value.queue_index >= sai_switch_max_uc_queues_get()){
            index = (SAI_QOS_MAX_PFC_PRI) + pfc_pri;
            map_list.list[loop_idx].value.queue_index -= sai_switch_max_uc_queues_get();
        }


        SAI_MAPS_LOG_TRACE("queue_index is %d  Index %d\r\n",
                           map_list.list[loop_idx].value.queue_index, index);

        p_map_node->map_to_value.list[index].key.prio = pfc_pri;

        p_map_node->map_to_value.list[index].value.queue_index =
            map_list.list[loop_idx].value.queue_index;
    }

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_MAPS_LOG_INFO("Map list updated for maptype %d",
                           p_map_node->map_type);
    }

    return sai_rc;
}

sai_status_t sai_qos_map_pfc_pri_to_queue_default_set (dn_sai_qos_map_t *p_map_node)
{
    size_t pfc_pri = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    uint_t count = 0;
    STD_ASSERT(p_map_node != NULL);

    count = sai_qos_maps_get_max_queue_idx()*SAI_QOS_MAX_PFC_PRI;
    sai_rc = sai_qos_map_list_alloc(p_map_node, count);

    if(sai_rc != SAI_STATUS_SUCCESS){
        return sai_rc;
    }

    for(pfc_pri = 0; pfc_pri < SAI_QOS_MAX_PFC_PRI; pfc_pri ++)
    {
        p_map_node->map_to_value.list[pfc_pri].key.prio = pfc_pri;
        p_map_node->map_to_value.list[pfc_pri].value.queue_index
                               = sai_switch_max_queues_get();

        p_map_node->map_to_value.list[pfc_pri+SAI_QOS_MAX_PFC_PRI].key.prio = pfc_pri;
        p_map_node->map_to_value.list[pfc_pri+SAI_QOS_MAX_PFC_PRI].value.queue_index
                               = sai_switch_max_queues_get();
    }
    return sai_rc;

}

sai_status_t sai_qos_map_pfc_pri_to_queue_map_value_set (dn_sai_qos_map_t *p_map_node,
                                                         sai_qos_map_list_t map_list,
                                                         dn_sai_operations_t op_type)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    if((op_type == SAI_OP_CREATE) || ((op_type == SAI_OP_SET)
                                      && p_map_node->map_to_value.list == NULL)){

        sai_rc = sai_qos_map_pfc_pri_to_queue_default_set (p_map_node);

        if(sai_rc != SAI_STATUS_SUCCESS){
            return sai_rc;
        }
    }
    sai_rc = sai_qos_map_pfc_pri_to_queue_list_update (p_map_node, map_list);
    return sai_rc;
}

