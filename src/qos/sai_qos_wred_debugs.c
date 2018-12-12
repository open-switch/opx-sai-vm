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
* @file  sai_qos_wred_debugs.c
*
* @brief This file contains function definitions for debug routines to
*        dump the data structure information for QoS Wred objects.
*
*************************************************************************/
#include "saitypes.h"
#include "sai_qos_util.h"
#include "sai_qos_common.h"
#include "sai_debug_utils.h"
#include "std_type_defs.h"
#include <inttypes.h>

void sai_qos_wred_dump_help (void)
{
    SAI_DEBUG ("******************** Dump routines ***********************");

    SAI_DEBUG ("  void sai_qos_wred_dump (sai_object_id_t wred_id)");
    SAI_DEBUG ("  void sai_qos_wred_dump_all (void)");
}

void sai_qos_wred_dump_node(dn_sai_qos_wred_t  *p_wred_node)
{
    size_t color = 0;
    void *p_wred_link_node = NULL;
    dn_sai_qos_wred_link_t  wred_link_type = DN_SAI_QOS_WRED_LINK_QUEUE;

    if(p_wred_node == NULL){
        SAI_DEBUG("WRED node is NULL");
        return;
    }

    SAI_DEBUG("WRED ID : 0x%"PRIx64" weight: %d ecn_mark_mode: %d",
              p_wred_node->key.wred_id, p_wred_node->weight, p_wred_node->ecn_mark_mode);

    SAI_DEBUG("WRED Thresholds:");
    for(color = 0; color < SAI_QOS_MAX_PACKET_COLORS; color ++){
        SAI_DEBUG("Color %ld", color);
        SAI_DEBUG("Enable: %d Minlimit: %d Maxlimit: %d Drop Probability: %d",
                  p_wred_node->threshold[color].enable, p_wred_node->threshold[color].min_limit,
                  p_wred_node->threshold[color].max_limit,p_wred_node->threshold[color].drop_probability);
    }

    for(wred_link_type = DN_SAI_QOS_WRED_LINK_QUEUE;
            wred_link_type < DN_SAI_QOS_WRED_LINK_MAX;
            wred_link_type++) {
        p_wred_link_node = sai_qos_wred_link_node_get_first(p_wred_node, wred_link_type);

        while(p_wred_link_node != NULL) {
            SAI_DEBUG("WRED applied on %s ID 0x%"PRIx64"",
                    sai_qos_wred_link_str(wred_link_type),
                    sai_qos_wred_link_oid_get(p_wred_link_node, wred_link_type));
            p_wred_link_node =
                sai_qos_wred_link_node_get_next(p_wred_node, p_wred_link_node, wred_link_type);
        }
    }
}

void sai_qos_wred_dump(sai_object_id_t wred_id)
{
    dn_sai_qos_wred_t *p_wred_node = NULL;

    SAI_DEBUG("Dumping the wred node contents: ");
    p_wred_node = sai_qos_wred_node_get(wred_id);

    if(p_wred_node != NULL){
        sai_qos_wred_dump_node(p_wred_node);
    }
    else{
        SAI_DEBUG("Policer id not in tree");
    }
}

void sai_qos_wred_dump_all(void)
{
    rbtree_handle  wred_tree;
    dn_sai_qos_wred_t *p_wred_node = NULL;

    wred_tree = sai_qos_access_global_config()->wred_tree;

    p_wred_node = std_rbtree_getfirst (wred_tree);
    while(p_wred_node != NULL){
        sai_qos_wred_dump_node (p_wred_node);

        p_wred_node = std_rbtree_getnext (wred_tree, p_wred_node);
    }
}
