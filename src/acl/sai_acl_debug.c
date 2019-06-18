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
 * * @file sai_acl_debug.c
 * *
 * * @brief This file contains functions to dump and debug for SAI ACL
 * *        data structures.
 * *
 * *************************************************************************/
#include "sai_common_acl.h"
#include "sai_acl_type_defs.h"
#include "sai_debug_utils.h"
#include "std_rbtree.h"
#include "sai_common_infra.h"
#include "sai_acl_utils.h"
#include "sai_acl_rule_utils.h"

#include <inttypes.h>
#include <arpa/inet.h>

static const struct {
    sai_acl_table_attr_t table_field;
    char *table_string;
} sai_acl_translate_field_to_string[] = {
    {SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6, "Source IPv6 Address"},
    {SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6, "Destination IPv6 Address"},
    {SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC, "Source MAC Address"},
    {SAI_ACL_TABLE_ATTR_FIELD_DST_MAC, "Destination MAC Address"},
    {SAI_ACL_TABLE_ATTR_FIELD_SRC_IP, "Source IPv4 Address"},
    {SAI_ACL_TABLE_ATTR_FIELD_DST_IP, "Destination Ipv4 Address"},
    {SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS, "In Ports"},
    {SAI_ACL_TABLE_ATTR_FIELD_IN_PORT, "In Port"},
    {SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS, "Out Ports"},
    {SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT, "Out Port"},
    {SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT, "Source Port"},
    {SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID, "Outer Vlan Id"},
    {SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI, "Outer Vlan Priority"},
    {SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI, "Outer Vlan CFI"},
    {SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID, "Inner Vlan Id"},
    {SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI, "Inner Vlan Priority"},
    {SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI, "Inner Vlan CFI"},
    {SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT, "L4 Source Port"},
    {SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT, "L4 Destination Port"},
    {SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE, "EtherType"},
    {SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL, "IP Protocol"},
    {SAI_ACL_TABLE_ATTR_FIELD_DSCP, "DSCP"},
    {SAI_ACL_TABLE_ATTR_FIELD_ECN, "ECN"},
    {SAI_ACL_TABLE_ATTR_FIELD_TTL, "TTL"},
    {SAI_ACL_TABLE_ATTR_FIELD_TOS, "TOS"},
    {SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS, "IP Flags"},
    {SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS, "TCP Flags"},
    {SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE, "IP Type"},
    {SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG, "IP Frag"},
    {SAI_ACL_TABLE_ATTR_FIELD_IPV6_FLOW_LABEL, "IPv6 Flow Label"},
    {SAI_ACL_TABLE_ATTR_FIELD_TC, "TC"},
    {SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE, "ICMPType"},
    {SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE, "ICMPCode"},
    {SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_USER_META, "FDB User MetaData"},
    {SAI_ACL_TABLE_ATTR_FIELD_ROUTE_DST_USER_META, "L3 Route User MetaData"},
    {SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_USER_META, "L3 Neighbor User MetaData"},
    {SAI_ACL_TABLE_ATTR_FIELD_PORT_USER_META, "Port User MetaData"},
    {SAI_ACL_TABLE_ATTR_FIELD_VLAN_USER_META, "Vlan User MetaData"},
    {SAI_ACL_TABLE_ATTR_FIELD_ACL_USER_META, "ACL User MetaData"},
    {SAI_ACL_TABLE_ATTR_FIELD_FDB_NPU_META_DST_HIT, "FDB NPU MetaData"},
    {SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT, "L3 Neighbor NPU MetaData"},
    {SAI_ACL_TABLE_ATTR_FIELD_ROUTE_NPU_META_DST_HIT, "L3 Route NPU Metadata"},

};

static const size_t sai_acl_translate_field_to_string_size =
                    (sizeof(sai_acl_translate_field_to_string)/
                     sizeof(sai_acl_translate_field_to_string[0]));

