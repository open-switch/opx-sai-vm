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
* @file sai_fdb_debug.c
*
* @brief This file contains Debug APIs for SAI FDB module
*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "saifdb.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_radix.h"
#include "sai_fdb_api.h"
#include "sai_fdb_common.h"
#include "sai_debug_utils.h"
#include "std_mac_utils.h"
#include "sai_l3_util.h"

static inline void print_fdb_header(void)
{
    SAI_DEBUG("%-20s %-20s %-20s %-20s %-5s %-5s %-5s","MAC","VLAN/BRIDGE","End point IP",
                                                 "Port","Type","Action","Pending");
    SAI_DEBUG("------------------------------------------------------------");
}

static inline void print_fdb_notification_header(void)
{
    SAI_DEBUG("%-20s %-20s %-20s %-5s %-5s","MAC","VLAN/BRIDGE","Port","InCL","Event");
    SAI_DEBUG("------------------------------------------------------------");
}

void sai_dump_all_fdb_entry_nodes (void)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_key_t   fdb_key;
    char                  mac_str[SAI_MAC_STR_LEN] = {0};
    char                  ip_addr_str[SAI_FIB_MAX_BUFSZ] = {0};

    memset(&fdb_key, 0, sizeof(fdb_key));

    fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);

    print_fdb_header();
    while(fdb_entry_node != NULL) {
        memcpy(&fdb_key,&(fdb_entry_node->fdb_key),
               sizeof(sai_fdb_entry_key_t));
        SAI_DEBUG("%-20s 0x%-20"PRIx64" %-20s 0x%-20"PRIx64" %-5d %-5d %-5d",
                  std_mac_to_string((const sai_mac_t*)&(fdb_entry_node->fdb_key.mac_address),
                  mac_str, sizeof(mac_str)), fdb_entry_node->fdb_key.bv_id,
                  sai_ip_addr_to_str(&fdb_entry_node->end_point_ip, ip_addr_str, SAI_FIB_MAX_BUFSZ),
                  fdb_entry_node->bridge_port_id,fdb_entry_node->entry_type,
                  fdb_entry_node->action,fdb_entry_node->is_pending_entry);
        fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);
    }
}

void sai_dump_all_fdb_entry_count (void)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_key_t fdb_key;
    uint_t count = 0;

    memset(&fdb_key, 0, sizeof(fdb_key));

    fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);

    while(fdb_entry_node != NULL) {
        memcpy(&fdb_key,&(fdb_entry_node->fdb_key),
               sizeof(sai_fdb_entry_key_t));
        fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);
        count++;
    }
    SAI_DEBUG("Number of MAC entries: %d", count);
}
void sai_dump_all_fdb_registered_nodes (void)
{
    sai_fdb_registered_node_t *fdb_registered_node = NULL;
    sai_fdb_entry_key_t fdb_key;
    char mac_str[SAI_MAC_STR_LEN] = {0};

    memset(&fdb_key, 0, sizeof(fdb_key));

    fdb_registered_node = sai_get_next_fdb_registered_node (&fdb_key);

    print_fdb_notification_header();
    while(fdb_registered_node != NULL) {
        memcpy(&fdb_key,&(fdb_registered_node->fdb_key),
               sizeof(sai_fdb_entry_key_t));
            SAI_DEBUG("%-20s 0x%-20"PRIx64" 0x%-20"PRIx64" %-5d %-5d",
                   std_mac_to_string((const sai_mac_t*)&(fdb_registered_node->fdb_key.mac_address),
                                     mac_str, sizeof(mac_str)),
                   fdb_registered_node->fdb_key.bv_id,fdb_registered_node->bridge_port_id,
                   fdb_registered_node->node_in_cl, fdb_registered_node->fdb_event);
            fdb_registered_node = sai_get_next_fdb_registered_node (&fdb_key);
    }
}

void sai_dump_pending_fdb_to_l3_notifs (void)
{
    sai_fdb_registered_node_t *fdb_registered_node = NULL;
    sai_fdb_entry_key_t fdb_key;
    char mac_str[SAI_MAC_STR_LEN] = {0};

    memset(&fdb_key, 0, sizeof(fdb_key));
    fdb_registered_node = sai_get_next_fdb_registered_node (&fdb_key);

    print_fdb_notification_header();
    while(fdb_registered_node != NULL) {
        memcpy(&fdb_key,&(fdb_registered_node->fdb_key),
               sizeof(sai_fdb_entry_key_t));
        if(fdb_registered_node->node_in_cl) {
            SAI_DEBUG("%-20s 0x%-20"PRIx64" 0x%-20"PRIx64" %-5d %-5d",
                    std_mac_to_string((const sai_mac_t*)&(fdb_registered_node->fdb_key.mac_address),
                        mac_str, sizeof(mac_str)),
                    fdb_registered_node->fdb_key.bv_id,fdb_registered_node->bridge_port_id,
                    fdb_registered_node->node_in_cl, fdb_registered_node->fdb_event);
        }
        fdb_registered_node = sai_get_next_fdb_registered_node (&fdb_key);
    }
}

