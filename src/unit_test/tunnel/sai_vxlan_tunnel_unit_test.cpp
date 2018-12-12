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
* @file  sai_vxlan_tunnel_unit_test.cpp
*
* @brief This file contains test cases for VXLAN Tunnel functionality.
*
*************************************************************************/

#include "gtest/gtest.h"

#include "sai_tunnel_unit_test.h"
#include "sai_l3_unit_test_utils.h"

extern "C" {
#include "sai.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saitunnel.h"
#include "saibridge.h"
#include <string.h>
}

void sai_test_vxlan_assert_success(sai_status_t status,
                                   bool assert_success)
{
    if(assert_success) {
        ASSERT_EQ (SAI_STATUS_SUCCESS, status);
    } else {
        ASSERT_NE (SAI_STATUS_SUCCESS, status);
    }
}

void sai_test_vxlan_expect_success(sai_status_t status,
                                   bool expect_success)
{
    if(expect_success) {
        EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    } else {
        EXPECT_NE (SAI_STATUS_SUCCESS, status);
    }
}

void sai_test_vxlan_1d_bridge_create(sai_object_id_t *bridge_id,
                                     bool assert_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_bridge_create (bridge_id, 1, SAI_BRIDGE_ATTR_TYPE,
                                     SAI_BRIDGE_TYPE_1D);

    sai_test_vxlan_assert_success(status, assert_success);
}

void sai_test_vxlan_1d_bridge_remove(sai_object_id_t bridge_id,
                                     bool expect_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_bridge_remove(bridge_id);

    sai_test_vxlan_expect_success(status, expect_success);
}

void sai_test_vxlan_encap_tunnel_map_create(sai_object_id_t *encap_map_id,
                                            bool assert_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_tunnel_map_create(encap_map_id, 1,
                                                       SAI_TUNNEL_MAP_ATTR_TYPE,
                                                       SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI);

    sai_test_vxlan_assert_success(status, assert_success);
}

void sai_test_vxlan_decap_tunnel_map_create(sai_object_id_t *decap_map_id,
                                            bool assert_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_tunnel_map_create(decap_map_id, 1,
                                                       SAI_TUNNEL_MAP_ATTR_TYPE,
                                                       SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF);

    sai_test_vxlan_assert_success(status, assert_success);
}

void sai_test_vxlan_tunnel_map_remove(sai_object_id_t tunnel_map_id,
                                      bool expect_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_tunnel_map_remove(tunnel_map_id);

    sai_test_vxlan_expect_success(status, expect_success);
}

void sai_test_vxlan_encap_map_entry_create(sai_object_id_t *encap_map_entry_id,
                                           sai_object_id_t encap_map_id,
                                           sai_object_id_t bridge_id,
                                           sai_uint32_t vnid,
                                           bool assert_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_tunnel_map_entry_create(encap_map_entry_id, 4,
                                              SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE,
                                              SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI,
                                              SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP,
                                              encap_map_id,
                                              SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_KEY,
                                              bridge_id,
                                              SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE,
                                              vnid);

    sai_test_vxlan_assert_success(status, assert_success);
}

void sai_test_vxlan_decap_map_entry_create(sai_object_id_t *decap_map_entry_id,
                                           sai_object_id_t decap_map_id,
                                           sai_uint32_t vnid,
                                           sai_object_id_t bridge_id,
                                           bool assert_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_tunnel_map_entry_create(decap_map_entry_id, 4,
                                              SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE,
                                              SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF,
                                              SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP,
                                              decap_map_id,
                                              SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_KEY,
                                              vnid,
                                              SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE,
                                              bridge_id);

    sai_test_vxlan_assert_success(status, assert_success);
}

void sai_test_vxlan_tunnel_map_entry_remove(sai_object_id_t tunnel_map_entry_id,
                                            bool expect_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_tunnel_map_entry_remove(tunnel_map_entry_id);

    sai_test_vxlan_expect_success(status, expect_success);
}

void sai_test_vxlan_encap_map_entry_set(sai_object_id_t tunnel_map_entry_id,
                                        sai_uint32_t vnid,
                                        bool expect_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_tunnel_map_entry_set(tunnel_map_entry_id,
                                                         SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE,
                                                         vnid);

    sai_test_vxlan_expect_success(status, expect_success);
}

void sai_test_vxlan_decap_map_entry_set(sai_object_id_t tunnel_map_entry_id,
                                        sai_object_id_t bridge_id,
                                        bool expect_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_tunnel_map_entry_set(tunnel_map_entry_id,
                                                          SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE,
                                                          bridge_id);

    sai_test_vxlan_expect_success(status, expect_success);
}

void sai_test_vxlan_encap_map_entry_get(sai_object_id_t tunnel_map_entry_id,
                                        unsigned int attr_count,
                                        sai_attribute_t *p_attr_list,
                                        bool expect_success)
{
    sai_status_t status;
    unsigned int encap_attr_count = 4;
    sai_attribute_t encap_attr_list[encap_attr_count];

    if(attr_count < encap_attr_count) {
        sai_test_vxlan_expect_success(SAI_STATUS_FAILURE, expect_success);
    }

    status = saiTunnelTest::sai_test_tunnel_map_entry_get (tunnel_map_entry_id,
                                                           encap_attr_list,
                                                           encap_attr_count,
                                                           SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE,
                                                           SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP,
                                                           SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_KEY,
                                                           SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE);

    sai_test_vxlan_expect_success(status, expect_success);
    memcpy(p_attr_list, &encap_attr_list, sizeof(encap_attr_list));
}

