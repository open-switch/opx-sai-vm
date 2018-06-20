/************************************************************************
* LEGALESE:   "Copyright (c) 2017, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file  sai_qos_maps_debug.c
*
* @brief This file contains function definitions for debug routines to
*        dump the data structure information for QoS Maps objects.
*
*************************************************************************/
#include "saitypes.h"
#include "sai_qos_util.h"
#include "sai_qos_common.h"
#include "sai_debug_utils.h"
#include "std_type_defs.h"
#include <inttypes.h>

void sai_qos_maps_dump_help (void)
{
    SAI_DEBUG ("******************** Dump routines ***********************");

    SAI_DEBUG ("  void sai_qos_maps_dump (sai_object_id_t map_id)");
    SAI_DEBUG ("  void sai_qos_maps_dump_all (void)");
}

void sai_qos_maps_dump_map_list(sai_qos_map_type_t map_type, sai_qos_map_list_t map_list)
{
    size_t count = 0;
    SAI_DEBUG("Map Type: %d",map_type);

    if(map_type == SAI_QOS_MAP_TYPE_DOT1P_TO_TC){
        for(count = 0; count < map_list.count; count ++){
            SAI_DEBUG("Dot1p: %d  Tc: %d ",map_list.list[count].key.dot1p, map_list.list[count].value.tc);
        }
    } else if(map_type == SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR){
        for(count = 0; count < map_list.count; count ++){
            SAI_DEBUG("Dot1p: %d  color: %d",map_list.list[count].key.dot1p, map_list.list[count].value.color);
        }
    } else if(map_type == SAI_QOS_MAP_TYPE_DSCP_TO_TC){
        for(count = 0; count < map_list.count; count ++){
            SAI_DEBUG("Dscp: %d  Tc: %d ",map_list.list[count].key.dscp, map_list.list[count].value.tc);
        }
    } else if(map_type == SAI_QOS_MAP_TYPE_DSCP_TO_COLOR){
        for(count = 0; count < map_list.count; count ++){
            SAI_DEBUG("Dscp: %d color: %d",map_list.list[count].key.dscp, map_list.list[count].value.color);
        }
    }
    else if(map_type == SAI_QOS_MAP_TYPE_TC_TO_QUEUE){
        for(count = 0; count < map_list.count; count ++){
            SAI_DEBUG("Tc: %d  qid: %d",map_list.list[count].key.tc, map_list.list[count].value.queue_index);
        }
    }
    else if(map_type == SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P){
        for(count = 0; count < map_list.count; count ++){
            SAI_DEBUG("Tc: %d  color: %d Dot1p: %d",map_list.list[count].key.tc, map_list.list[count].key.color,
                      map_list.list[count].value.dot1p);
        }
    }
    else if(map_type == SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP){
        for(count = 0; count < map_list.count; count ++){
            SAI_DEBUG("Tc: %d  color: %d Dscp: %d",map_list.list[count].key.tc, map_list.list[count].key.color,
                      map_list.list[count].value.dscp);
        }
    }
    else if(map_type == SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP){
        for(count = 0; count < map_list.count; count ++){
            SAI_DEBUG("Tc: %d  PG: %d",map_list.list[count].key.tc, map_list.list[count].value.pg);
        }
    }
    else if(map_type == SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE){
        for(count = 0; count < map_list.count; count ++){
            SAI_DEBUG("PFC: %d  qid: %d",map_list.list[count].key.prio, map_list.list[count].value.queue_index);
        }
    }


    else{
        SAI_DEBUG("Map type not supported/not implemented");
    }
}
void sai_qos_maps_dump_node(dn_sai_qos_map_t *p_map_node)
{
    dn_sai_qos_port_t *p_qos_port_node = NULL;
    SAI_DEBUG("Map Id : 0x%"PRIx64"",p_map_node->key.map_id);
    SAI_DEBUG("Map type : %d",p_map_node->map_type);

    SAI_DEBUG("Map list contents:");
    if(p_map_node->map_to_value.list == NULL){
        SAI_DEBUG("Map list is NULL");
    }
    else{
        sai_qos_maps_dump_map_list(p_map_node->map_type, p_map_node->map_to_value);
    }

    SAI_DEBUG("Map applied on portlist:");

    p_qos_port_node  = sai_qos_maps_get_port_node_from_map(p_map_node);

    while(p_qos_port_node != NULL)
    {
        SAI_DEBUG("Portid: 0x%"PRIx64"",p_qos_port_node->port_id);
        p_qos_port_node  = sai_qos_maps_next_port_node_from_map_get(p_map_node, p_qos_port_node);

    }

}

void sai_qos_maps_dump(sai_object_id_t map_id)
{
    dn_sai_qos_map_t *p_map_node = NULL;

    SAI_DEBUG("Dumping the map node contents: ");
    p_map_node = sai_qos_map_node_get(map_id);

    if(p_map_node != NULL){
        sai_qos_maps_dump_node(p_map_node);
    }
    else{
        SAI_DEBUG("Map id not in tree");
    }
}

void sai_qos_maps_dump_all(void)
{
    rbtree_handle  map_tree;
    dn_sai_qos_map_t *p_map_node = NULL;

    map_tree = sai_qos_access_global_config()->map_tree;

    p_map_node = std_rbtree_getfirst (map_tree);

    while (p_map_node) {
        sai_qos_maps_dump_node (p_map_node);

        p_map_node = std_rbtree_getnext (map_tree, p_map_node);
    }
}
