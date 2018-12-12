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

CREATE TABLE SAI_FDB (mac_address varchar(100), bv_id uint64, IS_STATIC bool, BRIDGE_PORT_ID uint64, PACKET_ACTION varchar(10), FDB_METADATA int, PRIMARY KEY(mac_address, bv_id));
CREATE TABLE SAI_VLAN (vlan_id int, STP_INSTANCE int, MAX_LEARN_LIMIT int, DISABLE_LEARN bool, VLAN_METADATA int, PRIMARY KEY(vlan_id));
CREATE TABLE SAI_VLAN_PORT_LIST (vlan_id int, BRIDGE_PORT_ID uint64, TAGGING_MODE varchar(20), PRIMARY KEY(vlan_id, BRIDGE_PORT_ID),
FOREIGN KEY(vlan_id) REFERENCES SAI_VLAN(vlan_id) ON DELETE CASCADE);

PRAGMA foreign_keys = ON;
