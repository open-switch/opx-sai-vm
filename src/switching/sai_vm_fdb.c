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
 * @file sai_vm_fdb.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI FDB object in VM environment.
 */

#include "sai_vm_defs.h"
#include "sai_switching_db_api.h"
#include "sai_switch_db_api.h"
#include "sai_npu_fdb.h"
#include "sai_fdb_common.h"
#include "sai_fdb_api.h"
#include "sai_vlan_api.h"
#include "sai_switch_common.h"
#include "sai_switch_utils.h"
#include "saifdb.h"
#include "saiswitch.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_mac_utils.h"
#include "std_assert.h"
#include <string.h>
#include <inttypes.h>
#include <stdio.h>

#define BR_PATH_NAME_SZ (256)
#define BR_FLUSH_PATH "/sys/devices/virtual/net/br%u/bridge/flush"

static void bridge_flush(sai_object_id_t bv_id)
{
    // Only for dynamic FDB types, no means to deal with STATIC MAC address entries
    if (bv_id != SAI_NULL_OBJECT_ID) {
        sai_vlan_id_t vlan_id = sai_vlan_obj_id_to_vlan_id(bv_id);
        if(sai_is_valid_vlan_id(vlan_id)) {

            // TODO get_bridge_name using a configurable parameter for bridge names, rather than br<VLANID>
            char name[BR_PATH_NAME_SZ];
            snprintf(name, sizeof(name), BR_FLUSH_PATH, (unsigned int)vlan_id);
            FILE *f=fopen(name, "w");
            if (f != NULL) {
                fprintf(f, "1\n\r");
                fclose(f);
            }
            else {
                SAI_FDB_LOG_ERR ("Cannot open %s errno=%s(%d)", name, strerror(errno), errno);
            }
        }
        else {
            SAI_FDB_LOG_ERR ("Not a valid VLAN, vlan_id = %u",(unsigned int)vlan_id);
        }
    }
}

static sai_status_t sai_npu_fdb_init (void)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_flush_all_fdb_entries (sai_object_id_t bridge_port_id,
                                            sai_object_id_t bv_id, bool delete_all,
                                            sai_fdb_flush_entry_type_t flush_type)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    SAI_FDB_LOG_TRACE ("FDB bulk flush operation, port: "
                       "0x%"PRIx64", vlan/bridge: 0x%"PRIx64" delete all:%d flush type:%d. ",
                       bridge_port_id, bv_id, delete_all, flush_type);
    bridge_flush(bv_id);
    sai_rc = sai_fdb_delete_all_db_entries (bridge_port_id, bv_id, delete_all, flush_type);
    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_FDB_LOG_ERR ("NPU Flush all failed with error %d. ",sai_rc);
    }
    return sai_rc;
}

