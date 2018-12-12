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
* @file sai_samplepacket_debug.c
*
* @brief This file contains debug utilities for SAI samplepacket
*        data structures.
*
*************************************************************************/

#include "sai_samplepacket_defs.h"
#include "sai_samplepacket_api.h"
#include "sai_samplepacket_util.h"
#include "sai_debug_utils.h"
#include "sai_oid_utils.h"
#include "saisamplepacket.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "std_mutex_lock.h"
#include "std_assert.h"

void sai_samplepacket_dump_session_node (sai_object_id_t samplepacket_session_id)
{
    dn_sai_samplepacket_port_info_t *p_port_node = NULL;
    dn_sai_samplepacket_session_info_t *p_samplepacket_info = NULL;

    p_samplepacket_info = std_rbtree_getexact (sai_samplepacket_sessions_db_get(),
                                               &samplepacket_session_id);

    if (p_samplepacket_info == NULL) {
        SAI_DEBUG ("Sample packet session not found");
        return ;
    }

    SAI_DEBUG ("Samplepacket session 0x%"PRIx64" params:",
            p_samplepacket_info->session_id);
    SAI_DEBUG ("Sampling type is %d",p_samplepacket_info->sampling_type);
    SAI_DEBUG ("Sample_rate is %u",p_samplepacket_info->sample_rate);

    SAI_DEBUG ("Port List");
    for (p_port_node = std_rbtree_getfirst(p_samplepacket_info->port_tree); p_port_node != NULL;
            p_port_node = std_rbtree_getnext (p_samplepacket_info->port_tree, (void*)p_port_node))
    {
        SAI_DEBUG ("Port 0x%"PRIx64" is  associated to samplepacket session "
                "0x%"PRIx64" in direction %d ref_cnt %d sample_mode %d",
                p_port_node->key.samplepacket_port,
                p_samplepacket_info->session_id,
                p_port_node->key.samplepacket_direction,
                p_port_node->ref_count,
                p_port_node->sample_mode);
    }
    SAI_DEBUG ("\r\n");
}

void sai_samplepacket_dump(sai_object_id_t session_id, bool all)
{
    dn_sai_samplepacket_session_info_t *p_samplepacket_info = NULL;
    sai_object_id_t sai_session_id = SAI_NULL_OBJECT_ID;

    if(all) {
        for (p_samplepacket_info = std_rbtree_getfirst(sai_samplepacket_sessions_db_get());
                p_samplepacket_info != NULL;
                p_samplepacket_info = std_rbtree_getnext (sai_samplepacket_sessions_db_get(),
                    p_samplepacket_info))
        {
            sai_samplepacket_dump_session_node (p_samplepacket_info->session_id);
        }
    } else {
        if(sai_is_obj_id_samplepkt_session(session_id)) {
            sai_session_id = session_id;
        } else {
            sai_session_id = sai_uoid_create(
                    SAI_OBJECT_TYPE_SAMPLEPACKET,
                    (sai_npu_object_id_t)session_id);
        }
        sai_samplepacket_dump_session_node(sai_session_id);
    }
}
