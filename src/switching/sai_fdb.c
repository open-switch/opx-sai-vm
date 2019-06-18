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
* @file sai_fdb.c
*
* @brief This file contains implementation of SAI FDB APIs.
*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "saifdb.h"
#include "saivlan.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_modules_init.h"
#include "sai_npu_fdb.h"
#include "sai_common_infra.h"
#include "sai_fdb_api.h"
#include "sai_fdb_common.h"
#include "sai_fdb_main.h"
#include "sai_vlan_api.h"
#include "std_mac_utils.h"
#include "sai_port_utils.h"
#include "sai_switch_utils.h"
#include "std_assert.h"
#include "sai_gen_utils.h"
#include "sai_lag_api.h"
#include "std_thread_tools.h"
#include "sai_stp_api.h"
#include "sai_lag_api.h"
#include "sai_bridge_api.h"


static std_thread_create_param_t _thread;
static int sai_fdb_fd[SAI_FDB_MAX_FD];
static sai_fdb_event_notification_fn sai_l2_fdb_notification_fn = NULL;
static sai_fdb_event_notification_data_t valid_notification_data[SAI_FDB_MAX_MACS_PER_CALLBACK];

static void * _sai_fdb_internal_notif(void * param) {
    int len = 0;
    while(1) {
        bool wake;
        len = read(sai_fdb_fd[SAI_FDB_READ_FD], &wake,sizeof(bool));
        STD_ASSERT (len!=0);
        if(wake) {
            sai_fdb_send_internal_notifications ();
        }
    }
    return NULL;
}


static void sai_fdb_wake_notification_thread(void)
{
    int rc = 0;
    bool wake = sai_fdb_is_notifications_pending();

    if ((wake) && (rc = write (sai_fdb_fd[SAI_FDB_WRITE_FD], &wake ,sizeof(bool)))!=sizeof(bool)) {
        SAI_FDB_LOG_ERR ("Writing to event queue failed");
    }
}

sai_status_t sai_fdb_init(void)
{
    sai_status_t ret_val;

    ret_val = sai_fdb_npu_api_get()->fdb_init();
    if(ret_val != SAI_STATUS_SUCCESS) {
        SAI_FDB_LOG_ERR("SAI NPU FDB init failled");
        return ret_val;
    }
    ret_val = sai_init_fdb_tree();
    if(ret_val != SAI_STATUS_SUCCESS) {
        SAI_FDB_LOG_ERR("SAI FDB Cache init failled");
        return ret_val;
    }

    if (pipe(sai_fdb_fd) != 0) {
        SAI_FDB_LOG_ERR("Pipe initilization failed");
        return SAI_STATUS_FAILURE;
    }

    std_thread_init_struct(&_thread);
    _thread.name = "sai_fdb_internal_notif";
    _thread.thread_function = _sai_fdb_internal_notif;

    if (std_thread_create(&_thread)!=STD_ERR_OK) {
        return SAI_STATUS_FAILURE;
    }


    return SAI_STATUS_SUCCESS;
}

// TODO use other means to deal with "flush FDB", but this would require a major rewrite of the FDB component
// For now, the idea is to flush all MAC addresses of a bridge - extra MAC address flooding/low performance is not an issue for the VM
static void force_flush_fdb_all(sai_object_id_t bridge_port_id, sai_object_id_t bv_id,
        sai_fdb_flush_entry_type_t flush_entry_type)
{
    if (SAI_FDB_FLUSH_ENTRY_TYPE_DYNAMIC == flush_entry_type) {
        sai_fdb_entry_t fdb_entry;

        fdb_entry.switch_id = sai_switch_id_get();
        memset(fdb_entry.mac_address, 0, sizeof(fdb_entry.mac_address));
        if (SAI_NULL_OBJECT_ID != bv_id) {
            fdb_entry.bv_id = bv_id;
            sai_fdb_npu_api_get()->flush_fdb_entry(&fdb_entry, false);
        }
        else {
            // Flush MAC entries on all VLANs associated to a port- represented by bridges.
            static const sai_vlan_id_t max_vlan_id = 4096;
            sai_vlan_id_t vlan_id;

            for (vlan_id = 1; vlan_id < max_vlan_id; ++vlan_id) {
                if ((SAI_NULL_OBJECT_ID == bridge_port_id) ||
                     sai_is_bridge_port_vlan_member(vlan_id, bridge_port_id))
                {
                    fdb_entry.bv_id = sai_vlan_id_to_vlan_obj_id(vlan_id);
                    sai_fdb_npu_api_get()->flush_fdb_entry(&fdb_entry, false);
                }
            }
        }
    }
}

