/************************************************************************
* * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
*************************************************************************/
/**
* @file sai_modules_init.h
*
* @brief This header contains the declarations for initialization
         functions for SAI modules
*
*************************************************************************/
#ifndef _SAI_MODULES_INIT_H_
#define _SAI_MODULES_INIT_H_

#include "saistatus.h"
#include "sai_oid_utils.h"
#include "sai_switch_common.h"

/* Switch Initialization config file handler */
sai_status_t sai_switch_init_config(sai_switch_info_t *sai_switch_info, const char *sai_cfg_file);

sai_status_t sai_port_init(void);

sai_status_t sai_port_init_event_notification(void);

void sai_ports_linkscan_enable(void);

sai_status_t sai_fdb_init(void);

sai_status_t sai_vlan_init(void);

sai_status_t sai_router_init(void);

sai_status_t sai_acl_init(void);

sai_status_t sai_qos_init(void);

sai_status_t sai_stp_init(void);

sai_status_t sai_lag_init(void);

sai_status_t sai_mirror_init(void);

sai_status_t sai_hostintf_init(void);

sai_status_t sai_samplepacket_init(void);

sai_status_t sai_udf_init(void);

sai_status_t sai_hash_obj_init(void);

void sai_hash_obj_deinit (void);

sai_status_t sai_tunnel_init (void);

void sai_tunnel_deinit (void);

sai_status_t sai_l2mc_init (void);

#endif
