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
 * @file sai_vm_qos.c
 *
 * @brief This file contains the initialization, default settings of
 *        QOS objects and accessor functions for static data strcutures in
 *        VM Environment.
 */

#include "sai_npu_qos.h"

#include "saistatus.h"
#include "saitypes.h"
#include "sai_vm_qos.h"
#include "std_bit_masks.h"
#include "sai_qos_util.h"
#include "std_assert.h"
#include <inttypes.h>

dn_sai_vm_qos_tcb_t g_sai_vm_qos_tcb;
dn_sai_vm_qos_tcb_t* sai_vm_qos_access_tcb(void)
{
    return (&g_sai_vm_qos_tcb);
}
static sai_status_t sai_vm_qos_global_init (void)
{
    sai_status_t status = SAI_STATUS_SUCCESS;


    g_sai_vm_qos_tcb.queue_bitmap =
        std_bitmap_create_array (SAI_VM_QOS_MAX_SUPPORTED_QUEUES);

    if(g_sai_vm_qos_tcb.queue_bitmap == NULL)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    g_sai_vm_qos_tcb.scheduler_bitmap =
        std_bitmap_create_array (SAI_VM_QOS_MAX_SCHEDULER_PROFILES);

    if(g_sai_vm_qos_tcb.scheduler_bitmap == NULL)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    g_sai_vm_qos_tcb.scheduler_group_bitmap =
        std_bitmap_create_array (SAI_VM_QOS_MAX_SCHEDULER_GROUPS);

    if(g_sai_vm_qos_tcb.scheduler_group_bitmap == NULL)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    g_sai_vm_qos_tcb.buffer_profile_bitmap =
        std_bitmap_create_array (SAI_VM_QOS_MAX_BUFFER_PROFILES);

    if(g_sai_vm_qos_tcb.buffer_profile_bitmap == NULL)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    g_sai_vm_qos_tcb.wred_bitmap =
        std_bitmap_create_array (SAI_VM_QOS_MAX_WRED_PROFILES);

    if(g_sai_vm_qos_tcb.wred_bitmap == NULL)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    g_sai_vm_qos_tcb.maps_bitmap =
        std_bitmap_create_array (SAI_VM_QOS_MAX_MAPS);

    if(g_sai_vm_qos_tcb.maps_bitmap == NULL)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    g_sai_vm_qos_tcb.policer_bitmap =
        std_bitmap_create_array (SAI_VM_QOS_MAX_POLICERS);

    if(g_sai_vm_qos_tcb.policer_bitmap == NULL)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    return status;
}

static sai_status_t sai_vm_qos_port_init (dn_sai_qos_port_t *p_qos_port_node)
{
    sai_status_t       sai_rc = SAI_STATUS_SUCCESS;

    /* TODO */
    return sai_rc;
}

sai_status_t sai_qos_port_attribute_get(sai_npu_object_id_t port_id,
                                        sai_port_attr_t port_attr,
                                        sai_attribute_value_t *value)
{
    STD_ASSERT(value != NULL);
    dn_sai_qos_port_t *p_port = NULL;

    p_port = sai_qos_port_node_get(port_id);

    if(p_port == NULL){
        SAI_QOS_LOG_ERR("Port node not found for port id 0x%"PRIx64"",port_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    SAI_QOS_LOG_TRACE ("Attr id %d get for port_id 0x%"PRIx64"", port_attr,
                       port_id);

    switch(port_attr){
        /*Fall through */
        case SAI_PORT_ATTR_POLICER_ID:
        case SAI_PORT_ATTR_QOS_DEFAULT_TC:
        case SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP:
        case SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP:
        case SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP:
        case SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP:
        case SAI_PORT_ATTR_QOS_TC_TO_DOT1P_MAP:
        case SAI_PORT_ATTR_QOS_TC_TO_DSCP_MAP:
        case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
        case SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
            return SAI_STATUS_ATTR_NOT_SUPPORTED_0;

        case SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP:
            value->oid =  p_port->maps_id[SAI_QOS_MAP_DOT1P_TO_TC_AND_COLOR];
            break;

        case SAI_PORT_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP:
            value->oid =  p_port->maps_id[SAI_QOS_MAP_DSCP_TO_TC_AND_COLOR];
            break;

        case SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP:
            value->oid =  p_port->maps_id[SAI_QOS_MAP_TC_TO_QUEUE];
            break;

        case SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP:
            value->oid =  p_port->maps_id[SAI_QOS_MAP_TC_TO_PRIORITY_GROUP];
            break;

        case SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP:
            value->oid =  p_port->maps_id[SAI_QOS_MAP_PFC_PRIORITY_TO_QUEUE];
            break;

        case SAI_PORT_ATTR_QOS_WRED_PROFILE_ID:
            value->oid = p_port->wred_id;
            break;

        case SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID:
            value->oid = p_port->scheduler_id;
            break;

        case SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID:
            value->oid = p_port->policer_id[SAI_QOS_POLICER_TYPE_STORM_FLOOD];
            break;

        case SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID:
            value->oid = p_port->policer_id[SAI_QOS_POLICER_TYPE_STORM_BCAST];
            break;

        case SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID:
            value->oid = p_port->policer_id[SAI_QOS_POLICER_TYPE_STORM_MCAST];
            break;

        case SAI_PORT_ATTR_NUMBER_OF_PRIORITY_GROUPS:
            value->u32 = p_port->num_pg;
            break;

        case SAI_PORT_ATTR_PRIORITY_GROUP_LIST:
            return sai_qos_port_get_pg_list(p_port->port_id, &value->objlist);

        default:
            return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_qos_switch_default_tc_set(uint_t default_tc)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    /* TODO */
    return sai_rc;
}

static sai_npu_qos_api_t sai_vm_qos_api_table = {
    sai_vm_qos_global_init,
    sai_vm_qos_port_init,
    sai_vm_qos_switch_default_tc_set
};

sai_npu_qos_api_t* sai_vm_qos_api_query (void)
{
    return &sai_vm_qos_api_table;
}

