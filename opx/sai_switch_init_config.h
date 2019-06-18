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
* @file sai_switch_init_config.h
*
* @brief This file contains the definitions and Utility APIs for SAI Switch
*        init config file handling
*
*************************************************************************/

/* OPENSOURCELICENSE */

#ifndef __SAI_SWITCH_INIT_CONFIG_H__
#define __SAI_SWITCH_INIT_CONFIG_H__

#include "std_type_defs.h"
#include "std_config_node.h"
#include "std_assert.h"
#include "saiqueue.h"

#include <string.h>
#include <stdlib.h>

/**
 * @def SAI_INIT_CONFIG_FILE
 * Attribute used to get SAI initialization config file path
 * @todo File path should be passed from Adapter Host and this
 * macro should be removed.
 */
#define SAI_INIT_CONFIG_FILE               "/etc/opx/sai/init.xml"

/**
 * @def SAI_ATTR_INFO_TYPE
 * Attribute used for representing the info type
 */
#define SAI_ATTR_INFO_TYPE                 "type"

/**
 * @def SAI_ATTR_INSTANCE
 * Attribute used for representing the node instance
 */
#define SAI_ATTR_INSTANCE                  "instance"

/**
 * @def SAI_ATTR_CPU_PORT
 * Attribute used for representing the CPU port
 */
#define SAI_ATTR_CPU_PORT                  "cpu_port"

/**
 * @def SAI_ATTR_MAX_LOGICAL_PORTs
 * Attribute used for representing the max logical ports count
 */
#define SAI_ATTR_MAX_LOGICAL_PORTs         "max_logical_ports"

/**
 * @def SAI_ATTR_MAX_PHYSICAL_PORTS
 * Attribute used for representing the max physical ports count
 */
#define SAI_ATTR_MAX_PHYSICAL_PORTS        "max_physical_ports"

/**
 * @def SAI_ATTR_MAX_LANE_PER_PORT
 * Attribute used for representing the max number of lanes in a port
 */
#define SAI_ATTR_MAX_LANE_PER_PORT         "max_lane_per_port"

/**
 * @def SAI_ATTR_MAX_PORT_MTU
 * Attribute used for representing the max mtu of port in bytes
 */
#define SAI_ATTR_MAX_PORT_MTU              "max_port_mtu"

/**
 * @def SAI_ATTR_L2_TABLE_SIZE
 * Attribute used for representing the L2 table size
 */
#define SAI_ATTR_L2_TABLE_SIZE             "l2_table"

/**
 * @def SAI_ATTR_L3_HOST_TABLE_SIZE
 * Attribute used for representing the L3 host table size
 */
#define SAI_ATTR_L3_HOST_TABLE_SIZE        "l3_host_table"

/**
 * @def SAI_ATTR_L3_ROUTE_TABLE_SIZE
 * Attribute used for representing the L3 route table size
 */
#define SAI_ATTR_L3_ROUTE_TABLE_SIZE       "l3_route_table"

/**
 * @def SAI_ATTR_QUEUE_COUNT
 * Attribute used for representing the max queue count
 */
#define SAI_ATTR_QUEUE_COUNT               "queue_count"

/**
 * @def SAI_ATTR_UC_COUNT
 * Attribute used for representing the Unicast queue count per port
 */
#define SAI_ATTR_UC_COUNT                  "uc_count"

/**
 * @def SAI_ATTR_MC_COUNT
 * Attribute used for representing the Multicast queuE count per port
 */
#define SAI_ATTR_MC_COUNT                  "mc_count"

/**
 * @def SAI_ATTR_HIERARCHY_LEVELS
 * Attribute used for representing the max Hierarchy scheduler group
 * levels supported per port
 */
#define SAI_ATTR_HIERARCHY_LEVELS          "hierarchy_levels"

/**
 * @def SAI_ATTR_HIERARCHY_FIXED
 * Attribute used for representing scheduler group hierarchy is fixed/flexible.
 *
 */
#define SAI_ATTR_HIERARCHY_FIXED          "hierarchy_fixed"

/**
 * @def SAI_ATTR_MAX_BUFFER_SIZE
 * Attribute used for representing the max buffer size
 * supported per NPU
 */
#define SAI_ATTR_MAX_BUFFER_SIZE           "max_buffer_size"

/**
 * @def SAI_ATTR_MAX_TILE_BUFFER_SIZE
 * Attribute used for representing the max buffer size per tile
 */