static void sai_delete_all_fdb_entry_nodes (bool delete_all, sai_fdb_flush_entry_type_t flush_entry_type)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_key_t fdb_key;
    sai_fdb_entry_type_t entry_type = sai_get_sai_fdb_entry_type_for_flush(flush_entry_type);
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    char mac_str[SAI_MAC_STR_LEN] = {0};
    sai_fdb_entry_t fdb_entry;
    bool remove_fdb_from_cache = true;

    memset(&fdb_key, 0, sizeof(fdb_key));

    fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);

    while(fdb_entry_node != NULL) {
        memcpy(&fdb_key,&(fdb_entry_node->fdb_key),
               sizeof(sai_fdb_entry_key_t));
        if ((delete_all == true) ||
            (entry_type == fdb_entry_node->entry_type)) {
            fdb_entry.bv_id = fdb_entry_node->fdb_key.bv_id;
            memcpy(fdb_entry.mac_address, fdb_entry_node->fdb_key.mac_address,
                    sizeof(sai_mac_t));
            sai_rc = sai_fdb_npu_api_get()->flush_fdb_entry(&fdb_entry, false);

            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_FDB_LOG_TRACE ("Delete failed for for MAC:%s vlan:0x%"PRIx64""
                                   " Error code %d", std_mac_to_string((const sai_mac_t *)
                                   &(fdb_entry.mac_address), mac_str,
                                   sizeof(mac_str)), fdb_entry.bv_id, sai_rc);
                remove_fdb_from_cache = false;
            } else {
                remove_fdb_from_cache = true;
            }
            if(remove_fdb_from_cache) {
                sai_remove_fdb_entry_node(fdb_entry_node);
            }
        }
        fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);
    }
    force_flush_fdb_all(SAI_NULL_OBJECT_ID, SAI_NULL_OBJECT_ID, flush_entry_type);
}

static void sai_delete_fdb_entry_nodes_per_port (sai_object_id_t bridge_port_id, bool delete_all,
                                                 sai_fdb_flush_entry_type_t flush_entry_type)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_key_t fdb_key;
    sai_fdb_entry_type_t entry_type = sai_get_sai_fdb_entry_type_for_flush(flush_entry_type);
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    char mac_str[SAI_MAC_STR_LEN] = {0};
    sai_fdb_entry_t fdb_entry;
    bool remove_fdb_from_cache = true;

    memset(&fdb_key, 0, sizeof(fdb_key));
    fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);
    while(fdb_entry_node != NULL) {
        memcpy(&fdb_key,&(fdb_entry_node->fdb_key),
               sizeof(sai_fdb_entry_key_t));
        if(fdb_entry_node->bridge_port_id == bridge_port_id){
            if ((delete_all == true) ||
                (entry_type == fdb_entry_node->entry_type)) {
                fdb_entry.bv_id = fdb_entry_node->fdb_key.bv_id;
                memcpy(fdb_entry.mac_address, fdb_entry_node->fdb_key.mac_address,
                        sizeof(sai_mac_t));
                sai_rc = sai_fdb_npu_api_get()->flush_fdb_entry(&fdb_entry, true);

                if(sai_rc != SAI_STATUS_SUCCESS) {
                    SAI_FDB_LOG_TRACE ("Delete failed for for MAC:%s vlan:0x%"PRIx64""
                                       " Error code %d", std_mac_to_string((const sai_mac_t *)
                                       &(fdb_entry.mac_address), mac_str,
                                       sizeof(mac_str)), fdb_entry.bv_id, sai_rc);
                    remove_fdb_from_cache = false;
                } else {
                    remove_fdb_from_cache = true;
                }
                if(remove_fdb_from_cache) {
                    sai_remove_fdb_entry_node(fdb_entry_node);
                }

            }
        }
        fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);
    }
    force_flush_fdb_all(bridge_port_id, SAI_NULL_OBJECT_ID, flush_entry_type);
}

static void sai_delete_fdb_entry_nodes_per_vlan (sai_object_id_t bv_id, bool delete_all,
                                                 sai_fdb_flush_entry_type_t flush_entry_type)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_key_t fdb_key;
    sai_fdb_entry_type_t entry_type = sai_get_sai_fdb_entry_type_for_flush(flush_entry_type);
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    char mac_str[SAI_MAC_STR_LEN] = {0};
    sai_fdb_entry_t fdb_entry;
    bool remove_fdb_from_cache = true;


    memset(&fdb_key, 0, sizeof(fdb_key));
    fdb_key.bv_id = bv_id;

    fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);

    while(fdb_entry_node != NULL) {
        memcpy(&fdb_key,&(fdb_entry_node->fdb_key),
              sizeof(sai_fdb_entry_key_t));
        if(fdb_key.bv_id != bv_id){
            break;
        }
        if(delete_all == true || entry_type == fdb_entry_node->entry_type) {
            fdb_entry.bv_id = fdb_entry_node->fdb_key.bv_id;
            memcpy(fdb_entry.mac_address, fdb_entry_node->fdb_key.mac_address,
                    sizeof(sai_mac_t));
            sai_rc = sai_fdb_npu_api_get()->flush_fdb_entry(&fdb_entry, false);

            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_FDB_LOG_TRACE ("Delete failed for for MAC:%s vlan:0x%"PRIx64" Error code %d",
                                   std_mac_to_string((const sai_mac_t *)
                                   &(fdb_entry.mac_address), mac_str,
                                   sizeof(mac_str)), fdb_entry.bv_id, sai_rc);
                remove_fdb_from_cache = false;
            } else {
                remove_fdb_from_cache = true;
            }
            if(remove_fdb_from_cache) {
                sai_remove_fdb_entry_node(fdb_entry_node);
            }
        }
        fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);
    }
    force_flush_fdb_all(SAI_NULL_OBJECT_ID, bv_id, flush_entry_type);
}

