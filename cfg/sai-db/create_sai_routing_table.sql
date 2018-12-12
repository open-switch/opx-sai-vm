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

CREATE TABLE SAI_NEIGHBOR (rif_id int, ip_addr varchar(100), DST_MAC_ADDRESS varchar(100), PACKET_ACTION int, NO_HOST_ROUTE bool, META_DATA int,
PRIMARY KEY(rif_id, ip_addr));

CREATE TABLE SAI_NEXT_HOP (nhop_id int, TYPE int, IP varchar(100), ROUTER_INTERFACE_ID int, PRIMARY KEY(nhop_id));

CREATE TABLE SAI_NEXT_HOP_GROUP (nhop_group_id int, NEXT_HOP_COUNT int, TYPE int, PRIMARY KEY(nhop_group_id));

CREATE TABLE SAI_NEXT_HOP_GROUP_LIST( nhop_group_id int, NEXT_HOP_ID int,
FOREIGN KEY(nhop_group_id) REFERENCES SAI_NEXT_HOP_GROUP(nhop_group_id) ON DELETE CASCADE);

CREATE TABLE SAI_ROUTE(vr_id int, ip_addr varchar(100), prefix_len int, PACKET_ACTION int, TRAP_PRIORITY int, NEXT_HOP_ID int, META_DATA int, PRIMARY KEY(vr_id, ip_addr, prefix_len));

CREATE TABLE SAI_ROUTER(vr_id int, ADMIN_V4_STATE bool, ADMIN_V6_STATE bool, SRC_MAC_ADDRESS varchar(100),
VIOLATION_TTL1_ACTION int, VIOLATION_IP_OPTIONS int, PRIMARY KEY(vr_id));

CREATE TABLE SAI_ROUTER_INTF(rif_id int, VIRTUAL_ROUTER_ID int, TYPE int, PORT_ID int, VLAN_ID int, SRC_MAC_ADDRESS varchar(100),
ADMIN_V4_STATE bool, ADMIN_V6_STATE bool,MTU int, PRIMARY KEY (rif_id));

PRAGMA foreign_keys = ON;
