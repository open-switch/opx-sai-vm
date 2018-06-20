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
/*!
 * \file   sai_switch.c
 * \brief  SAI Switch functionality
 * \date   12-2014
 * */

#include "std_type_defs.h"
#include "sai.h"
#include "std_utils.h"
#include "sai_modules_init.h"
#include "sai_infra_api.h"
#include "sai_common_infra.h"
#include "sai_switch_utils.h"
#include "std_assert.h"
#include "sai_port_main.h"
#include "sai_npu_switch.h"
#include "sai_switch_utils.h"
#include "sai_gen_utils.h"
#include "sai_fdb_api.h"
#include "sai_fdb_main.h"
#include "sai_vlan_api.h"
#include "sai_l3_api.h"
#include "sai_shell.h"
#include "sai_stp_api.h"
#include "sai_l3_api_utils.h"
#include "sai_hostif_api.h"
#include "sai_common_infra.h"
#include "sai_switch_init_config.h"
#include "sai_qos_api_utils.h"
#include "sai_hash_api_utils.h"
#include "sai_vlan_api.h"
#include "sai_vlan_common.h"
#include "sai_bridge_main.h"

static const dn_sai_attribute_entry_t dn_sai_switch_attr[] = {
    {SAI_SWITCH_ATTR_NUMBER_OF_ACTIVE_PORTS, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_PORT_NUMBER, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_MAX_NUMBER_OF_SUPPORTED_PORTS, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_PORT_LIST, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_PORT_MAX_MTU, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_CPU_PORT, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_FDB_TABLE_SIZE, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_L3_NEIGHBOR_TABLE_SIZE, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_L3_ROUTE_TABLE_SIZE, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_LAG_MEMBERS, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_NUMBER_OF_LAGS, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_ECMP_MEMBERS, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_NUMBER_OF_ECMP_GROUPS, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_NUMBER_OF_UNICAST_QUEUES, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_NUMBER_OF_MULTICAST_QUEUES, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_NUMBER_OF_QUEUES, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_NUMBER_OF_CPU_QUEUES, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_OPER_STATUS, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_MAX_TEMP, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_FDB_DST_USER_META_DATA_RANGE, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_ROUTE_DST_USER_META_DATA_RANGE, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_NEIGHBOR_DST_USER_META_DATA_RANGE, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_PORT_USER_META_DATA_RANGE, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_VLAN_USER_META_DATA_RANGE, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_ACL_USER_META_DATA_RANGE, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_ACL_USER_TRAP_ID_RANGE, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_TRAFFIC_CLASSES, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUP_HIERARCHY_LEVELS, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUPS_PER_HIERARCHY_LEVEL, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_CHILDS_PER_SCHEDULER_GROUP, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_TOTAL_BUFFER_SIZE, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_INGRESS_BUFFER_POOL_NUM, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_EGRESS_BUFFER_POOL_NUM, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_AVAILABLE_IPV4_ROUTE_ENTRY, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_AVAILABLE_IPV6_ROUTE_ENTRY, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_AVAILABLE_IPV4_NEXTHOP_ENTRY, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_AVAILABLE_IPV6_NEXTHOP_ENTRY, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_AVAILABLE_IPV4_NEIGHBOR_ENTRY, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_AVAILABLE_IPV6_NEIGHBOR_ENTRY, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_AVAILABLE_NEXT_HOP_GROUP_ENTRY, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_AVAILABLE_NEXT_HOP_GROUP_MEMBER_ENTRY, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_AVAILABLE_L2MC_ENTRY, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_AVAILABLE_IPMC_ENTRY, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_AVAILABLE_ACL_TABLE, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_AVAILABLE_ACL_TABLE_GROUP, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_ECMP_HASH, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_LAG_HASH, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_RESTART_WARM, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_RESTART_TYPE, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_MIN_PLANNED_RESTART_INTERVAL, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_NV_STORAGE_SIZE, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_MAX_ACL_ACTION_COUNT, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_MAX_ACL_RANGE_COUNT, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_ACL_CAPABILITY, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_SWITCHING_MODE, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_SRC_MAC_ADDRESS, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_FDB_AGING_TIME, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_ECMP_DEFAULT_SYMMETRIC_HASH, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_ECMP_HASH_IPV4, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_ECMP_HASH_IPV4_IN_IPV4, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_ECMP_HASH_IPV6, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_LAG_DEFAULT_SYMMETRIC_HASH, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_LAG_HASH_IPV4, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_LAG_HASH_IPV4_IN_IPV4, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_LAG_HASH_IPV6, false, false, false, true, true, true },
    {SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_QOS_DEFAULT_TC, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_MAP, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_QOS_DOT1P_TO_COLOR_MAP, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_MAP, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_QOS_DSCP_TO_COLOR_MAP, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_QOS_TC_TO_QUEUE_MAP, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_SWITCH_SHELL_ENABLE, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_SWITCH_PROFILE_ID, false, true, false, true, true, true },
    {SAI_SWITCH_ATTR_SWITCH_HARDWARE_INFO, false, true, false, true, true, true },
    {SAI_SWITCH_ATTR_FIRMWARE_PATH_NAME, false, true, false, true, true, true },
    {SAI_SWITCH_ATTR_INIT_SWITCH, true, true, false, true, true, true },
    {SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_TAM_EVENT_NOTIFY, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_FAST_API_ENABLE, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_MIRROR_TC, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_ACL_STAGE_INGRESS, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_ACL_STAGE_EGRESS, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_SEGMENTROUTE_MAX_SID_DEPTH, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_SEGMENTROUTE_TLV_TYPE, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_QOS_NUM_LOSSLESS_QUEUES, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_QUEUE_PFC_DEADLOCK_NOTIFY, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_PFC_DLR_PACKET_ACTION, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_PFC_TC_DLD_INTERVAL_RANGE, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_PFC_TC_DLD_INTERVAL, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_PFC_TC_DLR_INTERVAL_RANGE, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_PFC_TC_DLR_INTERVAL, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_SUPPORTED_PROTECTED_OBJECT_TYPE, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_TPID_OUTER_VLAN, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_TPID_INNER_VLAN, false, false, false, false, false, false },
    {SAI_SWITCH_ATTR_INGRESS_ACL, false, true, true, true, true, true },
    {SAI_SWITCH_ATTR_EGRESS_ACL, false, true, true, true, true, true },
};

/**
 * Function returning the attribute array for Switch object and
 * count of the total number of attributes.
 */
