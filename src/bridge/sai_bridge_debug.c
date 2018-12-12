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
* @file sai_bridge_debug.c
*
* @brief This file contains implementation of SAI Bridge Debug APIs
*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "saibridge.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saiswitch.h"
#include "sai_bridge_common.h"
#include "sai_bridge_api.h"
#include "sai_bridge_npu_api.h"
#include "sai_bridge_main.h"
#include "sai_common_infra.h"
#include "sai_switch_utils.h"
#include "sai_infra_api.h"

void sai_bridge_port_dump_vlan_members(sai_object_id_t bridge_port_id)
{
    sai_status_t    sai_rc = SAI_STATUS_FAILURE;
    uint_t          vlan_member_cnt = 0;
    uint_t          member_idx = 0;
    sai_object_id_t vlan_member_id = SAI_NULL_OBJECT_ID;

    sai_rc = sai_bridge_port_to_vlan_member_count_get(bridge_port_id, &vlan_member_cnt);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in obtaining the vlan member count for bridge port 0x%"PRIx64"",
                  sai_rc, bridge_port_id);
        return;
    }

    if(vlan_member_cnt == 0) {
        SAI_DEBUG("No Vlan members associated to bridge port 0x%"PRIx64"", bridge_port_id);
        return;
    }

    SAI_DEBUG("Vlan member count %d", vlan_member_cnt);

    for(member_idx = 0; member_idx < vlan_member_cnt; member_idx++) {
        sai_rc = sai_bridge_port_get_vlan_member_at_index(bridge_port_id, member_idx,
                                                          &vlan_member_id);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in obtaining the vlan member at idx %d for bridge port "
                      "0x%"PRIx64"", sai_rc, member_idx, bridge_port_id);
        } else {
            SAI_DEBUG("Vlan member %d - 0x%"PRIx64"", member_idx, vlan_member_id);
        }
    }
}

void sai_bridge_port_dump_stp_ports(sai_object_id_t bridge_port_id)
{
    sai_status_t    sai_rc = SAI_STATUS_FAILURE;
    uint_t          stp_port_cnt = 0;
    uint_t          port_idx = 0;
    sai_object_id_t stp_port_id = SAI_NULL_OBJECT_ID;

    sai_rc = sai_bridge_port_to_stp_port_count_get(bridge_port_id, &stp_port_cnt);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in obtaining the stp port count for bridge port 0x%"PRIx64"",
                  sai_rc, bridge_port_id);
        return;
    }

    if(stp_port_cnt == 0) {
        SAI_DEBUG("No stp port associated to bridge port 0x%"PRIx64"", bridge_port_id);
        return;
    }

    SAI_DEBUG("Stp port count %d", stp_port_cnt);

    for(port_idx = 0; port_idx < stp_port_cnt; port_idx++) {
        sai_rc = sai_bridge_port_get_stp_port_at_index(bridge_port_id, port_idx,
                &stp_port_id);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in obtaining the stp port at idx %d for bridge port "
                    "0x%"PRIx64"", sai_rc, port_idx, bridge_port_id);
            return;
        } else {
            SAI_DEBUG("Stp port %d - 0x%"PRIx64"", port_idx, stp_port_id);
        }
    }
}


void sai_bridge_port_dump_l2mc_members(sai_object_id_t bridge_port_id)
{
    sai_status_t    sai_rc = SAI_STATUS_FAILURE;
    uint_t          l2mc_member_cnt = 0;
    uint_t          member_idx = 0;
    sai_object_id_t l2mc_member_id = SAI_NULL_OBJECT_ID;

    sai_rc = sai_bridge_port_to_l2mc_member_count_get(bridge_port_id, &l2mc_member_cnt);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in obtaining the l2mc member count for bridge port 0x%"PRIx64"",
                  sai_rc, bridge_port_id);
        return;
    }

    if(l2mc_member_cnt == 0) {
        SAI_DEBUG("No L2mc members associated to bridge port 0x%"PRIx64"", bridge_port_id);
        return;
    }

    SAI_DEBUG("L2mc member count %d", l2mc_member_cnt);

    for(member_idx = 0; member_idx < l2mc_member_cnt; member_idx++) {
        sai_rc = sai_bridge_port_get_l2mc_member_at_index(bridge_port_id, member_idx,
                                                          &l2mc_member_id);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in obtaining the l2mc member at idx %d for bridge port "
                      "0x%"PRIx64"", sai_rc, member_idx, bridge_port_id);
        } else {
            SAI_DEBUG("L2mc member %d - 0x%"PRIx64"", member_idx, l2mc_member_id);
        }
    }
}

void sai_bridge_port_dump_dependent_objects(sai_object_id_t bridge_port_id)
{
    sai_bridge_port_dump_vlan_members(bridge_port_id);
    sai_bridge_port_dump_stp_ports(bridge_port_id);
    sai_bridge_port_dump_l2mc_members(bridge_port_id);
}

void sai_bridge_dump_bridge_info(sai_object_id_t bridge_id)
{
    dn_sai_bridge_info_t *p_bridge_info = NULL;
    sai_status_t          sai_rc = SAI_STATUS_FAILURE;
    uint_t                bridge_port_count = 0;
    uint_t                idx = 0;
    sai_object_id_t       bridge_port_id = SAI_NULL_OBJECT_ID;

    sai_rc = sai_bridge_cache_read(bridge_id, &p_bridge_info);
    if((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_info == NULL)) {
        SAI_DEBUG("Error %d in getting bridge info", sai_rc);
        return;
    }
    SAI_DEBUG("Bridge ID 0x%"PRIx64"", p_bridge_info->bridge_id);
    SAI_DEBUG("Bridge type %d", p_bridge_info->bridge_type);
    SAI_DEBUG("Max learned address %d", p_bridge_info->max_learned_address);
    SAI_DEBUG("Learn disable %d", p_bridge_info->learn_disable);
    SAI_DEBUG("Switch object id 0x%"PRIx64"", p_bridge_info->switch_obj_id);
    SAI_DEBUG("Unknown unicast flood group 0x%"PRIx64"",
              p_bridge_info->l2mc_flood_group[SAI_BRIDGE_FLOOD_TYPE_UNKNOWN_UNICAST]);
    SAI_DEBUG("Unknown multicast flood group  0x%"PRIx64"",
              p_bridge_info->l2mc_flood_group[SAI_BRIDGE_FLOOD_TYPE_UNKNOWN_MULTICAST]);
    SAI_DEBUG("Broadcast flood group  0x%"PRIx64"",
              p_bridge_info->l2mc_flood_group[SAI_BRIDGE_FLOOD_TYPE_BROADCAST]);
    SAI_DEBUG("Unknown unicast flood control %d",
              p_bridge_info->flood_control[SAI_BRIDGE_FLOOD_TYPE_UNKNOWN_UNICAST]);
    SAI_DEBUG("Unknown multicast flood group %d",
              p_bridge_info->flood_control[SAI_BRIDGE_FLOOD_TYPE_UNKNOWN_MULTICAST]);
    SAI_DEBUG("Broadcast flood group  %d",
              p_bridge_info->flood_control[SAI_BRIDGE_FLOOD_TYPE_BROADCAST]);
    SAI_DEBUG("Ref count is %d", p_bridge_info->ref_count);

    sai_rc = sai_bridge_map_get_port_count(bridge_id, &bridge_port_count);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in getting bridge port count", sai_rc);
    } else {
        SAI_DEBUG("Bridge port count %d", bridge_port_count);
        for(idx = 0; idx < bridge_port_count; idx++) {
            sai_bridge_map_get_bridge_port_at_index(p_bridge_info->bridge_id, idx, &bridge_port_id);
            SAI_DEBUG("Bridge port %d - 0x%"PRIx64"", idx, bridge_port_id);
        }
    }

    SAI_DEBUG("*************");
    SAI_DEBUG("Hardware info");
    SAI_DEBUG("*************");

    sai_bridge_npu_api_get()->bridge_dump_hw_info(p_bridge_info);
}

