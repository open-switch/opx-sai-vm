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
 * @file sai_acl_db_utils.cpp
 *
 * @brief This file contains the function definitions for the utilities
 *        related to updating the SQL DB tables for the SAI ACL objects -
 *        table, entry and counter objects in VM environment.
 */

extern "C" {
#include "sai_acl_utils.h"
#include "sai_acl_type_defs.h"
#include "sai_l3_util.h"
#include "sai_oid_utils.h"
}

#include "sai_acl_db_utils.h"
#include "sai_vm_defs.h"
#include "sai_db_gen_utils.h"

#include "saiacl.h"
#include "saitypes.h"

#include "ds_common_types.h"
#include "std_assert.h"
#include "std_mac_utils.h"

#include <string.h>
#include <inttypes.h>

#include <iostream>
#include <map>
#include <string>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>

static std::map<uint_t, std::string> ip_type_str_map =
{
    {SAI_ACL_IP_TYPE_ANY, "\"IP_TYPE_ANY\""},
    {SAI_ACL_IP_TYPE_IP, "\"IP_TYPE_IP\""},
    {SAI_ACL_IP_TYPE_NON_IP, "\"IP_TYPE_NON_IP\""},
    {SAI_ACL_IP_TYPE_IPV4ANY, "\"IP_TYPE_IPv4ANY\""},
    {SAI_ACL_IP_TYPE_NON_IPV4, "\"IP_TYPE_NON_IPv4\""},
    {SAI_ACL_IP_TYPE_IPV6ANY, "\"IP_TYPE_IPv6ANY\""},
    {SAI_ACL_IP_TYPE_NON_IPV6, "\"IP_TYPE_NON_IPv6\""},
    {SAI_ACL_IP_TYPE_ARP, "\"IP_TYPE_ARP\""},
    {SAI_ACL_IP_TYPE_ARP_REQUEST, "\"IP_TYPE_ARP_REQUEST\""},
    {SAI_ACL_IP_TYPE_ARP_REPLY, "\"IP_TYPE_ARP_REPLY\""},
};

static std::map<uint_t, std::string> ip_frag_str_map =
{
    {SAI_ACL_IP_FRAG_ANY, "\"IP_FRAG_ANY\""},
    {SAI_ACL_IP_FRAG_NON_FRAG, "\"IP_FRAG_NON_FRAG\""},
    {SAI_ACL_IP_FRAG_NON_FRAG_OR_HEAD, "\"IP_FRAG_NON_FRAG_OR_HEAD\""},
    {SAI_ACL_IP_FRAG_HEAD, "\"IP_FRAG_HEAD\""},
    {SAI_ACL_IP_FRAG_NON_HEAD, "\"IP_FRAG_NON_HEAD\""},
};

static std::map<sai_acl_stage_t, std::string> acl_stage_str_map =
{
    {SAI_ACL_STAGE_INGRESS, "\"INGRESS\""},
    {SAI_ACL_STAGE_EGRESS, "\"EGRESS\""},
};