void sai_test_vxlan_encap_map_entry_verify_bridge(unsigned int attr_count,
                                                  sai_attribute_t *p_attr_list,
                                                  sai_object_id_t bridge_id)
{
    unsigned int index = 0;
    sai_status_t status = SAI_STATUS_FAILURE;

    for(index = 0; index < attr_count; index++) {
        if(p_attr_list[index].id == SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_KEY) {
            if(p_attr_list[index].value.oid == bridge_id) {
                status = SAI_STATUS_SUCCESS;
                break;
            }
        }
    }

    EXPECT_EQ(SAI_STATUS_SUCCESS, status);
}

void sai_test_vxlan_encap_map_entry_verify_vnid(unsigned int attr_count,
                                                sai_attribute_t *p_attr_list,
                                                sai_uint32_t vnid)
{
    unsigned int index = 0;
    sai_status_t status = SAI_STATUS_FAILURE;

    for(index = 0; index < attr_count; index++) {
        if(p_attr_list[index].id == SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE) {
            if(p_attr_list[index].value.u32 == vnid) {
                status = SAI_STATUS_SUCCESS;
                break;
            }
        }
    }

    EXPECT_EQ(SAI_STATUS_SUCCESS, status);
}

void sai_test_vxlan_decap_map_entry_get(sai_object_id_t tunnel_map_entry_id,
                                        unsigned int attr_count,
                                        sai_attribute_t *p_attr_list,
                                        bool expect_success)
{
    sai_status_t status;
    unsigned int decap_attr_count = 4;
    sai_attribute_t decap_attr_list[decap_attr_count];

    if(attr_count < decap_attr_count) {
        sai_test_vxlan_expect_success(SAI_STATUS_FAILURE, expect_success);
    }

    status = saiTunnelTest::sai_test_tunnel_map_entry_get (tunnel_map_entry_id,
                                                           decap_attr_list,
                                                           decap_attr_count,
                                                           SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE,
                                                           SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP,
                                                           SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE,
                                                           SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_KEY);

    sai_test_vxlan_expect_success(status, expect_success);
    memcpy(p_attr_list, &decap_attr_list, sizeof(decap_attr_list));
}

void sai_test_vxlan_decap_map_entry_verify_bridge(unsigned int attr_count,
                                                  sai_attribute_t *p_attr_list,
                                                  sai_object_id_t bridge_id)
{
    unsigned int index = 0;
    sai_status_t status = SAI_STATUS_FAILURE;

    for(index = 0; index < attr_count; index++) {
        if(p_attr_list[index].id == SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE) {
            if(p_attr_list[index].value.oid == bridge_id) {
                status = SAI_STATUS_SUCCESS;
                break;
            }
        }
    }

    EXPECT_EQ(SAI_STATUS_SUCCESS, status);
}

void sai_test_vxlan_decap_map_entry_verify_vnid(unsigned int attr_count,
                                                sai_attribute_t *p_attr_list,
                                                sai_uint32_t vnid)
{
    unsigned int index = 0;
    sai_status_t status = SAI_STATUS_FAILURE;

    for(index = 0; index < attr_count; index++) {
        if(p_attr_list[index].id == SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_KEY) {
            if(p_attr_list[index].value.u32 == vnid) {
                status = SAI_STATUS_SUCCESS;
                break;
            }
        }
    }

    EXPECT_EQ(SAI_STATUS_SUCCESS, status);
}

void sai_test_vxlan_tunnel_create(sai_object_id_t *tunnel_id,
                                  sai_object_id_t underlay_rif,
                                  sai_object_id_t overlay_rif,
                                  const char *tunnel_sip,
                                  sai_object_id_t encap_map_id,
                                  sai_object_id_t decap_map_id,
                                  bool assert_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_tunnel_create (tunnel_id, 6,
                                     SAI_TUNNEL_ATTR_TYPE,
                                     SAI_TUNNEL_TYPE_VXLAN,
                                     SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE,
                                     underlay_rif,
                                     SAI_TUNNEL_ATTR_OVERLAY_INTERFACE,
                                     overlay_rif,
                                     SAI_TUNNEL_ATTR_ENCAP_SRC_IP,
                                     SAI_IP_ADDR_FAMILY_IPV4,
                                     tunnel_sip,
                                     SAI_TUNNEL_ATTR_ENCAP_MAPPERS,
                                     1, encap_map_id,
                                     SAI_TUNNEL_ATTR_DECAP_MAPPERS,
                                     1, decap_map_id);

    sai_test_vxlan_assert_success(status, assert_success);
}

void sai_test_vxlan_tunnel_remove(sai_object_id_t tunnel_id,
                                  bool expect_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_tunnel_remove (tunnel_id);

    sai_test_vxlan_expect_success(status, expect_success);
}

