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
* @file sai_acl_utils.c
*
* @brief This file contains SAI ACL Utility API's.
*
*************************************************************************/

#include "saiacl.h"
#include "saitypes.h"
#include "std_type_defs.h"
#include "sai_acl_utils.h"

static sai_acl_node_t acl_node;

acl_node_pt sai_acl_get_acl_node(void)
{
    return &acl_node;
}

static const struct {
    sai_acl_entry_attr_t rule_attr;
    sai_acl_rule_attr_type attr_type;
} sai_acl_rule_translate_attr_type[] = {
    {SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6, SAI_ACL_ENTRY_ATTR_IPv6},
    {SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6, SAI_ACL_ENTRY_ATTR_IPv6},
    {SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC, SAI_ACL_ENTRY_ATTR_MAC},
    {SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC, SAI_ACL_ENTRY_ATTR_MAC},
    {SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP, SAI_ACL_ENTRY_ATTR_IPv4},
    {SAI_ACL_ENTRY_ATTR_FIELD_DST_IP, SAI_ACL_ENTRY_ATTR_IPv4},
    {SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS, SAI_ACL_ENTRY_ATTR_OBJECT_LIST},
    {SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT, SAI_ACL_ENTRY_ATTR_OBJECT_ID},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS, SAI_ACL_ENTRY_ATTR_OBJECT_LIST},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT, SAI_ACL_ENTRY_ATTR_OBJECT_ID},
    {SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT, SAI_ACL_ENTRY_ATTR_OBJECT_ID},
    {SAI_ACL_ENTRY_ATTR_FIELD_DST_PORT, SAI_ACL_ENTRY_ATTR_OBJECT_ID},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID, SAI_ACL_ENTRY_ATTR_TWO_BYTES},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID, SAI_ACL_ENTRY_ATTR_TWO_BYTES},
    {SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT, SAI_ACL_ENTRY_ATTR_TWO_BYTES},
    {SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT, SAI_ACL_ENTRY_ATTR_TWO_BYTES},
    {SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE, SAI_ACL_ENTRY_ATTR_TWO_BYTES},
    {SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_FIELD_DSCP, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_FIELD_ECN, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_FIELD_TTL, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_FIELD_TOS, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE, SAI_ACL_ENTRY_ATTR_ENUM},
    {SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG, SAI_ACL_ENTRY_ATTR_ENUM},
    {SAI_ACL_ENTRY_ATTR_FIELD_IPV6_FLOW_LABEL, SAI_ACL_ENTRY_ATTR_FOUR_BYTES},
    {SAI_ACL_ENTRY_ATTR_FIELD_TC, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_FIELD_PACKET_VLAN, SAI_ACL_ENTRY_ATTR_ENUM},
    {SAI_ACL_ENTRY_ATTR_FIELD_FDB_DST_USER_META, SAI_ACL_ENTRY_ATTR_FOUR_BYTES},
    {SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_DST_USER_META, SAI_ACL_ENTRY_ATTR_FOUR_BYTES},
    {SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_DST_USER_META, SAI_ACL_ENTRY_ATTR_FOUR_BYTES},
    {SAI_ACL_ENTRY_ATTR_FIELD_PORT_USER_META, SAI_ACL_ENTRY_ATTR_FOUR_BYTES},
    {SAI_ACL_ENTRY_ATTR_FIELD_VLAN_USER_META, SAI_ACL_ENTRY_ATTR_FOUR_BYTES},
    {SAI_ACL_ENTRY_ATTR_FIELD_ACL_USER_META, SAI_ACL_ENTRY_ATTR_FOUR_BYTES},
    {SAI_ACL_ENTRY_ATTR_FIELD_FDB_NPU_META_DST_HIT, SAI_ACL_ENTRY_ATTR_BOOL},
    {SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT, SAI_ACL_ENTRY_ATTR_BOOL},
    {SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_NPU_META_DST_HIT, SAI_ACL_ENTRY_ATTR_BOOL},
    {SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE, SAI_ACL_ENTRY_ATTR_OBJECT_LIST},
    {SAI_ACL_ENTRY_ATTR_FIELD_IPV6_NEXT_HEADER, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT, SAI_ACL_ENTRY_ATTR_OBJECT_ID},
    {SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT_LIST, SAI_ACL_ENTRY_ATTR_OBJECT_LIST},
    {SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION, SAI_ACL_ENTRY_ATTR_ENUM},
    {SAI_ACL_ENTRY_ATTR_ACTION_COUNTER, SAI_ACL_ENTRY_ATTR_OBJECT_ID},
    {SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS, SAI_ACL_ENTRY_ATTR_OBJECT_LIST},
    {SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS, SAI_ACL_ENTRY_ATTR_OBJECT_LIST},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER, SAI_ACL_ENTRY_ATTR_OBJECT_ID},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_TC, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID, SAI_ACL_ENTRY_ATTR_TWO_BYTES},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID, SAI_ACL_ENTRY_ATTR_TWO_BYTES},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_MAC, SAI_ACL_ENTRY_ATTR_MAC},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_MAC, SAI_ACL_ENTRY_ATTR_MAC},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IP, SAI_ACL_ENTRY_ATTR_IPv4},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IP, SAI_ACL_ENTRY_ATTR_IPv4},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IPV6, SAI_ACL_ENTRY_ATTR_IPv6},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IPV6, SAI_ACL_ENTRY_ATTR_IPv6},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_ECN, SAI_ACL_ENTRY_ATTR_ONE_BYTE},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_SRC_PORT, SAI_ACL_ENTRY_ATTR_TWO_BYTES},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_DST_PORT, SAI_ACL_ENTRY_ATTR_TWO_BYTES},
    {SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE,
                                                SAI_ACL_ENTRY_ATTR_OBJECT_ID},
    {SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE,
                                                SAI_ACL_ENTRY_ATTR_OBJECT_ID},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_CPU_QUEUE, SAI_ACL_ENTRY_ATTR_OBJECT_ID},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_ACL_META_DATA, SAI_ACL_ENTRY_ATTR_FOUR_BYTES},
    {SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_BLOCK_PORT_LIST, SAI_ACL_ENTRY_ATTR_OBJECT_LIST},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_USER_TRAP_ID, SAI_ACL_ENTRY_ATTR_FOUR_BYTES},
    {SAI_ACL_ENTRY_ATTR_ACTION_SET_PACKET_COLOR, SAI_ACL_ENTRY_ATTR_ENUM},
};