#define SAI_ATTR_MAX_TILE_BUFFER_SIZE           "max_tile_buffer_size"

/**
 * @def SAI_ATTR_MAX_HIERARCHY_CHILDS
 * Attribute used for representing the max childs supported per each hierarchy node
 *
 */
#define SAI_ATTR_MAX_HIERARCHY_NODE_CHILDS  "max_hierarchy_node_childs"

/**
 * @def SAI_ATTR_TRAFFIC_CLASS
 * Attribute used for representing the max traffic classes supported
 */
#define SAI_ATTR_TRAFFIC_CLASS             "traffic_class"


/**
 * @def SAI_ATTR_NUM_PG
 * Attribute used for representing the number of Priority groups
 * supported per NPU
 */
#define SAI_ATTR_NUM_PG           "num_pg"

/**
 * @def SAI_ATTR_CELL_SIZE
 * Attribute used for representing the size of buffer cell in MMU
 */
#define SAI_ATTR_CELL_SIZE           "cell_size"

/**
 * @def SAI_ATTR_ING_MAX_BUF_POOLS
 * Attribute used for representing the maximum number of service pools at ingress
 */
#define SAI_ATTR_ING_MAX_BUF_POOLS           "ing_max_buf_pools"

/**
 * @def SAI_ATTR_EGR_MAX_BUF_POOLS
 * Attribute used for representing the maximum number of service pools at egress
 */
#define SAI_ATTR_EGR_MAX_BUF_POOLS           "egr_max_buf_pools"

/**
 * @def SAI_ATTR_TILES_PER_BUF_POOL
 * Attribute used for representing the tiles per buffer pool
 */
#define SAI_ATTR_TILES_PER_BUF_POOL          "tiles_per_buf_pool"


/**
 * @def SAI_ATTR_BIT_MASK
 * Attribute used for representing the bitmask
 */
#define SAI_ATTR_BIT_MASK                  "bitmask"

/**
 * @def  SAI_ATTR_BIT_ACTIVE
 * Attribute used for representing the port state active or
 * disabled from initializing
 */
#define SAI_ATTR_BIT_ACTIVE                "active"

/**
 * @def SAI_ATTR_BIT_VALID
 * Attribute used for representing the port default state valid or invalid.
 */
#define SAI_ATTR_BIT_VALID                 "valid"

/**
 * @def SAI_ATTR_SW_LINKSCAN_INTERVAL
 * Attribute used for representing the software linkscan polling interval in micro seconds
 */
#define SAI_ATTR_SW_LINKSCAN_INTERVAL      "interval"

/**
 * @def SAI_ATTR_LOGICAL_PORT
 * Attribute used for representing the logical_port id
 */
#define SAI_ATTR_LOGICAL_PORT              "logical_port"

/**
 * @def SAI_ATTR_DPORT
 * Attribute used for representing the dport id
 */
#define SAI_ATTR_DPORT                     "dport"

/**
 * @def SAI_ATTR_HW_PORT
 * Attribute used for representing the hardware port id
 */
#define SAI_ATTR_HW_PORT                   "hw_port"

/**
 * @def SAI_ATTR_PORT_GROUP
 * Attribute used for representing the port group, a port instance is part of
 */
#define SAI_ATTR_PORT_GROUP                "port_group"

/**
 * @def SAI_ATTR_SUPPORT
 * Attribute used for representing the feature support
 */
#define SAI_ATTR_SUPPORT                   "support"

/**
 * @def SAI_ATTR_BREAKOUT_LANE_1
 * Attribute used for representing the 1 lane breakout support
 */
#define SAI_ATTR_BREAKOUT_LANE_1           "lane_1"

/**
 * @def SAI_ATTR_BREAKOUT_LANE_2
 * Attribute used for representing the 2 lane breakout support
 */
#define SAI_ATTR_BREAKOUT_LANE_2           "lane_2"

/**
 * @def SAI_ATTR_BREAKOUT_LANE_4
 * Attribute used for representing the 4 lane breakout support
 */
#define SAI_ATTR_BREAKOUT_LANE_4           "lane_4"

/**
 * @def SAI_ATTR_DEFAULT_BREAKOUT_MODE
 * Attribute representing the default breakout mode
 */
#define SAI_ATTR_DEFAULT_BREAKOUT_MODE "default"

/**
 * @def SAI_ATTR_SPEED
 * Attribute used for representing the speed in Gbps
 */
#define SAI_ATTR_SPEED                     "speed"

/**
 * @def SAI_ATTR_COUNT
 * Attribute used for representing the count in a node
 */
