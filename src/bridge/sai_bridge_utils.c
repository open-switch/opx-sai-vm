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
* @file sai_bridge_utils.c
*
* @brief This file contains utility APIs for SAI BRIDGE module
*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "saibridge.h"
#include "saistatus.h"
#include "saitypes.h"
#include "std_llist.h"
#include "sai_bridge_api.h"
#include "sai_bridge_common.h"
#include "std_mutex_lock.h"
#include "std_assert.h"
#include "sai_switch_utils.h"
#include "sai_port_utils.h"
#include "sai_oid_utils.h"
#include "sai_gen_utils.h"
#include "sai_map_utl.h"
#include "sai_l2mc_api.h"

static std_mutex_lock_create_static_init_fast(bridge_lock);

void sai_bridge_lock(void)
{
    std_mutex_lock(&bridge_lock);
}

void sai_bridge_unlock(void)
{
    std_mutex_unlock(&bridge_lock);
}

void sai_bridge_init_default_bridge_info(dn_sai_bridge_info_t *bridge_info)
{
    uint_t idx = 0;
    if(bridge_info == NULL) {
        SAI_BRIDGE_LOG_TRACE("NULL bridge info passed in bridge info init");
        return;
    }
    bridge_info->max_learned_address = 0;
    bridge_info->learn_disable = false;
    bridge_info->ref_count = 0;
    for(idx = 0; idx < SAI_BRIDGE_FLOOD_TYPE_MAX; idx++) {
        bridge_info->flood_control[idx] = SAI_BRIDGE_FLOOD_CONTROL_TYPE_SUB_PORTS;
        bridge_info->l2mc_flood_group[idx] = SAI_NULL_OBJECT_ID;
    }
}

void sai_bridge_init_default_bridge_port_info(dn_sai_bridge_port_info_t *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        SAI_BRIDGE_LOG_TRACE("NULL bridge port info passed in bridge port info init");
        return;
    }
    bridge_port_info->fdb_learn_mode = SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW;
    bridge_port_info->max_learned_address = 0;
    bridge_port_info->learn_limit_violation_action = SAI_PACKET_ACTION_DROP;
    bridge_port_info->admin_state = false;
    bridge_port_info->ingress_filtering = false;
    bridge_port_info->egr_tagging_mode = SAI_BRIDGE_PORT_TAGGING_MODE_TAGGED;
    bridge_port_info->ref_count = 0;
    bridge_port_info->fdb_count = 0;
}

sai_status_t sai_bridge_map_insert (sai_object_id_t bridge_id, sai_object_id_t bridge_port_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_BRIDGE_TO_BRIDGE_PORT_LIST;
    key.id1  = bridge_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = bridge_port_id;

    return sai_map_insert (&key, &value);
}

