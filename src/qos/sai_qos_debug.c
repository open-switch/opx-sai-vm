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
* @file sai_qos_debug.c
*
* @brief This file contains the debug functions for SAI Qos component.
*
*************************************************************************/

#include "saiqueue.h"
#include "saischeduler.h"
#include "saischedulergroup.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_debug_utils.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_buffer_util.h"
#include "std_assert.h"
#include "std_utils.h"
#include <string.h>
#include <inttypes.h>
#include <stdio.h>

char *sai_qos_get_queue_type_str (sai_queue_type_t queue_type, char *str, uint_t len)
{
     STD_ASSERT (str != NULL);

     switch(queue_type) {
         case SAI_QUEUE_TYPE_ALL:
             snprintf(str,len, "All");
             break;
         case SAI_QUEUE_TYPE_UNICAST:
             snprintf(str, len, "Unicast");
             break;
         case SAI_QUEUE_TYPE_MULTICAST:
             snprintf(str, len, "Multicast");
             break;
         default:
             snprintf(str, len, "Unknown");
     }
     return str;
}

char *sai_qos_get_queue_type_short_str (sai_queue_type_t queue_type, char *str, uint_t len)
{
     STD_ASSERT (str != NULL);

     switch(queue_type) {
         case SAI_QUEUE_TYPE_ALL:
             snprintf(str,len, "A");
             break;
         case SAI_QUEUE_TYPE_UNICAST:
             snprintf(str, len, "UC");
             break;
         case SAI_QUEUE_TYPE_MULTICAST:
             snprintf(str, len, "MC");
             break;
         default:
             snprintf(str, len, "U");
     }
     return str;
}

char *sai_qos_get_scheduler_algo_str (sai_scheduling_type_t type, char *str, uint_t len)
{
     STD_ASSERT (str != NULL);

     switch(type) {
         case SAI_SCHEDULING_TYPE_STRICT:
             snprintf(str, len, "Strict");
             break;
         case SAI_SCHEDULING_TYPE_WRR:
             snprintf(str, len, "Weighted Round Robin");
             break;
         case SAI_SCHEDULING_TYPE_DWRR:
             snprintf(str, len, "Weighted Deficit Round Robin");
             break;
         default:
             snprintf(str, len, "Unknown");
     }
     return str;
}

char *sai_qos_get_scheduler_algo_short_str (sai_scheduling_type_t type, char *str, uint_t len)
{
     STD_ASSERT (str != NULL);

     switch(type) {
         case SAI_SCHEDULING_TYPE_STRICT:
             snprintf(str, len, "S");
             break;
         case SAI_SCHEDULING_TYPE_WRR:
             snprintf(str, len, "WRR");
             break;
         case SAI_SCHEDULING_TYPE_DWRR:
             snprintf(str, len, "DWRR");
             break;
         default:
             snprintf(str, len, "U");
     }
     return str;
}

char *sai_qos_get_meter_type_str (sai_meter_type_t type, char *str, uint_t len)
{
     STD_ASSERT (str != NULL);

     switch(type) {
         case SAI_METER_TYPE_PACKETS:
             snprintf(str, len, "Packets");
             break;
         case SAI_METER_TYPE_BYTES:
             snprintf(str, len, "Bytes");
             break;
         default:
             snprintf(str, len, "Unknown");
     }
     return str;
}