#define SAI_ATTR_COUNT                     "count"

/**
 * @def SAI_ATTR_BIT_MAP
 * Attribute used for representing the port lane bitmap
 */
#define SAI_ATTR_BIT_MAP                   "bitmap"

/**
 * @def SAI_NODE_PHY_TYPR
 * Node for representing PHY type as internal or external
 */
#define SAI_ATTR_PHY_TYPE                  "phy_typeE"

/**
 * @def SAI_ATTR_VAL_PLATFORM_INFO_GENERIC
 * Attribute values for representing generic platform info
 */
#define SAI_ATTR_VAL_PLATFORM_INFO_GENERIC "generic"

/**
 * @def SAI_ATTR_VAL_PLATFORM_INFO_VENDOR
 * Attribute values for representing vendor specific platform info
 */
#define SAI_ATTR_VAL_PLATFORM_INFO_VENDOR  "vendor"

/**
 * @def SAI_ATTR_VAL_SPEED_10M
 * Attribute value for representing port supported speed value of 10M
 */
#define SAI_ATTR_VAL_SPEED_10M             "speed10M"

/**
 * @def SAI_ATTR_VAL_SPEED_100M
 * Attribute value for representing port supported speed value of 100M
 */
#define SAI_ATTR_VAL_SPEED_100M            "speed100M"

/**
 * @def SAI_ATTR_VAL_SPEED_1G
 * Attribute value for representing port supported speed value of 1G
 */
#define SAI_ATTR_VAL_SPEED_1G              "speed1G"

/**
 * @def SAI_ATTR_VAL_SPEED_10G
 * Attribute value for representing port supported speed value of 10G
 */
#define SAI_ATTR_VAL_SPEED_10G             "speed10G"

/**
 * @def SAI_ATTR_VAL_SPEED_25G
 * Attribute value for representing port supported speed value of 25G
 */
#define SAI_ATTR_VAL_SPEED_25G             "speed25G"

/**
 * @def SAI_ATTR_VAL_SPEED_40G
 * Attribute value for representing port supported speed value of 40G
 */
#define SAI_ATTR_VAL_SPEED_40G             "speed40G"

/**
 * @def SAI_ATTR_VAL_SPEED_42G
 * Attribute value for representing port supported speed value of 42G
 */
#define SAI_ATTR_VAL_SPEED_42G             "speed42G"

/**
 * @def SAI_ATTR_VAL_SPEED_50G
 * Attribute value for representing port supported speed value of 50G
 */
#define SAI_ATTR_VAL_SPEED_50G             "speed50G"

/**
 * @def SAI_ATTR_VAL_SPEED_100G
 * Attribute value for representing port supported speed value of 100G
 */
#define SAI_ATTR_VAL_SPEED_100G            "speed100G"

/**
 * @def SAI_ATTR_ENABLE
 * Attribute value for representing enable/disable port configuration
 */
#define SAI_ATTR_ENABLE                    "enable"

/**
 * @def SAI_NODE_NAME_PLATFORM_INFO
 * Node for representing platform specific information
 */
#define SAI_NODE_NAME_PLATFORM_INFO        "PLATFORM_INFO"

/**
 * @def SAI_NODE_NAME_SWITCH
 * Node for representing the NPU switch information
 */
#define SAI_NODE_NAME_SWITCH               "SWITCH"

/**
 * @def SAI_NODE_NAME_PORT
 * Node for representing the port level information
 */
#define SAI_NODE_NAME_PORT                 "PORT"

/**
 * @def SAI_NODE_NAME_PORT_INFO
 * Node for representing the specific port instance information
 */
#define SAI_NODE_NAME_PORT_INFO            "PORT_INFO"

/**
 * @def SAI_NODE_NAME_TABLE_SIZE
 * Node for representing the L2/L3 table size
 */
#define SAI_NODE_NAME_TABLE_SIZE           "TABLE_SIZE"

/**
 * @def SAI_NODE_NAME_QOS
 * Node for representing the QOS related info
 */
#define SAI_NODE_NAME_QOS                  "QOS"

/**
 * @def SAI_NODE_NAME_PORT_QUEUE
 * Node for representing the per port queue related info
 */
#define SAI_NODE_NAME_PORT_QUEUE           "PORT_QUEUE"

/**
 * @def SAI_NODE_NAME_CPU_PORT_QUEUE
 * Node for representing the per CPU port queue related info
 */
