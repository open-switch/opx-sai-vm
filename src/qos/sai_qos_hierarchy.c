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
* @file  sai_qos_hierarchy.c
*
* @brief This file contains function definitions for SAI QOS default
*        scheduler groups initilization and hiearchy creation and SAI
*        scheduler group functionality API implementation.
*
*************************************************************************/

#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_api_utils.h"
#include "sai_common_infra.h"

#include "saistatus.h"
#include "saitypes.h"

#include "std_assert.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

static inline bool sai_qos_is_child_obj_valid (sai_object_id_t child_id)
{
    return (sai_is_obj_id_queue (child_id) || sai_is_obj_id_scheduler_group (child_id));
}

static sai_status_t sai_qos_port_hierarchy_create (sai_object_id_t port_id,
                                                   dn_sai_qos_hierarchy_t *p_default_hqos)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    uint_t           level = 0;
    uint_t           child_count = 0;
    uint_t           sg_groups = 0;
    uint_t           count = 0;
    uint_t           child_idx = 0;
    uint_t           child_type = 0;
    uint_t           queue_type = 0;
    uint_t           parent_idx = 0;
    uint_t           child_level = 0;
    uint_t           max_sg_childs = 0;
    sai_object_id_t  parent_sg_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t  sg_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t  queue_id = SAI_NULL_OBJECT_ID;
    dn_sai_qos_port_t *p_qos_port_node = NULL;
    uint8_t          uc_queue_id = 0;
    uint8_t          mc_queue_id = 0;

    SAI_SCHED_GRP_LOG_TRACE ("Port 0x%"PRIx64" default hierarchy create for "
                             "non leaf levels.", port_id);

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {
        SAI_SCHED_GRP_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                               port_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for (level = 0; level < sai_switch_max_hierarchy_levels_get(); level++) {
        for(sg_groups = 0; sg_groups < p_default_hqos->level_info[level].num_sg_groups;
            sg_groups ++) {

            child_count = p_default_hqos->level_info[level].sg_info[sg_groups].num_children;
            parent_idx = p_default_hqos->level_info[level].sg_info[sg_groups].node_id;

            SAI_SCHED_GRP_LOG_TRACE("Level %d sg_group %d node %d childcount %d",
                                    level, sg_groups, parent_idx, child_count);
            if (level == 0) {
                parent_sg_id = port_id;
                sai_rc = sai_qos_port_sched_group_create(port_id,
                                        parent_sg_id, level, child_count, &sg_id);
                if (sai_rc != SAI_STATUS_SUCCESS){
                    SAI_SCHED_GRP_LOG_ERR("Failed to get sgid for level %d sggroup %d",
                                          level, sg_groups );
                    return sai_rc;
                }
                parent_sg_id = sg_id;

            } else {
                sai_rc = sai_qos_indexed_sched_group_id_get(port_id, level, parent_idx,
                                                            &parent_sg_id);

                if (sai_rc != SAI_STATUS_SUCCESS){
                    SAI_SCHED_GRP_LOG_ERR("Failed to get parent sgid for level %d sggroup %d node %d",
                                          level, sg_groups, parent_idx);
                    return sai_rc;
                }
            }

            if(child_count == 0){
                continue;
            }

            for(count = 0; count < child_count; count ++){

                child_idx = p_default_hqos->level_info[level].sg_info[sg_groups].
                    child_info[count].child_index;
                child_type = p_default_hqos->level_info[level].sg_info[sg_groups].
                    child_info[count].type;
                child_level = p_default_hqos->level_info[level].sg_info[sg_groups].
                    child_info[count].level;

                SAI_SCHED_GRP_LOG_TRACE("childtype is %d childidx %d childlevel %d",
                                        child_type, child_idx, child_level);

                if (child_type == CHILD_TYPE_SCHEDULER) {
                    max_sg_childs = p_default_hqos->level_info[child_level].
                                                  sg_info[child_idx].num_children;
                    SAI_SCHED_GRP_LOG_TRACE("childtype is %d childidx %d "
                                          "childlevel %d max_sg_childs %d",
                                          child_type, child_idx, child_level, max_sg_childs);
                    sai_rc = sai_qos_port_sched_group_create(port_id,
                                        parent_sg_id, child_level, max_sg_childs, &sg_id);

                    if (sai_rc != SAI_STATUS_SUCCESS){
                        SAI_SCHED_GRP_LOG_ERR("Failed to create sgid for level %d sg index %d",
                                              level, child_idx );
                        return sai_rc;
                    }
                } else if(child_type == CHILD_TYPE_QUEUE) {
                    queue_type = p_default_hqos->level_info[level].sg_info[sg_groups].
                        child_info[count].queue_type;

                    sai_rc = sai_qos_port_queue_create(port_id, queue_type,
                                                       ((queue_type == SAI_QUEUE_TYPE_UNICAST) ?
                                                         uc_queue_id: mc_queue_id),
                                                       parent_sg_id, &queue_id);
                    if(sai_rc != SAI_STATUS_SUCCESS){
                        SAI_SCHED_GRP_LOG_ERR("Failed to create queue for level %d index %d",
                                              level, child_idx);
                        return sai_rc;
                    }
                    if (queue_type == SAI_QUEUE_TYPE_UNICAST)
                    {
                        uc_queue_id++;
                    }
                    else if (queue_type == SAI_QUEUE_TYPE_MULTICAST)
                    {
                        mc_queue_id++;
                    }
                }
            }
        }
    }

    if (sai_rc == SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_TRACE ("Port 0x%"PRIx64" default hierarchy create"
                                 "success.", port_id);
    }
    return sai_rc;
}