void sai_test_vxlan_access_port_create(sai_object_id_t *bridge_sub_port_id,
                                       sai_object_id_t bridge_id,
                                       sai_object_id_t port_id,
                                       sai_uint32_t vlan_id,
                                       bool assert_success)

{
    sai_status_t status;

    status = saiTunnelTest::sai_test_bridge_port_create(bridge_sub_port_id, 4,
                                          SAI_BRIDGE_PORT_ATTR_TYPE,
                                          SAI_BRIDGE_PORT_TYPE_SUB_PORT,
                                          SAI_BRIDGE_PORT_ATTR_BRIDGE_ID,
                                          bridge_id,
                                          SAI_BRIDGE_PORT_ATTR_PORT_ID,
                                          port_id,
                                          SAI_BRIDGE_PORT_ATTR_VLAN_ID,
                                          vlan_id);

    sai_test_vxlan_assert_success(status, assert_success);
}

void sai_test_vxlan_tunnel_port_create(sai_object_id_t *bridge_tunnel_port_id,
                                       sai_object_id_t bridge_id,
                                       sai_object_id_t tunnel_id,
                                       bool assert_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_bridge_port_create (bridge_tunnel_port_id, 3,
                                          SAI_BRIDGE_PORT_ATTR_TYPE,
                                          SAI_BRIDGE_PORT_TYPE_TUNNEL,
                                          SAI_BRIDGE_PORT_ATTR_BRIDGE_ID,
                                          bridge_id,
                                          SAI_BRIDGE_PORT_ATTR_TUNNEL_ID,
                                          tunnel_id);

    sai_test_vxlan_assert_success(status, assert_success);
}

void sai_test_vxlan_bridge_port_remove(sai_object_id_t bridge_port_id,
                                       bool expect_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_bridge_port_remove (bridge_port_id);

    sai_test_vxlan_expect_success(status, expect_success);
}

void sai_test_vxlan_tunnel_term_create(sai_object_id_t *tunnel_term_id,
                                       sai_object_id_t vr_id,
                                       const char *term_sip,
                                       const char *term_dip,
                                       sai_object_id_t tunnel_id,
                                       bool assert_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_tunnel_term_entry_create (tunnel_term_id, 6,
                                                SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_VR_ID,
                                                vr_id,
                                                SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TYPE,
                                                SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P,
                                                SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_DST_IP,
                                                SAI_IP_ADDR_FAMILY_IPV4, term_dip,
                                                SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP,
                                                SAI_IP_ADDR_FAMILY_IPV4, term_sip,
                                                SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TUNNEL_TYPE,
                                                SAI_TUNNEL_TYPE_VXLAN,
                                                SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID,
                                                tunnel_id);

    sai_test_vxlan_assert_success(status, assert_success);
}

void sai_test_vxlan_tunnel_term_remove(sai_object_id_t tunnel_term_id,
                                       bool expect_success)
{
    sai_status_t status;

    status = saiTunnelTest::sai_test_tunnel_term_entry_remove(tunnel_term_id);

    sai_test_vxlan_expect_success(status, expect_success);
}

TEST_F (saiTunnelTest, create_and_remove_vxlan_tunnel_map_object)
{
    sai_object_id_t    encap_map_id          = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_id          = SAI_NULL_OBJECT_ID;

    sai_test_vxlan_encap_tunnel_map_create(&encap_map_id, true);
    sai_test_vxlan_decap_tunnel_map_create(&decap_map_id, true);

    sai_test_vxlan_tunnel_map_remove (decap_map_id, true);
    sai_test_vxlan_tunnel_map_remove (encap_map_id, true);
}