static void sai_delete_fdb_entry_nodes_per_port_vlan (sai_object_id_t bridge_port_id,
                                                      sai_object_id_t bv_id, bool delete_all,
                                                      sai_fdb_flush_entry_type_t flush_entry_type)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_key_t fdb_key;
    sai_fdb_entry_type_t entry_type = sai_get_sai_fdb_entry_type_for_flush(flush_entry_type);
    sai_fdb_entry_t fdb_entry;
    bool remove_fdb_from_cache = true;

    memset(&fdb_key, 0, sizeof(fdb_key));
    fdb_key.bv_id = bv_id;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    char mac_str[SAI_MAC_STR_LEN] = {0};

    fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);

    while(fdb_entry_node != NULL) {
        memcpy(&fdb_key,&(fdb_entry_node->fdb_key),
               sizeof(sai_fdb_entry_key_t));
        if(fdb_key.bv_id != bv_id){
            break;
        }
        if ((fdb_entry_node->bridge_port_id == bridge_port_id)){
            if ((delete_all == true) || (entry_type == fdb_entry_node->entry_type)) {
                fdb_entry.bv_id = fdb_entry_node->fdb_key.bv_id;
                memcpy(fdb_entry.mac_address, fdb_entry_node->fdb_key.mac_address,
                        sizeof(sai_mac_t));
                sai_rc = sai_fdb_npu_api_get()->flush_fdb_entry(&fdb_entry, true);

                if(sai_rc != SAI_STATUS_SUCCESS) {
                    SAI_FDB_LOG_TRACE ("Delete failed for for MAC:%s vlan:0x%"PRIx64""
                                       " Error code %d", std_mac_to_string((const sai_mac_t *)
                                       &(fdb_entry.mac_address), mac_str,
                                       sizeof(mac_str)), fdb_entry.bv_id, sai_rc);
                    remove_fdb_from_cache = false;
                } else {
                    remove_fdb_from_cache = true;
                }
                if(remove_fdb_from_cache) {
                    sai_remove_fdb_entry_node(fdb_entry_node);
                }
            }
        }
        fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);
    }
    force_flush_fdb_all(bridge_port_id, bv_id, flush_entry_type);
}

static bool sai_is_valid_bv_id(sai_object_id_t bv_id)
{
    sai_vlan_id_t vlan_id = VLAN_UNDEF;

    if(sai_is_obj_id_bridge(bv_id)) {
        return sai_is_bridge_created(bv_id);
    } else if(sai_is_obj_id_vlan(bv_id)) {
        vlan_id = sai_vlan_obj_id_to_vlan_id(bv_id);
        return sai_is_valid_vlan_id(vlan_id);
    }
    return false;
}