void sai_dump_fdb_entry_nodes_per_bridge_port (sai_object_id_t bridge_port_id)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_key_t   fdb_key;
    char                  mac_str[SAI_MAC_STR_LEN] = {0};
    char                  ip_addr_str[SAI_FIB_MAX_BUFSZ] = {0};

    memset(&fdb_key, 0, sizeof(fdb_key));
    fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);
    print_fdb_header();
    while(fdb_entry_node != NULL) {
        memcpy(&fdb_key,&(fdb_entry_node->fdb_key),
               sizeof(sai_fdb_entry_key_t));
        if(fdb_entry_node->bridge_port_id == bridge_port_id){
            SAI_DEBUG("%-20s 0x%-20"PRIx64" %-20s 0x%-20"PRIx64" %-5d %-5d %-5d",
                      std_mac_to_string((const sai_mac_t*)&(fdb_entry_node->fdb_key.mac_address),
                      mac_str, sizeof(mac_str)), fdb_entry_node->fdb_key.bv_id,
                      sai_ip_addr_to_str(&fdb_entry_node->end_point_ip, ip_addr_str,
                      SAI_FIB_MAX_BUFSZ), fdb_entry_node->bridge_port_id,fdb_entry_node->entry_type,
                      fdb_entry_node->action,fdb_entry_node->is_pending_entry);

        }
        fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);
    }
}

void sai_dump_fdb_entry_nodes_per_vlan (sai_object_id_t bv_id)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_key_t   fdb_key;
    char                  mac_str[SAI_MAC_STR_LEN] = {0};
    char                  ip_addr_str[SAI_FIB_MAX_BUFSZ] = {0};

    memset(&fdb_key, 0, sizeof(fdb_key));
    fdb_key.bv_id = bv_id;

    fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);

    print_fdb_header();
    while(fdb_entry_node != NULL) {
        memcpy(&fdb_key,&(fdb_entry_node->fdb_key),
              sizeof(sai_fdb_entry_key_t));
        if(fdb_key.bv_id != bv_id){
            break;
        }
        SAI_DEBUG("%-20s 0x%-20"PRIx64" %-20s 0x%-20"PRIx64" %-5d %-5d %-5d",
                  std_mac_to_string((const sai_mac_t*)&(fdb_entry_node->fdb_key.mac_address),
                  mac_str, sizeof(mac_str)), fdb_entry_node->fdb_key.bv_id,
                  sai_ip_addr_to_str(&fdb_entry_node->end_point_ip,ip_addr_str,
                  SAI_FIB_MAX_BUFSZ), fdb_entry_node->bridge_port_id,fdb_entry_node->entry_type,
                  fdb_entry_node->action,fdb_entry_node->is_pending_entry);
        fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);
    }
}

void sai_dump_fdb_entry_nodes_per_bridge_port_vlan (sai_object_id_t bridge_port_id,
                                                    sai_object_id_t bv_id)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_key_t   fdb_key;
    char                  mac_str[SAI_MAC_STR_LEN] = {0};
    char                  ip_addr_str[SAI_FIB_MAX_BUFSZ] = {0};

    memset(&fdb_key, 0, sizeof(fdb_key));
    fdb_key.bv_id = bv_id;

    fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);
    print_fdb_header();
    while(fdb_entry_node != NULL) {
        memcpy(&fdb_key,&(fdb_entry_node->fdb_key),
               sizeof(sai_fdb_entry_key_t));
        if(fdb_key.bv_id != bv_id){
            break;
        }
        if((fdb_entry_node->bridge_port_id == bridge_port_id)){
            SAI_DEBUG("%-20s 0x%-20"PRIx64" %-20s 0x%-20"PRIx64" %-5d %-5d %-5d",
                      std_mac_to_string((const sai_mac_t*)&(fdb_entry_node->fdb_key.mac_address),
                      mac_str, sizeof(mac_str)), fdb_entry_node->fdb_key.bv_id,
                      sai_ip_addr_to_str(&fdb_entry_node->end_point_ip, ip_addr_str,
                      SAI_FIB_MAX_BUFSZ), fdb_entry_node->bridge_port_id,fdb_entry_node->entry_type,
                      fdb_entry_node->action,fdb_entry_node->is_pending_entry);
        }
        fdb_entry_node = sai_get_next_fdb_entry_node (&fdb_key);
    }
}
