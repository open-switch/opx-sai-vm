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
* @file sai_hostintf_debug.c
*
* @brief This file contains implementations SAI Host Interface functions.
*
*************************************************************************/
#include "sai_hostif_main.h"
#include "sai_hostif_common.h"
#include "sai_common_infra.h"
#include "sai_debug_utils.h"

#include "std_rbtree.h"
#include <inttypes.h>
void sai_hostif_dump_info()
{
    dn_sai_hostintf_info_t *hostif_info = dn_sai_hostintf_get_info();

    SAI_DEBUG("\r\nDumping global hostif information\r\n");
    SAI_DEBUG("Default trap group id = 0x%"PRIx64"",
                         hostif_info->default_trap_group_id);
    SAI_DEBUG("Mandatory send pkt attribute count = %u",
                         hostif_info->mandatory_send_pkt_attr_count);
    SAI_DEBUG("Mandatory trap group attribute count = %u",
                         hostif_info->mandatory_trap_group_attr_count);
    SAI_DEBUG("Mandatory user defined traps count = %u",
                         hostif_info->mandatory_user_def_trap_attr_count);
    SAI_DEBUG("Max pkt attributes = %u",
                         hostif_info->max_pkt_attrs);
    SAI_DEBUG("Max trapgroup attributes = %u",
                         hostif_info->max_trap_group_attrs);
    SAI_DEBUG("Max trap attributes = %u",
                         hostif_info->max_trap_attrs);
    SAI_DEBUG("Max user defined trap attributes = %u",
              hostif_info->max_user_def_trap_attrs);
    SAI_DEBUG("Max user defined traps in NPU = %u",
                         hostif_info->max_user_def_traps);
}

void sai_hostif_dump_traps()
{
    dn_sai_trap_node_t *trap_node = NULL;
    dn_sai_hostintf_info_t *hostif_info = dn_sai_hostintf_get_info();
    uint_t port_count = 0;

    if (NULL == hostif_info->trap_tree) {
        SAI_DEBUG("Trap DB not initialized");
        return;
    }

    trap_node = (dn_sai_trap_node_t *)std_rbtree_getfirst(hostif_info->trap_tree);
    if (NULL == trap_node) {
        SAI_DEBUG("No traps configured");
        return;
    }

    SAI_DEBUG("\r\nDumping all configured traps\r\n");

    while (trap_node != NULL) {

        SAI_DEBUG("Trap id     = 0x%"PRIx64"",trap_node->key.trap_id);
        SAI_DEBUG("Trap action = %d",trap_node->trap_action);
        SAI_DEBUG("Trap prio   = %u",trap_node->trap_prio);
        SAI_DEBUG("Trap group  = 0x%"PRIx64"",trap_node->trap_group);
        if (trap_node->port_list.count != 0) {
            if (NULL == trap_node->port_list.list) {
                SAI_DEBUG("Invalid port list");
                return;
            }

            for(port_count = 0; port_count < trap_node->port_list.count; port_count++) {
                SAI_DEBUG("Port id = 0x%"PRIx64"",
                                     trap_node->port_list.list[port_count]);
            }
        }
        sai_hostif_npu_api_get()->npu_dump_trap(trap_node);
        SAI_DEBUG("**************************\r\n");
        trap_node = (dn_sai_trap_node_t *)std_rbtree_getnext(hostif_info->trap_tree,
                                                                trap_node);
    }
}