#define SAI_NODE_NAME_CPU_PORT_QUEUE        "CPU_PORT_QUEUE"

/**
 * @def SAI_NODE_NAME_CAPABILITIES
 * Node for representing the capabilities of a node
 */
#define SAI_NODE_NAME_CAPABILITIES         "CAPABILITIES"

/**
 * @def SAI_NODE_NAME_SW_LINKSCAN
 * Node for representing the software linkscan paramters of a node
 */
#define SAI_NODE_NAME_SW_LINKSCAN          "SW_LINKSCAN"

/**
 * @def SAI_NODE_NAME_ID
 * Node for representing the identifier
 */
#define SAI_NODE_NAME_ID                   "ID"

/**
 * @def SAI_NODE_NAME_BREAKOUT
 * Node for representing the breakout support related information
 */
#define SAI_NODE_NAME_BREAKOUT             "BREAKOUT"

/**
 * @def SAI_NODE_NAME_LANE
 * Node for representing SerDes lane specific information
 */
#define SAI_NODE_NAME_LANE                 "LANE"

/**
 * @def SAI_NODE_NAME_AUTONEG
 * Node for representing port autoneg configuration
 */
#define SAI_NODE_NAME_AUTONEG              "AUTONEG"

/**
 * @def SAI_NODE_NAME_HIERARCHY
 * Node for representing the Qos Hierarchy tree
 */
#define SAI_NODE_NAME_HIERARCHY            "SCHEDULER_HIERARCHY_TREE"

/**
 * @def SAI_NODE_NAME_CPU_HIERARCHY
 * Node for representing the Cpu Qos Hierarchy tree
 */
#define SAI_NODE_NAME_CPU_HIERARCHY        "CPU_SCHEDULER_HIERARCHY_TREE"

/**
 * @def SAI_NODE_NAME_SCHED_GRP
 * Node for representing scheduler group in the Qos Hierarchy tree
 */
#define SAI_NODE_NAME_SCHED_GRP            "SCHEDULER_GROUP"

/**
 * @def SAI_NODE_NAME_CHILD_SCHED_GRP
 * Node for representing scheduler group in the Qos Hierarchy tree
 */
#define SAI_NODE_NAME_CHILD_SCHED_GRP      "child-scheduler-group"

/**
 * @def SAI_NODE_NAME_CHILD_QUEUE
 * Node for representing scheduler group in the Qos Hierarchy tree
 */
#define SAI_NODE_NAME_CHILD_QUEUE          "child-queue"
/**
 * @def SAI_ATTR_HQOS_LEVELS
 * Attribute used for representing the num levels in hqos
 */
#define SAI_ATTR_HQOS_NUM_LEVELS           "num_levels"

/**
 * @def SAI_ATTR_HQOS_LEVELS
 * Attribute used for representing the current level
 */
#define SAI_ATTR_HQOS_LEVEL                "level_idx"

/**
 * @def SAI_ATTR_HQOS_NUM_SCHED_GRPS
 * Attribute used for representing the number of scheduler groups in current level
 */
#define SAI_ATTR_HQOS_NUM_SCHED_GRPS       "scheduler_group_count"

/**
 * @def SAI_ATTR_HQOS_INDEX
 * Attribute used for representing index of the node
 */
#define SAI_ATTR_HQOS_INDEX                "index"

/**
 * @def SAI_ATTR_HQOS_NUM_CHILD
 * Attribute used for representing child count
 */
#define SAI_ATTR_HQOS_NUM_CHILD            "child_count"


/**
 * @def SAI_ATTR_HQOS_CHILD_LEVEL
 * Attribute used for representing child level
 */
#define SAI_ATTR_HQOS_CHILD_LEVEL          "child_level"

/**
 * @def SAI_ATTR_HQOS_QUEUE_NUMBER
 * Attribute used for representing queue number
 */
#define SAI_ATTR_HQOS_QUEUE_NUMBER         "queue-number"

/**
 * @def SAI_ATTR_HQOS_QUEUE_NUMBER
 * Attribute used for representing queue type
 */
#define SAI_ATTR_HQOS_QUEUE_TYPE           "type"

/**
 * @def SAI_NODE_NAME_SUPPORTED_SPEED
 * Node for representing port supported speed capability
 */
#define SAI_NODE_NAME_SUPPORTED_SPEED      "SUPPORTED_SPEED"

/**
 * @def SAI_NODE_NAME_AUTONEG
 * Node for representing port autoneg configuration
 */
#define SAI_NODE_NAME_AUTONEG              "AUTONEG"