static inline void dn_sai_switch_attr_table_get (const dn_sai_attribute_entry_t **p_attr_table,
                                                           uint_t *p_attr_count)
{
    *p_attr_table = &dn_sai_switch_attr [0];

    *p_attr_count = (sizeof(dn_sai_switch_attr)) /
                     (sizeof(dn_sai_switch_attr[0]));
}


static sai_status_t dn_sai_switch_attr_list_validate (sai_object_type_t obj_type,
                                            uint32_t attr_count,
                                            const sai_attribute_t *attr_list,
                                            dn_sai_operations_t op_type)
{
    const dn_sai_attribute_entry_t  *p_attr_table = NULL;
    uint_t                           max_attr_count = 0;
    sai_status_t                     status = SAI_STATUS_FAILURE;

    dn_sai_switch_attr_table_get (&p_attr_table, &max_attr_count);

    if ((p_attr_table == NULL) || (max_attr_count == 0)) {
        /* Not expected */
        SAI_SWITCH_LOG_ERR ("Unable to find attribute id table.");

        return status;
    }

    status = sai_attribute_validate (attr_count, attr_list, p_attr_table,
                                     op_type, max_attr_count);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_SWITCH_LOG_ERR ("Attr list validation failed. op-type: %d, Error: %d.",
                         op_type, status);
    } else {

        SAI_SWITCH_LOG_TRACE ("Attr list validation passed. op-type: %d, Error: %d.",
                           op_type, status);
    }

    return status;
}

static sai_status_t sai_switch_mac_address_set(const sai_mac_t *mac)
{
    sai_status_t status = SAI_STATUS_SUCCESS;

    STD_ASSERT(mac != NULL);
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    status = sai_router_npu_api_get()->router_mac_set (mac);

    if (status != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_ERR("Failed to configure Router MAC address.");
        return status;
    }

    memcpy(sai_switch_info_ptr->switch_mac_addr, mac,
           sizeof(sai_switch_info_ptr->switch_mac_addr));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_switch_common_counter_refresh_interval_set(const sai_attribute_value_t *value)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;

    STD_ASSERT(value != NULL);

    SAI_SWITCH_LOG_TRACE("SAI NPU counter refresh interval %d seconds set", value->u32);
    sai_switch_lock();

    ret_val = sai_switch_npu_api_get()->counter_refresh_interval_set(value->u32);
    if(ret_val != SAI_STATUS_SUCCESS) {
        sai_switch_unlock();

        SAI_SWITCH_LOG_ERR("SAI NPU counter refresh interval %d set failed with error %d",
                           value->u32, ret_val);
        return ret_val;
    }

    /* Update the cache with configured counter refresh interval */
    sai_switch_counter_refresh_interval_set(value->u32);

    sai_switch_unlock();

    return ret_val;
}

/* Read the counter interval from the cached switch info */
static sai_status_t sai_switch_common_counter_refresh_interval_get(sai_attribute_value_t *value)
{
    STD_ASSERT(value != NULL);

    sai_switch_lock();
    /* @todo relook if NPU support reading counter interval from SDK */
    value->u32 = sai_switch_counter_refresh_interval_get();
    sai_switch_unlock();

    SAI_SWITCH_LOG_TRACE("SAI counter refresh interval is  %d seconds", value->u32);

    return SAI_STATUS_SUCCESS;
}

static inline sai_status_t sai_switch_get_lag_attribute (sai_attribute_t *attr)
{
    sai_status_t ret_val = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
    STD_ASSERT (attr != NULL);

    switch(attr->id)
    {
        case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM:
            ret_val = sai_switch_npu_api_get()->lag_hash_algorithm_get ((sai_hash_algorithm_t *)
                                                                        &attr->value.s32);
            break;

        case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED:
            ret_val = sai_switch_npu_api_get()->lag_hash_seed_value_get ((sai_switch_hash_seed_t *)
                                                                         (&attr->value.u32));
            break;

        case SAI_SWITCH_ATTR_LAG_MEMBERS:
            ret_val = sai_switch_npu_api_get()->lag_max_members_get (&attr->value.u32);
            break;

        case SAI_SWITCH_ATTR_NUMBER_OF_LAGS:
            ret_val = sai_switch_npu_api_get()->lag_max_number_get (&attr->value.u32);
            break;

        default:
            SAI_SWITCH_LOG_TRACE ("Invalid Attribute Id %d in list", attr->id);
            break;
    }
    return ret_val;
}

static sai_status_t sai_switch_ecmp_max_paths_attr_set (uint32_t max_paths)
{
    sai_status_t status = sai_router_ecmp_max_paths_set (max_paths);

    if (status == SAI_STATUS_SUCCESS) {

        sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

        sai_switch_info_ptr->max_ecmp_paths = max_paths;

    } else {

        SAI_SWITCH_LOG_TRACE ("Failed to set ECMP max paths attribute.");
    }

    return status;
}

/* Inline functions to handle get and set attributes */
static inline sai_status_t sai_switch_get_ecmp_attribute (sai_attribute_t *attr)
{
    sai_status_t ret_val = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
    uint_t       max_ecmp = 0;

    STD_ASSERT(attr != NULL);

    switch(attr->id)
    {
        case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM:
            ret_val = sai_switch_npu_api_get()->ecmp_hash_algorithm_get ((sai_hash_algorithm_t *)
                                                                         &attr->value.s32);
            break;

        case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED:
            ret_val = sai_switch_npu_api_get()->ecmp_hash_seed_value_get ((sai_switch_hash_seed_t *)
                                                                          &attr->value.u32);
            break;

        case SAI_SWITCH_ATTR_ECMP_MEMBERS:
            ret_val = sai_switch_max_ecmp_paths_get(&max_ecmp);
            attr->value.u32 = max_ecmp;
            break;

        default:
            SAI_SWITCH_LOG_TRACE ("Invalid Attribute Id %d in list", attr->id);
            break;
    }
    return ret_val;
}

