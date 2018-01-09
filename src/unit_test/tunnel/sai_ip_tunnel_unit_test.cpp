/************************************************************************
* LEGALESE:   "Copyright (c) 2016, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file  sai_ip_tunnel_unit_test.cpp
*
* @brief This file contains test cases for IP Tunnel functionality.
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
#include "sainexthop.h"
}

/*
 * Validates IP Tunnel object creation and removal.
 */
TEST_F (saiTunnelTest, create_and_remove_tunnel_object)
{
    sai_status_t       status;
    sai_object_id_t    tunnel_id = SAI_NULL_OBJECT_ID;

    status = sai_test_tunnel_create (&tunnel_id, dflt_tunnel_obj_attr_count,
                                     SAI_TUNNEL_ATTR_TYPE,
                                     SAI_TUNNEL_TYPE_IPINIP,
                                     SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE,
                                     dflt_port_rif_id,
                                     SAI_TUNNEL_ATTR_OVERLAY_INTERFACE,
                                     dflt_overlay_rif_id,
                                     SAI_TUNNEL_ATTR_ENCAP_SRC_IP,
                                     SAI_IP_ADDR_FAMILY_IPV4,
                                     "10.0.0.1");

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_tunnel_remove (tunnel_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates IP Tunnel Termination table entry object creation and removal.
 */
TEST_F (saiTunnelTest, create_and_remove_tunnel_term_entry)
{
    sai_status_t          status;
    sai_object_id_t       tunnel_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t       tunnel_term_id = SAI_NULL_OBJECT_ID;
    const char           *tunnel_sip = "10.0.0.1";
    const char           *tunnel_dip = "20.0.0.1";
    sai_ip_addr_family_t  ip4_af = SAI_IP_ADDR_FAMILY_IPV4;

    status = sai_test_tunnel_create (&tunnel_id, dflt_tunnel_obj_attr_count,
                                     SAI_TUNNEL_ATTR_TYPE,
                                     SAI_TUNNEL_TYPE_IPINIP,
                                     SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE,
                                     dflt_port_rif_id,
                                     SAI_TUNNEL_ATTR_OVERLAY_INTERFACE,
                                     dflt_overlay_rif_id,
                                     SAI_TUNNEL_ATTR_ENCAP_SRC_IP,
                                     SAI_IP_ADDR_FAMILY_IPV4, tunnel_sip);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_tunnel_term_entry_create (&tunnel_term_id,
                                                max_tunnel_term_attr_count,
                                                SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_VR_ID,
                                                dflt_vr_id,
                                                SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TYPE,
                                                SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P,
                                                SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_DST_IP,
                                                ip4_af, tunnel_sip,
                                                SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP,
                                                ip4_af, tunnel_dip,
                                                SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TUNNEL_TYPE,
                                                SAI_TUNNEL_TYPE_IPINIP,
                                                SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID,
                                                tunnel_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_tunnel_term_entry_remove (tunnel_term_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_tunnel_remove (tunnel_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates IP Tunnel Encap Next Hop object creation and removal.
 */
TEST_F (saiTunnelTest, create_and_remove_tunnel_encap_next_hop)
{
    sai_status_t          status;
    sai_object_id_t       tunnel_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t       encap_nh_id = SAI_NULL_OBJECT_ID;
    const char           *tunnel_sip = "10.0.0.1";
    const char           *tunnel_dip = "20.0.0.1";
    const char           *prefix_str = "20.0.0.0";
    const unsigned int    prefix_len = 24;
    const unsigned int    route_attr_count = 1;
    sai_ip_addr_family_t  ip4_af = SAI_IP_ADDR_FAMILY_IPV4;

    status = sai_test_tunnel_create (&tunnel_id, dflt_tunnel_obj_attr_count,
                                     SAI_TUNNEL_ATTR_TYPE,
                                     SAI_TUNNEL_TYPE_IPINIP,
                                     SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE,
                                     dflt_port_rif_id,
                                     SAI_TUNNEL_ATTR_OVERLAY_INTERFACE,
                                     dflt_overlay_rif_id,
                                     SAI_TUNNEL_ATTR_ENCAP_SRC_IP,
                                     SAI_IP_ADDR_FAMILY_IPV4, tunnel_sip);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Underlay route for the tunnel DIP */
    status = saiL3Test::sai_test_route_create (dflt_vr_id, ip4_af,
                                               prefix_str, prefix_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               dflt_underlay_port_nh_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create the tunnel encap next hop object */
    status = saiL3Test::sai_test_nexthop_create (&encap_nh_id,
                                                 dflt_tunnel_encap_nh_attr_count,
                                                 SAI_NEXT_HOP_ATTR_TYPE,
                                                 SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP,
                                                 SAI_NEXT_HOP_ATTR_IP,
                                                 ip4_af, tunnel_dip,
                                                 SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
                                                 dflt_port_rif_id,
                                                 SAI_NEXT_HOP_ATTR_TUNNEL_ID,
                                                 tunnel_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel encap next hop */
    status = saiL3Test::sai_test_nexthop_remove (encap_nh_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the underlay route */
    status = saiL3Test::sai_test_route_remove (dflt_vr_id, ip4_af,
                                               prefix_str, prefix_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel object */
    status = sai_test_tunnel_remove (tunnel_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates IP Tunnel Initiation for IPv6/IPv4 overlay route pointing to
 * a IPv4 Tunnel Encap Next Hop object.
 */
TEST_F (saiTunnelTest, overlay_ip_route_with_tunnel_encap_next_hop)
{
    sai_status_t          status;
    sai_object_id_t       tunnel_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t       encap_nh_id = SAI_NULL_OBJECT_ID;
    const char           *tunnel_sip = "100.1.1.1";
    const char           *tunnel_dip = "200.1.1.1";
    const char           *prefix_str = "200.1.0.0";
    const unsigned int    prefix_len = 16;
    const unsigned int    route_attr_count = 1;
    sai_attribute_t       attr_list [route_attr_count];
    sai_ip_addr_family_t  ip4_af = SAI_IP_ADDR_FAMILY_IPV4;
    sai_ip_addr_family_t  ip6_af = SAI_IP_ADDR_FAMILY_IPV6;
    const char           *overlay_ip6_route = "2001:db8:85a3::";
    const unsigned int    overlay_ip6_route_len = 48;
    const char           *overlay_ip4_route = "201.1.1.0";
    const unsigned int    overlay_ip4_route_len = 24;

    status = sai_test_tunnel_create (&tunnel_id, dflt_tunnel_obj_attr_count,
                                     SAI_TUNNEL_ATTR_TYPE,
                                     SAI_TUNNEL_TYPE_IPINIP,
                                     SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE,
                                     dflt_port_rif_id,
                                     SAI_TUNNEL_ATTR_OVERLAY_INTERFACE,
                                     dflt_overlay_rif_id,
                                     SAI_TUNNEL_ATTR_ENCAP_SRC_IP,
                                     SAI_IP_ADDR_FAMILY_IPV4, tunnel_sip);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Underlay route for the tunnel DIP */
    status = saiL3Test::sai_test_route_create (dflt_vr_id, ip4_af,
                                               prefix_str, prefix_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               dflt_underlay_port_nh_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create the tunnel encap next hop object */
    status = saiL3Test::sai_test_nexthop_create (&encap_nh_id,
                                                 dflt_tunnel_encap_nh_attr_count,
                                                 SAI_NEXT_HOP_ATTR_TYPE,
                                                 SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP,
                                                 SAI_NEXT_HOP_ATTR_IP,
                                                 ip4_af, tunnel_dip,
                                                 SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
                                                 dflt_port_rif_id,
                                                 SAI_NEXT_HOP_ATTR_TUNNEL_ID,
                                                 tunnel_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Overlay IPv6 network route */
    status = saiL3Test::sai_test_route_create (dflt_overlay_vr_id, ip6_af,
                                               overlay_ip6_route,
                                               overlay_ip6_route_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               encap_nh_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Overlay route */
    status = saiL3Test::sai_test_route_attr_get (dflt_overlay_vr_id, ip6_af,
                                                 overlay_ip6_route,
                                                 overlay_ip6_route_len,
                                                 attr_list, route_attr_count,
                                                 SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    EXPECT_EQ (encap_nh_id, attr_list[0].value.oid);

    /* Create a Overlay IPv4 network route */
    status = saiL3Test::sai_test_route_create (dflt_overlay_vr_id, ip4_af,
                                               overlay_ip4_route,
                                               overlay_ip4_route_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               encap_nh_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Overlay route */
    status = saiL3Test::sai_test_route_attr_get (dflt_overlay_vr_id, ip4_af,
                                                 overlay_ip4_route,
                                                 overlay_ip4_route_len,
                                                 attr_list, route_attr_count,
                                                 SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    EXPECT_EQ (encap_nh_id, attr_list[0].value.oid);

    /* Verify that the tunnel encap next hop cannot be removed now */
    status = saiL3Test::sai_test_nexthop_remove (encap_nh_id);

    EXPECT_NE (SAI_STATUS_SUCCESS, status);

    /* Remove the Overlay route */
    status = saiL3Test::sai_test_route_remove (dflt_overlay_vr_id, ip6_af,
                                               overlay_ip6_route,
                                               overlay_ip6_route_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = saiL3Test::sai_test_route_remove (dflt_overlay_vr_id, ip4_af,
                                               overlay_ip4_route,
                                               overlay_ip4_route_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel encap next hop */
    status = saiL3Test::sai_test_nexthop_remove (encap_nh_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the underlay route */
    status = saiL3Test::sai_test_route_remove (dflt_vr_id, ip4_af,
                                               prefix_str, prefix_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel object */
    status = sai_test_tunnel_remove (tunnel_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates creation of ECMP Group of IPv4 Tunnel Encap Next Hop objects and
 * verify IP Tunnel Initiation for overlay route pointing to ECMP of tunnels.
 */
TEST_F (saiTunnelTest, overlay_next_hop_group_with_tunnel_encap_next_hop)
{
    sai_status_t          status;
    sai_object_id_t       tunnel_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t       group_id = SAI_NULL_OBJECT_ID;
    const unsigned int    nh_count = 2;
    sai_object_id_t       encap_nh_id [nh_count];
    const char           *tunnel_sip = "100.1.1.1";
    const char           *tunnel_dip_1 = "200.1.1.1";
    const char           *tunnel_dip_2 = "200.2.2.1";
    const char           *prefix_str_1 = "200.1.1.0";
    const char           *prefix_str_2 = "200.2.2.0";
    const unsigned int    prefix_len = 24;
    const unsigned int    route_attr_count = 1;
    sai_attribute_t       attr_list [route_attr_count];
    sai_ip_addr_family_t  ip4_af = SAI_IP_ADDR_FAMILY_IPV4;
    const char           *overlay_ip4_route = "201.1.1.0";
    const unsigned int    overlay_ip4_route_len = 24;

    status = sai_test_tunnel_create (&tunnel_id, dflt_tunnel_obj_attr_count,
                                     SAI_TUNNEL_ATTR_TYPE,
                                     SAI_TUNNEL_TYPE_IPINIP,
                                     SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE,
                                     dflt_port_rif_id,
                                     SAI_TUNNEL_ATTR_OVERLAY_INTERFACE,
                                     dflt_overlay_rif_id,
                                     SAI_TUNNEL_ATTR_ENCAP_SRC_IP,
                                     SAI_IP_ADDR_FAMILY_IPV4, tunnel_sip);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create Underlay route for the tunnel DIP */
    status = saiL3Test::sai_test_route_create (dflt_vr_id, ip4_af,
                                               prefix_str_1, prefix_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               dflt_underlay_port_nh_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = saiL3Test::sai_test_route_create (dflt_vr_id, ip4_af,
                                               prefix_str_2, prefix_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               dflt_underlay_vlan_nh_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create the tunnel encap next hop objects */
    status = saiL3Test::sai_test_nexthop_create (&encap_nh_id [0],
                                                 dflt_tunnel_encap_nh_attr_count,
                                                 SAI_NEXT_HOP_ATTR_TYPE,
                                                 SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP,
                                                 SAI_NEXT_HOP_ATTR_IP,
                                                 ip4_af, tunnel_dip_1,
                                                 SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
                                                 dflt_port_rif_id,
                                                 SAI_NEXT_HOP_ATTR_TUNNEL_ID,
                                                 tunnel_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = saiL3Test::sai_test_nexthop_create (&encap_nh_id [1],
                                                 dflt_tunnel_encap_nh_attr_count,
                                                 SAI_NEXT_HOP_ATTR_TYPE,
                                                 SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP,
                                                 SAI_NEXT_HOP_ATTR_IP,
                                                 ip4_af, tunnel_dip_2,
                                                 SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
                                                 dflt_vlan_rif_id,
                                                 SAI_NEXT_HOP_ATTR_TUNNEL_ID,
                                                 tunnel_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Next hop Group with the tunnel encap next hop objects */
    status = saiL3Test::sai_test_nh_group_create (&group_id, encap_nh_id,
                                                  saiL3Test::default_nh_group_attr_count,
                                                  SAI_NEXT_HOP_GROUP_ATTR_TYPE,
                                                  SAI_NEXT_HOP_GROUP_TYPE_ECMP,
                                                  SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST,
                                                  nh_count);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify that the tunnel encap next hop cannot be removed now */
    status = saiL3Test::sai_test_nexthop_remove (encap_nh_id[0]);

    EXPECT_NE (SAI_STATUS_SUCCESS, status);

    /* Create a Overlay IPv4 network route pointing to the group */
    status = saiL3Test::sai_test_route_create (dflt_overlay_vr_id, ip4_af,
                                               overlay_ip4_route,
                                               overlay_ip4_route_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               group_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Overlay route */
    status = saiL3Test::sai_test_route_attr_get (dflt_overlay_vr_id, ip4_af,
                                                 overlay_ip4_route,
                                                 overlay_ip4_route_len,
                                                 attr_list, route_attr_count,
                                                 SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    EXPECT_EQ (group_id, attr_list[0].value.oid);

    /* Remove the Overlay route */
    status = saiL3Test::sai_test_route_remove (dflt_overlay_vr_id, ip4_af,
                                               overlay_ip4_route,
                                               overlay_ip4_route_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the Overlay Next hop group */
    status = saiL3Test::sai_test_nh_group_remove (group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel encap next hop */
    status = saiL3Test::sai_test_nexthop_remove (encap_nh_id[0]);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = saiL3Test::sai_test_nexthop_remove (encap_nh_id[1]);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the underlay route */
    status = saiL3Test::sai_test_route_remove (dflt_vr_id, ip4_af,
                                               prefix_str_1, prefix_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = saiL3Test::sai_test_route_remove (dflt_vr_id, ip4_af,
                                               prefix_str_2, prefix_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel object */
    status = sai_test_tunnel_remove (tunnel_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates ECMP underlay paths for IP Tunnel Encap Next Hop object.
 */
TEST_F (saiTunnelTest, tunnel_encap_next_hop_with_underlay_ecmp_group)
{
    sai_status_t          status;
    sai_object_id_t       tunnel_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t       encap_nh_id = SAI_NULL_OBJECT_ID;
    const char           *tunnel_sip = "10.0.0.1";
    const char           *tunnel_dip = "20.0.0.1";
    const char           *prefix_str = "20.0.0.0";
    const unsigned int    prefix_len = 24;
    const unsigned int    route_attr_count = 1;
    sai_ip_addr_family_t  ip4_af = SAI_IP_ADDR_FAMILY_IPV4;
    const char           *overlay_ip4_route = "201.1.1.0";
    const unsigned int    overlay_ip4_route_len = 24;
    sai_object_id_t       test_bridge_port_id = sai_test_tunnel_bridge_port_id_get (2);
    sai_object_id_t       new_bridge_port_id = sai_test_tunnel_bridge_port_id_get (5);
    const char           *mac_str = "00:11:11:11:11:11";

    status = sai_test_tunnel_create (&tunnel_id, dflt_tunnel_obj_attr_count,
                                     SAI_TUNNEL_ATTR_TYPE,
                                     SAI_TUNNEL_TYPE_IPINIP,
                                     SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE,
                                     dflt_port_rif_id,
                                     SAI_TUNNEL_ATTR_OVERLAY_INTERFACE,
                                     dflt_overlay_rif_id,
                                     SAI_TUNNEL_ATTR_ENCAP_SRC_IP,
                                     SAI_IP_ADDR_FAMILY_IPV4, tunnel_sip);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Underlay route for the tunnel DIP */
    status = saiL3Test::sai_test_route_create (dflt_vr_id, ip4_af,
                                               prefix_str, prefix_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               dflt_underlay_nhg_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create the tunnel encap next hop object */
    status = saiL3Test::sai_test_nexthop_create (&encap_nh_id,
                                                 dflt_tunnel_encap_nh_attr_count,
                                                 SAI_NEXT_HOP_ATTR_TYPE,
                                                 SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP,
                                                 SAI_NEXT_HOP_ATTR_IP,
                                                 ip4_af, tunnel_dip,
                                                 SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
                                                 dflt_port_rif_id,
                                                 SAI_NEXT_HOP_ATTR_TUNNEL_ID,
                                                 tunnel_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Overlay IPv4 network route */
    status = saiL3Test::sai_test_route_create (dflt_overlay_vr_id, ip4_af,
                                               overlay_ip4_route,
                                               overlay_ip4_route_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               encap_nh_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Set the underlay neighbor DestMac attribute */
    status = saiL3Test::sai_test_neighbor_fdb_entry_create (mac_str,
                                                            test_vlan_obj_id, test_bridge_port_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = saiL3Test::sai_test_neighbor_attr_set (dflt_vlan_rif_id, ip4_af,
                                                    nh_ip_str_2,
                                                    SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS,
                                                    0, mac_str);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Set the underlay neighbor FDB entry's port attribute */
    status = saiL3Test::sai_test_neighbor_fdb_entry_port_set (mac_str,
                                                              test_vlan_obj_id, new_bridge_port_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the Overlay route */
    status = saiL3Test::sai_test_route_remove (dflt_overlay_vr_id, ip4_af,
                                               overlay_ip4_route,
                                               overlay_ip4_route_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel encap next hop */
    status = saiL3Test::sai_test_nexthop_remove (encap_nh_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the underlay route */
    status = saiL3Test::sai_test_route_remove (dflt_vr_id, ip4_af,
                                               prefix_str, prefix_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel object */
    status = sai_test_tunnel_remove (tunnel_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates IP Tunnel Encap Next Hop object creation for a directly
 * connected Neighbor IP address. Verifies creation of overlay routes
 * for the directly connected Encap Next Hop.
 */
TEST_F (saiTunnelTest, create_and_remove_tunnel_encap_next_hop_for_neighbor)
{
    sai_status_t          status;
    sai_object_id_t       tunnel_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t       encap_nh_id = SAI_NULL_OBJECT_ID;
    const char           *tunnel_sip = "10.0.0.1";
    const char           *ip_str = "11.0.0.1";
    const unsigned int    route_attr_count = 1;
    sai_ip_addr_family_t  ip4_af = SAI_IP_ADDR_FAMILY_IPV4;
    sai_attribute_t       attr_list [route_attr_count];
    const char           *overlay_ip4_route = "201.1.1.0";
    const unsigned int    overlay_ip4_route_len = 24;

    status = sai_test_tunnel_create (&tunnel_id, dflt_tunnel_obj_attr_count,
                                     SAI_TUNNEL_ATTR_TYPE,
                                     SAI_TUNNEL_TYPE_IPINIP,
                                     SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE,
                                     dflt_port_rif_id,
                                     SAI_TUNNEL_ATTR_OVERLAY_INTERFACE,
                                     dflt_overlay_rif_id,
                                     SAI_TUNNEL_ATTR_ENCAP_SRC_IP,
                                     SAI_IP_ADDR_FAMILY_IPV4, tunnel_sip);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create the tunnel encap next hop object for Neighbor IP address */
    status = saiL3Test::sai_test_nexthop_create (&encap_nh_id,
                                                 dflt_tunnel_encap_nh_attr_count,
                                                 SAI_NEXT_HOP_ATTR_TYPE,
                                                 SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP,
                                                 SAI_NEXT_HOP_ATTR_IP,
                                                 ip4_af, ip_str,
                                                 SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
                                                 dflt_vlan_rif_id,
                                                 SAI_NEXT_HOP_ATTR_TUNNEL_ID,
                                                 tunnel_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Overlay IPv4 network route */
    status = saiL3Test::sai_test_route_create (dflt_overlay_vr_id, ip4_af,
                                               overlay_ip4_route,
                                               overlay_ip4_route_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               encap_nh_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Overlay route */
    status = saiL3Test::sai_test_route_attr_get (dflt_overlay_vr_id, ip4_af,
                                                 overlay_ip4_route,
                                                 overlay_ip4_route_len,
                                                 attr_list, route_attr_count,
                                                 SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    EXPECT_EQ (encap_nh_id, attr_list[0].value.oid);

    /* Create the underlay neighbor */
    status = saiL3Test::sai_test_neighbor_create (dflt_vlan_rif_id,
                                                  SAI_IP_ADDR_FAMILY_IPV4,
                                                  ip_str,
                                                  saiL3Test::default_neighbor_attr_count,
                                                  SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS,
                                                  "00:44:44:44:44:44");

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the underlay neighbor */
    status = saiL3Test::sai_test_neighbor_remove (dflt_vlan_rif_id, ip4_af,
                                                  ip_str);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the Overlay route */
    status = saiL3Test::sai_test_route_remove (dflt_overlay_vr_id, ip4_af,
                                               overlay_ip4_route,
                                               overlay_ip4_route_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel encap next hop */
    status = saiL3Test::sai_test_nexthop_remove (encap_nh_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel object */
    status = sai_test_tunnel_remove (tunnel_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates Encap Next Hop object creation when the underlay route does not
 * exist. Create the underlay route later and verify the Encap Next Hop and
 * its overlay routes are updated.
 */
TEST_F (saiTunnelTest, tunnel_encap_next_hop_without_underlay_route)
{
    sai_status_t          status;
    sai_object_id_t       tunnel_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t       encap_nh_id = SAI_NULL_OBJECT_ID;
    const char           *tunnel_sip = "100.1.1.1";
    const char           *tunnel_dip = "200.1.1.1";
    const char           *prefix_str = "200.1.0.0";
    const unsigned int    prefix_len = 16;
    const char           *best_prefix_str = "200.1.1.0";
    const unsigned int    best_prefix_len = 24;
    const unsigned int    route_attr_count = 1;
    sai_ip_addr_family_t  ip4_af = SAI_IP_ADDR_FAMILY_IPV4;
    const char           *overlay_ip4_route = "201.1.1.0";
    const unsigned int    overlay_ip4_route_len = 24;

    status = sai_test_tunnel_create (&tunnel_id, dflt_tunnel_obj_attr_count,
                                     SAI_TUNNEL_ATTR_TYPE,
                                     SAI_TUNNEL_TYPE_IPINIP,
                                     SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE,
                                     dflt_port_rif_id,
                                     SAI_TUNNEL_ATTR_OVERLAY_INTERFACE,
                                     dflt_overlay_rif_id,
                                     SAI_TUNNEL_ATTR_ENCAP_SRC_IP,
                                     SAI_IP_ADDR_FAMILY_IPV4, tunnel_sip);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create the tunnel encap next hop object */
    status = saiL3Test::sai_test_nexthop_create (&encap_nh_id,
                                                 dflt_tunnel_encap_nh_attr_count,
                                                 SAI_NEXT_HOP_ATTR_TYPE,
                                                 SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP,
                                                 SAI_NEXT_HOP_ATTR_IP,
                                                 ip4_af, tunnel_dip,
                                                 SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
                                                 dflt_port_rif_id,
                                                 SAI_NEXT_HOP_ATTR_TUNNEL_ID,
                                                 tunnel_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Overlay IPv4 network route */
    status = saiL3Test::sai_test_route_create (dflt_overlay_vr_id, ip4_af,
                                               overlay_ip4_route,
                                               overlay_ip4_route_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               encap_nh_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a default underlay route */
    status = saiL3Test::sai_test_route_create (dflt_vr_id, ip4_af,
                                               "0.0.0.0", 0,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               dflt_underlay_vlan_nh_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Underlay route for the tunnel DIP */
    status = saiL3Test::sai_test_route_create (dflt_vr_id, ip4_af,
                                               prefix_str, prefix_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               dflt_underlay_port_nh_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create another best matching Underlay route for the tunnel DIP */
    status = saiL3Test::sai_test_route_create (dflt_vr_id, ip4_af,
                                               best_prefix_str, best_prefix_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               dflt_underlay_port_nh_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the underlay route */
    status = saiL3Test::sai_test_route_remove (dflt_vr_id, ip4_af,
                                               "0.0.0.0", 0);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = saiL3Test::sai_test_route_remove (dflt_vr_id, ip4_af,
                                               prefix_str, prefix_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = saiL3Test::sai_test_route_remove (dflt_vr_id, ip4_af,
                                               best_prefix_str, best_prefix_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the Overlay route */
    status = saiL3Test::sai_test_route_remove (dflt_overlay_vr_id, ip4_af,
                                               overlay_ip4_route,
                                               overlay_ip4_route_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel encap next hop */
    status = saiL3Test::sai_test_nexthop_remove (encap_nh_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel object */
    status = sai_test_tunnel_remove (tunnel_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates Encap Next Hop object and its dependent overlay routes are updated
 * when the underlay route Next hop Id is changed.
 */
TEST_F (saiTunnelTest, tunnel_encap_next_hop_underlay_route_nh_id_change)
{
    sai_status_t          status;
    sai_object_id_t       tunnel_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t       encap_nh_id = SAI_NULL_OBJECT_ID;
    const char           *tunnel_sip = "100.1.1.1";
    const char           *tunnel_dip = "200.1.1.1";
    const char           *prefix_str = "200.1.0.0";
    const unsigned int    prefix_len = 16;
    const unsigned int    route_attr_count = 1;
    sai_ip_addr_family_t  ip4_af = SAI_IP_ADDR_FAMILY_IPV4;
    const unsigned int    route_count = 5;
    const char           *overlay_ip4_route [route_count] = {
    "201.1.1.0", "202.1.1.0", "203.1.1.0", "204.1.1.0", "205.1.1.0" };
    const unsigned int    overlay_ip4_route_len = 24;

    status = sai_test_tunnel_create (&tunnel_id, dflt_tunnel_obj_attr_count,
                                     SAI_TUNNEL_ATTR_TYPE,
                                     SAI_TUNNEL_TYPE_IPINIP,
                                     SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE,
                                     dflt_port_rif_id,
                                     SAI_TUNNEL_ATTR_OVERLAY_INTERFACE,
                                     dflt_overlay_rif_id,
                                     SAI_TUNNEL_ATTR_ENCAP_SRC_IP,
                                     SAI_IP_ADDR_FAMILY_IPV4, tunnel_sip);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Underlay route for the tunnel DIP */
    status = saiL3Test::sai_test_route_create (dflt_vr_id, ip4_af,
                                               prefix_str, prefix_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               dflt_underlay_port_nh_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create the tunnel encap next hop object */
    status = saiL3Test::sai_test_nexthop_create (&encap_nh_id,
                                                 dflt_tunnel_encap_nh_attr_count,
                                                 SAI_NEXT_HOP_ATTR_TYPE,
                                                 SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP,
                                                 SAI_NEXT_HOP_ATTR_IP,
                                                 ip4_af, tunnel_dip,
                                                 SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
                                                 dflt_port_rif_id,
                                                 SAI_NEXT_HOP_ATTR_TUNNEL_ID,
                                                 tunnel_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create the Overlay IPv4 network routes */
    for (unsigned int itr = 0; itr < route_count; itr++)
    {
        status = saiL3Test::sai_test_route_create (dflt_overlay_vr_id, ip4_af,
                                                   overlay_ip4_route [itr],
                                                   overlay_ip4_route_len,
                                                   route_attr_count,
                                                   SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                                   encap_nh_id);
        EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    }

    /* Set the Underlay route to FWD to NHG Id */
    status = saiL3Test::sai_test_route_attr_set (dflt_vr_id, ip4_af,
                                                 prefix_str, prefix_len,
                                                 SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                                 dflt_underlay_nhg_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Set the Underlay route to TRAP action now */
    status = saiL3Test::sai_test_route_attr_set (dflt_vr_id, ip4_af,
                                                 prefix_str, prefix_len,
                                                 SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION,
                                                 SAI_PACKET_ACTION_TRAP);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Set the Underlay route to FWD back to the old NH Id */
    status = saiL3Test::sai_test_route_attr_set (dflt_vr_id, ip4_af,
                                                 prefix_str, prefix_len,
                                                 SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                                 dflt_underlay_port_nh_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Set the Underlay route to FORWARD action back */
    status = saiL3Test::sai_test_route_attr_set (dflt_vr_id, ip4_af,
                                                 prefix_str, prefix_len,
                                                 SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION,
                                                 SAI_PACKET_ACTION_FORWARD);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the Overlay route */
    for (unsigned int itr = 0; itr < route_count; itr++)
    {
        status = saiL3Test::sai_test_route_remove (dflt_overlay_vr_id, ip4_af,
                                                   overlay_ip4_route [itr],
                                                   overlay_ip4_route_len);
        EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    }

    /* Remove the tunnel encap next hop */
    status = saiL3Test::sai_test_nexthop_remove (encap_nh_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the underlay route */
    status = saiL3Test::sai_test_route_remove (dflt_vr_id, ip4_af,
                                               prefix_str, prefix_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel object */
    status = sai_test_tunnel_remove (tunnel_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates Underlay Neighbor entry attribute changes for the IP Tunnel
 * Encap Next Hop object.
 */
TEST_F (saiTunnelTest, tunnel_encap_nexthop_underlay_neighbor_attr)
{
    sai_status_t          status;
    sai_object_id_t       tunnel_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t       encap_nh_id = SAI_NULL_OBJECT_ID;
    const char           *tunnel_sip = "100.1.1.1";
    const char           *tunnel_dip = "200.1.1.1";
    const char           *prefix_str = "200.1.0.0";
    const unsigned int    prefix_len = 16;
    const unsigned int    route_attr_count = 1;
    sai_attribute_t       attr_list [route_attr_count];
    sai_ip_addr_family_t  ip4_af = SAI_IP_ADDR_FAMILY_IPV4;
    const char           *overlay_ip4_route = "201.1.1.0";
    const unsigned int    overlay_ip4_route_len = 24;
    sai_object_id_t       test_bridge_port_id = sai_test_tunnel_bridge_port_id_get (2);
    sai_object_id_t       new_bridge_port_id = sai_test_tunnel_bridge_port_id_get (5);
    const char           *mac_str = "00:11:11:11:11:11";

    status = sai_test_tunnel_create (&tunnel_id, dflt_tunnel_obj_attr_count,
                                     SAI_TUNNEL_ATTR_TYPE,
                                     SAI_TUNNEL_TYPE_IPINIP,
                                     SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE,
                                     dflt_port_rif_id,
                                     SAI_TUNNEL_ATTR_OVERLAY_INTERFACE,
                                     dflt_overlay_rif_id,
                                     SAI_TUNNEL_ATTR_ENCAP_SRC_IP,
                                     SAI_IP_ADDR_FAMILY_IPV4, tunnel_sip);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Underlay route for the tunnel DIP */
    status = saiL3Test::sai_test_route_create (dflt_vr_id, ip4_af,
                                               prefix_str, prefix_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               dflt_underlay_vlan_nh_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create the tunnel encap next hop object */
    status = saiL3Test::sai_test_nexthop_create (&encap_nh_id,
                                                 dflt_tunnel_encap_nh_attr_count,
                                                 SAI_NEXT_HOP_ATTR_TYPE,
                                                 SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP,
                                                 SAI_NEXT_HOP_ATTR_IP,
                                                 ip4_af, tunnel_dip,
                                                 SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
                                                 dflt_vlan_rif_id,
                                                 SAI_NEXT_HOP_ATTR_TUNNEL_ID,
                                                 tunnel_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Overlay IPv4 network route */
    status = saiL3Test::sai_test_route_create (dflt_overlay_vr_id, ip4_af,
                                               overlay_ip4_route,
                                               overlay_ip4_route_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               encap_nh_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Set the underlay neighbor DestMac attribute */
    status = saiL3Test::sai_test_neighbor_fdb_entry_create (mac_str,
                                                            test_vlan_obj_id, test_bridge_port_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = saiL3Test::sai_test_neighbor_attr_set (dflt_vlan_rif_id, ip4_af,
                                                    nh_ip_str_2,
                                                    SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS,
                                                    0, mac_str);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Set the underlay neighbor FDB entry's port attribute */
    status = saiL3Test::sai_test_neighbor_fdb_entry_port_set (mac_str, test_vlan_obj_id,
                                                              new_bridge_port_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Overlay route */
    status = saiL3Test::sai_test_route_attr_get (dflt_overlay_vr_id, ip4_af,
                                                 overlay_ip4_route,
                                                 overlay_ip4_route_len,
                                                 attr_list, route_attr_count,
                                                 SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    EXPECT_EQ (encap_nh_id, attr_list[0].value.oid);

    status = saiL3Test::sai_test_route_remove (dflt_overlay_vr_id, ip4_af,
                                               overlay_ip4_route,
                                               overlay_ip4_route_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel encap next hop */
    status = saiL3Test::sai_test_nexthop_remove (encap_nh_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the underlay route */
    status = saiL3Test::sai_test_route_remove (dflt_vr_id, ip4_af,
                                               prefix_str, prefix_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel object */
    status = sai_test_tunnel_remove (tunnel_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

TEST_F (saiTunnelTest, overlay_next_hop_group_add_remove_next_hop)
{
    sai_status_t          status;
    sai_object_id_t       tunnel_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t       group_id = SAI_NULL_OBJECT_ID;
    const unsigned int    nh_count = 2;
    const unsigned int    nh_count_4 = 4;
    sai_object_id_t       encap_nh_id [nh_count];
    sai_object_id_t       nh_list [nh_count_4];
    const char           *tunnel_sip = "100.1.1.1";
    const char           *tunnel_dip_1 = "200.1.1.1";
    const char           *tunnel_dip_2 = "200.2.2.1";
    const char           *prefix_str_1 = "200.1.1.0";
    const char           *prefix_str_2 = "200.2.2.0";
    const unsigned int    prefix_len = 24;
    const unsigned int    route_attr_count = 1;
    sai_attribute_t       attr_list [route_attr_count];
    sai_ip_addr_family_t  ip4_af = SAI_IP_ADDR_FAMILY_IPV4;
    const char           *overlay_ip4_route = "201.1.1.0";
    const unsigned int    overlay_ip4_route_len = 24;

    status = sai_test_tunnel_create (&tunnel_id, dflt_tunnel_obj_attr_count,
                                     SAI_TUNNEL_ATTR_TYPE,
                                     SAI_TUNNEL_TYPE_IPINIP,
                                     SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE,
                                     dflt_port_rif_id,
                                     SAI_TUNNEL_ATTR_OVERLAY_INTERFACE,
                                     dflt_overlay_rif_id,
                                     SAI_TUNNEL_ATTR_ENCAP_SRC_IP,
                                     SAI_IP_ADDR_FAMILY_IPV4, tunnel_sip);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create Underlay route for the tunnel DIP */
    status = saiL3Test::sai_test_route_create (dflt_vr_id, ip4_af,
                                               prefix_str_1, prefix_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               dflt_underlay_port_nh_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = saiL3Test::sai_test_route_create (dflt_vr_id, ip4_af,
                                               prefix_str_2, prefix_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               dflt_underlay_vlan_nh_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create the tunnel encap next hop objects */
    status = saiL3Test::sai_test_nexthop_create (&encap_nh_id [0],
                                                 dflt_tunnel_encap_nh_attr_count,
                                                 SAI_NEXT_HOP_ATTR_TYPE,
                                                 SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP,
                                                 SAI_NEXT_HOP_ATTR_IP,
                                                 ip4_af, tunnel_dip_1,
                                                 SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
                                                 dflt_port_rif_id,
                                                 SAI_NEXT_HOP_ATTR_TUNNEL_ID,
                                                 tunnel_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = saiL3Test::sai_test_nexthop_create (&encap_nh_id [1],
                                                 dflt_tunnel_encap_nh_attr_count,
                                                 SAI_NEXT_HOP_ATTR_TYPE,
                                                 SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP,
                                                 SAI_NEXT_HOP_ATTR_IP,
                                                 ip4_af, tunnel_dip_2,
                                                 SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
                                                 dflt_vlan_rif_id,
                                                 SAI_NEXT_HOP_ATTR_TUNNEL_ID,
                                                 tunnel_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Next hop Group with the tunnel encap next hop objects */
    nh_list[0] = encap_nh_id[0];
    nh_list[1] = dflt_underlay_vlan_nh_id;
    status = saiL3Test::sai_test_nh_group_create (&group_id, nh_list,
                                                  saiL3Test::default_nh_group_attr_count,
                                                  SAI_NEXT_HOP_GROUP_ATTR_TYPE,
                                                  SAI_NEXT_HOP_GROUP_TYPE_ECMP,
                                                  SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST,
                                                  nh_count);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the Overlay Next hop group */
    status = saiL3Test::sai_test_nh_group_remove (group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Next hop Group with weighted tunnel encap next hop objects */
    nh_list[0] = encap_nh_id[0];
    nh_list[1] = encap_nh_id[0];
    nh_list[2] = encap_nh_id[1];
    nh_list[3] = encap_nh_id[1];
    status = saiL3Test::sai_test_nh_group_create (&group_id, nh_list,
                                                  saiL3Test::default_nh_group_attr_count,
                                                  SAI_NEXT_HOP_GROUP_ATTR_TYPE,
                                                  SAI_NEXT_HOP_GROUP_TYPE_ECMP,
                                                  SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST,
                                                  nh_count_4);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Overlay IPv4 network route pointing to the group */
    status = saiL3Test::sai_test_route_create (dflt_overlay_vr_id, ip4_af,
                                               overlay_ip4_route,
                                               overlay_ip4_route_len,
                                               route_attr_count,
                                               SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
                                               group_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Overlay route */
    status = saiL3Test::sai_test_route_attr_get (dflt_overlay_vr_id, ip4_af,
                                                 overlay_ip4_route,
                                                 overlay_ip4_route_len,
                                                 attr_list, route_attr_count,
                                                 SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    EXPECT_EQ (group_id, attr_list[0].value.oid);

    /* Add more weight to the next hops */
    status = saiL3Test::sai_test_add_nh_to_group (group_id, nh_count_4, nh_list);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove weighted next hops */
    nh_list[0] = encap_nh_id[0];
    nh_list[1] = encap_nh_id[1];
    status = saiL3Test::sai_test_remove_nh_from_group (group_id, nh_count, nh_list);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the Overlay route */
    status = saiL3Test::sai_test_route_remove (dflt_overlay_vr_id, ip4_af,
                                               overlay_ip4_route,
                                               overlay_ip4_route_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the Overlay Next hop group */
    status = saiL3Test::sai_test_nh_group_remove (group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel encap next hop */
    status = saiL3Test::sai_test_nexthop_remove (encap_nh_id[0]);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = saiL3Test::sai_test_nexthop_remove (encap_nh_id[1]);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the underlay route */
    status = saiL3Test::sai_test_route_remove (dflt_vr_id, ip4_af,
                                               prefix_str_1, prefix_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = saiL3Test::sai_test_route_remove (dflt_vr_id, ip4_af,
                                               prefix_str_2, prefix_len);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the tunnel object */
    status = sai_test_tunnel_remove (tunnel_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

int main (int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