TEST_F (saiTunnelTest, create_and_remove_vxlan_tunnel_map_entry_object)
{
    sai_object_id_t    bridge_id             = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_id_1        = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_id_1        = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_entry_id_1  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_entry_id_1  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_id_2        = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_id_2        = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_entry_id_2  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_entry_id_2  = SAI_NULL_OBJECT_ID;
    sai_uint32_t       vnid                  = 100;
    /*vnid range is from 0 - 16777215*/
    sai_uint32_t       invalid_vnid          = 16777216;

    sai_test_vxlan_1d_bridge_create(&bridge_id, true);

    sai_test_vxlan_encap_tunnel_map_create(&encap_map_id_1, true);
    sai_test_vxlan_decap_tunnel_map_create(&decap_map_id_1, true);
    sai_test_vxlan_encap_tunnel_map_create(&encap_map_id_2, true);
    sai_test_vxlan_decap_tunnel_map_create(&decap_map_id_2, true);


    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id_1, encap_map_id_1,
                                          bridge_id, vnid, true);

    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id_1, decap_map_id_1,
                                          vnid, bridge_id, true);

    /* Negative cases start*/

    /* Try creating encap/decap entry with invalid bridge id */
    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id_2, encap_map_id_1,
                                          bridge_id + 1, vnid, false);

    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id_2, decap_map_id_1,
                                          vnid, bridge_id + 1, false);

    /* Try creating encap/decap entry with invalid vnid */
    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id_2, encap_map_id_2,
                                          bridge_id, invalid_vnid , false);

    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id_2, decap_map_id_2,
                                          invalid_vnid, bridge_id + 1, false);

    /* Try creating duplicate encap entry for same bridge id in the same tunnel
     * map.*/
    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id_2, encap_map_id_1,
                                          bridge_id, vnid, false);

    /* Try creating encap map entry with decap map object.*/
    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id_2, decap_map_id_1,
                                          bridge_id, vnid, false);

    /* Try creating duplicate decap entry for same vnid in the same tunnel
     * map.*/
    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id_2, decap_map_id_1,
                                          vnid, bridge_id, false);

    /* Try creating decap map entry with encap map object.*/
    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id_2, encap_map_id_1,
                                          vnid, bridge_id, false);
    /*Negative test cases end*/

    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id_2, encap_map_id_2,
                                          bridge_id, vnid, true);

    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id_2, decap_map_id_2,
                                          vnid, bridge_id, true);

    sai_test_vxlan_tunnel_map_entry_remove (decap_map_entry_id_1, true);
    sai_test_vxlan_tunnel_map_entry_remove (encap_map_entry_id_1, true);
    sai_test_vxlan_tunnel_map_remove (decap_map_id_1, true);
    sai_test_vxlan_tunnel_map_remove (encap_map_id_1, true);
    sai_test_vxlan_tunnel_map_entry_remove (decap_map_entry_id_2, true);
    sai_test_vxlan_tunnel_map_entry_remove (encap_map_entry_id_2, true);
    sai_test_vxlan_tunnel_map_remove (decap_map_id_2, true);
    sai_test_vxlan_tunnel_map_remove (encap_map_id_2, true);

    sai_test_vxlan_1d_bridge_remove (bridge_id, true);
}

TEST_F (saiTunnelTest, set_and_get_vxlan_tunnel_map_entry_object)
{
    sai_object_id_t    bridge_id             = SAI_NULL_OBJECT_ID;
    sai_object_id_t    bridge_id_2           = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_id_1        = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_id_1        = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_entry_id_1  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_entry_id_1  = SAI_NULL_OBJECT_ID;
    sai_uint32_t       vnid                  = 100;
    sai_uint32_t       vnid_2                = 200;
    sai_uint32_t       attr_count            = 4;
    sai_attribute_t    tunnel_map_entry_attr_list[attr_count];

    memset(&tunnel_map_entry_attr_list, 0, sizeof(tunnel_map_entry_attr_list));

    sai_test_vxlan_1d_bridge_create(&bridge_id, true);
    sai_test_vxlan_1d_bridge_create(&bridge_id_2, true);

    sai_test_vxlan_encap_tunnel_map_create(&encap_map_id_1, true);
    sai_test_vxlan_decap_tunnel_map_create(&decap_map_id_1, true);

    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id_1, encap_map_id_1,
                                          bridge_id, vnid, true);
    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id_1, decap_map_id_1,
                                          vnid, bridge_id, true);

    sai_test_vxlan_encap_map_entry_get(encap_map_entry_id_1, attr_count,
                                       tunnel_map_entry_attr_list, true);

    sai_test_vxlan_encap_map_entry_verify_bridge(attr_count,
                                                 tunnel_map_entry_attr_list,
                                                 bridge_id);

    sai_test_vxlan_encap_map_entry_verify_vnid(attr_count,
                                               tunnel_map_entry_attr_list,
                                               vnid);

    sai_test_vxlan_decap_map_entry_get(decap_map_entry_id_1, attr_count,
                                       tunnel_map_entry_attr_list, true);

    sai_test_vxlan_decap_map_entry_verify_bridge(attr_count,
                                                 tunnel_map_entry_attr_list,
                                                 bridge_id);

    sai_test_vxlan_decap_map_entry_verify_vnid(attr_count,
                                               tunnel_map_entry_attr_list,
                                               vnid);

    sai_test_vxlan_encap_map_entry_set(encap_map_entry_id_1, vnid_2, true);
    sai_test_vxlan_decap_map_entry_set(decap_map_entry_id_1, bridge_id_2, true);

    sai_test_vxlan_encap_map_entry_get(encap_map_entry_id_1, attr_count,
                                       tunnel_map_entry_attr_list, true);

    sai_test_vxlan_encap_map_entry_verify_vnid(attr_count,
                                               tunnel_map_entry_attr_list,
                                               vnid_2);

    sai_test_vxlan_decap_map_entry_get(decap_map_entry_id_1, attr_count,
                                       tunnel_map_entry_attr_list, true);

    sai_test_vxlan_decap_map_entry_verify_bridge(attr_count,
                                                 tunnel_map_entry_attr_list,
                                                 bridge_id_2);

    sai_test_vxlan_tunnel_map_entry_remove (decap_map_entry_id_1, true);
    sai_test_vxlan_tunnel_map_entry_remove (encap_map_entry_id_1, true);
    sai_test_vxlan_tunnel_map_remove (decap_map_id_1, true);
    sai_test_vxlan_tunnel_map_remove (encap_map_id_1, true);

    sai_test_vxlan_1d_bridge_remove (bridge_id, true);
    sai_test_vxlan_1d_bridge_remove (bridge_id_2, true);
}




