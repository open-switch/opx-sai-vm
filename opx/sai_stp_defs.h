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
* @file sai_stp_defs.h
*
* @brief This file contains the stp datastructures definitions and utility
*        functions
*
*************************************************************************/

/* OPENSOURCELICENSE */

#ifndef __SAI_STP_DEFS_H__
#define __SAI_STP_DEFS_H__

#include "std_rbtree.h"
#include "saistp.h"
#include "saitypes.h"

/**
 * @brief Datastructure for STG instance
 */
typedef struct _dn_sai_stp_info_t {

    /** stp_inst_id STP Instance Id */
    sai_object_id_t stp_inst_id;

    /** vlan_tree Vlans attached to this STG tree */
    rbtree_handle vlan_tree;

    /** stp_port_tree STP Ports info tree */
    rbtree_handle stp_port_tree;

    /** num_ports Number of ports in STP port info tree */
    uint_t num_ports;
} dn_sai_stp_info_t;

typedef struct _dn_sai_stp_port_info_t {
    /** stp_port_id STP Port SAI UOID */
    sai_object_id_t stp_port_id;

    /** stp_inst_id STP SAI UOID */
    sai_object_id_t stp_inst_id;

    /** bridge_port_id Port SAI UOID */
    sai_object_id_t bridge_port_id;

    /** port_state Port STP state */
    sai_stp_port_state_t port_state;
} dn_sai_stp_port_info_t;

#endif /* __SAI_STP_DEFS_H__ */