void sai_bridge_dump_multicast_info(sai_object_id_t bridge_id)
{
    dn_sai_bridge_info_t *p_bridge_info = NULL;
    sai_status_t          sai_rc = SAI_STATUS_FAILURE;

    sai_rc = sai_bridge_cache_read(bridge_id, &p_bridge_info);
    if((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_info == NULL)) {
        SAI_DEBUG("Error %d in getting bridge info", sai_rc);
        return;
    }
    SAI_DEBUG("Unknown unicast flood group 0x%"PRIx64"",
              p_bridge_info->l2mc_flood_group[SAI_BRIDGE_FLOOD_TYPE_UNKNOWN_UNICAST]);
    SAI_DEBUG("Unknown multicast flood group  0x%"PRIx64"",
              p_bridge_info->l2mc_flood_group[SAI_BRIDGE_FLOOD_TYPE_UNKNOWN_MULTICAST]);
    SAI_DEBUG("Broadcast flood group  0x%"PRIx64"",
              p_bridge_info->l2mc_flood_group[SAI_BRIDGE_FLOOD_TYPE_BROADCAST]);
    SAI_DEBUG("Unknown unicast flood control %d",
              p_bridge_info->flood_control[SAI_BRIDGE_FLOOD_TYPE_UNKNOWN_UNICAST]);
    SAI_DEBUG("Unknown multicast flood group %d",
              p_bridge_info->flood_control[SAI_BRIDGE_FLOOD_TYPE_UNKNOWN_MULTICAST]);
    SAI_DEBUG("Broadcast flood group  %d",
              p_bridge_info->flood_control[SAI_BRIDGE_FLOOD_TYPE_BROADCAST]);

    SAI_DEBUG("*************");
    SAI_DEBUG("Hardware info");
    SAI_DEBUG("*************");

    sai_bridge_npu_api_get()->bridge_dump_hw_info(p_bridge_info);
}
void sai_brige_dump_default_bridge(void)
{
    sai_object_id_t bridge_id;
    sai_status_t    sai_rc;

    sai_rc = sai_bridge_default_id_get(&bridge_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in getting default bridge id", sai_rc);
        return;
    }

    sai_bridge_dump_bridge_info(bridge_id);
}

void sai_bridge_dump_bridge_port_info(sai_object_id_t bridge_port_id)
{
    sai_status_t               sai_rc;
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;
    sai_object_id_t            attach_id;
    uint16_t                   vlan_id;

    sai_rc = sai_bridge_port_cache_read(bridge_port_id, &p_bridge_port_info);
    if((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_port_info == NULL)) {
        SAI_DEBUG("Error %d in getting bridge_port info", sai_rc);
        return;
    }
    SAI_DEBUG("Bridge port ID 0x%"PRIx64"", p_bridge_port_info->bridge_port_id);
    SAI_DEBUG("Bridge ID 0x%"PRIx64"", p_bridge_port_info->bridge_id);
    SAI_DEBUG("Bridge port type %d", p_bridge_port_info->bridge_port_type);
    SAI_DEBUG("Max learned address %d", p_bridge_port_info->max_learned_address);
    SAI_DEBUG("Learn mode %d", p_bridge_port_info->fdb_learn_mode);
    SAI_DEBUG("Learn limit violation action %d", p_bridge_port_info->learn_limit_violation_action);
    SAI_DEBUG("Admin state %d", p_bridge_port_info->admin_state);
    SAI_DEBUG("Ingress filtering %d", p_bridge_port_info->ingress_filtering);
    SAI_DEBUG("Switch object id 0x%"PRIx64"", p_bridge_port_info->switch_obj_id);
    SAI_DEBUG("Ref count is %d", p_bridge_port_info->ref_count);
    SAI_DEBUG("FDB count is %d", p_bridge_port_info->fdb_count);

    switch(p_bridge_port_info->bridge_port_type) {

        case SAI_BRIDGE_PORT_TYPE_PORT:
            attach_id = sai_bridge_port_info_get_port_id(p_bridge_port_info);
            SAI_DEBUG("Attach port id 0x%"PRIx64"",attach_id);
            break;

        case SAI_BRIDGE_PORT_TYPE_SUB_PORT:
            attach_id = sai_bridge_port_info_get_port_id(p_bridge_port_info);
            vlan_id = sai_bridge_port_info_get_vlan_id(p_bridge_port_info);
            SAI_DEBUG("Attach port id 0x%"PRIx64"",attach_id);
            SAI_DEBUG("Attach vlan id %d",vlan_id);
            break;

        case SAI_BRIDGE_PORT_TYPE_1Q_ROUTER:
        case SAI_BRIDGE_PORT_TYPE_1D_ROUTER:
            attach_id = sai_bridge_port_info_get_rif_id(p_bridge_port_info);
            SAI_DEBUG("Attach router interface id 0x%"PRIx64"",attach_id);
            break;

        case SAI_BRIDGE_PORT_TYPE_TUNNEL:
            attach_id = sai_bridge_port_info_get_tunnel_id(p_bridge_port_info);
            SAI_DEBUG("Attach tunnel id 0x%"PRIx64"",attach_id);
            break;
    }

    SAI_DEBUG("*************");
    SAI_DEBUG("Hardware info");
    SAI_DEBUG("*************");
    sai_bridge_npu_api_get()->bridge_port_dump_hw_info(p_bridge_port_info);

    SAI_DEBUG("Dumping dependent objects");
    sai_bridge_port_dump_dependent_objects(bridge_port_id);
    SAI_DEBUG("-------------------------------------------------------------------------");
}

void sai_bridge_dump_all_bridge_ids(void)
{
    uint_t           count = sai_bridge_total_count();
    sai_object_id_t *bridge_id = NULL;
    sai_status_t     sai_rc;
    uint_t           idx = 0;

    bridge_id = calloc(count, sizeof(sai_object_id_t));
    do {
        sai_rc = sai_bridge_list_get(&count, bridge_id);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in getting bridge list",sai_rc);
            break;
        }

        for(idx=0; idx< count; idx++) {
            SAI_DEBUG("Bridge ID %d - 0x%"PRIx64"", idx, bridge_id[idx]);
        }
    } while(0);

    free(bridge_id);

}

void sai_bridge_dump_all_bridge_info(void)
{
    uint_t           count = sai_bridge_total_count();
    sai_object_id_t *bridge_id = NULL;
    sai_status_t     sai_rc;
    uint_t           idx = 0;

    bridge_id = calloc(count, sizeof(sai_object_id_t));
    do {
        sai_rc = sai_bridge_list_get(&count, bridge_id);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in getting bridge list",sai_rc);
            break;
        }

        for(idx=0; idx< count; idx++) {
            sai_bridge_dump_bridge_info(bridge_id[idx]);
        }
    } while(0);
    free(bridge_id);
}

void sai_bridge_dump_all_bridge_port_ids(void)
{
    uint_t           count = sai_bridge_port_total_count();
    sai_object_id_t *bridge_port_id = NULL;
    sai_status_t     sai_rc;
    uint_t           idx = 0;

    bridge_port_id = calloc(count, sizeof(sai_object_id_t));
    do {
        sai_rc = sai_bridge_port_list_get(&count, bridge_port_id);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in getting bridge port list",sai_rc);
            break;
        }

        for(idx=0; idx< count; idx++) {
            SAI_DEBUG("Bridge Port ID %d - 0x%"PRIx64"", idx, bridge_port_id[idx]);
        }
    } while(0);

    free(bridge_port_id);

}

