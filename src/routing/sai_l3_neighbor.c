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
* @file sai_l3_neighbor.c
*
* @brief This file contains function definitions for the SAI neighbor
*        functionality API.
*
*************************************************************************/
#include "sai_l3_api.h"
#include "sai_l3_util.h"
#include "sai_l3_common.h"
#include "saistatus.h"
#include "saineighbor.h"
#include "saitypes.h"
#include "saifdb.h"
#include "std_mac_utils.h"
#include "std_assert.h"

#include "sai_l3_mem.h"
#include "sai_l3_api_utils.h"
#include "sai_common_infra.h"
#include "sai_fdb_main.h"
#include "sai_fdb_common.h"
#include "sai_bridge_api.h"
#include "sai_vlan_api.h"
#include <string.h>
#include <inttypes.h>

static sai_status_t sai_fib_neighbor_mac_entry_remove (sai_fib_nh_t *p_neighbor);

/*
 * This is a helper routine for parsing attribute list input. Caller must
 * recalculate the error code with the attribute's index in the list.
 */
static sai_status_t sai_fib_neighbor_dest_mac_set (sai_fib_nh_t *p_nh_info,
                                          const sai_attribute_value_t *p_value,
                                          bool is_create_req)
{
    char   mac_addr_str [SAI_FIB_MAX_BUFSZ];

    STD_ASSERT (p_value != NULL);

    if (sai_fib_is_mac_address_zero ((const sai_mac_t *) p_value->mac)) {

        SAI_NEIGHBOR_LOG_ERR ("Invalid MAC address.");

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    memcpy (p_nh_info->mac_addr, p_value->mac, HAL_MAC_ADDR_LEN);

    SAI_NEIGHBOR_LOG_TRACE ("SAI set Neighbor MAC address: %s.",
                            std_mac_to_string ((const hal_mac_addr_t *)
                            &p_nh_info->mac_addr, mac_addr_str,
                            SAI_FIB_MAX_BUFSZ));

    return SAI_STATUS_SUCCESS;
}

/*
 * This is a helper routine for parsing attribute list input. Caller must
 * recalculate the error code with the attribute's index in the list.
 */
static sai_status_t sai_fib_neighbor_pkt_action_set (sai_fib_nh_t *p_nh_info,
                                          const sai_attribute_value_t *p_value,
                                          bool is_create_req)
{
    sai_packet_action_t pkt_action;

    STD_ASSERT (p_value != NULL);

    pkt_action = (sai_packet_action_t) p_value->s32;

    if (!(sai_packet_action_validate (pkt_action))) {

        SAI_NEIGHBOR_LOG_ERR ("Invalid Next Hop Packet action: %d.",
                              pkt_action);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_nh_info->packet_action = pkt_action;

    SAI_NEIGHBOR_LOG_TRACE ("SAI set Neighbor packet action: %s.",
                            sai_packet_action_str (pkt_action));

    return SAI_STATUS_SUCCESS;
}

static inline void sai_fib_neighbor_default_attr_set (sai_fib_nh_t *p_nh_node)
{
    /* Neighbor entry default packet action */
    p_nh_node->packet_action = SAI_PACKET_ACTION_FORWARD;

    /* Neighbor entry default no_host_route flag */
    p_nh_node->no_host_route = false;

    p_nh_node->port_id = SAI_NULL_OBJECT_ID;
}

static inline void sai_fib_neighbor_info_reset (sai_fib_nh_t *p_nh_node)
{
    /* Neighbor entry is reset */
    p_nh_node->packet_action = SAI_PACKET_ACTION_TRAP;

    memset (p_nh_node->mac_addr, 0, HAL_MAC_ADDR_LEN);

    /* Neighbor Meta Data */
    p_nh_node->meta_data = 0;

    p_nh_node->port_id = SAI_NULL_OBJECT_ID;

    sai_fib_reset_next_hop_owner (p_nh_node, SAI_FIB_OWNER_NEIGHBOR);
}

static inline bool sai_fib_is_neighbor_pkt_action_attr_set (uint_t attr_flag)
{
    return ((attr_flag & SAI_FIB_NEIGHBOR_PKT_ACTION_ATTR_FLAG) ? true : false);
}

static inline bool sai_fib_is_neighbor_dest_mac_attr_set (uint_t attr_flag)
{
    return ((attr_flag & SAI_FIB_NEIGHBOR_DEST_MAC_ATTR_FLAG) ? true : false);
}

static inline bool sai_fib_is_neighbor_action_forward (
                                   sai_packet_action_t packet_action)
{
    return (packet_action == SAI_PACKET_ACTION_FORWARD ||
            packet_action == SAI_PACKET_ACTION_LOG);
}

static inline void sai_fib_neighbor_entry_log_trace (
                                const sai_neighbor_entry_t *p_neighbor_entry,
                                const char *p_trace_str)
{
    char   ip_addr_str [SAI_FIB_MAX_BUFSZ];

    STD_ASSERT (p_neighbor_entry != NULL);

    SAI_NEIGHBOR_LOG_TRACE ("%s Neighbor entry. IP Address: %s, VR Id: 0x%"PRIx64","
                            " RIF Id: 0x%"PRIx64".",
                            p_trace_str, sai_ip_addr_to_str (
                            &p_neighbor_entry->ip_address, ip_addr_str,
                            SAI_FIB_MAX_BUFSZ), p_neighbor_entry->vr_id,
                            p_neighbor_entry->rif_id);
}

static sai_status_t sai_fib_neighbor_info_fill (sai_fib_nh_t *p_nh_info,
                                              uint_t attr_count,
                                              const sai_attribute_t *attr_list,
                                              bool is_create_req,
                                              uint_t *p_flags)
{
    sai_status_t           status = SAI_STATUS_FAILURE;
    uint_t                 attr_index;
    const sai_attribute_t *p_attr = NULL;
    sai_attribute_t        sai_attr_get_range;

    STD_ASSERT (p_nh_info != NULL);
    STD_ASSERT (p_flags != NULL);
    STD_ASSERT (attr_list != NULL);

    if ((!attr_count)) {

        SAI_NEIGHBOR_LOG_ERR ("SAI Neighbor fill from attr list. Invalid input."
                              " attr_count: %d.", attr_count);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (attr_index = 0; attr_index < attr_count; attr_index++)
    {
        p_attr = &attr_list [attr_index];
        status = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;

        switch (p_attr->id) {

            case SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS:
                status = sai_fib_neighbor_dest_mac_set (p_nh_info,
                                                        &p_attr->value,
                                                        is_create_req);

                (*p_flags) |= SAI_FIB_NEIGHBOR_DEST_MAC_ATTR_FLAG;
                break;

            case SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION:
                status = sai_fib_neighbor_pkt_action_set (p_nh_info,
                                                          &p_attr->value,
                                                          is_create_req);

                (*p_flags) |= SAI_FIB_NEIGHBOR_PKT_ACTION_ATTR_FLAG;
                break;

            case SAI_NEIGHBOR_ENTRY_ATTR_NO_HOST_ROUTE:
                p_nh_info->no_host_route = p_attr->value.booldata;

                SAI_NEIGHBOR_LOG_TRACE ("SAI set Neighbor no host route: %d.",
                                        p_nh_info->no_host_route);

                (*p_flags) |= SAI_FIB_NEIGHBOR_NO_HOST_ROUTE_ATTR_FLAG;
                status      = SAI_STATUS_SUCCESS;
                break;

            case SAI_NEIGHBOR_ENTRY_ATTR_META_DATA:
                memset(&sai_attr_get_range, 0, sizeof(sai_attribute_t));

                sai_attr_get_range.id = SAI_SWITCH_ATTR_NEIGHBOR_DST_USER_META_DATA_RANGE;
                status = sai_acl_npu_api_get()->get_acl_attribute(&sai_attr_get_range);

                if (status != SAI_STATUS_SUCCESS) {
                    SAI_NEIGHBOR_LOG_ERR ("Failed to fetch L3 Neighbor Meta Data range");
                    break;
                }

                /* Validate the L3 Neighbor Meta data range */
                if ((p_attr->value.u32 < sai_attr_get_range.value.u32range.min) ||
                    (p_attr->value.u32 > sai_attr_get_range.value.u32range.max)) {
                    status = SAI_STATUS_INVALID_ATTR_VALUE_0;
                    break;
                }

                p_nh_info->meta_data = p_attr->value.u32;

                SAI_NEIGHBOR_LOG_TRACE ("SAI set Neighbor meta data: %d.",
                                        p_nh_info->meta_data);

                (*p_flags) |= SAI_FIB_NEIGHBOR_META_DATA_ATTR_FLAG;
                status      = SAI_STATUS_SUCCESS;
                break;
            case SAI_NEIGHBOR_ENTRY_ATTR_USER_TRAP_ID:
               status = SAI_STATUS_NOT_SUPPORTED;
               SAI_NEIGHBOR_LOG_ERR ("Unsupported attribute   Id: %d.",p_attr->id);
               break;
            default:
                SAI_NEIGHBOR_LOG_ERR ("Invalid atttribute id: %d.", p_attr->id);
                break;
        }

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NEIGHBOR_LOG_ERR ("Failure in Neighbor attr list. Index: %d, "
                                  "Attribute Id: %d, Error: %d.", attr_index,
                                  p_attr->id, status);

            return (sai_fib_attr_status_code_get (status, attr_index));
        }
    }

    if ((sai_fib_is_neighbor_action_forward (p_nh_info->packet_action))
        && sai_fib_is_mac_address_zero ((const sai_mac_t *) p_nh_info->mac_addr)) {

        /* Mandatory for create */
        SAI_NEIGHBOR_LOG_ERR ("Mandatory Neighbor %s attribute missing.",
                              "DEST-MAC");

        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_neighbor_entry_validate (
                                   const sai_neighbor_entry_t *neighbor_entry)
{
    sai_status_t  status;
    sai_fib_router_interface_t *p_rif_node = NULL;
    sai_fib_vrf_t *p_vrf_node = NULL;

    STD_ASSERT (neighbor_entry != NULL);

    p_rif_node = sai_fib_router_interface_node_get (neighbor_entry->rif_id);
    if (p_rif_node == NULL) {

        SAI_NEIGHBOR_LOG_ERR ("Invalid Neighbor RIF id: 0x%"PRIx64".",
                              neighbor_entry->rif_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    p_vrf_node = sai_fib_vrf_node_get (neighbor_entry->vr_id);

    if (p_vrf_node == NULL) {

        SAI_NEIGHBOR_LOG_ERR ("SAI Neighbor VR Id: 0x%"PRIx64" not found.",
                              neighbor_entry->vr_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (p_rif_node->type == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK) {

        SAI_NEIGHBOR_LOG_ERR ("Loopback RIF id 0x%"PRIx64" is not valid "
                              "in Neighbor entry", neighbor_entry->rif_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    status = sai_fib_next_hop_ip_address_validate (&neighbor_entry->ip_address);

    return status;
}

static sai_status_t sai_fib_neighbor_ip_next_hop_node_key_fill (
                                    const sai_neighbor_entry_t *neighbor_entry,
                                    sai_fib_nh_key_t *p_nh_key)
{
    sai_status_t      status;
    sai_ip_address_t *p_ip_addr = NULL;

    memset (p_nh_key, 0, sizeof (sai_fib_nh_key_t));

    p_nh_key->nh_type  = SAI_NEXT_HOP_TYPE_IP;
    p_nh_key->rif_id   = neighbor_entry->rif_id;
    p_nh_key->tunnel_type = SAI_FIB_TUNNEL_TYPE_NONE;
    p_ip_addr = &p_nh_key->info.ip_nh.ip_addr;

    status = sai_fib_ip_addr_copy (p_ip_addr, &neighbor_entry->ip_address);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_NEIGHBOR_LOG_ERR ("Failed to copy Neighbor IP address value.");
    }

    return status;
}

static inline void sai_fib_neighbor_attr_copy (sai_fib_nh_t *p_dest_nh_info,
                                               sai_fib_nh_t *p_src_nh_info)
{
    memcpy (p_dest_nh_info->mac_addr, p_src_nh_info->mac_addr,
            HAL_MAC_ADDR_LEN);

    p_dest_nh_info->packet_action = p_src_nh_info->packet_action;
}

static sai_status_t sai_fib_neighbor_key_validate_and_fill (
                                    const sai_neighbor_entry_t *neighbor_entry,
                                    sai_fib_nh_t *p_neighbor_info)
{
    sai_status_t     status;
    sai_fib_vrf_t   *p_vrf_node = NULL;

    STD_ASSERT (p_neighbor_info != NULL);

    /* Validate the input neighbor entry key */
    status = sai_fib_neighbor_entry_validate (neighbor_entry);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_NEIGHBOR_LOG_ERR ("SAI Neighbor entry validation failed.");
        return status;
    }

    /* Validate and fill the VRF Id */
    p_vrf_node = sai_fib_vrf_node_get (neighbor_entry->vr_id);

    if ((!p_vrf_node)) {

        SAI_NEIGHBOR_LOG_ERR ("SAI Neighbor VR Id: 0x%"PRIx64", VRF node not found.",
                              neighbor_entry->vr_id);

        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    p_neighbor_info->vrf_id = p_vrf_node->vrf_id;

    sai_fib_neighbor_ip_next_hop_node_key_fill (neighbor_entry,
                                                &p_neighbor_info->key);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_neighbor_port_id_resolve (sai_fib_nh_t *p_neighbor)
{
    sai_fib_router_interface_t *p_rif = NULL;
    sai_fdb_entry_t             fdb_entry;
    sai_status_t                status;
    const uint_t                attr_count = 1;
    sai_attribute_t             fdb_port_attr;
    char                        mac_str[SAI_FIB_MAX_BUFSZ] = {0};
    sai_object_id_t             port_obj_id = SAI_NULL_OBJECT_ID;

    STD_ASSERT (p_neighbor != NULL);

    p_rif = sai_fib_router_interface_node_get (p_neighbor->key.rif_id);

    if ((!p_rif)) {

        SAI_NEIGHBOR_LOG_ERR ("RIF node not found for RIF Id: 0x%"PRIx64".",
                              p_neighbor->key.rif_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (!sai_fib_is_neighbor_action_forward (p_neighbor->packet_action)) {

        SAI_NEIGHBOR_LOG_TRACE ("Neighbor action not set to FORWARD.");

        return SAI_STATUS_SUCCESS;
    }

    if (p_rif->type == SAI_ROUTER_INTERFACE_TYPE_PORT) {

        p_neighbor->port_id = p_rif->attachment.port_id;

    } else {

        memset (&fdb_entry, 0, sizeof(fdb_entry));
        if(p_rif->type == SAI_ROUTER_INTERFACE_TYPE_BRIDGE) {
            fdb_entry.bv_id = p_rif->attachment.bridge_id;
        } else {
            fdb_entry.bv_id = sai_vlan_id_to_vlan_obj_id(p_rif->attachment.vlan_id);
        }

        memcpy (fdb_entry.mac_address, p_neighbor->mac_addr, sizeof (sai_mac_t));

        fdb_port_attr.id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID;
        status = sai_fdb_api_query()->get_fdb_entry_attribute (&fdb_entry,
                                         attr_count, &fdb_port_attr);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NEIGHBOR_LOG_TRACE ("Failure to get port id for Neighbor from "
                                    "L2 FDB entry mac: %s, vlan 0x%"PRIx64". Status code %d.",
                                    std_mac_to_string((const hal_mac_addr_t *)&(fdb_entry.mac_address),
                                    mac_str, sizeof(mac_str)), fdb_entry.bv_id, status);
            /*Set port as unresolved so that a blackhole egress object is created*/
            p_neighbor->port_unresolved = true;
            p_neighbor->port_id = SAI_NULL_OBJECT_ID;

        } else {
            sai_bridge_lock();
            status = sai_bridge_port_get_port_id(fdb_port_attr.value.oid, &port_obj_id);
            sai_bridge_unlock();
            if(status != SAI_STATUS_SUCCESS) {
                SAI_NEIGHBOR_LOG_ERR ("Error %d in getting port obj from bridge port id "
                                      "0x%"PRIx64" for mac %s, vlan 0x%"PRIx64".", status,
                                      fdb_port_attr.value.oid, std_mac_to_string(
                                      (const hal_mac_addr_t *)&(fdb_entry.mac_address),
                                       mac_str, sizeof(mac_str)), fdb_entry.bv_id);
                return status;
            }
            p_neighbor->port_unresolved = false;
            p_neighbor->port_id = port_obj_id;
        }
        SAI_NEIGHBOR_LOG_TRACE ("Resolved port id 0x%"PRIx64" for Neighbor "
                                "from L2 FDB", p_neighbor->port_id);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_neighbor_fdb_entry_reg (
                                  sai_fib_neighbor_mac_entry_t *p_mac_entry)
{
    sai_fdb_entry_t  fdb_entry;

    fdb_entry.bv_id = sai_vlan_id_to_vlan_obj_id(p_mac_entry->key.vlan_id);

    memcpy (fdb_entry.mac_address, p_mac_entry->key.mac_addr, HAL_MAC_ADDR_LEN);

    return (sai_l2_register_fdb_entry (&fdb_entry));
}

static sai_status_t sai_fib_neighbor_mac_entry_insert (
                                              sai_fib_nh_t *p_neighbor)
{
    sai_fib_router_interface_t       *p_rif_node = NULL;
    std_rt_head                      *p_radix_head = NULL;
    sai_fib_neighbor_mac_entry_t     *p_mac_entry = NULL;
    sai_fib_neighbor_mac_entry_key_t  key;
    bool                              new_mac_entry = false;
    sai_status_t                      status;

    STD_ASSERT (p_neighbor != NULL);

    p_rif_node = sai_fib_router_interface_node_get (p_neighbor->key.rif_id);

    if (p_rif_node == NULL) {
        SAI_NEIGHBOR_LOG_ERR ("Neighbor RIF Id: 0x%"PRIx64" not found.",
                              p_neighbor->key.rif_id);

        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if ((p_rif_node->type != SAI_ROUTER_INTERFACE_TYPE_VLAN) &&
        (p_rif_node->type != SAI_ROUTER_INTERFACE_TYPE_BRIDGE)) {
        SAI_NEIGHBOR_LOG_TRACE ("Neighbor is not on VLAN or Bridge based RIF.");

        return SAI_STATUS_SUCCESS;
    }

    if (!sai_fib_is_neighbor_action_forward (p_neighbor->packet_action)) {
        SAI_NEIGHBOR_LOG_TRACE ("Neighbor action not set to FORWARD.");

        return SAI_STATUS_SUCCESS;
    }

    memset (&key, 0, sizeof (sai_fib_neighbor_mac_entry_key_t));

    /* Find if the neighbor's mac entry is known already */
    key.vlan_id = p_rif_node->attachment.vlan_id;
    memcpy (key.mac_addr, p_neighbor->mac_addr, HAL_MAC_ADDR_LEN);

    p_mac_entry = sai_fib_neighbor_mac_entry_find (&key);

    if (p_mac_entry == NULL) {

        SAI_NEIGHBOR_LOG_TRACE ("Creating Neighbor MAC entry node.");

        /* Create the Neighbor mac entry node */
        p_mac_entry = sai_fib_neighbor_mac_entry_node_alloc ();

        if (p_mac_entry == NULL) {
            SAI_NEIGHBOR_LOG_ERR ("Failed to allocate Neighbor MAC entry node");

            return SAI_STATUS_NO_MEMORY;
        }

        memcpy (&p_mac_entry->key, &key, sizeof (sai_fib_neighbor_mac_entry_key_t));

        /* Initialize the Neighbor list */
        std_dll_init (&p_mac_entry->neighbor_list);

        /* Insert the MAC entry in tree */
        p_mac_entry->rt_head.rth_addr = (uint8_t *)(&p_mac_entry->key);

        p_radix_head = std_radix_insert (
                             sai_fib_access_global_config()->neighbor_mac_tree,
                             &p_mac_entry->rt_head,
                             SAI_FIB_NEIGHBOR_MAC_ENTRY_TREE_KEY_LEN);

        if (p_radix_head == NULL) {

            SAI_NEIGHBOR_LOG_ERR ("Failed to insert MAC entry node in tree.");

            sai_fib_neighbor_mac_entry_node_free (p_mac_entry);

            return SAI_STATUS_FAILURE;
        }

        new_mac_entry = true;
    }

    /* Insert the Neighbor node in the MAC entry's list */
    std_dll_insertatback (&p_mac_entry->neighbor_list,
                          &p_neighbor->mac_entry_link);

    /* Register with L2 module for any changes to this MAC entry */
    if (new_mac_entry) {

        status = sai_fib_neighbor_fdb_entry_reg (p_mac_entry);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NEIGHBOR_LOG_ERR ("Failed to register MAC entry for internal "
                                  "FDB event notifications.");

            sai_fib_neighbor_mac_entry_remove (p_neighbor);

            return status;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_neighbor_fdb_entry_dereg (
                                  sai_fib_neighbor_mac_entry_t *p_mac_entry)
{
    sai_fdb_entry_t  fdb_entry;

    fdb_entry.bv_id = sai_vlan_id_to_vlan_obj_id(p_mac_entry->key.vlan_id);

    memcpy (fdb_entry.mac_address, p_mac_entry->key.mac_addr, HAL_MAC_ADDR_LEN);

    return (sai_l2_deregister_fdb_entry (&fdb_entry));
}

static sai_status_t sai_fib_neighbor_mac_entry_remove (sai_fib_nh_t *p_neighbor)
{
    sai_fib_router_interface_t       *p_rif_node = NULL;
    sai_fib_neighbor_mac_entry_t     *p_mac_entry = NULL;
    sai_fib_neighbor_mac_entry_key_t  key;

    STD_ASSERT (p_neighbor != NULL);

    p_rif_node = sai_fib_router_interface_node_get (p_neighbor->key.rif_id);

    if (p_rif_node == NULL) {
        SAI_NEIGHBOR_LOG_ERR ("Neighbor RIF Id: 0x%"PRIx64" not found.",
                              p_neighbor->key.rif_id);

        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if ((p_rif_node->type != SAI_ROUTER_INTERFACE_TYPE_VLAN) &&
        (p_rif_node->type != SAI_ROUTER_INTERFACE_TYPE_BRIDGE)) {

        SAI_NEIGHBOR_LOG_TRACE ("Neighbor is not on VLAN or Bridge based RIF.");

        return SAI_STATUS_SUCCESS;
    }

    if (!sai_fib_is_neighbor_action_forward (p_neighbor->packet_action)) {
        SAI_NEIGHBOR_LOG_TRACE ("Neighbor action not set to FORWARD.");

        return SAI_STATUS_SUCCESS;
    }

    memset (&key, 0, sizeof (sai_fib_neighbor_mac_entry_key_t));

    /* Find if the neighbor's mac entry is known already */
    key.vlan_id = p_rif_node->attachment.vlan_id;
    memcpy (key.mac_addr, p_neighbor->mac_addr, HAL_MAC_ADDR_LEN);

    p_mac_entry = sai_fib_neighbor_mac_entry_find (&key);

    if (p_mac_entry == NULL) {
        SAI_NEIGHBOR_LOG_TRACE ("Neighbor MAC entry node not found.");

        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Remove the Neighbor node from the MAC entry's list */
    std_dll_remove (&p_mac_entry->neighbor_list,
                    &p_neighbor->mac_entry_link);

    /* Free the MAC entry if there is no other neighbor in list */
    if (std_dll_getfirst (&p_mac_entry->neighbor_list) == NULL) {

        /* Remove the MAC entry from tree */
        std_radix_remove (sai_fib_access_global_config()->neighbor_mac_tree,
                          &p_mac_entry->rt_head);

        /* Deregister the MAC entry */
        sai_fib_neighbor_fdb_entry_dereg (p_mac_entry);

        sai_fib_neighbor_mac_entry_node_free (p_mac_entry);

        p_mac_entry = NULL;

        SAI_NEIGHBOR_LOG_TRACE ("Freed Neighbor MAC entry node.");
    }

    return SAI_STATUS_SUCCESS;
}

static sai_fib_nh_t* sai_fib_neighbor_node_get (sai_object_id_t vr_id,
                                                sai_fib_nh_key_t *nh_key)
{
    sai_fib_vrf_t *p_vrf_node = sai_fib_vrf_node_get (vr_id);

    if (p_vrf_node != NULL) {

        return ((sai_fib_nh_t *)
                std_radix_getexact (p_vrf_node->sai_nh_tree,
                                    (uint8_t *) nh_key,
                                    SAI_FIB_NH_IP_ADDR_TREE_KEY_LEN));
    }

    return NULL;
}

static sai_status_t sai_fib_neighbor_node_create (sai_fib_nh_t *nh_info,
                                                  sai_fib_nh_t **node)
{
    sai_fib_nh_t *p_nh_node = NULL;
    sai_status_t  status;

    STD_ASSERT (nh_info != NULL);
    STD_ASSERT (node != NULL);

    /* Check if there is an existing next hop node */
    p_nh_node = sai_fib_neighbor_node_get (nh_info->vrf_id, &nh_info->key);
    if (p_nh_node) {

        if (sai_fib_is_owner_neighbor (p_nh_node)) {

            sai_fib_next_hop_log_error (p_nh_node, "Neighbor already exists.");

            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }

        sai_fib_neighbor_attr_copy (p_nh_node, nh_info);

    } else {

        /* Create the new IP next hop node */
        p_nh_node = sai_fib_ip_next_hop_node_create (nh_info->vrf_id,
                                                     nh_info, &status);
        if (p_nh_node == NULL) {

            SAI_NEIGHBOR_LOG_ERR ("SAI Neighbor creation. Error in "
                                  "creating IP next hop node.");
            return status;
        }
    }

    sai_fib_set_next_hop_owner (p_nh_node, SAI_FIB_OWNER_NEIGHBOR);

    *node = p_nh_node;

    return SAI_STATUS_SUCCESS;
}

/* Neighbor IPv4 address is expected in Network Byte Order */
static sai_status_t sai_fib_neighbor_create (
                                    const sai_neighbor_entry_t *neighbor_entry,
                                    uint32_t attr_count,
                                    const sai_attribute_t *attr_list)
{
    sai_status_t       status = SAI_STATUS_FAILURE;
    sai_fib_nh_t      *p_nh_node = NULL;
    sai_fib_nh_t       nh_info;
    uint_t             attr_flag = 0;
    bool               mac_inserted = false;

    SAI_NEIGHBOR_LOG_TRACE ("SAI Neighbor creation.");

    memset (&nh_info, 0, sizeof (sai_fib_nh_t));

    sai_fib_neighbor_default_attr_set (&nh_info);

    sai_fib_lock ();

    do {
        /* Validate the input neighbor entry key */
        status = sai_fib_neighbor_key_validate_and_fill (neighbor_entry,
                                                         &nh_info);
        if (status != SAI_STATUS_SUCCESS) {

            SAI_NEIGHBOR_LOG_ERR ("SAI Neighbor creation. Error in "
                                  "neighbor key.");

            break;
        }

        sai_fib_neighbor_entry_log_trace (neighbor_entry, "SAI Neighbor creation.");

        /* Parse and fill from the input attribute list */
        status = sai_fib_neighbor_info_fill (&nh_info, attr_count, attr_list,
                                             true, &attr_flag);
        if (status != SAI_STATUS_SUCCESS) {

            SAI_NEIGHBOR_LOG_ERR ("SAI Neighbor creation. Error in attribute"
                                  " list.");

            break;
        }

        /* Create the neighbor entry node */
        status = sai_fib_neighbor_node_create (&nh_info, &p_nh_node);

        if (status != SAI_STATUS_SUCCESS) {
            break;
        }

        /* Insert the Neighbor in a MAC based view to handle FDB events */
        status = sai_fib_neighbor_mac_entry_insert (p_nh_node);

        if (status != SAI_STATUS_SUCCESS) {
            break;
        }

        mac_inserted = true;

        /* Resolve Egress port Id for the Neighbor */
        status = sai_fib_neighbor_port_id_resolve (p_nh_node);

        if (status != SAI_STATUS_SUCCESS) {
            break;
        }
        /* Create the neighbor entry in NPU with filled attributes */
        status = sai_neighbor_npu_api_get()->neighbor_create (p_nh_node);

        if (status != SAI_STATUS_SUCCESS) {
            break;
        }
    } while (0);

    if (status == SAI_STATUS_SUCCESS) {

        sai_fib_next_hop_log_trace (p_nh_node, "Next Hop node after neighbor "
                                    "entry creation.");
        SAI_NEIGHBOR_LOG_INFO ("Neighbor entry created.");

        sai_fib_neighbor_affected_encap_nh_resolve (p_nh_node, SAI_OP_CREATE);

    } else {

        SAI_NEIGHBOR_LOG_TRACE ("SAI Neighbor entry creation failed.");

        if (p_nh_node != NULL) {

            if (mac_inserted) {
                sai_fib_neighbor_mac_entry_remove (p_nh_node);
            }
            sai_fib_neighbor_info_reset (p_nh_node);

            sai_fib_check_and_delete_ip_next_hop_node (p_nh_node->vrf_id,
                                                       p_nh_node);
        }
    }

    sai_fib_unlock ();

    return status;
}

/* Neighbor IPv4 address is expected in Network Byte Order */
static sai_status_t sai_fib_neighbor_remove (
                                const sai_neighbor_entry_t *neighbor_entry)
{
    sai_status_t       status = SAI_STATUS_FAILURE;
    sai_fib_nh_t      *p_nh_node = NULL;
    sai_fib_nh_t       nh_node_copy;
    sai_fib_nh_key_t   nh_key;

    SAI_NEIGHBOR_LOG_TRACE ("SAI Neighbor remove.");

    sai_fib_lock ();

    do {
        /* Validate the input neighbor entry key */
        status = sai_fib_neighbor_entry_validate (neighbor_entry);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NEIGHBOR_LOG_ERR ("SAI Neighbor entry validation failed.");

            break;
        }

        sai_fib_neighbor_entry_log_trace (neighbor_entry, "SAI Neighbor remove.");

        /* Fill the neighbor IP address key */
        sai_fib_neighbor_ip_next_hop_node_key_fill (neighbor_entry, &nh_key);

        /* Get the neighbor node */
        p_nh_node = sai_fib_neighbor_node_get (neighbor_entry->vr_id, &nh_key);

        if ((!p_nh_node)) {

            SAI_NEIGHBOR_LOG_ERR ("SAI Neighbor removal. Neighbor not found.");

            status = SAI_STATUS_ITEM_NOT_FOUND;

            break;
        }

        if ((!sai_fib_is_owner_neighbor (p_nh_node))) {

            sai_fib_next_hop_log_error (p_nh_node, "Neighbor not found.");

            status = SAI_STATUS_ITEM_NOT_FOUND;

            break;
        }

        /* Copy the next hop node info */
        memcpy (&nh_node_copy, p_nh_node, sizeof (sai_fib_nh_t));

        status = sai_fib_neighbor_mac_entry_remove (p_nh_node);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NEIGHBOR_LOG_ERR ("Failed to remove Neighbor MAC entry node.");

            break;
        }

        /* Reset the neighbor attributes in next hop node */
        sai_fib_neighbor_info_reset (p_nh_node);

        /* Remove it in hardware */
        status = sai_neighbor_npu_api_get()->neighbor_remove (p_nh_node);

        if (status != SAI_STATUS_SUCCESS) {

            memcpy (p_nh_node, &nh_node_copy, sizeof (sai_fib_nh_t));

            sai_fib_neighbor_mac_entry_insert (p_nh_node);

            sai_fib_next_hop_log_error (p_nh_node, "Failed to remove Neighbor "
                                        "entry in NPU.");
            break;
        }
    } while (0);

    if (status == SAI_STATUS_SUCCESS) {

        sai_fib_next_hop_log_trace (p_nh_node, "Next Hop node after "
                                    "neighbor entry deletion.");

        sai_fib_neighbor_affected_encap_nh_resolve (p_nh_node, SAI_OP_REMOVE);

        /* Free the next hop node */
        sai_fib_check_and_delete_ip_next_hop_node (p_nh_node->vrf_id, p_nh_node);

        SAI_NEIGHBOR_LOG_INFO ("Neighbor entry removed.");
    }

    sai_fib_unlock ();

    return status;
}

static sai_status_t sai_fib_neighbor_attribute_set (
                                   const sai_neighbor_entry_t *neighbor_entry,
                                   const sai_attribute_t *p_attr)
{
    sai_status_t       status = SAI_STATUS_FAILURE;
    sai_fib_nh_t       nh_node_copy;
    sai_fib_nh_t      *p_nh_node = NULL;
    uint_t             attr_count = 1;
    sai_fib_nh_key_t   nh_key;
    uint_t             attr_flag = 0;
    bool               is_mac_entry_resolved = false;

    SAI_NEIGHBOR_LOG_TRACE ("SAI Neighbor Set Attribute, attr_count: %d.",
                            attr_count);

    sai_fib_lock ();

    do {
        /* Validate the input neighbor entry key */
        status = sai_fib_neighbor_entry_validate (neighbor_entry);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NEIGHBOR_LOG_ERR ("SAI Neighbor entry validation failed.");

            break;
        }

        sai_fib_neighbor_entry_log_trace (neighbor_entry, "SAI Neighbor "
                                          "Set Attribute.");

        /* Fill the neighbor IP address key */
        sai_fib_neighbor_ip_next_hop_node_key_fill (neighbor_entry, &nh_key);

        /* Get the neighbor node */
        p_nh_node = sai_fib_neighbor_node_get (neighbor_entry->vr_id, &nh_key);

        if ((!p_nh_node)) {

            SAI_NEIGHBOR_LOG_ERR ("SAI Neighbor Set Attribute. "
                                  "Neighbor not found.");

            status = SAI_STATUS_ITEM_NOT_FOUND;

            break;
        }

        if ((!sai_fib_is_owner_neighbor (p_nh_node))) {

            sai_fib_next_hop_log_error (p_nh_node, "SAI Neighbor Set Attribute."
                                        "Neighbor not found.");

            status = SAI_STATUS_ITEM_NOT_FOUND;

            break;
        }

        sai_fib_next_hop_log_trace (p_nh_node, "Neighbor node info before "
                                    "set attribute.");

        memcpy (&nh_node_copy, p_nh_node, sizeof (sai_fib_nh_t));

        /* Parse and fill from the input attribute list */
        status = sai_fib_neighbor_info_fill (&nh_node_copy, attr_count, p_attr,
                                             false, &attr_flag);

        if (status != SAI_STATUS_SUCCESS) {

            break;
        }

        if ((attr_flag & SAI_FIB_NEIGHBOR_DEST_MAC_ATTR_FLAG) ||
            ((attr_flag & SAI_FIB_NEIGHBOR_PKT_ACTION_ATTR_FLAG) &&
             (!sai_fib_is_neighbor_action_forward (p_nh_node->packet_action)))) {

            /* Resolve the egress port for the neighbor's new mac entry */
            status = sai_fib_neighbor_port_id_resolve (&nh_node_copy);

            if (status != SAI_STATUS_SUCCESS) {
                break;
            }

            is_mac_entry_resolved = true;
        }

        /* Set the neighbor attribute in NPU */
        status = sai_neighbor_npu_api_get()->neighbor_attr_set (&nh_node_copy, attr_flag);

        if (status != SAI_STATUS_SUCCESS) {

            sai_fib_next_hop_log_error (p_nh_node, "Failed to set neighbor "
                                        "attribute in NPU.");
            break;
        }

        sai_fib_neighbor_dep_encap_nh_list_update (p_nh_node, &nh_node_copy,
                                                   attr_flag);
    } while (0);

    if (status == SAI_STATUS_SUCCESS) {

        if (is_mac_entry_resolved) {
            /* Remove the neighbor from old mac entry */
            sai_fib_neighbor_mac_entry_remove (p_nh_node);
        }

        /* Update the NH node */
        memcpy (p_nh_node, &nh_node_copy, sizeof (sai_fib_nh_t));

        if (is_mac_entry_resolved) {
            /* Insert the neighbor in new mac entry */
            sai_fib_neighbor_mac_entry_insert (p_nh_node);
        }

        sai_fib_next_hop_log_trace (p_nh_node, "Neighbor set attribute "
                                    "success.");
    }

    sai_fib_unlock ();

    return status;
}

static sai_status_t sai_fib_neighbor_attribute_get (
                                    const sai_neighbor_entry_t* neighbor_entry,
                                    uint32_t attr_count,
                                    sai_attribute_t *attr_list)
{
    sai_status_t       status = SAI_STATUS_FAILURE;
    sai_fib_nh_t      *p_nh_node = NULL;
    sai_fib_nh_key_t   nh_key;

    SAI_NEIGHBOR_LOG_TRACE ("SAI Neighbor Get Attribute, attr_count: %d.",
                            attr_count);

    if ((!attr_count)) {

        SAI_NEIGHBOR_LOG_ERR ("SAI Neighbor Get attribute. Invalid input."
                              " attr_count: %d.", attr_count);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT (neighbor_entry != NULL);
    STD_ASSERT (attr_list != NULL);

    sai_fib_lock ();

    do {
        sai_fib_neighbor_entry_log_trace (neighbor_entry, "SAI Neighbor "
                                          "Get Attribute.");

        /* Fill the neighbor IP address key */
        sai_fib_neighbor_ip_next_hop_node_key_fill (neighbor_entry, &nh_key);

        /* Get the neighbor node */
        p_nh_node = sai_fib_neighbor_node_get (neighbor_entry->vr_id, &nh_key);
        if ((!p_nh_node)) {

            SAI_NEIGHBOR_LOG_ERR ("SAI Neighbor Get Attribute. "
                                  "Neighbor not found.");

            status = SAI_STATUS_ITEM_NOT_FOUND;

            break;
        }

        if ((!sai_fib_is_owner_neighbor (p_nh_node))) {

            sai_fib_next_hop_log_error (p_nh_node, "SAI Neighbor Get Attribute."
                                        "Neighbor not found.");

            status = SAI_STATUS_ITEM_NOT_FOUND;

            break;
        }

        /* Get the neighbor attributes from NPU */
        status = sai_neighbor_npu_api_get()->neighbor_attr_get (p_nh_node, attr_count,
                                                 attr_list);

        if (status != SAI_STATUS_SUCCESS) {

            sai_fib_next_hop_log_error (p_nh_node, "Failed to get Neighbor "
                                        "attributes from NPU.");
            break;
        }
    } while (0);

    if (status == SAI_STATUS_SUCCESS) {

        sai_fib_next_hop_log_trace (p_nh_node, "SAI Neighbor Get Attribute "
                                    "success.");
    } else {

        SAI_NEIGHBOR_LOG_ERR ("SAI Neighbor Get Attribute failed.");
    }

    sai_fib_unlock ();

    return status;
}

static sai_status_t sai_fib_neighbor_port_id_set (const sai_fdb_entry_t *fdb_entry,
                                                  sai_object_id_t bridge_port_id)
{
    sai_fib_neighbor_mac_entry_t     *p_mac_entry = NULL;
    sai_fib_neighbor_mac_entry_key_t  key;
    sai_object_id_t                   prev_port_id;
    sai_status_t                      status = SAI_STATUS_SUCCESS;
    sai_fib_nh_t                     *p_nh_node = NULL;
    sai_fib_nh_t                      nh_node_copy;
    sai_object_id_t                   port_id;

    STD_ASSERT (fdb_entry != NULL);


    /* Check if the MAC entry is present in Neighbor MAC entry tree */
    memset (&key, 0, sizeof (sai_fib_neighbor_mac_entry_key_t));

    key.vlan_id = sai_vlan_obj_id_to_vlan_id(fdb_entry->bv_id);
    memcpy (key.mac_addr, fdb_entry->mac_address, sizeof (sai_mac_t));

    sai_fib_lock ();

    do {
        /*This is to create a black hole egress object*/
        if(bridge_port_id == SAI_NULL_OBJECT_ID) {
            port_id = SAI_NULL_OBJECT_ID;
        } else {
            sai_bridge_lock();
            status = sai_bridge_port_get_port_id(bridge_port_id, &port_id);
            sai_bridge_unlock();
            if(status != SAI_STATUS_SUCCESS) {
                SAI_NEIGHBOR_LOG_ERR ("Error %d in getting port obj from bridge port id "
                        "0x%"PRIx64"", status, bridge_port_id);
                break;
            }
        }

        p_mac_entry = sai_fib_neighbor_mac_entry_find (&key);

        if (p_mac_entry == NULL) {

            SAI_NEIGHBOR_LOG_TRACE ("MAC entry not present in Neighbor.");

            break;
        }

        for (p_nh_node = sai_fib_get_first_neighbor_from_mac_entry (p_mac_entry);
             p_nh_node != NULL;
             p_nh_node = sai_fib_get_next_neighbor_from_mac_entry (p_mac_entry,
                                                                   p_nh_node))
        {
             if (p_nh_node->port_id == port_id) {

                sai_fib_next_hop_log_trace (p_nh_node, "No change in port.");

                continue;
             }

             memcpy (&nh_node_copy, p_nh_node, sizeof (sai_fib_nh_t));

             prev_port_id = p_nh_node->port_id;
             p_nh_node->port_id = port_id;
             if(port_id == SAI_NULL_OBJECT_ID) {
                 /* If FDB delete notification comes black hole the egress
                    until the FDB learn notification comes */
                 p_nh_node->port_unresolved = true;
                 status = sai_neighbor_npu_api_get()->neighbor_attr_set (p_nh_node,
                                                      SAI_FIB_NEIGHBOR_PKT_ACTION_ATTR_FLAG);
             } else {
                 if(p_nh_node->port_unresolved) {
                     /*  If port was unresolved earlier and resolved now through
                      *  FDB callback update MAC and PACKET ACTION flag to create
                      *  new egress object
                      */
                     p_nh_node->port_unresolved = false;
                     status = sai_neighbor_npu_api_get()->neighbor_attr_set (p_nh_node,
                                                          (SAI_FIB_NEIGHBOR_DEST_MAC_ATTR_FLAG |
                                                           SAI_FIB_NEIGHBOR_PKT_ACTION_ATTR_FLAG));
                 } else {
                     status = sai_neighbor_npu_api_get()->neighbor_attr_set (p_nh_node,
                             SAI_FIB_NEIGHBOR_PORT_ID_ATTR_FLAG);
                 }
             }
             /* Set the port id for Neighbor in NPU */

             if (status != SAI_STATUS_SUCCESS) {

                 sai_fib_next_hop_log_error (p_nh_node, "Failed to set port id"
                                             "for Neighbor in NPU.");
                 p_nh_node->port_id = prev_port_id;

             } else {

                sai_fib_next_hop_log_trace (p_nh_node, "Modified Neighbor "
                                            "port id for FDB event.");
             }

             sai_fib_neighbor_dep_encap_nh_list_update (p_nh_node, &nh_node_copy,
                                             SAI_FIB_NEIGHBOR_PORT_ID_ATTR_FLAG);
        }
    } while (0);

    sai_fib_unlock ();

    return status;
}

sai_status_t sai_neighbor_fdb_callback (uint_t num_upd,
                                        sai_fdb_internal_notification_data_t *fdb_upd)
{
    uint_t            count;
    sai_object_id_t   bridge_port_id = SAI_NULL_OBJECT_ID;

    STD_ASSERT (fdb_upd != NULL);

    SAI_NEIGHBOR_LOG_TRACE ("Handling L2 FDB event callback. Num updates: %u.",
                            num_upd);

    for (count = 0; count < num_upd; ++count) {
        bridge_port_id = SAI_NULL_OBJECT_ID;
        if (fdb_upd [count].fdb_event == SAI_FDB_EVENT_LEARNED) {
            bridge_port_id = fdb_upd[count].bridge_port_id;
        }
        sai_fib_neighbor_port_id_set (&fdb_upd [count].fdb_entry, bridge_port_id);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_neighbor_remove_all_entries (sai_object_id_t switch_id)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

static sai_neighbor_api_t sai_neighbor_method_table = {
    sai_fib_neighbor_create,
    sai_fib_neighbor_remove,
    sai_fib_neighbor_attribute_set,
    sai_fib_neighbor_attribute_get,
    sai_fib_neighbor_remove_all_entries,
};


sai_neighbor_api_t *sai_neighbor_api_query (void)
{
    return &sai_neighbor_method_table;
}