static sai_status_t sai_npu_flush_fdb_entry (const sai_fdb_entry_t* fdb_entry, bool validate_port)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    char         mac_str [SAI_MAC_STR_LEN] = {0};
    static sai_mac_t null_mac_addr = {0,0,0,0,0,0};

    STD_ASSERT(fdb_entry != NULL);

    SAI_FDB_LOG_TRACE ("FDB Entry flush for MAC: %s vlan/bridge: 0x%"PRIx64"",
                          std_mac_to_string (&(fdb_entry->mac_address),
                                             mac_str, sizeof (mac_str)),
                          fdb_entry->bv_id);

    if (memcmp(fdb_entry->mac_address, null_mac_addr, sizeof(sai_mac_t)) != 0) {
        /* Remove FDB record from DB. */
        sai_rc = sai_fdb_delete_db_entry (fdb_entry);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_FDB_LOG_ERR ("Error removing FDB entry from DB for MAC: %s, "
                            "vlan/bridge: 0x%"PRIx64".", std_mac_to_string
                            (&(fdb_entry->mac_address), mac_str,
                             sizeof (mac_str)), fdb_entry->bv_id);

            return SAI_STATUS_FAILURE;
        }
    }
    else {
        bridge_flush(fdb_entry->bv_id);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_create_fdb_entry (const sai_fdb_entry_t *fdb_entry,
                                              sai_fdb_entry_node_t *fdb_entry_node_data)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    char         mac_str [SAI_MAC_STR_LEN] = {0};

    STD_ASSERT(fdb_entry != NULL);
    STD_ASSERT(fdb_entry_node_data != NULL);

    SAI_FDB_LOG_TRACE ("FDB Entry create for MAC: %s vlan/bridge: 0x%"PRIx64" on bridge port: "
                          "0x%"PRIx64", entry_type: %d, packet action: %d. FDB Meta Data: %d",
                          std_mac_to_string (&(fdb_entry->mac_address),
                                             mac_str, sizeof (mac_str)),
                          fdb_entry->bv_id, fdb_entry_node_data->bridge_port_id,
                          fdb_entry_node_data->entry_type, fdb_entry_node_data->action,
                          fdb_entry_node_data->metadata);

    /* Insert FDB record to DB. */
    sai_rc = sai_fdb_create_db_entry (fdb_entry,fdb_entry_node_data);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_FDB_LOG_ERR ("Error inserting FDB entry to DB for MAC: %s, "
                            "vlan/bridge: 0x%"PRIx64"", std_mac_to_string
                            (&(fdb_entry->mac_address), mac_str,
                             sizeof (mac_str)), fdb_entry->bv_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_get_fdb_entry_from_hardware (
const sai_fdb_entry_t *fdb_entry, sai_fdb_entry_node_t *fdb_entry_node)
{
    STD_ASSERT(fdb_entry != NULL);
    STD_ASSERT(fdb_entry_node != NULL);

    return SAI_STATUS_ADDR_NOT_FOUND;
}

static sai_status_t sai_npu_write_fdb_entry_to_hardware (
sai_fdb_entry_node_t *fdb_entry_node)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    char            mac_str [SAI_MAC_STR_LEN] = {0};
    sai_fdb_entry_t sai_fdb_entry;

    SAI_FDB_LOG_TRACE ("FDB Entry attribute set for MAC: %s vlan/bridge: 0x%"PRIx64"",
                          std_mac_to_string ((const sai_mac_t *)
                           &(fdb_entry_node->fdb_key.mac_address), mac_str,
                           sizeof (mac_str)), fdb_entry_node->fdb_key.bv_id);

    /* Insert FDB record to DB. */
    memcpy (&sai_fdb_entry.mac_address, &(fdb_entry_node->fdb_key.mac_address),
            sizeof (sai_mac_t));

    sai_fdb_entry.bv_id = fdb_entry_node->fdb_key.bv_id;

    /* Update FDB record in DB with attribute set info. */
    sai_rc = sai_fdb_set_db_entry ((const sai_fdb_entry_t *)&sai_fdb_entry,
                                   fdb_entry_node->bridge_port_id,
                                   fdb_entry_node->entry_type,
                                   fdb_entry_node->action,
                                   fdb_entry_node->metadata);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_FDB_LOG_ERR ("Error updating FDB entry to DB for MAC: %s, "
                            "vlan/bridge: 0x%"PRIx64".", std_mac_to_string
                            ((const sai_mac_t *)&(sai_fdb_entry.mac_address),
                             mac_str, sizeof (mac_str)), sai_fdb_entry.bv_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_set_switch_max_learned_address (uint32_t value)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    SAI_FDB_LOG_TRACE ("MAX MAC learn Limit set to %d.", value);

    /* Update the Switch DB entry with this attribute info. */
    attr.id = SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES;
    attr.value.u32 = value;

    sai_rc = sai_switch_attribute_set_db_entry (sai_vm_switch_id_get(), &attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_FDB_LOG_ERR ("Failed to update the switch attribute %d in "
                         "DB table.", attr.id);
    }

    return sai_rc;
}

static sai_status_t sai_npu_get_switch_max_learned_address (uint32_t *value)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get ();

    STD_ASSERT(value != NULL);
    STD_ASSERT(sai_switch_info_ptr != NULL);

    *value = sai_switch_info_ptr->max_mac_learn_limit;

    SAI_FDB_LOG_TRACE ("MAX MAC learn Limit: %d.", *value);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_fdb_ucast_miss_action_set (
const sai_attribute_t *p_attr)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_attr != NULL);

    SAI_FDB_LOG_TRACE ("FDB Unicast miss action set to %d.",
                       p_attr->value.s32);

    /* Update the Switch DB entry with this attribute info. */
    sai_rc = sai_switch_attribute_set_db_entry (sai_vm_switch_id_get(),
                                                p_attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_FDB_LOG_ERR ("Failed to update the switch attribute %d in "
                         "DB table.", p_attr->id);
    }

    return sai_rc;
}