void sai_bridge_dump_all_bridge_port_info(void)
{
    uint_t          count = sai_bridge_port_total_count();
    sai_object_id_t *bridge_port_id = NULL;
    sai_status_t    sai_rc;
    uint_t          idx = 0;

    bridge_port_id = calloc(count, sizeof(sai_object_id_t));
    sai_rc = sai_bridge_port_list_get(&count, bridge_port_id);

    do {
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in getting bridge port list",sai_rc);
            break;
        }

        for(idx=0; idx< count; idx++) {
            sai_bridge_dump_bridge_port_info(bridge_port_id[idx]);

        }
    } while(0);

    free(bridge_port_id);
}

static sai_status_t sai_bridge_debug_bridge_create(sai_object_id_t *bridge_id)
{
    sai_attribute_t attr[1];
    sai_status_t    sai_rc;

    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    sai_rc = sai_bridge_api_query()->create_bridge(bridge_id, SAI_DEFAULT_SWITCH_ID, 1, attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating bridge",sai_rc);
        return sai_rc;;
    }
    SAI_DEBUG("Bridge ID created is 0x%"PRIx64"",*bridge_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_debug_bridge_remove(sai_object_id_t bridge_id)
{
    sai_status_t    sai_rc;

    sai_rc = sai_bridge_api_query()->remove_bridge(bridge_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing bridge",sai_rc);
        return sai_rc;;
    }
    SAI_DEBUG("Bridge ID removed is 0x%"PRIx64"",bridge_id);
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_bridge_debug_lag_create(sai_object_id_t *lag_id)
{
    sai_status_t   sai_rc = SAI_STATUS_FAILURE;

    sai_rc = sai_lag_api_query()->create_lag(lag_id, SAI_DEFAULT_SWITCH_ID, 0, NULL);

    if(sai_rc == SAI_STATUS_SUCCESS) {
        SAI_DEBUG("LAG ID created is 0x%"PRIx64"", *lag_id);
    }
    return sai_rc;
}

static sai_status_t sai_bridge_debug_lag_add_port(sai_object_id_t lag_id, sai_object_id_t port_id,
                                    sai_object_id_t *lag_member_id)
{
    sai_attribute_t attr[2];
    sai_status_t   sai_rc = SAI_STATUS_FAILURE;

    attr[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attr[0].value.oid = lag_id;

    attr[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attr[1].value.oid = port_id;

    sai_rc = sai_lag_api_query()->create_lag_member(lag_member_id, SAI_DEFAULT_SWITCH_ID,  2, attr);

    if(sai_rc == SAI_STATUS_SUCCESS) {
        SAI_DEBUG("LAG member id created is 0x%"PRIx64"", *lag_member_id);
    }
    return sai_rc;
}

sai_status_t sai_bridge_debug_lag_remove_member(sai_object_id_t lag_member_id)
{
    return sai_lag_api_query()->remove_lag_member(lag_member_id);
}

sai_status_t sai_bridge_debug_lag_remove(sai_object_id_t lag_id)
{
    return sai_lag_api_query()->remove_lag(lag_id);
}

static sai_status_t sai_bridge_debug_bridge_port_sub_port_create(sai_object_id_t bridge_id,
                                                          sai_object_id_t port_id, int vlan_id,
                                                          bool untagged,
                                                          sai_object_id_t *bridge_port_id)
{
    sai_attribute_t attr[5];
    sai_status_t    sai_rc;

    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = port_id;

    attr[2].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr[2].value.u16 = vlan_id;

    attr[3].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[3].value.oid = bridge_id;

    attr[4].id = SAI_BRIDGE_PORT_ATTR_TAGGING_MODE;
    if(untagged) {
        attr[4].value.s32 = SAI_BRIDGE_PORT_TAGGING_MODE_UNTAGGED;
    } else {
        attr[4].value.s32 = SAI_BRIDGE_PORT_TAGGING_MODE_TAGGED;
    }

    sai_rc = sai_bridge_api_query()->create_bridge_port(bridge_port_id, SAI_DEFAULT_SWITCH_ID,
                                                        5, attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating bridge port",sai_rc);
        return sai_rc;
    }
    SAI_DEBUG("Bridge port ID created is 0x%"PRIx64"",*bridge_port_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_debug_bridge_port_set_attribute(sai_object_id_t bridge_port_id,
                                                        int attr_id, int value)
{

    sai_attribute_t attr;
    sai_status_t sai_rc;

    attr.id = attr_id;
    attr.value.s32 = value;

    sai_rc = sai_bridge_api_query()->set_bridge_port_attribute(bridge_port_id, &attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in set bridge port attribute",sai_rc);
    }
    return sai_rc;
}

void sai_bridge_debug_bridge_port_remove(sai_object_id_t bridge_port_id)
{
    sai_status_t    sai_rc;

    sai_rc = sai_bridge_api_query()->remove_bridge_port(bridge_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removed bridge port",sai_rc);
        return;
    }
    SAI_DEBUG("Bridge port ID removed is 0x%"PRIx64"",bridge_port_id);
}


static sai_status_t sai_bridge_debug_bridge_port_type_port_create(sai_object_id_t port_id,
                                                           sai_object_id_t *bridge_port_id)
{
    sai_attribute_t bridge_port_attr[SAI_BRIDGE_DEF_BRIDGE_PORT_ATTR_COUNT];
    sai_object_id_t bridge_id;
    sai_attribute_t attr;
    sai_status_t    sai_rc;


    attr.id = SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID;

    sai_rc = sai_switch_api_query()->get_switch_attribute(SAI_DEFAULT_SWITCH_ID,1,&attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in getting default bridge",sai_rc);
        return sai_rc;
    }

    bridge_id = attr.value.oid;

    bridge_port_attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    bridge_port_attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_PORT;

    bridge_port_attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    bridge_port_attr[1].value.oid = port_id;

    bridge_port_attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    bridge_port_attr[2].value.oid = bridge_id;

    bridge_port_attr[3].id = SAI_BRIDGE_PORT_ATTR_ADMIN_STATE;
    bridge_port_attr[3].value.booldata = true;

    sai_rc = sai_bridge_api_query()->create_bridge_port(bridge_port_id, SAI_DEFAULT_SWITCH_ID,
                                                        SAI_BRIDGE_DEF_BRIDGE_PORT_ATTR_COUNT,
                                                        bridge_port_attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating bridge port",sai_rc);
        return sai_rc;
    }
    SAI_DEBUG("Bridge port ID created is 0x%"PRIx64"",*bridge_port_id);
    return sai_rc;

}

static sai_status_t sai_bridge_debug_vlan_create(int vlan_id, sai_object_id_t *vlan_obj)
{
    sai_attribute_t attr;
    sai_status_t    sai_rc;

    attr.id = SAI_VLAN_ATTR_VLAN_ID;
    attr.value.u16 = vlan_id;

    sai_rc = sai_vlan_api_query()->create_vlan(vlan_obj,SAI_DEFAULT_SWITCH_ID,1, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating vlan",sai_rc);
        return sai_rc;
    }
    SAI_DEBUG("VLAN Object ID created is 0x%"PRIx64"",*vlan_obj);
    return sai_rc;
}

static sai_status_t sai_bridge_debug_vlan_remove(sai_object_id_t vlan_obj)
{
    sai_status_t    sai_rc;

    sai_rc = sai_vlan_api_query()->remove_vlan(vlan_obj);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing vlan 0x%"PRIx64"", sai_rc, vlan_obj);
        return sai_rc;
    }
    SAI_DEBUG("VLAN Object ID removed is 0x%"PRIx64"",vlan_obj);
    return sai_rc;
}

static sai_status_t sai_bridge_debug_stp_create(sai_object_id_t *stp_obj)
{
    sai_status_t    sai_rc;

    sai_rc = sai_stp_api_query()->create_stp(stp_obj,SAI_DEFAULT_SWITCH_ID,0, NULL);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating stp",sai_rc);
        return sai_rc;
    }
    SAI_DEBUG("STP Object ID created is 0x%"PRIx64"",*stp_obj);
    return sai_rc;
}

static sai_status_t sai_bridge_debug_stp_remove(sai_object_id_t stp_obj)
{
    sai_status_t    sai_rc;

    sai_rc = sai_stp_api_query()->remove_stp(stp_obj);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing stp 0x%"PRIx64"", sai_rc, stp_obj);
        return sai_rc;
    }
    SAI_DEBUG("stp Object ID removed is 0x%"PRIx64"",stp_obj);
    return sai_rc;
}

static sai_status_t sai_bridge_debug_vlan_member_create(sai_object_id_t vlan_object_id,
                                           sai_object_id_t bridge_port_id,
                                           sai_object_id_t *vlan_member_id)
{
    sai_attribute_t attr[2];
    sai_status_t    sai_rc;

    attr[0].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr[0].value.oid = vlan_object_id;
    attr[1].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr[1].value.oid = bridge_port_id;
    sai_rc = sai_vlan_api_query()->create_vlan_member(vlan_member_id, SAI_DEFAULT_SWITCH_ID,
                                                      2, attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating vlan member",sai_rc);
        return sai_rc;
    }
    SAI_DEBUG("VLAN member ID created is 0x%"PRIx64"",*vlan_member_id);
    return sai_rc;
}

static sai_status_t sai_bridge_debug_stp_port_create(sai_object_id_t stp_object_id,
                                                     sai_object_id_t bridge_port_id,
                                                     sai_object_id_t *stp_port_id)
{
    sai_attribute_t attr[3];
    sai_status_t    sai_rc;

    attr[0].id = SAI_STP_PORT_ATTR_STP;
    attr[0].value.oid = stp_object_id;
    attr[1].id = SAI_STP_PORT_ATTR_BRIDGE_PORT;
    attr[1].value.oid = bridge_port_id;
    attr[2].id = SAI_STP_PORT_ATTR_STATE;
    attr[2].value.s32 = SAI_STP_PORT_STATE_FORWARDING;

    sai_rc = sai_stp_api_query()->create_stp_port(stp_port_id, SAI_DEFAULT_SWITCH_ID,
                                                  3, attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating stp port",sai_rc);
        return sai_rc;
    }
    SAI_DEBUG("STP Port ID created is 0x%"PRIx64"",*stp_port_id);
    return sai_rc;
}

static sai_status_t sai_bridge_debug_vlan_member_remove(sai_object_id_t vlan_member_id)
{
    sai_status_t    sai_rc;

    sai_rc = sai_vlan_api_query()->remove_vlan_member(vlan_member_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing vlan member 0x%"PRIx64"", sai_rc, vlan_member_id);
        return sai_rc;
    }
    SAI_DEBUG("VLAN member ID removed is 0x%"PRIx64"",vlan_member_id);
    return sai_rc;
}

static sai_status_t sai_bridge_debug_stp_port_remove(sai_object_id_t stp_port_id)
{
    sai_status_t    sai_rc;

    sai_rc = sai_stp_api_query()->remove_stp_port(stp_port_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing STP port 0x%"PRIx64"",sai_rc, stp_port_id);
        return sai_rc;
    }
    SAI_DEBUG("STP port ID removed is 0x%"PRIx64"",stp_port_id);
    return sai_rc;
}

static void sai_bridge_npu_vlan_debug_dump()
{
    SAI_DEBUG("\r\n");
    SAI_DEBUG("-----------------------------------------------------------------------------\r\n");
    SAI_DEBUG(" SAI VLAN NPU dump ");
    SAI_DEBUG("-----------------------------------------------------------------------------\r\n");
    sai_bridge_npu_api_get()->bridge_dump_npu(SAI_BRIDGE_DEBUG_DUMP_VLAN);
    SAI_DEBUG("-----------------------------------------------------------------------------\r\n");
    SAI_DEBUG("\r\n");
}

static void sai_bridge_npu_mcast_debug_dump()
{
    SAI_DEBUG("\r\n");
    SAI_DEBUG("-----------------------------------------------------------------------------\r\n");
    SAI_DEBUG(" SAI MCAST NPU dump ");
    SAI_DEBUG("-----------------------------------------------------------------------------\r\n");
    sai_bridge_npu_api_get()->bridge_dump_npu(SAI_BRIDGE_DEBUG_DUMP_MCAST);
    SAI_DEBUG("-----------------------------------------------------------------------------\r\n");
    SAI_DEBUG("\r\n");
}
static void sai_bridge_npu_stp_debug_dump()
{
    SAI_DEBUG("\r\n");
    SAI_DEBUG("-----------------------------------------------------------------------------\r\n");
    SAI_DEBUG(" SAI STP NPU dump ");
    SAI_DEBUG("-----------------------------------------------------------------------------\r\n");
    sai_bridge_npu_api_get()->bridge_dump_npu(SAI_BRIDGE_DEBUG_DUMP_STP);
    SAI_DEBUG("-----------------------------------------------------------------------------\r\n");
    SAI_DEBUG("\r\n");
}
static sai_status_t sai_bridge_debug_init_get_bridge_ports(sai_object_id_t def_vlan_id,
                                                           uint_t count,
                                                           sai_object_id_t *bridge_port_id)
{
    sai_object_id_t def_vlan_member_id[count];
    sai_attribute_t attr[1];
    sai_status_t    sai_rc = SAI_STATUS_FAILURE;
    uint_t          idx = 0;
    uint_t          stp_port_count = 0;
    sai_object_id_t def_stp_id = SAI_NULL_OBJECT_ID;

    attr[0].id = SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID;
    sai_rc = sai_switch_api_query()->get_switch_attribute(SAI_DEFAULT_SWITCH_ID,1, attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }
    def_stp_id = attr[0].value.oid;

    attr[0].id = SAI_VLAN_ATTR_MEMBER_LIST;
    attr[0].value.objlist.count = 0;

    sai_vlan_api_query()->get_vlan_attribute(def_vlan_id, 1, attr);

    if(attr[0].value.objlist.count < count) {
        SAI_DEBUG("Max count is %d\r\n",attr[0].value.objlist.count);
        return SAI_STATUS_BUFFER_OVERFLOW;
    }
    sai_object_id_t *vlan_member_list = calloc(attr[0].value.objlist.count,
                                               sizeof(sai_object_id_t));
    sai_object_id_t *stp_port_list = NULL;
    attr[0].value.objlist.list = vlan_member_list;

    do {
        sai_rc =  sai_vlan_api_query()->get_vlan_attribute(def_vlan_id, 1, attr);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in getting def vlan member list\r\n", sai_rc);
            break;
        }

        memcpy(def_vlan_member_id, vlan_member_list, count*sizeof(sai_object_id_t));

        attr[0].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;

        for(idx = 0; idx < count; idx++) {
            sai_rc = sai_vlan_api_query()->get_vlan_member_attribute(def_vlan_member_id[idx], 1, attr);

            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_DEBUG("Error %d in getting bridge port id from vlan member 0x%"PRIx64"\r\n",
                        sai_rc, def_vlan_member_id[idx]);
                break;
            }
            bridge_port_id[idx] = attr[0].value.oid;

            sai_rc = sai_vlan_api_query()->remove_vlan_member(def_vlan_member_id[idx]);
            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_DEBUG("Error %d in removing vlan member 0x%"PRIx64"\r\n", sai_rc,
                        def_vlan_member_id[idx]);
                break;
            }
        }
        if(sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }
        attr[0].id = SAI_STP_ATTR_PORT_LIST;
        attr[0].value.objlist.count = 0;

        sai_stp_api_query()->get_stp_attribute(def_stp_id, 1, attr);

        if(attr[0].value.objlist.count < count) {
            SAI_DEBUG("Max count is %d\r\n",attr[0].value.objlist.count);
            break;
        }
        stp_port_list = calloc(attr[0].value.objlist.count,
                sizeof(sai_object_id_t));

        attr[0].value.objlist.list = stp_port_list;

        sai_rc =  sai_stp_api_query()->get_stp_attribute(def_stp_id, 1, attr);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in getting def stp member list\r\n", sai_rc);
            break;
        }

        stp_port_count = attr[0].value.objlist.count;
        attr[0].id = SAI_STP_PORT_ATTR_BRIDGE_PORT;

        for(idx = 0; idx < stp_port_count; idx++) {
            sai_rc = sai_stp_api_query()->get_stp_port_attribute(stp_port_list[idx], 1, attr);

            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_DEBUG("Error %d in getting bridge port id from stp member 0x%"PRIx64"\r\n",
                        sai_rc, stp_port_list[idx]);
                break;
            }
            if(bridge_port_id[idx] == attr[0].value.oid) {
                sai_rc = sai_stp_api_query()->remove_stp_port(stp_port_list[idx]);
                if(sai_rc != SAI_STATUS_SUCCESS) {
                    SAI_DEBUG("Error %d in removing stp member 0x%"PRIx64"\r\n", sai_rc,
                              stp_port_list[idx]);
                    break;
                }
            }
        }
    } while(0);

    if(vlan_member_list != NULL) {
        free(vlan_member_list);
    }
    if(stp_port_list != NULL) {
        free(stp_port_list);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_bridge_debug_get_ports_from_bridge_ports(uint_t count,
                                                                 sai_object_id_t *bridge_port_id,
                                                                 sai_object_id_t *port_id)
{
    sai_attribute_t bridge_port_attr[1];
    sai_status_t    sai_rc;
    uint_t          idx = 0;

    bridge_port_attr[0].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;

    for(idx = 0; idx < count; idx++) {
        sai_rc = sai_bridge_api_query()->get_bridge_port_attribute(bridge_port_id[idx], 1,
                                                                   bridge_port_attr);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in getting port for bridge port 0x%"PRIx64"",sai_rc,
                      bridge_port_id[idx]);
        }
        port_id[idx] = bridge_port_attr[0].value.oid;

    }
    return SAI_STATUS_SUCCESS;
}