static std::map<sai_acl_table_attr_t, std::string> acl_table_field_str_map =
{
    {SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6, "\"SRC_IPv6\""},
    {SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6, "\"DST_IPv6\""},
    {SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC, "\"SRC_MAC\""},
    {SAI_ACL_TABLE_ATTR_FIELD_DST_MAC, "\"DST_MAC\""},
    {SAI_ACL_TABLE_ATTR_FIELD_SRC_IP, "\"SRC_IP\""},
    {SAI_ACL_TABLE_ATTR_FIELD_DST_IP, "\"DST_IP\""},
    {SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS, "\"IN_PORTS\""},
    {SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS, "\"OUT_PORTS\""},
    {SAI_ACL_TABLE_ATTR_FIELD_IN_PORT, "\"IN_PORT\""},
    {SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT, "\"OUT_PORT\""},
    {SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT, "\"SRC_PORT\""},
    {SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID, "\"OUTER_VLAN_ID\""},
    {SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI, "\"OUTER_VLAN_PRI\""},
    {SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI, "\"OUTER_VLAN_CFI\""},
    {SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID, "\"INNER_VLAN_ID\""},
    {SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI, "\"INNER_VLAN_PRI\""},
    {SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI, "\"INNER_VLAN_CFI\""},
    {SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT, "\"L4_SRC_PORT\""},
    {SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT, "\"L4_DST_PORT\""},
    {SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE, "\"ETHER_TYPE\""},
    {SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL, "\"IP_PROTOCOL\""},
    {SAI_ACL_TABLE_ATTR_FIELD_DSCP, "\"DSCP\""},
    {SAI_ACL_TABLE_ATTR_FIELD_ECN, "\"ECN\""},
    {SAI_ACL_TABLE_ATTR_FIELD_TTL, "\"TTL\""},
    {SAI_ACL_TABLE_ATTR_FIELD_TOS, "\"TOS\""},
    {SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS, "\"IP_FLAGS\""},
    {SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS, "\"TCP_FLAGS\""},
    {SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE, "\"IP_TYPE\""},
    {SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG, "\"IP_FRAG\""},
    {SAI_ACL_TABLE_ATTR_FIELD_IPV6_FLOW_LABEL, "\"IPv6_FLOW_LABEL\""},
    {SAI_ACL_TABLE_ATTR_FIELD_TC, "\"TC\""},
    {SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE, "\"ICMPType\""},
    {SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE, "\"ICMPCode\""},
    {SAI_ACL_TABLE_ATTR_FIELD_PACKET_VLAN, "\"VlanTags\""},
    {SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_USER_META, "\"FDB_USER_META_DATA\""},
    {SAI_ACL_TABLE_ATTR_FIELD_ROUTE_DST_USER_META, "\"L3_ROUTE_USER_META_DATA\""},
    {SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_USER_META, "\"L3_NEIGHBOR_USER_META_DATA\""},
    {SAI_ACL_TABLE_ATTR_FIELD_PORT_USER_META, "\"PORT_META_DATA\""},
    {SAI_ACL_TABLE_ATTR_FIELD_VLAN_USER_META, "\"VLAN_META_DATA\""},
    {SAI_ACL_TABLE_ATTR_FIELD_ACL_USER_META, "\"ACL_META_DATA\""},
    {SAI_ACL_TABLE_ATTR_FIELD_FDB_NPU_META_DST_HIT, "\"FDB_NPU_META_DATA\""},
    {SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT, "\"L3_NEIGHBOR_NPU_META_DATA\""},
    {SAI_ACL_TABLE_ATTR_FIELD_ROUTE_NPU_META_DST_HIT, "\"L3_ROUTE_NPU_META_DATA\""},
    {SAI_ACL_TABLE_ATTR_FIELD_ACL_RANGE_TYPE, "\"RANGE_TYPE\""},
    {SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST, "\"ACTION_LIST\""},
    {SAI_ACL_TABLE_ATTR_FIELD_IPV6_NEXT_HEADER, "\"IPv6_NEXT_HEADER\""},
};

static std::map<sai_acl_table_attr_extensions_t, std::string> acl_table_extn_field_str_map =
{
    {SAI_ACL_TABLE_ATTR_EXTENSIONS_FIELD_LAYER3_INTERFACE_USER_META, "\"L3_INTERFACE_META_DATA\""},
    {SAI_ACL_TABLE_ATTR_EXTENSIONS_FIELD_MCAST_ROUTE_NPU_META_DST_HIT, "\"MCAST_ROUTE_NPU_META_DATA\""},
};

