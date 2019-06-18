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
 * @file sai_vm_npu_init_config.c
 *
 * @brief This file contains vendor specific VM switch init configuration related APIs.
 *        Parses the vendor configuration node and updates the internal data structures.
 */

#include "std_config_node.h"
#include "std_assert.h"

#include "sai_switch_init_config.h"
#include "sai_switch_common.h"
#include "sai_switch_utils.h"
#include "sai_port_utils.h"
#include "sai_vm_port_util.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>

static void sai_vm_port_info_node_handler (std_config_node_t port_node,
                                           sai_vm_port_init_info_t *port_info)
{
    std_config_node_t sai_node = NULL;
    char *node_attr = NULL;

    STD_ASSERT (port_node != NULL);
    STD_ASSERT (port_info != NULL);

    /* Set port info defaults */
    port_info->eee_support = false;
    for (sai_node = std_config_get_child (port_node);
         sai_node != NULL;
         sai_node = std_config_next_node (sai_node)) {

        if (strncmp (std_config_name_get (sai_node), SAI_NODE_NAME_ID, SAI_MAX_NAME_LEN) == 0) {

            sai_std_config_attr_bool_update (sai_node, SAI_ATTR_BIT_VALID,
                                             (bool *) &(port_info->port_valid), 1);

            sai_std_config_attr_update (sai_node,
                                        SAI_ATTR_LOGICAL_PORT, &port_info->local_port_id, 0);
            sai_std_config_attr_update (sai_node, SAI_ATTR_HW_PORT, &port_info->phy_port_id, 0);
            sai_std_config_attr_update (sai_node, SAI_ATTR_PORT_GROUP, &port_info->port_group, 0);

            /* @todo handle external PHY ports */
            node_attr = std_config_attr_get(sai_node, SAI_ATTR_PHY_TYPE);
            if(node_attr == NULL) {
                port_info->phy_type = SAI_PORT_PHY_INTERNAL;
                port_info->ext_phy_addr = 0;
            }

        } else if (strncmp (std_config_name_get(sai_node),
                            SAI_NODE_NAME_SUPPORTED_SPEED, SAI_MAX_NAME_LEN) == 0) {

            size_t speed_map_sz;
            uint32_t index;
            const speed_desc_t* speed_map= get_speed_map(&speed_map_sz);
            port_info->port_speed_capb = 0;
            const speed_desc_t *map_entry = speed_map;

            for (index=0; index < speed_map_sz; ++index) {
                sai_std_config_attr_uint_update(sai_node, map_entry->speed,
                                                &port_info->port_speed_capb,
                                                map_entry->speed_cap_bit);
                ++map_entry;
            }

        } else if (strncmp (std_config_name_get (sai_node),
                            SAI_NODE_NAME_BREAKOUT, SAI_MAX_NAME_LEN) == 0) {

            sai_std_config_attr_uint_update (sai_node, SAI_ATTR_SUPPORT,
                                             &port_info->breakout_modes_support,
                                             SAI_PORT_CAP_BREAKOUT_MODE);

            sai_std_config_attr_uint_update (sai_node, SAI_ATTR_BREAKOUT_LANE_1,
                                             &port_info->breakout_modes_support,
                                             SAI_PORT_CAP_BREAKOUT_MODE_1X);

            sai_std_config_attr_uint_update (sai_node, SAI_ATTR_BREAKOUT_LANE_2,
                                             &port_info->breakout_modes_support,
                                             SAI_PORT_CAP_BREAKOUT_MODE_2X);

            sai_std_config_attr_uint_update (sai_node, SAI_ATTR_BREAKOUT_LANE_4,
                                             &port_info->breakout_modes_support,
                                             SAI_PORT_CAP_BREAKOUT_MODE_4X);

            node_attr = std_config_attr_get(sai_node, SAI_ATTR_DEFAULT_BREAKOUT_MODE);

            if (node_attr != NULL) {
                if (strncmp(node_attr, SAI_ATTR_BREAKOUT_LANE_1, strlen(node_attr)) == 0) {
                    /* Default non-breakout case */
                   port_info->default_breakout_mode = SAI_PORT_CAP_BREAKOUT_MODE_1X;
                } else if (strncmp(node_attr, SAI_ATTR_BREAKOUT_LANE_2,strlen(node_attr)) == 0) {
                   port_info->default_breakout_mode = (SAI_PORT_CAP_BREAKOUT_MODE |
                                                                 SAI_PORT_CAP_BREAKOUT_MODE_2X);
                } else if (strncmp(node_attr, SAI_ATTR_BREAKOUT_LANE_4,strlen(node_attr)) == 0) {
                   port_info->default_breakout_mode = (SAI_PORT_CAP_BREAKOUT_MODE |
                                                                 SAI_PORT_CAP_BREAKOUT_MODE_4X);
                }
            } else {
                port_info->default_breakout_mode = SAI_PORT_CAP_BREAKOUT_MODE_1X;
            }

        } else if (strncmp (std_config_name_get (sai_node),
                            SAI_NODE_NAME_LANE, SAI_MAX_NAME_LEN) == 0) {

            sai_std_config_attr_update (sai_node, SAI_ATTR_SPEED, &port_info->port_speed, 0);
            sai_std_config_attr_update (sai_node, SAI_ATTR_COUNT,
                                        &port_info->max_lanes_per_port, 0);
            sai_std_config_attr_update (sai_node, SAI_ATTR_BIT_MAP,
                                        (uint_t *)&port_info->port_lane_bmap, SAI_BASE_HEX);
        } else {
            SAI_PORT_LOG_ERR("Not a valid port init config node");
        }
    }
}