void sai_bridge_debug_bridge_port_set_admin_state(sai_object_id_t bridge_port_id, bool admin_state)

{

    sai_attribute_t attr;
    sai_status_t sai_rc;

    attr.id = SAI_BRIDGE_PORT_ATTR_ADMIN_STATE;
    attr.value.booldata = admin_state;

    sai_rc = sai_bridge_api_query()->set_bridge_port_attribute(bridge_port_id, &attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in set bridge admin_state",sai_rc);
    }
}

static sai_status_t sai_bridge_debug_remove_bridge_ports(uint_t count,
                                                         sai_object_id_t *bridge_port_id)
{
    sai_status_t    sai_rc;
    uint_t          idx = 0;

    for(idx = 0; idx < count; idx++) {
        sai_bridge_debug_bridge_port_set_admin_state(bridge_port_id[idx], false);
        sai_rc = sai_bridge_api_query()->remove_bridge_port(bridge_port_id[idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in removing bridge port 0x%"PRIx64"",sai_rc,
                      bridge_port_id[idx]);
        }
        SAI_DEBUG("Removed bridge port 0x%"PRIx64" successfully",bridge_port_id[idx]);
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_stp_lag_callback_npu_dump(void)
{
    sai_object_id_t lag_id;
    sai_object_id_t lag_bridge_port_id;
    sai_object_id_t lag_member_id[SAI_DEBUG_MAX_UT_PORTS];
    sai_object_id_t vlan_id;
    sai_object_id_t vlan_member_id;
    sai_object_id_t lag_vlan_member_id;
    sai_object_id_t lag_stp_port_id;
    sai_object_id_t def_vlan_id;
    sai_object_id_t port_id[SAI_DEBUG_MAX_UT_PORTS];
    sai_object_id_t bridge_port_id[SAI_DEBUG_MAX_UT_PORTS];
    sai_status_t    sai_rc;
    sai_attribute_t attr;
    uint_t          idx = 0;
    sai_object_id_t stp_id = 0;

    attr.id = SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID;

    SAI_DEBUG("Default NPU statue dump\r\n");
    sai_bridge_npu_stp_debug_dump();

    attr.id = SAI_SWITCH_ATTR_DEFAULT_VLAN_ID;

    sai_rc = sai_switch_api_query()->get_switch_attribute(SAI_DEFAULT_SWITCH_ID, 1, &attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in getting default vlan\r\n",sai_rc);
        return sai_rc;
    }
    def_vlan_id = attr.value.oid;
    SAI_DEBUG("Default vlan Id is 0x%"PRIx64"\r\n",def_vlan_id);
    sai_rc = sai_bridge_debug_init_get_bridge_ports(def_vlan_id, SAI_DEBUG_MAX_UT_PORTS,
                                                    bridge_port_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    sai_rc = sai_bridge_debug_stp_create(&stp_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating stp instance", sai_rc);
        return sai_rc;
    }


    sai_rc = sai_bridge_debug_lag_create(&lag_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating lag\r\n",sai_rc);
        return sai_rc;
    }
    sai_rc = sai_bridge_debug_bridge_port_type_port_create(lag_id, &lag_bridge_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating bridge port for lag 0x%"PRIx64"\r\n",sai_rc, lag_id);
        return sai_rc;
    }
    SAI_DEBUG("Lag id 0x%"PRIx64" bridge port 0x%"PRIx64"\r\n",lag_id, lag_bridge_port_id);

    sai_rc = sai_bridge_debug_vlan_create(SAI_DEBUG_TEST_VLAN, &vlan_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating vlan obj for vlan %d\r\n",sai_rc, SAI_DEBUG_TEST_VLAN);
        return sai_rc;
    }
    SAI_DEBUG("VLAN ID %d obj is 0x%"PRIx64"\r\n",SAI_DEBUG_TEST_VLAN, vlan_id);

    attr.id = SAI_VLAN_ATTR_STP_INSTANCE;
    attr.value.oid = stp_id;

    sai_rc = sai_vlan_api_query()->set_vlan_attribute(vlan_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in setting stp instance 0x%"PRIx64" to vlan 0x%"PRIx64"",
                  sai_rc, vlan_id, stp_id);
        return sai_rc;
    }
    sai_rc = sai_bridge_debug_vlan_member_create(vlan_id, lag_bridge_port_id,
                                                 &lag_vlan_member_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in adding lag 0x%"PRIx64" to vlan 0x%"PRIx64"\r\n",
                  sai_rc, lag_id, vlan_id );
        return sai_rc;
    }

    SAI_DEBUG("VLAN member created for lag  0x%"PRIx64" is 0x%"PRIx64"\r\n", lag_id,
              lag_vlan_member_id);

    sai_rc = sai_bridge_debug_stp_port_create(stp_id, lag_bridge_port_id, &lag_stp_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in adding lag 0x%"PRIx64" to stp 0x%"PRIx64"\r\n",
                  sai_rc, lag_id, stp_id );
        return sai_rc;
    }

    SAI_DEBUG("STP Port created for lag  0x%"PRIx64" is 0x%"PRIx64"\r\n", lag_id,
              lag_stp_port_id);


    SAI_DEBUG("NPU dump after creating lag, vlan and vlan member\r\n");
    sai_bridge_npu_stp_debug_dump();

    sai_rc = sai_bridge_debug_get_ports_from_bridge_ports(SAI_DEBUG_MAX_UT_PORTS,
                                                          bridge_port_id, port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    sai_rc = sai_bridge_debug_remove_bridge_ports(SAI_DEBUG_MAX_UT_PORTS, bridge_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    for(idx = 0; idx< SAI_DEBUG_MAX_UT_PORTS; idx++) {
        sai_rc = sai_bridge_debug_lag_add_port(lag_id, port_id[idx], &lag_member_id[idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in adding port 0x%"PRIx64" to lag 0x%"PRIx64"\r\n",
                      sai_rc, port_id[idx], lag_id);
        }
        SAI_DEBUG("NPU dump after adding port 0x%"PRIx64" to lag 0x%"PRIx64" creating "
                  "lag member 0x%"PRIx64"\r\n", port_id[idx], lag_id, lag_member_id[idx]);
        sai_bridge_npu_stp_debug_dump();
    }

    for(idx = 0; idx< SAI_DEBUG_MAX_UT_PORTS; idx++) {
        sai_rc = sai_bridge_debug_lag_remove_member(lag_member_id[idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in removing lag member 0x%"PRIx64"\r\n",
                      sai_rc, lag_member_id[idx]);
        }
        sai_rc = sai_bridge_debug_bridge_port_type_port_create(port_id[idx], &bridge_port_id[idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in creating bridge port for port 0x%"PRIx64"\r\n",sai_rc,
                      port_id[idx]);
            return sai_rc;
        }
        sai_rc = sai_bridge_debug_vlan_member_create(def_vlan_id, bridge_port_id[idx],
                                                     &vlan_member_id);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in adding lag 0x%"PRIx64" to vlan 0x%"PRIx64"\r\n",
                      sai_rc, port_id[idx], def_vlan_id );
            return sai_rc;
        }

        SAI_DEBUG("Port id 0x%"PRIx64" bridge port 0x%"PRIx64"\r\n",
                  port_id[idx], bridge_port_id[idx]);
        SAI_DEBUG("NPU dump after removing lag member 0x%"PRIx64"\r\n",lag_member_id[idx]);
        sai_bridge_npu_stp_debug_dump();
    }

    sai_rc = sai_bridge_debug_vlan_member_remove(lag_vlan_member_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing lag vlan member 0x%"PRIx64"\r\n",
                   sai_rc, lag_vlan_member_id);
    }

    sai_rc = sai_bridge_debug_stp_port_remove(lag_stp_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing lag stp port 0x%"PRIx64"\r\n",
                   sai_rc, lag_stp_port_id);
    }

    sai_rc = sai_bridge_debug_remove_bridge_ports(1, &lag_bridge_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    sai_rc = sai_bridge_debug_lag_remove(lag_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing lag 0x%"PRIx64"\r\n", sai_rc, lag_id);
        return sai_rc;
    }

    sai_rc = sai_bridge_debug_vlan_remove(vlan_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing vlan 0x%"PRIx64"\r\n", sai_rc, vlan_id);
        return sai_rc;
    }

    sai_rc = sai_bridge_debug_stp_remove(stp_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing stp 0x%"PRIx64"\r\n", sai_rc, stp_id);
        return sai_rc;
    }

    SAI_DEBUG("NPU dump after cleanup");
    sai_bridge_npu_stp_debug_dump();

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_vlan_lag_callback_npu_dump(void)
{
    sai_object_id_t lag_id;
    sai_object_id_t lag_bridge_port_id;
    sai_object_id_t lag_member_id[SAI_DEBUG_MAX_UT_PORTS];
    sai_object_id_t vlan_id;
    sai_object_id_t vlan_member_id;
    sai_object_id_t lag_vlan_member_id;
    sai_object_id_t def_vlan_id;
    sai_object_id_t port_id[SAI_DEBUG_MAX_UT_PORTS];
    sai_object_id_t bridge_port_id[SAI_DEBUG_MAX_UT_PORTS];
    sai_status_t    sai_rc;
    sai_attribute_t attr;
    uint_t          idx = 0;

    attr.id = SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID;

    SAI_DEBUG("Default NPU statue dump\r\n");
    sai_bridge_npu_vlan_debug_dump();

    attr.id = SAI_SWITCH_ATTR_DEFAULT_VLAN_ID;

    sai_rc = sai_switch_api_query()->get_switch_attribute(SAI_DEFAULT_SWITCH_ID, 1, &attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in getting default vlan\r\n",sai_rc);
        return sai_rc;
    }
    def_vlan_id = attr.value.oid;
    SAI_DEBUG("Default vlan Id is 0x%"PRIx64"\r\n",def_vlan_id);
    sai_rc = sai_bridge_debug_init_get_bridge_ports(def_vlan_id, SAI_DEBUG_MAX_UT_PORTS,
                                                    bridge_port_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    SAI_DEBUG("NPU dump after removing bridge ports from default vlan\r\n");
    sai_bridge_npu_vlan_debug_dump();

    sai_rc = sai_bridge_debug_lag_create(&lag_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating lag\r\n",sai_rc);
        return sai_rc;
    }

    sai_rc = sai_bridge_debug_bridge_port_type_port_create(lag_id, &lag_bridge_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating bridge port for lag 0x%"PRIx64"\r\n",sai_rc, lag_id);
        return sai_rc;
    }
    SAI_DEBUG("Lag id 0x%"PRIx64" bridge port 0x%"PRIx64"\r\n",lag_id, lag_bridge_port_id);

    sai_rc = sai_bridge_debug_vlan_create(SAI_DEBUG_TEST_VLAN, &vlan_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating vlan obj for vlan %d\r\n",sai_rc, SAI_DEBUG_TEST_VLAN);
        return sai_rc;
    }
    SAI_DEBUG("VLAN ID %d obj is 0x%"PRIx64"\r\n",SAI_DEBUG_TEST_VLAN, vlan_id);

    sai_rc = sai_bridge_debug_vlan_member_create(vlan_id, lag_bridge_port_id,
                                                 &lag_vlan_member_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in adding lag 0x%"PRIx64" to vlan 0x%"PRIx64"\r\n",
                  sai_rc, lag_id, vlan_id );
        return sai_rc;
    }
    SAI_DEBUG("VLAN member created for lag  0x%"PRIx64" is 0x%"PRIx64"\r\n", lag_id,
              lag_vlan_member_id);

    SAI_DEBUG("NPU dump after creating lag, vlan and vlan member\r\n");
    sai_bridge_npu_vlan_debug_dump();

    sai_rc = sai_bridge_debug_get_ports_from_bridge_ports(SAI_DEBUG_MAX_UT_PORTS,
                                                          bridge_port_id, port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    sai_rc = sai_bridge_debug_remove_bridge_ports(SAI_DEBUG_MAX_UT_PORTS, bridge_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    for(idx = 0; idx< SAI_DEBUG_MAX_UT_PORTS; idx++) {
        sai_rc = sai_bridge_debug_lag_add_port(lag_id, port_id[idx], &lag_member_id[idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in adding port 0x%"PRIx64" to lag 0x%"PRIx64"\r\n",
                      sai_rc, port_id[idx], lag_id);
        }
        SAI_DEBUG("NPU dump after adding port 0x%"PRIx64" to lag 0x%"PRIx64" creating "
                  "lag member 0x%"PRIx64"\r\n", port_id[idx], lag_id, lag_member_id[idx]);
        sai_bridge_npu_vlan_debug_dump();
    }

    for(idx = 0; idx< SAI_DEBUG_MAX_UT_PORTS; idx++) {
        sai_rc = sai_bridge_debug_lag_remove_member(lag_member_id[idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in removing lag member 0x%"PRIx64"\r\n",
                      sai_rc, lag_member_id[idx]);
        }
        sai_rc = sai_bridge_debug_bridge_port_type_port_create(port_id[idx], &bridge_port_id[idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in creating bridge port for port 0x%"PRIx64"\r\n",sai_rc,
                      port_id[idx]);
            return sai_rc;
        }
        sai_rc = sai_bridge_debug_vlan_member_create(def_vlan_id, bridge_port_id[idx],
                                                     &vlan_member_id);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in adding lag 0x%"PRIx64" to vlan 0x%"PRIx64"\r\n",
                      sai_rc, port_id[idx], def_vlan_id );
            return sai_rc;
        }

        SAI_DEBUG("Port id 0x%"PRIx64" bridge port 0x%"PRIx64"\r\n",
                  port_id[idx], bridge_port_id[idx]);
        SAI_DEBUG("NPU dump after removing lag member 0x%"PRIx64"\r\n",lag_member_id[idx]);
        sai_bridge_npu_vlan_debug_dump();
    }

    sai_rc = sai_bridge_debug_vlan_member_remove(lag_vlan_member_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing lag vlan member 0x%"PRIx64"\r\n",
                   sai_rc, lag_vlan_member_id);
    }

    sai_rc = sai_bridge_debug_remove_bridge_ports(1, &lag_bridge_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    sai_rc = sai_bridge_debug_lag_remove(lag_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing lag 0x%"PRIx64"\r\n", sai_rc, lag_id);
        return sai_rc;
    }

    sai_rc = sai_bridge_debug_vlan_remove(vlan_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing vlan 0x%"PRIx64"\r\n", sai_rc, vlan_id);
        return sai_rc;
    }

    SAI_DEBUG("NPU dump after cleanup");
    sai_bridge_npu_vlan_debug_dump();

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_mcast_lag_callback_npu_dump(void)
{
    sai_object_id_t lag_id;
    sai_object_id_t lag_bridge_port_id;
    sai_object_id_t lag_bridge_port_id_1d;
    sai_object_id_t lag_member_id[SAI_DEBUG_MAX_UT_PORTS];
    sai_object_id_t vlan_member_id;
    sai_object_id_t bridge_id;
    sai_object_id_t def_vlan_id;
    sai_object_id_t port_id[SAI_DEBUG_MAX_UT_PORTS];
    sai_object_id_t bridge_port_id[SAI_DEBUG_MAX_UT_PORTS];
    sai_status_t    sai_rc;
    sai_attribute_t attr;
    uint_t          idx = 0;

    attr.id = SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID;

    SAI_DEBUG("Default NPU statue dump\r\n");
    sai_bridge_npu_mcast_debug_dump();

    attr.id = SAI_SWITCH_ATTR_DEFAULT_VLAN_ID;

    sai_rc = sai_switch_api_query()->get_switch_attribute(SAI_DEFAULT_SWITCH_ID, 1, &attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in getting default vlan\r\n",sai_rc);
        return sai_rc;
    }
    def_vlan_id = attr.value.oid;
    SAI_DEBUG("Default vlan Id is 0x%"PRIx64"\r\n",def_vlan_id);
    sai_rc = sai_bridge_debug_init_get_bridge_ports(def_vlan_id, SAI_DEBUG_MAX_UT_PORTS,
                                                    bridge_port_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }


    sai_rc = sai_bridge_debug_lag_create(&lag_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating lag\r\n",sai_rc);
        return sai_rc;
    }

    sai_rc = sai_bridge_debug_bridge_port_type_port_create(lag_id, &lag_bridge_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in creating bridge port for lag 0x%"PRIx64"\r\n",sai_rc, lag_id);
        return sai_rc;
    }
    SAI_DEBUG("Lag id 0x%"PRIx64" bridge port 0x%"PRIx64"\r\n",lag_id, lag_bridge_port_id);

    sai_rc = sai_bridge_debug_bridge_create(&bridge_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }
    sai_rc = sai_bridge_debug_bridge_port_sub_port_create(bridge_id, lag_id, SAI_DEBUG_TEST_VLAN,
                                                          false, &lag_bridge_port_id_1d);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    sai_rc = sai_bridge_debug_get_ports_from_bridge_ports(SAI_DEBUG_MAX_UT_PORTS,
                                                          bridge_port_id, port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    sai_rc = sai_bridge_debug_remove_bridge_ports(SAI_DEBUG_MAX_UT_PORTS, bridge_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    for(idx = 0; idx< SAI_DEBUG_MAX_UT_PORTS; idx++) {
        sai_rc = sai_bridge_debug_lag_add_port(lag_id, port_id[idx], &lag_member_id[idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in adding port 0x%"PRIx64" to lag 0x%"PRIx64"\r\n",
                      sai_rc, port_id[idx], lag_id);
        }
        SAI_DEBUG("NPU dump after adding port 0x%"PRIx64" to lag 0x%"PRIx64" creating "
                  "lag member 0x%"PRIx64"\r\n", port_id[idx], lag_id, lag_member_id[idx]);
        sai_bridge_npu_mcast_debug_dump();
    }

    for(idx = 0; idx< SAI_DEBUG_MAX_UT_PORTS; idx++) {
        sai_rc = sai_bridge_debug_lag_remove_member(lag_member_id[idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in removing lag member 0x%"PRIx64"\r\n",
                      sai_rc, lag_member_id[idx]);
        }
        sai_rc = sai_bridge_debug_bridge_port_type_port_create(port_id[idx], &bridge_port_id[idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in creating bridge port for port 0x%"PRIx64"\r\n",sai_rc,
                      port_id[idx]);
            return sai_rc;
        }
        sai_rc = sai_bridge_debug_vlan_member_create(def_vlan_id, bridge_port_id[idx],
                                                     &vlan_member_id);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in adding lag 0x%"PRIx64" to vlan 0x%"PRIx64"\r\n",
                      sai_rc, port_id[idx], def_vlan_id );
            return sai_rc;
        }

        SAI_DEBUG("Port id 0x%"PRIx64" bridge port 0x%"PRIx64"\r\n",
                  port_id[idx], bridge_port_id[idx]);
        SAI_DEBUG("NPU dump after removing lag member 0x%"PRIx64"\r\n",lag_member_id[idx]);
        sai_bridge_npu_mcast_debug_dump();
    }

    sai_rc = sai_bridge_debug_remove_bridge_ports(1, &lag_bridge_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    sai_rc = sai_bridge_debug_remove_bridge_ports(1, &lag_bridge_port_id_1d);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }
    sai_rc = sai_bridge_debug_lag_remove(lag_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing lag 0x%"PRIx64"\r\n", sai_rc, lag_id);
        return sai_rc;
    }

    sai_rc = sai_bridge_debug_bridge_remove(bridge_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_DEBUG("Error %d in removing bridge 0x%"PRIx64"\r\n", sai_rc, bridge_id);
        return sai_rc;
    }
    SAI_DEBUG("NPU dump after removing lag 0x%"PRIx64"\r\n",lag_id);
    sai_bridge_npu_mcast_debug_dump();

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_admin_state_change_npu_dump(void)
{
    sai_status_t     sai_rc;
    uint_t           count = sai_bridge_port_total_count();
    sai_object_id_t *bridge_port_list = NULL;
    sai_object_id_t  bridge_port_id;
    sai_attribute_t  attr;

    bridge_port_list = calloc(count, sizeof(sai_object_id_t));

    sai_rc = sai_bridge_port_list_get(&count, bridge_port_list);

    do {
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in getting bridge port list",sai_rc);
            break;
        }

        bridge_port_id = bridge_port_list[0];
        attr.id = SAI_BRIDGE_PORT_ATTR_ADMIN_STATE;
        sai_rc = sai_bridge_api_query()->get_bridge_port_attribute(bridge_port_id, 1, &attr);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in getting admin state for bridge port 0x%"PRIx64"\r\n", sai_rc,
                      bridge_port_id);
            break;
        }

        SAI_DEBUG("Default admin state of bridge port 0x%"PRIx64" is %d\r\n",bridge_port_id,
                  attr.value.booldata);
        sai_bridge_npu_vlan_debug_dump();
        attr.value.booldata = !(attr.value.booldata);
        sai_rc = sai_bridge_api_query()->set_bridge_port_attribute(bridge_port_id, &attr);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in setting admin state for bridge port 0x%"PRIx64"\r\n", sai_rc,
                      bridge_port_id);
            break;
        }
        SAI_DEBUG("Updated admin state of bridge port 0x%"PRIx64" is %d\r\n",bridge_port_id,
                  attr.value.booldata);
        sai_bridge_npu_vlan_debug_dump();

        attr.value.booldata = !(attr.value.booldata);
        sai_rc = sai_bridge_api_query()->set_bridge_port_attribute(bridge_port_id, &attr);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in setting admin state for bridge port 0x%"PRIx64"\r\n", sai_rc,
                      bridge_port_id);
            break;
        }

        SAI_DEBUG("Updated admin state of bridge port 0x%"PRIx64" is %d\r\n",bridge_port_id,
                  attr.value.booldata);
        sai_bridge_npu_vlan_debug_dump();

    } while(0);
    free(bridge_port_list);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_debug_setup_1d_bridge(void)
{
    sai_object_id_t bridge_id;

    return sai_bridge_debug_bridge_create(&bridge_id);
}


