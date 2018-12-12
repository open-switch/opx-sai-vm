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
* @file sai_lag_debug.c
*
* @brief This file contains debug APIs for SAI LAG module
*************************************************************************/

#include "saistatus.h"
#include "saitypes.h"
#include "std_llist.h"
#include "sai_lag_common.h"
#include "sai_lag_api.h"
#include "sai_debug_utils.h"
#include "sai_oid_utils.h"
#include "sai_npu_lag.h"
#include "sai_common_infra.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

sai_status_t sai_dump_lag_port_list(sai_object_id_t lag_id)
{
    sai_lag_node_t *lag_node = sai_lag_node_get(lag_id);
    sai_lag_port_node_t *lag_port_node = NULL;
    std_dll *node = NULL;

    if(lag_node == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    SAI_DEBUG("Lag ID:0x%"PRIx64" Trunk ID:%d  port count:%d",lag_node->sai_lag_id,
            (int)sai_uoid_npu_obj_id_get(lag_node->sai_lag_id),
            lag_node->port_count);
    SAI_DEBUG("Port List:");
    for(node = std_dll_getfirst(&(lag_node->port_list));
        node != NULL;
        node = std_dll_getnext(&(lag_node->port_list),node)) {
        lag_port_node = (sai_lag_port_node_t *)node;
        SAI_DEBUG("\t0x%"PRIx64"(%d) ",lag_port_node->port_id,
              (int)sai_uoid_npu_obj_id_get(lag_port_node->port_id));
    }
    SAI_DEBUG("\r\n");

    return SAI_STATUS_SUCCESS;
}

void sai_dump_lag_info(sai_object_id_t lag_id, bool all)
{
    sai_lag_node_t *lag_node = NULL;
    std_dll *node = NULL;
    std_dll_head *lag_list = sai_lag_list_get();
    sai_object_id_t sai_lag_id = SAI_NULL_OBJECT_ID;

    if(all) {
        for(node = std_dll_getfirst(lag_list);
                node != NULL;
                node = std_dll_getnext(lag_list,node)) {
            lag_node = (sai_lag_node_t *)node;
            sai_dump_lag_port_list(lag_node->sai_lag_id);
        }
    } else {
        if(sai_is_obj_id_stp_instance(lag_id)) {
            sai_lag_id = lag_id;
        } else {
            sai_lag_id = sai_uoid_create(SAI_OBJECT_TYPE_LAG,
                    (sai_npu_object_id_t)lag_id);
        }

        if(sai_dump_lag_port_list(sai_lag_id) != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Lag/Trunk ID:0x%"PRIx64" not found or invalid\r\n",lag_id);
        }
    }

    return;
}

/*To be called after creating LAG and its members*/
sai_status_t sai_debug_validate_lag_attribute_set_on_members(sai_object_id_t lag_id)
{
    sai_lag_node_t      *lag_node = sai_lag_node_get(lag_id);
    sai_lag_port_node_t *lag_port_node = NULL;
    std_dll             *node = NULL;
    sai_attribute_t      get_attr[4];
    sai_status_t         sai_rc = SAI_STATUS_FAILURE;

    get_attr[0].id =  SAI_LAG_ATTR_PORT_VLAN_ID;
    get_attr[1].id =  SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY;
    get_attr[2].id =  SAI_LAG_ATTR_DROP_UNTAGGED;
    get_attr[3].id =  SAI_LAG_ATTR_DROP_TAGGED;

    for(node = std_dll_getfirst(&(lag_node->port_list));
        node != NULL;
        node = std_dll_getnext(&(lag_node->port_list),node)) {
        lag_port_node = (sai_lag_port_node_t *)node;
        sai_rc = sai_lag_npu_api_get()->lag_attribute_get_from_member(lag_id,
                                                                      lag_port_node->member_id,
                                                                      4, get_attr);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_DEBUG("Error %d in getting lag attributes for member 0x%"PRIx64" lag 0x%"PRIx64""
                      " from npu\r\n", sai_rc, lag_port_node->member_id, lag_id);
            return sai_rc;
        }

        if(lag_node->pvid != get_attr[0].value.u16) {
            SAI_DEBUG("PVID of LAG 0x%"PRIx64" is %d member 0x%"PRIx64" is %d",
                      lag_id, lag_node->pvid, lag_port_node->member_id, get_attr[0].value.u16);
             return SAI_STATUS_FAILURE;
        }
        if(lag_node->default_vlan_priority != get_attr[1].value.u8) {
            SAI_DEBUG("Default VLAN Priority of LAG 0x%"PRIx64" is %d member 0x%"PRIx64" is %d",
                      lag_id, lag_node->default_vlan_priority, lag_port_node->member_id,
                      get_attr[1].value.u8);
             return SAI_STATUS_FAILURE;
        }

        if(lag_node->drop_untagged != get_attr[2].value.booldata) {
            SAI_DEBUG("Drop untagged of LAG 0x%"PRIx64" is %d member 0x%"PRIx64" is %d",
                      lag_id, lag_node->drop_untagged, lag_port_node->member_id,
                      get_attr[2].value.booldata);
             return SAI_STATUS_FAILURE;
        }

        if(lag_node->drop_tagged != get_attr[3].value.booldata) {
            SAI_DEBUG("Drop tagged of LAG 0x%"PRIx64" is %d member 0x%"PRIx64" is %d",
                      lag_id, lag_node->drop_tagged, lag_port_node->member_id,
                      get_attr[3].value.booldata);
             return SAI_STATUS_FAILURE;
        }

    }
    return SAI_STATUS_SUCCESS;
}
