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
* @file  sai_qos_init.c
*
* @brief This file contains function definitions for SAI Qos Initialization
*        functionality API implementation.
*
*************************************************************************/
#include "std_config_node.h"
#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_api_utils.h"
#include "sai_modules_init.h"
#include "sai_common_infra.h"
#include "sai_switch_init_config.h"
#include "sai_qos_port_util.h"

#include "saistatus.h"

#include "std_assert.h"
#include "std_struct_utils.h"
#include <string.h>
#include "sai_port_main.h"
#include <inttypes.h>

static dn_sai_qos_hierarchy_t *default_hqos = NULL;
static void sai_qos_global_cleanup (void)
{
    dn_sai_qos_global_t    *p_qos_global_cfg = sai_qos_access_global_config ();

    if (p_qos_global_cfg->policer_tree != NULL) {
        std_rbtree_destroy (p_qos_global_cfg->policer_tree);
    }

    if (p_qos_global_cfg->wred_tree != NULL) {
        std_rbtree_destroy (p_qos_global_cfg->wred_tree);
    }

    if (p_qos_global_cfg->map_tree != NULL) {
        std_rbtree_destroy (p_qos_global_cfg->map_tree);
    }

    if (p_qos_global_cfg->scheduler_tree != NULL) {
        std_rbtree_destroy (p_qos_global_cfg->scheduler_tree);
    }

    if (p_qos_global_cfg->queue_tree) {
        std_rbtree_destroy (p_qos_global_cfg->queue_tree);
    }

    if (p_qos_global_cfg->scheduler_group_tree) {
        std_rbtree_destroy (p_qos_global_cfg->scheduler_group_tree);
    }

    if (p_qos_global_cfg->pg_tree) {
        std_rbtree_destroy (p_qos_global_cfg->pg_tree);
    }

    if (p_qos_global_cfg->buffer_pool_tree) {
        std_rbtree_destroy (p_qos_global_cfg->buffer_pool_tree);
    }

    if (p_qos_global_cfg->buffer_profile_tree) {
        std_rbtree_destroy (p_qos_global_cfg->buffer_profile_tree);
    }

    memset (p_qos_global_cfg, 0, sizeof (dn_sai_qos_global_t));

    sai_qos_init_complete_set (false);
}