static sai_status_t sai_switch_get_gen_attribute(sai_attribute_t *attr)
{
    sai_status_t ret_val = SAI_STATUS_ATTR_NOT_SUPPORTED_0;
    uint_t       max_vrf = 0;

    STD_ASSERT(attr != NULL);
    switch(attr->id)
    {
        case SAI_SWITCH_ATTR_SWITCHING_MODE:
            ret_val = sai_switch_npu_api_get()->switching_mode_get(
                           (sai_switch_switching_mode_t *)&attr->value.s32);
            break;
        case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
            ret_val = sai_switch_mac_address_get(&attr->value.mac);
            break;
        case SAI_SWITCH_ATTR_PORT_NUMBER:
            attr->value.u32 = sai_switch_get_max_lport();
            ret_val = SAI_STATUS_SUCCESS;
            break;
        case SAI_SWITCH_ATTR_PORT_MAX_MTU:
            attr->value.u32 = sai_switch_get_max_port_mtu();
            ret_val = SAI_STATUS_SUCCESS;
            break;
        case SAI_SWITCH_ATTR_PORT_LIST:
            ret_val = sai_port_list_get((sai_object_list_t *)
                                        &attr->value.objlist);
            break;
        case SAI_SWITCH_ATTR_CPU_PORT:
            attr->value.oid = sai_switch_cpu_port_obj_id_get();
            ret_val = SAI_STATUS_SUCCESS;
            break;
        case SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS:
            ret_val = sai_switch_max_virtual_routers_get(&max_vrf);
            attr->value.u32 = max_vrf;
            break;
        case SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID:
            ret_val = sai_bridge_default_id_get(&attr->value.oid);
            break;
        case SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED:
            break;
        case SAI_SWITCH_ATTR_OPER_STATUS:
            ret_val = sai_switch_oper_status_get((sai_switch_oper_status_t *)
                                                 &attr->value.s32);
            break;
        case SAI_SWITCH_ATTR_MAX_TEMP:
            ret_val = sai_switch_npu_api_get()->switch_temp_get(&attr->value);
            break;
        case SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL:
            ret_val = sai_switch_common_counter_refresh_interval_get(&attr->value);
            break;
        default:
            SAI_SWITCH_LOG_TRACE("Invalid Attribute Id %d in list", attr->id);
            return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    return ret_val;
}

static sai_status_t sai_switch_get_l2_attribute(sai_attribute_t *attr)
{
    sai_status_t ret_val = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;

    STD_ASSERT(attr != NULL);
    switch(attr->id)
    {
        case SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE:
            ret_val = sai_l2_mcast_cpu_flood_enable_get((bool*)&attr->value.booldata);
            break;
        case SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE:
            ret_val = sai_l2_bcast_cpu_flood_enable_get((bool*)&attr->value.booldata);
            break;
        case SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES:
            ret_val = sai_l2_fdb_get_switch_max_learned_address(&(attr->value.u32));
            break;
        case SAI_SWITCH_ATTR_FDB_TABLE_SIZE:
            ret_val = sai_l2_get_fdb_table_size(attr);
            break;
        case SAI_SWITCH_ATTR_FDB_AGING_TIME:
            ret_val = sai_l2_fdb_get_aging_time(&(attr->value.u32));
            break;
        case SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION:
            ret_val = sai_l2_fdb_ucast_miss_action_get(attr);
            break;
        case SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION:
            ret_val = sai_l2_fdb_bcast_miss_action_get(attr);
            break;
        case SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION:
            ret_val = sai_l2_fdb_mcast_miss_action_get(attr);
            break;
        case SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID:
            ret_val = sai_l2_stp_default_instance_id_get(attr);
            break;
        case SAI_SWITCH_ATTR_DEFAULT_VLAN_ID:
            attr->value.oid = sai_vlan_id_to_vlan_obj_id(SAI_INIT_DEFAULT_VLAN);
            ret_val = SAI_STATUS_SUCCESS;
            break;
        default:
            SAI_SWITCH_LOG_TRACE ("Invalid Attribute Id %d in list", attr->id);
            ret_val = SAI_STATUS_INVALID_ATTRIBUTE_0;
    }
    return ret_val;
}

static void sai_switch_info_update_l2_attribute (const sai_attribute_t *attr)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get ();

    STD_ASSERT (attr != NULL);

    switch (attr->id) {
        case SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE:
            sai_switch_info_ptr->isBcastCpuFloodEnable =
                (bool) attr->value.booldata;
            break;

        case SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE:
            sai_switch_info_ptr->isMcastCpuFloodEnable =
                (bool) attr->value.booldata;
            break;

        case SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES:
            sai_switch_info_ptr->max_mac_learn_limit =
                (uint32_t) attr->value.u32;
            break;

        case SAI_SWITCH_ATTR_FDB_AGING_TIME:
            sai_switch_info_ptr->fdb_aging_time = (uint32_t) attr->value.u32;
            break;

        case SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION:
            sai_switch_info_ptr->fdbUcastMissPktAction =
                (int32_t) attr->value.s32;
            break;

        case SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION:
            sai_switch_info_ptr->fdbBcastMissPktAction =
                (int32_t) attr->value.s32;
            break;

        case SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION:
            sai_switch_info_ptr->fdbMcastMissPktAction =
                (int32_t) attr->value.s32;
            break;
    }
}

