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
* @file sai_hostintf_utils.c
*
* @brief This file contains SAI Host Interface utility functions.
*
*************************************************************************/
#include "std_type_defs.h"
#include "std_mutex_lock.h"
#include "sai_hostif_main.h"
#include "sai_hostif_api.h"
#include "sai_gen_utils.h"
#include "sai_hostif_common.h"

#include "saihostif.h"
#include "saiswitch.h"
#include "saitypes.h"
#include "saistatus.h"

#include<inttypes.h>

static const dn_sai_hostif_valid_traps_t valid_traps[] = {
    {SAI_HOSTIF_TRAP_TYPE_STP, SAI_PACKET_ACTION_DROP},
    {SAI_HOSTIF_TRAP_TYPE_LACP, SAI_PACKET_ACTION_DROP},
    {SAI_HOSTIF_TRAP_TYPE_EAPOL, SAI_PACKET_ACTION_DROP},
    {SAI_HOSTIF_TRAP_TYPE_LLDP, SAI_PACKET_ACTION_DROP},
    {SAI_HOSTIF_TRAP_TYPE_PVRST, SAI_PACKET_ACTION_DROP},
    {SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_QUERY, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_LEAVE, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V1_REPORT, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V2_REPORT, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V3_REPORT, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET, SAI_PACKET_ACTION_DROP},
    {SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_DHCP, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_OSPF, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_PIM, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_VRRP, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_BGP, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_DHCPV6, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_OSPFV6, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_VRRPV6, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_BGPV6, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_IPV6_NEIGHBOR_DISCOVERY, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_V2, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_REPORT, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_DONE, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_MLD_V2_REPORT, SAI_PACKET_ACTION_FORWARD},
    {SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR, SAI_PACKET_ACTION_DROP},
    {SAI_HOSTIF_TRAP_TYPE_TTL_ERROR, SAI_PACKET_ACTION_DROP}
};

static std_mutex_lock_create_static_init_fast(sai_hostintf_lock);

void sai_hostif_lock()
{
    std_mutex_lock (&sai_hostintf_lock);
}

void sai_hostif_unlock()
{
    std_mutex_unlock (&sai_hostintf_lock);
}

bool dn_sai_hostif_is_valid_trap_type(
                            sai_hostif_trap_type_t trap_type)
{
    uint_t index = 0;
    static const uint_t max_trap_count = sizeof(valid_traps) /
                                         sizeof(valid_traps[0]);

    for (index = 0; index < max_trap_count; index++) {
        if (trap_type == valid_traps[index].trap_type) {
            return true;
        }
    }
    return false;
}

sai_packet_action_t dn_sai_hostif_get_default_action(
                               sai_hostif_trap_type_t trap_type)
{
    uint_t index = 0;
    static const uint_t max_trap_count = sizeof(valid_traps) /
                                         sizeof(valid_traps[0]);

    for (index = 0; index < max_trap_count; index++) {
        if (trap_type == valid_traps[index].trap_type) {
            return valid_traps[index].def_action;
        }
    }
    return SAI_PACKET_ACTION_FORWARD;
}

sai_status_t sai_hostif_get_default_trap_group(sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);
    dn_sai_hostintf_info_t *hostif_info = dn_sai_hostintf_get_info();

    SAI_HOSTIF_LOG_TRACE("Get default switch trap group");
    sai_hostif_lock();
    attr->value.oid = hostif_info->default_trap_group_id;
    sai_hostif_unlock();

    return SAI_STATUS_SUCCESS;
}

sai_status_t dn_sai_hostif_validate_trapgroup_oid(const sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);
    dn_sai_hostintf_info_t *hostif_info = dn_sai_hostintf_get_info();

    if ((SAI_NULL_OBJECT_ID != attr->value.oid) &&
        (NULL == dn_sai_hostif_find_trapgroup(hostif_info->trap_group_tree,
                                              attr->value.oid))) {
        SAI_HOSTIF_LOG_ERR("Trap group %"PRIu64" not present",
                           attr->value.oid);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t dn_sai_hostif_validate_portlist(const sai_attribute_t *attr)
{
    unsigned int invalid_idx = 0;
    STD_ASSERT(attr != NULL);

    if(!sai_is_port_list_valid(&attr->value.objlist,&invalid_idx)) {
        SAI_HOSTIF_LOG_ERR("Invalid port in object list");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t dn_sai_hostif_validate_action(const sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);

    if ((SAI_PACKET_ACTION_DROP == attr->value.s32) ||
        (SAI_PACKET_ACTION_TRAP == attr->value.s32) ||
        (SAI_PACKET_ACTION_LOG == attr->value.s32) ||
        (SAI_PACKET_ACTION_FORWARD == attr->value.s32)) {
        return SAI_STATUS_SUCCESS;
    }

    SAI_HOSTIF_LOG_ERR("Invalid action %d", attr->value.s32);
    return SAI_STATUS_INVALID_ATTR_VALUE_0;
}

dn_sai_trap_group_node_t *dn_sai_hostif_find_trapgroup_by_queue(uint_t cpu_queue)
{
    dn_sai_hostintf_info_t *hostif_info = dn_sai_hostintf_get_info();

    dn_sai_trap_group_node_t *trap_group = (dn_sai_trap_group_node_t *)
                         std_rbtree_getfirst(hostif_info->trap_group_tree);

    while (trap_group != NULL) {
        if (trap_group->cpu_queue == cpu_queue) {
            SAI_HOSTIF_LOG_TRACE("Found trap group with queue = %u", cpu_queue);
            return trap_group;
        }
        trap_group = (dn_sai_trap_group_node_t *)std_rbtree_getnext(
                                               hostif_info->trap_group_tree, trap_group);
    }
    return trap_group;
}