TEST_F (saiTunnelTest, create_and_remove_vxlan_tunnel_object)
{
    sai_object_id_t    bridge_id             = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_id          = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_id          = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_entry_id    = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_entry_id    = SAI_NULL_OBJECT_ID;
    sai_object_id_t    tunnel_id             = SAI_NULL_OBJECT_ID;
    const char         *tunnel_sip           = "10.0.0.1";
    sai_uint32_t       vnid                  = 100;

    sai_test_vxlan_1d_bridge_create(&bridge_id, true);

    sai_test_vxlan_encap_tunnel_map_create(&encap_map_id, true);
    sai_test_vxlan_decap_tunnel_map_create(&decap_map_id, true);

    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id, encap_map_id,
                                          bridge_id, vnid, true);
    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id, decap_map_id,
                                          vnid, bridge_id, true);

    sai_test_vxlan_tunnel_create(&tunnel_id, dflt_underlay_rif_id, dflt_overlay_rif_id,
                                 tunnel_sip, encap_map_id, decap_map_id, true);

    /*Negative test cases*/

    /*Try passing invalid underlay rif*/
    sai_test_vxlan_tunnel_create(&tunnel_id,SAI_NULL_OBJECT_ID, dflt_overlay_rif_id,
                                 tunnel_sip, encap_map_id, decap_map_id, false);

    /*Try passing invalid overlay rif*/
    sai_test_vxlan_tunnel_create(&tunnel_id,dflt_underlay_rif_id, SAI_NULL_OBJECT_ID,
                                 tunnel_sip, encap_map_id, decap_map_id, false);

    /*Try passing invalid encap map object*/
    sai_test_vxlan_tunnel_create(&tunnel_id, dflt_underlay_rif_id, dflt_overlay_rif_id,
                                 tunnel_sip, SAI_NULL_OBJECT_ID, decap_map_id, false);

    /*Try passing invalid decap map object*/
    sai_test_vxlan_tunnel_create(&tunnel_id, dflt_underlay_rif_id, dflt_overlay_rif_id,
                                 tunnel_sip, encap_map_id, SAI_NULL_OBJECT_ID, false);

    sai_test_vxlan_tunnel_remove (tunnel_id, true);
    /*Try removing the tunnel id again, it should fail*/
    sai_test_vxlan_tunnel_remove (tunnel_id, false);
    sai_test_vxlan_tunnel_map_entry_remove (decap_map_entry_id, true);
    sai_test_vxlan_tunnel_map_entry_remove (encap_map_entry_id, true);
    sai_test_vxlan_tunnel_map_remove (decap_map_id, true);
    sai_test_vxlan_tunnel_map_remove (encap_map_id, true);
    sai_test_vxlan_1d_bridge_remove (bridge_id, true);
}

TEST_F (saiTunnelTest, create_and_remove_vxlan_tunnel_term_object)
{
    sai_object_id_t    bridge_id             = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_id          = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_id          = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_entry_id    = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_entry_id    = SAI_NULL_OBJECT_ID;
    sai_object_id_t    tunnel_id             = SAI_NULL_OBJECT_ID;
    sai_object_id_t    tunnel_term_id        = SAI_NULL_OBJECT_ID;
    const char         *tunnel_sip           = "10.0.0.1";
    const char         *tunnel_dip           = "20.0.0.1";
    sai_uint32_t       vnid                  = 100;

    sai_test_vxlan_1d_bridge_create(&bridge_id, true);

    sai_test_vxlan_encap_tunnel_map_create(&encap_map_id, true);
    sai_test_vxlan_decap_tunnel_map_create(&decap_map_id, true);

    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id, encap_map_id,
                                          bridge_id, vnid, true);
    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id, decap_map_id,
                                          vnid, bridge_id, true);

    sai_test_vxlan_tunnel_create(&tunnel_id, dflt_underlay_rif_id, dflt_overlay_rif_id,
                                 tunnel_sip, encap_map_id, decap_map_id, true);

    /*Negative case, pass NULL object as tunnel id*/
    sai_test_vxlan_tunnel_term_create(&tunnel_term_id, dflt_vr_id, tunnel_dip,
                                      tunnel_sip, SAI_NULL_OBJECT_ID, false);

    sai_test_vxlan_tunnel_term_create(&tunnel_term_id, dflt_vr_id, tunnel_dip,
                                      tunnel_sip, tunnel_id, true);

    /*Negative case*/
    /*Try creating the same tunnel term entry*/
    sai_test_vxlan_tunnel_term_create(&tunnel_term_id, dflt_vr_id, tunnel_dip,
                                      tunnel_sip, tunnel_id, false);

    sai_test_vxlan_tunnel_term_remove (tunnel_term_id, true);
    /*Negative case, remove the same tunnel terminator object*/
    sai_test_vxlan_tunnel_term_remove (tunnel_term_id, false);

    sai_test_vxlan_tunnel_remove (tunnel_id, true);
    sai_test_vxlan_tunnel_map_entry_remove (decap_map_entry_id, true);
    sai_test_vxlan_tunnel_map_entry_remove (encap_map_entry_id, true);
    sai_test_vxlan_tunnel_map_remove (decap_map_id, true);
    sai_test_vxlan_tunnel_map_remove (encap_map_id, true);
    sai_test_vxlan_1d_bridge_remove (bridge_id, true);
}