static sai_status_t sai_l2_flush_fdb_entry (sai_object_id_t switch_id, unsigned int attr_count,
                                            const sai_attribute_t *attr_list)
{
    sai_object_id_t            bridge_port_id = 0;
    sai_object_id_t            bv_id = 0;
    unsigned int               attr_idx = 0;
    sai_status_t               ret_val = SAI_STATUS_SUCCESS;
    bool                       delete_all = true;
    sai_fdb_flush_entry_type_t flush_type = SAI_FDB_FLUSH_ENTRY_TYPE_DYNAMIC;

    if((attr_count > 0) && (attr_list == NULL)) {
        SAI_FDB_LOG_ERR("Null pointer is passed as attribute list");
        STD_ASSERT(0);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        if(attr_list[attr_idx].id == SAI_FDB_FLUSH_ATTR_BRIDGE_PORT_ID) {
            bridge_port_id = attr_list[attr_idx].value.oid;
        } else if(attr_list[attr_idx].id == SAI_FDB_FLUSH_ATTR_BV_ID) {
            bv_id = attr_list[attr_idx].value.oid;
            if(!sai_is_valid_bv_id(bv_id)) {
                SAI_FDB_LOG_ERR("Invalid bv_id id 0x%"PRIx64"", bv_id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
        } else if(attr_list[attr_idx].id == SAI_FDB_FLUSH_ATTR_ENTRY_TYPE) {
            flush_type = attr_list[attr_idx].value.s32;
            delete_all = false;
        } else {
            return sai_get_indexed_ret_val(SAI_STATUS_UNKNOWN_ATTRIBUTE_0, attr_idx);
        }
    }

    if(bridge_port_id != 0) {
        if(!sai_is_bridge_port_created(bridge_port_id)) {
            SAI_FDB_LOG_ERR("Error - Unknown bridge port id 0x%"PRIx64"", bridge_port_id);
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }

    switch(flush_type) {
        case SAI_FDB_FLUSH_ENTRY_TYPE_DYNAMIC:
        case SAI_FDB_FLUSH_ENTRY_TYPE_STATIC:
            break;
        default:
            SAI_FDB_LOG_TRACE("Invalid flush type:%d",flush_type);
            return SAI_STATUS_INVALID_PARAMETER;
    }
    sai_fdb_lock();
    sai_bridge_lock();

    do {
        if((bridge_port_id == SAI_NULL_OBJECT_ID) && (bv_id == SAI_NULL_OBJECT_ID)) {
            SAI_FDB_LOG_TRACE("Flushing all FDB entries");
            sai_delete_all_fdb_entry_nodes (delete_all, flush_type);
        } else if((bridge_port_id != SAI_NULL_OBJECT_ID) && (bv_id == SAI_NULL_OBJECT_ID)) {
            SAI_FDB_LOG_TRACE("Flushing all FDB entries per bridge port:0x%"PRIx64"",
                    bridge_port_id);
            sai_delete_fdb_entry_nodes_per_port (bridge_port_id, delete_all, flush_type);
        } else if((bridge_port_id == SAI_NULL_OBJECT_ID) && (bv_id != SAI_NULL_OBJECT_ID)) {
            SAI_FDB_LOG_TRACE("Flushing all FDB entries per bv_id:0x%"PRIx64"",
                    bv_id);
            sai_delete_fdb_entry_nodes_per_vlan (bv_id, delete_all, flush_type);
        } else {
            SAI_FDB_LOG_TRACE("Flushing all FDB entries per bridge port:0x%"PRIx64" "
                    "per bv_id:0x%"PRIx64"", bridge_port_id, bv_id);
            sai_delete_fdb_entry_nodes_per_port_vlan (bridge_port_id, bv_id, delete_all,
                                                      flush_type);
        }
    } while(0);

    sai_bridge_unlock();
    sai_fdb_unlock();
    sai_fdb_wake_notification_thread ();
    return ret_val;
}

static sai_status_t sai_l2_remove_fdb_entry(const sai_fdb_entry_t *fdb_entry)
{
    char mac_str[SAI_MAC_STR_LEN] = {0};
    sai_status_t ret_val = SAI_STATUS_SUCCESS;
    sai_fdb_entry_node_t *fdb_entry_node = NULL;

    STD_ASSERT(fdb_entry != NULL);

    sai_fdb_lock();
    sai_bridge_lock();
    do {
        if(!sai_is_valid_bv_id(fdb_entry->bv_id)) {
            SAI_FDB_LOG_ERR("Invalid vlan/bridge id 0x%"PRIx64"", fdb_entry->bv_id);
            ret_val = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }
        fdb_entry_node = sai_get_fdb_entry_node(fdb_entry);
        if(fdb_entry_node == NULL) {
            ret_val = SAI_STATUS_ADDR_NOT_FOUND;
            break;
        }

        SAI_FDB_LOG_TRACE("Remove FDB Node MAC:%s vlan/bridge:0x%"PRIx64"",
                          std_mac_to_string(&(fdb_entry->mac_address), mac_str,
                          sizeof(mac_str)), fdb_entry->bv_id);
        ret_val = sai_fdb_npu_api_get()->flush_fdb_entry(fdb_entry , false);
        if(ret_val != SAI_STATUS_SUCCESS) {
            break;
        }
        sai_remove_fdb_entry_node(fdb_entry_node);
    } while (0);
    sai_bridge_unlock();
    sai_fdb_unlock();
    sai_fdb_wake_notification_thread ();
    return ret_val;
}

static sai_status_t sai_l2_create_fdb_entry(const sai_fdb_entry_t *fdb_entry,
                uint32_t attr_count,
                const sai_attribute_t *attr_list)
{
    unsigned int attr_index = 0;
    bool port_attr_init = false;
    bool type_attr_init = false;
    bool action_attr_init = false;
    bool endpoint_ip_attr_init = false;
    bool is_tunnel_bridge_port = false;
    bool is_forwarding_action = true;
    sai_status_t ret_val = SAI_STATUS_FAILURE;
    char mac_str[SAI_MAC_STR_LEN] = {0};
    sai_fdb_entry_node_t fdb_entry_node_data;

    STD_ASSERT(fdb_entry != NULL);
    STD_ASSERT(attr_list != NULL);
    if(attr_count == 0) {
        SAI_FDB_LOG_ERR("Invalid attribute count");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_fdb_lock();
    sai_bridge_lock();
    do {
        if(!sai_is_valid_bv_id(fdb_entry->bv_id)) {
            SAI_FDB_LOG_ERR("Invalid vlan/bridge id 0x%"PRIx64"", fdb_entry->bv_id);
            ret_val =  SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        SAI_FDB_LOG_TRACE("Create FDB entry for MAC:%s vlan:0x%"PRIx64"",
                          std_mac_to_string(&(fdb_entry->mac_address),
                          mac_str, sizeof(mac_str)), fdb_entry->bv_id);

        memset(&fdb_entry_node_data, 0, sizeof(fdb_entry_node_data));
        for(attr_index = 0; attr_index < attr_count; attr_index++) {
            ret_val = sai_is_valid_fdb_attribute_val(&attr_list[attr_index]);
            if(ret_val != SAI_STATUS_SUCCESS) {
                ret_val = sai_get_indexed_ret_val(ret_val, attr_index);
                SAI_FDB_LOG_ERR("Invalid attribute for MAC:%s vlan:0x%"PRIx64"",
                                std_mac_to_string(&(fdb_entry->mac_address),
                                mac_str, sizeof(mac_str)), fdb_entry->bv_id);
                break;
            }
            if(attr_list[attr_index].id == SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID) {
                fdb_entry_node_data.bridge_port_id = attr_list[attr_index].value.oid;
                port_attr_init = true;
            } else if(attr_list[attr_index].id == SAI_FDB_ENTRY_ATTR_TYPE) {
                fdb_entry_node_data.entry_type = (sai_fdb_entry_type_t)attr_list[attr_index].value.s32;
                type_attr_init = true;
            } else if(attr_list[attr_index].id ==SAI_FDB_ENTRY_ATTR_PACKET_ACTION) {
                fdb_entry_node_data.action = (sai_packet_action_t)attr_list[attr_index].value.s32;
                action_attr_init = true;
                is_forwarding_action = sai_fdb_entry_is_forwarding_action(
                                                               fdb_entry_node_data.action);
            } else if(attr_list[attr_index].id == SAI_FDB_ENTRY_ATTR_META_DATA) {
                fdb_entry_node_data.metadata = attr_list[attr_index].value.u32;
            } else if(attr_list[attr_index].id == SAI_FDB_ENTRY_ATTR_ENDPOINT_IP) {
                fdb_entry_node_data.end_point_ip = attr_list[attr_index].value.ipaddr;
                endpoint_ip_attr_init = true;
            }
        }
        if(!(type_attr_init && action_attr_init)) {
            if(!port_attr_init && is_forwarding_action) {
                SAI_FDB_LOG_ERR("Mandatory attr missing for MAC:%s vlan:0x%"PRIx64"",
                                std_mac_to_string(&(fdb_entry->mac_address),
                                                  mac_str, sizeof(mac_str)), fdb_entry->bv_id);
                ret_val = SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
                break;
            }
        }

        if(fdb_entry_node_data.bridge_port_id != SAI_NULL_OBJECT_ID) {
            is_tunnel_bridge_port = sai_is_bridge_port_type_tunnel(fdb_entry_node_data.bridge_port_id);
            if(is_tunnel_bridge_port && !endpoint_ip_attr_init) {
                SAI_FDB_LOG_ERR("Endpoint ip address not provided for tunnel "
                                "bridge port 0x%"PRIx64, fdb_entry_node_data.bridge_port_id);
                ret_val = SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
                break;
            }
        }

        if(endpoint_ip_attr_init && !is_tunnel_bridge_port) {
            SAI_FDB_LOG_ERR("Endpoint ip address attribute is valid only "
                            "for tunnel bridge port");
            ret_val = SAI_STATUS_INVALID_PARAMETER;
            break;
        }

        ret_val = sai_fdb_npu_api_get()->create_fdb_entry(fdb_entry, &fdb_entry_node_data);

        if(ret_val == SAI_STATUS_SUCCESS) {
            sai_insert_fdb_entry_node(fdb_entry, &fdb_entry_node_data);
        }
    }while (0);
    sai_bridge_unlock();
    sai_fdb_unlock();
    sai_fdb_wake_notification_thread ();
    return ret_val;
}

static sai_fdb_entry_node_t *sai_fdb_populate_node_from_hardware(
                                     const sai_fdb_entry_t *fdb_entry)
{
    sai_fdb_entry_node_t fdb_entry_node_data;
    char                  mac_str[SAI_MAC_STR_LEN] = {0};

    memset(&fdb_entry_node_data, 0, sizeof(fdb_entry_node_data));

    if(sai_fdb_npu_api_get()->get_fdb_entry_from_hardware(fdb_entry,
               &fdb_entry_node_data)!= SAI_STATUS_SUCCESS) {
        SAI_FDB_LOG_TRACE("FDB Entry not found for MAC:%s vlan:0x%"PRIx64"",
                           std_mac_to_string(&(fdb_entry->mac_address),
                            mac_str, sizeof(mac_str)), fdb_entry->bv_id);
        return NULL;
    }

    sai_insert_fdb_entry_node(fdb_entry, &fdb_entry_node_data);

    return sai_get_fdb_entry_node(fdb_entry);;
}

static sai_status_t sai_l2_set_fdb_entry_attribute(const sai_fdb_entry_t *fdb_entry,
                     const sai_attribute_t *attr)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_node_t temp_node;
    sai_status_t ret_val = SAI_STATUS_FAILURE;
    char mac_str[SAI_MAC_STR_LEN] = {0};

    STD_ASSERT(fdb_entry != NULL);
    STD_ASSERT(attr != NULL);
    memset(&temp_node, 0, sizeof(temp_node));

    sai_fdb_lock();
    sai_bridge_lock();
    do {
        if(!sai_is_valid_bv_id(fdb_entry->bv_id)) {
            SAI_FDB_LOG_ERR("Invalid vlan/bridge id 0x%"PRIx64"", fdb_entry->bv_id);
            ret_val =  SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        SAI_FDB_LOG_TRACE("Set FDB attribute:%d MAC:%s vlan:0x%"PRIx64"",
                attr->id,std_mac_to_string(&(fdb_entry->mac_address),
                    mac_str, sizeof(mac_str)), fdb_entry->bv_id);
        ret_val = sai_is_valid_fdb_attribute_val(attr);
        if(ret_val != SAI_STATUS_SUCCESS) {
            memset(mac_str, 0, sizeof(mac_str));
            SAI_FDB_LOG_ERR("Invalid attribute for MAC:%s vlan:0x%"PRIx64"",
                            std_mac_to_string(&(fdb_entry->mac_address),
                            mac_str, sizeof(mac_str)), fdb_entry->bv_id);
            break;
        }
        fdb_entry_node = sai_get_fdb_entry_node(fdb_entry);
        if(fdb_entry_node == NULL) {
            fdb_entry_node = sai_fdb_populate_node_from_hardware(fdb_entry);
            if(fdb_entry_node == NULL) {
                ret_val = SAI_STATUS_ADDR_NOT_FOUND;
                break;
            }
        }
        memcpy(&temp_node, fdb_entry_node, sizeof(temp_node));
        sai_update_fdb_entry_node(fdb_entry_node, attr);
        ret_val = sai_fdb_npu_api_get()->write_fdb_entry_to_hardware(fdb_entry_node);
        if(ret_val != SAI_STATUS_SUCCESS) {
            memcpy(fdb_entry_node, &temp_node, sizeof(temp_node));
        }
    } while(0);
    sai_bridge_unlock();
    sai_fdb_unlock();
    sai_fdb_wake_notification_thread ();

    return ret_val;
}

static sai_status_t sai_l2_get_fdb_entry_attribute(const sai_fdb_entry_t *fdb_entry,
                     uint32_t attr_count,
                     sai_attribute_t *attr_list)
{
    unsigned int attr_index = 0;
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    char mac_str[SAI_MAC_STR_LEN];
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    memset(mac_str, 0, sizeof(mac_str));

    STD_ASSERT(fdb_entry != NULL);
    STD_ASSERT(attr_list != NULL);
    if(attr_count == 0) {
        SAI_FDB_LOG_ERR("Invalid attribute count");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    sai_fdb_lock();
    sai_bridge_lock();
    do {
        if(!sai_is_valid_bv_id(fdb_entry->bv_id)) {
            SAI_FDB_LOG_ERR("Invalid vlan/bridge id 0x%"PRIx64"", fdb_entry->bv_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        fdb_entry_node = sai_get_fdb_entry_node(fdb_entry) ;

        /* If FDB entry is not in Cache, look into hardware. This need not
           look into SAI FDB BCM SW FIFO */
        if(fdb_entry_node == NULL) {
            fdb_entry_node = sai_fdb_populate_node_from_hardware(fdb_entry);
            if(fdb_entry_node == NULL) {
                sai_rc = SAI_STATUS_ADDR_NOT_FOUND;
                break;
            }
        }

        for(attr_index = 0; attr_index < attr_count; attr_index++) {
            if(attr_list[attr_index].id == SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID) {
                attr_list[attr_index].value.oid = fdb_entry_node->bridge_port_id;
            } else if(attr_list[attr_index].id == SAI_FDB_ENTRY_ATTR_TYPE) {
                attr_list[attr_index].value.s32 = fdb_entry_node->entry_type;
            } else if(attr_list[attr_index].id == SAI_FDB_ENTRY_ATTR_PACKET_ACTION) {
                attr_list[attr_index].value.s32 = fdb_entry_node->action;
            } else if(attr_list[attr_index].id == SAI_FDB_ENTRY_ATTR_META_DATA) {
                attr_list[attr_index].value.u32 = fdb_entry_node->metadata;
            } else if(attr_list[attr_index].id == SAI_FDB_ENTRY_ATTR_ENDPOINT_IP) {
                attr_list[attr_index].value.ipaddr = fdb_entry_node->end_point_ip;
            } else {
                sai_rc = sai_get_indexed_ret_val (SAI_STATUS_UNKNOWN_ATTRIBUTE_0, attr_index);
                break;
            }
        }
    } while(0);
    sai_bridge_unlock();
    sai_fdb_unlock();
    sai_fdb_wake_notification_thread ();
    return sai_rc;
}

sai_status_t sai_l2_fdb_set_aging_time(uint32_t value)
{
    return sai_fdb_npu_api_get()->set_aging_time(value);
}

sai_status_t sai_l2_fdb_get_aging_time(uint32_t *value)
{
    STD_ASSERT(value != NULL);
    return sai_fdb_npu_api_get()->get_aging_time(value);
}

static void sai_fdb_take_module_lock (sai_object_id_t bv_id, sai_object_id_t bridge_port_id)
{
     if(sai_is_obj_id_vlan(bv_id)) {
         sai_vlan_lock();
     }
     sai_stp_lock();
     sai_bridge_lock();
     if(sai_is_bridge_port_obj_lag(bridge_port_id)) {
         sai_lag_lock();
     }
}

static void sai_fdb_give_module_lock (sai_object_id_t bv_id, sai_object_id_t bridge_port_id)
{
     if(sai_is_bridge_port_obj_lag(bridge_port_id)) {
         sai_lag_unlock();
     }
     sai_bridge_unlock();
     sai_stp_unlock();
     if(sai_is_obj_id_vlan(bv_id)) {
         sai_vlan_unlock();
     }

}
static bool sai_is_valid_fdb_learn_on_lag (const sai_fdb_entry_t *fdb_entry, sai_object_id_t lag_id)
{
    uint_t lag_port_count = 0;
    uint_t lag_port_idx = 0;
    sai_status_t ret_val;
    sai_object_list_t lag_port_list;
    bool valid_learn = false;

    if(!sai_is_lag_created(lag_id)) {
        return false;
    }

    ret_val = sai_lag_port_count_get (lag_id, &lag_port_count);

    if((ret_val != SAI_STATUS_SUCCESS) || (lag_port_count == 0)) {
        return false;
    }

    lag_port_list.count = lag_port_count;
    lag_port_list.list = calloc (lag_port_list.count, sizeof(sai_object_id_t));

    do {
        ret_val = sai_lag_port_list_get (lag_id, &lag_port_list);
        if(ret_val != SAI_STATUS_SUCCESS) {
            break;

        }

        /* Check if atleast one member is oper up */
        for (lag_port_idx = 0; lag_port_idx < lag_port_count; lag_port_idx++) {
            if (sai_port_is_oper_up (lag_port_list.list[lag_port_idx])) {
                valid_learn = true;
                break;
            }
        }

    } while (0);

    free (lag_port_list.list);
    return valid_learn;
}

static bool sai_is_valid_fdb_learn (const sai_fdb_entry_t *fdb_entry,
                                    sai_object_id_t bridge_port_id)
{
    sai_object_id_t port_obj_id = SAI_NULL_OBJECT_ID;
    sai_vlan_id_t   vlan_id = VLAN_UNDEF;
    sai_status_t    sai_rc = SAI_STATUS_FAILURE;
    bool            admin_state = false;

    if(!sai_is_bridge_port_created(bridge_port_id)) {
        SAI_FDB_LOG_ERR("Bridge port 0x%"PRIx64" does not exists", bridge_port_id);
        return false;
    }
    sai_rc = sai_bridge_port_get_admin_state(bridge_port_id, &admin_state);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_FDB_LOG_ERR("Error %d in getting bridge port 0x%"PRIx64" admin state",
                        sai_rc, bridge_port_id);
        return sai_rc;
    }

    if(!admin_state) {
        return false;
    }

    if(sai_is_bridge_port_type_port(bridge_port_id)) {
        sai_rc = sai_bridge_port_get_port_id(bridge_port_id, &port_obj_id);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_FDB_LOG_ERR("Error %d in getting port obj from bridge port 0x%"PRIx64"",
                            sai_rc, bridge_port_id);
            return false;
        }
        if(!sai_is_obj_id_vlan(fdb_entry->bv_id)) {
            SAI_FDB_LOG_ERR("Invalid object type for bv_id 0x%"PRIx64"",fdb_entry->bv_id);
            return false;
        }

        vlan_id = sai_vlan_obj_id_to_vlan_id(fdb_entry->bv_id);
        if (!sai_is_bridge_port_vlan_member (vlan_id, bridge_port_id)) {
            return false;
        }

        if (!sai_stp_can_bridge_port_learn_mac (vlan_id, bridge_port_id)) {
            return false;
        }
        if(sai_is_obj_id_port(port_obj_id)) {
            if (!sai_port_is_oper_up (port_obj_id)) {
                return false;
            }
        } else if(sai_is_obj_id_lag (port_obj_id)) {
            return sai_is_valid_fdb_learn_on_lag (fdb_entry, port_obj_id);
        } else {
            SAI_FDB_LOG_ERR("Error unknown type for attached object 0x%"PRIx64"", port_obj_id);
            return false;
        }
    } else if (sai_is_bridge_port_type_sub_port(bridge_port_id)) {
        if((!sai_is_obj_id_bridge(fdb_entry->bv_id))
            || (!sai_is_bridge_created(fdb_entry->bv_id))) {
            SAI_FDB_LOG_ERR("Invalid object type for bv_id 0x%"PRIx64"",fdb_entry->bv_id);
            return false;
        }
        if(!sai_is_bridge_port_created(bridge_port_id)) {
            SAI_FDB_LOG_ERR("Error - Unknown bridge port id 0x%"PRIx64"",bridge_port_id);
            return false;
        }
    }
    return true;
}

static void sai_common_fdb_event_notification (uint32_t count, sai_fdb_event_data_t *data)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_node_t fdb_entry_node_data;
    unsigned int attr_idx = 0;
    unsigned int entry_idx = 0;
    uint_t valid_count = 0;
    sai_status_t sai_rc;
    sai_fdb_event_notification_data_t *notification_data;
    char mac_str[SAI_MAC_STR_LEN] = {0};
    sai_object_id_t bv_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;

    STD_ASSERT(data != NULL);
    if((count == 0) || count > SAI_FDB_MAX_MACS_PER_CALLBACK) {
        SAI_FDB_LOG_ERR("Invalid FDB num count %d",count);
        return;
    }
    sai_fdb_lock();
    memset(valid_notification_data, 0, sizeof(valid_notification_data));
    for(entry_idx = 0; entry_idx < count; entry_idx++) {
        notification_data = data[entry_idx].notification_data;
        memset(&fdb_entry_node_data, 0, sizeof(fdb_entry_node_data));
        for(attr_idx = 0; attr_idx < notification_data->attr_count; attr_idx++) {
            if(notification_data->attr[attr_idx].id == SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID) {
                fdb_entry_node_data.bridge_port_id = notification_data->attr[attr_idx].value.oid;
            } else if(notification_data->attr[attr_idx].id == SAI_FDB_ENTRY_ATTR_TYPE) {
                fdb_entry_node_data.entry_type = (sai_fdb_entry_type_t)
                                                 notification_data->attr[attr_idx].value.s32;
            } else if(notification_data->attr[attr_idx].id == SAI_FDB_ENTRY_ATTR_PACKET_ACTION) {
                fdb_entry_node_data.action = (sai_packet_action_t)
                                              notification_data->attr[attr_idx].value.s32;
            } else if (notification_data->attr[attr_idx].id == SAI_FDB_ENTRY_ATTR_META_DATA) {
                fdb_entry_node_data.metadata = notification_data->attr[attr_idx].value.u32;
            } else if(notification_data->attr[attr_idx].id == SAI_FDB_ENTRY_ATTR_ENDPOINT_IP) {
                fdb_entry_node_data.end_point_ip =
                                        notification_data->attr[attr_idx].value.ipaddr;
            }
        }

        fdb_entry_node_data.is_pending_entry = data[entry_idx].is_pending_entry;

        bv_id = notification_data->fdb_entry.bv_id;
        bridge_port_id =  fdb_entry_node_data.bridge_port_id;
        sai_fdb_take_module_lock(bv_id, bridge_port_id);
        if((notification_data->event_type == SAI_FDB_EVENT_LEARNED) ||
           (notification_data->event_type == SAI_FDB_EVENT_MOVE)) {
            if(!sai_is_valid_fdb_learn ((const sai_fdb_entry_t*)&notification_data->fdb_entry,
                                         fdb_entry_node_data.bridge_port_id)) {
                SAI_FDB_LOG_TRACE ("Invalid learn for MAC:%s vlan:0x%"PRIx64" bridge port "
                                   "0x%"PRIx64"", std_mac_to_string((const sai_mac_t *)
                                   &(notification_data->fdb_entry.mac_address), mac_str,
                                   sizeof(mac_str)), notification_data->fdb_entry.bv_id,
                                   fdb_entry_node_data.bridge_port_id);
                sai_fdb_npu_api_get()->flush_fdb_entry(&notification_data->fdb_entry, false);
            } else {
                sai_rc = sai_insert_fdb_entry_node((const sai_fdb_entry_t*)&notification_data->fdb_entry,
                                                   &fdb_entry_node_data);
                if((sai_rc == SAI_STATUS_SUCCESS) ||
                   (sai_rc == SAI_STATUS_ITEM_ALREADY_EXISTS)) {
                    valid_notification_data[valid_count] = *notification_data;
                    valid_count++;
                }
            }
        } else if((notification_data->event_type == SAI_FDB_EVENT_AGED) ||
                 (notification_data->event_type == SAI_FDB_EVENT_FLUSHED)) {
            fdb_entry_node = sai_get_fdb_entry_node(&notification_data->fdb_entry);
            if(fdb_entry_node != NULL) {
                sai_remove_fdb_entry_node (fdb_entry_node);
                valid_notification_data[valid_count] = *notification_data;
                valid_count++;
            }
        }
        sai_fdb_give_module_lock(bv_id, bridge_port_id);
    }
    sai_fdb_unlock();
    sai_fdb_wake_notification_thread ();
    if(sai_l2_fdb_notification_fn != NULL) {
        sai_l2_fdb_notification_fn (valid_count, valid_notification_data);
    }
}
sai_status_t sai_l2_fdb_register_callback(sai_fdb_event_notification_fn
                                                         fdb_notification_fn)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;

    sai_fdb_lock();
    ret_val = sai_fdb_npu_api_get()->register_callback((sai_fdb_npu_event_notification_fn)
                                                        sai_common_fdb_event_notification);
    if(ret_val == SAI_STATUS_SUCCESS) {
        sai_l2_fdb_notification_fn = fdb_notification_fn;
    }
    sai_fdb_unlock();
    return ret_val;
}

sai_status_t sai_l2_register_fdb_entry (const sai_fdb_entry_t *fdb_entry)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;

    sai_fdb_lock();
    ret_val =  sai_fdb_write_registered_entry_into_cache (fdb_entry);
    sai_fdb_unlock();
    return ret_val;
}

sai_status_t sai_l2_deregister_fdb_entry (const sai_fdb_entry_t *fdb_entry)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;

    sai_fdb_lock();
    ret_val = sai_fdb_remove_registered_entry_from_cache (fdb_entry);
    sai_fdb_unlock();
    return ret_val;
}

void sai_l2_fdb_register_internal_callback (sai_fdb_internal_callback_fn
                                                       fdb_internal_callback)
{
    sai_fdb_lock();
    sai_fdb_internal_callback_cache_update(fdb_internal_callback);
    sai_fdb_unlock();
}

sai_status_t sai_l2_fdb_set_switch_max_learned_address(uint32_t value)
{
    return sai_fdb_npu_api_get()->set_switch_max_learned_address(value);
}

sai_status_t sai_l2_fdb_get_switch_max_learned_address(uint32_t *value)
{
    STD_ASSERT(value != NULL);
    return sai_fdb_npu_api_get()->get_switch_max_learned_address(value);
}

sai_status_t sai_l2_fdb_ucast_miss_action_set(const sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);
    return sai_fdb_npu_api_get()->fdb_ucast_miss_action_set(attr);
}

sai_status_t sai_l2_fdb_ucast_miss_action_get(sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);
    return sai_fdb_npu_api_get()->fdb_ucast_miss_action_get(attr);
}

