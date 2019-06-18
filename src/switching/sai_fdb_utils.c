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
* @file sai_fdb_utils.c
*
* @brief This file contains utility APIs for SAI FDB module
*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "std_assert.h"
#include "saifdb.h"
#include "saitypes.h"
#include "saiswitch.h"
#include "saistatus.h"
#include "std_radix.h"
#include "sai_fdb_api.h"
#include "sai_fdb_common.h"
#include "std_mutex_lock.h"
#include "std_mac_utils.h"
#include "sai_switch_utils.h"
#include "sai_oid_utils.h"
#include "sai_npu_fdb.h"
#include "sai_bridge_api.h"
#include "sai_l3_util.h"


static sai_fdb_global_data_t sai_fdb_global_cache;
static std_mutex_lock_create_static_init_fast(fdb_lock);
static sai_fdb_internal_callback_fn fdb_internal_callback = NULL;
static sai_npu_flush_fdb_entry_fn sai_npu_flush_fdb_entry = NULL;

void sai_fdb_lock(void)
{
    std_mutex_lock(&fdb_lock);
}

void sai_fdb_unlock(void)
{
    std_mutex_unlock(&fdb_lock);
}

sai_status_t sai_init_fdb_tree(void)
{
    SAI_FDB_LOG_TRACE("Performing FDB Module Init");
    sai_fdb_global_cache.sai_global_fdb_tree = std_radix_create("FDBTree", SAI_FDB_ENTRY_KEY_SIZE,
                                           NULL, NULL, 0);
    if(sai_fdb_global_cache.sai_global_fdb_tree == NULL) {
        SAI_FDB_LOG_CRIT("Unable to perform FDB Cache Init");
        return SAI_STATUS_UNINITIALIZED;
    }

    sai_fdb_global_cache.sai_registered_fdb_entry_tree = std_radix_create("FDBNotificationTree",
                                                        SAI_FDB_ENTRY_KEY_SIZE,
                                                        NULL, NULL, 0);

    if(sai_fdb_global_cache.sai_registered_fdb_entry_tree == NULL) {
        SAI_FDB_LOG_CRIT("Unable to perform FDB Registered entries tree Init");
        return SAI_STATUS_UNINITIALIZED;
    }

    std_radix_enable_radical(sai_fdb_global_cache.sai_registered_fdb_entry_tree);
    std_radical_walkconstructor (sai_fdb_global_cache.sai_registered_fdb_entry_tree,
                                 &(sai_fdb_global_cache.fdb_marker));
    sai_fdb_global_cache.num_notifications = 0;
    sai_fdb_global_cache.cur_notification_idx = 0;
    return SAI_STATUS_SUCCESS;
}

std_rt_table *sai_fdb_cache_get(void)
{
    return sai_fdb_global_cache.sai_global_fdb_tree;
}

std_rt_table *sai_fdb_registered_entry_cache_get(void)
{
    return sai_fdb_global_cache.sai_registered_fdb_entry_tree;
}
sai_fdb_entry_node_t* sai_get_fdb_entry_node(const sai_fdb_entry_t *fdb_entry)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_key_t fdb_key;

    STD_ASSERT(fdb_entry != NULL);
    memset(&fdb_key, 0, sizeof(fdb_key));
    memcpy(fdb_key.mac_address, fdb_entry->mac_address, sizeof(sai_mac_t));
    fdb_key.bv_id = fdb_entry->bv_id;

    fdb_entry_node = (sai_fdb_entry_node_t *)std_radix_getexact(
                                                        sai_fdb_global_cache.sai_global_fdb_tree,
                                                        (u_char *)&fdb_key,
                                                        SAI_FDB_ENTRY_KEY_SIZE);
    return fdb_entry_node;
}

sai_fdb_registered_node_t* sai_get_fdb_registered_node (const sai_fdb_entry_t *fdb_entry)
{
    sai_fdb_registered_node_t *fdb_registered_node = NULL;
    sai_fdb_entry_key_t fdb_key;

    STD_ASSERT(fdb_entry != NULL);
    memset(&fdb_key, 0, sizeof(fdb_key));
    memcpy(fdb_key.mac_address, fdb_entry->mac_address, sizeof(sai_mac_t));
    fdb_key.bv_id = fdb_entry->bv_id;

    fdb_registered_node = (sai_fdb_registered_node_t *)
                             std_radix_getexact (sai_fdb_global_cache.sai_registered_fdb_entry_tree,
                                                (u_char *)&fdb_key, SAI_FDB_ENTRY_KEY_SIZE);
    return fdb_registered_node;
}