static const struct {
    sai_acl_entry_attr_t rule_attr;
    char *rule_string;
} sai_acl_translate_rule_attr_to_string[] = {
    {SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6, "Source IPv6 Address"},
    {SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6, "Destination IPv6 Address"},
    {SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC, "Source MAC Address"},
    {SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC, "Destination MAC Address"},
    {SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP, "Source IPv4 Address"},
    {SAI_ACL_ENTRY_ATTR_FIELD_DST_IP, "Destination Ipv4 Address"},
    {SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS, "In Ports"},
    {SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT, "In Port"},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS, "Out Ports"},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT, "Out Port"},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID, "Outer Vlan Id"},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI, "Outer Vlan Priority"},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI, "Outer Vlan CFI"},
    {SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID, "Inner Vlan Id"},
    {SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI, "Inner Vlan Priority"},
    {SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI, "Inner Vlan CFI"},
    {SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT, "L4 Source Port"},
    {SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT, "L4 Destination Port"},
    {SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE, "EtherType"},
    {SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL, "IP Protocol"},
    {SAI_ACL_ENTRY_ATTR_FIELD_DSCP, "DSCP"},
    {SAI_ACL_ENTRY_ATTR_FIELD_ECN, "ECN"},
    {SAI_ACL_ENTRY_ATTR_FIELD_TTL, "TTL"},
    {SAI_ACL_ENTRY_ATTR_FIELD_TOS, "TOS"},
    {SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS, "IP Flags"},
    {SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS, "TCP Flags"},
    {SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE, "IP Type"},
    {SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG, "IP Frag"},
    {SAI_ACL_ENTRY_ATTR_FIELD_IPV6_FLOW_LABEL, "IPv6 Flow Label"},
    {SAI_ACL_ENTRY_ATTR_FIELD_TC, "TC"},
    {SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE, "ICMPType"},
    {SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE, "ICMPCode"},
    {SAI_ACL_ENTRY_ATTR_FIELD_FDB_DST_USER_META, "FDB User MetaData"},
    {SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_DST_USER_META, "L3 Route User MetaData"},
    {SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_DST_USER_META, "L3 Neighbor User MetaData"},
    {SAI_ACL_ENTRY_ATTR_FIELD_PORT_USER_META, "Port User MetaData"},
    {SAI_ACL_ENTRY_ATTR_FIELD_VLAN_USER_META, "Vlan User MetaData"},
    {SAI_ACL_ENTRY_ATTR_FIELD_ACL_USER_META, "ACL User MetaData"},
    {SAI_ACL_ENTRY_ATTR_FIELD_FDB_NPU_META_DST_HIT, "FDB NPU MetaData"},
    {SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT, "L3 Neighbor NPU MetaData"},
    {SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_NPU_META_DST_HIT, "L3 Route NPU MetaData"},
    {SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE, "ACL RAnge type"},
    {SAI_ACL_ENTRY_ATTR_FIELD_IPV6_NEXT_HEADER, "IPV6 Next Header"},
    {SAI_ACL_ENTRY_ATTR_FIELD_DROP_MARKED, "Drop Marked"},
    {SAI_ACL_ENTRY_ATTR_FIELD_BRIDGE_TYPE, "Bridge Type"},
    {SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT, "Redirect Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION, "Packet Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_FLOOD, "Flood Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS, "Mirror Ingress"},
    {SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS, "Mirror Egress"},
    {SAI_ACL_ENTRY_ATTR_ACTION_COUNTER, "Counter Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER, "Policer Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_DECREMENT_TTL, "TTL Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_TC, "Set TC Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID, "Set Inner Vlan Id"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI, "Set Inner Vlan Pri"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID, "Set Outer Vlan Id"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI, "Set Outer Vlan Pri"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_MAC, "Set Source MAC Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_MAC, "Set Destination MAC Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IP, "Set Source IP Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IP, "Set Destination IP Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IPV6, "Set Source IPv6 Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IPV6, "Set Destination IPv6 Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP, "Set DSCP Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_SRC_PORT, "Set L4 Source Port"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_DST_PORT, "Set L4 Destination Port"},
    {SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE, "Ingress Sample Paccket Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE, "Egress Sample Paccket Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_CPU_QUEUE, "Set CPU Queue Action"},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_ACL_META_DATA, "Set ACL MetaData"}
};

static const size_t sai_acl_translate_rule_attr_to_string_size =
                    (sizeof(sai_acl_translate_rule_attr_to_string)/
                     sizeof(sai_acl_translate_rule_attr_to_string[0]));

