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
* @file  sai_l3_nexthopgroup_unit_test.cpp
*
* @brief This file contains the google unit test cases for testing the
*        SAI Next Hop Group functionality.
*
*************************************************************************/

#include "gtest/gtest.h"

#include "sai_l3_unit_test_utils.h"

extern "C" {
#include "sai.h"
#include "saistatus.h"
#include "saitypes.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <inttypes.h>
}


class saiL3NextHopGroupTest : public saiL3Test
{
    public:
        static void SetUpTestCase (void);
        static void TearDownTestCase (void);

        static void sai_test_setup_max_ecmp_paths (void);
        static void sai_test_setup_global_nh_list (void);
        static void sai_test_remove_global_nh_list (void);

        static void sai_nh_group_verify_after_creation (
                                         sai_object_id_t group_id,
                                         sai_next_hop_group_type_t type,
                                         const unsigned int nh_count,
                                         const sai_object_id_t *p_nh_id_list);
        static void sai_nh_group_verify_after_creation (
                                      sai_object_id_t group_id,
                                      sai_next_hop_group_type_t type,
                                      const unsigned int grp_nh_count,
                                      const unsigned int nh_count,
                                      const sai_object_id_t *p_nh_id_list,
                                      const sai_object_id_t *p_member_id_list,
                                      bool  present);
        static void sai_nh_group_verify_after_remove_nh (
                                         sai_object_id_t group_id,
                                         const unsigned int group_nh_count,
                                         const unsigned int removed_nh_count,
                                         const sai_object_id_t *p_nh_id_list);
        static void sai_nh_group_verify_after_removal (
                                         sai_object_id_t group_id);

        static void sai_nh_group_verify_nh_weight (
                                         sai_object_id_t group_id,
                                         const unsigned int nh_count,
                                         sai_object_id_t *nh_list,
                                         sai_object_id_t *member_list,
                                         sai_object_id_t nh_id);
        static sai_status_t sai_test_remove_member_ids_from_group (
                                              sai_object_id_t  group_id,
                                              unsigned int     nh_count,
                                              sai_object_id_t *member_id);

        static const unsigned int default_port = 0;
        static const unsigned int max_nh_group_attr_count = 3;

        /* Default setting for the test case */
        static const unsigned int  max_ecmp_paths = 64;

        static sai_object_id_t  port_id;
        static sai_object_id_t  vrf_id;
        static sai_object_id_t  port_rif_id;

        static unsigned int       prev_max_ecmp_paths_value;
        static sai_object_id_t   *p_nh_id_list;
        static const char        *p_nh_list_start_ip_addr;
};

sai_object_id_t saiL3NextHopGroupTest::port_id = 0;
sai_object_id_t saiL3NextHopGroupTest::vrf_id = 0;
sai_object_id_t saiL3NextHopGroupTest::port_rif_id = 0;
unsigned int saiL3NextHopGroupTest::prev_max_ecmp_paths_value = 0;
sai_object_id_t* saiL3NextHopGroupTest::p_nh_id_list = NULL;
const char* saiL3NextHopGroupTest::p_nh_list_start_ip_addr = "10.0.0.1";

