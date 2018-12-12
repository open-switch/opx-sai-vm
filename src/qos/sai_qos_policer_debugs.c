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
* @file  sai_qos_policer_debug.c
*
* @brief This file contains function definitions for debug routines to
*        dump the data structure information for QoS Policer objects.
*
*************************************************************************/
#include "saitypes.h"
#include "sai_qos_util.h"
#include "sai_qos_common.h"
#include "sai_debug_utils.h"
#include "std_type_defs.h"
#include <inttypes.h>

void sai_qos_policer_dump_help (void)
{
    SAI_DEBUG ("******************** Dump routines ***********************");

    SAI_DEBUG ("  void sai_qos_policer_dump (sai_object_id_t policer_id)");
    SAI_DEBUG ("  void sai_qos_policer_dump_all (void)");
}


void sai_qos_policer_dump_node(dn_sai_qos_policer_t *p_policer_node)
{
    size_t count = 0;
    size_t type = 0;
    dn_sai_qos_port_t *p_port_node = NULL;

    SAI_DEBUG("Policer Id : 0x%"PRIx64" Policer mode: %d Meter_type: %d "
              "Color source : %d Cbs %lu Pbs %lu Cir %lu Pir %lu actioncount %d",
              p_policer_node->key.policer_id, p_policer_node->policer_mode,
              p_policer_node->meter_type, p_policer_node->color_source,
              p_policer_node->cbs, p_policer_node->pbs, p_policer_node->cir,
              p_policer_node->pir, p_policer_node->action_count);

    SAI_DEBUG("Policer action list:");

    for(count = 0; count < SAI_POLICER_MAX_ACTION_COUNT; count ++){
        SAI_DEBUG("Action %d Enable %d Value %d",p_policer_node->action_list[count].action,
                  p_policer_node->action_list[count].enable,
                  p_policer_node->action_list[count].value);
    }
    SAI_DEBUG("Policer applied on portlist:");

    for(type = 0; type < SAI_QOS_POLICER_TYPE_MAX; type ++){
        p_port_node = sai_qos_port_node_from_policer_get(p_policer_node, type);

        while(p_port_node != NULL){
            SAI_DEBUG("Policer type %ld Portid: 0x%"PRIx64"",type, p_port_node->port_id);
            p_port_node = sai_qos_next_port_node_from_policer_get(p_policer_node,
                                                                  p_port_node, type);
        }
    }
}

void sai_qos_policer_dump(sai_object_id_t policer_id)
{
    dn_sai_qos_policer_t *p_policer_node = NULL;

    SAI_DEBUG("Dumping the policer node contents: ");
    p_policer_node = sai_qos_policer_node_get(policer_id);

    if(p_policer_node != NULL){
        sai_qos_policer_dump_node(p_policer_node);
    }
    else{
        SAI_DEBUG("Policer id not in tree");
    }
}

void sai_qos_policer_dump_all(void)
{
    rbtree_handle  policer_tree;
    dn_sai_qos_policer_t *p_policer_node = NULL;

    policer_tree = sai_qos_access_global_config()->policer_tree;

    p_policer_node = std_rbtree_getfirst (policer_tree);
    while(p_policer_node != NULL){
        sai_qos_policer_dump_node (p_policer_node);

        p_policer_node = std_rbtree_getnext (policer_tree, p_policer_node);
    }
}