sai_status_t sai_bridge_map_remove (sai_object_id_t bridge_id, sai_object_id_t bridge_port_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    uint_t       count = 0;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_BRIDGE_TO_BRIDGE_PORT_LIST;
    key.id1  = bridge_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = bridge_port_id;

    sai_map_delete_elements (&key, &value, SAI_MAP_VAL_FILTER_VAL1);

    if (sai_map_get_val_count (&key, &count) == SAI_STATUS_SUCCESS) {
        if (count == 0) {
            sai_map_delete (&key);
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_map_port_list_get (sai_object_id_t  bridge_id,
                                           uint_t          *count,
                                           sai_object_id_t *bridge_port_list)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_status_t   rc = SAI_STATUS_FAILURE;
    uint_t         index = 0;
    sai_map_data_t data;
    uint_t         map_cnt = 0;

    if((count == NULL) || (bridge_port_list == NULL)) {
        SAI_BRIDGE_LOG_TRACE("Error count is %p bridge_port_list is %p for bridge id 0x%"PRIx64""
                             " in bridge map port list get",count, bridge_port_list, bridge_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data , 0, sizeof (sai_map_data_t));

    key.type = SAI_MAP_TYPE_BRIDGE_TO_BRIDGE_PORT_LIST;
    key.id1  = bridge_id;

    rc = sai_map_get_val_count (&key, &map_cnt);
    if(rc == SAI_STATUS_ITEM_NOT_FOUND) {
        *count = 0;
        return SAI_STATUS_SUCCESS;
    }

    if(*count < map_cnt) {
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    value.data  = &data;

    for (index = 0; index < map_cnt; index++) {
        rc = sai_map_get_element_at_index(&key, index, &value);

        if (rc != SAI_STATUS_SUCCESS) {
            return rc;
        }
        bridge_port_list [index] = data.val1;
    }
    *count = map_cnt;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_map_get_bridge_port_at_index (sai_object_id_t  bridge_id,
                                                      uint_t           index,
                                                      sai_object_id_t *bridge_port_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_status_t   rc = SAI_STATUS_FAILURE;
    sai_map_data_t data;

    if(bridge_port_id == NULL) {
        SAI_BRIDGE_LOG_TRACE("Error bridge_port_id is NULL in bridge map port list get for bridge"
                             " 0x%"PRIx64"", bridge_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data , 0, sizeof (sai_map_data_t));

    key.type = SAI_MAP_TYPE_BRIDGE_TO_BRIDGE_PORT_LIST;
    key.id1  = bridge_id;

    value.data  = &data;

    rc = sai_map_get_element_at_index(&key, index, &value);

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }
    *bridge_port_id = data.val1;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_map_get_port_count (sai_object_id_t  bridge_id,
                                            uint_t        *p_out_count)
{
    sai_map_key_t  key;
    sai_status_t   rc = SAI_STATUS_FAILURE;

    if(p_out_count == NULL) {
        SAI_BRIDGE_LOG_TRACE("Error count is NULL for bridge id 0x%"PRIx64""
                             " in bridge map port count get", bridge_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    memset (&key, 0, sizeof (key));

    key.type = SAI_MAP_TYPE_BRIDGE_TO_BRIDGE_PORT_LIST;
    key.id1  = bridge_id;

    rc = sai_map_get_val_count (&key, p_out_count);

    if(rc == SAI_STATUS_ITEM_NOT_FOUND) {
        *p_out_count = 0;
        return SAI_STATUS_SUCCESS;
    }

    return rc;
}

sai_status_t sai_bridge_port_vlan_to_bridge_port_map_insert (sai_object_id_t port_id,
                                                             sai_vlan_id_t vlan_id,
                                                             sai_object_id_t bridge_port_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_PORT_VLAN_TO_BRIDGE_PORT_LIST;
    key.id1  = port_id;
    key.id2  = vlan_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = bridge_port_id;

    return sai_map_insert (&key, &value);
}

static sai_status_t sai_bridge_get_bridge_port_id_from_port_vlan (sai_object_id_t port_id,
                                                                  sai_vlan_id_t vlan_id,
                                                                  sai_object_id_t *bridge_port_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    sai_status_t   rc = SAI_STATUS_FAILURE;

    if(bridge_port_id == NULL) {
        SAI_BRIDGE_LOG_TRACE("Error bridge_port_id is NULL for port id 0x%"PRIx64" vlan id %d"
                             " in bridge port get from port vlan",port_id, vlan_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_PORT_VLAN_TO_BRIDGE_PORT_LIST;
    key.id1  = port_id;
    key.id2  = vlan_id;

    value.count = 1;
    value.data  = &data;


    rc = sai_map_get (&key, &value);

    if (rc != SAI_STATUS_SUCCESS) {
        *bridge_port_id = SAI_NULL_OBJECT_ID;
        return rc;
    }

    *bridge_port_id = data.val1;

    return SAI_STATUS_SUCCESS;
}

bool sai_bridge_is_sub_port_created(sai_object_id_t port_id, sai_vlan_id_t vlan_id)
{
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_status_t    sai_rc = SAI_STATUS_FAILURE;

    sai_rc = sai_bridge_get_bridge_port_id_from_port_vlan(port_id, vlan_id, &bridge_port_id);

    if((sai_rc == SAI_STATUS_SUCCESS) && (bridge_port_id != SAI_NULL_OBJECT_ID)) {
        return true;
    }
    return false;
}

sai_status_t sai_bridge_port_vlan_to_bridge_port_map_remove (sai_object_id_t port_id,
                                                             sai_vlan_id_t vlan_id)
{
    sai_map_key_t  key;

    memset (&key, 0, sizeof (key));

    key.type = SAI_MAP_TYPE_PORT_VLAN_TO_BRIDGE_PORT_LIST;
    key.id1  = port_id;
    key.id2  = vlan_id;

    sai_map_delete (&key);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_lag_to_bridge_port_map_insert (sai_object_id_t lag_id,
                                                sai_object_id_t bridge_port_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_LAG_TO_BRIDGE_PORT_LIST;
    key.id1  = lag_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = bridge_port_id;

    return sai_map_insert (&key, &value);
}

sai_status_t sai_lag_to_bridge_port_map_remove (sai_object_id_t lag_id,
                                                sai_object_id_t bridge_port_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    uint_t         count = 0;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_LAG_TO_BRIDGE_PORT_LIST;
    key.id1  = lag_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = bridge_port_id;

    sai_map_delete_elements (&key, &value, SAI_MAP_VAL_FILTER_VAL1);

    if (sai_map_get_val_count (&key, &count) == SAI_STATUS_SUCCESS) {
        if (count == 0) {
            sai_map_delete (&key);
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_lag_bridge_map_port_list_get (sai_object_id_t  lag_id,
                                               uint_t          *count,
                                               sai_object_id_t *bridge_port_list)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_status_t   rc = SAI_STATUS_FAILURE;
    sai_map_data_t data;
    uint_t         map_cnt = 0;
    uint_t         index;


    if((count == NULL) || (bridge_port_list == NULL)) {
        SAI_BRIDGE_LOG_TRACE("Error count is %p bridge_port_list is %p for lag id 0x%"PRIx64""
                             " in lag map bridge port list get",count, bridge_port_list, lag_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (sai_map_data_t));

    key.type = SAI_MAP_TYPE_LAG_TO_BRIDGE_PORT_LIST;
    key.id1  = lag_id;

    rc = sai_map_get_val_count (&key, &map_cnt);
    if(rc == SAI_STATUS_ITEM_NOT_FOUND) {
        *count = 0;
        return SAI_STATUS_SUCCESS;
    }

    if(*count < map_cnt) {
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    value.data  = &data;

    for (index = 0; index < map_cnt; index++) {
        rc = sai_map_get_element_at_index(&key, index, &value);

        if (rc != SAI_STATUS_SUCCESS) {
            return rc;
        }
        bridge_port_list [index] = data.val1;
    }

    *count = map_cnt;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_lag_map_get_bridge_port_count (sai_object_id_t  lag_id,
                                                uint_t          *p_out_count)
{
    sai_map_key_t  key;
    sai_status_t   rc = SAI_STATUS_FAILURE;

    if(p_out_count == NULL) {
        SAI_BRIDGE_LOG_TRACE("Error count is NULL for lag id 0x%"PRIx64""
                             " in lag map bridge port list get", lag_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    memset (&key, 0, sizeof (key));

    key.type = SAI_MAP_TYPE_LAG_TO_BRIDGE_PORT_LIST;
    key.id1  = lag_id;

    rc = sai_map_get_val_count (&key, p_out_count);
    if(rc == SAI_STATUS_ITEM_NOT_FOUND) {
        *p_out_count = 0;
        return SAI_STATUS_SUCCESS;
    }

    if(rc == SAI_STATUS_ITEM_NOT_FOUND) {
        *p_out_count = 0;
        return SAI_STATUS_SUCCESS;
    }
    return rc;
}

sai_status_t sai_bridge_port_to_vlan_member_map_insert (sai_object_id_t bridge_port_id,
                                                        sai_object_id_t vlan_member_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    sai_status_t   rc = SAI_STATUS_FAILURE;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_BRIDGE_PORT_TO_VLAN_MEMBER_LIST;
    key.id1  = bridge_port_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = vlan_member_id;

    rc = sai_map_insert (&key, &value);
    if(rc == SAI_STATUS_SUCCESS) {
        sai_bridge_port_increment_ref_count(bridge_port_id);
    }
    return rc;
}

sai_status_t sai_bridge_port_to_vlan_member_map_remove (sai_object_id_t bridge_port_id,
                                                        sai_object_id_t vlan_member_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    uint_t         count = 0;
    sai_status_t   rc = SAI_STATUS_FAILURE;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_BRIDGE_PORT_TO_VLAN_MEMBER_LIST;
    key.id1  = bridge_port_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = vlan_member_id;

    rc = sai_map_delete_elements (&key, &value, SAI_MAP_VAL_FILTER_VAL1);

    if (sai_map_get_val_count (&key, &count) == SAI_STATUS_SUCCESS) {
        if (count == 0) {
            sai_map_delete (&key);
        }
    }

    if(rc == SAI_STATUS_SUCCESS) {
        sai_bridge_port_decrement_ref_count(bridge_port_id);
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_get_vlan_member_at_index (sai_object_id_t bridge_port_id,
                                                      uint_t           index,
                                                      sai_object_id_t *vlan_member_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_status_t   rc = SAI_STATUS_FAILURE;
    sai_map_data_t data;

    if(vlan_member_id == NULL) {
        SAI_BRIDGE_LOG_TRACE("vlan member id is NULL for bridge port id "
                             " 0x%"PRIx64" in bridge port vlan member list get", bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (sai_map_data_t));


    key.type = SAI_MAP_TYPE_BRIDGE_PORT_TO_VLAN_MEMBER_LIST;
    key.id1  = bridge_port_id;

    value.data  = &data;

    rc = sai_map_get_element_at_index(&key, index, &value);

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }
    *vlan_member_id = data.val1;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_to_vlan_member_count_get(sai_object_id_t  bridge_port_id,
                                                      uint_t          *p_out_count)
{
    sai_map_key_t  key;
    sai_status_t   rc = SAI_STATUS_FAILURE;

    if(p_out_count == NULL) {
        SAI_BRIDGE_LOG_TRACE("Error count is NULL for bridge port id 0x%"PRIx64""
                             " in bridge port vlan member count get", bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset (&key, 0, sizeof (key));

    key.type = SAI_MAP_TYPE_BRIDGE_PORT_TO_VLAN_MEMBER_LIST;
    key.id1  = bridge_port_id;

    rc = sai_map_get_val_count (&key, p_out_count);

    if(rc == SAI_STATUS_ITEM_NOT_FOUND) {
        *p_out_count = 0;
        return SAI_STATUS_SUCCESS;
    }

    return rc;
}

sai_status_t sai_bridge_port_to_stp_port_map_insert (sai_object_id_t bridge_port_id,
                                                     sai_object_id_t stp_port_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    sai_status_t   rc = SAI_STATUS_FAILURE;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_BRIDGE_PORT_TO_STP_PORT_LIST;
    key.id1  = bridge_port_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = stp_port_id;

    rc = sai_map_insert (&key, &value);

    if(rc == SAI_STATUS_SUCCESS) {
        sai_bridge_port_increment_ref_count(bridge_port_id);
    }
    return rc;
}

sai_status_t sai_bridge_port_to_stp_port_map_remove (sai_object_id_t bridge_port_id,
                                                     sai_object_id_t stp_port_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    uint_t         count = 0;
    sai_status_t   rc = SAI_STATUS_FAILURE;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_BRIDGE_PORT_TO_STP_PORT_LIST;
    key.id1  = bridge_port_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = stp_port_id;

    rc = sai_map_delete_elements (&key, &value, SAI_MAP_VAL_FILTER_VAL1);

    if (sai_map_get_val_count (&key, &count) == SAI_STATUS_SUCCESS) {
        if (count == 0) {
            sai_map_delete (&key);
        }
    }

    if(rc == SAI_STATUS_SUCCESS) {
        sai_bridge_port_decrement_ref_count(bridge_port_id);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_get_stp_port_at_index (sai_object_id_t  bridge_port_id,
                                                      uint_t           index,
                                                      sai_object_id_t *stp_port_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_status_t   rc = SAI_STATUS_FAILURE;
    sai_map_data_t data;

    if(stp_port_id == NULL) {
        SAI_BRIDGE_LOG_TRACE("stp port id is NULL for bridge port id "
                             " 0x%"PRIx64" in bridge port stp port list get",bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (sai_map_data_t));


    key.type = SAI_MAP_TYPE_BRIDGE_PORT_TO_STP_PORT_LIST;
    key.id1  = bridge_port_id;

    value.data  = &data;

    rc = sai_map_get_element_at_index(&key, index, &value);

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }
    *stp_port_id = data.val1;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_to_stp_port_count_get(sai_object_id_t  bridge_port_id,
                                                   uint_t          *p_out_count)
{
    sai_map_key_t  key;
    sai_status_t   rc = SAI_STATUS_FAILURE;

    memset (&key, 0, sizeof (key));

    if(p_out_count == NULL) {
        SAI_BRIDGE_LOG_TRACE("Error count is NULL for bridge port id 0x%"PRIx64""
                             " in bridge port stp port count get", bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.type = SAI_MAP_TYPE_BRIDGE_PORT_TO_STP_PORT_LIST;
    key.id1  = bridge_port_id;

    rc = sai_map_get_val_count (&key, p_out_count);

    if(rc == SAI_STATUS_ITEM_NOT_FOUND) {
        *p_out_count = 0;
        return SAI_STATUS_SUCCESS;
    }
    return rc;
}

sai_status_t sai_tunnel_to_bridge_port_map_insert (sai_object_id_t tunnel_id,
                                                   sai_object_id_t bridge_port_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_TUNNEL_TO_BRIDGE_PORT_LIST;
    key.id1  = tunnel_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = bridge_port_id;

    return sai_map_insert (&key, &value);
}

sai_status_t sai_tunnel_to_bridge_port_map_remove (sai_object_id_t tunnel_id,
                                                   sai_object_id_t bridge_port_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    uint_t         count = 0;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_TUNNEL_TO_BRIDGE_PORT_LIST;
    key.id1  = tunnel_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = bridge_port_id;

    sai_map_delete_elements (&key, &value, SAI_MAP_VAL_FILTER_VAL1);

    if (sai_map_get_val_count (&key, &count) == SAI_STATUS_SUCCESS) {
        if (count == 0) {
            sai_map_delete (&key);
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_tunnel_to_bridge_port_list_get (sai_object_id_t  tunnel_id,
                                                 uint_t          *count,
                                                 sai_object_id_t *bridge_port_list)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_status_t   rc = SAI_STATUS_FAILURE;
    uint_t         index = 0;
    sai_map_data_t data;
    uint_t         map_cnt = 0;

    if((count == NULL) || (bridge_port_list == NULL)) {
        SAI_BRIDGE_LOG_TRACE("Error count is %p bridge_port_list is %p for tunnel id "
                             " 0x%"PRIx64" in tunnel bridge port list get",
                             count, bridge_port_list, tunnel_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (sai_map_data_t));

    key.type = SAI_MAP_TYPE_TUNNEL_TO_BRIDGE_PORT_LIST;
    key.id1  = tunnel_id;

    rc = sai_map_get_val_count (&key, &map_cnt);

    if(rc == SAI_STATUS_ITEM_NOT_FOUND) {
        *count = 0;
        return SAI_STATUS_SUCCESS;
    }

    if(*count < map_cnt) {
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    value.data = &data;

    for (index = 0; index < map_cnt; index++) {
        rc = sai_map_get_element_at_index(&key, index, &value);

        if (rc != SAI_STATUS_SUCCESS) {
            return rc;
        }
        bridge_port_list [index] = data.val1;
    }

    *count = map_cnt;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_tunnel_to_bridge_port_count_get(sai_object_id_t  tunnel_id,
                                                 uint_t          *p_out_count)
{
    sai_map_key_t  key;
    sai_status_t   rc = SAI_STATUS_FAILURE;

    if(p_out_count == NULL) {
        SAI_BRIDGE_LOG_TRACE("Count is NULL for tunnel id 0x%"PRIx64""
                             " in tunnel bridge port count get", tunnel_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset (&key, 0, sizeof (key));

    key.type = SAI_MAP_TYPE_TUNNEL_TO_BRIDGE_PORT_LIST;
    key.id1  = tunnel_id;

    rc = sai_map_get_val_count (&key, p_out_count);

    if(rc == SAI_STATUS_ITEM_NOT_FOUND) {
        *p_out_count = 0;
        return SAI_STATUS_SUCCESS;
    }

    return rc;
}

sai_status_t sai_tunnel_to_bridge_port_get_at_index(sai_object_id_t tunnel_id,
                                                    uint_t index,
                                                    sai_object_id_t *bridge_port)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_status_t   rc = SAI_STATUS_FAILURE;
    sai_map_data_t data;

    if(bridge_port == NULL) {
        SAI_BRIDGE_LOG_TRACE("Bridge port is NULL for tunnel 0x%"PRIx64" in "
                             "tunnel to bridge port get at index", tunnel_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (sai_map_data_t));

    key.type = SAI_MAP_TYPE_TUNNEL_TO_BRIDGE_PORT_LIST;
    key.id1 = tunnel_id;
    value.data = &data;

    rc = sai_map_get_element_at_index(&key, index, &value);

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }

    *bridge_port = data.val1;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_get_attr_value_from_bridge_info (const dn_sai_bridge_info_t *bridge_info,
                                                         uint_t attr_count,
                                                         sai_attribute_t *attr_list)
{
    uint_t                     attr_idx = 0;
    sai_status_t               sai_rc = SAI_STATUS_FAILURE;
    dn_sai_bridge_flood_type_t flood_type = SAI_BRIDGE_FLOOD_TYPE_MAX;
    uint_t                     bridge_port_count = 0;


    if((bridge_info == NULL) || (attr_list == NULL)) {
        SAI_BRIDGE_LOG_TRACE("Bridge info is %p attr_list is %p in get attr value from bridge info",
                             bridge_info, attr_list);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {

        switch(attr_list[attr_idx].id) {
            case SAI_BRIDGE_ATTR_TYPE:
                attr_list[attr_idx].value.s32 = bridge_info->bridge_type;
                break;

            case SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES:
                attr_list[attr_idx].value.u32 = bridge_info->max_learned_address;
                break;

            case SAI_BRIDGE_ATTR_LEARN_DISABLE:
                attr_list[attr_idx].value.booldata = bridge_info->learn_disable;
                break;

            case SAI_BRIDGE_ATTR_PORT_LIST:
                sai_rc = sai_bridge_map_get_port_count(bridge_info->bridge_id, &bridge_port_count);
                if(sai_rc != SAI_STATUS_SUCCESS) {
                    return sai_rc;
                }
                if(bridge_port_count > attr_list[attr_idx].value.objlist.count) {
                    SAI_BRIDGE_LOG_ERR("Expect %d objects in bridge 0x%"PRIx64" port list get",
                                       bridge_port_count ,bridge_info->bridge_id);
                    attr_list[attr_idx].value.objlist.count = bridge_port_count;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }
                sai_rc = sai_bridge_map_port_list_get(bridge_info->bridge_id,
                                                      &attr_list[attr_idx].value.objlist.count,
                                                      attr_list[attr_idx].value.objlist.list);
                if(sai_rc != SAI_STATUS_SUCCESS) {
                    SAI_BRIDGE_LOG_ERR("Error %d in getting bridge port list for bridge id "
                                       "0x%"PRIx64"",sai_rc, bridge_info->bridge_id);
                    return sai_rc;
                }
                break;

            case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
            case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
            case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
                sai_bridge_get_flood_type_from_attr(attr_list[attr_idx].id, &flood_type);
                attr_list[attr_idx].value.s32 = bridge_info->flood_control[flood_type];
                break;

            case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
            case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
            case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP:
                sai_bridge_get_flood_type_from_attr(attr_list[attr_idx].id, &flood_type);
                attr_list[attr_idx].value.oid = bridge_info->l2mc_flood_group[flood_type];
                break;

            default:
                return (SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + attr_idx);
        }
    }

    return SAI_STATUS_SUCCESS;

}

sai_status_t sai_bridge_update_attr_value_in_cache(dn_sai_bridge_info_t *bridge_info,
                                                   const sai_attribute_t *attr)
{
    dn_sai_bridge_flood_type_t flood_type = SAI_BRIDGE_FLOOD_TYPE_MAX;
    sai_object_id_t            l2mc_object_id = SAI_NULL_OBJECT_ID;
    sai_status_t               sai_rc = SAI_STATUS_FAILURE;

    if((bridge_info == NULL) || (attr == NULL)) {
        SAI_BRIDGE_LOG_TRACE("Bridge info is %p attr is %p in update attr value in bridge info",
                             bridge_info, attr);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch(attr->id) {

        case SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES:
            bridge_info->max_learned_address = attr->value.u32;
            break;

        case SAI_BRIDGE_ATTR_LEARN_DISABLE:
            bridge_info->learn_disable = attr->value.booldata;
            break;

        case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
        case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
        case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
            sai_bridge_get_flood_type_from_attr(attr->id, &flood_type);
            bridge_info->flood_control[flood_type] = attr->value.s32;
            break;

        case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
        case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
        case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP:
            sai_bridge_get_flood_type_from_attr(attr->id, &flood_type);
            if(attr->value.oid == SAI_NULL_OBJECT_ID) {
                l2mc_object_id = bridge_info->l2mc_flood_group[flood_type];
                sai_rc = sai_l2mc_remove_bridge_id(l2mc_object_id, bridge_info->bridge_id);
            } else {
                l2mc_object_id = attr->value.oid;
                sai_rc = sai_l2mc_add_bridge_id(l2mc_object_id, bridge_info->bridge_id);
            }
            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_BRIDGE_LOG_ERR("Error %d in updating bridge 0x%"PRIx64" in l2mc obj "
                                   "0x%"PRIx64"", sai_rc, bridge_info->bridge_id, l2mc_object_id);
                return sai_rc;
            }
            bridge_info->l2mc_flood_group[flood_type] = attr->value.oid;
            break;

        default:
            return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    return SAI_STATUS_SUCCESS;
}

bool sai_bridge_is_duplicate_attr_val(const dn_sai_bridge_info_t *bridge_info,
                                      const sai_attribute_t *attr)
{
    dn_sai_bridge_flood_type_t flood_type = SAI_BRIDGE_FLOOD_TYPE_MAX;

    if((bridge_info == NULL) || (attr == NULL)) {
        SAI_BRIDGE_LOG_TRACE("Bridge info is %p attr is %p in check for duplicate attr",
                             bridge_info, attr);
        return false;
    }

    switch(attr->id) {

        case SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES:
            return (bridge_info->max_learned_address == attr->value.u32);

        case SAI_BRIDGE_ATTR_LEARN_DISABLE:
            return (bridge_info->learn_disable == attr->value.booldata);

        case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
        case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
        case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
            sai_bridge_get_flood_type_from_attr(attr->id, &flood_type);
            return (bridge_info->flood_control[flood_type] == attr->value.s32);

        case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
        case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
        case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP:
            sai_bridge_get_flood_type_from_attr(attr->id, &flood_type);
            return (bridge_info->l2mc_flood_group[flood_type] == attr->value.oid);

        default:
            return false;
    }

    return false;
}
sai_status_t sai_bridge_increment_ref_count(sai_object_id_t bridge_id)
{
    sai_status_t          sai_rc = SAI_STATUS_FAILURE;
    dn_sai_bridge_info_t *p_bridge_info = NULL;

    sai_rc = sai_bridge_cache_read (bridge_id, &p_bridge_info);
    if ((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_info == NULL)) {
        SAI_BRIDGE_LOG_ERR("Invalid bridge object id 0x%"PRIx64" in set attribute", bridge_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    p_bridge_info->ref_count++;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_decrement_ref_count(sai_object_id_t bridge_id)
{
    sai_status_t          sai_rc = SAI_STATUS_FAILURE;
    dn_sai_bridge_info_t *p_bridge_info = NULL;

    sai_rc = sai_bridge_cache_read (bridge_id, &p_bridge_info);
    if ((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_info == NULL)) {
        SAI_BRIDGE_LOG_ERR("Invalid bridge object id 0x%"PRIx64" in set attribute", bridge_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    p_bridge_info->ref_count--;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_increment_ref_count(sai_object_id_t bridge_port_id)
{
    sai_status_t               sai_rc = SAI_STATUS_FAILURE;
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;

    sai_rc = sai_bridge_port_cache_read (bridge_port_id, &p_bridge_port_info);
    if ((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_port_info == NULL)) {
        SAI_BRIDGE_LOG_ERR("Invalid bridge_port object id 0x%"PRIx64" in set attribute",
                            bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    p_bridge_port_info->ref_count++;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_decrement_ref_count(sai_object_id_t bridge_port_id)
{
    sai_status_t               sai_rc = SAI_STATUS_FAILURE;
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;

    sai_rc = sai_bridge_port_cache_read (bridge_port_id, &p_bridge_port_info);
    if ((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_port_info == NULL)) {
        SAI_BRIDGE_LOG_ERR("Invalid bridge_port object id 0x%"PRIx64" in set attribute",
                           bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    p_bridge_port_info->ref_count--;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_increment_fdb_count(sai_object_id_t bridge_port_id)
{
    sai_status_t          sai_rc = SAI_STATUS_FAILURE;
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;

    sai_rc = sai_bridge_port_cache_read (bridge_port_id, &p_bridge_port_info);
    if ((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_port_info == NULL)) {
        SAI_BRIDGE_LOG_ERR("Invalid bridge_ ort object id 0x%"PRIx64" in set attribute", bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    p_bridge_port_info->fdb_count++;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_decrement_fdb_count(sai_object_id_t bridge_port_id)
{
    sai_status_t          sai_rc = SAI_STATUS_FAILURE;
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;

    if(bridge_port_id == SAI_NULL_OBJECT_ID) {
        return SAI_STATUS_SUCCESS;
    }

    sai_rc = sai_bridge_port_cache_read (bridge_port_id, &p_bridge_port_info);
    if ((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_port_info == NULL)) {
        SAI_BRIDGE_LOG_ERR("Invalid bridge port object id 0x%"PRIx64" in set attribute", bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    p_bridge_port_info->fdb_count--;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_get_attr_value_from_bridge_port_info (const dn_sai_bridge_port_info_t
                                                                   *bridge_port_info,
                                                                   uint_t attr_count,
                                                                   sai_attribute_t *attr_list)
{
    uint_t       attr_idx = 0;

    if((bridge_port_info == NULL) || (attr_list == NULL)) {
        SAI_BRIDGE_LOG_TRACE("Bridge port info is %p attr_list is %p in get attr value from "
                             "bridge port info", bridge_port_info, attr_list);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {

        switch(attr_list[attr_idx].id) {

            case SAI_BRIDGE_PORT_ATTR_TYPE:
                attr_list[attr_idx].value.s32 = bridge_port_info->bridge_port_type;
                break;

            case SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES:
                attr_list[attr_idx].value.u32 = bridge_port_info->max_learned_address;
                break;

            case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE:
                attr_list[attr_idx].value.s32 = bridge_port_info->fdb_learn_mode;
                break;

            case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION:
                attr_list[attr_idx].value.s32 = bridge_port_info->learn_limit_violation_action;
                break;

            case SAI_BRIDGE_PORT_ATTR_ADMIN_STATE:
                attr_list[attr_idx].value.booldata = bridge_port_info->admin_state;
                break;

            case SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING:
                attr_list[attr_idx].value.booldata = bridge_port_info->ingress_filtering;
                break;

            case SAI_BRIDGE_PORT_ATTR_EGRESS_FILTERING:
                attr_list[attr_idx].value.booldata = bridge_port_info->egress_filtering;
                break;

            case SAI_BRIDGE_PORT_ATTR_TAGGING_MODE:
                attr_list[attr_idx].value.s32 = bridge_port_info->egr_tagging_mode;
                break;

            case SAI_BRIDGE_PORT_ATTR_BRIDGE_ID:
                attr_list[attr_idx].value.oid = bridge_port_info->bridge_id;
                break;

            case SAI_BRIDGE_PORT_ATTR_PORT_ID:
                attr_list[attr_idx].value.oid = sai_bridge_port_info_get_port_id(bridge_port_info);
                break;

            case SAI_BRIDGE_PORT_ATTR_VLAN_ID:
                attr_list[attr_idx].value.u16 = sai_bridge_port_info_get_vlan_id(bridge_port_info);
                break;

            case SAI_BRIDGE_PORT_ATTR_RIF_ID:
                attr_list[attr_idx].value.oid = sai_bridge_port_info_get_rif_id(bridge_port_info);
                break;

            case SAI_BRIDGE_PORT_ATTR_TUNNEL_ID:
                attr_list[attr_idx].value.oid = sai_bridge_port_info_get_tunnel_id(bridge_port_info);
                break;

            default:
                return (SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + attr_idx);
        }
    }

    return SAI_STATUS_SUCCESS;

}

sai_status_t sai_bridge_port_update_attr_value_in_cache (dn_sai_bridge_port_info_t *bridge_port_info,
                                                         const sai_attribute_t *attr)
{
    if((bridge_port_info == NULL) || (attr == NULL)) {
        SAI_BRIDGE_LOG_TRACE("Bridge port info is %p attr is %p in set attr value in "
                             "bridge port info", bridge_port_info, attr);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch(attr->id) {

            case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE:
                bridge_port_info->fdb_learn_mode = attr->value.s32;
                break;

            case SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES:
                bridge_port_info->max_learned_address = attr->value.u32;
                break;

            case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION:
                bridge_port_info->learn_limit_violation_action = attr->value.s32;
                break;

            case SAI_BRIDGE_PORT_ATTR_ADMIN_STATE:
                bridge_port_info->admin_state = attr->value.booldata;
                break;

            case SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING:
                bridge_port_info->ingress_filtering = attr->value.booldata;
                break;

            case SAI_BRIDGE_PORT_ATTR_EGRESS_FILTERING:
                bridge_port_info->egress_filtering = attr->value.booldata;
                break;

            case SAI_BRIDGE_PORT_ATTR_TAGGING_MODE:
                bridge_port_info->egr_tagging_mode = attr->value.s32;
                break;

            case SAI_BRIDGE_PORT_ATTR_BRIDGE_ID:
                bridge_port_info->bridge_id = attr->value.oid;
                break;

            default:
                return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    return SAI_STATUS_SUCCESS;
}

bool sai_bridge_port_is_duplicate_attr_val(const dn_sai_bridge_port_info_t *bridge_port_info,
                                           const sai_attribute_t *attr)
{
    if((bridge_port_info == NULL) || (attr == NULL)) {
        SAI_BRIDGE_LOG_TRACE("Bridge port info is %p attr is %p in check duplicate attr value",
                              bridge_port_info, attr);
        return false;
    }

    switch(attr->id) {

            case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE:
                return (bridge_port_info->fdb_learn_mode == attr->value.s32);

            case SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES:
                return (bridge_port_info->max_learned_address == attr->value.u32);

            case SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION:
                return (bridge_port_info->learn_limit_violation_action == attr->value.s32);

            case SAI_BRIDGE_PORT_ATTR_ADMIN_STATE:
                return (bridge_port_info->admin_state == attr->value.booldata);

            case SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING:
                return (bridge_port_info->ingress_filtering == attr->value.booldata);

            case SAI_BRIDGE_PORT_ATTR_EGRESS_FILTERING:
                return (bridge_port_info->egress_filtering == attr->value.booldata);

            case SAI_BRIDGE_PORT_ATTR_TAGGING_MODE:
                return (bridge_port_info->egr_tagging_mode == attr->value.s32);
            case SAI_BRIDGE_PORT_ATTR_BRIDGE_ID:
                return (bridge_port_info->bridge_id == attr->value.oid);

            default:
                return false;
    }

    return false;
}

sai_status_t sai_bridge_port_get_bridge_id(sai_object_id_t bridge_port_id,
                                           sai_object_id_t *bridge_id)
{
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;
    sai_status_t               sai_rc = SAI_STATUS_FAILURE;

    if(bridge_id == NULL) {
        SAI_BRIDGE_LOG_TRACE("bridge_id is NULL for bridge port 0x%"PRIx64" in get port",
                             bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_rc = sai_bridge_port_cache_read(bridge_port_id, &p_bridge_port_info);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BRIDGE_LOG_ERR("Error %d in reading bridge port cache for bridge port"
                           " 0x%"PRIx64"", sai_rc, bridge_port_id);
        return sai_rc;
    }
    *bridge_id = p_bridge_port_info->bridge_id;
    return SAI_STATUS_SUCCESS;
}
sai_status_t sai_bridge_port_get_port_id(sai_object_id_t bridge_port_id,
                                         sai_object_id_t *sai_port_id)
{
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;
    sai_status_t               sai_rc = SAI_STATUS_FAILURE;

    if(sai_port_id == NULL) {
        SAI_BRIDGE_LOG_TRACE("sai_port_id is NULL for bridge port 0x%"PRIx64" in get port",
                             bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_rc = sai_bridge_port_cache_read(bridge_port_id, &p_bridge_port_info);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BRIDGE_LOG_ERR("Error %d in reading bridge port cache for bridge port"
                           " 0x%"PRIx64"", sai_rc, bridge_port_id);
        return sai_rc;
    }
    *sai_port_id = sai_bridge_port_info_get_port_id(p_bridge_port_info);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_get_vlan_id(sai_object_id_t  bridge_port_id,
                                         uint16_t        *vlan_id)
{
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;
    sai_status_t               sai_rc = SAI_STATUS_FAILURE;

    if(vlan_id == NULL) {
        SAI_BRIDGE_LOG_TRACE("vlan_id is NULL for bridge port 0x%"PRIx64" in get port",
                             bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_rc = sai_bridge_port_cache_read(bridge_port_id, &p_bridge_port_info);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BRIDGE_LOG_ERR("Error %d in reading bridge port cache for bridge port"
                           " 0x%"PRIx64"", sai_rc, bridge_port_id);
        return sai_rc;
    }
    *vlan_id = sai_bridge_port_info_get_vlan_id(p_bridge_port_info);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_get_rif_id(sai_object_id_t  bridge_port_id,
                                        sai_object_id_t *rif_id)
{
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;
    sai_status_t               sai_rc = SAI_STATUS_FAILURE;

    if(rif_id == NULL) {
        SAI_BRIDGE_LOG_TRACE("rif_id is NULL for bridge port 0x%"PRIx64" in get port",
                             bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_rc = sai_bridge_port_cache_read(bridge_port_id, &p_bridge_port_info);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BRIDGE_LOG_ERR("Error %d in reading bridge port cache for bridge port"
                           " 0x%"PRIx64"", sai_rc, bridge_port_id);
        return sai_rc;
    }
    *rif_id = sai_bridge_port_info_get_rif_id(p_bridge_port_info);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_get_tunnel_id(sai_object_id_t  bridge_port_id,
                                           sai_object_id_t *tunnel_id)
{
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;
    sai_status_t               sai_rc = SAI_STATUS_FAILURE;

    if(tunnel_id == NULL) {
        SAI_BRIDGE_LOG_TRACE("tunnel_id is NULL for bridge port 0x%"PRIx64" in get port",
                             bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_rc = sai_bridge_port_cache_read(bridge_port_id, &p_bridge_port_info);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BRIDGE_LOG_ERR("Error %d in reading bridge port cache for bridge port"
                           " 0x%"PRIx64"", sai_rc, bridge_port_id);
        return sai_rc;
    }
    *tunnel_id = sai_bridge_port_info_get_tunnel_id(p_bridge_port_info);
    return SAI_STATUS_SUCCESS;
}

void *sai_bridge_port_info_get_bridge_hw_info(dn_sai_bridge_port_info_t  *bridge_port_info)
{
    dn_sai_bridge_info_t *p_bridge_info = NULL;
    sai_status_t         sai_rc = SAI_STATUS_FAILURE;

    if(bridge_port_info == NULL) {
        SAI_BRIDGE_LOG_TRACE("bridge_port_info is NULL in get hardware info");
        return NULL;
    }

    sai_rc = sai_bridge_cache_read(bridge_port_info->bridge_id, &p_bridge_info);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BRIDGE_LOG_ERR("Error %d in reading bridge cache for bridge"
                           " 0x%"PRIx64"", sai_rc, bridge_port_info->bridge_id);
       return NULL;
    }
    return p_bridge_info->hw_info;
}

bool sai_is_bridge_port_type_port(sai_object_id_t bridge_port_id)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;;
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;

    sai_rc = sai_bridge_port_cache_read(bridge_port_id, &p_bridge_port_info);

    if((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_port_info == NULL)) {
        SAI_BRIDGE_LOG_ERR("Error in reading cache for bridge port id 0x%"PRIx64"",
                           bridge_port_id);
        return false;
    }
    return (p_bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_PORT);
}

bool sai_bridge_is_bridge_connected_to_tunnel(sai_object_id_t bridge_id,
                                              sai_object_id_t tunnel_id)
{
    bool is_connected = false;
    uint_t bridge_port_idx = 0;
    uint_t bridge_port_count = 0;
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;

    sai_rc = sai_tunnel_to_bridge_port_count_get(tunnel_id, &bridge_port_count);

    if(sai_rc != SAI_STATUS_SUCCESS)
    {
        SAI_BRIDGE_LOG_ERR("Failed to get bridge port count in tunnel 0x%"
                             PRIx64"object",tunnel_id);
        return false;
    }

    if(bridge_port_count == 0) {
        return false;
    }

    for(bridge_port_idx = 0; bridge_port_idx < bridge_port_count; bridge_port_idx++)
    {
        sai_rc = sai_tunnel_to_bridge_port_get_at_index(tunnel_id, bridge_port_idx,
                                                        &bridge_port_id);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BRIDGE_LOG_ERR("Failed to get bridge port at index %u in tunnel "
                               "to bridge port list for tunnel 0x%"PRIx64"",
                               bridge_port_idx, tunnel_id);
            continue;
        }

        sai_rc = sai_bridge_port_cache_read(bridge_port_id,
                                            &p_bridge_port_info);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BRIDGE_LOG_ERR("Error %d in reading bridge port cache for bridge port"
                               " 0x%"PRIx64"", sai_rc, bridge_port_id);
            continue;
        }

        if(p_bridge_port_info->bridge_id == bridge_id) {
            is_connected = true;
            break;
        }
    }

    return is_connected;
}

sai_status_t sai_bridge_port_get_type(sai_object_id_t bridge_port_id,
                                      sai_bridge_port_type_t *bridge_port_type)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;;
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;

    if(bridge_port_type == NULL) {
        SAI_BRIDGE_LOG_TRACE("Error bridge port type is null for bridge port 0x%"PRIx64""
                             "in bridge port type get", bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_rc = sai_bridge_port_cache_read(bridge_port_id, &p_bridge_port_info);

    if((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_port_info == NULL)) {
        SAI_BRIDGE_LOG_ERR("Error in reading cache for bridge port id 0x%"PRIx64"",
                           bridge_port_id);
        return sai_rc;
    }
    *bridge_port_type = p_bridge_port_info->bridge_port_type;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_to_l2mc_member_map_insert (sai_object_id_t bridge_port_id,
                                                        sai_object_id_t l2mc_member_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    sai_status_t   rc = SAI_STATUS_FAILURE;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_BRIDGE_PORT_TO_L2MC_MEMBER_LIST;
    key.id1  = bridge_port_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = l2mc_member_id;

    rc = sai_map_insert (&key, &value);
    if(rc == SAI_STATUS_SUCCESS) {
        sai_bridge_port_increment_ref_count(bridge_port_id);
    }
    return rc;
}

sai_status_t sai_bridge_port_to_l2mc_member_map_remove (sai_object_id_t bridge_port_id,
                                                        sai_object_id_t l2mc_member_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    uint_t         count = 0;
    sai_status_t   rc = SAI_STATUS_FAILURE;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_BRIDGE_PORT_TO_L2MC_MEMBER_LIST;
    key.id1  = bridge_port_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = l2mc_member_id;

    rc = sai_map_delete_elements (&key, &value, SAI_MAP_VAL_FILTER_VAL1);

    if (sai_map_get_val_count (&key, &count) == SAI_STATUS_SUCCESS) {
        if (count == 0) {
            sai_map_delete (&key);
        }
    }

    if(rc == SAI_STATUS_SUCCESS) {
        sai_bridge_port_decrement_ref_count(bridge_port_id);
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_get_l2mc_member_at_index (sai_object_id_t  bridge_port_id,
                                                       uint_t           index,
                                                       sai_object_id_t *l2mc_member_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_status_t   rc;
    sai_map_data_t data;

    if(l2mc_member_id == NULL) {
        SAI_BRIDGE_LOG_TRACE("Error l2mc_member_id is NULL for bridge port id "
                             " 0x%"PRIx64" in bridge port l2mc member get at index %d",
                             bridge_port_id, index);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (sai_map_data_t));

    key.type = SAI_MAP_TYPE_BRIDGE_PORT_TO_L2MC_MEMBER_LIST;
    key.id1  = bridge_port_id;
    value.data  = &data;

    rc = sai_map_get_element_at_index(&key, index, &value);

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }
    *l2mc_member_id = data.val1;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_port_to_l2mc_member_count_get(sai_object_id_t  bridge_port_id,
                                                      uint_t          *p_out_count)
{
    sai_map_key_t  key;
    sai_status_t   rc;

    if(p_out_count == NULL) {
        SAI_BRIDGE_LOG_TRACE("Error count is NULL for bridge port id 0x%"PRIx64""
                             " in bridge port l2mc member count get", bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset (&key, 0, sizeof (key));

    key.type = SAI_MAP_TYPE_BRIDGE_PORT_TO_L2MC_MEMBER_LIST;
    key.id1  = bridge_port_id;

    rc = sai_map_get_val_count (&key, p_out_count);

    if(rc == SAI_STATUS_ITEM_NOT_FOUND) {
        *p_out_count = 0;
        return SAI_STATUS_SUCCESS;
    }

    return rc;
}

bool sai_is_bridge_port_type_sub_port(sai_object_id_t bridge_port_id)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;;
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;

    sai_rc = sai_bridge_port_cache_read(bridge_port_id, &p_bridge_port_info);

    if((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_port_info == NULL)) {
        SAI_BRIDGE_LOG_ERR("Error in reading cache for bridge port id 0x%"PRIx64"",
                           bridge_port_id);
        return false;
    }
    return (p_bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_SUB_PORT);
}

bool sai_is_bridge_port_type_tunnel(sai_object_id_t bridge_port_id)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;;
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;

    sai_rc = sai_bridge_port_cache_read(bridge_port_id, &p_bridge_port_info);

    if((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_port_info == NULL)) {
        SAI_BRIDGE_LOG_ERR("Error in reading cache for bridge port id 0x%"PRIx64"",
                           bridge_port_id);
        return false;
    }
    return (p_bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_TUNNEL);
}

bool sai_is_bridge_port_obj_lag(sai_object_id_t bridge_port_id)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;;
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;
    sai_object_id_t port_id = SAI_NULL_OBJECT_ID;
    sai_rc = sai_bridge_port_cache_read(bridge_port_id, &p_bridge_port_info);

    if((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_port_info == NULL)) {
        SAI_BRIDGE_LOG_ERR("Error in reading cache for bridge port id 0x%"PRIx64"",
                           bridge_port_id);
        return false;
    }
    if (p_bridge_port_info->bridge_port_type == SAI_BRIDGE_PORT_TYPE_PORT) {
        port_id = sai_bridge_port_info_get_port_id(p_bridge_port_info);
        return sai_is_obj_id_lag(port_id);
    }
    return false;
}

sai_status_t sai_bridge_port_get_admin_state(sai_object_id_t bridge_port_id,
                                             bool *admin_state)
{
    sai_status_t               sai_rc = SAI_STATUS_FAILURE;;
    dn_sai_bridge_port_info_t *p_bridge_port_info = NULL;

    if(admin_state == NULL) {
        SAI_BRIDGE_LOG_TRACE("Error admin state is null for bridge port 0x%"PRIx64""
                             "in bridge port type get", bridge_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_rc = sai_bridge_port_cache_read(bridge_port_id, &p_bridge_port_info);

    if((sai_rc != SAI_STATUS_SUCCESS) || (p_bridge_port_info == NULL)) {
        SAI_BRIDGE_LOG_ERR("Error in reading cache for bridge port id 0x%"PRIx64"",
                           bridge_port_id);
        return sai_rc;
    }
    *admin_state = p_bridge_port_info->admin_state;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_bridge_cleanup_l2mc_group_references(const dn_sai_bridge_info_t *bridge_info)
{
    uint_t                     flood_idx = 0;

    if(bridge_info == NULL) {
        SAI_BRIDGE_LOG_TRACE("bridge_info is NULL in cleanup l2mc references");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for(flood_idx = 0; flood_idx < SAI_BRIDGE_FLOOD_TYPE_MAX; flood_idx++) {
        if(bridge_info->l2mc_flood_group[flood_idx] !=  SAI_NULL_OBJECT_ID) {
            sai_l2mc_remove_bridge_id(bridge_info->l2mc_flood_group[flood_idx],
                                      bridge_info->bridge_id);
        }
    }
    return SAI_STATUS_SUCCESS;
}