static sai_status_t sai_vm_port_info_handler (std_config_node_t port_node,
                                              sai_vm_port_init_info_t *port_info_tbl,
                                              uint_t max_port_instance)
{
    uint_t port_instance = UINT_MAX;
    uint_t port_instance_id = UINT_MAX;
    char   *node_attr = NULL;
    static uint_t g_port_instance_id = 0;

    STD_ASSERT(port_info_tbl != NULL);
    STD_ASSERT (port_node != NULL);

    sai_std_config_attr_update (port_node, SAI_ATTR_INSTANCE, &port_instance_id, 0);
    if (port_instance_id == UINT_MAX) {
        SAI_PORT_LOG_TRACE("No valid port instance id passed from init config");
        port_instance_id = g_port_instance_id;
    }

    if (g_port_instance_id >= max_port_instance) {
        SAI_PORT_LOG_ERR ("Port instance %d is more than the max possible port instance %d",
                          g_port_instance_id, max_port_instance);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    port_instance = g_port_instance_id;

    port_info_tbl [port_instance].port_inst_id = port_instance_id + 1;

    SAI_PORT_LOG_TRACE ("VM Port instance %d init conifg handler", port_instance);

    node_attr = std_config_attr_get(port_node, SAI_ATTR_BIT_ACTIVE);
    if (node_attr == NULL) {
       /* Port assumed to be active unless it is set to false in init config */
        port_info_tbl [port_instance].port_active = true;
    } else {
        sai_std_config_attr_bool_update(port_node, SAI_ATTR_BIT_ACTIVE,
                                        &((port_info_tbl + port_instance)->port_active), 1);
    }

    g_port_instance_id++;

    sai_vm_port_info_node_handler (port_node, (port_info_tbl + port_instance));

    return SAI_STATUS_SUCCESS;
}

/* Update common port level info based on the Hardware */
sai_status_t sai_vm_port_info_update (sai_vm_port_init_info_t *vm_init_info)
{
    t_std_error rc = STD_ERR_OK;
    sai_port_info_t *port_info = NULL;
    sai_npu_port_id_t pport = 0;
    sai_status_t ret_code = SAI_STATUS_SUCCESS;
    uint_t max_pport = (sai_switch_get_max_pport () *
                        sai_switch_get_max_lane_per_port ());

    SAI_PORT_LOG_TRACE ("Port HW common info update");
    STD_ASSERT (vm_init_info != NULL);

    /* Initialize common port info table */
    ret_code = sai_port_info_init();
    if(ret_code != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_ERR("Common port info table update failed with err %d", ret_code);
        return ret_code;
    }

    sai_port_info_table_t port_info_table = sai_port_info_table_get ();
    STD_ASSERT (port_info_table != NULL);

    for (pport = 0; pport < max_pport; pport++) {

        /* Skip the disabled port */
        if ( !(vm_init_info[pport].port_active)) {
            continue;
        }

        port_info = (sai_port_info_t *) calloc (1, sizeof (sai_port_info_t));
        STD_ASSERT (port_info != NULL);

        port_info->port_valid = vm_init_info[pport].port_valid;
        port_info->phy_port_id = vm_init_info[pport].phy_port_id;
        port_info->port_group = vm_init_info[pport].port_group;
        port_info->phy_type = vm_init_info[pport].phy_type;
        port_info->fwd_mode = SAI_PORT_FWD_MODE_UNKNOWN;
        port_info->ext_phy_addr = vm_init_info[pport].ext_phy_addr;
        port_info->port_supported_capb = vm_init_info[pport].breakout_modes_support;
        port_info->port_enabled_capb = vm_init_info[pport].default_breakout_mode;
        port_info->max_lanes_per_port = vm_init_info[pport].max_lanes_per_port;
        port_info->port_lane_bmap = vm_init_info[pport].port_lane_bmap;
        port_info->port_speed = vm_init_info[pport].port_speed;
        port_info->port_speed_capb = vm_init_info[pport].port_speed_capb;
        port_info->eee_support = vm_init_info[pport].eee_support;
        port_info->media_type = SAI_PORT_MEDIA_TYPE_NOT_PRESENT;
        port_info->local_port_id = vm_init_info[pport].local_port_id;
        port_info->sai_port_id = sai_port_id_create (SAI_PORT_TYPE_LOGICAL,
                                                     sai_switch_id_get (),
                                                     port_info->local_port_id);

        /* Insert the new port_info node into the port_info_table Tree */
        rc = std_rbtree_insert (port_info_table, port_info);
        if (STD_IS_ERR (rc)) {
            SAI_PORT_LOG_ERR ("Error in inserting sai port id 0x%"PRIx64" "
                              "with err %d", port_info->sai_port_id, rc);

            free (port_info);
            port_info = NULL;
            return SAI_STATUS_FAILURE;
        }
    }

    return ret_code;
}

static void sai_vm_switch_info_handler (std_config_node_t switch_node)
{
    uint_t switch_capb = 0;
    std_config_node_t sai_node = NULL;

    STD_ASSERT (switch_node != NULL);
    SAI_SWITCH_LOG_TRACE ("Init config: VM Switch info handler");

    for (sai_node = std_config_get_child(switch_node);
         sai_node != NULL;
         sai_node = std_config_next_node (sai_node)) {

        if (strncmp(std_config_name_get(sai_node),
                    SAI_NODE_NAME_CAPABILITIES, SAI_MAX_NAME_LEN) == 0) {

            sai_std_config_attr_update (sai_node, SAI_ATTR_BIT_MASK,
                                        &switch_capb, SAI_BASE_HEX);
        }
    }

    sai_switch_supported_capability_set((uint_t) switch_capb);
}

sai_status_t sai_npu_switch_init_config (sai_switch_info_t *sai_switch_info,
                                         std_config_node_t vm_node)
{
    std_config_node_t sai_node = NULL;
    sai_status_t ret_code = SAI_STATUS_FAILURE;
    uint_t max_port_instance = 0, switch_instance = UINT_MAX;
    sai_vm_port_init_info_t *port_init_config = NULL;
    sai_switch_id_t switch_id = sai_switch_info->switch_id;

    STD_ASSERT (vm_node != NULL);
    SAI_SWITCH_LOG_TRACE ("Init config: vendor VM init config handler");

    max_port_instance = (sai_switch_get_max_pport() *
                         sai_switch_get_max_lane_per_port());

    port_init_config = (sai_vm_port_init_info_t *) calloc (max_port_instance,
                                                           sizeof (sai_vm_port_init_info_t));

    do {
        for (sai_node = std_config_get_child (vm_node);
             sai_node != NULL;
             sai_node = std_config_next_node (sai_node)) {

            if (strncmp (std_config_name_get (sai_node),
                         SAI_NODE_NAME_SWITCH, SAI_MAX_NAME_LEN) == 0) {

                sai_std_config_attr_update (sai_node, SAI_ATTR_INSTANCE, &switch_instance, 0);

                /* A config file will contain only one switch instance;
                 * On instance mismatch it is expected to return failure*/
                if (switch_instance != switch_id) {
                    SAI_SWITCH_LOG_ERR ("Switch instance id %d in the config doesn't match"
                                        "the Hardware id %d", switch_instance, switch_id);
                    ret_code = SAI_STATUS_FAILURE;
                    break;
                }

                sai_vm_switch_info_handler (sai_node);

            } else if (strncmp (std_config_name_get (sai_node),
                                SAI_NODE_NAME_PORT, SAI_MAX_NAME_LEN) == 0) {

                ret_code = sai_vm_port_info_handler (sai_node, port_init_config,
                                                     max_port_instance);
                if (ret_code != SAI_STATUS_SUCCESS) {
                    SAI_SWITCH_LOG_ERR ("VM port info handling failed with err %d", ret_code);
                    return ret_code;
                    break;
                }
            } else {
                SAI_PORT_LOG_ERR("Not a valid switch init config node");
            }

        }

        if (ret_code != SAI_STATUS_SUCCESS) {
            break;
        }

        /* Initialize and update common port level info based on the Init config */
        ret_code = sai_vm_port_info_update(port_init_config);
        if(ret_code != SAI_STATUS_SUCCESS) {
            SAI_SWITCH_LOG_ERR("Common port info table update failed with err %d", ret_code);
            break;
        }

    } while (0);

    free(port_init_config);
    port_init_config = NULL;

    return ret_code;
}
