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
* @file sai_samplepacket_defs.h
*
* @brief This file contains the samplepacket datastructures definitions and utility
*        functions
*
*************************************************************************/
#ifndef __SAI_SAMPLEPACKET_DEFS_H__
#define __SAI_SAMPLEPACKET_DEFS_H__

#include "std_type_defs.h"
#include "std_rbtree.h"
#include "std_radix.h"

#include "saisamplepacket.h"
#include "saitypes.h"

/** Maximum number of Sample Sessions */
#define SAI_NPU_MAX_SAMPLE_ID 256

/**
 * @brief Sampling mode - Port based / Flow based
 */
typedef enum _dn_sai_samplepacket_mode_t
{
    /** Lower Limit */
    SAI_SAMPLEPACKET_MODE_MIN = 0,

    /** Port Based Sampling */
    SAI_SAMPLEPACKET_MODE_PORT_BASED = 1 << 0,

    /** Flow Based Sampling */
    SAI_SAMPLEPACKET_MODE_FLOW_BASED = 1 << 1,

    /** Upper Limit */
    SAI_SAMPLEPACKET_MODE_MAX = 1 << 2,

} dn_sai_samplepacket_mode_t;

/**
 * @brief Direction of the traffic to be samplepacketed
 */
typedef enum _sai_samplepacket_direction_t {

    /** Ingress samplepacket */
    SAI_SAMPLEPACKET_DIR_INGRESS,

    /** Egress samplepacket */
    SAI_SAMPLEPACKET_DIR_EGRESS,

    /** Maximum direction */
    SAI_SAMPLEPACKET_DIR_MAX,

} sai_samplepacket_direction_t;

/**
 * @brief Datastructure for samplepacket session
 */
typedef struct _dn_sai_samplepacket_session_info_t {

    /** Samplepacket session Id */
    sai_object_id_t session_id;

    /** Sampling type */
    sai_samplepacket_type_t sampling_type;

    /** Sample rate */
    sai_uint32_t sample_rate;

    /** Samplepacket ports tree */
    rbtree_handle port_tree;
} dn_sai_samplepacket_session_info_t;

/**
 * @brief Datastructure of source port info key maintained per session
 */
typedef struct _dn_sai_samplepacket_port_info_key_t {

    /** Samplepacket Source Port */
    sai_object_id_t samplepacket_port;

    /** Samplepacket direction */
    sai_samplepacket_direction_t samplepacket_direction;
} dn_sai_samplepacket_port_info_key_t;

/**
 * @brief Datastructure of source port info maintained per session
 */
typedef struct _dn_sai_samplepacket_port_info_t {

    /** Samplepacket Source Port Info key */
    dn_sai_samplepacket_port_info_key_t key;

    /** Samplepacket mode of type @Sa dn_sai_samplepacket_mode_t */
    uint_t sample_mode;

    /** reference count for this port info node */
    uint_t ref_count;
} dn_sai_samplepacket_port_info_t;

/**
 * @brief Datastructure of acl info key per session
 */
typedef struct _dn_sai_samplepacket_acl_info_key_t {
    /** Samplepacket Source Port */
    sai_object_id_t port_id;

    /** Samplepacket direction */
    sai_samplepacket_direction_t samplepacket_direction;

    /** Samplepacket object id */
    sai_object_id_t sample_object;

} dn_sai_samplepacket_acl_info_key_t;

/**
 * @brief Datastructure of acl info maintained per session
 */
typedef struct _dn_sai_samplepacket_acl_info_t {
    /** Radix tree head */
    std_rt_head head;

    /** Radix tree key */
    dn_sai_samplepacket_acl_info_key_t key;
} dn_sai_samplepacket_acl_info_t;
#endif /* __SAI_SAMPLEPACKET_DEFS_H__ */