void sai_acl_dump_help (void)
{
    SAI_DEBUG ("\n**************** ACL Dump Routines ***********************\n");

    SAI_DEBUG ("  void sai_acl_dump_all_tables (void)   -    Dump all ACL Tables \n");
    SAI_DEBUG ("  void sai_acl_dump_table (sai_object_id_t acl_table_id) - ");
    SAI_DEBUG ("                                             Dump specified ACL Table \n");
    SAI_DEBUG ("  void sai_acl_dump_all_rules_in_table (sai_object_id_t acl_table_id) - ");
    SAI_DEBUG ("                                 Dump all ACL rules within the specified ACL Table \n");
    SAI_DEBUG ("  void sai_acl_dump_rule (sai_object_id_t acl_rule_id) - ");
    SAI_DEBUG ("                                             Dump specified ACL Rule \n");
    SAI_DEBUG ("  void sai_acl_dump_all_counters (void) -    Dump all ACL Counters \n");
    SAI_DEBUG ("  void sai_acl_dump_counter (sai_object_id_t acl_counter_id)  - ");
    SAI_DEBUG ("                                             Dump specified ACL Counter \n");
}

static void sai_acl_dump_mac_in_rule(sai_acl_rule_t *acl_rule,
                                     uint_t index, bool filter)
{
    uint8_t *mac_data;
    uint8_t *mac_mask;

    if (filter) {
        mac_data = (uint8_t *)&acl_rule->filter_list[index].match_data.mac;
        mac_mask = (uint8_t *)&acl_rule->filter_list[index].match_mask.mac;

        SAI_DEBUG("     MAC Data: %02x:%02x:%02x:%02x:%02x:%02x\n "
              "    MAC Mask: %02x:%02x:%02x:%02x:%02x:%02x",
              mac_data[0], mac_data[1], mac_data[2],
              mac_data[3], mac_data[4], mac_data[5],
              mac_mask[0], mac_mask[1], mac_mask[2],
              mac_mask[3], mac_mask[4], mac_mask[5]);
    } else {
        mac_data = (uint8_t *)&acl_rule->action_list[index].parameter.mac;

        SAI_DEBUG("     MAC Data: %02x:%02x:%02x:%02x:%02x:%02x ",
              mac_data[0], mac_data[1], mac_data[2],
              mac_data[3], mac_data[4], mac_data[5]);
    }
}

static void sai_acl_dump_ipv6_in_rule(sai_acl_rule_t *acl_rule,
                                      uint_t index, bool filter)
{
    char       buf[INET6_ADDRSTRLEN+1];

    if (filter) {
        SAI_DEBUG("     Filter IPv6 Address: %s",
              inet_ntop (AF_INET6, (const void *)(&acl_rule->filter_list[index].match_data.ip6),
                         buf, sizeof(buf)));
        memset(buf, 0, sizeof(buf));
        SAI_DEBUG("     Filter IPv6 Mask:  %s",
              inet_ntop (AF_INET6, (const void *)(&acl_rule->filter_list[index].match_mask.ip6),
                         buf, sizeof(buf)));
    } else {
        SAI_DEBUG("     Action IPv6 Address: %s",
              inet_ntop (AF_INET6, (const void *)(&acl_rule->action_list[index].parameter.ip6),
                         buf, sizeof(buf)));
    }
}

static void sai_acl_dump_obj_list_in_rule(sai_object_list_t *obj_list)
{
    uint_t obj_cnt = 0;

    SAI_DEBUG("    Object Count in the Object List: %d",
              obj_list->count);

    for (obj_cnt = 0; obj_cnt < obj_list->count; obj_cnt++) {
         SAI_DEBUG("      Object Id %d: 0x%"PRIx64"", (obj_cnt+1),
                   obj_list->list[obj_cnt]);
    }
}