static std::map<sai_acl_entry_attr_t, std::string> rule_filter_str_map =
{
    {SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6, "\"SRC_IPv6\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6, "\"DST_IPv6\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC, "\"SRC_MAC\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC, "\"DST_MAC\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP, "\"SRC_IP\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_DST_IP, "\"DST_IP\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS, "\"IN_PORTS\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS, "\"OUT_PORTS\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT, "\"IN_PORT\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT, "\"OUT_PORT\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT, "\"SRC_PORT\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID, "\"OUTER_VLAN_ID\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI, "\"OUTER_VLAN_PRI\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI, "\"OUTER_VLAN_CFI\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID, "\"INNER_VLAN_ID\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI, "\"INNER_VLAN_PRI\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI, "\"INNER_VLAN_CFI\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT, "\"L4_SRC_PORT\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT, "\"L4_DST_PORT\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE, "\"ETHER_TYPE\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL, "\"IP_PROTOCOL\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_DSCP, "\"DSCP\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_ECN, "\"ECN\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_TTL, "\"TTL\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_TOS, "\"TOS\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS, "\"IP_FLAGS\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS, "\"TCP_FLAGS\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE, "\"IP_TYPE\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG, "\"IP_FRAG\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_IPV6_FLOW_LABEL, "\"IPv6_FLOW_LABEL\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_TC, "\"TC\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE, "\"ICMPType\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE, "\"ICMPCode\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_PACKET_VLAN, "\"VlanTags\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_FDB_DST_USER_META, "\"FDB_USER_META_DATA\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_DST_USER_META, "\"L3_ROUTE_USER_META_DATA\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_DST_USER_META, "\"L3_NEIGHBOR_USER_META_DATA\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_PORT_USER_META, "\"PORT_META_DATA\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_VLAN_USER_META, "\"VLAN_META_DATA\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_ACL_USER_META, "\"ACL_META_DATA\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_FDB_NPU_META_DST_HIT, "\"FDB_NPU_META_DATA\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT, "\"L3_NEIGHBOR_NPU_META_DATA\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_NPU_META_DST_HIT, "\"L3_ROUTE_NPU_META_DATA\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE, "\"RANGE_TYPE\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_IPV6_NEXT_HEADER, "\"IPv6_NEXT_HEADER\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_DROP_MARKED, "\"DROP MARKED\""},
    {SAI_ACL_ENTRY_ATTR_FIELD_BRIDGE_TYPE, "\"BRIDGE_TYPE\""},
};

static std::map<sai_acl_entry_attr_extensions_t, std::string> rule_filter_extn_str_map =
{
    {SAI_ACL_ENTRY_ATTR_EXTENSIONS_FIELD_LAYER3_INTERFACE_USER_META, "\"L3_INTERFACE_META_DATA\""},
    {SAI_ACL_ENTRY_ATTR_EXTENSIONS_FIELD_MCAST_ROUTE_NPU_META_DST_HIT, "\"MCAST_ROUTE_NPU_META_DATA\""},
};

static std::map<sai_acl_entry_attr_t, std::string> rule_action_str_map =
{
    {SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT, "\"REDIRECT\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT_LIST, "\"REDIRECTLIST\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION, "\"PACKET_ACTION\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_FLOOD, "\"FLOOD\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_COUNTER, "\"COUNTER\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS, "\"MIRROR_INGRESS\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS, "\"MIRROR_EGRESS\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER, "\"POLICER\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_DECREMENT_TTL, "\"DECREMENT_TTL\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_TC, "\"TC\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID, "\"INNER_VLAN_ID\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI, "\"INNER_VLAN_PRI\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID, "\"OUTER_VLAN_ID\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI, "\"OUTER_VLAN_PRI\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_MAC, "\"SRC_MAC\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_MAC, "\"DST_MAC\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IP, "\"SRC_IP\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IP, "\"DST_IP\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IPV6, "\"SRC_IPv6\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IPV6, "\"DST_IPv6\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP, "\"DSCP\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_ECN, "\"ECN\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_SRC_PORT, "\"L4_SRC_PORT\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_DST_PORT, "\"L4_DST_PORT\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE,
        "\"INGRESS_SAMPLEPACKET_ENABLE\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE,
        "\"EGRESS_SAMPLEPACKET_ENABLE\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_CPU_QUEUE, "\"CPU_QUEUE\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_ACL_META_DATA, "\"ACL_META_DATA\""},
    {SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_BLOCK_PORT_LIST, "\"EGRESS_BLOCK_PORT_LIST\""},
};

