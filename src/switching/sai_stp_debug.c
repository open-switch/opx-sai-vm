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
* @file sai_stp_debug.c
*
* @brief This file contains debug APIs for SAI STP module
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "std_utils.h"
#include "saitypes.h"
#include "saistp.h"
#include "sai_stp_api.h"
#include "sai_stp_defs.h"
#include "sai_oid_utils.h"
#include "sai_debug_utils.h"

static std_code_text_t sai_stp_port_state_str_tbl[]={
    {SAI_STP_PORT_STATE_LEARNING, "LEARNING"},
    {SAI_STP_PORT_STATE_FORWARDING, "FORWARDING"},
    {SAI_STP_PORT_STATE_BLOCKING, "BLOCKING"}
};

static inline const char *sai_stp_port_state_to_str(sai_stp_port_state_t stp_port_state)
{
    const char *stp_port_state_str = NULL;

    if((stp_port_state_str = std_code_to_text(stp_port_state,
                    sai_stp_port_state_str_tbl,
                    (sizeof(sai_stp_port_state_str_tbl)/
                     sizeof(sai_stp_port_state_str_tbl[0]))))
            != NULL) {
        return stp_port_state_str;
    } else {
        return "UNKNOWN";
    }
}

void sai_stp_dump_stp_info_node(dn_sai_stp_info_t *p_stp_info)
{
    sai_vlan_id_t *p_vlan_node = NULL;
    dn_sai_stp_port_info_t *p_stp_port_info = NULL;

    SAI_DEBUG("STP Inst ID:0x%lx",p_stp_info->stp_inst_id);
    SAI_DEBUG("Num Ports:%u",p_stp_info->num_ports);
    SAI_DEBUG("VLANs:");
    for(p_vlan_node = std_rbtree_getfirst(p_stp_info->vlan_tree);
            p_vlan_node != NULL;
            p_vlan_node = std_rbtree_getnext(
                p_stp_info->vlan_tree,p_vlan_node)) {
        SAI_DEBUG("\t%4d",*p_vlan_node);
    }

    SAI_DEBUG("STP Ports:");
    for(p_stp_port_info = std_rbtree_getfirst(p_stp_info->stp_port_tree);
            p_stp_port_info != NULL;
            p_stp_port_info = std_rbtree_getnext(
                p_stp_info->stp_port_tree,p_stp_port_info)) {
        SAI_DEBUG("\tBridge Port ID:0x%lx",p_stp_port_info->bridge_port_id);
        SAI_DEBUG("\t\tSTP Port ID:0x%lx",p_stp_port_info->stp_port_id);
        SAI_DEBUG("\t\tSTP Inst ID:0x%lx",p_stp_port_info->stp_inst_id);
        SAI_DEBUG("\t\tSTP Port State:%s",sai_stp_port_state_to_str(
        p_stp_port_info->port_state));
    }
}

void sai_stp_dump_global_info(sai_object_id_t inst_id, bool all)
{
    sai_object_id_t stp_inst_id = SAI_NULL_OBJECT_ID;
    dn_sai_stp_info_t *p_stp_info = NULL;
    rbtree_handle stp_info_tree = sai_stp_global_info_tree_get();

    if(all) {
        for(p_stp_info = std_rbtree_getfirst(stp_info_tree);
                p_stp_info != NULL;
                p_stp_info = std_rbtree_getnext(
                    stp_info_tree,p_stp_info)) {
            sai_stp_dump_stp_info_node(p_stp_info);
        }
    } else {
        if(sai_is_obj_id_stp_instance(inst_id)) {
            stp_inst_id = inst_id;
        } else {
            stp_inst_id = sai_uoid_create(SAI_OBJECT_TYPE_STP,inst_id);
        }

        p_stp_info = (dn_sai_stp_info_t *)std_rbtree_getexact(
                stp_info_tree,(void *)&stp_inst_id);
        if(!p_stp_info) {
            SAI_DEBUG("STP Inst:0x%lx - Does not exist",inst_id);
        } else {
            sai_stp_dump_stp_info_node(p_stp_info);
        }
    }
}
