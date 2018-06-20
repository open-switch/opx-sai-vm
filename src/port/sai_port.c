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
* @file sai_port.c
*
* @brief This file contains port level functionality for SAI common component.
*        Adapter Host use these API's for port level Attribute Set/Get, get
*        port counter statistics and get port state change notifications.
*
*************************************************************************/

#include "sai_switch_utils.h"
#include "sai_port_utils.h"
#include "sai_npu_port.h"
#include "sai_mirror_defs.h"
#include "sai_mirror_api.h"
#include "sai_qos_api_utils.h"
#include "sai_qos_util.h"
#include "sai_samplepacket_api.h"
#include "sai_common_infra.h"
#include "sai_common_utils.h"
#include "sai_port_common.h"
#include "sai_gen_utils.h"

#include "saiport.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saimirror.h"

#include "std_type_defs.h"
#include "std_assert.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

/* Internal port event notification handler list for all SAI module,
 * used to invoke all registered internal modules notification function */
static sai_port_event_internal_notf_t port_event_list[SAI_MODULE_MAX];

static bool sai_port_is_attr_valid_on_port(const sai_port_info_t *port_info, sai_attr_id_t attr_id)
{
    if(port_info == NULL)  {
        SAI_PORT_LOG_TRACE("port_info is NULL in attr valid on port");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(port_info->lag_id != SAI_NULL_OBJECT_ID) {
        switch(attr_id) {
            case SAI_PORT_ATTR_PORT_VLAN_ID:
            case SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY:
            case SAI_PORT_ATTR_DROP_UNTAGGED:
            case SAI_PORT_ATTR_DROP_TAGGED:
                return false;
            default:
                return true;
        }
    }
    return true;
}

static const struct {
    sai_port_attr_t attr_id;
    char *string;
} sai_port_attr_to_string[] = {
    {SAI_PORT_ATTR_TYPE,    "SAI_PORT_ATTR_TYPE"},
    {SAI_PORT_ATTR_OPER_STATUS,    "SAI_PORT_ATTR_OPER_STATUS"},
    {SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE_TYPE,    "SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE_TYPE"},
    {SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE,    "SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE"},
    {SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES,    "SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES"},
    {SAI_PORT_ATTR_QOS_QUEUE_LIST,    "SAI_PORT_ATTR_QOS_QUEUE_LIST"},
    {SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS,    "SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS"},
    {SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST,    "SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST"},
    {SAI_PORT_ATTR_SUPPORTED_SPEED,    "SAI_PORT_ATTR_SUPPORTED_SPEED"},
    {SAI_PORT_ATTR_SUPPORTED_FEC_MODE,    "SAI_PORT_ATTR_SUPPORTED_FEC_MODE"},
    {SAI_PORT_ATTR_SUPPORTED_HALF_DUPLEX_SPEED,    "SAI_PORT_ATTR_SUPPORTED_HALF_DUPLEX_SPEED"},
    {SAI_PORT_ATTR_SUPPORTED_AUTO_NEG_MODE,    "SAI_PORT_ATTR_SUPPORTED_AUTO_NEG_MODE"},
    {SAI_PORT_ATTR_SUPPORTED_FLOW_CONTROL_MODE,    "SAI_PORT_ATTR_SUPPORTED_FLOW_CONTROL_MODE"},
    {SAI_PORT_ATTR_SUPPORTED_ASYMMETRIC_PAUSE_MODE,    "SAI_PORT_ATTR_SUPPORTED_ASYMMETRIC_PAUSE_MODE"},
    {SAI_PORT_ATTR_SUPPORTED_MEDIA_TYPE,    "SAI_PORT_ATTR_SUPPORTED_MEDIA_TYPE"},
    {SAI_PORT_ATTR_REMOTE_ADVERTISED_SPEED,    "SAI_PORT_ATTR_REMOTE_ADVERTISED_SPEED"},
    {SAI_PORT_ATTR_REMOTE_ADVERTISED_FEC_MODE,    "SAI_PORT_ATTR_REMOTE_ADVERTISED_FEC_MODE"},
    {SAI_PORT_ATTR_REMOTE_ADVERTISED_HALF_DUPLEX_SPEED,    "SAI_PORT_ATTR_REMOTE_ADVERTISED_HALF_DUPLEX_SPEED"},
    {SAI_PORT_ATTR_REMOTE_ADVERTISED_AUTO_NEG_MODE,    "SAI_PORT_ATTR_REMOTE_ADVERTISED_AUTO_NEG_MODE"},
    {SAI_PORT_ATTR_REMOTE_ADVERTISED_FLOW_CONTROL_MODE,    "SAI_PORT_ATTR_REMOTE_ADVERTISED_FLOW_CONTROL_MODE"},
    {SAI_PORT_ATTR_REMOTE_ADVERTISED_ASYMMETRIC_PAUSE_MODE,    "SAI_PORT_ATTR_REMOTE_ADVERTISED_ASYMMETRIC_PAUSE_MODE"},
    {SAI_PORT_ATTR_REMOTE_ADVERTISED_MEDIA_TYPE,    "SAI_PORT_ATTR_REMOTE_ADVERTISED_MEDIA_TYPE"},
    {SAI_PORT_ATTR_NUMBER_OF_INGRESS_PRIORITY_GROUPS,    "SAI_PORT_ATTR_NUMBER_OF_INGRESS_PRIORITY_GROUPS"},
    {SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST,    "SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST"},
    {SAI_PORT_ATTR_HW_LANE_LIST,    "SAI_PORT_ATTR_HW_LANE_LIST"},
    {SAI_PORT_ATTR_SPEED,    "SAI_PORT_ATTR_SPEED"},
    {SAI_PORT_ATTR_FULL_DUPLEX_MODE,    "SAI_PORT_ATTR_FULL_DUPLEX_MODE"},
    {SAI_PORT_ATTR_AUTO_NEG_MODE,    "SAI_PORT_ATTR_AUTO_NEG_MODE"},
    {SAI_PORT_ATTR_ADMIN_STATE,    "SAI_PORT_ATTR_ADMIN_STATE"},
    {SAI_PORT_ATTR_MEDIA_TYPE,    "SAI_PORT_ATTR_MEDIA_TYPE"},
    {SAI_PORT_ATTR_ADVERTISED_SPEED,    "SAI_PORT_ATTR_ADVERTISED_SPEED"},
    {SAI_PORT_ATTR_ADVERTISED_FEC_MODE,    "SAI_PORT_ATTR_ADVERTISED_FEC_MODE"},
    {SAI_PORT_ATTR_ADVERTISED_HALF_DUPLEX_SPEED,    "SAI_PORT_ATTR_ADVERTISED_HALF_DUPLEX_SPEED"},
    {SAI_PORT_ATTR_ADVERTISED_AUTO_NEG_MODE,    "SAI_PORT_ATTR_ADVERTISED_AUTO_NEG_MODE"},
    {SAI_PORT_ATTR_ADVERTISED_FLOW_CONTROL_MODE,    "SAI_PORT_ATTR_ADVERTISED_FLOW_CONTROL_MODE"},
    {SAI_PORT_ATTR_ADVERTISED_ASYMMETRIC_PAUSE_MODE,    "SAI_PORT_ATTR_ADVERTISED_ASYMMETRIC_PAUSE_MODE"},
    {SAI_PORT_ATTR_ADVERTISED_MEDIA_TYPE,    "SAI_PORT_ATTR_ADVERTISED_MEDIA_TYPE"},
    {SAI_PORT_ATTR_PORT_VLAN_ID,    "SAI_PORT_ATTR_PORT_VLAN_ID"},
    {SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY,    "SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY"},
    {SAI_PORT_ATTR_DROP_UNTAGGED,    "SAI_PORT_ATTR_DROP_UNTAGGED"},
    {SAI_PORT_ATTR_DROP_TAGGED,    "SAI_PORT_ATTR_DROP_TAGGED"},
    {SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE,    "SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE"},
    {SAI_PORT_ATTR_FEC_MODE,    "SAI_PORT_ATTR_FEC_MODE"},
    {SAI_PORT_ATTR_UPDATE_DSCP,    "SAI_PORT_ATTR_UPDATE_DSCP"},
    {SAI_PORT_ATTR_MTU,    "SAI_PORT_ATTR_MTU"},
    {SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID,    "SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID"},
    {SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID,    "SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID"},
    {SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID,    "SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID"},
    {SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE,    "SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE"},
    {SAI_PORT_ATTR_INGRESS_ACL,    "SAI_PORT_ATTR_INGRESS_ACL"},
    {SAI_PORT_ATTR_EGRESS_ACL,    "SAI_PORT_ATTR_EGRESS_ACL"},
    {SAI_PORT_ATTR_INGRESS_MIRROR_SESSION,    "SAI_PORT_ATTR_INGRESS_MIRROR_SESSION"},
    {SAI_PORT_ATTR_EGRESS_MIRROR_SESSION,    "SAI_PORT_ATTR_EGRESS_MIRROR_SESSION"},
    {SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE,    "SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE"},
    {SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE,    "SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE"},
    {SAI_PORT_ATTR_POLICER_ID,    "SAI_PORT_ATTR_POLICER_ID"},
    {SAI_PORT_ATTR_QOS_DEFAULT_TC,    "SAI_PORT_ATTR_QOS_DEFAULT_TC"},
    {SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP,    "SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP"},
    {SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP,    "SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP"},
    {SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP,    "SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP"},
    {SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP,    "SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP"},
    {SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP,    "SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP"},
    {SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP,    "SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP"},
    {SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP,    "SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP"},
    {SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP,    "SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP"},
    {SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_PRIORITY_GROUP_MAP,    "SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_PRIORITY_GROUP_MAP"},
    {SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP,    "SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP"},
    {SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID,    "SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID"},
    {SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST,    "SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST"},
    {SAI_PORT_ATTR_QOS_EGRESS_BUFFER_PROFILE_LIST,    "SAI_PORT_ATTR_QOS_EGRESS_BUFFER_PROFILE_LIST"},
    {SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL,    "SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL"},
    {SAI_PORT_ATTR_META_DATA,    "SAI_PORT_ATTR_META_DATA"},
    {SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST,    "SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST"},
    {SAI_PORT_ATTR_HW_PROFILE_ID,    "SAI_PORT_ATTR_HW_PROFILE_ID"},
    {SAI_PORT_ATTR_EEE_ENABLE,    "SAI_PORT_ATTR_EEE_ENABLE"},
    {SAI_PORT_ATTR_EEE_IDLE_TIME,    "SAI_PORT_ATTR_EEE_IDLE_TIME"},
    {SAI_PORT_ATTR_EEE_WAKE_TIME,    "SAI_PORT_ATTR_EEE_WAKE_TIME"},
    {SAI_PORT_ATTR_LOCATION_LED,    "SAI_PORT_ATTR_LOCATION_LED"},
    {SAI_PORT_ATTR_REMOTE_ADVERTISED_OUI_CODE,    "SAI_PORT_ATTR_REMOTE_ADVERTISED_OUI_CODE"},
    {SAI_PORT_ATTR_ADVERTISED_OUI_CODE,    "SAI_PORT_ATTR_ADVERTISED_OUI_CODE"}
};
static const size_t sai_port_attr_to_string_size =
                    (sizeof(sai_port_attr_to_string)/
                     sizeof(sai_port_attr_to_string[0]));


static char *sai_port_attr_id_to_string (sai_attr_id_t id)
{
    uint_t attr_id = 0;

    for (attr_id = 0; attr_id < sai_port_attr_to_string_size; attr_id++) {
        if (id == sai_port_attr_to_string[attr_id].attr_id) {
            return sai_port_attr_to_string[attr_id].string;
        }
    }
    return "Invalid ID";
}
static sai_status_t sai_port_apply_attribute (sai_object_id_t port_id,
                                    const sai_attribute_t *attr)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_port_info_t *port_info = sai_port_info_get(port_id);

    if(attr == NULL)  {
        SAI_PORT_LOG_TRACE("attr is %p for port 0x%"PRIx64" in apply attribute",attr, port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    SAI_PORT_LOG_TRACE("Attribute set attr id %d(%s) val %lu for port 0x%"PRIx64"", attr->id,
                       sai_port_attr_id_to_string(attr->id),attr->value.u64, port_id);

    do {
        if(!sai_is_obj_id_port(port_id)) {
            SAI_PORT_LOG_ERR("port id 0x%"PRIx64" is not a port object", port_id);
            ret = SAI_STATUS_INVALID_OBJECT_TYPE;
            break;
        }

        if(!sai_is_port_valid(port_id)) {
            SAI_PORT_LOG_ERR("port id 0x%"PRIx64" is not valid", port_id);
            ret = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        /*
         * Duplicate checks making problem in 4x10G,4x25G and 2x50G breakout
         * ports not oper up during bootup and CLI, so removing the code now.
         * This will be fixed later.
         */
        if(!sai_port_is_attr_valid_on_port(port_info, attr->id)) {
            SAI_PORT_LOG_ERR("Attr %d is not valid on port 0x%"PRIx64"", attr->id, port_id);
            ret = SAI_STATUS_INVALID_ATTRIBUTE_0;
            break;
        }
        switch (attr->id)
        {

            case SAI_PORT_ATTR_INGRESS_MIRROR_SESSION:
                ret = sai_mirror_handle_per_port (port_id, attr,
                                                  SAI_MIRROR_DIR_INGRESS);
                if (ret != SAI_STATUS_SUCCESS) {
                    SAI_PORT_LOG_ERR("Ingress Mirror set on port id 0x%"PRIx64" failed", port_id);
                }
                break;

            case SAI_PORT_ATTR_EGRESS_MIRROR_SESSION:
                ret = sai_mirror_handle_per_port (port_id, attr,
                                                  SAI_MIRROR_DIR_EGRESS);
                if (ret != SAI_STATUS_SUCCESS) {
                    SAI_PORT_LOG_ERR("Egress Mirror set on port id 0x%"PRIx64" failed", port_id);
                }
                break;

            case SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE:
                ret = sai_samplepacket_handle_per_port (port_id, attr,
                                                        SAI_SAMPLEPACKET_DIR_INGRESS);
                if (ret != SAI_STATUS_SUCCESS) {
                    SAI_PORT_LOG_ERR("Ingress SamplePacket set on port "
                                     "id 0x%"PRIx64" failed", port_id);
                }
                break;

            case SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE:
                ret = sai_samplepacket_handle_per_port (port_id, attr,
                                                        SAI_SAMPLEPACKET_DIR_EGRESS);
                if (ret != SAI_STATUS_SUCCESS) {
                    SAI_PORT_LOG_ERR("Egress SamplePacket set on port "
                                     "id 0x%"PRIx64" failed", port_id);
                }
                break;
                /** Fall through */
            case SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP:
            case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
            case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
            case SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP:
            case SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP:
            case SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP:
            case SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP:
            case SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP:
            case SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP:
                ret = sai_qos_map_on_port_set(port_id, attr,
                                              sai_get_map_type_from_port_attr(attr->id));
                if(ret != SAI_STATUS_SUCCESS)
                {
                    SAI_PORT_LOG_ERR("Map set failed for map %d set failed on "
                                     "portid 0x%"PRIx64"",attr->id, port_id);
                }
                break;

            case SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID:
                ret = sai_qos_port_scheduler_set(port_id, attr);
                if(ret != SAI_STATUS_SUCCESS)
                {
                    SAI_PORT_LOG_ERR("Port Scheduler set failed on "
                                     "portid 0x%"PRIx64"", port_id);
                }

                break;
            case SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST:
            case SAI_PORT_ATTR_QOS_EGRESS_BUFFER_PROFILE_LIST:
                ret = SAI_STATUS_NOT_IMPLEMENTED;
                break;
                /** Fall through **/
            case SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID:
            case SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID:
            case SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID:
            case SAI_PORT_ATTR_POLICER_ID:
                ret = sai_port_attr_storm_control_policer_set(port_id, attr);
                if(ret != SAI_STATUS_SUCCESS)
                {
                    SAI_PORT_LOG_ERR("Storm control set failed for %d on"
                                     "portid 0x%"PRIx64"",attr->id, port_id);
                }
                break;

            case SAI_PORT_ATTR_INGRESS_ACL:
            case SAI_PORT_ATTR_EGRESS_ACL:
                ret = SAI_STATUS_SUCCESS;
                break;

            default:
                ret = sai_port_npu_api_get()->port_set_attribute(port_id, port_info, attr);
                if(ret != SAI_STATUS_SUCCESS) {
                    SAI_PORT_LOG_ERR("Attr set for port id 0x%"PRIx64" failed with err %d",
                                     port_id, ret);
                }
                break;
        }

        if(ret != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_ERR("SAI Error code is %d", ret);
            break;
        }

        /* Update the port attribute info cache with the new attribute value */
        ret = sai_port_attr_info_cache_set(port_id, port_info, attr);
        if(ret != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_ERR("Attr cache info update failed for port id 0x%"PRIx64" with err %d",
                             port_id, ret);
            break;
        }

    } while(0);

    return ret;
}

static sai_status_t sai_port_set_attribute(sai_object_id_t port_id,
                                    const sai_attribute_t *attr)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;

    STD_ASSERT (attr != NULL);
    sai_port_lock();

    sai_rc = sai_port_apply_attribute(port_id, attr);

    sai_port_unlock();
    return sai_rc;
}

sai_status_t sai_port_get_attribute(sai_object_id_t port_id,
                                    uint32_t attr_count,
                                    sai_attribute_t *attr_list)
{
    sai_status_t     ret = SAI_STATUS_FAILURE;
    sai_port_info_t *sai_port_info = NULL;
    uint_t           attr_idx = 0;

    if(!attr_count) {
        SAI_PORT_LOG_ERR("Attr get: number of attributes is zero");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(!(attr_list ==NULL));

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        SAI_PORT_LOG_TRACE("Attribute get attr_Id %s attributes of port 0x%"PRIx64"",
                            sai_port_attr_id_to_string(attr_list[attr_idx].id), port_id);
    }

    sai_port_lock();
    do {
        if(!sai_is_obj_id_port(port_id)) {
            SAI_PORT_LOG_ERR("port id 0x%"PRIx64" is not a port object", port_id);
            ret = SAI_STATUS_INVALID_OBJECT_TYPE;
            break;
        }
        if(!sai_is_port_valid(port_id)) {
            SAI_PORT_LOG_ERR("port id 0x%"PRIx64" is not valid", port_id);
            ret = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_port_info = sai_port_info_get (port_id);

        ret = SAI_STATUS_SUCCESS;
        for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
            if(!sai_port_is_attr_valid_on_port(sai_port_info, attr_list[attr_idx].id)) {
                SAI_PORT_LOG_ERR("Attr %d is not valid on port 0x%"PRIx64"",
                                 attr_list[attr_idx].id, port_id);
                ret = sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0, attr_idx);
                break;
            }
        }
        if(ret != SAI_STATUS_SUCCESS) {
            break;
        }

        ret = sai_port_npu_api_get()->port_get_attribute(port_id, sai_port_info,
                                                         attr_count, attr_list);
        if(ret != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_TRACE("Attr get for port id 0x%"PRIx64" failed with err %d",
                                port_id, ret);
            break;
        }
    } while(0);

    sai_port_unlock();
    return ret;
}

sai_status_t sai_port_get_stats(sai_object_id_t port_id,
                                uint32_t number_of_counters,
                                const sai_port_stat_t *counter_ids,
                                uint64_t* counters)
{
    sai_status_t     ret = SAI_STATUS_FAILURE;
    sai_port_info_t *sai_port_info = NULL;

    if(number_of_counters == 0) {
        SAI_PORT_LOG_ERR("Stat get: number of counters is zero");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    STD_ASSERT(!(counter_ids == NULL));
    STD_ASSERT(!(counters == NULL));

    sai_port_lock();
    do {
        if(!sai_is_obj_id_port(port_id)) {
            SAI_PORT_LOG_ERR("port id 0x%"PRIx64" is not a port object", port_id);
            ret = SAI_STATUS_INVALID_OBJECT_TYPE;
            break;
        }

        if(!sai_is_port_valid(port_id)) {
            SAI_PORT_LOG_ERR("port id 0x%"PRIx64" is not valid", port_id);
            ret = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_port_info = sai_port_info_get(port_id);

        ret = sai_port_npu_api_get()->port_get_stats(port_id, sai_port_info, counter_ids,
                                                     number_of_counters, counters);
        if(ret != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_ERR("Stats get for port id 0x%"PRIx64" failed with err %d",
                             port_id, ret);
            break;
        }
    } while(0);

    sai_port_unlock();
    return ret;
}

sai_status_t sai_port_clear_stats(sai_object_id_t port_id,
                                  uint32_t number_of_counters,
                                  const sai_port_stat_t *counter_ids)
{
    sai_status_t     ret = SAI_STATUS_FAILURE;
    sai_port_info_t *sai_port_info = NULL;

    if(number_of_counters == 0) {
        SAI_PORT_LOG_ERR("Stat clear: number of counters is zero");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(!(counter_ids == NULL));

    sai_port_lock();
    do {
        if(!sai_is_obj_id_port(port_id)) {
            SAI_PORT_LOG_ERR("port id 0x%"PRIx64" is not a port object", port_id);
            ret = SAI_STATUS_INVALID_OBJECT_TYPE;
            break;
        }

        if(!sai_is_port_valid(port_id)) {
            SAI_PORT_LOG_ERR("port id 0x%"PRIx64" is not valid", port_id);
            ret = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }
        sai_port_info = sai_port_info_get(port_id);

        ret = sai_port_npu_api_get()->port_clear_stats(port_id, sai_port_info, counter_ids,
                                                       number_of_counters);
        if(ret != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_ERR("Stats clear for port id 0x%"PRIx64" failed with err %d",
                             port_id, ret);
            break;
        }
    } while(0);

    sai_port_unlock();
    return ret;
}

sai_status_t sai_port_clear_all_stats(sai_object_id_t port_id)
{
    sai_status_t     ret = SAI_STATUS_FAILURE;
    sai_port_info_t *sai_port_info = NULL;

    SAI_PORT_LOG_TRACE("Clear Statistics counters of port 0x%"PRIx64"", port_id);

    sai_port_lock();
    do {
        if(!sai_is_obj_id_port(port_id)) {
            SAI_PORT_LOG_ERR("port id 0x%"PRIx64" is not a port object", port_id);
            ret = SAI_STATUS_INVALID_OBJECT_TYPE;
            break;
        }

        if(!sai_is_port_valid(port_id)) {
            SAI_PORT_LOG_ERR("port id 0x%"PRIx64" is not valid", port_id);
            ret = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_port_info = sai_port_info_get(port_id);
        ret = sai_port_npu_api_get()->port_clear_all_stats(port_id, sai_port_info);
        if(ret != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_ERR("Stats clear for port id 0x%"PRIx64" failed with err %d",
                             port_id, ret);
            break;
        }
    } while(0);

    sai_port_unlock();
    return ret;
}

/* configure sai port with default value of attributes */
static sai_status_t sai_port_set_attribute_default(sai_object_id_t sai_port_id)
{
    sai_status_t          ret = SAI_STATUS_FAILURE;
    sai_attribute_t       sai_attr_set;
    sai_port_info_t      *sai_port_info = sai_port_info_get(sai_port_id);

    if(sai_is_port_valid(sai_port_id) == false) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not valid port", sai_port_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    const sai_port_attr_info_t *port_attr_info = sai_port_attr_info_read_only_get(sai_port_id,
                                                                                  sai_port_info);
    STD_ASSERT(port_attr_info != NULL);

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    sai_attr_set.id = SAI_PORT_ATTR_ADMIN_STATE;
    sai_attr_set.value.booldata = port_attr_info->admin_state;
    ret = sai_port_set_attribute(sai_port_id, &sai_attr_set);
    if (ret != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("SAI Port ID 0x%"PRIx64" default admin state %d set failed with err %d",
                          sai_port_id, port_attr_info->admin_state, ret);
        return ret;
    }

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    sai_attr_set.id = SAI_PORT_ATTR_FULL_DUPLEX_MODE;
    sai_attr_set.value.booldata = port_attr_info->duplex;
    ret = sai_port_set_attribute(sai_port_id, &sai_attr_set);
    if (ret != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("SAI Port ID 0x%"PRIx64" default duplex state %d set failed with err %d",
                          sai_port_id, port_attr_info->duplex, ret);
        return ret;
    }

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    sai_attr_set.id = SAI_PORT_ATTR_AUTO_NEG_MODE;
    sai_attr_set.value.booldata = port_attr_info->autoneg;
    ret = sai_port_set_attribute(sai_port_id, &sai_attr_set);
    if (ret != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("SAI Port ID 0x%"PRIx64" default autoneg state %d set failed with err %d",
                          sai_port_id, port_attr_info->autoneg, ret);
        return ret;
    }


    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    sai_attr_set.id = SAI_PORT_ATTR_DROP_UNTAGGED;
    sai_attr_set.value.booldata = port_attr_info->drop_untagged;
    ret = sai_port_set_attribute(sai_port_id, &sai_attr_set);
    if (ret != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("SAI Port ID 0x%"PRIx64" default drop untagged %d set failed with err %d",
                          sai_port_id, port_attr_info->drop_untagged, ret);
        return ret;
    }

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    sai_attr_set.id = SAI_PORT_ATTR_DROP_TAGGED;
    sai_attr_set.value.booldata = port_attr_info->drop_tagged;
    ret = sai_port_set_attribute(sai_port_id, &sai_attr_set);
    if (ret != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("SAI Port ID 0x%"PRIx64" default drop tagged %d set failed with err %d",
                          sai_port_id, port_attr_info->drop_tagged, ret);
        return ret;
    }

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    sai_attr_set.id = SAI_PORT_ATTR_UPDATE_DSCP;
    sai_attr_set.value.booldata = port_attr_info->update_dscp;
    ret = sai_port_set_attribute(sai_port_id, &sai_attr_set);
    if (ret != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("SAI Port ID 0x%"PRIx64" default dscp %d set failed with err %d",
                          sai_port_id, port_attr_info->update_dscp, ret);
        return ret;
    }

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    sai_attr_set.id = SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE;
    sai_attr_set.value.s32 = port_attr_info->internal_loopback;
    ret = sai_port_set_attribute(sai_port_id, &sai_attr_set);
    if (ret != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("SAI Port ID 0x%"PRIx64" default internal loopback %d set failed with err %d",
                          sai_port_id, port_attr_info->internal_loopback, ret);
        return ret;
    }

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    sai_attr_set.id = SAI_PORT_ATTR_MTU;
    sai_attr_set.value.u32 = port_attr_info->mtu;
    ret = sai_port_set_attribute(sai_port_id, &sai_attr_set);
    if (ret != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("SAI Port ID 0x%"PRIx64" default mtu %d set failed with err %d",
                          sai_port_id, port_attr_info->mtu, ret);
        return ret;
    }

    return ret;
}

/* Port Link State change notification registration callback
 * Null input will unregister from callback */
void sai_port_state_register_callback(sai_port_state_change_notification_fn port_state_notf_fn)
{
    SAI_PORT_LOG_TRACE("Port Link state change notification registration");

    sai_port_lock();
    sai_port_npu_api_get()->reg_link_state_cb(port_state_notf_fn);
    sai_port_unlock();
}

/* Port Module's internal port_event notification handler */
static void sai_port_event_internal_notif_handler(uint32_t count,
                                                  sai_port_event_notification_t *data)
{
    uint32_t port_idx = 0;
    sai_status_t ret = SAI_STATUS_FAILURE;
    for(port_idx = 0; port_idx < count; port_idx++) {
        if (data[port_idx].port_event == SAI_PORT_EVENT_ADD) {
            ret = sai_port_set_attribute_default(data[port_idx].port_id);
            if (ret != SAI_STATUS_SUCCESS) {
                SAI_PORT_LOG_ERR("SAI Port ID 0x%"PRIx64" default configuration failed with err %d",
                                 data[port_idx].port_id, ret);
            }
        }
    }
    sai_port_npu_api_get()->switching_mode_update(count, data);
}

/* Per SAI module registration for port event notification.
 * Port_event is set to NULL for unregistering from the event
 */
sai_status_t sai_port_event_internal_notif_register(sai_module_t module_id,
                                                    sai_port_event_notification_fn port_event)
{
    if(module_id >= SAI_MODULE_MAX) {
        SAI_PORT_LOG_ERR("Invalid module id %d for port event registration", module_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    SAI_PORT_LOG_TRACE("SAI module %d updates the port event internal notification", module_id);

    sai_port_lock();
    port_event_list[module_id].port_event = port_event;
    sai_port_unlock();

    return SAI_STATUS_SUCCESS;
}

/* Notify all the registered modules with port events  */
static void sai_port_event_internal_notf(uint32_t count,
                                         sai_port_event_notification_t *data)
{
    sai_module_t mod_idx = 0;

    STD_ASSERT(data != NULL);

    for(mod_idx = 0; mod_idx < SAI_MODULE_MAX; mod_idx++) {

        if(port_event_list[mod_idx].port_event != NULL) {
            port_event_list[mod_idx].port_event(count, data);
        }
    }
}

void sai_port_init_event_notification(void)
{
    uint32_t count = 0;
    uint32_t max_count = sai_switch_get_max_lport();
    sai_port_event_notification_t data[max_count];
    sai_port_info_t *port_info = NULL;

    sai_port_lock();
    count = 0;
    /* After NPU Switch Initialization, default port's event default notification for
     * all valid logical Ports. CPU port is not part of this event notification */
    for (port_info = sai_port_info_getfirst(); (port_info != NULL);
         port_info = sai_port_info_getnext(port_info)) {
        if(port_info->port_valid) {
            data[count].port_id = port_info->sai_port_id;
            data[count].port_event = SAI_PORT_EVENT_ADD;
            SAI_PORT_LOG_NTC("SAI port 0x%"PRIx64" is Added", port_info->sai_port_id);
            count++;
        }
    }
    sai_port_unlock();

    sai_port_event_internal_notf(count, data);
}

void sai_ports_linkscan_enable(void)
{
    sai_port_info_t *port_info = NULL;

    sai_port_lock();
    for (port_info = sai_port_info_getfirst(); (port_info != NULL);
         port_info = sai_port_info_getnext(port_info)) {
        if (port_info->port_valid) {
            if ((sai_port_npu_api_get()->linkscan_mode_set != NULL) &&
                    (sai_port_npu_api_get()->linkscan_mode_set(port_info->sai_port_id, port_info, true)
                            != SAI_STATUS_SUCCESS)) {
                SAI_PORT_LOG_ERR("Failed to enable linkcan on port 0x%"PRIx64" in npu",
                                 port_info->sai_port_id);
            }
        }
    }
    sai_port_unlock();
}

/* Get the list of valid logical ports in the switch */
sai_status_t sai_port_list_get(sai_object_list_t *objlist)
{
    sai_status_t ret_code = SAI_STATUS_SUCCESS;
    sai_object_list_t port_list;

    STD_ASSERT(objlist != NULL);

    port_list.list = (sai_object_id_t *) calloc(sai_switch_get_max_lport(),
                                                sizeof(sai_object_id_t));
    if(port_list.list == NULL) {
        SAI_SWITCH_LOG_ERR ("Allocation of Memory failed for port object list");
        return SAI_STATUS_NO_MEMORY;
    }

    sai_port_lock();
    sai_port_logical_list_get(&port_list);
    sai_port_unlock();

    do {
        if(port_list.count == 0) { /* Not likely */
            ret_code = SAI_STATUS_INVALID_ATTR_VALUE_0;
            break;
        }

        if(objlist->count < port_list.count) {
            SAI_SWITCH_LOG_ERR("get port list count %d is less than "
                               "actual port list count %d",
                               objlist->count, port_list.count);

            objlist->count = port_list.count;
            ret_code = SAI_STATUS_BUFFER_OVERFLOW;
            break;
        }

        memcpy(objlist->list, port_list.list, (port_list.count * sizeof(sai_object_id_t)));

        /* Update the object list count with actual port list count */
        objlist->count = port_list.count;
    } while(0);

    free(port_list.list);
    port_list.list = NULL;
    port_list.count = 0;

    return ret_code;
}

sai_status_t sai_port_init(void)
{
    sai_status_t ret_code = SAI_STATUS_SUCCESS;

    /* Initialize port attributes default value */
    sai_port_attr_defaults_init();

    /* SAI port module registration for port_event */
    ret_code = sai_port_event_internal_notif_register(SAI_MODULE_PORT,
                                                      sai_port_event_internal_notif_handler);

    if(ret_code != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("Internal port event notif register failed for PORT module with err %d",
                         ret_code);
    }

    return ret_code;
}

sai_status_t sai_port_remove(sai_object_id_t port_id)
{

    sai_status_t                   sai_rc = SAI_STATUS_SUCCESS;
    sai_port_event_notification_t  data;
    bool                           linkscan_set = false;
    sai_port_info_t               *sai_port_info = NULL;

    SAI_PORT_LOG_TRACE ("SAI port remove, port id 0x%"PRIx64"", port_id);

    memset (&data, 0, sizeof(data));
    sai_port_lock();
    do {
        if(!sai_is_port_valid(port_id)) {
            SAI_PORT_LOG_ERR("port id 0x%"PRIx64" is not valid", port_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if(sai_is_port_in_use(port_id)) {
            SAI_PORT_LOG_ERR("port id 0x%"PRIx64" is in use", port_id);
            sai_rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        sai_port_info = sai_port_info_get(port_id);
        if (sai_port_npu_api_get()->linkscan_mode_set != NULL) {
            if (sai_port_npu_api_get()->linkscan_mode_set(port_id, sai_port_info, false)
                      != SAI_STATUS_SUCCESS) {
                SAI_PORT_LOG_ERR("Failed to disable linkscan on port 0x%"PRIx64" in npu",
                                 port_id);
            } else {
                linkscan_set = true;
            }
        }

        sai_rc = sai_port_npu_api_get()->npu_port_remove(port_id);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_ERR("Failed to remove port id 0x%"PRIx64" in npu",port_id);
            break;
        }

    } while(0);

    sai_port_unlock();

    if(sai_rc == SAI_STATUS_SUCCESS) {
        data.port_id = port_id;
        data.port_event = SAI_PORT_EVENT_DELETE;
        sai_port_event_internal_notf (1,&data);
        sai_port_info->port_valid = false;
    } else if (linkscan_set) { /* Enable linkscan when port removal failed */

        /* Function pointer check is not required as the flag will be enabled
         * only when it is not NULL  and function call to disable linkscan is
         * successful
         */
        if (sai_port_npu_api_get()->linkscan_mode_set(port_id, sai_port_info, true)
                      != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_ERR("Failed to enable linkscan on port 0x%"PRIx64" in npu",
                             port_id);
        }
    }

    return sai_rc;
}

sai_status_t sai_port_create(sai_object_id_t *port_id, sai_object_id_t switch_id,
                            uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_port_info_t               *port_info = NULL;
    uint_t                         attr_idx = 0;
    bool                           sai_port_created = false;
    sai_port_event_notification_t  data;
    sai_status_t                   sai_rc = SAI_STATUS_FAILURE;

    SAI_PORT_LOG_TRACE("SAI port create Start ");

    sai_port_lock ();
    do {
        sai_rc = sai_port_npu_api_get()->npu_port_create(port_id, attr_count, attr_list);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_ERR("Port create failed at npu");
            break;
        }

        sai_port_created = true;
        port_info = sai_port_info_get (*port_id);
        if(port_info == NULL) {
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }
        port_info->port_valid = true;

        for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {
            if (attr_list[attr_idx].id != SAI_PORT_ATTR_HW_LANE_LIST) {
                sai_rc = sai_port_apply_attribute(*port_id, &attr_list[attr_idx]);
                if(sai_rc != SAI_STATUS_SUCCESS) {
                    SAI_PORT_LOG_ERR("Error: Unable to set attribute %d on port 0x%"PRIx64"",
                                      attr_list[attr_idx].id, *port_id);
                    break;
                }
            }
        }
    } while(0);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        if(sai_port_created) {
            port_info->port_valid = false;
            sai_port_npu_api_get()->npu_port_remove(*port_id);
        }
    }
    sai_port_unlock();

    if(sai_rc == SAI_STATUS_SUCCESS) {
        data.port_id = *port_id;
        data.port_event = SAI_PORT_EVENT_ADD;
        sai_port_event_internal_notf (1,&data);

        sai_port_lock();
        if ((sai_port_npu_api_get()->linkscan_mode_set != NULL) &&
                 (sai_port_npu_api_get()->linkscan_mode_set(*port_id, port_info, true)
                      != SAI_STATUS_SUCCESS)) {
            SAI_PORT_LOG_ERR("Failed to enable linkcan on port 0x%"PRIx64" in npu",
                             *port_id);
        }
        sai_port_unlock();
        SAI_PORT_LOG_TRACE("SAI Port create succcess, port id 0x%"PRIx64"", *port_id);
    }


    return sai_rc;
}

static sai_port_api_t sai_port_method_table =
{
    sai_port_create,
    sai_port_remove,
    sai_port_set_attribute,
    sai_port_get_attribute,
    sai_port_get_stats,
    sai_port_clear_stats,
    sai_port_clear_all_stats,
    sai_qos_port_create_port_pool,
    sai_qos_port_remove_port_pool,
    sai_qos_port_set_port_pool_attribute,
    sai_qos_port_get_port_pool_attribute,
    sai_qos_port_get_port_pool_stats,
    sai_qos_port_clear_port_pool_stats
};

sai_port_api_t* sai_port_api_query(void)
{
    return (&sai_port_method_table);
}