sai_status_t sai_fdb_get_bridge_port_from_cache(const sai_fdb_entry_t *fdb_entry,
                                                sai_object_id_t *bridge_port_id)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;

    STD_ASSERT(fdb_entry != NULL);
    STD_ASSERT(bridge_port_id != NULL);
    fdb_entry_node = sai_get_fdb_entry_node(fdb_entry);
    if(fdb_entry_node == NULL) {
        return SAI_STATUS_ADDR_NOT_FOUND;
    }
    *bridge_port_id = fdb_entry_node->bridge_port_id;
    return SAI_STATUS_SUCCESS;
}
void sai_remove_fdb_entry_node (sai_fdb_entry_node_t *fdb_entry_node)
{
    sai_fdb_registered_node_t *fdb_registered_node = NULL;
    sai_fdb_entry_t fdb_entry;
    STD_ASSERT(fdb_entry_node != NULL);

    fdb_entry.bv_id = fdb_entry_node->fdb_key.bv_id;
    memcpy(fdb_entry.mac_address, fdb_entry_node->fdb_key.mac_address, sizeof(sai_mac_t));

    fdb_registered_node = sai_get_fdb_registered_node(&fdb_entry);
    if(fdb_registered_node != NULL) {
        fdb_registered_node->fdb_event = SAI_FDB_EVENT_FLUSHED;
        std_radical_appendtochangelist (sai_fdb_global_cache.sai_registered_fdb_entry_tree,
                                        &fdb_registered_node->fdb_radical_head);
        if(!fdb_registered_node->node_in_cl) {
            sai_fdb_global_cache.num_notifications++;
        }
        fdb_registered_node->node_in_cl = true;
    }
    sai_bridge_port_decrement_fdb_count(fdb_entry_node->bridge_port_id);
    std_radix_remove(sai_fdb_global_cache.sai_global_fdb_tree,&(fdb_entry_node->fdb_rt_head));
    free(fdb_entry_node);
}

sai_fdb_entry_node_t *sai_get_next_fdb_entry_node (sai_fdb_entry_key_t *fdb_key)
{
    sai_fdb_entry_node_t *fdb_entry_node;

    fdb_entry_node = (sai_fdb_entry_node_t *)std_radix_getnext(
                                                        sai_fdb_global_cache.sai_global_fdb_tree,
                                                        (u_char *)fdb_key, SAI_FDB_ENTRY_KEY_SIZE);
    return fdb_entry_node;
}

sai_fdb_registered_node_t *sai_get_next_fdb_registered_node (sai_fdb_entry_key_t *fdb_key)
{
    sai_fdb_registered_node_t *fdb_registered_node;

    fdb_registered_node = (sai_fdb_registered_node_t *)std_radix_getnext(
                                                        sai_fdb_global_cache.sai_registered_fdb_entry_tree,
                                                        (u_char *)fdb_key, SAI_FDB_ENTRY_KEY_SIZE);
    return fdb_registered_node;
}

sai_fdb_entry_node_t *sai_add_fdb_entry_node_in_global_tree(sai_fdb_entry_node_t
                                                            *fdb_entry_node)
{
    sai_fdb_entry_node_t *p_out_fdb_entry_node = NULL;
    std_rt_head *fdb_rt_head = NULL;
    char mac_str[SAI_MAC_STR_LEN] = {0};

    STD_ASSERT(fdb_entry_node != NULL);
    fdb_entry_node->fdb_rt_head.rth_addr = (unsigned char *)
                                                 &fdb_entry_node->fdb_key;
    fdb_rt_head = std_radix_insert (sai_fdb_global_cache.sai_global_fdb_tree,
                                    &(fdb_entry_node->fdb_rt_head),
                                    SAI_FDB_ENTRY_KEY_SIZE);

    if(fdb_rt_head == NULL) {
        SAI_FDB_LOG_ERR("Unable to add fdb node MAC:%s vlan:0x%"PRIx64"",
                        std_mac_to_string((const sai_mac_t*)
                              &(fdb_entry_node->fdb_key.mac_address), mac_str,
                             sizeof(mac_str)), fdb_entry_node->fdb_key.bv_id);
    }
    else {
        p_out_fdb_entry_node = (sai_fdb_entry_node_t *)
            ((char *) fdb_rt_head - STD_STR_OFFSET_OF (sai_fdb_entry_node_t, fdb_rt_head));
    }

    return p_out_fdb_entry_node;
}