void sai_hostif_dump_trapgroups()
{
    dn_sai_trap_group_node_t *trap_group = NULL;
    dn_sai_trap_node_t *trap_node = NULL;
    dn_sai_user_def_trap_node_t *user_def_trap_node = NULL;
    dn_sai_hostintf_info_t *hostif_info = dn_sai_hostintf_get_info();

    if (NULL == hostif_info->trap_group_tree) {
        SAI_DEBUG("Trap group DB not initialized");
        return;
    }

    trap_group = (dn_sai_trap_group_node_t *)std_rbtree_getfirst(hostif_info->trap_group_tree);
    if (NULL == trap_group) {
        SAI_DEBUG("No trap groups created");
        return;
    }

    SAI_DEBUG("\r\nDumping all existing trap groups\r\n");

    while (trap_group != NULL) {
        SAI_DEBUG("Trap group obj id = 0x%"PRIx64"", trap_group->key.trap_group_id);
        SAI_DEBUG("Admin state = %s", trap_group->admin_state ? "enabled":"disabled");
        SAI_DEBUG("Trap group priority = %u", trap_group->group_prio);
        SAI_DEBUG("Trap group cpu queue = %u", trap_group->cpu_queue);
        SAI_DEBUG("Trap group policer = 0x%"PRIx64"", trap_group->policer_id);
        SAI_DEBUG("Number of traps associated with the trap_group =%u",
                             trap_group->trap_count);

        trap_node = (dn_sai_trap_node_t *) std_dll_getfirst(&trap_group->trap_list);
        while(trap_node != NULL) {
            SAI_DEBUG("Associated Trap id = 0x%"PRIx64"",
                      trap_node->key.trap_id);
            trap_node = (dn_sai_trap_node_t *) std_dll_getnext(&trap_group->trap_list,
                                                               (std_dll *)trap_node);
        }

        SAI_DEBUG("Number of user defined traps associated with the "
                  "trap_group =%u", trap_group->user_def_trap_count);

        user_def_trap_node = (dn_sai_user_def_trap_node_t *)
            std_dll_getfirst(&trap_group->user_def_trap_list);
        while(user_def_trap_node != NULL) {
            SAI_DEBUG("Associated User defined Trap id = 0x%"PRIx64"",
                      user_def_trap_node->key.user_def_trap_id);
            user_def_trap_node = (dn_sai_user_def_trap_node_t *)
                std_dll_getnext(&trap_group->user_def_trap_list,
                                (std_dll *)user_def_trap_node);
        }

        SAI_DEBUG("*****************************************************\r\n");
        trap_group = (dn_sai_trap_group_node_t *)
            std_rbtree_getnext(hostif_info->trap_group_tree, trap_group);

    }
}

void sai_hostif_dump_user_def_traps()
{
    dn_sai_user_def_trap_node_t *trap_node = NULL;
    dn_sai_hostintf_info_t *hostif_info = dn_sai_hostintf_get_info();

    if (NULL == hostif_info->user_def_trap_tree) {
        SAI_DEBUG("User Defined Trap DB not initialized");
        return;
    }

    trap_node = (dn_sai_user_def_trap_node_t *)
        std_rbtree_getfirst(hostif_info->user_def_trap_tree);
    if (NULL == trap_node) {
        SAI_DEBUG("No user defined traps configured");
        return;
    }

    SAI_DEBUG("\r\nDumping all configured user defined traps\r\n");

    while (trap_node != NULL) {

        SAI_DEBUG("User defined Trap id = 0x%"PRIx64"",trap_node->key.user_def_trap_id);
        SAI_DEBUG("Trap prio   = %u",trap_node->trap_prio);
        SAI_DEBUG("Trap group  = 0x%"PRIx64"",trap_node->trap_group);
        SAI_DEBUG("**************************\r\n");
        trap_node = (dn_sai_user_def_trap_node_t *)
            std_rbtree_getnext(hostif_info->user_def_trap_tree, trap_node);
    }
}

void sai_hostif_help()
{
    SAI_DEBUG("The dump functions are:");
    SAI_DEBUG("1. sai_hostif_dump_info(void)");
    SAI_DEBUG("2. sai_hostif_dump_traps(void)");
    SAI_DEBUG("3. sai_hostif_dump_trapgroups(void)");
    SAI_DEBUG("4. sai_hostif_dump_user_def_traps(void)");
}

void sai_hostif_debug_set(sai_hostif_debug_attr_t attr_id, int value)
{
    sai_hostif_npu_api_get()->npu_debug_set(attr_id, value);
}

void sai_hostif_dump_rx_errors_count(void)
{
    uint64_t rx_errors = 0;
    rx_errors = sai_hostif_npu_api_get()->rx_errors_get();
    SAI_DEBUG("SAI Host interface RX Error count - %"PRIu64"", rx_errors);
}

