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
 * @file sai_vm_port.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI PORT object in VM environment.
 */

#include "sai_gen_utils.h"
#include "sai_npu_port.h"
#include "sai_port_utils.h"
#include "sai_vm_port_util.h"
#include "saiport.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include "sai_qos_util.h"
#include "sai_vm_qos.h"
#include "sai_vm_vport.h"
#include "sai_vm_vport_event.h"

#include <stddef.h>
#include <inttypes.h>
#include <string.h>



/*
 * VM internal link state change notification handler, which calls back
 * common SAI link state change function
 */
static sai_port_state_change_notification_fn sai_vm_link_state_callback = NULL;

static void sai_vm_port_state_change_ntfn (uint32_t count,
                                           sai_port_oper_status_notification_t *data)
{

    if(data == NULL) {
        SAI_PORT_LOG_TRACE("NULL data in vm port state change notif");
        return;
    }
    if (sai_vm_link_state_callback != NULL) {
        sai_vm_link_state_callback (count, data);
    }
}

sai_status_t sai_port_attr_oper_status_set(const sai_npu_port_id_t npu_port_id,
                                           const sai_port_oper_status_t oper_status)
{
    sai_status_t      sai_status = SAI_STATUS_FAILURE;
    sai_object_id_t   port_id;
    sai_port_info_t  *port_info;
    sai_attribute_t   port_attr;
    sai_port_oper_status_notification_t port_oper_state_change;

    /* extract port information for the given NPU port number*/
    port_info = sai_port_info_get_from_npu_phy_port(npu_port_id);
    if(port_info == NULL) {
        SAI_PORT_LOG_ERR ("failed retrieving port information from npu port (%d)", npu_port_id);
        return sai_status;
    }

    port_id = port_info->sai_port_id;

    /*
     * Retrieve the current state of the interface.
     * Compare the newly reported state change with the existing state and only
     * report if there is a known change.  Avoids unnecessary reporting.
     */
    memset(&port_attr, 0, sizeof(port_attr));
    port_attr.id = SAI_PORT_ATTR_OPER_STATUS;

    sai_status = sai_port_attr_info_cache_get( port_id, port_info, &port_attr);
    if (sai_status != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR ("failed retrieving cached status for port-id (0x%"PRIx64"), error (%d)",
                port_id, sai_status);
        return sai_status;
    }

    /* ignore any changes that have already been recorded (in cache) */
    if (port_attr.value.s32 == oper_status) {
        return sai_status;
    }

    memset(&port_attr, 0, sizeof(sai_attribute_t));
    port_attr.id = SAI_PORT_ATTR_OPER_STATUS;
    port_attr.value.s32 = oper_status;

    /* set the changed attributes in cache */
    sai_status = sai_port_attr_info_cache_set(port_id, port_info, &port_attr);
    if (sai_status != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR ("Oper status %d cache set for port 0x%"PRIx64" failed with err %d",
                port_attr.value.s32, port_id, sai_status);
        return sai_status;
    }

    port_oper_state_change.port_id = port_id;
    port_oper_state_change.port_state = oper_status;
    sai_vm_port_state_change_ntfn(1, &port_oper_state_change);

    return sai_status;
}

