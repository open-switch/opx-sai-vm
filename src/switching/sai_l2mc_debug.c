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
* @file sai_l2mc_debug.c
*
* @brief This file contains debug APIs of SAI L2MC Group and SAI L2MC modules.
*************************************************************************/

#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "sail2mc.h"
#include "sail2mcgroup.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_l2mc_common.h"
#include "sai_l2mc_api.h"
#include "sai_debug_utils.h"
#include "sai_l3_util.h"
#include "sai_npu_l2mc.h"
#include "sai_common_infra.h"

static void sai_l2mc_group_info_dump(dn_sai_l2mc_group_node_t* l2mc_group_node)
{
    dn_sai_l2mc_member_dll_node_t *l2mc_member_dll_node = NULL;
    std_dll *node = NULL;

    SAI_DEBUG("L2MC Group ID: 0x%"PRIx64"\r\n",l2mc_group_node->l2mc_group_id);
    SAI_DEBUG("-----------------------------\r\n");
    SAI_DEBUG("Member count: %d\r\n",l2mc_group_node->port_count);
    SAI_DEBUG("Bridge ID: 0x%"PRIx64"\r\n",l2mc_group_node->bridge_id);
    SAI_DEBUG("Bridge ref count: %d\r\n",l2mc_group_node->bridge_ref_count);
    sai_l2mc_npu_api_get()->l2mc_group_dump_hw_info(l2mc_group_node->hw_info);
    SAI_DEBUG("\r\nMember List\r\n");
    SAI_DEBUG("+++++++++++++++++++++++++++++\r\n");
    for(node = std_dll_getfirst(&(l2mc_group_node->member_list));  node != NULL;
        node = std_dll_getnext(&(l2mc_group_node->member_list),node)) {
        l2mc_member_dll_node = (dn_sai_l2mc_member_dll_node_t *)node;
        SAI_DEBUG("Member Node: 0x%"PRIx64" - Bridge port: 0x%"PRIx64"\r\n",
                  l2mc_member_dll_node->l2mc_member_info->l2mc_member_id,
                  l2mc_member_dll_node->l2mc_member_info->bridge_port_id);
    }
    SAI_DEBUG("+++++++++++++++++++++++++++++\r\n\r\n");

}

void sai_l2mc_group_dump_all(void)
{
    dn_sai_l2mc_group_node_t* l2mc_group_node = NULL;
    dn_sai_l2mc_group_node_t  tmp_group_node;

    memset(&tmp_group_node, 0, sizeof(tmp_group_node));

    l2mc_group_node = sai_l2mc_group_get_next(&tmp_group_node);

    while(l2mc_group_node != NULL) {
        sai_l2mc_group_info_dump(l2mc_group_node);
        l2mc_group_node = sai_l2mc_group_get_next(l2mc_group_node);
    }
}

void sai_l2mc_group_dump(sai_object_id_t l2mc_group_obj)
{
    dn_sai_l2mc_group_node_t* l2mc_group_node = sai_find_l2mc_group_node(l2mc_group_obj);

    if(l2mc_group_node == NULL){
        SAI_DEBUG("Error - L2MC Group Object 0x%"PRIx64" not found\r\n", l2mc_group_obj);
    } else {
        sai_l2mc_group_info_dump(l2mc_group_node);
    }
}


static void sai_l2mc_member_info_dump(dn_sai_l2mc_member_node_t* l2mc_member_node)
{
    char ip_addr_str[SAI_FIB_MAX_BUFSZ] = {0};


    SAI_DEBUG("L2MC Member ID: 0x%"PRIx64"\r\n",l2mc_member_node->l2mc_member_id);
    SAI_DEBUG("-----------------------------\r\n");
    SAI_DEBUG("Switch ID: 0x%"PRIx64"\r\n",l2mc_member_node->switch_id);
    SAI_DEBUG("L2MC Group ID: 0x%"PRIx64"\r\n",l2mc_member_node->l2mc_group_id);
    SAI_DEBUG("Bridge port ID: 0x%"PRIx64"\r\n",l2mc_member_node->bridge_port_id);
    SAI_DEBUG("Encap IP %s\r\n",sai_ip_addr_to_str(&l2mc_member_node->encap_ip, ip_addr_str, SAI_FIB_MAX_BUFSZ));
    SAI_DEBUG("-----------------------------\r\n");
}

void sai_l2mc_member_dump_all(void)
{
    dn_sai_l2mc_member_node_t* l2mc_member_node = NULL;
    dn_sai_l2mc_member_node_t  tmp_member_node;

    memset(&tmp_member_node, 0, sizeof(tmp_member_node));

    l2mc_member_node = sai_l2mc_member_get_next(&tmp_member_node);

    while(l2mc_member_node != NULL) {
        sai_l2mc_member_info_dump(l2mc_member_node);
        l2mc_member_node = sai_l2mc_member_get_next(l2mc_member_node);
    }
}

void sai_l2mc_member_dump(sai_object_id_t l2mc_member_obj)
{
    dn_sai_l2mc_member_node_t* l2mc_member_node = sai_find_l2mc_member_node(l2mc_member_obj);

    if(l2mc_member_node == NULL){
        SAI_DEBUG("Error - L2MC Member Object 0x%"PRIx64" not found\r\n", l2mc_member_obj);
    } else {
        sai_l2mc_member_info_dump(l2mc_member_node);
    }
}