sai_status_t sai_insert_fdb_entry_node(const sai_fdb_entry_t* fdb_entry,
                                       sai_fdb_entry_node_t *fdb_entry_node_data)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_node_t *tmp_fdb_entry_node;
    sai_fdb_registered_node_t *fdb_registered_node = NULL;
    bool notify = true;

    STD_ASSERT(fdb_entry != NULL);
    fdb_entry_node = (sai_fdb_entry_node_t *)
                           calloc(1, sizeof(sai_fdb_entry_node_t));
    if(fdb_entry_node == NULL) {
        SAI_FDB_LOG_CRIT("No memory for %lu",
                         sizeof(sai_fdb_entry_node_t));
        return SAI_STATUS_NO_MEMORY;
    }
    fdb_entry_node->fdb_key.bv_id = fdb_entry->bv_id;
    memcpy(fdb_entry_node->fdb_key.mac_address,fdb_entry->mac_address,
           sizeof(sai_mac_t));
    tmp_fdb_entry_node = sai_add_fdb_entry_node_in_global_tree (fdb_entry_node);

    if (tmp_fdb_entry_node != fdb_entry_node) {
        free (fdb_entry_node);
        if (tmp_fdb_entry_node == NULL) {
            return SAI_STATUS_FAILURE;
        }
        fdb_entry_node = tmp_fdb_entry_node;
        if((fdb_entry_node->bridge_port_id == fdb_entry_node_data->bridge_port_id) &&
           (fdb_entry_node->entry_type == fdb_entry_node_data->entry_type) &&
           (fdb_entry_node->action == fdb_entry_node_data->action)) {
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }
        if(fdb_entry_node->bridge_port_id == fdb_entry_node_data->bridge_port_id) {
            notify = false;
        }
        /* For move action since the fdb entry is getting dissociated from old port
         * the ref count on the old port needs to be decremented.
         */
        if(fdb_entry_node->bridge_port_id != fdb_entry_node_data->bridge_port_id) {
            sai_bridge_port_decrement_fdb_count(fdb_entry_node->bridge_port_id);
        }
    }
    if(fdb_entry_node_data->is_pending_entry) {
        notify = false;
    }
    fdb_registered_node = sai_get_fdb_registered_node(fdb_entry);
    if((notify) && (fdb_registered_node != NULL)) {
        fdb_registered_node->fdb_event = SAI_FDB_EVENT_LEARNED;
        std_radical_appendtochangelist (sai_fdb_global_cache.sai_registered_fdb_entry_tree,
                                        &fdb_registered_node->fdb_radical_head);
        fdb_registered_node->bridge_port_id = fdb_entry_node_data->bridge_port_id;
        if(!fdb_registered_node->node_in_cl) {
            sai_fdb_global_cache.num_notifications++;
        }
        fdb_registered_node->node_in_cl = true;
    }
    fdb_entry_node->bridge_port_id =fdb_entry_node_data-> bridge_port_id;
    fdb_entry_node->entry_type = fdb_entry_node_data->entry_type;
    fdb_entry_node->action = fdb_entry_node_data->action;
    fdb_entry_node->metadata = fdb_entry_node_data->metadata;
    fdb_entry_node->is_pending_entry = fdb_entry_node_data->is_pending_entry;
    fdb_entry_node->end_point_ip = fdb_entry_node_data->end_point_ip;
    if(fdb_entry_node->bridge_port_id != SAI_NULL_OBJECT_ID) {
        sai_bridge_port_increment_fdb_count(fdb_entry_node->bridge_port_id);
    }
    return SAI_STATUS_SUCCESS;
}

