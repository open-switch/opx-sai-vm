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

/**
 * @file sai_vm_port.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI PORT object in VM environment.
 */

#include "sai_gen_utils.h"
#include "sai_npu_port.h"
#include "sai_port_utils.h"
#include "sai_common_utils.h"
#include "saiport.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include "sai_qos_util.h"
#include "sai_vm_qos.h"

/*
 * VM internal link state change notification handler, which calls back
 * common SAI link state change function
 */
static sai_port_state_change_notification_fn sai_vm_link_state_callback = NULL;

static void sai_vm_port_state_change_ntfn (uint32_t count,
                                           sai_port_oper_status_notification_t *data)
{

    if (sai_vm_link_state_callback != NULL) {
        sai_vm_link_state_callback (count, data);
    }
}

/* Admin state should be reflected in the port Operational state */
static sai_status_t sai_port_attr_admin_state_set (sai_object_id_t port_id,
                                                   const sai_attribute_t *attr)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_set;
    sai_port_oper_status_notification_t port_oper_state_change;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&port_oper_state_change, 0, sizeof(sai_port_oper_status_notification_t));

    STD_ASSERT(attr != NULL);

    sai_attr_set.id = SAI_PORT_ATTR_OPER_STATUS;
    sai_attr_set.value.s32 = ((attr->value.booldata) ?
                              SAI_PORT_OPER_STATUS_UP : SAI_PORT_OPER_STATUS_DOWN);


    ret = sai_port_attr_info_cache_set (port_id, &sai_attr_set);
    if (ret != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR ("Oper status %d cache set for port 0x%"PRIx64" failed "
                          "with err %d", sai_attr_set.value.s32, port_id, ret);
        return ret;
    }

    port_oper_state_change.port_id = port_id;
    port_oper_state_change.port_state = sai_attr_set.value.s32;
    sai_vm_port_state_change_ntfn(1, &port_oper_state_change);

    return ret;
}

/* LOOPBACK mode should be reflected in the port Operational state */
static sai_status_t sai_port_attr_internal_loopback_set (sai_object_id_t port_id,
                                                         const sai_attribute_t *attr)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_set;
    sai_port_oper_status_notification_t port_oper_state_change;

    memset (&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&port_oper_state_change, 0, sizeof(sai_port_oper_status_notification_t));

    STD_ASSERT(attr != NULL);

    sai_attr_set.id = SAI_PORT_ATTR_OPER_STATUS;
    sai_attr_set.value.s32 = (( (attr->value.s32 == SAI_PORT_INTERNAL_LOOPBACK_MODE_PHY) ||
                                (attr->value.s32 == SAI_PORT_INTERNAL_LOOPBACK_MODE_MAC) ) ?
                              SAI_PORT_OPER_STATUS_UP : SAI_PORT_OPER_STATUS_DOWN);


    ret = sai_port_attr_info_cache_set (port_id, &sai_attr_set);
    if (ret != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR ("Oper status %d cache set for port 0x%"PRIx64" failed "
                          "with err %d", sai_attr_set.value.s32, port_id, ret);
        return ret;
    }

    port_oper_state_change.port_id = port_id;
    port_oper_state_change.port_state = sai_attr_set.value.s32;
    sai_vm_port_state_change_ntfn(1, &port_oper_state_change);

    return ret;
}

