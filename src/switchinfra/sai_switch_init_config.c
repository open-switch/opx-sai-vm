/************************************************************************
* LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file sai_switch_init_config.c
*
* @brief This file contains switch init configuration related APIs.
*        Parses the configuration file and updates the internal data structures.
*
*************************************************************************/

#include "std_config_node.h"
#include "std_assert.h"

#include "saistatus.h"

#include "sai_switch_init_config.h"
#include "sai_switch_common.h"
#include "sai_switch_utils.h"
#include "sai_common_infra.h"
#include "sai_qos_api_utils.h"
#include "sai_acl_utils.h"
#include "sai_acl_rule_utils.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>

static void sai_generic_qos_info_handler(sai_switch_init_config_t *switch_info,
                                         std_config_node_t qos_node)
{
    STD_ASSERT(switch_info != NULL);
    STD_ASSERT(qos_node != NULL);

    SAI_SWITCH_LOG_TRACE("Init config: Generic Switch QOS info handler");

    if(strncmp(std_config_name_get(qos_node), SAI_NODE_NAME_QOS, SAI_MAX_NAME_LEN) == 0) {
        sai_std_config_attr_update(qos_node, SAI_ATTR_MAX_HIERARCHY_NODE_CHILDS,
                                   &switch_info->max_childs_per_hierarchy_node, 0);
        sai_std_config_attr_update(qos_node, SAI_ATTR_HIERARCHY_LEVELS,
                                   &switch_info->max_hierarchy_levels, 0);
        sai_std_config_attr_update(qos_node, SAI_ATTR_TRAFFIC_CLASS,
                                   &switch_info->max_supported_tc, 0);
        sai_std_config_attr_update(qos_node, SAI_ATTR_MAX_BUFFER_SIZE,
                                   &switch_info->max_buffer_size, 0);
        sai_std_config_attr_update(qos_node, SAI_ATTR_MAX_TILE_BUFFER_SIZE,
                                   &switch_info->max_tile_buffer_size, 0);
        sai_std_config_attr_update(qos_node, SAI_ATTR_NUM_PG,
                                   &switch_info->num_pg, 0);
        sai_std_config_attr_update(qos_node, SAI_ATTR_CELL_SIZE,
                                   &switch_info->cell_size, 0);
        sai_std_config_attr_update(qos_node, SAI_ATTR_ING_MAX_BUF_POOLS,
                                   &switch_info->ing_max_buf_pools, 0);
        sai_std_config_attr_update(qos_node, SAI_ATTR_EGR_MAX_BUF_POOLS,
                                   &switch_info->egr_max_buf_pools, 0);
        sai_std_config_attr_update(qos_node, SAI_ATTR_TILES_PER_BUF_POOL,
                                   &switch_info->tiles_per_buf_pool, 0);
        sai_std_config_attr_bool_update(qos_node, SAI_ATTR_HIERARCHY_FIXED,
                                        &switch_info->hierarchy_fixed, 1);
    } else if(strncmp(std_config_name_get(qos_node),
                      SAI_NODE_NAME_PORT_QUEUE, SAI_MAX_NAME_LEN) == 0) {

        sai_std_config_attr_update(qos_node, SAI_ATTR_QUEUE_COUNT,
                                   &switch_info->max_queues_per_port, 0);
        sai_std_config_attr_update(qos_node, SAI_ATTR_UC_COUNT,
                                   &switch_info->max_uc_queues_per_port, 0);
        sai_std_config_attr_update(qos_node, SAI_ATTR_MC_COUNT,
                                   &switch_info->max_mc_queues_per_port, 0);
    } else if(strncmp(std_config_name_get(qos_node),
                      SAI_NODE_NAME_CPU_PORT_QUEUE, SAI_MAX_NAME_LEN) == 0) {

        sai_std_config_attr_update(qos_node, SAI_ATTR_QUEUE_COUNT,
                                   &switch_info->max_queues_per_cpu_port, 0);
        sai_std_config_attr_update(qos_node, SAI_ATTR_UC_COUNT,
                                   &switch_info->max_uc_queues_per_cpu_port, 0);
        sai_std_config_attr_update(qos_node, SAI_ATTR_MC_COUNT,
                                   &switch_info->max_mc_queues_per_cpu_port, 0);
    } else {
        SAI_SWITCH_LOG_TRACE("No valid QOS info available. Using the default QOS value");
    }
}