sai_acl_rule_attr_type sai_acl_rule_get_attr_type (sai_attr_id_t attribute_id)
{
    uint_t attr = 0;
    static const uint_t attr_elements = (sizeof(sai_acl_rule_translate_attr_type)/
                                         sizeof(sai_acl_rule_translate_attr_type[0]));

    for (attr = 0; attr < attr_elements; attr++) {
         if (attribute_id == sai_acl_rule_translate_attr_type[attr].rule_attr) {
             return (sai_acl_rule_translate_attr_type[attr].attr_type);
         }
    }

    if (sai_acl_rule_udf_field_attr_range(attribute_id)) {
        return SAI_ACL_ENTRY_ATTR_ONE_BYTE_LIST;
    }

    return SAI_ACL_ENTRY_ATTR_INVALID;
}

uint_t sai_acl_max_ifp_slice_get (void)
{
    sai_acl_table_static_config_t *sai_acl_config = NULL;
    sai_acl_config = &acl_node.sai_acl_table_config;
    STD_ASSERT(sai_acl_config != NULL);

    return (sai_acl_config->max_ifp_slice);
}

uint_t sai_acl_entry_depth_get (void)
{
    sai_acl_table_static_config_t *sai_acl_config = NULL;
    sai_acl_config = &acl_node.sai_acl_table_config;
    STD_ASSERT(sai_acl_config != NULL);

    return (sai_acl_config->depth_per_entry);
}

uint_t sai_acl_max_efp_slice_get (void)
{
    sai_acl_table_static_config_t *sai_acl_config = NULL;
    sai_acl_config = &acl_node.sai_acl_table_config;
    STD_ASSERT(sai_acl_config != NULL);

    return (sai_acl_config->max_efp_slice);
}

uint_t sai_acl_fp_slice_depth_get (sai_acl_stage_t stage, sai_uint32_t slice_id)
{
    sai_acl_table_static_config_t *sai_acl_config = NULL;
    sai_acl_config = &acl_node.sai_acl_table_config;
    STD_ASSERT(sai_acl_config != NULL);

    if ((stage == SAI_ACL_STAGE_INGRESS) &&
        (slice_id<sai_acl_config->max_ifp_slice))
        return (sai_acl_config->ifp_slice_depth_list[slice_id]);

    if ((stage == SAI_ACL_STAGE_EGRESS) &&
        (slice_id<sai_acl_config->max_efp_slice))
        return (sai_acl_config->efp_slice_depth_list[slice_id]);

    return 0;
}