static sai_status_t sai_npu_port_set_attribute (sai_object_id_t port_id,
                                                const sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);

    if (!sai_is_port_valid (port_id)) {
        SAI_PORT_LOG_ERR ("Port id 0x%"PRIx64" is not valid", port_id);

        return SAI_STATUS_INVALID_PORT_NUMBER;
    }

    switch(attr->id)
    {
        case SAI_PORT_ATTR_ADMIN_STATE:
            return sai_port_attr_admin_state_set (port_id, attr);

        case SAI_PORT_ATTR_INTERNAL_LOOPBACK:
            return sai_port_attr_internal_loopback_set (port_id, attr);

        default:
            return SAI_STATUS_SUCCESS;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_port_attr_current_breakout_mode_get (sai_object_id_t port_id,
                                                             sai_attribute_value_t *value)
{
    STD_ASSERT (value != NULL);

    value->s32 = sai_port_current_breakout_mode_get (port_id);

    SAI_PORT_LOG_TRACE("Port 0x%"PRIx64" current breakout mode is %d",
                       port_id, value->s32);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_port_get_attribute (sai_object_id_t port_id,
                                                uint_t attr_count,
                                                sai_attribute_t *attr_list)
{
    uint_t attr_idx = 0;
    sai_status_t ret_code = SAI_STATUS_FAILURE;

    if (!sai_is_port_valid (port_id)) {
        SAI_PORT_LOG_ERR ("Port id 0x%"PRIx64" is not valid", port_id);

        return SAI_STATUS_INVALID_PORT_NUMBER;
    }

    if (!attr_count) {
        SAI_PORT_LOG_ERR ("Attr get for port 0x%"PRIx64": number of "
                          "attributes is zero", port_id);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(attr_list != NULL);

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {

        switch(attr_list->id) {
            case SAI_PORT_ATTR_TYPE:
                ret_code = sai_port_attr_type_get (port_id, &attr_list[attr_idx].value);
                break;

            case SAI_PORT_ATTR_HW_LANE_LIST:
                ret_code = sai_port_attr_hw_lane_list_get (port_id,
                                                           &attr_list[attr_idx].value);
                break;

            case SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE:
                ret_code = sai_port_attr_supported_breakout_mode_get (port_id,
                                                                      &attr_list[attr_idx].value);
                break;

            case SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE:
                ret_code = sai_port_attr_current_breakout_mode_get (port_id,
                                                                    &attr_list[attr_idx].value);
                break;

            case SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES:
                ret_code = sai_qos_port_queue_count_get (port_id, &attr_list[attr_idx].value.u32);
                break;

            case SAI_PORT_ATTR_QOS_QUEUE_LIST:
                ret_code = sai_qos_port_queue_id_list_get (port_id,
                                                           &attr_list[attr_idx].value.objlist);
                break;

            case SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS:
                ret_code = sai_qos_port_sched_group_count_get (port_id,
                                                               &attr_list[attr_idx].value.u32);
                break;

            case SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST:
                ret_code = sai_qos_port_sched_group_id_list_get (port_id,
                                                                 &attr_list[attr_idx].value.objlist);
                break;

            case SAI_PORT_ATTR_POLICER_ID:
            case SAI_PORT_ATTR_QOS_DEFAULT_TC:
            case SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP:
            case SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP:
            case SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP:
            case SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP:
            case SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP:
            case SAI_PORT_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP:
            case SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP:
            case SAI_PORT_ATTR_QOS_TC_TO_DOT1P_MAP:
            case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
            case SAI_PORT_ATTR_QOS_TC_TO_DSCP_MAP:
            case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
            case SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP:
            case SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP:
            case SAI_PORT_ATTR_QOS_WRED_PROFILE_ID:
            case SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID:
            case SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID:
            case SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID:
            case SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID:
            case SAI_PORT_ATTR_NUMBER_OF_PRIORITY_GROUPS:
            case SAI_PORT_ATTR_PRIORITY_GROUP_LIST:
                ret_code = sai_qos_port_attribute_get (port_id,
                                                       attr_list[attr_idx].id,
                                                       &attr_list[attr_idx].value);
                break;


            default:
                ret_code = sai_port_attr_info_cache_get (port_id, &attr_list[attr_idx]);
        }

        if (ret_code != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_ERR ("Attr get for port id 0x%"PRIx64"'s attr index %d "
                              "failed with err %d", port_id, attr_idx, ret_code);
            return sai_get_indexed_ret_val(ret_code, attr_idx);
        }
    }

    return ret_code;
}

static sai_status_t sai_npu_port_get_stats (sai_object_id_t port_id,
                                            const sai_port_stat_t *counter_ids,
                                            uint32_t number_of_counters,
                                            uint64_t* counters)
{
    if (!sai_is_port_valid (port_id)) {
        SAI_PORT_LOG_ERR ("Port id 0x%"PRIx64" is not valid", port_id);

        return SAI_STATUS_INVALID_PORT_NUMBER;
    }

    if (!number_of_counters) {
        SAI_PORT_LOG_ERR ("Stat get for port 0x%"PRIx64": number of "
                          "counters is zero", port_id);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(!(counter_ids == NULL));
    STD_ASSERT(!(counters == NULL));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_port_clear_stats(sai_object_id_t port_id,
                                             const sai_port_stat_t *counter_ids,
                                             uint32_t number_of_counters)
{
    if (!sai_is_port_valid (port_id)) {
        SAI_PORT_LOG_ERR ("Port id 0x%"PRIx64" is not valid", port_id);

        return SAI_STATUS_INVALID_PORT_NUMBER;
    }

    if (number_of_counters == 0) {
        SAI_PORT_LOG_ERR ("Stat clear for port 0x%"PRIx64": number of "
                          "counters is zero", port_id);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(!(counter_ids == NULL));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_port_clear_all_stats(sai_object_id_t port_id)
{
    if (!sai_is_port_valid (port_id)) {
        SAI_PORT_LOG_ERR ("Port id 0x%"PRIx64" is not valid", port_id);

        return SAI_STATUS_INVALID_PORT_NUMBER;
    }

    return SAI_STATUS_SUCCESS;
}

static void sai_npu_reg_link_state_cb (
                                       sai_port_state_change_notification_fn link_state_cb_fn)
{
    SAI_PORT_LOG_TRACE ("Link State notification registration");

    sai_vm_link_state_callback = link_state_cb_fn;
}

static inline sai_port_speed_t sai_vm_port_default_speed_get (sai_port_breakout_mode_type_t mode)
{
    return ((mode == SAI_PORT_BREAKOUT_MODE_1_LANE) ?
            (SAI_PORT_SPEED_FORTY_GIG) : (SAI_PORT_SPEED_TEN_GIG));
}

static sai_status_t sai_npu_port_breakout_set (const sai_port_breakout_t *portbreakout)
{
    uint_t lane = 0, lane_count  = 0, port_idx = 0;
    sai_object_id_t port = 0;
    sai_attribute_t sai_attr_get;
    sai_attribute_t sai_attr_set;
    sai_status_t ret = SAI_STATUS_FAILURE;

    STD_ASSERT (portbreakout != NULL);
    STD_ASSERT (portbreakout->port_list.list != NULL);

    memset (&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset (&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_port_breakout_mode_type_t breakout_mode_lanes = portbreakout->breakout_mode;
    sai_port_speed_t speed = sai_vm_port_default_speed_get(breakout_mode_lanes);

    sai_port_breakout_mode_type_t cur_breakout_mode_lanes =
        sai_port_current_breakout_mode_get (portbreakout->port_list.list[port_idx]);

    /* Port with maximum number of lanes acts as the control port for breakout mode */
    for(port_idx = 0; port_idx < portbreakout->port_list.count; port_idx++) {

        port = portbreakout->port_list.list[port_idx];
        ret = sai_port_max_lanes_get(port, &lane_count);
        if(ret != SAI_STATUS_SUCCESS) {
            SAI_SWITCH_LOG_ERR("Max port lane get failed for port 0x%"PRIx64" with err %d",
                               port, ret);
            return ret;
        }

        if(lane_count >= cur_breakout_mode_lanes) {
            break;
        }
    }

    sai_attr_get.id = SAI_PORT_ATTR_ADMIN_STATE;
    ret = sai_port_attr_info_cache_get (port, &sai_attr_get);
    if (ret != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR ("Admin status cache get for port 0x%"PRIx64" failed "
                          "with err %d", port, ret);
        return ret;
    }

    /* Admin state UP results in breakout mode set */
    if (sai_attr_get.value.booldata) {
        return SAI_STATUS_OBJECT_IN_USE;
    }

    ret = sai_port_breakout_mode_update (port, speed,
                                         breakout_mode_lanes, cur_breakout_mode_lanes);
    if (ret != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR ("Breakout mode update failed for port 0x%"PRIx64" with err %d",
                          port, ret);
        return ret;
    }

    /* Set the port speed based on the breakout mode */
    sai_attr_set.id = SAI_PORT_ATTR_SPEED;
    sai_attr_set.value.u32 = (speed * 1000); // in Mbps

    sai_port_info_t *port_info = sai_port_info_get(port);

    /* @todo re-look at the usage of enum for arith. operations */
    for(lane = 0; lane < breakout_mode_lanes; lane++) {

        ret = sai_port_attr_info_cache_set (port_info->sai_port_id, &sai_attr_set);
        if (ret != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_ERR ("Speed %d cache set for port 0x%"PRIx64" failed with err %d",
                              sai_attr_set.value.u32, port_info->sai_port_id, ret);
            return ret;
        }
        port_info = sai_port_info_getnext(port_info);
    }

    return ret;
}

/* Update switching mode based on the port event type and current port speed */
static sai_status_t sai_npu_port_switching_mode_update (uint32_t count,
                                                        sai_port_event_notification_t *data)
{
    /* Not expected to modify the port level config in VM environment */
    return SAI_STATUS_SUCCESS;
}

static sai_npu_port_api_t sai_vm_port_api_table = {
    sai_npu_port_set_attribute,
    sai_npu_port_get_attribute,
    sai_npu_port_get_stats,
    sai_npu_port_clear_stats,
    sai_npu_port_clear_all_stats,
    sai_npu_reg_link_state_cb,
    sai_npu_port_breakout_set,
    sai_npu_port_switching_mode_update,
};

sai_npu_port_api_t* sai_vm_port_api_query (void)
{
    return &sai_vm_port_api_table;
}

