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
* @file sai_vm_bridge.c
*
* @brief This file contains Bridge implementation APIs for for BCM.
*************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "std_assert.h"
#include "saibridge.h"
#include "sai_bridge_api.h"
#include "sai_bridge_npu_api.h"
#include "sai_bridge_common.h"
#include "sai_port_utils.h"
#include "sai_lag_api.h"
#include "sai_l3_common.h"
#include "sai_l3_util.h"

static dn_sai_id_gen_info_t bridge_obj_gen_info;
static dn_sai_id_gen_info_t bridge_port_obj_gen_info;

/**
 * Vendor attribute array for bridge port containing the attribute
 * values and properties for create,set and get functionality.
 */
static const dn_sai_attribute_entry_t sai_bridge_port_type_sub_port_attr[] =  {
    /*            ID             MC     VC     VS    VG     IMP    SUP */
    { SAI_BRIDGE_PORT_ATTR_TYPE
                               , true , true , false, true , true , true },
    { SAI_BRIDGE_PORT_ATTR_PORT_ID
                               , true , true , false, true , true , true },
    { SAI_BRIDGE_PORT_ATTR_TAGGING_MODE
                               , false, true , true , true , true , true },
    { SAI_BRIDGE_PORT_ATTR_VLAN_ID
                               , true , true , false, true , true , true },
    { SAI_BRIDGE_PORT_ATTR_RIF_ID
                               , false, false, false, false, true , true },
    { SAI_BRIDGE_PORT_ATTR_TUNNEL_ID
                               , false, false, false, false, true , true },
    { SAI_BRIDGE_PORT_ATTR_BRIDGE_ID
                               , true , true , false, true , true , true },
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE
                               , false, true , true , true , true , true },
    { SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES
                               , false, true , true , true , false, false},
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION
                               , false, true , true , true , false, false},
    { SAI_BRIDGE_PORT_ATTR_ADMIN_STATE
                               , false, true , true , true , true , true },
    { SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING
                               , false, true , true , true , true , false},
    { SAI_BRIDGE_PORT_ATTR_EGRESS_FILTERING
                               , false, true , true , true , true , false},
    { SAI_BRIDGE_PORT_ATTR_INGRESS_SPLIT_HORIZON_ID
                               , false, true , true , true , true , true },
    { SAI_BRIDGE_PORT_ATTR_EGRESS_SPLIT_HORIZON_ID
                               , false, true , true , true , true , true },
};

static const dn_sai_attribute_entry_t sai_bridge_port_type_port_attr[] =  {
    /*            ID              MC     VC     VS    VG     IMP    SUP */
    { SAI_BRIDGE_PORT_ATTR_TYPE
                               , true , true , false, true , true , true },
    { SAI_BRIDGE_PORT_ATTR_PORT_ID
                               , true , true , false, true , true , true },
    { SAI_BRIDGE_PORT_ATTR_TAGGING_MODE
                               , false, false, false, false, true , true },
    { SAI_BRIDGE_PORT_ATTR_VLAN_ID
                               , false, false, false, false, true , true },
    { SAI_BRIDGE_PORT_ATTR_RIF_ID
                               , false, false, false, false, true , true },
    { SAI_BRIDGE_PORT_ATTR_TUNNEL_ID
                               , false, false, false, false, true , true },
    { SAI_BRIDGE_PORT_ATTR_BRIDGE_ID
                               , false, true , false, true , true , true },
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE
                               , false, true , true , true , true , true },
    { SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES
                               , false, true , true , true , false, false},
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION
                               , false, true , true , true , false, false},
    { SAI_BRIDGE_PORT_ATTR_ADMIN_STATE
                               , false, true , true , true , true , true },
    { SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING
                               , false, true , true , true , true , true },
    { SAI_BRIDGE_PORT_ATTR_EGRESS_FILTERING
                               , false, true , true , true , true , true },
};

