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
* @file sai_vlan_debug.c
*
* @brief This file contains debug APIs for SAI VLAN module
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "saivlan.h"
#include "saitypes.h"
#include "sai_oid_utils.h"
#include "sai_vlan_common.h"
#include "sai_vlan_api.h"
#include "sai_debug_utils.h"
#include "std_utils.h"
#include "sai_vlan_main.h"

static std_code_text_t sai_vlan_tagging_mode_str_tbl[]={
    {SAI_VLAN_TAGGING_MODE_UNTAGGED, "UNTAGGED"},
    {SAI_VLAN_TAGGING_MODE_TAGGED, "TAGGED"},
    {SAI_VLAN_TAGGING_MODE_PRIORITY_TAGGED, "PRIORITY_TAGGED"}
};

static inline const char *sai_vlan_tagging_mode_to_str(sai_vlan_tagging_mode_t tagging_mode)
{
    const char *tagging_mode_str = NULL;

    if((tagging_mode_str = std_code_to_text(tagging_mode,
                    sai_vlan_tagging_mode_str_tbl,
                    (sizeof(sai_vlan_tagging_mode_str_tbl)/
                     sizeof(sai_vlan_tagging_mode_str_tbl[0]))))
            != NULL) {
        return tagging_mode_str;
    } else {
        return "UNKNOWN";
    }
}

void sai_vlan_dump_member_node(sai_vlan_member_node_t *vlan_member_info, char *prefix)
{
    SAI_DEBUG("%sMember ID:0x%lx",prefix,vlan_member_info->vlan_member_id);
    SAI_DEBUG("%sBridge Port ID:0x%lx",prefix,vlan_member_info->bridge_port_id);
    SAI_DEBUG("%sSwitch ID:0x%lx",prefix,vlan_member_info->switch_id);
    SAI_DEBUG("%sVLAN ID:0x%lx",prefix,vlan_member_info->vlan_id);
    SAI_DEBUG("%sTagging mode:%s(%d)",prefix,sai_vlan_tagging_mode_to_str(
                vlan_member_info->tagging_mode),vlan_member_info->tagging_mode);
}

void sai_vlan_dump_global_info(sai_vlan_id_t vlan_id, bool all)
{
    sai_vlan_member_dll_node_t *vlan_port_node = NULL;
    std_dll *node = NULL;
    sai_vlan_global_cache_node_t *vlan_port_list = NULL;
    sai_vlan_id_t start_id,end_id,id;

    if(all) {
        start_id = SAI_MIN_VLAN_TAG_ID;
        end_id = SAI_MAX_VLAN_TAG_ID;
    } else {
        start_id = end_id = vlan_id;
    }

    for(id = start_id; id <= end_id; id++) {
        vlan_port_list = sai_vlan_portlist_cache_read(id);
        if(vlan_port_list == NULL) {
            if(!all) {
                SAI_DEBUG("VLAN:%d - Does not exist",id);
                break;
            } else {
                continue;
            }
        }
        SAI_DEBUG("VLAN:%d VLAN ID:0x%lx",id,
                sai_vlan_id_to_vlan_obj_id(vlan_port_list->vlan_id));
        SAI_DEBUG("\tPort count:%u",vlan_port_list->port_count);
        SAI_DEBUG("\tMax learned address:%u",vlan_port_list->max_learned_address);
        SAI_DEBUG("\tLearn disble:%s",(vlan_port_list->learn_disable)?"True":"False");
        SAI_DEBUG("\tMeta data:0x%x",vlan_port_list->meta_data);

        for(node = std_dll_getfirst(&(vlan_port_list->member_list));
                node != NULL;
                node = std_dll_getnext(&(vlan_port_list->member_list),node)) {
            vlan_port_node = (sai_vlan_member_dll_node_t *)node;
            SAI_DEBUG("\tBridge Port ID:0x%lx",
                    vlan_port_node->vlan_member_info->bridge_port_id);
            sai_vlan_dump_member_node(vlan_port_node->vlan_member_info,"\t\t");
        }
    }
    return;
}

void sai_vlan_dump_member_info(sai_vlan_id_t vlan_id, sai_object_id_t bridge_port_id, bool all)
{
    sai_vlan_member_node_t *vlan_member_node_ptr = NULL;
    sai_vlan_member_dll_node_t *vlan_member_node_dll_ptr = NULL;
    rbtree_handle vlan_member_tree_ptr = sai_vlan_global_member_tree_get();

    if(all) {
        for(vlan_member_node_ptr = std_rbtree_getfirst(vlan_member_tree_ptr);
                vlan_member_node_ptr != NULL;
                vlan_member_node_ptr = std_rbtree_getnext(vlan_member_tree_ptr,
                    vlan_member_node_ptr)) {
            SAI_DEBUG("Port ID:0x%lx",vlan_member_node_ptr->bridge_port_id);
            sai_vlan_dump_member_node(vlan_member_node_ptr,"\t");
        }
    } else {
        if((vlan_member_node_dll_ptr = sai_find_vlan_member_node_from_port(
            vlan_id,bridge_port_id)) != NULL) {
            SAI_DEBUG("VLAN:%d Bridge Port ID:0x%lx",vlan_id,bridge_port_id);
            sai_vlan_dump_member_node(
            vlan_member_node_dll_ptr->vlan_member_info,"\t");
        } else {
            SAI_DEBUG("VLAN:%d Bridge Port ID:0x%lx - Not found",vlan_id,bridge_port_id);
        }
    }
}