void sai_acl_dump_rule_qual(sai_acl_rule_t *acl_rule)
{

    uint_t filter = 0, filter_idx = 0;
    uint_t u8_count = 0;

    for (filter = 0; filter < acl_rule->filter_count; filter++) {
         if (sai_acl_rule_udf_field_attr_range(acl_rule->filter_list[filter].field)) {
             SAI_DEBUG("%d> UDF Filter Admin Status : %s\n "
                       "  UDF Group Id : 0x%"PRIx64", UDF Attr Index = %d",
                       (filter+1), acl_rule->filter_list[filter].enable ? "Enable" : "Disable",
                       acl_rule->filter_list[filter].udf_field.udf_group_id,
                       acl_rule->filter_list[filter].udf_field.udf_attr_index);
             SAI_DEBUG("Rule UDF Filter Field Data and Mask :");
             for (u8_count = 0; u8_count < acl_rule->filter_list[filter].match_data.u8_list.count;
                  u8_count++) {
                  SAI_DEBUG("Byte %d> UDF Data : %x UDF Mask : %x", (u8_count+1),
                             acl_rule->filter_list[filter].match_data.u8_list.list[u8_count],
                             acl_rule->filter_list[filter].match_mask.u8_list.list[u8_count]);
             }
         } else {

         for (filter_idx = 0; filter_idx < sai_acl_translate_rule_attr_to_string_size;
              filter_idx++) {
              if (acl_rule->filter_list[filter].field ==
                        sai_acl_translate_rule_attr_to_string[filter_idx].rule_attr) {
                  SAI_DEBUG("%d>  Rule Filter Field = %s", (filter+1), sai_acl_translate_rule_attr_to_string[
                            filter_idx].rule_string);
                  SAI_DEBUG("    Rule Filter Admin Status = %s",
                            acl_rule->filter_list[filter].enable ? "Enable" : "Disable");
                  SAI_DEBUG("    Rule Filter Field Data:");

                  switch(sai_acl_rule_get_attr_type(acl_rule->filter_list[filter].field)) {
                    case SAI_ACL_ENTRY_ATTR_BOOL:
                        SAI_DEBUG("     Field Data: %d",
                            acl_rule->filter_list[filter].match_data.booldata);
                        break;
                    case SAI_ACL_ENTRY_ATTR_ONE_BYTE:
                        SAI_DEBUG("     Field Data: %d, Field Mask: %d",
                            acl_rule->filter_list[filter].match_data.u8,
                            acl_rule->filter_list[filter].match_mask.u8);
                        break;
                    case SAI_ACL_ENTRY_ATTR_TWO_BYTES:
                        SAI_DEBUG("     Field Data: %d, Field Mask: %d",
                            acl_rule->filter_list[filter].match_data.u16,
                            acl_rule->filter_list[filter].match_mask.u16);
                        break;
                    case SAI_ACL_ENTRY_ATTR_FOUR_BYTES:
                        SAI_DEBUG("     Field Data: %d, Field Mask: %d",
                            acl_rule->filter_list[filter].match_data.u32,
                            acl_rule->filter_list[filter].match_mask.u32);
                        break;
                    case SAI_ACL_ENTRY_ATTR_ENUM:
                        SAI_DEBUG("     Field Data: %d, Field Mask: %d",
                            acl_rule->filter_list[filter].match_data.s32,
                            acl_rule->filter_list[filter].match_mask.s32);
                        break;
                    case SAI_ACL_ENTRY_ATTR_MAC:
                        sai_acl_dump_mac_in_rule(acl_rule, filter, true);
                        break;
                    case SAI_ACL_ENTRY_ATTR_IPv4:
                        SAI_DEBUG("     Field IPv4 Data: 0x%x, Field IPv4 Mask: 0x%x",
                            acl_rule->filter_list[filter].match_data.ip4,
                            acl_rule->filter_list[filter].match_mask.ip4);
                        break;
                    case SAI_ACL_ENTRY_ATTR_IPv6:
                        sai_acl_dump_ipv6_in_rule(acl_rule, filter, true);
                        break;
                    case SAI_ACL_ENTRY_ATTR_OBJECT_ID:
                        SAI_DEBUG("     Field Object Data: 0x%"PRIx64"",
                            acl_rule->filter_list[filter].match_data.oid);
                        break;
                    case SAI_ACL_ENTRY_ATTR_OBJECT_LIST:
                        sai_acl_dump_obj_list_in_rule(&acl_rule->filter_list[filter].match_data.obj_list);
                        break;
                    default:
                        break;
                  }

                  break;
              }
         }
        }
    }
}