static const dn_sai_attribute_entry_t sai_bridge_port_type_tunnel_attr[] =  {
    /*            ID              MC     VC     VS    VG     IMP    SUP */
    { SAI_BRIDGE_PORT_ATTR_TYPE
                               , true , true , false, true , true , true },
    { SAI_BRIDGE_PORT_ATTR_PORT_ID
                               , false, false, false, false, true , true },
    { SAI_BRIDGE_PORT_ATTR_TAGGING_MODE
                               , false, false, false, false, true , true },
    { SAI_BRIDGE_PORT_ATTR_VLAN_ID
                               , false, false, false, false, true , true },
    { SAI_BRIDGE_PORT_ATTR_RIF_ID
                               , false, false, false, false, true , true },
    { SAI_BRIDGE_PORT_ATTR_TUNNEL_ID
                               , true , true , false, true , true , true },
    { SAI_BRIDGE_PORT_ATTR_BRIDGE_ID
                               , true , true , false, true , true , true },
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE
                               , false, true , true , true , true , true },
    { SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES
                               , false, true , true , true , false, false},
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION
                               , false, true , true , true , false, false},
    { SAI_BRIDGE_PORT_ATTR_ADMIN_STATE
                               , false, true , true , true , true , true },
    { SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING
                               , false, true , true , true , true , false},
    { SAI_BRIDGE_PORT_ATTR_EGRESS_FILTERING
                               , false, true , true , true , true , false},
    { SAI_BRIDGE_PORT_ATTR_INGRESS_SPLIT_HORIZON_ID
                               , false, true , true , true , true , true },
    { SAI_BRIDGE_PORT_ATTR_EGRESS_SPLIT_HORIZON_ID
                               , false, true , true , true , true , true },
};

static const dn_sai_attribute_entry_t sai_bridge_port_type_1d_router_attr[] =  {
    /*            ID              MC     VC     VS    VG     IMP    SUP */
    { SAI_BRIDGE_PORT_ATTR_TYPE
                               , true , true , false, true , true, true },
    { SAI_BRIDGE_PORT_ATTR_PORT_ID
                               , false, false, false, false, false, false },
    { SAI_BRIDGE_PORT_ATTR_TAGGING_MODE
                               , false, false, false, false, false, false },
    { SAI_BRIDGE_PORT_ATTR_VLAN_ID
                               , false, false, false, false, false, false },
    { SAI_BRIDGE_PORT_ATTR_RIF_ID
                               , true,  true,  false, true,  true,  true },
    { SAI_BRIDGE_PORT_ATTR_TUNNEL_ID
                               , false, false, false, false, false, false },
    { SAI_BRIDGE_PORT_ATTR_BRIDGE_ID
                               , true,  true,  false, true,  true , true },
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE
                               , false, false, false, false, false, false },
    { SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES
                               , false, false, false, false, false, false },
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION
                               , false, false, false, false, false, false },
    { SAI_BRIDGE_PORT_ATTR_ADMIN_STATE
                               , false, true,  true,  true,  true,   true },
    { SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING
                               , false, false, false, false, false, false },
    { SAI_BRIDGE_PORT_ATTR_EGRESS_FILTERING
                               , false, false, false, false, false, false },
    { SAI_BRIDGE_PORT_ATTR_INGRESS_SPLIT_HORIZON_ID
                               , false, false, false, false, false, false },
    { SAI_BRIDGE_PORT_ATTR_EGRESS_SPLIT_HORIZON_ID
                               , false, false, false, false, false, false },
};

/**
 * Vendor attribute array for bridge containing the attribute
 * values and properties for create,set and get functionality.
 */
static const dn_sai_attribute_entry_t sai_bridge_type_1q_attr[] =  {
    /*            ID                                           MC     VC     VS    VG     IMP    SUP */
    { SAI_BRIDGE_ATTR_TYPE                                 , true , true , false, true , true , true },
    { SAI_BRIDGE_ATTR_PORT_LIST                            , false, false, false, true , true , true },
    { SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES                , false, true , true , true , true , false},
    { SAI_BRIDGE_ATTR_LEARN_DISABLE                        , false, true , true , true , true , false},
    { SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE   , false, false, false, false, true , false},
    { SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP          , false, false, false, false, true , false},
    { SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE , false, false, false, false, true , false},
    { SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP        , false, false, false, false, true , false},
    { SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE         , false, false, false, false, true , false},
    { SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP                , false, false, false, false, true , false},
};