sai_status_t sai_l2_fdb_mcast_miss_action_set(const sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);
    return sai_fdb_npu_api_get()->fdb_mcast_miss_action_set(attr);
}

sai_status_t sai_l2_fdb_mcast_miss_action_get(sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);
    return sai_fdb_npu_api_get()->fdb_mcast_miss_action_get(attr);
}

sai_status_t sai_l2_fdb_bcast_miss_action_set(const sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);
    return sai_fdb_npu_api_get()->fdb_bcast_miss_action_set(attr);
}

sai_status_t sai_l2_fdb_bcast_miss_action_get(sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);
    return sai_fdb_npu_api_get()->fdb_bcast_miss_action_get(attr);
}

sai_status_t sai_l2_mcast_cpu_flood_enable_set(bool enable)
{
     return sai_fdb_npu_api_get()->mcast_cpu_flood_enable_set(enable);
}

sai_status_t sai_l2_mcast_cpu_flood_enable_get(bool *enable)
{
    STD_ASSERT(enable != NULL);
    return sai_fdb_npu_api_get()->mcast_cpu_flood_enable_get(enable);
}

sai_status_t sai_l2_bcast_cpu_flood_enable_set(bool enable)
{
     return sai_fdb_npu_api_get()->bcast_cpu_flood_enable_set(enable);
}

sai_status_t sai_l2_bcast_cpu_flood_enable_get(bool *enable)
{
    STD_ASSERT(enable != NULL);
    return sai_fdb_npu_api_get()->bcast_cpu_flood_enable_get(enable);
}
sai_status_t sai_l2_get_fdb_table_size(sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);
    return sai_fdb_npu_api_get()->get_fdb_table_size(attr);
}

static sai_fdb_api_t sai_fdb_method_table =
{
    sai_l2_create_fdb_entry,
    sai_l2_remove_fdb_entry,
    sai_l2_set_fdb_entry_attribute,
    sai_l2_get_fdb_entry_attribute,
    sai_l2_flush_fdb_entry,
};

sai_fdb_api_t  *sai_fdb_api_query (void)
{
    return (&sai_fdb_method_table);
}