void sai_fdb_internal_callback_cache_update (sai_fdb_internal_callback_fn
                                                 fdb_callback)
{
   fdb_internal_callback = fdb_callback;
}

void sai_fdb_npu_flush_callback_cache_update (sai_npu_flush_fdb_entry_fn flush_fdb_entry)
{
   sai_npu_flush_fdb_entry = flush_fdb_entry;
}

int sai_fdb_notification_list_walk(std_radical_head_t *radical_head, va_list ap)
{
   sai_fdb_registered_node_t *fdb_registered_node = (sai_fdb_registered_node_t *)radical_head;
   char                  mac_str[SAI_MAC_STR_LEN] = {0};
   sai_fdb_internal_notification_data_t *data = NULL;

   SAI_FDB_LOG_INFO ("FDB Node MAC:%s vlan:0x%"PRIx64" Event:%d port:0x%"PRIx64"\r\n",
                     std_mac_to_string((const sai_mac_t*)
                     &(fdb_registered_node->fdb_key.mac_address), mac_str,
                     sizeof(mac_str)), fdb_registered_node->fdb_key.bv_id,
                     fdb_registered_node->fdb_event, fdb_registered_node->bridge_port_id);

   data = va_arg(ap,sai_fdb_internal_notification_data_t *);

   memcpy(data[sai_fdb_global_cache.cur_notification_idx].fdb_entry.mac_address,
          fdb_registered_node->fdb_key.mac_address, sizeof(sai_mac_t));


   data[sai_fdb_global_cache.cur_notification_idx].fdb_entry.bv_id
                                  = fdb_registered_node->fdb_key.bv_id;
   data[sai_fdb_global_cache.cur_notification_idx].bridge_port_id = fdb_registered_node->bridge_port_id;
   data[sai_fdb_global_cache.cur_notification_idx].fdb_event = fdb_registered_node->fdb_event;
   fdb_registered_node->node_in_cl = false;
   sai_fdb_global_cache.cur_notification_idx++;
   sai_fdb_global_cache.num_notifications--;
   return 0;
}

bool sai_fdb_is_notifications_pending (void)
{
    if (sai_fdb_global_cache.num_notifications > 0) {
        return true;
    }
    return false;
}

void sai_fdb_send_internal_notifications(void)
{
    int ret;
    sai_fdb_internal_notification_data_t *data = NULL;
    uint_t num_notifications = 0;

    if(fdb_internal_callback == NULL) {
        return;
    }

    while (sai_fdb_global_cache.num_notifications > 0) {
        sai_fdb_lock();

        if(sai_fdb_global_cache.num_notifications < SAI_FDB_MAX_NOTIFICATION_NODES) {
            num_notifications = sai_fdb_global_cache.num_notifications;
        } else {
            num_notifications = SAI_FDB_MAX_NOTIFICATION_NODES;
        }
        data = calloc(num_notifications, sizeof(sai_fdb_internal_notification_data_t));
        if (data == NULL) {
            SAI_FDB_LOG_CRIT ("Error- No memory to allocate for walk");
            sai_fdb_unlock();
            return;
        }
        std_radical_walkchangelist (sai_fdb_global_cache.sai_registered_fdb_entry_tree,
                                    &sai_fdb_global_cache.fdb_marker,
                                    sai_fdb_notification_list_walk, 0,
                                    SAI_FDB_MAX_NOTIFICATION_NODES,
                                    std_radix_getversion(sai_fdb_global_cache.
                                    sai_registered_fdb_entry_tree),&ret, data);

        sai_fdb_unlock();
        fdb_internal_callback (sai_fdb_global_cache.cur_notification_idx, data);
        free(data);
        sai_fdb_global_cache.cur_notification_idx = 0;
    }
}