static std::map<sai_acl_counter_type_t, std::string> cntr_type_str_map =
{
    {SAI_ACL_COUNTER_BYTES, "\"BYTES\""},
    {SAI_ACL_COUNTER_PACKETS, "\"PKTS\""},
    {SAI_ACL_COUNTER_BYTES_PACKETS, "\"BYTES_AND_PKTS\""}
};

static std::string sai_acl_ip_type_str_get (uint_t ip_type)
{
    if (ip_type_str_map.find (ip_type) != ip_type_str_map.end()) {
        return (ip_type_str_map [ip_type]);
    } else {
        return "\"" + std::string ("UNKNOWN") + "\"";
    }
}

static std::string sai_acl_ip_frag_str_get (uint_t ip_frag)
{
    if (ip_frag_str_map.find (ip_frag) != ip_frag_str_map.end()) {
        return (ip_frag_str_map [ip_frag]);
    } else {
        return "\"" + std::string ("UNKNOWN") + "\"";
    }
}

std::string sai_acl_table_stage_str_get (sai_acl_stage_t stage)
{
    if (acl_stage_str_map.find (stage) != acl_stage_str_map.end()) {
        return (acl_stage_str_map [stage]);
    } else {
        return "\"" + std::string ("INVALID") + "\"";
    }
}

std::string sai_acl_table_field_attr_str_get (sai_acl_table_attr_t field)
{
    if (sai_acl_table_udf_field_attr_range(field)) {
        return "\"" + std::string ("UDF") + "\"";
    } else if (acl_table_field_str_map.find (field) != acl_table_field_str_map.end()) {
        return (acl_table_field_str_map [field]);
    } else if (acl_table_extn_field_str_map.find ((sai_acl_table_attr_extensions_t)field) !=
               acl_table_extn_field_str_map.end()) {
        return acl_table_extn_field_str_map [(sai_acl_table_attr_extensions_t)field];
    } else {
        return "\"" + std::string ("UNKNOWN") + "\"";
    }
}

std::string sai_acl_rule_filter_attr_str_get (sai_acl_entry_attr_t filter)
{
    if (sai_acl_rule_udf_field_attr_range(filter)) {
        return "\"" + std::string ("UDF Filter") + "\"";
    } else if (rule_filter_str_map.find (filter) != rule_filter_str_map.end()) {
        return (rule_filter_str_map [filter]);
    } else if (rule_filter_extn_str_map.find ((sai_acl_entry_attr_extensions_t)filter) !=
               rule_filter_extn_str_map.end()) {
        return rule_filter_extn_str_map [(sai_acl_entry_attr_extensions_t)filter];
    } else {
        return "\"" + std::string ("UNKNOWN") + "\"";
    }
}

std::string sai_acl_rule_action_attr_str_get (sai_acl_entry_attr_t action)
{
    if (rule_action_str_map.find (action) != rule_action_str_map.end()) {
        return (rule_action_str_map [action]);
    } else {
        return "\"" + std::string ("UNKNOWN") + "\"";
    }
}

std::string sai_acl_cntr_type_str_get (sai_acl_counter_type_t cntr_type)
{
    if (cntr_type_str_map.find (cntr_type) != cntr_type_str_map.end()) {
        return (cntr_type_str_map [cntr_type]);
    } else {
        return "\"" + std::string ("UNKNOWN") + "\"";
    }
}