static sai_status_t sai_generic_switch_info_handler(std_config_node_t switch_node)
{
    std_config_node_t sai_node = NULL;
    sai_status_t ret_code = SAI_STATUS_SUCCESS;
    sai_switch_init_config_t init_info;

    STD_ASSERT(switch_node != NULL);
    SAI_SWITCH_LOG_TRACE("Init config: Generic Switch info handler");
    memset(&init_info, 0, sizeof(sai_switch_init_config_t));

    init_info.max_port_mtu = SAI_MAX_PORT_MTU_DEFAULT;
    init_info.max_tiles = SAI_SWITCH_DEFAULT_MAX_TILES;

    for (sai_node = std_config_get_child(switch_node);
         sai_node != NULL;
         sai_node = std_config_next_node(sai_node)) {

        if(strncmp(std_config_name_get(sai_node), SAI_NODE_NAME_PORT_INFO,
                   SAI_MAX_NAME_LEN) == 0) {

            sai_std_config_attr_update(sai_node, SAI_ATTR_CPU_PORT, &init_info.cpu_port, 0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_MAX_LOGICAL_PORTs,
                                       &init_info.max_logical_ports, 0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_MAX_PHYSICAL_PORTS,
                                       &init_info.max_physical_ports, 0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_MAX_LANE_PER_PORT,
                                       &init_info.max_lane_per_port, 0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_MAX_PORT_MTU,
                                       &init_info.max_port_mtu, 0);

        } else if(strncmp(std_config_name_get(sai_node), SAI_NODE_NAME_TABLE_SIZE,
                          SAI_MAX_NAME_LEN) == 0) {

            sai_std_config_attr_update(sai_node, SAI_ATTR_L2_TABLE_SIZE,
                                       &init_info.l2_table_size, 0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_L3_HOST_TABLE_SIZE,
                                       &init_info.l3_host_table_size, 0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_L3_ROUTE_TABLE_SIZE,
                                       &init_info.l3_route_table_size, 0);
        } else {

            sai_generic_qos_info_handler(&init_info, sai_node);
        }
    }

    /* Update the global switch info based on init config */
    sai_switch_info_initialize(&init_info);

    return ret_code;
}

