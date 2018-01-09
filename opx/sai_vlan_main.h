/************************************************************************
* * LEGALESE:   "Copyright (c) 2017, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/***
 * \file    sai_vlan_main.h
 *
 * \brief Declaration of SAI VLAN APIs
 */

#if !defined (__SAIVLANMAIN_H_)
#define __SAIVLANMAIN_H_

#include "saitypes.h"

void sai_vlan_dump_global_info(sai_vlan_id_t vlan_id, bool all);

void sai_vlan_dump_member_info(sai_vlan_id_t vlan_id, sai_object_id_t port_id, bool all);

sai_status_t sai_vlan_add_bridge_port_to_default_vlan(sai_object_id_t bridge_port_id);
#endif