static sai_status_t sai_port_attr_admin_state_set(sai_object_id_t sai_port_id,
                                           const sai_port_info_t *sai_port_info,
                                           const sai_attribute_t *attr)
{
    if (!sai_vport_set_admin_state(sai_port_info->phy_port_id, attr->value.booldata)) {
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_port_attr_mtu_size_set(sai_object_id_t sai_port_id,
                                           const sai_port_info_t *sai_port_info,
                                           const sai_attribute_t *attr)
{
    if (!sai_vport_set_mtu_size(sai_port_info->phy_port_id, attr->value.u32)) {
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

/* LOOPBACK mode should be reflected in the port Operational state */
static sai_status_t sai_port_attr_internal_loopback_set (sai_object_id_t port_id,
                                                         sai_port_info_t *sai_port_info,
                                                         const sai_attribute_t *attr)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_set;
    sai_port_oper_status_notification_t port_oper_state_change;

    if((attr == NULL ) || (sai_port_info == NULL)) {
        SAI_PORT_LOG_TRACE("Internal loop back set port_id 0x%"PRIx64" attr is %p"
                           " sai_port_info is %p", port_id,attr,sai_port_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset (&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&port_oper_state_change, 0, sizeof(sai_port_oper_status_notification_t));

    sai_attr_set.id = SAI_PORT_ATTR_OPER_STATUS;
    sai_attr_set.value.s32 = (( (attr->value.s32 == SAI_PORT_INTERNAL_LOOPBACK_MODE_PHY) ||
                                (attr->value.s32 == SAI_PORT_INTERNAL_LOOPBACK_MODE_MAC) ) ?
                              SAI_PORT_OPER_STATUS_UP : SAI_PORT_OPER_STATUS_DOWN);

    ret = sai_port_attr_info_cache_set (port_id, sai_port_info, &sai_attr_set);
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
                                                sai_port_info_t *sai_port_info,
                                                const sai_attribute_t *attr)
{
    /*NULL check is not required for sai_port_info as cpu port will not have port info structure*/
    if(NULL == attr) {
        SAI_PORT_LOG_TRACE("VM Npu set attribute 0x%"PRIx64" attr is NULL sai_port_info is %p",
                           port_id, sai_port_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(NULL == sai_port_info) {
        SAI_PORT_LOG_TRACE("VM Npu set attribute 0x%"PRIx64" attr id=%u sai_port_info is NULL",
                           port_id, (unsigned int)attr->id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch(attr->id)
    {
        case SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE:
            return sai_port_attr_internal_loopback_set (port_id, sai_port_info, attr);

        case SAI_PORT_ATTR_ADMIN_STATE:
            return sai_port_attr_admin_state_set(port_id, sai_port_info, attr);

        case SAI_PORT_ATTR_MTU:
            return sai_port_attr_mtu_size_set(port_id, sai_port_info, attr);

        default:
            return SAI_STATUS_SUCCESS;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_port_attr_current_breakout_mode_get (sai_object_id_t port_id,
                                                             const sai_port_info_t *sai_port_info,
                                                             sai_attribute_value_t *value)
{

    if((value == NULL) || (sai_port_info == NULL)) {
        SAI_PORT_LOG_TRACE("Current breakout mode get Port Id is 0x%"PRIx64""
                           "value is %p sai_port_info is %p",
                           port_id, value,sai_port_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    value->s32 = sai_port_current_breakout_mode_get (sai_port_info);

    SAI_PORT_LOG_TRACE("Port 0x%"PRIx64" current breakout mode is %d",
                       port_id, value->s32);

    return SAI_STATUS_SUCCESS;
}

const speed_desc_t* get_speed_map(size_t *sz)
{
    const static speed_desc_t speed_map[] = {

        { SAI_PORT_CAP_SPEED_TEN_MEG, SAI_PORT_SPEED_TEN_MEG, SAI_ATTR_VAL_SPEED_10M },

        { SAI_PORT_CAP_SPEED_HUNDRED_MEG, SAI_PORT_SPEED_HUNDRED_MEG, SAI_ATTR_VAL_SPEED_100M },

        { SAI_PORT_CAP_SPEED_GIG, SAI_PORT_SPEED_GIG, SAI_ATTR_VAL_SPEED_1G },

        { SAI_PORT_CAP_SPEED_TEN_GIG, SAI_PORT_SPEED_TEN_GIG, SAI_ATTR_VAL_SPEED_10G },

        { SAI_PORT_CAP_SPEED_TWENTY_FIVE_GIG, SAI_PORT_SPEED_TWENTY_FIVE_GIG, SAI_ATTR_VAL_SPEED_25G },

        { SAI_PORT_CAP_SPEED_FORTY_GIG, SAI_PORT_SPEED_FORTY_GIG, SAI_ATTR_VAL_SPEED_40G },

        { SAI_PORT_CAP_SPEED_FIFTY_GIG, SAI_PORT_SPEED_FIFTY_GIG, SAI_ATTR_VAL_SPEED_50G },

        { SAI_PORT_CAP_SPEED_HUNDRED_GIG, SAI_PORT_SPEED_HUNDRED_GIG, SAI_ATTR_VAL_SPEED_100G },

    };

    *sz = sizeof(speed_map)/sizeof(speed_desc_t);
    return speed_map;
}

static sai_status_t sai_port_attr_supported_speed_get(sai_object_id_t port,
                                                      const sai_port_info_t *sai_port_info,
                                                      sai_attribute_value_t *value)
{
    uint32_t count = 0;
    uint32_t index;

    size_t speed_map_sz;
    const speed_desc_t* speed_map= get_speed_map(&speed_map_sz);
    sai_status_t ret_code = SAI_STATUS_SUCCESS;
    sai_uint32_t sai_port_supported_speed_list[SAI_MAX_SUPPORTED_SPEEDS] = {0};

    if(!sai_is_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_info = sai_port_info_get(port);

    if (sai_port_info == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    const speed_desc_t *map_entry = speed_map;

    for (index=0; index < speed_map_sz; ++index) {

    if ((map_entry->speed_cap_bit & sai_port_info->port_speed_capb) != 0) {
        sai_port_supported_speed_list[count++] = map_entry->speed_value;
                                  }
        ++map_entry;
    }

    memcpy(value->u32list.list, sai_port_supported_speed_list, count * sizeof(sai_uint32_t));
    value->u32list.count = count;

    SAI_PORT_LOG_TRACE("Supported speed get successful for port 0x%"PRIx64" count %d",port, value->s32list.count);

    return ret_code;
}

sai_status_t sai_port_attr_eee_get(sai_object_id_t sai_port_id,  sai_attribute_value_t *value)
{
    int eee_state;
    sai_port_info_t *port_info = sai_port_info_get(sai_port_id);

    STD_ASSERT (port_info != NULL);

    value->booldata=port_info->eee_support;
    eee_state = (value->booldata) ? true : false;

    SAI_PORT_LOG_TRACE("EEE state get for npu port %d is %d", sai_port_id, eee_state);

    return SAI_STATUS_SUCCESS;
}
static sai_status_t sai_npu_port_get_attribute (sai_object_id_t port_id,
                                                const sai_port_info_t *sai_port_info,
                                                uint_t attr_count,
                                                sai_attribute_t *attr_list)
{
    uint_t attr_idx = 0;
    sai_status_t ret_code = SAI_STATUS_FAILURE;

    /*NULL check is not required for sai_port_info as cpu port will not have port info structure*/
    if ((attr_count == 0) || (attr_list == NULL)) {
        SAI_PORT_LOG_TRACE("Get attribute Port Id 0x%"PRIx64" attr_list is %p attr_count is %d"
                           " sai_port_info is %p",  port_id, attr_list, attr_count, sai_port_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    const sai_port_attr_info_t *port_attr_info = sai_port_attr_info_read_only_get(port_id, sai_port_info);

    if (port_attr_info == NULL) {
        SAI_PORT_LOG_ERR ("Port attr info not found for port 0x%"PRIx64"",port_id);
        return SAI_STATUS_FAILURE;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {

        switch(attr_list->id) {
            case SAI_PORT_ATTR_SUPPORTED_SPEED:
                ret_code= sai_port_attr_supported_speed_get(port_id, sai_port_info,
                                                            &attr_list[attr_idx].value);
                break;

            case SAI_PORT_ATTR_SPEED:
                attr_list[attr_idx].value.u32=port_attr_info->speed;
                ret_code = SAI_STATUS_SUCCESS;
                break;

            case SAI_PORT_ATTR_TYPE:
                ret_code = sai_port_attr_type_get (port_id, sai_port_info, &attr_list[attr_idx].value);
                break;

            case SAI_PORT_ATTR_HW_LANE_LIST:
                ret_code = sai_port_attr_hw_lane_list_get (port_id, sai_port_info,
                                                           &attr_list[attr_idx].value);
                break;

            case SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE_TYPE:
                ret_code = sai_port_attr_supported_breakout_mode_get (port_id, sai_port_info,
                                                                      &attr_list[attr_idx].value);
                break;

            case SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE:
                ret_code = sai_port_attr_current_breakout_mode_get (port_id, sai_port_info,
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

            case SAI_PORT_ATTR_EEE_ENABLE:
            case SAI_PORT_ATTR_EEE_IDLE_TIME:
            case SAI_PORT_ATTR_EEE_WAKE_TIME:
                ret_code= sai_port_attr_eee_get(port_id, &attr_list[attr_idx].value);
                break;
            case SAI_PORT_ATTR_POLICER_ID:
            case SAI_PORT_ATTR_QOS_DEFAULT_TC:
            case SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP:
            case SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP:
            case SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP:
            case SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP:
            case SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP:
            case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
            case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
            case SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP:
            case SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP:
            case SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID:
            case SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID:
            case SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID:
            case SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID:
            case SAI_PORT_ATTR_NUMBER_OF_INGRESS_PRIORITY_GROUPS:
            case SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST:
                ret_code = sai_qos_port_attribute_get (port_id,
                                                       attr_list[attr_idx].id,
                                                       &attr_list[attr_idx].value);
                break;

            default:
                ret_code = sai_port_attr_info_cache_get (port_id, sai_port_info, &attr_list[attr_idx]);
        }

        if (ret_code != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_TRACE ("Attr get for port id 0x%"PRIx64"'s attr index %d "
                              "attr id %d failed with err %d", port_id, attr_idx,
                              attr_list[attr_idx].id, ret_code);
            return sai_get_indexed_ret_val(ret_code, attr_idx);
        }
    }

    return ret_code;
}

static sai_status_t sai_npu_port_get_stats (sai_object_id_t port_id,
                                            const sai_port_info_t *sai_port_info,
                                            const sai_port_stat_t *counter_ids,
                                            uint32_t number_of_counters,
                                            uint64_t* counters)
{
    /*NULL check is not required for sai_port_info as cpu port will not have port info structure*/
    if ((number_of_counters == 0 ) || (counter_ids == NULL) || (counters == NULL)) {
        SAI_PORT_LOG_TRACE("Get stats port 0x%"PRIx64" counter_ids is %p counters is %p"
                           " sai_port_info is %p number_of_counters is %d", port_id,
                           counter_ids, counters,sai_port_info,number_of_counters);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_port_clear_stats(sai_object_id_t port_id,
                                             const sai_port_info_t *sai_port_info,
                                             const sai_port_stat_t *counter_ids,
                                             uint32_t number_of_counters)
{
    /*NULL check is not required for sai_port_info as cpu port will not have port info structure*/
    if ((number_of_counters == 0) || (counter_ids == 0)) {
        SAI_PORT_LOG_ERR ("Stat clear for port 0x%"PRIx64": number of "
                          "counters is zero", port_id);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(!(counter_ids == NULL));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_port_clear_all_stats(sai_object_id_t port_id,
                                                 const sai_port_info_t *sai_port_info)
{
    return SAI_STATUS_SUCCESS;
}

static void sai_npu_reg_link_state_cb (
                                       sai_port_state_change_notification_fn link_state_cb_fn)
{
    SAI_PORT_LOG_TRACE ("Link State notification registration");

    sai_vm_link_state_callback = link_state_cb_fn;

    // register call back function with lower level (virtual port)
    sai_vm_vport_event_oper_status_callback(sai_port_attr_oper_status_set);
}

/* For any given SAI port, get its control ports's max number of lanes per port */
sai_status_t sai_vm_control_port_max_lanes_get(sai_object_id_t port, uint_t *max_lanes)
{
    *max_lanes = SAI_VM_MAX_LANE_PER_PORT ;

    return SAI_STATUS_SUCCESS;
}

static sai_port_breakout_mode_type_t sai_vm_get_port_breakout_lane_from_count(uint_t port_count,
                                                                              uint_t max_lanes)
{
    uint_t lanes_per_port = 0;

    if (port_count > max_lanes) {
        SAI_PORT_LOG_ERR("Invalid port count %d for create", port_count);
        return SAI_PORT_BREAKOUT_MODE_TYPE_MAX;
    }

    lanes_per_port = max_lanes/port_count;

    switch(lanes_per_port) {
        case 1:
            return SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE;
        case 2:
             return SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE;
        case 4:
             return SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE;
        default:
              SAI_PORT_LOG_ERR("Unknown count %d for breakout", port_count);
    }
    return SAI_PORT_BREAKOUT_MODE_TYPE_MAX;
}

/* Update switching mode based on the port event type and current port speed */
static sai_status_t sai_npu_port_switching_mode_update (uint32_t count,
                                                        sai_port_event_notification_t *data)
{
    /* Not expected to modify the port level config in VM environment */
    return SAI_STATUS_SUCCESS;
}


static sai_status_t sai_npu_port_create (sai_object_id_t *port_id,uint32_t attr_count,
                                         const sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_port_speed_t breakout_speed = 0;
    uint_t attr_idx = 0;
    uint_t npu_id_idx = 0;
    sai_port_info_t *port_info = NULL;
    sai_npu_port_id_t npu_port_id = 0;
    bool npu_port_found = false;
    bool port_speed_found = false;
    sai_port_breakout_mode_type_t breakout_mode = SAI_PORT_BREAKOUT_MODE_TYPE_MAX;
    sai_port_breakout_mode_type_t prev_breakout_mode = SAI_PORT_BREAKOUT_MODE_TYPE_MAX;
    uint_t port_count = 0;
    uint_t max_lanes = 0;
    sai_port_breakout_config_t breakout_cfg;

    memset(&breakout_cfg, 0, sizeof(breakout_cfg));
    if((attr_list == NULL) || (port_id == NULL)) {
        SAI_PORT_LOG_TRACE("VM Npu port create attr_list is %p port_id is %p",attr_list, port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        if (attr_list[attr_idx].id == SAI_PORT_ATTR_SPEED) {
            breakout_speed = attr_list[attr_idx].value.u32;
            port_speed_found = true;
        } else if(attr_list[attr_idx].id == SAI_PORT_ATTR_HW_LANE_LIST) {
            npu_port_found = true;
            npu_port_id = attr_list[attr_idx].value.u32list.list[0];
            npu_id_idx = attr_idx;
            port_count = attr_list[attr_idx].value.u32list.count;
        }
    }

    if (!port_speed_found || !npu_port_found) {
        SAI_PORT_LOG_ERR ("Error mandatory attribute speed is missing in port create");
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }
    port_info = sai_port_info_get_from_npu_phy_port(npu_port_id);

    if(port_info == NULL) {
        SAI_PORT_LOG_ERR ("Error unknow npu port %d", npu_port_id);
        return SAI_STATUS_INVALID_ATTR_VALUE_0 + npu_id_idx;
    }
    if (port_info->port_valid) {
        SAI_PORT_LOG_ERR ("Error npu port %d exists", npu_port_id);
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }
    sai_vm_control_port_max_lanes_get(port_info->sai_port_id , &max_lanes);


    breakout_mode = sai_vm_get_port_breakout_lane_from_count (port_count, max_lanes);
    if(breakout_mode == SAI_PORT_BREAKOUT_MODE_TYPE_MAX) {
        SAI_PORT_LOG_ERR ("Error unknown breakout mode for count %d",
                          attr_list[attr_idx].value.u32list.count);
        return SAI_STATUS_INVALID_ATTR_VALUE_0 + npu_id_idx;
    }

    if(!sai_port_is_breakout_mode_supported(port_info, breakout_mode)) {
        SAI_PORT_LOG_ERR("Breakout mode %d is not supported", breakout_mode);
        return SAI_STATUS_INVALID_ATTR_VALUE_0 + npu_id_idx;
    }

    port_info->port_valid = true;
    prev_breakout_mode = sai_port_current_breakout_mode_get(port_info);

    if (prev_breakout_mode == breakout_mode) {
        *port_id = port_info->sai_port_id;
        return SAI_STATUS_SUCCESS;
    }

    breakout_cfg.new_speed = breakout_speed;
    breakout_cfg.new_mode = breakout_mode;
    breakout_cfg.curr_mode = prev_breakout_mode;
    sai_rc = sai_port_breakout_mode_update (port_info, &breakout_cfg);
    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR ("Breakout mode update failed for port 0x%"PRIx64" with err %d",
                          port_info->sai_port_id, sai_rc);
    } else {
        sai_port_update_valdity_on_create (port_info->sai_port_id, port_info, breakout_mode);
        *port_id = port_info->sai_port_id;
    }
    return sai_rc;
}

static sai_status_t sai_npu_port_remove (sai_object_id_t port_id)
{
    /* Nothing to be done */
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_linkscan_mode_set (sai_object_id_t port_id,
                                               const sai_port_info_t *sai_port_info, bool enable)
{
    /* Nothing to be done */
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_port_dump(sai_object_id_t port_id,sai_port_debug_function_t fn_name)
{
    return SAI_STATUS_SUCCESS;
}

static sai_npu_port_api_t sai_vm_port_api_table = {
    sai_npu_port_create,
    sai_npu_port_remove,
    sai_npu_port_set_attribute,
    sai_npu_port_get_attribute,
    sai_npu_port_get_stats,
    sai_npu_port_clear_stats,
    sai_npu_port_clear_all_stats,
    sai_npu_reg_link_state_cb,
    sai_npu_port_switching_mode_update,
    sai_npu_linkscan_mode_set,
    sai_npu_port_dump,
};

sai_npu_port_api_t* sai_vm_port_api_query (void)
{
    return &sai_vm_port_api_table;
}