static sai_status_t sai_switch_cache_lag_hash_info (const sai_attribute_t *attr)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get ();

    STD_ASSERT (attr != NULL);
    STD_ASSERT (sai_switch_info_ptr != NULL);

    switch(attr->id)
    {
        case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM:
            sai_switch_info_ptr->lag_hash_algo = attr->value.s32;
            break;

        case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED:
            sai_switch_info_ptr->lag_hash_seed = attr->value.u32;
            break;

        default:
            SAI_SWITCH_LOG_TRACE ("Invalid Attribute Id %d in list", attr->id);
            return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_switch_cache_ecmp_hash_info (const sai_attribute_t *attr)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get ();

    STD_ASSERT (attr != NULL);
    STD_ASSERT (sai_switch_info_ptr != NULL);

    switch(attr->id)
    {
        case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM:
            sai_switch_info_ptr->ecmp_hash_algo = attr->value.s32;
            break;

        case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED:
            sai_switch_info_ptr->ecmp_hash_seed = attr->value.u32;
            break;

        default:
            SAI_SWITCH_LOG_TRACE ("Invalid Attribute Id %d in list", attr->id);
            return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_switch_set_lag_attribute (const sai_attribute_t *attr)
{
    sai_status_t ret_val = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;

    STD_ASSERT (attr != NULL);

    switch(attr->id)
    {
        case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM:
            ret_val = sai_switch_npu_api_get()->lag_hash_algorithm_set ((sai_hash_algorithm_t)
                                                                        attr->value.s32);
            break;

        case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED:
            ret_val = sai_switch_npu_api_get()->lag_hash_seed_value_set ((sai_switch_hash_seed_t)
                                                                         attr->value.u32);
            break;

        case SAI_SWITCH_ATTR_LAG_MEMBERS:
        case SAI_SWITCH_ATTR_NUMBER_OF_LAGS:
            ret_val = SAI_STATUS_INVALID_ATTRIBUTE_0;
            break;

        default:
            SAI_SWITCH_LOG_TRACE ("Invalid Attribute Id %d in list", attr->id);
            break;
    }

    if (ret_val == SAI_STATUS_SUCCESS)
    {
        sai_switch_cache_lag_hash_info (attr);
    }

    return ret_val;
}

static sai_status_t sai_switch_set_ecmp_attribute (const sai_attribute_t *attr)
{
    sai_status_t ret_val = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;

    STD_ASSERT(attr != NULL);

    switch(attr->id)
    {
        case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM:
            ret_val = sai_switch_npu_api_get()->ecmp_hash_algorithm_set ((sai_hash_algorithm_t)
                                                                         attr->value.s32);
            break;

        case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED:
            ret_val = sai_switch_npu_api_get()->ecmp_hash_seed_value_set ((sai_switch_hash_seed_t)
                                                                          attr->value.u32);
            break;

        case SAI_SWITCH_ATTR_ECMP_MEMBERS:
            ret_val = sai_switch_ecmp_max_paths_attr_set (attr->value.u32);
            break;

        default:
            SAI_SWITCH_LOG_TRACE ("Invalid Attribute Id %d in list", attr->id);
            break;
    }

    if (ret_val == SAI_STATUS_SUCCESS)
    {
        sai_switch_cache_ecmp_hash_info (attr);
    }
    return ret_val;
}

static sai_status_t sai_switch_set_gen_attribute(const sai_attribute_t *attr)
{
    sai_status_t ret_val = SAI_STATUS_ATTR_NOT_SUPPORTED_0;

    STD_ASSERT(attr != NULL);
    switch(attr->id)
    {
        case SAI_SWITCH_ATTR_SWITCHING_MODE:
            ret_val = sai_switch_npu_api_get()->switching_mode_set(
                                 (sai_switch_switching_mode_t)attr->value.s32);
            break;
        case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
            ret_val = sai_switch_mac_address_set(&attr->value.mac);
            break;
        case SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL:
            ret_val = sai_switch_common_counter_refresh_interval_set(&attr->value);
            break;
        default:
            SAI_SWITCH_LOG_TRACE("Invalid Attribute Id %d in list", attr->id);
            return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }
    return ret_val;
}

static sai_status_t sai_switch_set_l2_attribute(const sai_attribute_t *attr)
{
    sai_status_t ret_val = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;

    STD_ASSERT(attr != NULL);
    switch(attr->id)
    {
        case SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE:
            ret_val = sai_l2_bcast_cpu_flood_enable_set((bool)
                                                        attr->value.booldata);
            break;
        case SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE:
            ret_val = sai_l2_mcast_cpu_flood_enable_set((bool)
                                                        attr->value.booldata);
            break;
        case SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES:
            ret_val = sai_l2_fdb_set_switch_max_learned_address(attr->value.u32);
            break;
        case SAI_SWITCH_ATTR_FDB_AGING_TIME:
            ret_val = sai_l2_fdb_set_aging_time(attr->value.u32);
            break;
        case SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION:
            ret_val = sai_l2_fdb_ucast_miss_action_set(attr);
            break;
        case SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION:
            ret_val = sai_l2_fdb_bcast_miss_action_set(attr);
            break;
        case SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION:
            ret_val = sai_l2_fdb_mcast_miss_action_set(attr);
            break;
        default:
            SAI_SWITCH_LOG_TRACE("Invalid Attribute Id %d in list", attr->id);
            ret_val = SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    /* Update the new attribute setting in the switch info DB */
    if (ret_val == SAI_STATUS_SUCCESS) {
        sai_switch_info_update_l2_attribute (attr);
    }

    return ret_val;
}

static sai_status_t sai_switch_get_hostif_attribute(sai_attribute_t *attr)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;

    STD_ASSERT(attr != NULL);
    switch(attr->id)
    {
        case SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP:
            ret_val = sai_hostif_get_default_trap_group(attr);
            break;
        default:
            SAI_SWITCH_LOG_TRACE("Invalid hostif switch attribute %d", attr->id);
            ret_val = SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    return ret_val;
}

static sai_status_t sai_switch_get_qos_attribute(sai_attribute_t *attr)
{
    sai_status_t ret_val = SAI_STATUS_SUCCESS;

    STD_ASSERT(attr != NULL);

    switch(attr->id)
    {
        case SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_TRAFFIC_CLASSES:
            attr->value.u32 = sai_switch_max_traffic_class_get ();
            break;

        case SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUP_HIERARCHY_LEVELS:
            attr->value.u32 = sai_switch_max_hierarchy_levels_get ();
            break;

        case SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUPS_PER_HIERARCHY_LEVEL:
            ret_val = SAI_STATUS_ATTR_NOT_IMPLEMENTED_0;
            break;

        case SAI_SWITCH_ATTR_QOS_DEFAULT_TC:
            attr->value.u32 = sai_switch_default_tc_get ();
            break;

        case SAI_SWITCH_ATTR_TOTAL_BUFFER_SIZE:
            attr->value.u32 = sai_switch_max_buffer_size_get ();
            break;

        case SAI_SWITCH_ATTR_INGRESS_BUFFER_POOL_NUM:
            attr->value.u32 = sai_switch_ing_max_buf_pools_get ();
            break;

        case SAI_SWITCH_ATTR_EGRESS_BUFFER_POOL_NUM:
            attr->value.u32 = sai_switch_egr_max_buf_pools_get ();
            break;

        case SAI_SWITCH_ATTR_NUMBER_OF_UNICAST_QUEUES:
            attr->value.u32 = sai_switch_max_uc_queues_get ();
            break;

        case SAI_SWITCH_ATTR_NUMBER_OF_MULTICAST_QUEUES:
            attr->value.u32 = sai_switch_max_mc_queues_get ();
            break;

        case SAI_SWITCH_ATTR_NUMBER_OF_QUEUES:
            attr->value.u32 = sai_switch_max_queues_get ();
            break;

        case SAI_SWITCH_ATTR_NUMBER_OF_CPU_QUEUES:
            attr->value.u32 = sai_switch_num_cpu_queues_get();
            break;

        default:
            SAI_SWITCH_LOG_TRACE("Invalid qos switch attribute %d", attr->id);
            ret_val = SAI_STATUS_INVALID_ATTRIBUTE_0;
            break;
    }

    return ret_val;
}

static sai_status_t sai_switch_get_acl_attribute(sai_attribute_t *attr)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(attr != NULL);
    sai_rc = sai_acl_npu_api_get()->get_acl_attribute(attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_ERR("Failed to fetch ACL priority");
    }

    return sai_rc;
}


static sai_status_t sai_switch_modules_initialize(void)
{
    sai_status_t ret_val = SAI_STATUS_UNINITIALIZED;

    if((ret_val = sai_port_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI port init failed with err %d",ret_val);
        return ret_val;
    }

    if((ret_val = sai_fdb_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI FDB init failed with err %d",ret_val);
        return ret_val;
    }

    if((ret_val = sai_vlan_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI VLAN init failed with err %d",ret_val);
        return ret_val;
    }

    if((ret_val = sai_l2mc_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI L2MC init failed with err %d",ret_val);
        return ret_val;
    }

    if((ret_val = sai_lag_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI LAG init failed with err %d",ret_val);
        return ret_val;
    }

    if((ret_val = sai_shell_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI Shell init failed with err %d",ret_val);
        return ret_val;
    }

    if ((ret_val = sai_router_init ()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI Router Initialization failed with error: %d",
                            ret_val);
        return ret_val;
    }

    if ((ret_val = sai_acl_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI ACL init failed with error %d",ret_val);
        return ret_val;
    }
    if ((ret_val = sai_stp_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI STP init failed with error %d",ret_val);
        return ret_val;
    }

    if ((ret_val = sai_mirror_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI Mirror init failed with error %d",ret_val);
        return ret_val;
    }
    if ((ret_val = sai_hostintf_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI Host Interface Init failed with error %d",ret_val);
        return ret_val;
    }
    if ((ret_val = sai_qos_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI Qos init failed with error %d",ret_val);
        return ret_val;
    }
    if ((ret_val = sai_samplepacket_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI SamplePacket init failed with error %d",
                            ret_val);
        return ret_val;
    }

    if ((ret_val = sai_udf_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI UDF init failed with error %d", ret_val);
        return ret_val;
    }

    if ((ret_val = sai_hash_obj_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI Hash object init failed with error %d", ret_val);
        return ret_val;
    }

    if ((ret_val = sai_tunnel_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI Tunnel init failed with error %d", ret_val);
        return ret_val;
    }

    if ((ret_val = sai_bridge_init()) != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI Bridge init failed with error %d", ret_val);
        return ret_val;
    }
    sai_port_init_event_notification();
    sai_ports_linkscan_enable();

    return ret_val;
}

static sai_status_t sai_switch_attribute_config(sai_switch_profile_id_t profile_id)
{
    int key_len = 0;
    uint_t value = 0;
    const char *key = NULL;
    const char *value_str = NULL;
    int ret = 0;

    const sai_service_method_table_t *service_method_table = sai_service_method_table_get();
    if (service_method_table == NULL) {
        return SAI_STATUS_NOT_SUPPORTED;
    }

    if (service_method_table->profile_get_next_value == NULL) {
        return SAI_STATUS_NOT_SUPPORTED;
    }

    do {
        ret = service_method_table->profile_get_next_value(profile_id,
                                                           &key,
                                                           &value_str);
        if ((ret == 0) && (key != NULL) && (value_str != NULL)) {
            key_len = strlen(key);
            value = (uint_t) strtol(value_str, NULL, 0);
            if (strncmp(key, SAI_KEY_FDB_TABLE_SIZE, key_len) == 0) {
                sai_switch_fdb_table_size_set(value);
                SAI_SWITCH_LOG_TRACE("FDB table size is %d", value);
            } else if (strncmp(key, SAI_KEY_L3_ROUTE_TABLE_SIZE, key_len) == 0) {
                sai_switch_l3_route_table_size_set(value);
                SAI_SWITCH_LOG_TRACE("L3 table size is %d", value);
            } else if (strncmp(key, SAI_KEY_L3_NEIGHBOR_TABLE_SIZE, key_len) == 0) {
                sai_switch_l3_host_table_size_set(value);
                SAI_SWITCH_LOG_TRACE("Neighbor table size is %d", value);
            } else if (strncmp(key, SAI_KEY_NUM_LAG_MEMBERS, key_len) == 0) {
                sai_switch_num_lag_members_set(value);
                SAI_SWITCH_LOG_TRACE("Number of LAG Members is %d", value);
            } else if (strncmp(key, SAI_KEY_NUM_LAGS, key_len) == 0) {
                sai_switch_num_lag_set(value);
                SAI_SWITCH_LOG_TRACE("Number of LAG count is %d", value);
            } else if (strncmp(key, SAI_KEY_NUM_ECMP_MEMBERS, key_len) == 0) {
                sai_switch_num_ecmp_members_set(value);
                SAI_SWITCH_LOG_TRACE("Number of ECMP Members is %d", value);
            } else if (strncmp(key, SAI_KEY_NUM_ECMP_GROUPS, key_len) == 0) {
                sai_switch_num_ecmp_groups_set(value);
                SAI_SWITCH_LOG_TRACE("Number of ECMP Groups is %d", value);
            } else if (strncmp(key, SAI_KEY_NUM_UNICAST_QUEUES, key_len) == 0) {
                sai_switch_num_unicast_queues_set(value);
                SAI_SWITCH_LOG_TRACE("Number of Unicast Queues is %d", value);
            } else if (strncmp(key, SAI_KEY_NUM_MULTICAST_QUEUES, key_len) == 0) {
                sai_switch_num_multicast_queues_set(value);
                SAI_SWITCH_LOG_TRACE("Number of Multiicast Queues is %d", value);
            } else if (strncmp(key, SAI_KEY_NUM_QUEUES, key_len) == 0) {
                sai_switch_num_queues_set(value);
                SAI_SWITCH_LOG_TRACE("Number of Queues is %d", value);
            } else if (strncmp(key, SAI_KEY_NUM_CPU_QUEUES, key_len) == 0) {
                sai_switch_num_cpu_queues_set(value);
                SAI_SWITCH_LOG_TRACE("Number of CPU Queues is %d", value);
            } else {
                /* unsupported switch attribute */
                continue;
            }
        }
    } while (ret != -1);

    return SAI_STATUS_SUCCESS;
}

static const char *sai_switch_init_config_file_get(sai_switch_profile_id_t profile_id)
{
    const sai_service_method_table_t *service_method_table = sai_service_method_table_get();
    if (service_method_table == NULL) {
        return NULL;
    }

    if (service_method_table->profile_get_value == NULL) {
        return NULL;
    }

    return service_method_table->profile_get_value(profile_id, SAI_KEY_INIT_CONFIG_FILE);
}

static sai_status_t sai_switch_profile_id_initialize(uint32_t profile_id,
                                                  sai_switch_info_t *sai_switch_info)
{
    sai_switch_info->profile_id = profile_id;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t  sai_switch_api_initialize(bool value)
{
    sai_status_t ret_val = SAI_STATUS_SUCCESS;

    if(value != true){
        SAI_SWITCH_LOG_ERR("Switch api init with not supported value");
        return SAI_STATUS_NOT_SUPPORTED;
    }
    ret_val = sai_npu_api_initialize (SAI_DEFAULT_NPU_LIB_NAME);
    if (ret_val != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_ERR("Switch init api failed");
        return ret_val;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_switch_hardware_info_init(sai_s8_list_t switch_hardware_info,
                                                sai_switch_info_t * sai_switch_info)
{
    sai_switch_info->switch_hardware_info = switch_hardware_info;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_switch_firmware_path_init(sai_s8_list_t switch_firmware_info,
                                                sai_switch_info_t * sai_switch_info)
{
    sai_switch_info->microcode_module_name = switch_firmware_info;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_switch_create_attr(sai_switch_attr_t switch_attr,
                                              const sai_attribute_value_t *value,
                                              sai_switch_info_t *sai_switch_info)
{
    sai_status_t ret_val = SAI_STATUS_SUCCESS;

    SAI_PORT_LOG_TRACE("Attribute %d Create for switch",switch_attr);

    STD_ASSERT(value != NULL);


    switch(switch_attr)
    {
        case SAI_SWITCH_ATTR_INIT_SWITCH:
            if(!sai_switch_info->switch_init_flag){
                ret_val = sai_switch_api_initialize (value->booldata);
                sai_switch_info->switch_init_flag = value->booldata;
                //TODO need to get the switch ID from the create_OID
                sai_switch_info->switch_object_id = SAI_DEFAULT_SWITCH_ID;
            }
            break;

        case SAI_SWITCH_ATTR_SWITCH_PROFILE_ID:
            ret_val = sai_switch_profile_id_initialize(value->u32,sai_switch_info);
            break;

        case SAI_SWITCH_ATTR_SWITCH_HARDWARE_INFO:
            ret_val = sai_switch_hardware_info_init(value->s8list,sai_switch_info);
            break;

        case SAI_SWITCH_ATTR_FIRMWARE_PATH_NAME:
            ret_val = sai_switch_firmware_path_init(value->s8list,sai_switch_info);
            break;

        default:
            SAI_SWITCH_LOG_TRACE("default switch create attribute Id %d",switch_attr);
            return SAI_STATUS_SUCCESS;
    }

    if(ret_val != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_CRIT("SAI switch Initialization failed with err %d", ret_val);
    }

    return ret_val;
}

sai_status_t sai_create_switch(sai_object_id_t* switch_id,
                               uint32_t attr_count,
                               const sai_attribute_t *attr_list)
{

    uint32_t             attr_idx;
    sai_switch_info_t *sai_switch_info = NULL;
    const char *sai_init_config_file = NULL;
    sai_status_t ret_val = SAI_STATUS_UNINITIALIZED;

    if(!attr_count) {
        SAI_SWITCH_LOG_ERR("Attr get: number of attributes is zero");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    STD_ASSERT(!(attr_list ==NULL));

    sai_switch_info = sai_switch_info_get();
    if(sai_switch_info == NULL) {
        sai_switch_info = sai_switch_info_alloc();
    }
    STD_ASSERT(sai_switch_info != NULL);

    SAI_SWITCH_LOG_TRACE("Attribute get for %d attributes of switch",attr_count);

    ret_val = dn_sai_switch_attr_list_validate (SAI_OBJECT_TYPE_SWITCH,
            attr_count, attr_list,
            SAI_OP_CREATE);
    if (ret_val != SAI_STATUS_SUCCESS) {

        SAI_SWITCH_LOG_ERR ("Switch creation attr list validation failed.");

        return ret_val;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        ret_val = sai_switch_create_attr(attr_list[attr_idx].id,
                &(attr_list[attr_idx].value), sai_switch_info);

        if(ret_val != SAI_STATUS_SUCCESS) {
            sai_switch_oper_status_set(SAI_SWITCH_OPER_STATUS_FAILED);
            SAI_SWITCH_LOG_ERR("Create switch  attr index %d "
                    "failed with err %d", attr_idx, ret_val);
            return sai_get_indexed_ret_val(ret_val, attr_idx);
        }
    }

    do {
        sai_init_config_file = sai_switch_init_config_file_get(sai_switch_info->profile_id);
        if (sai_init_config_file == NULL) {
            SAI_SWITCH_LOG_TRACE("Using default switch init config file %s", SAI_INIT_CONFIG_FILE);
            ret_val = sai_switch_init_config(sai_switch_info, SAI_INIT_CONFIG_FILE);
        } else {
            SAI_SWITCH_LOG_TRACE("Using switch init config file %s", sai_init_config_file);
            ret_val = sai_switch_init_config(sai_switch_info, sai_init_config_file);
        }
        if(ret_val != SAI_STATUS_SUCCESS) {
            SAI_SWITCH_LOG_TRACE("Switch init config info initialize failed with err %d",ret_val);
            break;
        }

        ret_val = sai_switch_attribute_config(sai_switch_info->profile_id);
        if (ret_val == SAI_STATUS_NOT_SUPPORTED) {
            SAI_SWITCH_LOG_TRACE("Switch init attributes configuration unsupported");
        }

        sai_log_init ();

        if((ret_val = sai_switch_npu_api_get()->switch_init(sai_switch_info))
                != SAI_STATUS_SUCCESS) {
            SAI_SWITCH_LOG_ERR("SAI Switch initialize failed with err %d",ret_val);
            SAI_SWITCH_LOG_CRIT("SAI Switch initialize failed with err %d",ret_val);
            break;
        }

        if((ret_val = sai_switch_modules_initialize()) != SAI_STATUS_SUCCESS) {
            SAI_SWITCH_LOG_CRIT("SAI Internal modules initialize failed with er %d",ret_val);
            break;
        }

        for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
            if(attr_list[attr_idx].id == SAI_SWITCH_ATTR_INIT_SWITCH ||
               attr_list[attr_idx].id == SAI_SWITCH_ATTR_SWITCH_PROFILE_ID ||
            attr_list[attr_idx].id == SAI_SWITCH_ATTR_FIRMWARE_PATH_NAME ||
            attr_list[attr_idx].id == SAI_SWITCH_ATTR_SWITCH_HARDWARE_INFO){
                continue;
            }

            ret_val = sai_switch_set_attribute(sai_switch_info->switch_object_id,&attr_list[attr_idx]);
            if(ret_val != SAI_STATUS_SUCCESS) {
                SAI_SWITCH_LOG_ERR("default Create switch  attr index %d "
                        "failed with err %d", attr_idx, ret_val);
                break;
            }
        }


    } while(0);

    if(ret_val != SAI_STATUS_SUCCESS) {
        sai_switch_oper_status_set(SAI_SWITCH_OPER_STATUS_FAILED);
        SAI_SWITCH_LOG_CRIT("SAI switch Initialization failed with err %d", ret_val);
        return ret_val;
    }

    sai_switch_oper_status_set(SAI_SWITCH_OPER_STATUS_UP);

    *switch_id = sai_switch_info->switch_object_id;

    return ret_val;
}

sai_status_t sai_remove_switch(sai_object_id_t switch_id)
{
    /*TODO: To be filled in later.
    */
    sai_npu_api_uninitialize();

    return SAI_STATUS_SUCCESS;
}

static void sai_switch_default_tc_update(uint_t default_tc)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get ();

    sai_switch_info_ptr->default_tc = default_tc;
}

sai_status_t sai_switch_set_attribute(sai_object_id_t switch_id,const sai_attribute_t *attr)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;

    STD_ASSERT(attr != NULL);
    SAI_SWITCH_LOG_TRACE("Attribute set for %d value", attr->id);

    /* TODO: use function  dn_sai_switch_attr_list_validate to validate the
             attributes
       TODO: actual APIS for each attribute to be filled later
    */
    switch(attr->id)
    {
        case SAI_SWITCH_ATTR_SWITCHING_MODE:
        case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
        case SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL:
            sai_rc = sai_switch_set_gen_attribute(attr);
            break;

        case SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE:
        case SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE:
        case SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES:
        case SAI_SWITCH_ATTR_FDB_AGING_TIME:
        case SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION:
        case SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION:
        case SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION:
            sai_rc = sai_switch_set_l2_attribute(attr);
            break;

        case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM:
        case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED:
        case SAI_SWITCH_ATTR_LAG_MEMBERS:
        case SAI_SWITCH_ATTR_NUMBER_OF_LAGS:
            sai_rc = sai_switch_set_lag_attribute (attr);
            break;

        case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM:
        case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED:
        case SAI_SWITCH_ATTR_ECMP_MEMBERS:
            sai_rc = sai_switch_set_ecmp_attribute (attr);
            break;

        case SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID:
            /* Default STP Instance Id is a READ-ONLY attribute */
            SAI_SWITCH_LOG_TRACE("Setting a READ-ONLY attribute %d", attr->id);
            return SAI_STATUS_INVALID_ATTRIBUTE_0;

        case SAI_SWITCH_ATTR_DEFAULT_VLAN_ID:
            /* Default VLAN Id is a READ-ONLY attribute */
            SAI_SWITCH_LOG_ERR("Setting a READ-ONLY attribute %d", attr->id);
            return SAI_STATUS_INVALID_ATTRIBUTE_0;

        case SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP:
            /* Default HostIf Trap Group is a READ-ONLY attribute */
            SAI_SWITCH_LOG_TRACE("Setting a Host Interface READ-ONLY "
                                 "attribute %d", attr->id);
            return SAI_STATUS_INVALID_ATTRIBUTE_0;

        case SAI_SWITCH_ATTR_QOS_DEFAULT_TC:
            sai_rc = sai_switch_set_qos_default_tc(attr->value.u32);
            if(sai_rc == SAI_STATUS_SUCCESS){
                sai_switch_default_tc_update(attr->value.u32);
            }
            break;

        case SAI_SWITCH_ATTR_ECMP_HASH:
            SAI_SWITCH_LOG_TRACE ("Default ECMP Hash attribute is READ-ONLY");
            return SAI_STATUS_INVALID_ATTRIBUTE_0;

        case SAI_SWITCH_ATTR_LAG_HASH:
            SAI_SWITCH_LOG_TRACE ("Default LAG Hash attribute is READ-ONLY");
            return SAI_STATUS_INVALID_ATTRIBUTE_0;

        case SAI_SWITCH_ATTR_ECMP_HASH_IPV4:
        case SAI_SWITCH_ATTR_ECMP_HASH_IPV6:
        case SAI_SWITCH_ATTR_ECMP_HASH_IPV4_IN_IPV4:
        case SAI_SWITCH_ATTR_LAG_HASH_IPV4:
        case SAI_SWITCH_ATTR_LAG_HASH_IPV6:
        case SAI_SWITCH_ATTR_LAG_HASH_IPV4_IN_IPV4:
            sai_rc = dn_sai_switch_hash_attr_set (attr->id, attr->value.oid);
            break;

            /** @TODO Deprecated. To be cleaned up */
        case SAI_SWITCH_ATTR_LAG_DEFAULT_SYMMETRIC_HASH:
        case SAI_SWITCH_ATTR_ECMP_DEFAULT_SYMMETRIC_HASH:
            sai_rc = SAI_STATUS_SUCCESS;
            break;
        case SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY:
            sai_switch_state_register_callback(attr->value.ptr);
            sai_rc = SAI_STATUS_SUCCESS;
            break;

        case SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY:
            sai_rc = sai_l2_fdb_register_callback(attr->value.ptr);
            break;

        case SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY:
            sai_port_state_register_callback(attr->value.ptr);
            sai_rc = SAI_STATUS_SUCCESS;
            break;

        case SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY:
            sai_hostif_rx_register_callback(attr->value.ptr);
            sai_rc = SAI_STATUS_SUCCESS;
            break;

        case SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY:
            sai_rc = SAI_STATUS_SUCCESS;
            break;

        case SAI_SWITCH_ATTR_INGRESS_ACL:
        case SAI_SWITCH_ATTR_EGRESS_ACL:
            sai_rc = SAI_STATUS_SUCCESS;
            break;

        case SAI_SWITCH_ATTR_SWITCH_SHELL_ENABLE:
            sai_rc = sai_shell_set(attr->value.booldata);
            break;

        default:
            SAI_SWITCH_LOG_TRACE("Invalid Attribute Id %d in list", attr->id);
            return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }
    return sai_rc;
}


sai_status_t sai_switch_get_attribute(sai_object_id_t switch_id,
                                      sai_uint32_t attr_count,
                                      sai_attribute_t *attr_list)
{
    unsigned int attr_id_loop = 0;
    sai_attribute_t *p_attr = NULL;
    sai_status_t sai_rc = SAI_STATUS_FAILURE;

    /* TODO: actual APIS for each attribute to be filled later
    */
    STD_ASSERT(attr_list != NULL);
    if(attr_count == 0)
    {
        SAI_SWITCH_LOG_TRACE("Attribute count is 0");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for(attr_id_loop = 0; attr_id_loop < attr_count; attr_id_loop ++)
    {
        p_attr = &attr_list[attr_id_loop];

        if(p_attr == NULL)
        {
            SAI_SWITCH_LOG_TRACE("Array entry %d is NULL",
                                 attr_id_loop);
            continue;
        }

        SAI_SWITCH_LOG_TRACE("Attribute get for id %d", p_attr->id);

        switch(p_attr->id)
        {
            case SAI_SWITCH_ATTR_PORT_NUMBER:
            case SAI_SWITCH_ATTR_PORT_LIST:
            case SAI_SWITCH_ATTR_CPU_PORT:
            case SAI_SWITCH_ATTR_PORT_MAX_MTU:
            case SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS:
            case SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID:
            case SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED:
            case SAI_SWITCH_ATTR_OPER_STATUS:
            case SAI_SWITCH_ATTR_MAX_TEMP:
            case SAI_SWITCH_ATTR_SWITCHING_MODE:
            case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
            case SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL:
                sai_rc = sai_switch_get_gen_attribute(p_attr);
                break;

            case SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE:
            case SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE:
            case SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES:
            case SAI_SWITCH_ATTR_FDB_TABLE_SIZE:
            case SAI_SWITCH_ATTR_FDB_AGING_TIME:
            case SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION:
            case SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION:
            case SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION:
            case SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID:
            case SAI_SWITCH_ATTR_DEFAULT_VLAN_ID:
                sai_rc = sai_switch_get_l2_attribute(p_attr);
                break;

            case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM:
            case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED:
            case SAI_SWITCH_ATTR_LAG_MEMBERS:
            case SAI_SWITCH_ATTR_NUMBER_OF_LAGS:
                sai_rc = sai_switch_get_lag_attribute (p_attr);
                break;

            case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM:
            case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED:
            case SAI_SWITCH_ATTR_ECMP_MEMBERS:
                sai_rc = sai_switch_get_ecmp_attribute (p_attr);
                break;

            case SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP:
                sai_rc = sai_switch_get_hostif_attribute(p_attr);
                break;

            case SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_TRAFFIC_CLASSES:
            case SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUP_HIERARCHY_LEVELS:
            case SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUPS_PER_HIERARCHY_LEVEL:
            case SAI_SWITCH_ATTR_QOS_DEFAULT_TC:
            case SAI_SWITCH_ATTR_TOTAL_BUFFER_SIZE:
            case SAI_SWITCH_ATTR_INGRESS_BUFFER_POOL_NUM:
            case SAI_SWITCH_ATTR_EGRESS_BUFFER_POOL_NUM:
            case SAI_SWITCH_ATTR_NUMBER_OF_UNICAST_QUEUES:
            case SAI_SWITCH_ATTR_NUMBER_OF_MULTICAST_QUEUES:
            case SAI_SWITCH_ATTR_NUMBER_OF_QUEUES:
            case SAI_SWITCH_ATTR_NUMBER_OF_CPU_QUEUES:
                sai_rc = sai_switch_get_qos_attribute(p_attr);
                break;

            case SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY:
            case SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY:
            case SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY:
            case SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY:
            case SAI_SWITCH_ATTR_FDB_DST_USER_META_DATA_RANGE:
            case SAI_SWITCH_ATTR_ROUTE_DST_USER_META_DATA_RANGE:
            case SAI_SWITCH_ATTR_NEIGHBOR_DST_USER_META_DATA_RANGE:
            case SAI_SWITCH_ATTR_PORT_USER_META_DATA_RANGE:
            case SAI_SWITCH_ATTR_VLAN_USER_META_DATA_RANGE:
            case SAI_SWITCH_ATTR_ACL_USER_META_DATA_RANGE:
                sai_rc = sai_switch_get_acl_attribute(p_attr);
                break;

            case SAI_SWITCH_ATTR_ECMP_HASH:
            case SAI_SWITCH_ATTR_LAG_HASH:
            case SAI_SWITCH_ATTR_ECMP_HASH_IPV4:
            case SAI_SWITCH_ATTR_ECMP_HASH_IPV6:
            case SAI_SWITCH_ATTR_ECMP_HASH_IPV4_IN_IPV4:
            case SAI_SWITCH_ATTR_LAG_HASH_IPV4:
            case SAI_SWITCH_ATTR_LAG_HASH_IPV6:
            case SAI_SWITCH_ATTR_LAG_HASH_IPV4_IN_IPV4:
                sai_rc = dn_sai_switch_hash_attr_get (p_attr);
                break;

            /** @TODO Deprecated. To be cleaned up */
            case SAI_SWITCH_ATTR_LAG_DEFAULT_SYMMETRIC_HASH:
            case SAI_SWITCH_ATTR_ECMP_DEFAULT_SYMMETRIC_HASH:
                sai_rc = SAI_STATUS_SUCCESS;
                break;

            default:
                SAI_SWITCH_LOG_TRACE("Invalid Attribute Id %d in list",p_attr->id);
                return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                               attr_id_loop);
        }
        if(sai_rc != SAI_STATUS_SUCCESS) {
            return sai_get_indexed_ret_val(sai_rc, attr_id_loop);
        }
    }

    return sai_rc;
}

static sai_switch_api_t sai_switch_method_table =
{
    sai_create_switch,
    sai_remove_switch,
    sai_switch_set_attribute,
    sai_switch_get_attribute,
};

/* Switch functions passed to the method table */

sai_switch_api_t* sai_switch_api_query(void)
{
    return (&sai_switch_method_table);
}