void sai_acl_dump_rule_action(sai_acl_rule_t *acl_rule)
{

    uint_t action = 0, action_idx = 0;

    for (action = 0; action < acl_rule->action_count; action++) {
         for (action_idx = 0; action_idx < sai_acl_translate_rule_attr_to_string_size;
              action_idx++) {
              if (acl_rule->action_list[action].action ==
                  sai_acl_translate_rule_attr_to_string[action_idx].rule_attr) {
                  SAI_DEBUG("%d>   Rule Action = %s", (action+1), sai_acl_translate_rule_attr_to_string[
                            action_idx].rule_string);
                  SAI_DEBUG("     Rule Action Admin Status = %s",
                              acl_rule->action_list[action].enable ? "Enable" : "Disable");
                  SAI_DEBUG("     Rule Action Data:");

                  switch(sai_acl_rule_get_attr_type(acl_rule->action_list[action].action)) {
                    case SAI_ACL_ENTRY_ATTR_ONE_BYTE:
                        SAI_DEBUG("     Action Data: %d",
                            acl_rule->action_list[action].parameter.u8);
                        break;
                    case SAI_ACL_ENTRY_ATTR_TWO_BYTES:
                        SAI_DEBUG("     Action Data: %d",
                            acl_rule->action_list[action].parameter.u16);
                        break;
                    case SAI_ACL_ENTRY_ATTR_FOUR_BYTES:
                        SAI_DEBUG("     Action Data: %d",
                            acl_rule->action_list[action].parameter.u32);
                        break;
                    case SAI_ACL_ENTRY_ATTR_ENUM:
                        SAI_DEBUG("     Action Data: %d",
                            acl_rule->action_list[action].parameter.s32);
                        break;
                    case SAI_ACL_ENTRY_ATTR_MAC:
                        sai_acl_dump_mac_in_rule(acl_rule, action, false);
                        break;
                    case SAI_ACL_ENTRY_ATTR_IPv4:
                        SAI_DEBUG("     Action IPv4 Data: 0x%x",
                            acl_rule->action_list[action].parameter.ip4);
                        break;
                    case SAI_ACL_ENTRY_ATTR_IPv6:
                        sai_acl_dump_ipv6_in_rule(acl_rule, action, false);
                        break;
                    case SAI_ACL_ENTRY_ATTR_OBJECT_ID:
                        SAI_DEBUG("     Action Object Data: 0x%"PRIx64"",
                            acl_rule->action_list[action].parameter.oid);
                        break;
                    case SAI_ACL_ENTRY_ATTR_OBJECT_LIST:
                        sai_acl_dump_obj_list_in_rule(&acl_rule->action_list[action].parameter.obj_list);
                    default:
                        break;
                  }

                  break;
              }
        }
    }
}
void sai_acl_dump_counter(sai_object_id_t counter_id)
{
    acl_node_pt acl_node = NULL;
    sai_acl_counter_t *acl_counter = NULL;

    acl_node = sai_acl_get_acl_node();
    acl_counter = sai_acl_cntr_find(acl_node->sai_acl_counter_tree,
                                       counter_id);

    if (acl_counter == NULL) {
        SAI_DEBUG(" ACL Counter Id 0x%"PRIx64" not found", counter_id);
        return;
    }

    SAI_DEBUG("\n ********** Dumping ACL Counter Id: 0x%"PRIx64" ********** \n", counter_id);

    SAI_DEBUG("Counter Id: 0x%"PRIx64", Counter Table Id: 0x%"PRIx64" \n"
              "Counter Type: %s, Counter Shared Count: %d",
              acl_counter->counter_key.counter_id,
              acl_counter->table_id,
              (acl_counter->counter_type == SAI_ACL_COUNTER_BYTES ? "Bytes" :
              acl_counter->counter_type == SAI_ACL_COUNTER_PACKETS ? "Packets" :
              acl_counter->counter_type == SAI_ACL_COUNTER_BYTES_PACKETS ? "Bytes/Packets"
              : "Unknown"), acl_counter->shared_count);

    sai_acl_npu_api_get()->dump_acl_counter(acl_counter);

    return;
}