/**
 * @def SAI_NODE_NAME_FULLDUPLEX
 * Node for representing port fullduplex configuration
 */
#define SAI_NODE_NAME_FULLDUPLEX           "FULLDUPLEX"

/**
 * @def SAI_NODE_NAME_ACL_CONFIG
 * Node for representing ACL configurations
 */
#define SAI_NODE_NAME_ACL_CONFIG           "ACL_CONFIG"
/**
 * @def SAI_NODE_NAME_ACL_PRIO
 * Node for representing ACL priority configurations
 */
#define SAI_NODE_NAME_ACL_PRIO             "ACL_PRIO"

#define SAI_NODE_NAME_ACL_INGRESS_FP       "ACL_INGRESS_FP"

#define SAI_NODE_NAME_ACL_EGRESS_FP        "ACL_EGRESS_FP"

#define SAI_NODE_NAME_ACL_FP_SLICE         "FP_SLICE"

/**
 * @def SAI_ATTR_ACL_XXX
 * Attribute for representing ACL group names
 */
#define SAI_ATTR_ACL_INGRESS_SYS_FLOW      "system-flow"
#define SAI_ATTR_ACL_INGRESS_OPENFLOW      "openflow"
#define SAI_ATTR_ACL_INGRESS_VLT           "vlt"
#define SAI_ATTR_ACL_INGRESS_ISCSI         "iscsi"
#define SAI_ATTR_ACL_INGRESS_FCOE          "fcoe"
#define SAI_ATTR_ACL_INGRESS_FCOE_FPORT    "fcoe_fport"
#define SAI_ATTR_ACL_INGRESS_FEDGOV        "fedgov"
#define SAI_ATTR_ACL_INGRESS_L2_ACL        "l2_acl"
#define SAI_ATTR_ACL_INGRESS_V4_ACL        "v4_acl"
#define SAI_ATTR_ACL_INGRESS_V6_ACL        "v6_acl"
#define SAI_ATTR_ACL_INGRESS_V4_PBR        "v4_pbr"
#define SAI_ATTR_ACL_INGRESS_V6_PBR        "v6_pbr"
#define SAI_ATTR_ACL_INGRESS_L2_QOS        "l2_qos"
#define SAI_ATTR_ACL_INGRESS_V4_QOS        "v4_qos"
#define SAI_ATTR_ACL_INGRESS_V6_QOS        "v6_qos"
#define SAI_ATTR_ACL_EGRESS_L2_ACL         "egr_l2_acl"
#define SAI_ATTR_ACL_EGRESS_V4_ACL         "egr_v4_acl"
#define SAI_ATTR_ACL_EGRESS_V6_ACL         "egr_v6_acl"

/**
 * @def SAI_ATTR_ACL_MAX_IFP_SLICE
 * Attribute used for representing the max ifp slice
 */
#define SAI_ATTR_ACL_MAX_IFP_SLICE          "max_ifp_slice"

/**
 * @def SAI_ATTR_ACL_MAX_EFP_SLICE
 * Attribute used for representing the max efp slice
 */
#define SAI_ATTR_ACL_MAX_EFP_SLICE          "max_efp_slice"

#define SAI_ATTR_ACL_FP_SLICE_ID            "slice_id"

#define SAI_ATTR_ACL_FP_SLICE_DEPTH         "slice_depth"

#define SAI_ATTR_ACL_DEPTH_PER_ENTRY        "depth_per_entry"

/**
 * @def SAI_MAX_NAME_LEN
 * MAX SAI Configuration name length
 */
#define SAI_MAX_NAME_LEN (64)

/**
 * @def SAI_BASE_HEX
 * SAI base hex value in decimals
 */
#define SAI_BASE_HEX     (16)

/**
 * @def SAI_MAX_PORT_MTU_DEFAULT
 * MAX SAI Port MTU default
 */
#define SAI_MAX_PORT_MTU_DEFAULT (12000)

/**
 * @def SAI_SWITCH_DEFAULT_MAX_TILES
 * MAX SAI switch default max tiles
 */
#define SAI_SWITCH_DEFAULT_MAX_TILES (1)

/**
 * @def SAI_SWITCH_DEFAULT_MAX_IFP_SLICE
 * MAX SAI switch default max ifp slice
 */
#define SAI_SWITCH_DEFAULT_MAX_IFP_SLICE (12)
/**
 * @def SAI_SWITCH_DEFAULT_MAX_EFP_SLICE
 * MAX SAI switch default max efp slice
 */