static sai_status_t sai_npu_fdb_ucast_miss_action_get (sai_attribute_t *attr)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get ();

    STD_ASSERT(attr != NULL);
    STD_ASSERT(sai_switch_info_ptr != NULL);

    attr->value.s32 = sai_switch_info_ptr->fdbUcastMissPktAction;

    SAI_FDB_LOG_TRACE ("FDB Unicast miss action: %d.", attr->value.s32);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_fdb_mcast_miss_action_set (
const sai_attribute_t *p_attr)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_attr != NULL);

    SAI_FDB_LOG_TRACE ("FDB Multicast miss action set to %d.",
                       p_attr->value.s32);

    /* Update the Switch DB entry with this attribute info. */
    sai_rc = sai_switch_attribute_set_db_entry (sai_vm_switch_id_get(),
                                                p_attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_FDB_LOG_ERR ("Failed to update the switch attribute %d in "
                         "DB table.", p_attr->id);
    }

    return sai_rc;
}

static sai_status_t sai_npu_fdb_mcast_miss_action_get (sai_attribute_t *attr)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get ();

    STD_ASSERT(attr != NULL);
    STD_ASSERT(sai_switch_info_ptr != NULL);

    attr->value.s32 = sai_switch_info_ptr->fdbMcastMissPktAction;

    SAI_FDB_LOG_TRACE ("FDB Multicast miss action: %d.", attr->value.s32);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_fdb_bcast_miss_action_set (
const sai_attribute_t *p_attr)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_attr != NULL);

    SAI_FDB_LOG_TRACE ("FDB Broadcast miss action set to %d.",
                       p_attr->value.s32);

    /* Update the Switch DB entry with this attribute info. */
    sai_rc = sai_switch_attribute_set_db_entry (sai_vm_switch_id_get(),
                                                p_attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_FDB_LOG_ERR ("Failed to update the switch attribute %d in "
                         "DB table.", p_attr->id);
    }

    return sai_rc;
}

static sai_status_t sai_npu_fdb_bcast_miss_action_get (sai_attribute_t *attr)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get ();

    STD_ASSERT(attr != NULL);
    STD_ASSERT(sai_switch_info_ptr != NULL);

    attr->value.s32 = sai_switch_info_ptr->fdbBcastMissPktAction;

    SAI_FDB_LOG_TRACE ("FDB Broadcast miss action: %d.", attr->value.s32);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_set_aging_time (uint32_t value)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    SAI_FDB_LOG_TRACE ("FDB aging time set to %d.", value);

    /* Update the Switch DB entry with this attribute info. */
    attr.id = SAI_SWITCH_ATTR_FDB_AGING_TIME;
    attr.value.u32 = value;

    sai_rc = sai_switch_attribute_set_db_entry (sai_vm_switch_id_get(), &attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_FDB_LOG_ERR ("Failed to update the switch attribute %d in "
                         "DB table.", attr.id);
    }

    return sai_rc;
}

static sai_status_t sai_npu_get_aging_time (uint32_t *value)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get ();

    STD_ASSERT(value != NULL);
    STD_ASSERT(sai_switch_info_ptr != NULL);

    *value = sai_switch_info_ptr->fdb_aging_time;

    SAI_FDB_LOG_TRACE ("FDB aging time: %d.", *value);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_register_fdb_callback (
sai_fdb_npu_event_notification_fn fdb_notification_fn)
{
    STD_ASSERT(fdb_notification_fn != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_get_fdb_table_size (sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);

    attr->value.u32 = SAI_VM_FDB_TABLE_SIZE;

    SAI_FDB_LOG_TRACE ("FDB Table size: %d.", attr->value.u32);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_bcast_cpu_flood_enable_set (bool enable)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    SAI_FDB_LOG_TRACE ("Broadcast CPU flood enable set to %d.", enable);

    /* Update the Switch DB entry with this attribute info. */
    attr.id = SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE;
    attr.value.booldata = enable;

    sai_rc = sai_switch_attribute_set_db_entry (sai_vm_switch_id_get(), &attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_FDB_LOG_ERR ("Failed to update the switch attribute %d in "
                         "DB table.", attr.id);
    }

    return sai_rc;
}

static sai_status_t sai_npu_mcast_cpu_flood_enable_set (bool enable)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    SAI_FDB_LOG_TRACE ("Multicast CPU flood enable set to %d.", enable);

    /* Update the Switch DB entry with this attribute info. */
    attr.id = SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE;
    attr.value.booldata = enable;

    sai_rc = sai_switch_attribute_set_db_entry (sai_vm_switch_id_get(), &attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_FDB_LOG_ERR ("Failed to update the switch attribute %d in "
                         "DB table.", attr.id);
    }

    return sai_rc;
}