void sai_acl_dump_all_counters(void)
{
    acl_node_pt acl_node = NULL;
    sai_acl_counter_t *acl_counter = NULL;
    rbtree_handle acl_counter_tree = NULL;

    acl_node = sai_acl_get_acl_node();
    if (acl_node == NULL)
    {
        SAI_DEBUG("ACL Parent Node not present");
        return;
    }

    acl_counter_tree = acl_node->sai_acl_counter_tree;

    if(acl_counter_tree == NULL)
    {
        SAI_DEBUG("ACL Counter not present");
        return;
    }


    acl_counter = (sai_acl_counter_t *)std_rbtree_getfirst(acl_counter_tree);
    if (acl_counter == NULL)
    {
        SAI_DEBUG("ACL Counter not present in the ACL Counter tree");
        return;
    }

    for ( ; acl_counter != NULL; acl_counter = (sai_acl_counter_t *)
          std_rbtree_getnext(acl_counter_tree, acl_counter))
    {
        sai_acl_dump_counter(acl_counter->counter_key.counter_id);
    }

    return;
}

void sai_acl_dump_rule(sai_object_id_t rule_id)
{
    sai_acl_rule_t *acl_rule = NULL;
    acl_node_pt acl_node = NULL;

    acl_node = sai_acl_get_acl_node();
    acl_rule = sai_acl_rule_find(acl_node->sai_acl_rule_tree, rule_id);

    if (acl_rule == NULL) {
        SAI_DEBUG("ACL Rule not present for Rule ID 0x%"PRIx64"", rule_id);
        return;
    }

    SAI_DEBUG("\n ******* Dumping ACL Rule Id: 0x%"PRIx64" ******* \n", rule_id);
    SAI_DEBUG("Rule Id: 0x%"PRIx64", Rule Priority: %d, Rule Table Id: 0x%"PRIx64"\n "
              "Rule Admin State: %s, Rule Filter Count: %d, "
              "Rule Action Count: %d,\n Rule Counter Id: 0x%"PRIx64" "
              "Rule Ingress SamplePacket Id: 0x%"PRIx64", \n "
              "Rule Egress SamplePacket Id: 0x%"PRIx64", "
              "Rule Policer Id: 0x%"PRIx64" ",
              acl_rule->rule_key.acl_id, acl_rule->acl_rule_priority,
              acl_rule->table_id, acl_rule->acl_rule_state ? "Enable" : "Disable",
              acl_rule->filter_count, acl_rule->action_count,
              acl_rule->counter_id, acl_rule->samplepacket_id[0],
              acl_rule->samplepacket_id[1],
              acl_rule->policer_id);

    SAI_DEBUG("\nQualifier Set in the Rule");
    SAI_DEBUG("-------------------------");

    sai_acl_dump_rule_qual(acl_rule);

    SAI_DEBUG("\nAction Set in the Rule");
    SAI_DEBUG("-------------------------");

    sai_acl_dump_rule_action(acl_rule);

    sai_acl_npu_api_get()->dump_acl_rule(acl_rule);

    sai_acl_dump_counter(acl_rule->counter_id);
    return;
}

void sai_acl_dump_all_rules_in_table (sai_object_id_t table_id)
{
    acl_node_pt acl_node = NULL;
    sai_acl_rule_t *acl_rule = NULL;
    sai_acl_table_t *acl_table = NULL;

    acl_node = sai_acl_get_acl_node();
    acl_table = sai_acl_table_find(acl_node->sai_acl_table_tree,
                                   table_id);

    if (acl_table == NULL) {
        SAI_DEBUG(" ACL Table Id 0x%"PRIx64" not found", table_id);
        return;
    }

    acl_rule = (sai_acl_rule_t *)std_dll_getfirst(&acl_table->rule_head);

    if (acl_rule == NULL) {
        SAI_DEBUG("No Rules present in Table 0x%"PRIx64"", table_id);
        return;
    }

    SAI_DEBUG("\n ***** Dumping all Rules in ACL Table Id: 0x%"PRIx64" *****", table_id);
    SAI_DEBUG("-------------------------------------------------------------");
    while (acl_rule != NULL)
    {
        sai_acl_dump_rule(acl_rule->rule_key.acl_id);
        acl_rule = (sai_acl_rule_t *)std_dll_getnext(&acl_table->rule_head,
                                                     &acl_rule->rule_link);
    }

    return;
}

