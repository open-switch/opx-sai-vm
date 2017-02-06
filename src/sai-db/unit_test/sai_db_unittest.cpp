/*
 * Copyright (c) 2016 Dell Inc.
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
 * @file sai_db_unittest.cpp
 */

#include "sai_db_api.h"
#include "saitypes.h"
#include "sairouter.h"
#include "saistatus.h"
#include "sainexthop.h"
#include "sairouterintf.h"
#include "gtest/gtest.h"

#include <stdint.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>

using namespace std;

bool sai_neighbor_db_test(){

    sai_neighbor_entry_t entry;
    entry.rif_id = 24;
    entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    entry.ip_address.addr.ip4 = 12345;
    uint8_t mac_addr[6] = {1,2,3,4,5,6};

    sai_attribute_t attrs[2];
    attrs[0].id = SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS;
    memcpy(attrs[0].value.mac,mac_addr,sizeof(mac_addr));

    attrs[1].id = SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION;
    attrs[1].value.u32 = 1;
    if ( sai_neighbor_create_db_entry(&entry,2,attrs) != SAI_STATUS_SUCCESS) return false;

    mac_addr[0]=100;
    sai_attribute_t set_attr;
    set_attr.id = SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS;
    memcpy(set_attr.value.mac,mac_addr,sizeof(mac_addr));
    if ( sai_neighbor_set_db_entry(&entry, &set_attr) != SAI_STATUS_SUCCESS) return false;

    if ( sai_neighbor_delete_db_entry(&entry) != SAI_STATUS_SUCCESS) return false;

    return true;
}

bool sai_next_hop_db_test(){

    sai_object_id_t nhop_id;

    sai_attribute_t attrs[3];
    attrs[0].id = SAI_NEXT_HOP_ATTR_TYPE;
    attrs[0].value.s32 = 1;


    attrs[1].id = SAI_NEXT_HOP_ATTR_IP;
    attrs[1].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attrs[1].value.ipaddr.addr.ip4 = 1111;

    attrs[2].id = SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID;
    attrs[2].value.oid = 5;
    if ( sai_nexthop_create_db_entry(&nhop_id,3,attrs) != SAI_STATUS_SUCCESS) return false;

    sai_attribute_t set_attr;
    set_attr.id=SAI_NEXT_HOP_ATTR_IP;
    set_attr.value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    set_attr.value.ipaddr.addr.ip4 = 2222;
     if ( sai_nexthop_set_db_entry(nhop_id, &set_attr) != SAI_STATUS_SUCCESS) return false;

 //   if ( sai_nexthop_delete_db_entry(nhop_id) != SAI_STATUS_SUCCESS) return false;

    return true;
}

bool sai_route_test(){
    sai_route_entry_t route_entry;
    route_entry.vr_id =1;
    route_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    route_entry.destination.addr.ip4 = 4321;
    route_entry.destination.mask.ip4 = 3333;

    sai_attribute_t attrs[3];
    attrs[0].id = SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID;
    attrs[0].value.oid = 1000;


    attrs[1].id = SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION;
    attrs[1].value.u32 = 2;

    attrs[2].id = SAI_ROUTE_ENTRY_ATTR_TRAP_PRIORITY;
    attrs[2].value.u8 = 55;
    if ( sai_route_create_db_entry(&route_entry,3,attrs) != SAI_STATUS_SUCCESS) return false;

    sai_attribute_t set_attr;
    set_attr.id=SAI_ROUTE_ENTRY_ATTR_TRAP_PRIORITY;
    set_attr.value.u32 = 23;
    if ( sai_route_set_db_entry(&route_entry,&set_attr) != SAI_STATUS_SUCCESS) return false;

    if (  sai_route_delete_db_entry(&route_entry) != SAI_STATUS_SUCCESS) return false;

    return true;
}

bool sai_router_test(){
    sai_object_id_t vr_id;
    vr_id =11;
    uint8_t mac_addr[6] = {111,32,3,4,155,6};

    sai_attribute_t attrs[3];
    attrs[0].id = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE;
    attrs[0].value.booldata = false;

    attrs[1].id = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE;
    attrs[1].value.booldata = true;

    attrs[2].id = SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS;
    memcpy(attrs[2].value.mac,mac_addr,sizeof(mac_addr));
    if ( sai_router_create_db_entry(&vr_id,3,attrs) != SAI_STATUS_SUCCESS) return false;

    mac_addr[1]=123;
    sai_attribute_t set_attr;
    set_attr.id=SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS;
    memcpy(set_attr.value.mac,mac_addr,sizeof(mac_addr));
    if ( sai_router_set_db_entry(vr_id,&set_attr) != SAI_STATUS_SUCCESS) return false;

    if (   sai_router_delete_db_entry(vr_id) != SAI_STATUS_SUCCESS) return false;

    return true;
}

bool sai_router_intf_test(){

    sai_object_id_t rif_id;
    rif_id =211;
    uint8_t mac_addr[6] = {111,32,3,4,155,6};

    sai_attribute_t attrs[8];
    attrs[0].id = SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE;
    attrs[0].value.booldata = false;

    attrs[1].id = SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE;
    attrs[1].value.booldata = true;

    attrs[2].id = SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS;
    memcpy(attrs[2].value.mac,mac_addr,sizeof(mac_addr));

    attrs[3].id = SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID;
    attrs[3].value.oid = 1;

    attrs[4].id = SAI_ROUTER_INTERFACE_ATTR_TYPE;
    attrs[4].value.u32 = 2;

    attrs[5].id = SAI_ROUTER_INTERFACE_ATTR_PORT_ID;
    attrs[5].value.oid = 3;

    attrs[6].id = SAI_ROUTER_INTERFACE_ATTR_VLAN_ID;
    attrs[6].value.u16 = 4;

    attrs[7].id = SAI_ROUTER_INTERFACE_ATTR_MTU;
    attrs[7].value.u32 = 5;

    if( sai_routerintf_create_db_entry(&rif_id,8,attrs) != SAI_STATUS_SUCCESS) return false;

    mac_addr[1]=123;
    sai_attribute_t set_attr;
    set_attr.id=SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE;
    set_attr.value.booldata = true;
    if(sai_routerintf_set_db_entry(rif_id,&set_attr) != SAI_STATUS_SUCCESS) return false;

    if (sai_routerintf_delete_db_entry(rif_id) != SAI_STATUS_SUCCESS) return false;

    return true;
}

bool sai_route_delete_test(){
    sai_object_id_t nhop_id=1000;
    if(sai_nexthop_delete_db_entry(nhop_id) != SAI_STATUS_SUCCESS) return false;
    return true;
}

TEST(sai_db_unittest, sai_db_create_set_delete_test) {
    ASSERT_EQ(sai_vm_init(),SAI_STATUS_SUCCESS);
    ASSERT_TRUE(sai_neighbor_db_test());
    ASSERT_TRUE(sai_next_hop_db_test());
    ASSERT_TRUE(sai_route_test());
    ASSERT_TRUE(sai_router_test());
    ASSERT_TRUE(sai_router_intf_test());
    ASSERT_TRUE(sai_route_delete_test());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