#define SAI_SWITCH_DEFAULT_MAX_EFP_SLICE (4)

/**
 * @brief This api takes a config node and attribute to be looked in that node;
 *        if attr value is true, assign the input value to the data.
 *
 * @param[in] node config node
 * @param[in] attr string to be searched in the config node
 * @param[inout] data pointer to the data to be updated
 * @param[in] value value to be assigned in the data if true
 */
static inline void sai_std_config_attr_bool_update(std_config_node_t node,
                                                   const char *attr,
                                                   bool *data, bool value)
{
    char *node_attr = NULL;

    STD_ASSERT(attr != NULL);
    STD_ASSERT(data != NULL);

    node_attr = std_config_attr_get(node, attr);
    if(node_attr != NULL) {
        if(strncmp(node_attr, "true", SAI_MAX_NAME_LEN) == 0) {
            *data = value;
        }
    }
}

/**
 * @brief This api takes a config node and attribute to be looked in that node;
 *        if attr value is true, it appends the input bitmaps value to the data.
 *
 * @param[in] node config node
 * @param[in] attr string to be searched in the config node
 * @param[inout] data pointer to the data to be updated
 * @param[in] value bitmap value to be appended in the data if true
 */
static inline void sai_std_config_attr_uint_update(std_config_node_t node,
                                                   const char *attr,
                                                   uint_t *data, uint_t value)
{
    char *node_attr = NULL;

    STD_ASSERT(attr != NULL);
    STD_ASSERT(data != NULL);

    node_attr = std_config_attr_get(node, attr);
    if(node_attr != NULL) {
        if(strncmp(node_attr, "true", SAI_MAX_NAME_LEN) == 0) {
            *data |= value;
        }
    }
}

/**
 * @brief This api takes a config node and attribute to be looked in that node;
 *        if attr is present, it converts the string to ul according to the given
 *        base value and assigns the converted value to data.
 *
 * @param[in] node config node
 * @param[in] attr string to be searched in the config node
 * @param[out] data pointer to the data to be updated
 * @param[in] base base value for string to ul conversion
 */
static inline void sai_std_config_attr_update(std_config_node_t node,
                                              const char *attr,
                                              uint_t *data, uint_t base)
{
    char *node_attr = NULL;
    STD_ASSERT(data != NULL);
    STD_ASSERT(attr != NULL);

    node_attr = std_config_attr_get(node, attr);
    if(node_attr != NULL) {
        *data = (uint_t) strtoul(node_attr, NULL, base);
    }
}

/**
 * @brief This api takes a config node and attribute to be looked in that node;
 *        if attr is present, it converts the string to ul according to the given
 *        base value and assigns the converted value to data.
 *
 * @param[in] node config node
 * @param[in] attr string to be searched in the config node
 * @param[out] data pointer to the data to be updated
 * @param[in] base base value for string to l conversion
 */
static inline void sai_std_config_attr_int_update(std_config_node_t node,
                                                  const char *attr,
                                                  int *data, uint_t base)
{
    char *node_attr = NULL;
    STD_ASSERT(data != NULL);
    STD_ASSERT(attr != NULL);

    node_attr = std_config_attr_get(node, attr);
    if(node_attr != NULL) {
        *data = (int)strtol(node_attr, NULL, base);
    }
}

/**
 * @brief This api takes a config node and attribute to be looked in that node;
 *        if attr is present, it converts the string to ul according to the given
 *        base value and assigns the converted value to data.
 *
 * @param[in] node config node
 * @param[in] attr string to be searched in the config node
 * @param[out] data pointer to the data to be updated
 * @param[in] base base value for string to ul conversion
 */
static inline void sai_std_config_attr_update_str(std_config_node_t node,
                                              const char *attr,
                                              uint_t *data, uint_t base)
{
    char *node_attr = NULL;
    STD_ASSERT(data != NULL);
    STD_ASSERT(attr != NULL);

    node_attr = std_config_attr_get(node, attr);
    if(node_attr != NULL) {
        if(strncmp(node_attr, "UCAST", SAI_MAX_NAME_LEN) == 0) {
            *data = SAI_QUEUE_TYPE_UNICAST;
        }
        else if(strncmp(node_attr, "MCAST", SAI_MAX_NAME_LEN) == 0) {
            *data = SAI_QUEUE_TYPE_MULTICAST;
        }
    }
}

#endif /* __SAI_SWITCH_INIT_CONFIG_H__ */