static sai_status_t sai_qos_leaf_level_hierarchy_remove (sai_object_id_t port_id)
{
    sai_status_t         sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_port_t    *p_qos_port_node = NULL;
    dn_sai_qos_queue_t   *p_queue_node = NULL;
    dn_sai_qos_queue_t   *p_next_queue_node = NULL;

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {

        SAI_SCHED_GRP_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                               port_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    SAI_SCHED_GRP_LOG_TRACE ("Port 0x%"PRIx64" leaf level hierarchy remove.",
                             port_id);

    for (p_queue_node = sai_qos_port_get_first_queue (p_qos_port_node);
         p_queue_node != NULL; p_queue_node = p_next_queue_node) {

        p_next_queue_node = sai_qos_port_get_next_queue (p_qos_port_node,
                                                         p_queue_node);
        sai_rc = sai_qos_port_queue_remove (p_queue_node->key.queue_id);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QUEUE_LOG_ERR ("Queue remove failed "
                               "for port 0x%"PRIx64", Queue 0x%"PRIx64".",
                               port_id, p_queue_node->key.queue_id);
            return sai_rc;
        }
    }

    SAI_SCHED_GRP_LOG_TRACE ("Port 0x%"PRIx64" leaf hierarchy remove success.",
                             port_id);

    return sai_rc;
}

static sai_status_t sai_qos_non_leaf_level_hierarchy_remove (sai_object_id_t port_id)
{
    sai_status_t              sai_rc = SAI_STATUS_SUCCESS;
    int                       level = 0;
    dn_sai_qos_sched_group_t *p_sg_node = NULL;
    dn_sai_qos_sched_group_t *p_next_sg_node = NULL;
    dn_sai_qos_port_t        *p_qos_port_node = NULL;
    sai_object_id_t           sg_id = SAI_NULL_OBJECT_ID;

    SAI_SCHED_GRP_LOG_TRACE ("Port  0x%"PRIx64" hierarchy remove for non "
                             "leaf levels.", port_id);

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {

        SAI_SCHED_GRP_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                               port_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for (level = sai_switch_leaf_hierarchy_level_get();
         level >= 0; level--) {

        SAI_SCHED_GRP_LOG_TRACE ("level %d.", level);

        for (p_sg_node =
             sai_qos_port_get_first_sched_group (p_qos_port_node, level);
             (p_sg_node != NULL);
             p_sg_node = p_next_sg_node) {

            p_next_sg_node = sai_qos_port_get_next_sched_group (p_qos_port_node, p_sg_node);

            sg_id = p_sg_node->key.sched_group_id;

            sai_rc = sai_qos_port_sched_group_remove(sg_id);

            if (sai_rc != SAI_STATUS_SUCCESS) {
                SAI_SCHED_GRP_LOG_ERR ("Failed to remove SG 0x%"PRIx64" at level %d"
                                       "for port 0x%"PRIx64"",
                                       sg_id, level, port_id);
                return sai_rc;
            }
        }
    }

    SAI_SCHED_GRP_LOG_TRACE ("Port 0x%"PRIx64" hierarchy remove for non leaf "
                             "levels success.", port_id);

    return sai_rc;
}

static sai_status_t sai_qos_hierarchy_remove (sai_object_id_t port_id)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;

    SAI_SCHED_GRP_LOG_TRACE ("Port 0x%"PRIx64" hierarchy remove.", port_id);

    sai_rc = sai_qos_leaf_level_hierarchy_remove (port_id);
    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Failed to remove leaf hierarchy for port 0x%"PRIx64".",
                               port_id);
        return sai_rc;
    }

    sai_rc = sai_qos_non_leaf_level_hierarchy_remove (port_id);
    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Failed to remove non leaf hierarchy for port 0x%"PRIx64".",
                               port_id);
        return sai_rc;
    }

    SAI_SCHED_GRP_LOG_INFO ("Port 0x%"PRIx64" Hierarchy remove success.",
                             port_id);

    return sai_rc;
}

sai_status_t sai_qos_port_default_hierarchy_init (sai_object_id_t port_id)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;

    SAI_SCHED_GRP_LOG_TRACE ("Port default hierarchy create.");

    /* Create 1 scheduler group at each level other than the leaf.
     * Leaf level, NPU suport seperate unicast and multicast queues
     * on port create logical pairs */

    if(!sai_is_obj_id_cpu_port(port_id)){
        sai_rc = sai_qos_port_hierarchy_create (port_id, sai_qos_default_hqos_get());
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Failed to hierarchy for port 0x%"PRIx64".", port_id);
            return sai_rc;
        }
    }else{
        sai_rc = sai_qos_port_hierarchy_create (port_id, sai_qos_default_cpu_hqos_get());
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_GRP_LOG_ERR ("Failed to hierarchy for port 0x%"PRIx64".", port_id);
            return sai_rc;
        }
    }
    SAI_SCHED_GRP_LOG_INFO ("Port 0x%"PRIx64" Hierarchy create success.",
                             port_id);

    return sai_rc;
}

sai_status_t sai_qos_port_hierarchy_deinit (sai_object_id_t port_id)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    SAI_SCHED_GRP_LOG_TRACE ("Port 0x%"PRIx64" Hierarchy De-Init.", port_id);

    /* Remove present hierarchy created on port */
    sai_rc = sai_qos_hierarchy_remove (port_id);
    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Port hierarchy remove failed "
                               "for port 0x%"PRIx64".", port_id);
        return sai_rc;
    }

    SAI_SCHED_GRP_LOG_INFO ("Port 0x%"PRIx64" hierarchy De-Init success.",
                             port_id);
    return sai_rc;
}
