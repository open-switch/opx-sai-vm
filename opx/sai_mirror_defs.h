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
* @file sai_mirror_defs.h
*
* @brief This file contains the mirror datastructures definitions and utility
*        functions
*
*************************************************************************/
#ifndef __SAI_MIRROR_DEFS_H__
#define __SAI_MIRROR_DEFS_H__

#include "std_type_defs.h"
#include "std_rbtree.h"

#include "saimirror.h"
#include "saitypes.h"

/**
 * @brief Direction of the traffic to be mirrored
 */
typedef enum _sai_mirror_direction_t {

    /** Ingress mirror */
    SAI_MIRROR_DIR_INGRESS,

    /** Egress mirror */
    SAI_MIRROR_DIR_EGRESS,

} sai_mirror_direction_t;

/**
 * @brief Mirror session parameters
 */
typedef struct _sai_mirror_session_params_t {

    /** Traffic Class of the mirrored traffic */
    uint8_t class_of_service;

    /** Vlan TPID */
    uint16_t tpid;

    /** Vlan Id */
    sai_vlan_id_t vlan_id;

    /** Vlan priority */
    uint8_t vlan_priority;

    /** Encapsulation of the mirrored traffic */
    sai_erspan_encapsulation_type_t encap_type;

    /** L3 header version */
    uint8_t ip_hdr_version;

    /** L3 header type of service */
    uint8_t tos;

    /** L3 header ttl */
    uint8_t ttl;

    /** L3 header source ip */
    sai_ip_address_t src_ip;

    /** L3 header destination ip */
    sai_ip_address_t dst_ip;

    /** L2 header source mac */
    sai_mac_t src_mac;

    /** L2 header destination mac */
    sai_mac_t dst_mac;

    /** GRE protocol type */
    uint16_t gre_protocol;

} sai_mirror_session_params_t;

/**
 * @brief Datastructure for mirror session
 */
typedef struct _sai_mirror_session_info_t {

    /** Mirror session Id */
    sai_object_id_t session_id;

    /** Mirror type SPAN/RSPAN/ERSPAN */
    sai_mirror_session_type_t span_type;

    /** Mirror session parameters based on SPAN/RSPAN/ERSPAN */
    sai_mirror_session_params_t session_params;

    /** Mirror destination port */
    sai_object_id_t monitor_port;

    /** Mirror source ports tree */
    rbtree_handle source_ports_tree;
} sai_mirror_session_info_t;

/**
 * @brief Datastructure of source port info maintained per session
 */
typedef struct _sai_mirror_port_info_t {

    /** Mirror Source Port */
    sai_object_id_t mirror_port;

    /** Mirroring direction */
    sai_mirror_direction_t mirror_direction;
} sai_mirror_port_info_t;

/**
 * @brief Datastructure of mirror session maintained per port
 */
typedef struct _sai_mirror_session_per_port_t {

    /** Mirror session Id*/
    sai_object_id_t session_id;

    /** Mirroring direction */
    sai_mirror_direction_t mirror_direction;
} sai_mirror_session_per_port_t;

#endif /* __SAI_MIRROR_DEFS_H__ */
