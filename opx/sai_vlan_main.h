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