sai_status_t sai_fdb_write_registered_entry_into_cache (const sai_fdb_entry_t *fdb_entry)
{
    sai_fdb_registered_node_t *fdb_registered_node = NULL;
    std_rt_head *fdb_rt_head = NULL;
    char mac_str[SAI_MAC_STR_LEN] = {0};
    sai_fdb_entry_node_t  *fdb_entry_node = sai_get_fdb_entry_node(fdb_entry);

    STD_ASSERT(fdb_entry != NULL);
    fdb_registered_node = (sai_fdb_registered_node_t *)
                           calloc(1, sizeof(sai_fdb_registered_node_t));

    if(fdb_registered_node == NULL) {
        SAI_FDB_LOG_CRIT ("No memory for %lu",
                          sizeof(sai_fdb_registered_node_t));
        return SAI_STATUS_NO_MEMORY;
    }

    fdb_registered_node->fdb_key.bv_id = fdb_entry->bv_id;
    memcpy (fdb_registered_node->fdb_key.mac_address,fdb_entry->mac_address,
            sizeof(sai_mac_t));
    if(fdb_entry_node != NULL) {
        fdb_registered_node->bridge_port_id = fdb_entry_node->bridge_port_id;
    }
    fdb_registered_node->fdb_radical_head.rth_addr = (unsigned char *)
                                                   &fdb_registered_node->fdb_key;
    fdb_rt_head = std_radix_insert (sai_fdb_global_cache.sai_registered_fdb_entry_tree,
                                    (std_rt_head *)&(fdb_registered_node->fdb_radical_head),
                                    SAI_FDB_ENTRY_KEY_SIZE);

    if(fdb_rt_head == NULL) {
        SAI_FDB_LOG_ERR ("Unable to add fdb node MAC:%s vlan:0x%"PRIx64"",
                         std_mac_to_string((const sai_mac_t*)
                         &(fdb_registered_node->fdb_key.mac_address), mac_str,
                         sizeof(mac_str)), fdb_registered_node->fdb_key.bv_id);
        free (fdb_registered_node);
        return SAI_STATUS_FAILURE;
    }
    else {
        if(fdb_rt_head != (std_rt_head *)&(fdb_registered_node->fdb_radical_head)) {
            SAI_FDB_LOG_INFO ("Duplicate add to the tree");
            free(fdb_registered_node);
        }
    }


    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_fdb_remove_registered_entry_from_cache (const sai_fdb_entry_t *fdb_entry)
{
    sai_fdb_registered_node_t *fdb_registered_node = NULL;
    char mac_str[SAI_MAC_STR_LEN] = {0};

    STD_ASSERT(fdb_entry != NULL);

    fdb_registered_node = sai_get_fdb_registered_node (fdb_entry);
    if(fdb_registered_node == NULL) {
        SAI_FDB_LOG_ERR("FDB Entry not found MAC:%s vlan:0x%"PRIx64"",
                         std_mac_to_string((const sai_mac_t*)&(fdb_entry->mac_address), mac_str,
                                           sizeof(mac_str)), fdb_entry->bv_id);
        return SAI_STATUS_ADDR_NOT_FOUND;
    }
    if(fdb_registered_node->node_in_cl) {
        SAI_FDB_LOG_WARN("Warning object is in CL");
        return SAI_STATUS_OBJECT_IN_USE;
    }
    std_radix_remove (sai_fdb_global_cache.sai_registered_fdb_entry_tree,
                      (std_rt_head *)&(fdb_registered_node->fdb_radical_head));
    free(fdb_registered_node);
    return SAI_STATUS_SUCCESS;

}

void sai_update_fdb_entry_node (sai_fdb_entry_node_t *fdb_entry_node,
                                const sai_attribute_t *attr)
{
    sai_fdb_registered_node_t *fdb_registered_node = NULL;
    sai_fdb_entry_t fdb_entry;

    STD_ASSERT(fdb_entry_node != NULL);
    STD_ASSERT(attr != NULL);
    if(attr->id == SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID) {
        if(fdb_entry_node->bridge_port_id != attr->value.oid) {
            sai_bridge_port_decrement_fdb_count(fdb_entry_node->bridge_port_id);
            fdb_entry_node->bridge_port_id = attr->value.oid;
            sai_bridge_port_increment_fdb_count(fdb_entry_node->bridge_port_id);
            fdb_entry.bv_id = fdb_entry_node->fdb_key.bv_id;
            memcpy(fdb_entry.mac_address, fdb_entry_node->fdb_key.mac_address,
                    sizeof(sai_mac_t));
            fdb_registered_node = sai_get_fdb_registered_node((const sai_fdb_entry_t *)
                                                                  &fdb_entry);
            if(fdb_registered_node != NULL) {
                fdb_registered_node->fdb_event = SAI_FDB_EVENT_LEARNED;
                fdb_registered_node->bridge_port_id = attr->value.oid;
                std_radical_appendtochangelist (sai_fdb_global_cache.sai_registered_fdb_entry_tree,
                                                &fdb_registered_node->fdb_radical_head);
                if(!fdb_registered_node->node_in_cl) {
                    sai_fdb_global_cache.num_notifications++;
                }
                fdb_registered_node->node_in_cl = true;
            }
        }

    } else if(attr->id == SAI_FDB_ENTRY_ATTR_TYPE) {
        fdb_entry_node->entry_type = (sai_fdb_entry_type_t)attr->value.s32;
    } else if(attr->id == SAI_FDB_ENTRY_ATTR_PACKET_ACTION) {
        fdb_entry_node->action = (sai_packet_action_t)attr->value.s32;
    } else if (attr->id == SAI_FDB_ENTRY_ATTR_META_DATA) {
        fdb_entry_node->metadata = attr->value.u32;
    } else if (attr->id == SAI_FDB_ENTRY_ATTR_ENDPOINT_IP) {
        fdb_entry_node->end_point_ip = attr->value.ipaddr;
    }
}

sai_status_t sai_is_valid_fdb_attribute_val(const sai_attribute_t *fdb_attr)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;

    STD_ASSERT(fdb_attr != NULL);
    switch(fdb_attr->id) {
        case SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID:
            if(!sai_is_bridge_port_created(fdb_attr->value.oid)) {
                ret_val = SAI_STATUS_INVALID_ATTR_VALUE_0;
                SAI_FDB_LOG_WARN("Invalid attribute value for  bridge port:0x%"PRIx64"",
                                 fdb_attr->value.oid);
            } else {
                ret_val = SAI_STATUS_SUCCESS;
            }
            break;
        case SAI_FDB_ENTRY_ATTR_TYPE:
            if(!((fdb_attr->value.s32 == SAI_FDB_ENTRY_TYPE_STATIC) ||
                (fdb_attr->value.s32 == SAI_FDB_ENTRY_TYPE_DYNAMIC))) {
                ret_val = SAI_STATUS_INVALID_ATTR_VALUE_0;
                SAI_FDB_LOG_WARN("Invalid attribute value for \
                                  attribute:%d", fdb_attr->id);
            }else {
                ret_val = SAI_STATUS_SUCCESS;
            }
            break;
        case SAI_FDB_ENTRY_ATTR_PACKET_ACTION:
            switch(fdb_attr->value.s32){
                case SAI_PACKET_ACTION_FORWARD:
                case SAI_PACKET_ACTION_TRAP:
                case SAI_PACKET_ACTION_LOG:
                case SAI_PACKET_ACTION_DROP:
                    ret_val = SAI_STATUS_SUCCESS;
                    break;
                default:
                SAI_FDB_LOG_WARN("Invalid attribute value for \
                                  attribute:%d value:%d", fdb_attr->id,
                                  (int)fdb_attr->value.s32);
                    ret_val = SAI_STATUS_INVALID_ATTR_VALUE_0;
                    break;
            }
            break;
        case SAI_FDB_ENTRY_ATTR_META_DATA:
            SAI_FDB_LOG_TRACE("FDB Meta Data value %d",
                              fdb_attr->value.u32);
            ret_val = SAI_STATUS_SUCCESS;
            break;
        case SAI_FDB_ENTRY_ATTR_ENDPOINT_IP:
            if((sai_fib_is_ip_addr_zero(&fdb_attr->value.ipaddr)) ||
               (sai_fib_is_ip_addr_loopback(&fdb_attr->value.ipaddr))) {
                SAI_FDB_LOG_ERR("Error invalid ip in attribute");
                ret_val = SAI_STATUS_INVALID_ATTR_VALUE_0;
            } else {
                ret_val = SAI_STATUS_SUCCESS;
            }
            break;
        default:
            ret_val = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            SAI_FDB_LOG_WARN("Unknown attribute %d", fdb_attr->id);
            break;
    }
    return ret_val;
}