sai_status_t sai_debug_setup_lag(void)
{
    sai_object_id_t lag_id;

    return sai_bridge_debug_lag_create(&lag_id);
}

sai_status_t sai_debug_setup_lag_member(sai_object_id_t lag_id, sai_object_id_t port_id)
{
    sai_object_id_t lag_member_id;

    return sai_bridge_debug_lag_add_port(lag_id, port_id, &lag_member_id);
}

sai_status_t sai_debug_bridge_setup_sub_port(sai_object_id_t bridge_id,
                                             sai_object_id_t port_id, int vlan_id, bool untagged)
{
    sai_object_id_t bridge_port_id;
    return sai_bridge_debug_bridge_port_sub_port_create(bridge_id, port_id, vlan_id, untagged,
                                                        &bridge_port_id);
}

void sai_bridge_debug_show_stats(sai_object_id_t bridge_id)
{

    sai_status_t      sai_rc;
    sai_bridge_stat_t bridge_stat[] = {SAI_BRIDGE_STAT_IN_OCTETS, SAI_BRIDGE_STAT_IN_PACKETS,
                                       SAI_BRIDGE_STAT_OUT_OCTETS, SAI_BRIDGE_STAT_OUT_PACKETS};

    unsigned int      num_stats = sizeof(bridge_stat)/ sizeof(sai_bridge_stat_t);
    unsigned int      stat_idx = 0;
    uint64_t          val = 0;

    for(stat_idx = 0; stat_idx < num_stats; stat_idx++) {
        sai_rc = sai_bridge_api_query()->get_bridge_stats(bridge_id, 1,
                                                          &bridge_stat[stat_idx],&val);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Stat id %d  get returns error %d\r\n",
                   bridge_stat[stat_idx], sai_rc);
        } else {
            SAI_DEBUG("Stat id %d value %ld \r\n", bridge_stat[stat_idx], val);
        }
    }
}