TEST_F (saiTunnelTest, create_and_remove_vxlan_tunnel_bridge_ports)
{
    sai_object_id_t    bridge_id_1           = SAI_NULL_OBJECT_ID;
    sai_object_id_t    bridge_id_2           = SAI_NULL_OBJECT_ID;
    sai_object_id_t    bridge_tunnel_port_1  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    bridge_tunnel_port_2  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    bridge_tunnel_port_3  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_id          = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_id          = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_entry_id_1  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_entry_id_1  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_entry_id_2  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_entry_id_2  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    tunnel_id             = SAI_NULL_OBJECT_ID;
    sai_object_id_t    tunnel_term_id        = SAI_NULL_OBJECT_ID;
    sai_uint32_t       vnid_1                = 100;
    sai_uint32_t       vnid_2                = 200;
    const char         *tunnel_sip           = "10.0.0.1";
    const char         *tunnel_dip           = "20.0.0.1";

    sai_test_vxlan_1d_bridge_create(&bridge_id_1, true);
    sai_test_vxlan_1d_bridge_create(&bridge_id_2, true);

    sai_test_vxlan_encap_tunnel_map_create(&encap_map_id, true);
    sai_test_vxlan_decap_tunnel_map_create(&decap_map_id, true);

    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id_1, encap_map_id,
                                          bridge_id_1, vnid_1, true);
    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id_1, decap_map_id,
                                          vnid_1, bridge_id_1, true);

    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id_2, encap_map_id,
                                          bridge_id_2, vnid_2, true);
    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id_2, decap_map_id,
                                          vnid_2, bridge_id_2, true);

    sai_test_vxlan_tunnel_create(&tunnel_id, dflt_underlay_rif_id, dflt_overlay_rif_id,
                                 tunnel_sip, encap_map_id, decap_map_id, true);

    /* Negative case, create tunnel bridge port with invalid tunnel and bridge
     * objects*/
    sai_test_vxlan_tunnel_port_create(&bridge_tunnel_port_1, SAI_NULL_OBJECT_ID,
                                      tunnel_id, false);
    sai_test_vxlan_tunnel_port_create(&bridge_tunnel_port_1, bridge_id_1,
                                      SAI_NULL_OBJECT_ID, false);

    sai_test_vxlan_tunnel_port_create(&bridge_tunnel_port_1, bridge_id_1, tunnel_id, true);
    sai_test_vxlan_tunnel_port_create(&bridge_tunnel_port_2, bridge_id_2, tunnel_id, true);
    /*negative test case, create duplicate tunnel bridge port*/
    sai_test_vxlan_tunnel_port_create(&bridge_tunnel_port_3, bridge_id_2, tunnel_id, false);

    sai_test_vxlan_tunnel_term_create(&tunnel_term_id, dflt_vr_id, tunnel_dip,
                                      tunnel_sip, tunnel_id, true);

    sai_test_vxlan_tunnel_term_remove (tunnel_term_id, true);
    sai_test_vxlan_bridge_port_remove (bridge_tunnel_port_1, true);
    sai_test_vxlan_bridge_port_remove (bridge_tunnel_port_2, true);
    /*Negative case, try removing deleted bridge port*/
    sai_test_vxlan_bridge_port_remove (bridge_tunnel_port_2, false);

    sai_test_vxlan_tunnel_remove (tunnel_id, true);
    sai_test_vxlan_tunnel_map_entry_remove (decap_map_entry_id_1, true);
    sai_test_vxlan_tunnel_map_entry_remove (encap_map_entry_id_1, true);
    sai_test_vxlan_tunnel_map_entry_remove (decap_map_entry_id_2, true);
    sai_test_vxlan_tunnel_map_entry_remove (encap_map_entry_id_2, true);
    sai_test_vxlan_tunnel_map_remove (decap_map_id, true);
    sai_test_vxlan_tunnel_map_remove (encap_map_id, true);
    sai_test_vxlan_1d_bridge_remove (bridge_id_1, true);
    sai_test_vxlan_1d_bridge_remove (bridge_id_2, true);
}