void sai_acl_rule_filter_match_info_str_get (sai_acl_filter_t *p_filter,
                                             std::string *match_data_str,
                                             std::string *match_mask_str)
{
    uint_t                 npu_obj_id = 0;
    char                   addr_buf [SAI_VM_MAX_BUFSZ];
    char                   mask_buf [SAI_VM_MAX_BUFSZ];
    sai_acl_rule_attr_type rule_attr_type = SAI_ACL_ENTRY_ATTR_INVALID;

    STD_ASSERT (p_filter != NULL);
    STD_ASSERT (match_data_str != NULL);
    STD_ASSERT (match_mask_str != NULL);

    memset (addr_buf, 0, sizeof (addr_buf));

    memset (mask_buf, 0, sizeof (mask_buf));

    rule_attr_type = sai_acl_rule_get_attr_type (p_filter->field);

    switch (rule_attr_type) {
        case SAI_ACL_ENTRY_ATTR_BOOL:
            *match_data_str = std::to_string (p_filter->match_data.booldata);
            *match_mask_str = std::to_string (p_filter->match_mask.booldata);
            break;

        case SAI_ACL_ENTRY_ATTR_ONE_BYTE:
            *match_data_str = std::to_string (p_filter->match_data.u8);
            *match_mask_str = std::to_string (p_filter->match_mask.u8);
            break;

        case SAI_ACL_ENTRY_ATTR_TWO_BYTES:
            *match_data_str = std::to_string (p_filter->match_data.u16);
            *match_mask_str = std::to_string (p_filter->match_mask.u16);
            break;

        case SAI_ACL_ENTRY_ATTR_FOUR_BYTES:
            *match_data_str = std::to_string (p_filter->match_data.u32);
            *match_mask_str = std::to_string (p_filter->match_mask.u32);
            break;

        case SAI_ACL_ENTRY_ATTR_ENUM:
            if (p_filter->field == SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE) {
                *match_data_str =
                    sai_acl_ip_type_str_get (p_filter->match_data.s32);
                *match_mask_str = "\"NA\"";
            } else if (p_filter->field == SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG) {
                *match_data_str =
                    sai_acl_ip_frag_str_get (p_filter->match_data.s32);
                *match_mask_str = "\"NA\"";
            }else {
                *match_data_str = std::to_string (p_filter->match_data.s32);
                *match_mask_str = std::to_string (p_filter->match_mask.s32);
            }

            break;

        case SAI_ACL_ENTRY_ATTR_MAC:
            std_mac_to_string ((const hal_mac_addr_t *)p_filter->match_data.mac,
                               addr_buf, sizeof (addr_buf));

            std_mac_to_string ((const hal_mac_addr_t *)p_filter->match_mask.mac,
                               mask_buf, sizeof (mask_buf));
            break;

        case SAI_ACL_ENTRY_ATTR_IPv4:
            inet_ntop (AF_INET, (void *)&p_filter->match_data.ip4, addr_buf,
                       sizeof (addr_buf));

            inet_ntop (AF_INET, (void *)&p_filter->match_mask.ip4, mask_buf,
                       sizeof (mask_buf));
            break;

        case SAI_ACL_ENTRY_ATTR_IPv6:
            inet_ntop (AF_INET6, (void *)&p_filter->match_data.ip6, addr_buf,
                       sizeof (addr_buf));

            inet_ntop (AF_INET6, (void *)&p_filter->match_mask.ip6, mask_buf,
                       sizeof (mask_buf));

            break;

        case SAI_ACL_ENTRY_ATTR_OBJECT_ID:
            npu_obj_id =
                (uint_t) sai_uoid_npu_obj_id_get (p_filter->match_data.oid);
            *match_data_str = std::to_string (npu_obj_id);
            *match_mask_str = "\"NA\"";
            break;

        case SAI_ACL_ENTRY_ATTR_OBJECT_LIST:
            *match_data_str =
                sai_vm_obj_list_str_get (&p_filter->match_data.obj_list);
            *match_mask_str = "\"NA\"";
            break;

        case SAI_ACL_ENTRY_ATTR_ONE_BYTE_LIST:
            *match_data_str =
                sai_vm_byte_list_str_get (&p_filter->match_data.u8_list);
            *match_mask_str =
                sai_vm_byte_list_str_get (&p_filter->match_mask.u8_list);
            break;
        default:
            *match_data_str = std::string ("\"INVALID\"");
            *match_mask_str = std::string ("\"INVALID\"");
            break;
    }

    if ((rule_attr_type == SAI_ACL_ENTRY_ATTR_MAC) ||
        (rule_attr_type == SAI_ACL_ENTRY_ATTR_IPv4) ||
        (rule_attr_type == SAI_ACL_ENTRY_ATTR_IPv6)) {
            (*match_data_str) = "\"" + std::string (addr_buf) + "\"";
            (*match_mask_str) = "\"" + std::string (mask_buf) + "\"";
    }
}