void sai_bridge_debug_clear_stats(sai_object_id_t bridge_id)
{

    sai_status_t      sai_rc;
    sai_bridge_stat_t bridge_stat[] = {SAI_BRIDGE_STAT_IN_OCTETS, SAI_BRIDGE_STAT_IN_PACKETS,
                                       SAI_BRIDGE_STAT_OUT_OCTETS, SAI_BRIDGE_STAT_OUT_PACKETS};

    unsigned int      num_stats = sizeof(bridge_stat)/ sizeof(sai_bridge_stat_t);
    unsigned int      stat_idx = 0;

    for(stat_idx = 0; stat_idx < num_stats; stat_idx++) {
        sai_rc = sai_bridge_api_query()->clear_bridge_stats(bridge_id, 1,
                                                            &bridge_stat[stat_idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Stat id %d clear returns error %d\r\n",
                      bridge_stat[stat_idx], sai_rc);
        } else {
            SAI_DEBUG("Stat id %d cleared \r\n", bridge_stat[stat_idx]);
        }
    }
}

void sai_bridge_debug_show_bridge_port_stats(sai_object_id_t bridge_port_id)
{
    sai_status_t           sai_rc;
    sai_bridge_port_stat_t bridge_port_stat[] = {SAI_BRIDGE_PORT_STAT_IN_OCTETS,
                                                 SAI_BRIDGE_PORT_STAT_IN_PACKETS,
                                                 SAI_BRIDGE_PORT_STAT_OUT_OCTETS,
                                                 SAI_BRIDGE_PORT_STAT_OUT_PACKETS};

    unsigned int           num_stats = sizeof(bridge_port_stat)/ sizeof(sai_bridge_port_stat_t);
    unsigned int           stat_idx = 0;
    uint64_t               val = 0;

    for(stat_idx = 0; stat_idx < num_stats; stat_idx++) {
        val = 0;
        sai_rc = sai_bridge_api_query()->get_bridge_port_stats(bridge_port_id, 1,
                                                               &bridge_port_stat[stat_idx],&val);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Stat id %d get returns error %d\r\n", bridge_port_stat[stat_idx], sai_rc);
        } else {
            SAI_DEBUG("Stat id %d value %ld\r\n", bridge_port_stat[stat_idx], val);
        }
    }
}

void sai_bridge_debug_clear_bridge_port_stats(sai_object_id_t bridge_port_id)
{
    sai_status_t           sai_rc;
    sai_bridge_port_stat_t bridge_port_stat[] = {SAI_BRIDGE_PORT_STAT_IN_OCTETS,
                                                 SAI_BRIDGE_PORT_STAT_IN_PACKETS,
                                                 SAI_BRIDGE_PORT_STAT_OUT_OCTETS,
                                                 SAI_BRIDGE_PORT_STAT_OUT_PACKETS};

    unsigned int           num_stats = sizeof(bridge_port_stat)/ sizeof(sai_bridge_port_stat_t);
    unsigned int           stat_idx = 0;

    for(stat_idx = 0; stat_idx < num_stats; stat_idx++) {
        sai_rc = sai_bridge_api_query()->clear_bridge_port_stats(bridge_port_id, 1,
                                                               &bridge_port_stat[stat_idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Stat id %d clear returns error %d\r\n", bridge_port_stat[stat_idx], sai_rc);
        } else {
            SAI_DEBUG("Stat id %d cleared\r\n", bridge_port_stat[stat_idx]);
        }
    }
}