static const dn_sai_attribute_entry_t sai_bridge_type_1d_attr[] =  {
    /*            ID                                           MC     VC     VS    VG     IMP    SUP */
    { SAI_BRIDGE_ATTR_TYPE                                 , true , true , false, true , true , true },
    { SAI_BRIDGE_ATTR_PORT_LIST                            , false, false, false, true , true , true },
    { SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES                , false, true , true , true , true , false},
    { SAI_BRIDGE_ATTR_LEARN_DISABLE                        , false, true , true , true , true , false},
    { SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE   , false, true , true , true , true , true },
    { SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP          , false, true , true , true , true , true },
    { SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE , false, true , true , true , true , true },
    { SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP        , false, true , true , true , true , true },
    { SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE         , false, true , true , true , true , true },
    { SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP                , false, true , true , true , true , true },
};

static sai_status_t sai_vm_bridge_port_attr_table_get (sai_bridge_port_type_t bridge_port_type,
                                                        const dn_sai_attribute_entry_t **p_vendor,
                                                        uint_t *p_max_attr_count)
{
    if(bridge_port_type == SAI_BRIDGE_PORT_TYPE_PORT) {
        *p_vendor = &sai_bridge_port_type_port_attr[0];

        *p_max_attr_count = sizeof(sai_bridge_port_type_port_attr)/
            sizeof(sai_bridge_port_type_port_attr[0]);
    } else if(bridge_port_type == SAI_BRIDGE_PORT_TYPE_SUB_PORT) {
        *p_vendor = &sai_bridge_port_type_sub_port_attr[0];

        *p_max_attr_count = sizeof(sai_bridge_port_type_sub_port_attr)/
            sizeof(sai_bridge_port_type_sub_port_attr[0]);
    } else if(bridge_port_type == SAI_BRIDGE_PORT_TYPE_TUNNEL) {
        *p_vendor = &sai_bridge_port_type_tunnel_attr[0];

        *p_max_attr_count = sizeof(sai_bridge_port_type_tunnel_attr)/
            sizeof(sai_bridge_port_type_tunnel_attr[0]);
    } else if(bridge_port_type == SAI_BRIDGE_PORT_TYPE_1D_ROUTER) {
        *p_vendor = &sai_bridge_port_type_1d_router_attr[0];

        *p_max_attr_count = sizeof(sai_bridge_port_type_1d_router_attr)/
            sizeof(sai_bridge_port_type_1d_router_attr[0]);
    } else {
        return SAI_STATUS_NOT_SUPPORTED;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_attr_table_get (sai_bridge_type_t bridge_type,
                                                   const dn_sai_attribute_entry_t **p_vendor,
                                                   uint_t *p_max_attr_count)
{
    if(bridge_type == SAI_BRIDGE_TYPE_1Q) {
        *p_vendor = &sai_bridge_type_1q_attr[0];

        *p_max_attr_count = sizeof(sai_bridge_type_1q_attr)/
            sizeof(sai_bridge_type_1q_attr[0]);
    } else if(bridge_type == SAI_BRIDGE_TYPE_1D) {
        *p_vendor = &sai_bridge_type_1d_attr[0];

        *p_max_attr_count = sizeof(sai_bridge_type_1d_attr)/
            sizeof(sai_bridge_type_1d_attr[0]);
    } else {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    return SAI_STATUS_SUCCESS;
}

static bool sai_vm_is_bridge_port_id_in_use(uint64_t obj_id)
{
    sai_object_id_t bridge_port_id =  sai_uoid_create(SAI_OBJECT_TYPE_BRIDGE_PORT, obj_id);
    return sai_is_bridge_port_created(bridge_port_id);
}

static bool sai_vm_is_bridge_id_in_use(uint64_t obj_id)
{
    sai_object_id_t bridge_id = sai_uoid_create(SAI_OBJECT_TYPE_BRIDGE, obj_id);
    return sai_is_bridge_created(bridge_id);
}

static sai_object_id_t sai_vm_bridge_port_id_create(void)
{
    if(SAI_STATUS_SUCCESS == dn_sai_get_next_free_id(&bridge_port_obj_gen_info)) {
        return (sai_uoid_create(SAI_OBJECT_TYPE_BRIDGE_PORT, bridge_port_obj_gen_info.cur_id));
    }
    return SAI_NULL_OBJECT_ID;
}

static sai_object_id_t sai_vm_bridge_id_create(void)
{
    if(SAI_STATUS_SUCCESS == dn_sai_get_next_free_id(&bridge_obj_gen_info)) {
        return (sai_uoid_create(SAI_OBJECT_TYPE_BRIDGE, bridge_obj_gen_info.cur_id));
    }
    return SAI_NULL_OBJECT_ID;
}

static sai_status_t sai_vm_bridge_init(bool init)
{
    if(init) {
        bridge_port_obj_gen_info.cur_id = 0;
        bridge_port_obj_gen_info.is_wrappped = false;
        bridge_port_obj_gen_info.mask = SAI_UOID_NPU_OBJ_ID_MASK;
        bridge_port_obj_gen_info.is_id_in_use = sai_vm_is_bridge_port_id_in_use;

        bridge_obj_gen_info.cur_id = 0;
        bridge_obj_gen_info.is_wrappped = false;
        bridge_obj_gen_info.mask = SAI_UOID_NPU_OBJ_ID_MASK;
        bridge_obj_gen_info.is_id_in_use = sai_vm_is_bridge_id_in_use;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_create(sai_object_id_t      *bridge_id,
                                          dn_sai_bridge_info_t *bridge_info)
{

    if ((bridge_id == NULL) || (bridge_info == NULL)) {
        SAI_BRIDGE_LOG_TRACE("bridge id is %p bridge info is %p in bcm bridge create",
                             bridge_id, bridge_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *bridge_id = sai_vm_bridge_id_create();
    bridge_info->hw_info = NULL;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_remove(dn_sai_bridge_info_t *bridge_info)
{
    if(bridge_info == NULL) {
        SAI_BRIDGE_LOG_TRACE("bridge info is NULL in bcm bridge remove");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(bridge_info->bridge_type == SAI_BRIDGE_TYPE_1Q){
        return SAI_STATUS_NOT_SUPPORTED;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_set_attribute (dn_sai_bridge_info_t *bridge_info,
                                                 const sai_attribute_t *attr)
{
    if((bridge_info == NULL) || (attr == NULL)){
        SAI_BRIDGE_LOG_TRACE("bridge info is %p attr is %p in bcm bridge set attr",
                             bridge_info, attr);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_port_create(sai_object_id_t *bridge_port_id,
                                              dn_sai_bridge_port_info_t *bridge_port_info)
{
    sai_fib_router_interface_t *p_rif_node = NULL;
    sai_object_id_t rif_id = SAI_NULL_OBJECT_ID;

    if((bridge_port_id == NULL) || (bridge_port_info == NULL)) {
        SAI_BRIDGE_LOG_TRACE("bridge port id is %p bridge port info is %p in bcm bridge port create",
                             bridge_port_id, bridge_port_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_1D_ROUTER) {
        rif_id = sai_bridge_port_info_get_rif_id(bridge_port_info);

        p_rif_node = sai_fib_router_interface_node_get (rif_id);
        if(NULL == p_rif_node) {
            SAI_BRIDGE_LOG_ERR("Unable to find rif node for rif 0x%"PRIx64
                               " while setting up RIF for 1D bridge port "
                               " in the hardware", rif_id);
            return SAI_STATUS_INVALID_ATTR_VALUE_0;
        }
        p_rif_node->attachment.bridge_id = bridge_port_info->bridge_id;
    }

    *bridge_port_id =  sai_vm_bridge_port_id_create();

    return SAI_STATUS_SUCCESS;
}


static sai_status_t sai_vm_bridge_port_remove(dn_sai_bridge_port_info_t *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        SAI_BRIDGE_LOG_TRACE("bridge port info is NULL in bridge port remove");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Nothing to be done*/
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_port_set_attribute (const dn_sai_bridge_port_info_t
                                                      *bridge_port_info,
                                                      const sai_attribute_t *attr)
{
    if((bridge_port_info == NULL) || (attr == NULL)){
        SAI_BRIDGE_LOG_TRACE("bridge port info is %p and attr is %p in bridge port set attr",
                             bridge_port_info, attr);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_port_lag_handler(dn_sai_bridge_port_info_t
                                                   *bridge_port_info,
                                                   sai_object_id_t lag_id, bool add_ports,
                                                   const sai_object_list_t *port_list)
{
    if((bridge_port_info == NULL) || (port_list == NULL)) {
        SAI_BRIDGE_LOG_TRACE("bridge port info is %p port_list is %p in sub port attribute set",
                             bridge_port_info, port_list);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

static void sai_vm_bridge_dump_bridge_hw_info(const dn_sai_bridge_info_t *p_bridge_info)
{
}

static void sai_vm_bridge_dump_bridge_port_info(const dn_sai_bridge_port_info_t *bridge_port_info)
{
}

static void sai_vm_bridge_dump_npu(sai_bridge_debug_dump_t dump_type)
{
}

static sai_status_t sai_vm_bridge_get_stats (const dn_sai_bridge_info_t *bridge_info,
                                              uint32_t number_of_counters,
                                              const sai_bridge_stat_t *counter_ids,
                                              uint64_t *counters)
{
    if((number_of_counters == 0) || (counters == NULL) ||
       (counter_ids == NULL) || (bridge_info == NULL)) {
        SAI_BRIDGE_LOG_TRACE("counter_ids is %p counters is %p num counters is %d for "
                             "bridge info is %p", counter_ids, counters,
                             number_of_counters, bridge_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_clear_stats (const dn_sai_bridge_info_t *bridge_info,
                                                uint32_t number_of_counters,
                                                const sai_bridge_stat_t *counter_ids)
{
    if((number_of_counters == 0) || (counter_ids == NULL) || (bridge_info == NULL)) {
        SAI_BRIDGE_LOG_TRACE("counter_ids is %p num counters is %d for bridge info is %p",
                             counter_ids, number_of_counters, bridge_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_port_get_stats(const dn_sai_bridge_port_info_t
                                                  *bridge_port_info,
                                                  uint32_t number_of_counters,
                                                  const sai_bridge_port_stat_t *counter_ids,
                                                  uint64_t *counters)
{
    if((number_of_counters == 0) || (counters == NULL) ||
       (counter_ids == NULL) ||(bridge_port_info == NULL)) {
        SAI_BRIDGE_LOG_TRACE("counter_ids is %p counters is %p num counters is %d for "
                             "bridge port info is %p", counter_ids, counters,
                             number_of_counters, bridge_port_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_port_clear_stats(const dn_sai_bridge_port_info_t
                                                    *bridge_port_info,
                                                    uint32_t number_of_counters,
                                                    const sai_bridge_port_stat_t *counter_ids)
{
    if((number_of_counters == 0) || (counter_ids == NULL) || (bridge_port_info == NULL)) {
        SAI_BRIDGE_LOG_TRACE("counter_ids is %p number of counters is %d bridge port info %p",
                             counter_ids, number_of_counters, bridge_port_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_npu_bridge_api_t sai_vm_bridge_api_table = {
    sai_vm_bridge_init,
    sai_vm_bridge_create,
    sai_vm_bridge_remove,
    sai_vm_bridge_set_attribute,
    sai_vm_bridge_get_stats,
    sai_vm_bridge_clear_stats,
    sai_vm_bridge_port_create,
    sai_vm_bridge_port_remove,
    sai_vm_bridge_port_set_attribute,
    sai_vm_bridge_port_get_stats,
    sai_vm_bridge_port_clear_stats,
    sai_vm_bridge_port_lag_handler,
    sai_vm_bridge_attr_table_get,
    sai_vm_bridge_port_attr_table_get,
    sai_vm_bridge_dump_bridge_hw_info,
    sai_vm_bridge_dump_bridge_port_info,
    sai_vm_bridge_dump_npu
};

sai_npu_bridge_api_t* sai_vm_bridge_api_query (void)
{
    return &sai_vm_bridge_api_table;
}