void saiL3NextHopGroupTest::SetUpTestCase (void)
{
    sai_status_t      status;

    /* Base SetUpTestCase for SAI initialization */
    saiL3Test::SetUpTestCase ();

    /* SAI Router default MAC address init */
    status = sai_test_router_mac_init (router_mac);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Set the Max ECMP Paths attribute for the test case */
    sai_test_setup_max_ecmp_paths ();

    port_id = sai_l3_port_id_get (default_port);

    /* Create a Virtual Router instance */
    status = sai_test_vrf_create (&vrf_id, 0);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a Port RIF */
    status = sai_test_rif_create (&port_rif_id, default_rif_attr_count,
                                  SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID,
                                  vrf_id,
                                  SAI_ROUTER_INTERFACE_ATTR_TYPE,
                                  SAI_ROUTER_INTERFACE_TYPE_PORT,
                                  SAI_ROUTER_INTERFACE_ATTR_PORT_ID,
                                  port_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create a global Next Hop list for the test case */
    sai_test_setup_global_nh_list ();
}

void saiL3NextHopGroupTest::TearDownTestCase (void)
{
    sai_status_t    status;
    sai_attribute_t attr;

    memset (&attr, 0, sizeof (sai_attribute_t));

    /* Remove the global Next Hop list */
    sai_test_remove_global_nh_list ();

    /* Remove the Port RIF */
    status = sai_test_rif_remove (port_rif_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the VRF */
    status = sai_test_vrf_remove (vrf_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Reset the Max ECMP Paths attribute to its old value */
    attr.id        = SAI_SWITCH_ATTR_ECMP_MEMBERS;
    attr.value.u32 = prev_max_ecmp_paths_value;

    status  = saiL3Test::switch_api_tbl_get()->set_switch_attribute (switch_id,
                                               (const sai_attribute_t *)&attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Set ECMP max-paths attribute for the Next Hop Group test cases.
 */
void saiL3NextHopGroupTest::sai_test_setup_max_ecmp_paths (void)
{
    sai_status_t    status;
    sai_attribute_t attr;

    memset (&attr, 0, sizeof (sai_attribute_t));

    attr.id = SAI_SWITCH_ATTR_ECMP_MEMBERS;

    /* Retrieve and store the current Max ECMP Paths value */
    status  = saiL3Test::switch_api_tbl_get()->get_switch_attribute (switch_id,1, &attr);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    prev_max_ecmp_paths_value = attr.value.u32;

    /* Set the default Max ECMP Paths value */
    attr.id        = SAI_SWITCH_ATTR_ECMP_MEMBERS;
    attr.value.u32 = max_ecmp_paths;

    status  = saiL3Test::switch_api_tbl_get()->set_switch_attribute (switch_id,
                                               (const sai_attribute_t *)&attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Max ECMP Paths attribute */
    memset (&attr, 0, sizeof (sai_attribute_t));

    attr.id = SAI_SWITCH_ATTR_ECMP_MEMBERS;

    status  = saiL3Test::switch_api_tbl_get()->get_switch_attribute (switch_id,1, &attr);

    ASSERT_TRUE (attr.value.u32 != max_ecmp_paths);

    printf ("Setup Max ECMP Paths attribute to %d.\n", max_ecmp_paths);
}

/*
 * Create a list of Neighbors, Nexthops to be used in nexthop group test cases.
 */
void saiL3NextHopGroupTest::sai_test_setup_global_nh_list (void)
{
    sai_status_t    status;
    const char     *p_neighbor_mac = "00:a1:a2:a3:a4:00";
    unsigned int    count;
    unsigned int    addr_byte3 = 0;
    unsigned int    addr_byte4 = 0;
    char            ip_addr_str [64];

    printf ("Setting up a global Next Hop list of count %d.\n", max_ecmp_paths);

    p_nh_id_list = (sai_object_id_t *) calloc (max_ecmp_paths,
                                                 sizeof (sai_object_id_t));

    if (p_nh_id_list == NULL) {

        printf ("%s(): Failed to allocate memory for Next Hop list.\n",
                __FUNCTION__);
    }

    ASSERT_NE ((sai_object_id_t *) NULL, p_nh_id_list);

    /* Create a global Next Hop list for the Max ECMP paths */
    for (count = 0; count < max_ecmp_paths; count++)
    {
        if (addr_byte4 == 255) {
            addr_byte4 = 0;
            addr_byte3++;
        }
        addr_byte4++;
        snprintf (ip_addr_str, sizeof (ip_addr_str), "10.0.%d.%d", addr_byte3,
                  addr_byte4);

        status = sai_test_nexthop_create (&p_nh_id_list [count],
                                          default_nh_attr_count,
                                          SAI_NEXT_HOP_ATTR_TYPE,
                                          SAI_NEXT_HOP_TYPE_IP,
                                          SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
                                          port_rif_id,
                                          SAI_NEXT_HOP_ATTR_IP,
                                          SAI_IP_ADDR_FAMILY_IPV4, ip_addr_str);

        ASSERT_EQ (SAI_STATUS_SUCCESS, status);

        status = sai_test_neighbor_create (port_rif_id, SAI_IP_ADDR_FAMILY_IPV4,
                                           ip_addr_str,
                                           default_neighbor_attr_count,
                                           SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS,
                                           p_neighbor_mac);

        ASSERT_EQ (SAI_STATUS_SUCCESS, status);
    }
}

/*
 * Remove list of Neighbors, Nexthops used in nexthop group test cases.
 */
void saiL3NextHopGroupTest::sai_test_remove_global_nh_list (void)
{
    unsigned int    count;
    sai_status_t    status;
    unsigned int    addr_byte3 = 0;
    unsigned int    addr_byte4 = 0;
    char            ip_addr_str [64];

    ASSERT_NE ((sai_object_id_t *) NULL, p_nh_id_list);

    printf ("Removing the global Next Hop list.\n");

    /* Scan through the global Next Hop list */
    for (count = 0; count < max_ecmp_paths; count++)
    {
        status = sai_test_nexthop_remove (p_nh_id_list [count]);

        EXPECT_EQ (SAI_STATUS_SUCCESS, status);

        if (addr_byte4 == 255) {

            addr_byte4 = 0;
            addr_byte3++;
        }

        addr_byte4++;

        snprintf (ip_addr_str, sizeof (ip_addr_str), "10.0.%d.%d", addr_byte3,
                  addr_byte4);

        status = sai_test_neighbor_remove (port_rif_id, SAI_IP_ADDR_FAMILY_IPV4,
                                           ip_addr_str);

        EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    }

    free (p_nh_id_list);
}

/*
 * Helper function to find a nexthop in a nexthop list and also return it's
 * weight.
 */
static bool sai_test_find_nh_id_in_nh_list (sai_object_id_t nh_id,
                                            sai_object_list_t *p_nh_list,
                                            unsigned int *p_weight)
{
    bool         is_found = false;
    unsigned int weight = 0;
    unsigned int index;

    for (index = 0; index < p_nh_list->count; index++) {

        if (p_nh_list->list [index] == nh_id) {

            is_found = true;
            weight++;
        }
    }

    (*p_weight) = weight;

    return is_found;
}

/*
 * Helper function to verify the Nexthop group attributes after creation.
 */
void saiL3NextHopGroupTest::sai_nh_group_verify_after_creation (
                                          sai_object_id_t group_id,
                                          sai_next_hop_group_type_t type,
                                          const unsigned int nh_count,
                                          const sai_object_id_t *p_nh_id_list)
{
    sai_status_t      status;
    sai_attribute_t   attr_list [max_nh_group_attr_count];
    sai_object_id_t   nh_id_list [max_ecmp_paths];
    unsigned int      weight = 0;
    unsigned int      index;
    bool              is_found;

    /* Set the nh count and pointer in next hop list attribute */
    attr_list[2].value.objlist.count = nh_count;
    attr_list[2].value.objlist.list  = nh_id_list;

    status = sai_test_nh_group_attr_get (group_id, attr_list,
                                         max_nh_group_attr_count,
                                         SAI_NEXT_HOP_GROUP_ATTR_TYPE,
                                         SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT,
                                         SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    EXPECT_EQ (type, attr_list[0].value.s32);

    EXPECT_EQ (nh_count, attr_list[1].value.u32);

    EXPECT_EQ (nh_count, attr_list[2].value.objlist.count);

    for (index = 0; index < nh_count; index++)
    {
        is_found = sai_test_find_nh_id_in_nh_list (p_nh_id_list [index],
                                                   &attr_list[2].value.objlist,
                                                   &weight);

        EXPECT_EQ (is_found, true);
    }
}

void saiL3NextHopGroupTest::sai_nh_group_verify_after_creation(
                                        sai_object_id_t group_id,
                                        sai_next_hop_group_type_t type,
                                        const unsigned int grp_nh_count,
                                        const unsigned int nh_count,
                                        const sai_object_id_t *p_nh_id_list,
                                        const sai_object_id_t *p_memb_id_list,
                                        bool  present)
{
    sai_status_t      status;
    sai_attribute_t   attr_list [max_nh_group_attr_count];
    sai_object_id_t   sai_member_list [max_ecmp_paths];
    unsigned int      weight = 0;
    unsigned int      index;
    bool              is_found;

    attr_list[0].id = SAI_NEXT_HOP_GROUP_ATTR_TYPE;
    attr_list[1].id = SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT;
    attr_list[2].id = SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST;

    /* Set the nh count and pointer in next hop list attribute */
    attr_list[2].value.objlist.count = max_ecmp_paths;
    attr_list[2].value.objlist.list  = sai_member_list;

    status = p_sai_nh_grp_api_tbl->
        get_next_hop_group_attribute (group_id,
                                      max_nh_group_attr_count,
                                      attr_list);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    EXPECT_EQ (type, attr_list[0].value.s32);

    EXPECT_EQ (grp_nh_count, attr_list[1].value.u32);

    EXPECT_EQ (grp_nh_count, attr_list[2].value.objlist.count);

    for (index = 0; index < nh_count; index++) {

        is_found = sai_test_find_nh_id_in_nh_list (p_memb_id_list [index],
                                                   &attr_list[2].value.objlist,
                                                   &weight);

        EXPECT_EQ (is_found, present);
    }
}

/*
 * Helper function to verify that nexthops are removed from the nexthop group.
 */
void saiL3NextHopGroupTest::sai_nh_group_verify_after_remove_nh (
                                         sai_object_id_t group_id,
                                         const unsigned int group_nh_count,
                                         const unsigned int removed_nh_count,
                                         const sai_object_id_t *p_nh_id_list)
{
    sai_status_t      status;
    sai_attribute_t   attr_list [max_nh_group_attr_count];
    sai_object_id_t   nh_id_list [max_ecmp_paths];
    unsigned int      weight = 0;
    unsigned int      index;
    bool              is_found;

    /* Set the nh count and pointer in next hop list attribute */
    attr_list[1].value.objlist.count = group_nh_count;
    attr_list[1].value.objlist.list  = nh_id_list;

    status = sai_test_nh_group_attr_get (group_id, attr_list,
                                         (max_nh_group_attr_count - 1),
                                         SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT,
                                         SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    EXPECT_EQ (group_nh_count, attr_list[0].value.u32);

    EXPECT_EQ (group_nh_count, attr_list[1].value.objlist.count);

    for (index = 0; index < removed_nh_count; index++)
    {
        is_found = sai_test_find_nh_id_in_nh_list (p_nh_id_list [index],
                                                   &attr_list[1].value.objlist,
                                                   &weight);

        EXPECT_EQ (is_found, false);
    }
}

/*
 * Helper function to verify nexthop group remove is successful.
 */
void saiL3NextHopGroupTest::sai_nh_group_verify_after_removal (
                                             sai_object_id_t group_id)
{
    sai_status_t  status;

    status = sai_test_nh_group_remove (group_id);

    EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_ID, status);
}

/*
 * Helper function to verify the weight for a nexthop in the nexthop group.
 */
void saiL3NextHopGroupTest::sai_nh_group_verify_nh_weight (
                                             sai_object_id_t group_id,
                                             const unsigned int group_nh_count,
                                             sai_object_id_t *nh_list,
                                             sai_object_id_t *member_list,
                                             sai_object_id_t nh_id)
{
    sai_status_t      status;
    sai_attribute_t   attr_list [max_nh_group_attr_count];
    sai_object_id_t   sai_member_list [max_ecmp_paths];
    unsigned int      weight = 0;
    unsigned int      index;
    bool              is_found;

    /* Set the nh count and pointer in next hop list attribute */
    attr_list[0].id = SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST;
    attr_list[0].value.objlist.count = max_ecmp_paths;
    attr_list[0].value.objlist.list  = sai_member_list;

    status = p_sai_nh_grp_api_tbl->
        get_next_hop_group_attribute (group_id,
                                      max_nh_group_attr_count - 2,
                                      attr_list);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    for (index = 0; index < group_nh_count; index++) {
        if (nh_list [index] == nh_id) {
            is_found =
                sai_test_find_nh_id_in_nh_list (member_list [index],
                                                &attr_list[0].value.objlist,
                                                &weight);

            EXPECT_EQ (is_found, true);
        }
    }
}

sai_status_t saiL3NextHopGroupTest::
sai_test_remove_member_ids_from_group (sai_object_id_t  group_id,
                                       unsigned int     nh_count,
                                       sai_object_id_t *member_id)
{
    sai_status_t status;
    unsigned int index;

    for (index = 0; index < nh_count; index++) {
        status = p_sai_nh_grp_api_tbl->
            remove_next_hop_group_member (member_id [index]);

        if (status != SAI_STATUS_SUCCESS) {
            return status;
        }
    }

    return SAI_STATUS_SUCCESS;
}

/*
 * Validate nexthop group creation and removal.
 */
TEST_F (saiL3NextHopGroupTest, create_and_remove_ecmp_group)
{
    sai_status_t        status;
    const unsigned int  nh_count = max_ecmp_paths;
    sai_object_id_t     group_id = 0;

    status = sai_test_nh_group_create (&group_id, p_nh_id_list,
                                       default_nh_group_attr_count,
                                       SAI_NEXT_HOP_GROUP_ATTR_TYPE,
                                       SAI_NEXT_HOP_GROUP_TYPE_ECMP,
                                       SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST,
                                       nh_count);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    printf ("SAI Next Hop Group Object Id: 0x%" PRIx64 ".\n", group_id);

    sai_nh_group_verify_after_creation (group_id, SAI_NEXT_HOP_GROUP_TYPE_ECMP,
                                        nh_count, p_nh_id_list);

    status = sai_test_nh_group_remove (group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    sai_nh_group_verify_after_removal (group_id);
}

/*
 * Validate addition of a new nexthops to a nexthop group.
 */
TEST_F (saiL3NextHopGroupTest, add_nh_to_ecmp_group)
{
    sai_status_t         status;
    const unsigned int   old_nh_count = 10;
    sai_object_id_t      group_id = 0;
    const unsigned int   new_nh_count = 5;

    status = sai_test_nh_group_create (&group_id, p_nh_id_list,
                                       default_nh_group_attr_count,
                                       SAI_NEXT_HOP_GROUP_ATTR_TYPE,
                                       SAI_NEXT_HOP_GROUP_TYPE_ECMP,
                                       SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST,
                                       old_nh_count);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    printf ("SAI Next Hop Group Object Id: 0x%" PRIx64 ".\n", group_id);

    /* Add a set of new next hops from the global list into group */
    status = sai_test_add_nh_to_group (group_id, new_nh_count,
                                       &p_nh_id_list [old_nh_count]);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    printf ("Added NH Count: %d to Next Hop Group Id: 0x%" PRIx64 ".\n",
            new_nh_count, group_id);

    /* Verify the entire count of next hops are added in the group */
    sai_nh_group_verify_after_creation (group_id, SAI_NEXT_HOP_GROUP_TYPE_ECMP,
                                        (old_nh_count + new_nh_count),
                                        p_nh_id_list);

    status = sai_test_nh_group_remove (group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    sai_nh_group_verify_after_removal (group_id);
}

/*
 * Validate deletion of nexthops from a nexthop group.
 */
TEST_F (saiL3NextHopGroupTest, remove_nh_from_ecmp_group)
{
    sai_status_t         status;
    sai_object_id_t      group_id = 0;
    const unsigned int   old_nh_count = 20;
    const unsigned int   new_nh_count = 15;
    sai_object_id_t     *p_new_nh_id_list = NULL;
    unsigned int         group_nh_count = 0;

    status = sai_test_nh_group_create (&group_id, p_nh_id_list,
                                       default_nh_group_attr_count,
                                       SAI_NEXT_HOP_GROUP_ATTR_TYPE,
                                       SAI_NEXT_HOP_GROUP_TYPE_ECMP,
                                       SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST,
                                       old_nh_count);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    printf ("SAI Next Hop Group Object Id: 0x%" PRIx64 ".\n", group_id);

    /* Add a set of new next hops from the global list into group */
    p_new_nh_id_list = &p_nh_id_list [old_nh_count];

    status = sai_test_add_nh_to_group (group_id, new_nh_count,
                                       p_new_nh_id_list);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    printf ("Added NH Count: %d to Next Hop Group Id: 0x%" PRIx64 ".\n",
            new_nh_count, group_id);

    /* Verify the entire count of next hops are added in the group */
    sai_nh_group_verify_after_creation (group_id, SAI_NEXT_HOP_GROUP_TYPE_ECMP,
                                        (old_nh_count + new_nh_count),
                                        p_nh_id_list);

    group_nh_count = old_nh_count + new_nh_count;

    /* Remove the set of new next hops from group */
    status = sai_test_remove_nh_from_group (group_id, new_nh_count,
                                            p_new_nh_id_list);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    printf ("Removed NH Count: %d from Next Hop Group Id: 0x%" PRIx64 ".\n",
            new_nh_count, group_id);

    /* Verify the new next hops are removed from the group */
    sai_nh_group_verify_after_remove_nh (group_id,
                                         (group_nh_count - new_nh_count),
                                         new_nh_count, p_new_nh_id_list);

    status = sai_test_nh_group_remove (group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    sai_nh_group_verify_after_removal (group_id);
}

/*
 * Check if nexthop group creation with invalid value set for
 * SAI_NEXT_HOP_GROUP_ATTR_TYPE attribute returns appropriate error status.
 */
TEST_F (saiL3NextHopGroupTest, invalid_nh_group_type_attr_value)
{
    sai_status_t         status;
    const unsigned int   nh_count = 10;
    sai_object_id_t      group_id = 0;
    unsigned int         invalid_attr_index = 0;
    sai_int32_t          invalid_group_type = -1;

    /* Pass invalid NH Group type attr value */
    status = sai_test_nh_group_create (&group_id, p_nh_id_list,
                                       default_nh_group_attr_count,
                                       SAI_NEXT_HOP_GROUP_ATTR_TYPE,
                                       invalid_group_type,
                                       SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST,
                                       nh_count);

    EXPECT_EQ ((sai_test_invalid_attr_status_code (SAI_STATUS_INVALID_ATTR_VALUE_0,
                invalid_attr_index)), status);
}

/*
 * Check if nexthop group creation with invalid Nexthop ID value returns
 * appropriate error status.
 */
TEST_F (saiL3NextHopGroupTest, invalid_nh_list_attr_value)
{
    sai_status_t         status;
    const unsigned int   nh_count = 1;
    sai_object_id_t      group_id = 0;
    sai_object_id_t      invalid_nh_id = 0;

    /* Pass invalid NH Id attr value */
    status = sai_test_nh_group_create (&group_id, &invalid_nh_id,
                                       default_nh_group_attr_count,
                                       SAI_NEXT_HOP_GROUP_ATTR_TYPE,
                                       SAI_NEXT_HOP_GROUP_TYPE_ECMP,
                                       SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST,
                                       nh_count);

    EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_TYPE, status);
}

/*
 * Check if nexthop group creation with mandatory attribute missing returns
 * appropriate error status.
 */
TEST_F (saiL3NextHopGroupTest, mandatory_attr_missing)
{
    sai_status_t              status;
    sai_object_id_t           group_id = 0;
    sai_object_id_t           member_id = 0;
    static const unsigned int num_nh_member_attr = 3;
    sai_attribute_t           attr [num_nh_member_attr];
    uint32_t                  index;

    /* Creating NH Member object with missing mandatory attributes */
    status = sai_test_nh_group_create_no_nh_list (&group_id,
                                                  SAI_NEXT_HOP_GROUP_TYPE_ECMP);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Missing Group Id */
    memset (attr, 0, sizeof (attr));
    index = 0;

    attr[index].id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_GROUP_ID;
    attr[index].value.oid = group_id;
    index++;

    status = p_sai_nh_grp_api_tbl->
        create_next_hop_group_member (&member_id, saiL3Test::switch_id, index, attr);

    EXPECT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING, status);

    /* Missing NH Id */
    memset (attr, 0, sizeof (attr));
    index = 0;

    attr[index].id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID;
    attr[index].value.oid = p_nh_id_list [0];
    index++;

    status = p_sai_nh_grp_api_tbl->
        create_next_hop_group_member (&member_id, saiL3Test::switch_id, index, attr);

    EXPECT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING, status);
}

/*
 * Check if nexthop group creation with invalid attribute ID returns
 * appropriate error status.
 */
TEST_F (saiL3NextHopGroupTest, invalid_attr_id)
{
    sai_status_t              status;
    sai_object_id_t           group_id = 0;
    sai_object_id_t           member_id = 0;
    static const unsigned int num_nh_member_attr = 3;
    sai_attribute_t           attr [num_nh_member_attr];
    uint32_t                  index;
    const unsigned int        invalid_grp_attr_pos = 1;
    const unsigned int        invalid_nh_attr_pos = 2;
    const unsigned int        invalid_attr_id = 0xffff;

    /* Pass invalid attribtue id in list */
    memset (&attr, 0, sizeof (attr));
    index = 0;

    attr [index].id = SAI_NEXT_HOP_GROUP_ATTR_TYPE;
    attr [index].value.s32 = SAI_NEXT_HOP_GROUP_TYPE_ECMP;
    index++;

    attr [index].id = invalid_attr_id;
    index++;

    status = p_sai_nh_grp_api_tbl->
        create_next_hop_group (&group_id, switch_id, index, attr);

    EXPECT_EQ ((sai_test_invalid_attr_status_code(SAI_STATUS_UNKNOWN_ATTRIBUTE_0,
                invalid_grp_attr_pos)), status);

    /* Creating NH Member object with missing mandatory attributes */
    status = sai_test_nh_group_create_no_nh_list (&group_id,
                                                  SAI_NEXT_HOP_GROUP_TYPE_ECMP);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Pass invalid attribtue id in list */
    memset (&attr, 0, sizeof (attr));
    index = 0;

    attr [index].id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_GROUP_ID;
    attr [index].value.oid = group_id;
    index++;

    attr [index].id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID;
    attr [index].value.oid = p_nh_id_list [0];
    index++;

    attr [index].id = invalid_attr_id;
    index++;

    status = p_sai_nh_grp_api_tbl->
        create_next_hop_group_member (&member_id, saiL3Test::switch_id, index, attr);

    EXPECT_EQ ((sai_test_invalid_attr_status_code(SAI_STATUS_UNKNOWN_ATTRIBUTE_0,
                invalid_nh_attr_pos)), status);

}

/*
 * Check if addition of an invalid Nexthop ID to the nexthop group returns
 * appropriate error status.
 */
TEST_F (saiL3NextHopGroupTest, invalid_param_in_add_nh_to_group)
{
    sai_status_t             status;
    const unsigned int       old_nh_count = 10;
    sai_object_id_t          group_id = 0;
    const unsigned int       new_nh_count = 1;
    sai_object_id_t          invalid_nh_id = 0;

    status = sai_test_nh_group_create (&group_id, p_nh_id_list,
                                       default_nh_group_attr_count,
                                       SAI_NEXT_HOP_GROUP_ATTR_TYPE,
                                       SAI_NEXT_HOP_GROUP_TYPE_ECMP,
                                       SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST,
                                       old_nh_count);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    printf ("SAI Next Hop Group Object Id: 0x%" PRIx64 ".\n", group_id);

    /* Pass invalid NH Id */
    status = sai_test_add_nh_to_group (group_id, new_nh_count,
                                       &invalid_nh_id);

    EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_TYPE, status);

    status = sai_test_nh_group_remove (group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Check if removal of an invalid Nexthop ID from the nexthop group returns
 * appropriate error status.
 */
TEST_F (saiL3NextHopGroupTest, invalid_param_in_remove_nh_from_group)
{
    sai_status_t         status;
    const unsigned int   old_nh_count = 10;
    sai_object_id_t      group_id = 0;
    sai_object_id_t      invalid_member_id = 0;

    status = sai_test_nh_group_create (&group_id, p_nh_id_list,
                                       default_nh_group_attr_count,
                                       SAI_NEXT_HOP_GROUP_ATTR_TYPE,
                                       SAI_NEXT_HOP_GROUP_TYPE_ECMP,
                                       SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST,
                                       old_nh_count);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    printf ("SAI Next Hop Group Object Id: 0x%" PRIx64 ".\n", group_id);

    /* Pass invalid Member Id */
    status = p_sai_nh_grp_api_tbl->
        remove_next_hop_group_member (invalid_member_id);

    EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_TYPE, status);

    status = sai_test_nh_group_remove (group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Check if nexthop group removal API called with invalid nexthop group ID
 * returns appropriate error status.
 */
TEST_F (saiL3NextHopGroupTest, invalid_group_id)
{
    sai_status_t        status;
    const unsigned int  nh_count = 10;
    sai_object_id_t     invalid_group_id = 0xffff;

    /* Pass invalid NH Group Id in remove */
    status = sai_test_nh_group_remove (invalid_group_id);

    EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_TYPE, status);

    /* Pass invalid NH Group Id in add NH to group */
    status = sai_test_add_nh_to_group (invalid_group_id, nh_count, p_nh_id_list);

    EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_TYPE, status);

    /* Pass invalid NH Group Id in add NH to group */
    status = sai_test_add_nh_to_group (invalid_group_id, nh_count, p_nh_id_list);

    EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_TYPE, status);
}

/*
 * Check if adding Nexthops to a nexthop group more than the max ecmp paths
 * configured (SAI_SWITCH_ATTR_ECMP_MEMBERS) returns appropriate error status.
 */
TEST_F (saiL3NextHopGroupTest, exceed_max_ecmp_paths)
{
    sai_status_t     status;
    sai_object_id_t  group_id = 0;

    /* Create NH Group with nh count more than max paths and verify it fails */
    status = sai_test_nh_group_create_no_nh_list (&group_id,
                                                  SAI_NEXT_HOP_GROUP_TYPE_ECMP);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_add_nh_to_group (group_id,
                                       max_ecmp_paths + 1, p_nh_id_list);
    EXPECT_NE (SAI_STATUS_SUCCESS, status);

    /* Add NH to Group whose nh count is equal to max paths and
     * verify it fails */
    status = sai_test_add_nh_to_group (group_id, max_ecmp_paths, p_nh_id_list);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    printf ("SAI Next Hop Group Object Id: 0x%" PRIx64 ".\n", group_id);

    status = sai_test_add_nh_to_group (group_id, 1, p_nh_id_list);
    EXPECT_NE (SAI_STATUS_SUCCESS, status);

    status = sai_test_nh_group_remove (group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validate the nexthop group creation and removal for weighted ECMP.
 */
TEST_F (saiL3NextHopGroupTest, create_and_remove_with_weighted_nh)
{
    sai_status_t               status;
    sai_object_id_t            group_id = 0;
    const unsigned int         nh1_weight = 8;
    const unsigned int         nh2_weight = 4;
    const unsigned int         nh3_weight = 2;
    const sai_object_id_t      nh1_id = p_nh_id_list [0];
    const sai_object_id_t      nh2_id = p_nh_id_list [1];
    const sai_object_id_t      nh3_id = p_nh_id_list [2];
    sai_object_id_t            wt_nh_id_list [max_ecmp_paths] = {0};
    sai_object_id_t            member_list [max_ecmp_paths] = {0};
    unsigned int               nh_count = 0;
    unsigned int               wt_index;
    sai_next_hop_group_type_t  group_type = SAI_NEXT_HOP_GROUP_TYPE_ECMP;

    /* Copy the next hop id to introduce weights */
    for (wt_index = 0; wt_index < nh1_weight; wt_index++, nh_count++) {
        wt_nh_id_list [nh_count] = nh1_id;
    }

    for (wt_index = 0; wt_index < nh2_weight; wt_index++, nh_count++) {
        wt_nh_id_list [nh_count] = nh2_id;
    }

    for (wt_index = 0; wt_index < nh3_weight; wt_index++, nh_count++) {
        wt_nh_id_list [nh_count] = nh3_id;
    }

    status = sai_test_nh_group_create_no_nh_list (&group_id, group_type);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_add_nh_to_group (group_id, nh_count,
                                       wt_nh_id_list, member_list);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    printf ("SAI Next Hop Group Object Id: 0x%" PRIx64 ".\n", group_id);

    sai_nh_group_verify_after_creation (group_id, group_type, nh_count,
                                        nh_count, wt_nh_id_list,
                                        member_list, true);

    /* Verify the weights */
    sai_nh_group_verify_nh_weight (group_id, nh_count, wt_nh_id_list,
                                   member_list, nh1_id);

    sai_nh_group_verify_nh_weight (group_id, nh_count, wt_nh_id_list,
                                   member_list, nh2_id);

    sai_nh_group_verify_nh_weight (group_id, nh_count, wt_nh_id_list,
                                   member_list, nh3_id);

    status = sai_test_remove_member_ids_from_group (group_id,
                                                    nh_count, member_list);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = p_sai_nh_grp_api_tbl->remove_next_hop_group (group_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify if nexthop group remove is successful. */
    status = p_sai_nh_grp_api_tbl->remove_next_hop_group (group_id);
    EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_ID, status);
}

/*
 * Validate adding weighted nexthops to the nexthop group for weighted ECMP.
 */
TEST_F (saiL3NextHopGroupTest, add_weighted_nh_to_group)
{
    sai_status_t               status;
    const unsigned int         nh_count = 10;
    sai_object_id_t            group_id = 0;
    const unsigned int         nh1_weight = 10;
    const unsigned int         nh2_weight = 4;
    const sai_object_id_t      nh1_id = p_nh_id_list [5];
    const sai_object_id_t      nh2_id = p_nh_id_list [7];
    sai_object_id_t            wt_nh_id_list [max_ecmp_paths]= {0};
    sai_object_id_t            member_list [max_ecmp_paths] = {0};
    unsigned int               new_nh_count = 0;
    unsigned int               wt_index;
    unsigned int               index;
    sai_next_hop_group_type_t  group_type = SAI_NEXT_HOP_GROUP_TYPE_ECMP;

    for (index = 0; index < nh_count; index++) {
        wt_nh_id_list [index] = p_nh_id_list [index];
    }

    status = sai_test_nh_group_create_no_nh_list (&group_id, group_type);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);
    printf ("SAI Next Hop Group Object Id: 0x%" PRIx64 ".\n", group_id);

    status = sai_test_add_nh_to_group (group_id, nh_count,
                                       wt_nh_id_list, member_list);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Increase weight for the next hops */
    for (wt_index = 0; wt_index < nh1_weight; wt_index++, index++) {
        wt_nh_id_list [index] = nh1_id;
    }

    for (wt_index = 0; wt_index < nh2_weight; wt_index++, index++) {
        wt_nh_id_list [index] = nh2_id;
    }

    new_nh_count = nh1_weight + nh2_weight;

    /* Add the new list to group to increase weight for the next hops */
    status = sai_test_add_nh_to_group (group_id, new_nh_count,
                                       &wt_nh_id_list [nh_count],
                                       &member_list [nh_count]);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    printf ("Added NH Count: %d to Next Hop Group Id: 0x%" PRIx64 ".\n",
            new_nh_count, group_id);

    /* Verify the next hop weight is updated from 1 to the given weight in
     * the NH Group */
    sai_nh_group_verify_nh_weight (group_id, (nh_count + new_nh_count),
                                   wt_nh_id_list, member_list, nh1_id);

    sai_nh_group_verify_nh_weight (group_id, (nh_count + new_nh_count),
                                   wt_nh_id_list, member_list, nh2_id);

    status = sai_test_remove_member_ids_from_group (group_id,
                                                    nh_count + new_nh_count,
                                                    member_list);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);


    status = p_sai_nh_grp_api_tbl->remove_next_hop_group (group_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify if nexthop group remove is successful. */
    status = p_sai_nh_grp_api_tbl->remove_next_hop_group (group_id);
    EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_ID, status);
}

/*
 * Validate removing weighted nexthops from the nexthop group for weighted ECMP.
 */
TEST_F (saiL3NextHopGroupTest, remove_weighted_nh_from_group)
{
    sai_status_t               status;
    sai_object_id_t            group_id = 0;
    const unsigned int         nh1_weight = 12;
    const unsigned int         nh2_weight = 8;
    const unsigned int         nh3_weight = 4;
    const sai_object_id_t      nh1_id = p_nh_id_list [0];
    const sai_object_id_t      nh2_id = p_nh_id_list [1];
    const sai_object_id_t      nh3_id = p_nh_id_list [2];
    sai_object_id_t            wt_nh_id_list [max_ecmp_paths]= {0};
    sai_object_id_t            member_list [max_ecmp_paths] = {0};
    sai_object_id_t            tmp_member_list [max_ecmp_paths] = {0};
    sai_object_id_t            nh1_members [max_ecmp_paths] = {0};
    sai_object_id_t            nh2_members [max_ecmp_paths] = {0};
    sai_object_id_t            nh3_members [max_ecmp_paths] = {0};
    unsigned int               nh_count = 0;
    unsigned int               nh1_id_start = 0;
    unsigned int               nh2_id_start = 0;
    unsigned int               nh3_id_start = 0;
    unsigned int               wt_index;
    unsigned int               index;
    unsigned int               tmp_index;
    sai_object_id_t            remove_nh_id_list [max_ecmp_paths] = {0};
    unsigned int               remove_nh_count = 0;
    const unsigned int         remove_weight = 4;
    sai_next_hop_group_type_t  group_type = SAI_NEXT_HOP_GROUP_TYPE_ECMP;

    /* Copy the next hop id to introduce weights */
    nh1_id_start = 0;
    for (wt_index = 0; wt_index < nh1_weight; wt_index++, nh_count++) {
        wt_nh_id_list [nh_count] = nh1_id;
    }

    nh2_id_start = nh_count;
    for (wt_index = 0; wt_index < nh2_weight; wt_index++, nh_count++) {
        wt_nh_id_list [nh_count] = nh2_id;
    }

    nh3_id_start = nh_count;
    for (wt_index = 0; wt_index < nh3_weight; wt_index++, nh_count++) {
        wt_nh_id_list [nh_count] = nh3_id;
    }

    /* Create a Group with weighted next hops */
    status = sai_test_nh_group_create_no_nh_list (&group_id, group_type);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);
    printf ("SAI Next Hop Group Object Id: 0x%" PRIx64 ".\n", group_id);

    status = sai_test_add_nh_to_group (group_id, nh_count,
                                       wt_nh_id_list, member_list);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    sai_nh_group_verify_after_creation (group_id, group_type, nh_count,
                                        nh_count, wt_nh_id_list,
                                        member_list, true);

    for (index = 0; index < nh1_weight; index++) {
        nh1_members [index] = member_list [nh1_id_start + index];
    }

    for (index = 0; index < nh2_weight; index++) {
        nh2_members [index] = member_list [nh2_id_start + index];
    }

    for (index = 0; index < nh3_weight; index++) {
        nh3_members [index] = member_list [nh3_id_start + index];
    }

    /* Remove the NH1 fully from group */
    for (wt_index = 0; wt_index < nh1_weight; wt_index++, remove_nh_count++) {
        remove_nh_id_list [remove_nh_count] = nh1_members [wt_index];
        nh1_members [wt_index] = 0;
    }

    /* Reduce weight for NH2 by four times */
    for (wt_index = 0; wt_index < remove_weight; wt_index++, remove_nh_count++) {
        remove_nh_id_list [remove_nh_count] = nh2_members [wt_index];
        nh2_members [wt_index] = 0;
    }

    /* Remove the list of NHs from group */
    status = sai_test_remove_member_ids_from_group (group_id, remove_nh_count,
                                                    remove_nh_id_list);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    printf ("Removed NH Count: %d from Next Hop Group Id: 0x%" PRIx64 ".\n",
            remove_nh_count, group_id);

    /* Verify NH3 weight is same as before */
    for (index = 0; index < nh3_weight; index++) {
        wt_nh_id_list [index] = nh3_id;
        tmp_member_list [index] = nh3_members [index];
    }
    sai_nh_group_verify_nh_weight (group_id, index,
                                   wt_nh_id_list, tmp_member_list, nh3_id);

    /* Verify NH2 weight is reduced by the given weight in the NH Group */
    for (index = 0, tmp_index = 0; index < nh2_weight; index++) {
        if (nh2_members [index] != 0) {
            wt_nh_id_list [tmp_index] = nh2_id;
            tmp_member_list [tmp_index] = nh2_members [index];
            tmp_index++;
        }
    }
    sai_nh_group_verify_nh_weight (group_id, tmp_index,
                                   wt_nh_id_list, tmp_member_list, nh2_id);

    /* Verify NH1 is removed from the NH Group fully */
    for (index = nh1_id_start; index < nh1_weight; index++) {
        wt_nh_id_list [index] = nh3_id;
    }
    sai_nh_group_verify_after_creation (group_id, group_type,
                                        nh_count - remove_nh_count,
                                        index, wt_nh_id_list,
                                        nh1_members, false);

    /* Get the Valid members of NH2 and NH3 to remove them from the Group. */
    for (index = 0, tmp_index = 0; index < nh2_weight; index++) {
        if (nh2_members [index] != 0) {
            member_list [tmp_index] = nh2_members [index];
            tmp_index++;
        }
    }

    for (index = 0; index < nh3_weight; index++, tmp_index++) {
        member_list [tmp_index] = nh3_members [index];
    }

    /* Remove the list of NHs from group */
    status = sai_test_remove_member_ids_from_group (group_id, tmp_index,
                                                    member_list);
    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = p_sai_nh_grp_api_tbl->remove_next_hop_group (group_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify if nexthop group remove is successful. */
    status = p_sai_nh_grp_api_tbl->remove_next_hop_group (group_id);
    EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_ID, status);
}

/*
 * Check if SAI_STATUS_BUFFER_OVERFLOW status is returned when the
 * nexthop group attribute get API is called with nexthop count input less than
 * the actual nexthop count in the nexthop group.
 */
TEST_F (saiL3NextHopGroupTest, nh_list_get_attr_buffer_overflow)
{
    sai_status_t             status;
    const unsigned int       nh_count = 10;
    sai_object_id_t          group_id = 0;
    sai_attribute_t          attr_list [max_nh_group_attr_count];
    sai_object_id_t          nh_id_list [max_ecmp_paths];

    status = sai_test_nh_group_create (&group_id, p_nh_id_list,
                                       default_nh_group_attr_count,
                                       SAI_NEXT_HOP_GROUP_ATTR_TYPE,
                                       SAI_NEXT_HOP_GROUP_TYPE_ECMP,
                                       SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST,
                                       nh_count);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    printf ("SAI Next Hop Group Object Id: 0x%" PRIx64 ".\n", group_id);

    sai_nh_group_verify_after_creation (group_id, SAI_NEXT_HOP_GROUP_TYPE_ECMP,
                                        nh_count, p_nh_id_list);

    /* Set the nh count as 0 in next hop list attribute */
    attr_list[2].value.objlist.count = 0;
    attr_list[2].value.objlist.list  = nh_id_list;

    status = sai_test_nh_group_attr_get (group_id, attr_list,
                                         max_nh_group_attr_count,
                                         SAI_NEXT_HOP_GROUP_ATTR_TYPE,
                                         SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT,
                                         SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST);

    EXPECT_EQ (SAI_STATUS_BUFFER_OVERFLOW, status);

    /* Verify the actual nh count is returned */
    EXPECT_EQ (nh_count, attr_list[2].value.objlist.count);

    /* Set the nh count to lesser value in next hop list attribute */
    attr_list[2].value.objlist.count = nh_count - 2;

    status = sai_test_nh_group_attr_get (group_id, attr_list,
                                         max_nh_group_attr_count,
                                         SAI_NEXT_HOP_GROUP_ATTR_TYPE,
                                         SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT,
                                         SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST);

    EXPECT_EQ (SAI_STATUS_BUFFER_OVERFLOW, status);

    /* Verify the actual nh count is returned */
    EXPECT_EQ (nh_count, attr_list[2].value.objlist.count);

    status = sai_test_nh_group_remove (group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    sai_nh_group_verify_after_removal (group_id);
}

int main (int argc, char **argv)
{
    ::testing::InitGoogleTest (&argc, argv);

    return RUN_ALL_TESTS ();
}