static sai_status_t sai_generic_info_handler(std_config_node_t generic_node)
{
    std_config_node_t sai_node = NULL;
    uint_t switch_instance = UINT_MAX;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(generic_node != NULL);
    SAI_SWITCH_LOG_TRACE("Init config: Generic info handler");

    for (sai_node = std_config_get_child(generic_node);
         sai_node != NULL;
         sai_node = std_config_next_node(sai_node)) {

        if(strncmp(std_config_name_get(sai_node), SAI_NODE_NAME_SWITCH, SAI_MAX_NAME_LEN) == 0) {

            sai_std_config_attr_update(sai_node, SAI_ATTR_INSTANCE, &switch_instance, 0);

            sai_generic_switch_info_handler(sai_node);
        } else if(strncmp(std_config_name_get(sai_node),
                          SAI_NODE_NAME_HIERARCHY, SAI_MAX_NAME_LEN) == 0) {

            sai_rc = sai_qos_hierarchy_handler(sai_node, sai_qos_default_hqos_get());
            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_SWITCH_LOG_ERR("Default hierarchy info handling failed with err %d", sai_rc);
                return sai_rc;

            }
        } else if(strncmp(std_config_name_get(sai_node),
                          SAI_NODE_NAME_CPU_HIERARCHY, SAI_MAX_NAME_LEN) == 0) {
            sai_rc = sai_qos_hierarchy_handler(sai_node, sai_qos_default_cpu_hqos_get());
            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_SWITCH_LOG_ERR("Default CPU hierarchy handling failed with err %d", sai_rc);
                return sai_rc;
            }
        } else if(strncmp(std_config_name_get(sai_node),
                          SAI_NODE_NAME_ACL_CONFIG, SAI_MAX_NAME_LEN) == 0) {

            SAI_SWITCH_LOG_TRACE("ACL_USAGE: Process ACL config");
            /**Read ACL config and update local data structure*/
            sai_rc = sai_acl_config_handler(sai_node, sai_acl_get_acl_node());
            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_SWITCH_LOG_ERR("Default ACL config handling failed with err %d", sai_rc);
                return sai_rc;
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_switch_init_file_handler(sai_switch_info_t *sai_switch_info,
                                                 std_config_node_t root_node)
{
    std_config_node_t sai_node = NULL;
    char *node_attr = NULL;
    sai_status_t ret_code = SAI_STATUS_FAILURE;

    STD_ASSERT(root_node != NULL);
    for (sai_node = std_config_get_child(root_node);
         sai_node != NULL;
         sai_node = std_config_next_node(sai_node)) {

        if(strncmp(std_config_name_get(sai_node),
                   SAI_NODE_NAME_PLATFORM_INFO, SAI_MAX_NAME_LEN) == 0) {

            node_attr = std_config_attr_get(sai_node, SAI_ATTR_INFO_TYPE);
            if(node_attr == NULL) {
                SAI_SWITCH_LOG_ERR("Init info type is not valid");
                return SAI_STATUS_INVALID_PARAMETER;
            }

            if(strncmp(node_attr, SAI_ATTR_VAL_PLATFORM_INFO_GENERIC, SAI_MAX_NAME_LEN) == 0) {
                ret_code = sai_generic_info_handler(sai_node);
                if(ret_code != SAI_STATUS_SUCCESS) {
                    SAI_SWITCH_LOG_ERR("Generic init config handler failed with err %d", ret_code);
                    return ret_code;
                }

            } else if(strncmp(node_attr, SAI_ATTR_VAL_PLATFORM_INFO_VENDOR,
                              SAI_MAX_NAME_LEN) == 0) {

                ret_code = sai_switch_npu_api_get()->switch_init_config(sai_switch_info, sai_node);
                if(ret_code != SAI_STATUS_SUCCESS) {
                    SAI_SWITCH_LOG_ERR("Vendor init config handler failed with err %d", ret_code);
                    return ret_code;
                }
            } else {

                SAI_SWITCH_LOG_TRACE("Unknown init configuration info type");
            }
        } else {
            SAI_SWITCH_LOG_ERR("Expected init config info is not available");
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    return SAI_STATUS_SUCCESS;
}

/* A Init config file while hold information about a single switch id instance. */
sai_status_t sai_switch_init_config(sai_switch_info_t *sai_switch_info, const char * sai_cfg_file)
{

    std_config_hdl_t cfg_hdl = NULL;
    std_config_node_t root =  NULL;
    sai_status_t ret_code = SAI_STATUS_FAILURE;

    if(sai_cfg_file == NULL) {
        SAI_SWITCH_LOG_ERR("No valid Init config file is passed");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    SAI_SWITCH_LOG_TRACE("Handling the Switch Init config file %s", sai_cfg_file);

    cfg_hdl = std_config_load(sai_cfg_file);
    if(cfg_hdl == NULL) {
        SAI_SWITCH_LOG_ERR("No valid Init config file is passed");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    root =  std_config_get_root(cfg_hdl);
    STD_ASSERT(root != NULL);

    ret_code = sai_switch_init_file_handler(sai_switch_info, root);
    if(ret_code != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_ERR("Switch Init file handler failed with err %d", ret_code);
    }

    /* Unload the config file handle since configuration
     * parsing is done */
    std_config_unload(cfg_hdl);

    return ret_code;
}