std::string sai_acl_rule_action_parameter_str_get (sai_acl_action_t *p_action)
{
    uint_t                 npu_obj_id = 0;
    std::string            param_str;
    char                   buf [SAI_VM_MAX_BUFSZ];
    sai_acl_rule_attr_type rule_attr_type = SAI_ACL_ENTRY_ATTR_INVALID;

    STD_ASSERT (p_action != NULL);

    memset (buf, 0, sizeof (buf));

    rule_attr_type = sai_acl_rule_get_attr_type (p_action->action);

    switch (rule_attr_type) {
        case SAI_ACL_ENTRY_ATTR_BOOL:
            param_str = std::to_string (p_action->parameter.booldata);
            break;

        case SAI_ACL_ENTRY_ATTR_ONE_BYTE:
            param_str = std::to_string (p_action->parameter.u8);
            break;

        case SAI_ACL_ENTRY_ATTR_TWO_BYTES:
            param_str = std::to_string (p_action->parameter.u16);
            break;

        case SAI_ACL_ENTRY_ATTR_FOUR_BYTES:
            param_str = std::to_string (p_action->parameter.u32);
            break;

        case SAI_ACL_ENTRY_ATTR_ENUM:
            if (p_action->action == SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION) {
                param_str = sai_packet_action_str ((sai_packet_action_t)
                                                   p_action->parameter.s32);
                param_str = "\"" + param_str + "\"";
            } else {
                param_str = std::to_string (p_action->parameter.s32);
            }
            break;

        case SAI_ACL_ENTRY_ATTR_MAC:
            std_mac_to_string ((const hal_mac_addr_t *) p_action->parameter.mac,
                               buf, sizeof (buf));
            break;

        case SAI_ACL_ENTRY_ATTR_IPv4:
            inet_ntop (AF_INET, (void *)&p_action->parameter.ip4, buf,
                       sizeof (buf));
            break;

        case SAI_ACL_ENTRY_ATTR_IPv6:
            inet_ntop (AF_INET6, (void *)&p_action->parameter.ip6, buf,
                       sizeof (buf));
            break;

        case SAI_ACL_ENTRY_ATTR_OBJECT_ID:
            npu_obj_id =
                (uint_t) sai_uoid_npu_obj_id_get (p_action->parameter.oid);

            param_str = std::to_string (npu_obj_id);
            break;

        case SAI_ACL_ENTRY_ATTR_OBJECT_LIST:
            param_str =
                sai_vm_obj_list_str_get (&p_action->parameter.obj_list);
            break;
        default:
            param_str = "INVALID";
            break;
    }

    if ((rule_attr_type == SAI_ACL_ENTRY_ATTR_MAC) ||
        (rule_attr_type == SAI_ACL_ENTRY_ATTR_IPv4) ||
        (rule_attr_type == SAI_ACL_ENTRY_ATTR_IPv6)) {
        param_str = "\"" + std::string (buf) + "\"";
    }

    return param_str;
}