TEST_F (saiTunnelTest, modify_encap_decap_mapping)
{
    sai_object_id_t    bridge_id_1           = SAI_NULL_OBJECT_ID;
    sai_object_id_t    bridge_id_2           = SAI_NULL_OBJECT_ID;
    sai_object_id_t    bridge_tunnel_port_1  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    bridge_tunnel_port_2  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_id          = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_id          = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_entry_id_1  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_entry_id_1  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_entry_id_2  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_entry_id_2  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_entry_id_3  = SAI_NULL_OBJECT_ID;
    sai_object_id_t    tunnel_id             = SAI_NULL_OBJECT_ID;
    sai_object_id_t    tunnel_term_id        = SAI_NULL_OBJECT_ID;
    sai_uint32_t       vnid_1                = 100;
    sai_uint32_t       vnid_2                = 200;
    sai_uint32_t       vnid_3                = 300;
    const char         *tunnel_sip           = "10.0.0.1";
    const char         *tunnel_dip           = "20.0.0.1";

    sai_test_vxlan_1d_bridge_create(&bridge_id_1, true);
    sai_test_vxlan_1d_bridge_create(&bridge_id_2, true);

    sai_test_vxlan_encap_tunnel_map_create(&encap_map_id, true);
    sai_test_vxlan_decap_tunnel_map_create(&decap_map_id, true);

    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id_1, encap_map_id,
                                          bridge_id_1, vnid_1, true);
    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id_1, decap_map_id,
                                          vnid_1, bridge_id_1, true);

    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id_2, encap_map_id,
                                          bridge_id_2, vnid_2, true);
    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id_2, decap_map_id,
                                          vnid_2, bridge_id_2, true);

    sai_test_vxlan_tunnel_create(&tunnel_id, dflt_underlay_rif_id, dflt_overlay_rif_id,
                                 tunnel_sip, encap_map_id, decap_map_id, true);

    sai_test_vxlan_tunnel_port_create(&bridge_tunnel_port_1, bridge_id_1, tunnel_id, true);
    sai_test_vxlan_tunnel_port_create(&bridge_tunnel_port_2, bridge_id_2, tunnel_id, true);

    sai_test_vxlan_tunnel_term_create(&tunnel_term_id, dflt_vr_id, tunnel_dip,
                                      tunnel_sip, tunnel_id, true);

    /* Try changing bridge2's mapping to vnid_3 by removing the existing entry
     * This should fail as the tunnel port 2 is using the mapping*/
    sai_test_vxlan_tunnel_map_entry_remove (decap_map_entry_id_2, false);
    sai_test_vxlan_tunnel_map_entry_remove (encap_map_entry_id_2, false);

    /* Try changing the vnid value in encap entry 2 by setting to new vnid
     * This should pass*/
    sai_test_vxlan_encap_map_entry_set(encap_map_entry_id_2, vnid_3, true);

    /*Create a new mapping for vnid_3 to bridge 2*/
    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id_3, decap_map_id,
                                          vnid_3, bridge_id_2, true);
    /*Now remove old decap entry 2*/
    sai_test_vxlan_tunnel_map_entry_remove (decap_map_entry_id_2, true);

    sai_test_vxlan_tunnel_term_remove (tunnel_term_id, true);
    sai_test_vxlan_bridge_port_remove (bridge_tunnel_port_1, true);
    sai_test_vxlan_bridge_port_remove (bridge_tunnel_port_2, true);
    sai_test_vxlan_tunnel_remove (tunnel_id, true);
    sai_test_vxlan_tunnel_map_entry_remove (decap_map_entry_id_1, true);
    sai_test_vxlan_tunnel_map_entry_remove (encap_map_entry_id_1, true);
    sai_test_vxlan_tunnel_map_entry_remove (decap_map_entry_id_3, true);
    sai_test_vxlan_tunnel_map_entry_remove (encap_map_entry_id_2, true);
    sai_test_vxlan_tunnel_map_remove (decap_map_id, true);
    sai_test_vxlan_tunnel_map_remove (encap_map_id, true);
    sai_test_vxlan_1d_bridge_remove (bridge_id_1, true);
    sai_test_vxlan_1d_bridge_remove (bridge_id_2, true);
}

TEST_F (saiTunnelTest, create_and_remove_vxlan_underlay_neighbor)
{
    sai_status_t       status;
    sai_object_id_t    bridge_id             = SAI_NULL_OBJECT_ID;
    sai_object_id_t    bridge_tunnel_port_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_id          = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_id          = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_entry_id    = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_entry_id    = SAI_NULL_OBJECT_ID;
    sai_object_id_t    tunnel_id             = SAI_NULL_OBJECT_ID;
    sai_object_id_t    tunnel_term_id        = SAI_NULL_OBJECT_ID;
    sai_object_id_t    bridge_sub_port_id    = SAI_NULL_OBJECT_ID;
    sai_uint32_t       vnid                  = 100;
    const char         *tunnel_sip           = "10.0.0.1";
    const char         *tunnel_dip           = "20.0.0.1";
    const char         *mac_str_1            = "00:22:22:22:22:22";
    sai_ip_addr_family_t  ip4_af             = SAI_IP_ADDR_FAMILY_IPV4;

    sai_test_vxlan_1d_bridge_create(&bridge_id, true);

    sai_test_vxlan_encap_tunnel_map_create(&encap_map_id, true);
    sai_test_vxlan_decap_tunnel_map_create(&decap_map_id, true);

    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id, encap_map_id,
                                          bridge_id, vnid, true);
    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id, decap_map_id,
                                          vnid, bridge_id, true);

    sai_test_vxlan_tunnel_create(&tunnel_id, dflt_underlay_rif_id, dflt_overlay_rif_id,
                                 tunnel_sip, encap_map_id, decap_map_id, true);

    sai_test_vxlan_access_port_create(&bridge_sub_port_id, bridge_id,
                                      sai_test_tunnel_port_id_get(1), 1, true);

    sai_test_vxlan_tunnel_port_create(&bridge_tunnel_port_id, bridge_id,
                                      tunnel_id, true);

    sai_test_vxlan_tunnel_term_create(&tunnel_term_id, dflt_vr_id, tunnel_dip,
                                      tunnel_sip, tunnel_id, true);

    /*Tunnel dip is directly connected neighbor*/
    status = saiL3Test::sai_test_neighbor_create (dflt_port_rif_id, ip4_af, tunnel_dip,
                                                  saiL3Test::default_neighbor_attr_count,
                                                  SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS,
                                                  mac_str_1);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /*Trapping packets destined to tunnel dip to cpu*/
    status = saiL3Test::sai_test_neighbor_attr_set (dflt_port_rif_id, ip4_af, tunnel_dip,
                                                    SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION,
                                                    SAI_PACKET_ACTION_TRAP, NULL);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = saiL3Test::sai_test_neighbor_remove (dflt_port_rif_id, ip4_af, tunnel_dip);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    sai_test_vxlan_tunnel_term_remove (tunnel_term_id, true);
    sai_test_vxlan_bridge_port_remove (bridge_tunnel_port_id, true);
    sai_test_vxlan_bridge_port_remove (bridge_sub_port_id, true);
    sai_test_vxlan_tunnel_remove (tunnel_id, true);
    sai_test_vxlan_tunnel_map_entry_remove (decap_map_entry_id, true);
    sai_test_vxlan_tunnel_map_entry_remove (encap_map_entry_id, true);
    sai_test_vxlan_tunnel_map_remove (decap_map_id, true);
    sai_test_vxlan_tunnel_map_remove (encap_map_id, true);
    sai_test_vxlan_1d_bridge_remove (bridge_id, true);
}