void sai_acl_dump_table(sai_object_id_t table_id)
{
    uint_t field = 0, field_idx = 0;
    acl_node_pt acl_node = NULL;
    sai_acl_table_t *acl_table = NULL;
    uint_t udf_field_cnt = 0;
    bool udf_fields = false;

    acl_node = sai_acl_get_acl_node();
    acl_table = sai_acl_table_find(acl_node->sai_acl_table_tree,
                                   table_id);

    if (acl_table == NULL) {
        SAI_DEBUG(" ACL Table Id 0x%"PRIx64" not found", table_id);
        return;
    }

    SAI_DEBUG("\n ********** Dumping ACL Table Id: 0x%"PRIx64" ********** \n", table_id);

    SAI_DEBUG("Table Id: 0x%"PRIx64", Priority: %lu , Stage: %s, \n "
              "Qualifier Count: %d, Number of Rules: %d "
              "Number of Counters: %d, UDF Count : %d\n",
              acl_table->table_key.acl_table_id,
              acl_table->acl_table_priority, acl_table->acl_stage ? "Egress" :
              "Ingress", acl_table->field_count, acl_table->rule_count,
              acl_table->num_counters, acl_table->udf_field_count);

    SAI_DEBUG("Qualifier Set Description");
    SAI_DEBUG("-------------------------");

    for (field = 0; field < acl_table->field_count; field++) {
         if (!udf_fields && sai_acl_table_udf_field_attr_range(acl_table->field_list[field])) {
             udf_fields = true;
             for (udf_field_cnt = 0; udf_field_cnt < acl_table->udf_field_count;
                  udf_field_cnt++) {
                  SAI_DEBUG("%d> UDF Group Id : 0x%"PRIx64", UDF Attr Index = %d",
                              (udf_field_cnt+1), acl_table->udf_field_list[udf_field_cnt].udf_group_id,
                              acl_table->udf_field_list[udf_field_cnt].udf_attr_index);
             }
        } else {
            for (field_idx = 0; field < sai_acl_translate_field_to_string_size;
                 field_idx++) {
                 if (sai_acl_translate_field_to_string[field_idx].table_field ==
                     acl_table->field_list[field]) {
                     SAI_DEBUG("%d>  %s", (field+1),
                               sai_acl_translate_field_to_string[field_idx].table_string);
                     break;
                 }
              }
         }
    }

    sai_acl_npu_api_get()->dump_acl_table(acl_table);

    sai_acl_dump_all_rules_in_table(table_id);
    return;
}

void sai_acl_dump_all_tables(void)
{
    acl_node_pt acl_node = NULL;
    sai_acl_table_t *acl_table = NULL;
    rbtree_handle acl_table_tree = NULL;

    acl_node = sai_acl_get_acl_node();
    if (acl_node == NULL)
    {
        SAI_DEBUG("ACL Parent Node not present");
        return;
    }

    acl_table_tree = acl_node->sai_acl_table_tree;

    if(acl_table_tree == NULL)
    {
        SAI_DEBUG("ACL Table not present");
        return;
    }


    acl_table = (sai_acl_table_t *)std_rbtree_getfirst(acl_table_tree);
    if (acl_table == NULL)
    {
        SAI_DEBUG("No ACL Table present in the ACL Table tree");
        return;
    }

    for ( ; acl_table != NULL; acl_table = (sai_acl_table_t *)
          std_rbtree_getnext(acl_table_tree,acl_table))
    {
        sai_acl_dump_table(acl_table->table_key.acl_table_id);
    }

    return;
}

void sai_acl_dump_counters()
{
  sai_acl_npu_api_get()->dump_all_counters();
}

void sai_acl_dump_counter_per_entry(int eid)
{
    sai_acl_npu_api_get()->dump_entry_counter(eid);
}