sai_status_t sai_dump_queue_node(dn_sai_qos_queue_t *queue_node)
{
    char queue_type_str[SAI_MAX_QUEUE_TYPE_STRLEN];

    STD_ASSERT(queue_node != NULL);

    SAI_DEBUG ("\nQueue id: 0x%"PRIx64"", queue_node->key.queue_id);
    SAI_DEBUG ("---------------------------------------------");
    SAI_DEBUG ("Port ID           : 0x%"PRIx64"", queue_node->port_id);
    SAI_DEBUG ("Queue type        : %s", sai_qos_get_queue_type_str (queue_node->queue_type,
                                       queue_type_str, SAI_MAX_QUEUE_TYPE_STRLEN));
    SAI_DEBUG ("Queue index       : %u", queue_node->queue_index);
    SAI_DEBUG ("Wred ID           : 0x%"PRIx64"", queue_node->wred_id);
    SAI_DEBUG ("Buffer profile ID : 0x%"PRIx64"", queue_node->buffer_profile_id);
    SAI_DEBUG ("Scheduler ID      : 0x%"PRIx64"", queue_node->scheduler_id);
    SAI_DEBUG ("Parent SG ID      : 0x%"PRIx64"", queue_node->parent_sched_group_id);
    SAI_DEBUG ("Child offset      : %u", queue_node->child_offset);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_dump_queue_info(sai_object_id_t queue_id)
{
    dn_sai_qos_queue_t *queue_node = sai_qos_queue_node_get(queue_id);

    if(queue_node == NULL) {
        SAI_DEBUG ("Error Queue object not found.");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    return sai_dump_queue_node(queue_node);
}

sai_status_t sai_dump_sg_node(dn_sai_qos_sched_group_t *sg_node)
{
    dn_sai_qos_sched_group_t *child_sg_node = NULL;
    dn_sai_qos_queue_t *child_queue_node = NULL;
    uint_t child_idx = 0;
    uint_t queue_idx = 0;
    char queue_type_str[SAI_MAX_QUEUE_TYPE_STRLEN];

    STD_ASSERT (sg_node != NULL);

    SAI_DEBUG ("\nScheduler group id: 0x%"PRIx64"", sg_node->key.sched_group_id);
    SAI_DEBUG ("---------------------------------------------");
    SAI_DEBUG ("Port ID         : 0x%"PRIx64"",sg_node->port_id);
    SAI_DEBUG ("Hierarchy level : %u",sg_node->hierarchy_level);
    SAI_DEBUG ("Child offset    : %u",sg_node->child_offset);
    SAI_DEBUG ("Scheduler ID    : 0x%"PRIx64"", sg_node->scheduler_id);

    SAI_DEBUG ("\nHierarchy info:");
    SAI_DEBUG ("---------------------------------------------");
    SAI_DEBUG ("Parent SG ID    : 0x%"PRIx64"", sg_node->parent_id);
    SAI_DEBUG ("Max childs      : %u", sg_node->max_childs);
    SAI_DEBUG ("Child count     : %u", sg_node->hqos_info.child_count);
    SAI_DEBUG ("Next free index : %u", std_find_first_bit (sg_node->hqos_info.child_index_bitmap,
                                                           sg_node->max_childs, 0));
    SAI_DEBUG ("Child SG List :");

    for(child_sg_node = sai_qos_sched_group_get_first_child_sched_group(sg_node);
        child_sg_node != NULL;
        child_sg_node = sai_qos_sched_group_get_next_child_sched_group(sg_node, child_sg_node))
    {
        SAI_DEBUG ("Child %u SG ID (%u) : 0x%"PRIx64"", child_idx, child_sg_node->hierarchy_level,
                   child_sg_node->key.sched_group_id);
        child_idx++;
    }

    for(child_queue_node = sai_qos_sched_group_get_first_child_queue(sg_node);
        child_queue_node != NULL;
        child_queue_node = sai_qos_sched_group_get_next_child_queue(sg_node, child_queue_node))
    {
        SAI_DEBUG ("Child %u Queue ID (%s)   : 0x%"PRIx64"", queue_idx,
                   sai_qos_get_queue_type_short_str (child_queue_node->queue_type,
                                                    queue_type_str, SAI_MAX_QUEUE_TYPE_STRLEN),
                   child_queue_node->key.queue_id);
        queue_idx++;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_dump_sg_info(sai_object_id_t sg_id)
{
    dn_sai_qos_sched_group_t *sg_node = sai_qos_sched_group_node_get(sg_id);

    if(sg_node == NULL) {
        SAI_DEBUG ("Error Scheduler group object not found.");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    return sai_dump_sg_node(sg_node);
}

sai_status_t sai_dump_port_node(dn_sai_qos_port_t *port_node)
{
    char queue_type_str[SAI_MAX_QUEUE_TYPE_STRLEN] = {0};
    uint_t level = 0;
    uint_t sg_count = 0;
    uint_t queue_count = 0;
    uint_t pg_count = 0;
    dn_sai_qos_sched_group_t *sg_node = NULL;
    dn_sai_qos_queue_t *queue_node = NULL;
    dn_sai_qos_pg_t *pg_node = NULL;
    uint_t policer_id = 0;
    uint_t map_id = 0;

    STD_ASSERT (port_node != NULL);

    SAI_DEBUG ("\nPort ID   : 0x%"PRIx64"", port_node->port_id);
    SAI_DEBUG ("---------------------------------------------");
    SAI_DEBUG ("Scheduler ID     : 0x%"PRIx64"", port_node->scheduler_id);
    SAI_DEBUG ("Buffer profile ID: 0x%"PRIx64"", port_node->buffer_profile_id);
    SAI_DEBUG ("is_app_hqos_init : %u", port_node->is_app_hqos_init);
    SAI_DEBUG ("is_app_hqos_init : %u", port_node->is_app_hqos_init);

    SAI_DEBUG ("Policer List");
    for(policer_id = 0; policer_id < SAI_QOS_POLICER_TYPE_MAX; policer_id++) {
        SAI_DEBUG ("Policer %u ID : 0x%"PRIx64"", policer_id, port_node->policer_id[policer_id]);
    }

    SAI_DEBUG ("Maps List");
    for(map_id = 0; map_id < SAI_QOS_MAX_QOS_MAPS_TYPES; map_id++) {
        SAI_DEBUG ("Map %u ID : 0x%"PRIx64"", map_id, port_node->maps_id[map_id]);
    }

    for (level = 0; level < sai_switch_max_hierarchy_levels_get (); level++)
    {
        SAI_DEBUG ("\nLevel %u SG list:", level);
        SAI_DEBUG ("---------------------------------------------");
        sg_count = 0;
        for (sg_node = sai_qos_port_get_first_sched_group (port_node, level);
             (sg_node != NULL);
             sg_node = sai_qos_port_get_next_sched_group (port_node, sg_node)) {
             SAI_DEBUG ("SG ID %u : 0x%"PRIx64" - Scheduler ID: 0x%"PRIx64"", sg_count,
                        sg_node->key.sched_group_id, sg_node->scheduler_id);
             sg_count++;
        }
    }

    SAI_DEBUG ("\nQueue List");
    SAI_DEBUG ("---------------------------------------------");
    for (queue_node = sai_qos_port_get_first_queue (port_node);
         (queue_node != NULL) ; queue_node =
         sai_qos_port_get_next_queue (port_node, queue_node)) {
        SAI_DEBUG ("Queue ID %u (%s) : 0x%"PRIx64" Scheduler ID: 0x%"PRIx64""
                   " WRED ID: 0x%"PRIx64" Buffer profile ID: 0x%"PRIx64"", queue_count,
                   sai_qos_get_queue_type_short_str (queue_node->queue_type, queue_type_str,
                                                     SAI_MAX_QUEUE_TYPE_STRLEN),
                   queue_node->key.queue_id, queue_node->scheduler_id, queue_node->wred_id,
                   queue_node->buffer_profile_id);
        queue_count++;
    }

    SAI_DEBUG ("\nNumber of priority groups: %u", port_node->num_pg);

    SAI_DEBUG ("Priority group List");
    SAI_DEBUG ("---------------------------------------------");
    for (pg_node = sai_qos_port_get_first_pg (port_node);
         (pg_node != NULL) ; pg_node =
         sai_qos_port_get_next_pg (port_node, pg_node)) {
        SAI_DEBUG ("PG ID %u: 0x%"PRIx64" Buffer profile ID: 0x%"PRIx64"", pg_count,
                   pg_node->key.pg_id, pg_node->buffer_profile_id);
        pg_count++;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_dump_qos_port_info(sai_object_id_t port_id)
{
    dn_sai_qos_port_t *port_node = sai_qos_port_node_get(port_id);

    if(port_node == NULL) {
        SAI_DEBUG ("Error Port object not found.");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    return sai_dump_port_node(port_node);
}

sai_status_t sai_dump_scheduler_node(dn_sai_qos_scheduler_t *scheduler_node)
{
    dn_sai_qos_sched_group_t *sg_node = NULL;
    dn_sai_qos_queue_t *queue_node = NULL;
    dn_sai_qos_port_t *qos_port_node = NULL;
    uint_t port_idx = 0;
    uint_t sg_idx = 0;
    uint_t queue_idx = 0;
    char alg_str[SAI_MAX_SCHED_TYPE_STRLEN];
    char meter_str[SAI_MAX_METER_TYPE_STRLEN];
    char queue_type_str[SAI_MAX_QUEUE_TYPE_STRLEN];

    STD_ASSERT (scheduler_node != NULL);

    SAI_DEBUG ("\nScheduler id: 0x%"PRIx64"", scheduler_node->key.scheduler_id);
    SAI_DEBUG ("---------------------------------------------");
    SAI_DEBUG ("Scheduler Algorithm  : %s", sai_qos_get_scheduler_algo_str(
                                            scheduler_node->sched_algo, alg_str,
                                            SAI_MAX_SCHED_TYPE_STRLEN));
    SAI_DEBUG ("Weight               : %u", scheduler_node->weight);
    SAI_DEBUG ("Meter Type           : %s", sai_qos_get_meter_type_str(
                                            scheduler_node->shape_type, meter_str,
                                            SAI_MAX_METER_TYPE_STRLEN));
    SAI_DEBUG ("Max bandwidth rate   : 0x%"PRIx64"", scheduler_node->max_bandwidth_rate);
    SAI_DEBUG ("Max bandwidth burst  : 0x%"PRIx64"", scheduler_node->max_bandwidth_burst);
    SAI_DEBUG ("Min bandwidth rate   : 0x%"PRIx64"", scheduler_node->min_bandwidth_rate);
    SAI_DEBUG ("Min bandwidth burst  : 0x%"PRIx64"", scheduler_node->min_bandwidth_burst);

    SAI_DEBUG ("\nPort List :");
    SAI_DEBUG ("---------------------------------------------");

    for(qos_port_node = sai_qos_scheduler_get_first_port(scheduler_node);
        qos_port_node != NULL;
        qos_port_node = sai_qos_scheduler_get_next_port(scheduler_node, qos_port_node))
    {
        SAI_DEBUG ("Port %u ID : 0x%"PRIx64"", port_idx, qos_port_node->port_id);
        port_idx++;
    }

    SAI_DEBUG ("\nScheduler Group List :");
    SAI_DEBUG ("---------------------------------------------");

    for(sg_node = sai_qos_scheduler_get_first_sched_group(scheduler_node);
        sg_node != NULL;
        sg_node = sai_qos_scheduler_get_next_sched_group(scheduler_node, sg_node))
    {
        SAI_DEBUG ("Scheduler group %u ID (%u) : 0x%"PRIx64"", sg_idx, sg_node->hierarchy_level,
                   sg_node->key.sched_group_id);
        sg_idx++;
    }

    SAI_DEBUG ("\nQueue List :");
    SAI_DEBUG ("---------------------------------------------");

    for(queue_node = sai_qos_scheduler_get_first_queue(scheduler_node);
        queue_node != NULL;
        queue_node = sai_qos_scheduler_get_next_queue(scheduler_node, queue_node))
    {
        SAI_DEBUG ("Queue %u ID (%s) : 0x%"PRIx64"", queue_idx,
                   sai_qos_get_queue_type_short_str (queue_node->queue_type,
                                                    queue_type_str, SAI_MAX_QUEUE_TYPE_STRLEN),
                   queue_node->key.queue_id);
        queue_idx++;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_dump_scheduler_info(sai_object_id_t scheduler_id)
{
    dn_sai_qos_scheduler_t *scheduler_node = sai_qos_scheduler_node_get(scheduler_id);

    if(scheduler_node == NULL) {
        SAI_DEBUG ("Error Scheduler object not found.");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    return sai_dump_scheduler_node(scheduler_node);
}

sai_status_t sai_dump_all_queue_nodes (void)
{
    rbtree_handle           queue_tree;
    dn_sai_qos_queue_t      *queue_node = NULL;

    queue_tree = sai_qos_access_global_config()->queue_tree;

    if (NULL == queue_tree) {
        SAI_DEBUG ("Queue not initialized");
        return SAI_STATUS_UNINITIALIZED;
    }

    queue_node = std_rbtree_getfirst (queue_tree);

    while(queue_node != NULL) {
        sai_dump_queue_node(queue_node);
        queue_node = std_rbtree_getnext(queue_tree, queue_node);
    }
    return SAI_STATUS_SUCCESS;
}


sai_status_t sai_dump_all_sg_nodes (void)
{
    rbtree_handle             sched_group_tree;
    dn_sai_qos_sched_group_t  *sched_group_node = NULL;

    sched_group_tree = sai_qos_access_global_config()->scheduler_group_tree;

    if (NULL == sched_group_tree) {
        SAI_DEBUG ("Scheduler group not initialized");
        return SAI_STATUS_UNINITIALIZED;
    }

    sched_group_node = std_rbtree_getfirst(sched_group_tree);
    while(sched_group_node != NULL) {
        sai_dump_sg_node(sched_group_node);
        sched_group_node = std_rbtree_getnext(sched_group_tree, sched_group_node);
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_dump_all_scheduler_nodes (void)
{
    rbtree_handle           scheduler_tree;
    dn_sai_qos_scheduler_t  *scheduler_node = NULL;

    scheduler_tree = sai_qos_access_global_config()->scheduler_tree;

    if (NULL == scheduler_tree) {
        SAI_DEBUG ("scheduler not initialized");
        return SAI_STATUS_UNINITIALIZED;
    }

    scheduler_node = std_rbtree_getfirst (scheduler_tree);

    while(scheduler_node != NULL) {
        sai_dump_scheduler_node(scheduler_node);
        scheduler_node = std_rbtree_getnext(scheduler_tree, scheduler_node);
    }
    return SAI_STATUS_SUCCESS;
}


sai_status_t sai_dump_child_nodes (dn_sai_qos_sched_group_t *sg_node, char *level_str)
{
    dn_sai_qos_sched_group_t *child_sg_node = NULL;
    dn_sai_qos_queue_t *child_queue_node = NULL;
    uint_t sg_idx = 0;
    uint_t queue_idx = 0;
    char hierarchy_str[SAI_MAX_HIERARCHY_STRLEN]= {0};

    STD_ASSERT (sg_node != NULL);
    STD_ASSERT (level_str != NULL);

    snprintf(hierarchy_str, SAI_MAX_HIERARCHY_STRLEN, "%s%s",
             level_str, SAI_HIERARCHY_LEVEL_CHAR);

    for(child_sg_node = sai_qos_sched_group_get_first_child_sched_group(sg_node);
        child_sg_node != NULL;
        child_sg_node = sai_qos_sched_group_get_next_child_sched_group(sg_node, child_sg_node))
    {
        SAI_DEBUG ("%s SG ID %u : 0x%"PRIx64"", level_str, sg_idx, child_sg_node->key.sched_group_id);
        sai_dump_child_nodes(child_sg_node,hierarchy_str);
        sg_idx++;
    }

    for(child_queue_node = sai_qos_sched_group_get_first_child_queue(sg_node);
        child_queue_node != NULL;
        child_queue_node = sai_qos_sched_group_get_next_child_queue(sg_node, child_queue_node))
    {
        SAI_DEBUG ("%s Queue ID %u : 0x%"PRIx64"", level_str, queue_idx,
                   child_queue_node->key.queue_id);
        queue_idx++;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_dump_port_hierarchy(sai_object_id_t port_id)
{
    char hierarchy_str[SAI_MAX_HIERARCHY_STRLEN]= {0};
    uint_t level = 0;
    dn_sai_qos_sched_group_t *sg_node = NULL;
    dn_sai_qos_port_t *port_node = sai_qos_port_node_get(port_id);
    uint_t sg_count = 0;

    if(port_node == NULL) {
        SAI_DEBUG ("Error Port object not found.");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    safestrncpy(hierarchy_str,SAI_HIERARCHY_LEVEL_CHAR,SAI_MAX_HIERARCHY_STRLEN);
    STD_ASSERT (port_node != NULL);

    SAI_DEBUG ("Port ID   : 0x%"PRIx64"", port_node->port_id);


    for (sg_node = sai_qos_port_get_first_sched_group (port_node, level);
        (sg_node != NULL);
        sg_node = sai_qos_port_get_next_sched_group (port_node, sg_node)) {
        SAI_DEBUG ("SG ID %u : 0x%"PRIx64"", sg_count, sg_node->key.sched_group_id);
        sai_dump_child_nodes(sg_node,hierarchy_str);
        sg_count++;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_dump_all_parents(sai_object_id_t oid)
{
    sai_object_type_t obj_type = sai_uoid_obj_type_get(oid);
    dn_sai_qos_queue_t *queue_node = NULL;
    dn_sai_qos_sched_group_t *sg_node = NULL;

    switch(obj_type) {
        case SAI_OBJECT_TYPE_QUEUE:
            queue_node = sai_qos_queue_node_get (oid);

            if (NULL == queue_node) {
                SAI_DEBUG ("Queue 0x%"PRIx64" does not exist in tree.", oid);
                return SAI_STATUS_ITEM_NOT_FOUND;
            }

            SAI_DEBUG ("Queue ID: 0x%"PRIx64"",oid);

            if(queue_node->parent_sched_group_id != SAI_NULL_OBJECT_ID) {
                sai_dump_all_parents(queue_node->parent_sched_group_id);
            }
            break;

        case SAI_OBJECT_TYPE_SCHEDULER_GROUP:
            sg_node = sai_qos_sched_group_node_get(oid);

            if(NULL == sg_node) {
                SAI_DEBUG ("SG 0x%"PRIx64" does not exist in tree.", oid);
                return SAI_STATUS_ITEM_NOT_FOUND;
            }

            SAI_DEBUG ("SG Level %u node : 0x%"PRIx64"",sg_node->hierarchy_level,oid);

            if(sg_node->parent_id != SAI_NULL_OBJECT_ID) {
                sai_dump_all_parents(sg_node->parent_id);
            }
            break;

        default:
            SAI_DEBUG (" Error Invalid object ID 0x%"PRIx64"",oid);
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_dump_pg_node(dn_sai_qos_pg_t *pg_node)
{

    STD_ASSERT(pg_node != NULL);

    SAI_DEBUG ("\nPriority group id: 0x%"PRIx64"", pg_node->key.pg_id);
    SAI_DEBUG ("---------------------------------------------");
    SAI_DEBUG ("Port ID            : 0x%"PRIx64"", pg_node->port_id);
    SAI_DEBUG ("Buffer Profile ID  : 0x%"PRIx64"", pg_node->buffer_profile_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_dump_pg_info(sai_object_id_t pg_id)
{
     dn_sai_qos_pg_t *pg_node;

     pg_node = sai_qos_pg_node_get(pg_id);
     if(pg_node == NULL) {
         SAI_DEBUG ("Error PG object not found.");
         return SAI_STATUS_ITEM_NOT_FOUND;
     }
     return sai_dump_pg_node(pg_node);
}

sai_status_t sai_dump_all_pg_nodes (void)
{
    rbtree_handle         pg_tree;
    dn_sai_qos_pg_t      *pg_node = NULL;

    pg_tree = sai_qos_access_global_config()->pg_tree;

    if (NULL == pg_tree) {
        SAI_DEBUG ("PG not initialized");
        return SAI_STATUS_UNINITIALIZED;
    }

    pg_node = std_rbtree_getfirst (pg_tree);

    while(pg_node != NULL) {
        sai_dump_pg_node(pg_node);
        pg_node = std_rbtree_getnext(pg_tree, pg_node);
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_dump_buffer_profile_node(dn_sai_qos_buffer_profile_t *buffer_profile_node)
{
    dn_sai_qos_queue_t *queue_node = NULL;
    dn_sai_qos_pg_t *pg_node = NULL;
    dn_sai_qos_port_t *port_node;
    uint_t pg_count = 0;
    uint_t port_count = 0;
    uint_t queue_count = 0;

    STD_ASSERT(buffer_profile_node != NULL);

    SAI_DEBUG ("\nBuffer profile id: 0x%"PRIx64"", buffer_profile_node->key.profile_id);
    SAI_DEBUG ("---------------------------------------------");
    SAI_DEBUG ("Buffer pool ID     : 0x%"PRIx64"", buffer_profile_node->buffer_pool_id);
    SAI_DEBUG ("Size  : %u", buffer_profile_node->size);
    SAI_DEBUG ("Threshold Enable   : %d", buffer_profile_node->profile_th_enable);
    SAI_DEBUG ("Threshold mode     : %d", buffer_profile_node->threshold_mode);
    SAI_DEBUG ("Dynamic threshold  : %d", buffer_profile_node->dynamic_th);
    SAI_DEBUG ("Static threshold   : %u", buffer_profile_node->static_th);
    SAI_DEBUG ("XOFF threshold     : %u", buffer_profile_node->xoff_th);
    SAI_DEBUG ("XON threshold      : %u", buffer_profile_node->xon_th);
    SAI_DEBUG ("Num Reference      : %u", buffer_profile_node->num_ref);

    SAI_DEBUG ("\nPort List");
    SAI_DEBUG ("---------------------------------------------");
    for (port_node = sai_qos_buffer_profile_get_first_port (buffer_profile_node);
         (port_node != NULL) ; port_node =
         sai_qos_buffer_profile_get_next_port (buffer_profile_node, port_node)) {
        SAI_DEBUG ("port ID %u: 0x%"PRIx64"", port_count,  port_node->port_id);
        port_count++;
    }

    SAI_DEBUG ("\nPriority group List");
    SAI_DEBUG ("---------------------------------------------");
    for (pg_node = sai_qos_buffer_profile_get_first_pg (buffer_profile_node);
         (pg_node != NULL) ; pg_node =
         sai_qos_buffer_profile_get_next_pg (buffer_profile_node, pg_node)) {
        SAI_DEBUG ("PG ID %u: 0x%"PRIx64"", pg_count,  pg_node->key.pg_id);
        pg_count++;
    }

    SAI_DEBUG ("\nQueue List");
    SAI_DEBUG ("---------------------------------------------");
    for (queue_node = sai_qos_buffer_profile_get_first_queue (buffer_profile_node);
         (queue_node != NULL) ; queue_node =
         sai_qos_buffer_profile_get_next_queue (buffer_profile_node, queue_node)) {
        SAI_DEBUG ("queue ID %u: 0x%"PRIx64"", queue_count,  queue_node->key.queue_id);
        queue_count++;
    }


    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_dump_buffer_profile_info (sai_object_id_t profile_id)
{
    dn_sai_qos_buffer_profile_t *buffer_profile_node;

    buffer_profile_node = sai_qos_buffer_profile_node_get(profile_id);

    if(buffer_profile_node == NULL) {
        SAI_DEBUG ("Error Buffer profile object not found.");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    return sai_dump_buffer_profile_node(buffer_profile_node);
}

sai_status_t sai_dump_all_buffer_profile_nodes (void)
{
    rbtree_handle         buffer_profile_tree;
    dn_sai_qos_buffer_profile_t      *buffer_profile_node = NULL;

    buffer_profile_tree = sai_qos_access_global_config()->buffer_profile_tree;

    if (NULL == buffer_profile_tree) {
        SAI_DEBUG ("buffer_profile not initialized");
        return SAI_STATUS_UNINITIALIZED;
    }

    buffer_profile_node = std_rbtree_getfirst (buffer_profile_tree);

    while(buffer_profile_node != NULL) {
        sai_dump_buffer_profile_node (buffer_profile_node);
        buffer_profile_node = std_rbtree_getnext(buffer_profile_tree, buffer_profile_node);
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_dump_buffer_pool_node(dn_sai_qos_buffer_pool_t *buffer_pool_node)
{
    dn_sai_qos_buffer_profile_t *buffer_profile_node = NULL;
    uint_t buffer_profile_count = 0;

    STD_ASSERT(buffer_pool_node != NULL);

    SAI_DEBUG ("\nBuffer pool id: 0x%"PRIx64"", buffer_pool_node->key.pool_id);
    SAI_DEBUG ("---------------------------------------------");
    SAI_DEBUG ("Pool type  : %d", buffer_pool_node->pool_type);
    SAI_DEBUG ("Threshold mode  : %d", buffer_pool_node->threshold_mode);
    SAI_DEBUG ("Size: %u", buffer_pool_node->size);
    SAI_DEBUG ("Shared size  : %u", buffer_pool_node->shared_size);
    SAI_DEBUG ("Num Reference  : %u", buffer_pool_node->num_ref);


    SAI_DEBUG ("\nBuffer Profile List");
    SAI_DEBUG ("---------------------------------------------");
    for (buffer_profile_node = sai_qos_buffer_pool_get_first_buffer_profile (buffer_pool_node);
         (buffer_profile_node != NULL) ; buffer_profile_node =
         sai_qos_buffer_pool_get_next_buffer_profile (buffer_pool_node, buffer_profile_node)) {
        SAI_DEBUG ("buffer_profile ID %u: 0x%"PRIx64"", buffer_profile_count,  buffer_profile_node->key.profile_id);
        buffer_profile_count++;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_dump_buffer_pool_info (sai_object_id_t pool_id)
{
    dn_sai_qos_buffer_pool_t *buffer_pool_node;

    buffer_pool_node = sai_qos_buffer_pool_node_get(pool_id);

    if(buffer_pool_node == NULL) {
        SAI_DEBUG ("Error Buffer pool object not found.");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    return sai_dump_buffer_pool_node(buffer_pool_node);
}

sai_status_t sai_dump_all_buffer_pool_nodes (void)
{
    rbtree_handle         buffer_pool_tree;
    dn_sai_qos_buffer_pool_t      *buffer_pool_node = NULL;

    buffer_pool_tree = sai_qos_access_global_config()->buffer_pool_tree;

    if (NULL == buffer_pool_tree) {
        SAI_DEBUG ("buffer_pool not initialized");
        return SAI_STATUS_UNINITIALIZED;
    }

    buffer_pool_node = std_rbtree_getfirst (buffer_pool_tree);

    while(buffer_pool_node != NULL) {
        sai_dump_buffer_pool_node (buffer_pool_node);
        buffer_pool_node = std_rbtree_getnext(buffer_pool_tree, buffer_pool_node);
    }
    return SAI_STATUS_SUCCESS;
}