TEST_F (saiTunnelTest, create_and_remove_vxlan_underlay_ecmp_route)
{
    sai_status_t       status;
    sai_object_id_t    bridge_id             = SAI_NULL_OBJECT_ID;
    sai_object_id_t    bridge_tunnel_port_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_id          = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_id          = SAI_NULL_OBJECT_ID;
    sai_object_id_t    encap_map_entry_id    = SAI_NULL_OBJECT_ID;
    sai_object_id_t    decap_map_entry_id    = SAI_NULL_OBJECT_ID;
    sai_object_id_t    tunnel_id             = SAI_NULL_OBJECT_ID;
    sai_object_id_t    tunnel_term_id        = SAI_NULL_OBJECT_ID;
    sai_object_id_t    bridge_sub_port_id    = SAI_NULL_OBJECT_ID;
    sai_uint32_t       vnid                  = 100;
    const char         *tunnel_sip           = "10.0.0.1";
    const char         *tunnel_dip           = "20.0.0.1";
    const char         *prefix_str           = "20.0.0.0";
    const unsigned int prefix_len            = 24;
    const unsigned int route_attr_count      = 1;
    sai_ip_addr_family_t  ip4_af             = SAI_IP_ADDR_FAMILY_IPV4;

    sai_test_vxlan_1d_bridge_create(&bridge_id, true);

    sai_test_vxlan_encap_tunnel_map_create(&encap_map_id, true);
    sai_test_vxlan_decap_tunnel_map_create(&decap_map_id, true);

    sai_test_vxlan_encap_map_entry_create(&encap_map_entry_id, encap_map_id,
                                          bridge_id, vnid, true);
    sai_test_vxlan_decap_map_entry_create(&decap_map_entry_id, decap_map_id,
                                          vnid, bridge_id, true);

    sai_test_vxlan_tunnel_create(&tunnel_id, dflt_underlay_rif_id, dflt_overlay_rif_id,
                                 tunnel_sip, encap_map_id, decap_map_id, true);

    sai_test_vxlan_access_port_create(&bridge_sub_port_id, bridge_id,
                                      sai_test_tunnel_port_id_get(1), 1, true);

    sai_test_vxlan_tunnel_port_create(&bridge_tunnel_port_id, bridge_id,
                                      tunnel_id, true);

    sai_test_vxlan_tunnel_term_create(&tunnel_term_id, dflt_vr_id, tunnel_dip,
                                      tunnel_sip, tunnel_id, true);

    /* Create a Underlay route for the tunnel DIP */
    status = saiL3Test::sai_test_route_create (dflt_vr_id, ip4_af,
                                               prefix_str, prefix_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               dflt_underlay_nhg_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Delete Underlay route */
    status = saiL3Test::sai_test_route_remove (dflt_vr_id, ip4_af,
                                               prefix_str, prefix_len);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    sai_test_vxlan_tunnel_term_remove (tunnel_term_id, true);
    sai_test_vxlan_bridge_port_remove (bridge_tunnel_port_id, true);
    sai_test_vxlan_bridge_port_remove (bridge_sub_port_id, true);
    sai_test_vxlan_tunnel_remove (tunnel_id, true);
    sai_test_vxlan_tunnel_map_entry_remove (decap_map_entry_id, true);
    sai_test_vxlan_tunnel_map_entry_remove (encap_map_entry_id, true);
    sai_test_vxlan_tunnel_map_remove (decap_map_id, true);
    sai_test_vxlan_tunnel_map_remove (encap_map_id, true);
    sai_test_vxlan_1d_bridge_remove (bridge_id, true);
}

int main (int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