static sai_status_t sai_qos_global_init (void)
{
    sai_status_t            sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_global_t    *p_qos_global_cfg = sai_qos_access_global_config ();

    if (sai_qos_is_init_complete ()) {
        SAI_QOS_LOG_WARN ("QOS Init done already.");
        return sai_rc;
    }

    do {
        memset (p_qos_global_cfg, 0, sizeof (dn_sai_qos_global_t));

        SAI_QOS_LOG_TRACE ("Creating QOS Policer Tree.");

        p_qos_global_cfg->policer_tree =
            std_rbtree_create_simple ("SAI QOS Policer tree",
                                      STD_STR_OFFSET_OF (dn_sai_qos_policer_t, key),
                                      STD_STR_SIZE_OF (dn_sai_qos_policer_t, key));

        if (p_qos_global_cfg->policer_tree == NULL) {
            SAI_QOS_LOG_CRIT ("SAI QOS Policer Tree creation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        SAI_QOS_LOG_TRACE ("Creating QOS WRED Tree.");

        p_qos_global_cfg->wred_tree =
            std_rbtree_create_simple ("SAI QOS WRED tree",
                                      STD_STR_OFFSET_OF (dn_sai_qos_wred_t, key),
                                      STD_STR_SIZE_OF (dn_sai_qos_wred_t, key));

        if (p_qos_global_cfg->wred_tree == NULL) {
            SAI_QOS_LOG_CRIT ("SAI QOS WRED Tree creation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        SAI_QOS_LOG_TRACE ("Creating QOS Maps Tree.");

        p_qos_global_cfg->map_tree =
            std_rbtree_create_simple ("SAI QOS Maps tree",
                                      STD_STR_OFFSET_OF (dn_sai_qos_map_t, key),
                                      STD_STR_SIZE_OF (dn_sai_qos_map_t, key));

        if (p_qos_global_cfg->map_tree == NULL) {
            SAI_QOS_LOG_CRIT ("SAI QOS Maps Tree creation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        SAI_QOS_LOG_TRACE ("Creating Scheduler Tree.");

        p_qos_global_cfg->scheduler_tree =
            std_rbtree_create_simple ("SAI QOS Scheduler tree",
                                      STD_STR_OFFSET_OF (dn_sai_qos_scheduler_t, key),
                                      STD_STR_SIZE_OF (dn_sai_qos_scheduler_t, key));

        if (p_qos_global_cfg->scheduler_tree == NULL) {
            SAI_QOS_LOG_CRIT ("SAI QOS Scheduler Tree creation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        SAI_QOS_LOG_TRACE ("Creating SAI QOS Queue Tree.");

        p_qos_global_cfg->queue_tree =
            std_rbtree_create_simple ("SAI QOS Queue tree",
                                      STD_STR_OFFSET_OF (dn_sai_qos_queue_t, key),
                                      STD_STR_SIZE_OF (dn_sai_qos_queue_t, key));

        if (p_qos_global_cfg->queue_tree == NULL) {
            SAI_QOS_LOG_CRIT ("SAI QOS Queue Tree creation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        SAI_QOS_LOG_TRACE ("Creating SAI QOS Schedulder Group Tree.");

        p_qos_global_cfg->scheduler_group_tree =
            std_rbtree_create_simple ("SAI QOS Scheduler Group tree",
                                      STD_STR_OFFSET_OF (dn_sai_qos_sched_group_t, key),
                                      STD_STR_SIZE_OF (dn_sai_qos_sched_group_t, key));

        if (p_qos_global_cfg->scheduler_group_tree == NULL) {
            SAI_QOS_LOG_CRIT ("SAI QOS Scheduler Group Tree creation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        p_qos_global_cfg->pg_tree =
            std_rbtree_create_simple ("SAI QOS PG tree",
                                      STD_STR_OFFSET_OF (dn_sai_qos_pg_t, key),
                                      STD_STR_SIZE_OF (dn_sai_qos_pg_t, key));

        if (p_qos_global_cfg->pg_tree == NULL) {
            SAI_QOS_LOG_CRIT ("SAI QOS pg Tree creation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        p_qos_global_cfg->buffer_pool_tree =
            std_rbtree_create_simple ("SAI QOS buffer pool tree",
                                      STD_STR_OFFSET_OF (dn_sai_qos_buffer_pool_t, key),
                                      STD_STR_SIZE_OF (dn_sai_qos_buffer_pool_t, key));

        if (p_qos_global_cfg->buffer_pool_tree == NULL) {
            SAI_QOS_LOG_CRIT ("SAI QOS buffer pool Tree creation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        p_qos_global_cfg->buffer_profile_tree =
            std_rbtree_create_simple ("SAI QOS buffer profile tree",
                                      STD_STR_OFFSET_OF (dn_sai_qos_buffer_profile_t, key),
                                      STD_STR_SIZE_OF (dn_sai_qos_buffer_profile_t, key));

        if (p_qos_global_cfg->buffer_profile_tree == NULL) {
            SAI_QOS_LOG_CRIT ("SAI QOS buffer profile Tree creation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

    } while (0);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        sai_qos_global_cleanup ();
    } else {
        SAI_QOS_LOG_INFO ("SAI QOS Global Data structures Init complete.");
    }
    return sai_rc;
}

static void sai_qos_init_free_resource (bool is_qos_global_init)
{
    sai_qos_port_all_deinit();

    sai_qos_remove_default_scheduler ();

    if (is_qos_global_init)
        sai_qos_global_cleanup();
}

static void sai_qos_hierarchy_info_free(uint_t max_level)
{
    uint_t num_level = 0;
    uint_t num_sg_groups = 0;
    uint_t max_sg_groups = 0;

    for(num_level = 0; num_level < max_level; num_level ++){
        max_sg_groups = default_hqos->level_info[num_level].num_sg_groups;
        for(num_sg_groups = 0; num_sg_groups < max_sg_groups ; num_sg_groups ++){
            if(default_hqos->level_info[num_level].sg_info[num_sg_groups].child_info != NULL){
                free(default_hqos->level_info[num_level].sg_info[num_sg_groups].child_info);
            }
        }
        if(default_hqos->level_info[num_level].sg_info != NULL){
            free(default_hqos->level_info[num_level].sg_info);
        }
    }
    free(default_hqos->level_info);
}

static sai_status_t sai_qos_child_info_update(uint_t level, uint_t sg_index, uint_t max_child_count,
                                                  std_config_node_t sg_node)
{
    STD_ASSERT(sg_node != NULL);
    uint_t child_index = 0;
    uint_t child_level = 0;
    uint_t queue_number = 0;
    uint_t queue_type = 0;
    uint_t num_child_count = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    std_config_node_t child_node = NULL;

    SAI_QOS_LOG_TRACE("child count is %d",max_child_count);
    for(child_node = std_config_get_child(sg_node);
        ((child_node != NULL) && (num_child_count < max_child_count));
        child_node = std_config_next_node(child_node),num_child_count ++){

        if(strncmp(std_config_name_get(child_node), SAI_NODE_NAME_CHILD_SCHED_GRP,
                   SAI_MAX_NAME_LEN) == 0) {
            sai_std_config_attr_update(child_node, SAI_ATTR_HQOS_CHILD_LEVEL, &child_level, 0);
            sai_std_config_attr_update(child_node, SAI_ATTR_HQOS_INDEX, &child_index, 0);

            default_hqos->level_info[level].sg_info[sg_index].child_info[num_child_count].level =
                child_level;
            default_hqos->level_info[level].sg_info[sg_index].child_info[num_child_count].child_index =
                child_index;

            default_hqos->level_info[level].sg_info[sg_index].child_info[num_child_count].type =
                CHILD_TYPE_SCHEDULER;
            SAI_QOS_LOG_TRACE("schdeuler type childlevel %d index %d",child_level,child_index);
        } else if(strncmp(std_config_name_get(child_node), SAI_NODE_NAME_CHILD_QUEUE,
                          SAI_MAX_NAME_LEN) == 0) {
            sai_std_config_attr_update(child_node, SAI_ATTR_HQOS_QUEUE_NUMBER, &queue_number, 0);
            sai_std_config_attr_update_str(child_node, SAI_ATTR_HQOS_QUEUE_TYPE, &queue_type, 0);
            sai_std_config_attr_update(child_node, SAI_ATTR_HQOS_INDEX, &child_index, 0);

            default_hqos->level_info[level].sg_info[sg_index].child_info[num_child_count].queue_type =
                queue_type;
            default_hqos->level_info[level].sg_info[sg_index].child_info[num_child_count].child_index =
                queue_number;

            default_hqos->level_info[level].sg_info[sg_index].child_info[num_child_count].type =
                CHILD_TYPE_QUEUE;
            SAI_QOS_LOG_TRACE("queuetype queue number %d type %d",queue_number,queue_type);
        }
    }
    return sai_rc;
}

static sai_status_t sai_qos_sg_info_update(uint_t level, uint_t max_scheduler_groups,
                                                 std_config_node_t level_node)
{
    STD_ASSERT(level_node != NULL);
    uint_t sg_index = 0;
    uint_t child_count = 0;
    uint_t num_scheduler_groups = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    std_config_node_t sg_node = NULL;

    for(sg_node = std_config_get_child(level_node);
        ((sg_node != NULL) && (num_scheduler_groups < max_scheduler_groups));
        sg_node = std_config_next_node(sg_node),num_scheduler_groups ++){

        sai_std_config_attr_update(sg_node, SAI_ATTR_HQOS_INDEX, &sg_index, 0);
        sai_std_config_attr_update(sg_node, SAI_ATTR_HQOS_NUM_CHILD, &child_count, 0);

        SAI_QOS_LOG_TRACE("sgindex %d numchild %d",num_scheduler_groups,child_count);
        default_hqos->level_info[level].sg_info[num_scheduler_groups].child_info = (dn_sai_qos_child_info_t *)
            calloc(child_count, sizeof(dn_sai_qos_child_info_t));

        if(default_hqos->level_info[level].sg_info[sg_index].child_info == NULL){
            return SAI_STATUS_NO_MEMORY;
        }

        default_hqos->level_info[level].sg_info[num_scheduler_groups].node_id = sg_index;
        default_hqos->level_info[level].sg_info[num_scheduler_groups].num_children = child_count;
        default_hqos->level_info[level].sg_info[num_scheduler_groups].sched_mode = 0;

        sai_rc = sai_qos_child_info_update(level, sg_index, child_count, sg_node);
        if(sai_rc != SAI_STATUS_SUCCESS){
            return sai_rc;
        }
    }
    return sai_rc;
}

sai_status_t sai_qos_hierarchy_handler(std_config_node_t hqos_node,
                                       dn_sai_qos_hierarchy_t *p_hqos)
{
    uint_t num_scheduler_groups = 0;
    uint_t cur_level = 0;
    uint_t num_levels = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    std_config_node_t sai_node = NULL;

    default_hqos = p_hqos;
    sai_std_config_attr_update(hqos_node, SAI_ATTR_HQOS_NUM_LEVELS, &num_levels, 0);

    SAI_QOS_LOG_TRACE("Num levels is %d", num_levels);

    if (num_levels == 0)
        return SAI_STATUS_SUCCESS;

    default_hqos->level_info = (dn_sai_qos_level_info_t *) calloc(num_levels, sizeof(dn_sai_qos_level_info_t));

    if(default_hqos->level_info == NULL){
        return SAI_STATUS_NO_MEMORY;
    }

    for (sai_node = std_config_get_child(hqos_node);
         sai_node != NULL;
         sai_node = std_config_next_node(sai_node)) {
        sai_std_config_attr_update(sai_node, SAI_ATTR_HQOS_LEVEL, &cur_level, 0);
        sai_std_config_attr_update(sai_node, SAI_ATTR_HQOS_NUM_SCHED_GRPS, &num_scheduler_groups, 0);

        SAI_QOS_LOG_TRACE("Level %d num sched groups %d",cur_level, num_scheduler_groups);

        default_hqos->level_info[cur_level].sg_info = (dn_sai_qos_sched_grp_info_t *)
            calloc(num_scheduler_groups, sizeof(dn_sai_qos_sched_grp_info_t));

        if(default_hqos->level_info[cur_level].sg_info == NULL){
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }
        default_hqos->level_info[cur_level].level = cur_level;
        default_hqos->level_info[cur_level].num_sg_groups = num_scheduler_groups;

        sai_rc = sai_qos_sg_info_update(cur_level, num_scheduler_groups, sai_node);
        if(sai_rc != SAI_STATUS_SUCCESS){
            break;
        }
    }
    if(sai_rc != SAI_STATUS_SUCCESS){
        SAI_QOS_LOG_ERR("Hierarchy init failed. Freeing the allocated structure");
        sai_qos_hierarchy_info_free(num_levels);
    }
    return sai_rc;
}

static sai_status_t sai_qos_port_notification_handler (uint32_t count,
                                                       sai_port_event_notification_t *data)
{
    uint32_t port_idx = 0;
    sai_object_id_t port_id = 0;
    sai_port_event_t port_event = 0;
    sai_status_t sai_rc;

    for(port_idx = 0; port_idx < count; port_idx++) {
        port_id = data[port_idx].port_id;
        port_event = data[port_idx].port_event;
        /*Add newly created port to default VLAN ID*/
        if(port_event == SAI_PORT_EVENT_ADD) {

            SAI_QOS_LOG_TRACE ("Port init 0x%"PRIx64"\r\n", port_id);
            sai_rc = sai_qos_port_init (port_id);

            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_QOS_LOG_ERR ("Port init 0x%"PRIx64" failed\r\n", port_id);
            }

        } else if(port_event == SAI_PORT_EVENT_DELETE) {

            SAI_QOS_LOG_TRACE ("Port deinit 0x%"PRIx64"\r\n", port_id);
            sai_rc= sai_qos_port_deinit (port_id);

            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_QOS_LOG_ERR ("Port deinit 0x%"PRIx64" failed\r\n", port_id);
            }
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_init (void)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    bool         is_qos_global_init = false;
    sai_object_id_t  def_sched_id = SAI_NULL_OBJECT_ID;

    SAI_QOS_LOG_TRACE ("Qos Init.");

    sai_qos_lock();

    do {
        sai_rc = sai_qos_global_init ();

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_CRIT ("SAI QOS Global Data structures init failed.");
            break;
        }

        is_qos_global_init = true;

        sai_rc = sai_qos_npu_api_get()->qos_global_init ();

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_CRIT ("SAI QOS NPU Global init failed.");
            break;
        }

        if (sai_qos_is_hierarchy_qos_supported () ||
                sai_switch_max_queues_get ())
        {
            sai_rc = sai_qos_create_default_scheduler(&def_sched_id);

            if(sai_rc != SAI_STATUS_SUCCESS){
                SAI_QOS_LOG_CRIT ("Default scheduler create failed");
                break;
            }

            sai_qos_default_sched_id_set(def_sched_id);
        }

        sai_rc = sai_port_event_internal_notif_register(SAI_MODULE_QOS_PORT,(sai_port_event_notification_fn)
                sai_qos_port_notification_handler);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_CRIT ("SAI QOS port event init failed.");
            break;
        }

        /* Initialize Port pool tree */
        sai_rc = sai_qos_port_pool_tree_init();
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_ERR("Port pool tree init failed, sai_rc%d", sai_rc);
            return sai_rc;
        }

        /* Initialize Port pool SAI oid generator */
        sai_qos_port_pool_oid_gen_init();

        sai_rc = sai_qos_port_all_init ();

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_CRIT ("SAI QOS All ports init failed.");
            break;
        }
        sai_rc = sai_buffer_init ();

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_ERR ("SAI QOS Buffer init failed.");
            break;
        }
    } while (0);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        sai_qos_init_free_resource (is_qos_global_init);
        SAI_QOS_LOG_INFO ("Qos Init Failed.");
    } else {
        sai_qos_init_complete_set (true);
        SAI_QOS_LOG_INFO ("Qos Init complete.");
    }

    sai_qos_unlock();
    return sai_rc;
}

sai_status_t sai_switch_set_qos_default_tc(uint_t default_tc)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    sai_rc = sai_qos_npu_api_get()->qos_switch_default_tc(default_tc);

    return sai_rc;
}