static sai_status_t sai_npu_bcast_cpu_flood_enable_get (bool *enable)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get ();

    STD_ASSERT(enable != NULL);
    STD_ASSERT(sai_switch_info_ptr != NULL);

    *enable = sai_switch_info_ptr->isBcastCpuFloodEnable;

    SAI_FDB_LOG_TRACE ("Broadcast CPU flood enable: %d.", *enable);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_mcast_cpu_flood_enable_get (bool *enable)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get ();

    STD_ASSERT(enable != NULL);
    STD_ASSERT(sai_switch_info_ptr != NULL);

    *enable = sai_switch_info_ptr->isMcastCpuFloodEnable;

    SAI_FDB_LOG_TRACE ("Multicast CPU flood enable: %d.", *enable);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_bridge_port_for_fdb_entry (const sai_fdb_entry_t *fdb_entry,
                                         sai_object_id_t *bridge_port_id)
{
    sai_fdb_entry_node_t fdb_entry_node;
    char                 mac_str [SAI_MAC_STR_LEN] = {0};
    sai_status_t         ret_val = SAI_STATUS_FAILURE;

    STD_ASSERT(fdb_entry != NULL);
    STD_ASSERT(bridge_port_id != NULL);

    sai_fdb_lock ();

    if (sai_fdb_get_bridge_port_from_cache (fdb_entry, bridge_port_id) !=
        SAI_STATUS_SUCCESS) {
        memset (&fdb_entry_node, 0, sizeof(sai_fdb_entry_node_t));

        ret_val =
            sai_npu_get_fdb_entry_from_hardware (fdb_entry, &fdb_entry_node);

        if (ret_val != SAI_STATUS_SUCCESS) {
            SAI_FDB_LOG_TRACE ("FDB Entry not found for MAC: %s vlan/bridge: 0x%"PRIx64"",
                                std_mac_to_string (&(fdb_entry->mac_address),
                                                   mac_str, sizeof (mac_str)),
                                fdb_entry->bv_id);

            sai_fdb_unlock ();

            return ret_val;
        }

        *bridge_port_id = fdb_entry_node.bridge_port_id;

        /* Adding to cache to be used for future calls */
        ret_val = sai_insert_fdb_entry_node (fdb_entry,&fdb_entry_node);

        if (ret_val != SAI_STATUS_SUCCESS) {
            SAI_FDB_LOG_WARN ("Unable to cache FDB entry MAC: %s, vlan/bridge: 0x%"PRIx64"",
                                std_mac_to_string (&(fdb_entry->mac_address),
                                                   mac_str, sizeof(mac_str)),
                                fdb_entry->bv_id);
        }
    }

    sai_fdb_unlock ();

    return SAI_STATUS_SUCCESS;
}

static sai_npu_fdb_api_t sai_vm_fdb_api_table = {

    sai_npu_fdb_init,
    sai_npu_flush_all_fdb_entries,
    sai_npu_flush_fdb_entry,
    sai_npu_create_fdb_entry,
    sai_npu_get_fdb_entry_from_hardware,
    sai_npu_write_fdb_entry_to_hardware,
    sai_npu_set_aging_time,
    sai_npu_get_aging_time,
    sai_npu_get_fdb_table_size,
    sai_npu_register_fdb_callback,
    sai_npu_set_switch_max_learned_address,
    sai_npu_get_switch_max_learned_address,
    sai_npu_fdb_ucast_miss_action_set,
    sai_npu_fdb_ucast_miss_action_get,
    sai_npu_fdb_mcast_miss_action_set,
    sai_npu_fdb_mcast_miss_action_get,
    sai_npu_fdb_bcast_miss_action_set,
    sai_npu_fdb_bcast_miss_action_get,
    sai_npu_bcast_cpu_flood_enable_set,
    sai_npu_mcast_cpu_flood_enable_set,
    sai_npu_bcast_cpu_flood_enable_get,
    sai_npu_mcast_cpu_flood_enable_get
};

sai_npu_fdb_api_t* sai_vm_fdb_api_query (void)
{
    return &sai_vm_fdb_api_table;
}

