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
* @file sai_mirror_debug.c
*
* @brief This file contains debug utilities for SAI mirror
*        data structures.
*
*************************************************************************/

#include "sai_mirror_defs.h"
#include "sai_mirror_api.h"
#include "sai_l3_util.h"
#include "sai_mirror_util.h"
#include "sai_debug_utils.h"

#include "std_assert.h"
#include "std_mac_utils.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

void sai_mirror_dump_session_params (sai_mirror_session_info_t *p_mirror_info)
{
    char   addr_str [SAI_FIB_MAX_BUFSZ];

    SAI_DEBUG ("Class of service %u", p_mirror_info->session_params.class_of_service);
    SAI_DEBUG ("Monitor Port 0x%"PRIx64"", p_mirror_info->monitor_port);
    if (p_mirror_info->span_type == SAI_MIRROR_SESSION_TYPE_LOCAL) {
        SAI_DEBUG ("SPAN Session");
    } else if (p_mirror_info->span_type == SAI_MIRROR_SESSION_TYPE_REMOTE) {
        SAI_DEBUG ("RSPAN Session");
        SAI_DEBUG ("Vlan TPID 0x%u", p_mirror_info->session_params.tpid);
        SAI_DEBUG ("Vlan Id %u", p_mirror_info->session_params.vlan_id);
        SAI_DEBUG ("Vlan Priority %u", p_mirror_info->session_params.vlan_priority);
    } else if (p_mirror_info->span_type == SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE) {
        SAI_DEBUG ("ERSPAN Session");
        SAI_DEBUG ("Vlan TPID 0x%u", p_mirror_info->session_params.tpid);
        SAI_DEBUG ("Vlan Id %u", p_mirror_info->session_params.vlan_id);
        SAI_DEBUG ("Vlan Priority %u", p_mirror_info->session_params.vlan_priority);
        SAI_DEBUG ("encap_type %d", p_mirror_info->session_params.encap_type);
        SAI_DEBUG ("Ip Header version %u", p_mirror_info->session_params.ip_hdr_version);
        SAI_DEBUG ("Type of service %u", p_mirror_info->session_params.tos);
        SAI_DEBUG ("TTL %u", p_mirror_info->session_params.ttl);
        SAI_DEBUG ("GRE protocol type %u", p_mirror_info->session_params.gre_protocol);
        SAI_DEBUG ("Source IP address %s",
            sai_ip_addr_to_str (&p_mirror_info->session_params.src_ip, addr_str, SAI_FIB_MAX_BUFSZ));
        SAI_DEBUG ("Destination IP address %s",
            sai_ip_addr_to_str (&p_mirror_info->session_params.dst_ip, addr_str, SAI_FIB_MAX_BUFSZ));
        SAI_DEBUG ("Source Mac %s", std_mac_to_string (
                                                    (const hal_mac_addr_t *)
                                                    &p_mirror_info->session_params.src_mac,
                                                     addr_str, SAI_FIB_MAX_BUFSZ));
        SAI_DEBUG ("Destination Mac %s", std_mac_to_string (
                                         (const hal_mac_addr_t *)
                                         &p_mirror_info->session_params.dst_mac, addr_str,
                                          SAI_FIB_MAX_BUFSZ));

    }
}
void sai_mirror_dump_session_node (sai_object_id_t mirror_session_id)
{
    sai_mirror_port_info_t *p_port_node = NULL;
    sai_mirror_session_info_t *p_mirror_info = NULL;

    p_mirror_info = std_rbtree_getexact (sai_mirror_sessions_db_get(), &mirror_session_id);

    if (p_mirror_info == NULL) {
        SAI_MIRROR_LOG_ERR ("Mirror session 0x%"PRIx64" not found", mirror_session_id);
        return;
    }

    SAI_DEBUG ("Mirror session 0x%"PRIx64" params:",
            p_mirror_info->session_id);

    sai_mirror_dump_session_params (p_mirror_info);

    SAI_DEBUG ("Port List");
    for (p_port_node = std_rbtree_getfirst(p_mirror_info->source_ports_tree); p_port_node != NULL;
            p_port_node = std_rbtree_getnext (p_mirror_info->source_ports_tree, (void*)p_port_node))
    {
        SAI_DEBUG ("Port 0x%"PRIx64" is associated to mirror session"
                "0x%"PRIx64" in direction %d",
                p_port_node->mirror_port,
                p_mirror_info->session_id,
                p_port_node->mirror_direction);
    }
    SAI_DEBUG ("\r\n");
}

void sai_mirror_dump(sai_object_id_t session_id, bool all)
{
    sai_mirror_session_info_t *p_mirror_info = NULL;
    sai_object_id_t sai_session_id = SAI_NULL_OBJECT_ID;

    if(all) {
        for (p_mirror_info = std_rbtree_getfirst(sai_mirror_sessions_db_get());
                p_mirror_info != NULL;
                p_mirror_info = std_rbtree_getnext (sai_mirror_sessions_db_get(),
                    p_mirror_info))
        {
            sai_mirror_dump_session_node (p_mirror_info->session_id);
        }
    } else {
        if(sai_is_obj_id_mirror_session(session_id)) {
            sai_session_id = session_id;
        } else {
            sai_session_id = sai_uoid_create(
                    SAI_OBJECT_TYPE_MIRROR_SESSION,
                    (sai_npu_object_id_t)session_id);
        }
        sai_mirror_dump_session_node(sai_session_id);
    }

}
