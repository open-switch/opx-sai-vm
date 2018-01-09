/************************************************************************
 * * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
 * *
 * * This source code is confidential, proprietary, and contains trade
 * * secrets that are the sole property of Dell Inc.
 * * Copy and/or distribution of this source code or disassembly or reverse
 * * engineering of the resultant object code are strictly forbidden without
 * * the written consent of Dell Inc.
 * *
 * ************************************************************************/
 /**
 * * @file sai_acl_table_unit_test.cpp
 * *
 * * @brief This file contains the google unit test cases to test the
 * *        SAI APIs for SAI ACL Table functionality
 * *
 * *************************************************************************/

#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"

#include "sai_acl_unit_test_utils.h"
#include "sai_l3_unit_test_utils.h"

extern "C" {
#include "saistatus.h"
#include "saitypes.h"
#include <inttypes.h>
}

class saiACLTableTest : public saiACLTest {
    public:
        static void SetUpTestCase (void);

        static const sai_ip4_t src_ip_data;
        static const sai_ip4_t src_ip_mask;
        static const sai_mac_t src_mac_data;
        static const sai_mac_t src_mac_mask;

        static sai_object_id_t port_id_1;
};

const sai_ip4_t         saiACLTableTest ::src_ip_data = 0x01020304;
const sai_ip4_t         saiACLTableTest ::src_ip_mask = 0xffffffff;
const sai_mac_t         saiACLTableTest ::src_mac_data = {0x01,0x02,0x03,0x04,0x05,0x06};
const sai_mac_t         saiACLTableTest ::src_mac_mask = {0xff,0xff,0xff,0xff,0xff,0xff};
sai_object_id_t         saiACLTableTest ::port_id_1 = 0;

void saiACLTableTest ::SetUpTestCase (void)
{
    /* Base SetUpTestCase for SAI initialization */
    saiACLTest ::SetUpTestCase ();

    port_id_1 = sai_acl_port_id_get (0);
}

TEST_F(saiACLTableTest, get_acl_priority)
{
    sai_status_t          sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t      *p_attr_list_get = NULL;
    unsigned int          test_attr_count = 4;

    sai_rc = sai_test_acl_table_create_attr_list(&p_attr_list_get,
                                                 test_attr_count);

    ASSERT_TRUE (SAI_STATUS_SUCCESS == sai_rc);

    /* Switch Attributes to fetch priority range for ACL Table and Rule */
    sai_rc = sai_test_acl_table_switch_get(p_attr_list_get,
                                test_attr_count,
                                SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY,
                                SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY,
                                SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY,
                                SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* NPU Specific attributes */
    printf ("Table Minimun Priority = %d\n", p_attr_list_get[0].value.u32);
    printf ("Table Maximum Priority = %d\n", p_attr_list_get[1].value.u32);
    printf ("Rule entry Minimun Priority = %d\n", p_attr_list_get[2].value.u32);
    printf ("Rule entry Maximum Priority = %d\n", p_attr_list_get[3].value.u32);

    sai_test_acl_table_free_attr_list(p_attr_list_get);
}

TEST_F(saiACLTableTest, table_create_with_null_attr)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t          acl_table_id = 0;

    /* Create ACL Table */

    printf ("Table Creation with Null attributes\r\n");
    printf ("Expecting error - SAI_STATUS_FAILURE.\r\n");

    /* Attribute Count is 0 */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 0);
    EXPECT_EQ (SAI_STATUS_INVALID_PARAMETER, sai_rc);
}

TEST_F(saiACLTableTest, table_create_with_out_of_range_attr)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t          acl_table_id = 0;

    printf ("Table Creation with Out of Range attributes\r\n");
    printf ("Expecting error - SAI_STATUS_INVALID_ATTRIBUTE.\r\n");

    /* ACL Table Attribute not in the range */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 1,
                            SAI_ACL_TABLE_ATTR_GROUP_ID + 1);
    EXPECT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_rc);
}

TEST_F(saiACLTableTest, table_create_with_unknown_attr)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t          acl_table_id = 0;
    unsigned int             invalid_attribute = SAI_ACL_TABLE_ATTR_FIELD_START - 1;

    printf ("Table Creation with Unknown attributes\r\n");
    printf ("Expecting error - SAI_STATUS_UNKNOWN_ATTRIBUTE_0.\r\n");

    /* ACL Table Attribute within the range but not defined */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 1,
                                        invalid_attribute);
    EXPECT_EQ (SAI_STATUS_UNKNOWN_ATTRIBUTE_0, sai_rc);
}

TEST_F(saiACLTableTest, table_create_without_mandatory_attr)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t          acl_table_id = 0;

    printf ("Table Creation without Mandatory Attributes\r\n");
    printf ("Expecting error - SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING.\r\n");

    /* ACL Table Stage, Priority and Field are mandatory attributes */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 1,
                            SAI_ACL_TABLE_ATTR_ACL_STAGE, SAI_ACL_STAGE_INGRESS);
    EXPECT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING, sai_rc);

    sai_rc = sai_test_acl_table_create (&acl_table_id, 2,
                            SAI_ACL_TABLE_ATTR_ACL_STAGE, SAI_ACL_STAGE_INGRESS,
                            SAI_ACL_TABLE_ATTR_PRIORITY, 2);
    EXPECT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING, sai_rc);

    sai_rc = sai_test_acl_table_create (&acl_table_id, 2,
                            SAI_ACL_TABLE_ATTR_ACL_STAGE, SAI_ACL_STAGE_INGRESS,
                            SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT);
    EXPECT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING, sai_rc);
}

TEST_F(saiACLTableTest, table_create_with_invalid_stage_value)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t          acl_table_id = 0;

    printf ("Table Creation with Incorrect Table Stage attribute value\r\n");
    printf ("Expecting error - SAI_STATUS_INVALID_ATTR_VALUE. \r\n");

    sai_rc = sai_test_acl_table_create (&acl_table_id, 3,
                            SAI_ACL_TABLE_ATTR_ACL_STAGE,
                            SAI_ACL_STAGE_EGRESS + 1,
                            SAI_ACL_TABLE_ATTR_PRIORITY, 2,
                            SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT);
    EXPECT_EQ (SAI_STATUS_INVALID_ATTR_VALUE_0, sai_rc);
}

TEST_F(saiACLTableTest, table_create_with_dup_attr)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t          acl_table_id = 0;

    printf ("Table Creation with Duplicate Attributes\r\n ");
    printf ("Expecting error - SAI_STATUS_INVALID_ATTRIBUTE.\r\n");

    /* Duplicate Attribute ACL Table Stage */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 4,
                            SAI_ACL_TABLE_ATTR_ACL_STAGE,
                            SAI_ACL_STAGE_EGRESS,
                            SAI_ACL_TABLE_ATTR_PRIORITY, 2,
                            SAI_ACL_TABLE_ATTR_ACL_STAGE,
                            SAI_ACL_STAGE_INGRESS,
                            SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT);
    EXPECT_EQ (sai_test_invalid_attr_status_code (SAI_STATUS_INVALID_ATTRIBUTE_0, 2), sai_rc);
}

TEST_F(saiACLTableTest, table_create_with_unsupported_field_attr)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t          acl_table_id = 0;

    printf ("Table Creation with Unsupported Ingress Fields \r\n");
    printf ("Expecting error - SAI_STATUS_NOT_SUPPORTED.\r\n");

    /* Testing table create with unsupported ingress field. */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 4,
                            SAI_ACL_TABLE_ATTR_ACL_STAGE,
                            SAI_ACL_STAGE_INGRESS,
                            SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                            SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                            SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT /* Unsupported */);

    /* Handle the case where all fields are supported in the NPU. */
    if (SAI_STATUS_SUCCESS == sai_rc) {
        sai_rc = sai_test_acl_table_remove (acl_table_id);

        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    } else {
        EXPECT_EQ (SAI_STATUS_NOT_SUPPORTED, sai_rc);
    }

    printf ("Table Creation with Unsupported Egress Fields \r\n");
    printf ("Expecting error - SAI_STATUS_NOT_SUPPORTED.\r\n");

    /* Testing table create with unsupported egress field. */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 4,
                            SAI_ACL_TABLE_ATTR_ACL_STAGE,
                            SAI_ACL_STAGE_EGRESS,
                            SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                            SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                            SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS /* Unsupported */);

    /* Handle the case where all fields are supported in the NPU. */
    if (SAI_STATUS_SUCCESS == sai_rc) {
        sai_rc = sai_test_acl_table_remove (acl_table_id);

        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    } else {
        EXPECT_EQ (SAI_STATUS_NOT_SUPPORTED, sai_rc);
    }
}

TEST_F(saiACLTableTest, table_create_and_remove)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t       acl_table_id = 0;

    printf ("Table Creation with Valid Attributes \r\n");
    printf ("Expecting error - SAI_STATUS_SUCCESS.\r\n");

    /* Table Create */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 8,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                             SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
                             SAI_ACL_TABLE_ATTR_FIELD_DSCP,
                             SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        printf ("ACL Table Successfully created with ID 0x%" PRIx64 " \r\n",
                acl_table_id);
    }

    /* Table Remove */
    sai_rc = sai_test_acl_table_remove (acl_table_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        printf ("ACL Table Successfully removed for ID 0x%" PRIx64 " \r\n",
                acl_table_id);
    }
}

TEST_F(saiACLTableTest, table_remove_with_invalid_id)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t          acl_table_id = 0;

    /* Table Create */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 8,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                             SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
                             SAI_ACL_TABLE_ATTR_FIELD_DSCP,
                             SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    acl_table_id++;

    printf ("Table Deletion with Invalid Table Id \r\n");
    printf ("Expecting error - SAI_STATUS_INVALID_OBJECT_ID. \r\n");

    sai_rc = sai_test_acl_table_remove (acl_table_id);

    EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_ID, sai_rc);

    acl_table_id--;
    sai_rc = sai_test_acl_table_remove (acl_table_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

TEST_F(saiACLTableTest, table_remove_with_rule_present)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t       acl_table_id = 0;
    sai_object_id_t       acl_rule_id = 0;

    printf ("Table Creation with Valid Attributes \r\n");
    printf ("Expecting error - SAI_STATUS_SUCCESS.\r\n");

    /* Table Create */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 8,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                             SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
                             SAI_ACL_TABLE_ATTR_FIELD_DSCP,
                             SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        printf ("ACL Table Successfully created with ID 0x%" PRIx64 " \r\n",
                acl_table_id);
    }

    /* Rule Create */
    sai_rc = sai_test_acl_rule_create (&acl_rule_id, 4,
                             SAI_ACL_ENTRY_ATTR_TABLE_ID, acl_table_id,
                             SAI_ACL_ENTRY_ATTR_PRIORITY, 5,
                             SAI_ACL_ENTRY_ATTR_ADMIN_STATE, true,
                             SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT,
                             1, 10, 255);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Table Deletion with rule present */
    printf ("Table Deletion with rules attached to Table Id \r\n");
    printf ("Expecting error - SAI_STATUS_FAILURE. \r\n");

    sai_rc = sai_test_acl_table_remove (acl_table_id);

    EXPECT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);

    /* Rule Deletion */
    sai_rc = sai_test_acl_rule_remove (acl_rule_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Table Deletion */
    printf ("Table Deletion with no rules attached to Table Id \r\n");
    printf ("Expecting error - SAI_STATUS_SUCCESS. \r\n");

    sai_rc = sai_test_acl_table_remove (acl_table_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

TEST_F(saiACLTableTest, table_create_with_same_priority)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t       acl_table_id = 0;
    sai_object_id_t       acl_table_id_dup = 0;

    printf ("Table Creation with Valid Attributes \r\n");
    printf ("Expecting error - SAI_STATUS_SUCCESS.\r\n");

    /* Table Create */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 8,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                             SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
                             SAI_ACL_TABLE_ATTR_FIELD_DSCP,
                             SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        printf ("ACL Table Successfully created with ID 0x%" PRIx64 " \r\n",
                acl_table_id);
        printf ("Table Creation with Same Priority & Stage of "
                "Existing ACL Table \r\n");
        printf ("Expecting error - SAI_STATUS_INVALID_ATTR_VALUE. \r\n");

        sai_rc = sai_test_acl_table_create (&acl_table_id_dup, 5,
                            SAI_ACL_TABLE_ATTR_ACL_STAGE,
                            SAI_ACL_STAGE_INGRESS,
                            SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                            SAI_ACL_TABLE_ATTR_FIELD_SRC_IPv6,
                            SAI_ACL_TABLE_ATTR_FIELD_DST_MAC,
                            SAI_ACL_TABLE_ATTR_FIELD_DST_IP);
        EXPECT_EQ (sai_test_invalid_attr_status_code (SAI_STATUS_INVALID_ATTR_VALUE_0, 1), sai_rc);

        printf ("Table Creation with Same Priority but with different"
                "stage should be allowed \r\n");
        printf ("Expecting error - SAI_STATUS_SUCCESS.\r\n");

        sai_rc = sai_test_acl_table_create (&acl_table_id_dup, 5,
                            SAI_ACL_TABLE_ATTR_ACL_STAGE,
                            SAI_ACL_STAGE_EGRESS,
                            SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                            SAI_ACL_TABLE_ATTR_FIELD_SRC_IPv6,
                            SAI_ACL_TABLE_ATTR_FIELD_DST_MAC,
                            SAI_ACL_TABLE_ATTR_FIELD_DST_IP);
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

        sai_rc = sai_test_acl_table_remove (acl_table_id_dup);

        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }

    sai_rc = sai_test_acl_table_remove (acl_table_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

TEST_F(saiACLTableTest, table_set)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t       acl_table_id = 0;

    printf ("Table Creation with Valid Attributes \r\n");
    printf ("Expecting error - SAI_STATUS_SUCCESS.\r\n");

    /* Table Create */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 8,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                             SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
                             SAI_ACL_TABLE_ATTR_FIELD_DSCP,
                             SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        printf ("ACL Table Successfully created with ID 0x%" PRIx64 " \r\n",
                acl_table_id);
    }

    printf ("Table Set for Table Id 0x%" PRIx64 " \r\n", acl_table_id);
    printf ("Expecting error - SAI_STATUS_NOT_SUPPORTED.\r\n");

    /* Table Set */
    sai_rc = sai_test_acl_table_set (acl_table_id, 2,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 5 /* New Value */,
                             SAI_ACL_TABLE_ATTR_FIELD_DST_IP /*New Field */);

    EXPECT_EQ (SAI_STATUS_NOT_SUPPORTED, sai_rc);

    sai_rc = sai_test_acl_table_remove (acl_table_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

TEST_F(saiACLTableTest, table_get)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t       acl_table_id = 0;
    sai_attribute_t      *p_attr_list_get = NULL;
    unsigned int          test_attr_count = 4;

    sai_rc = sai_test_acl_table_create_attr_list(&p_attr_list_get,
                                                 test_attr_count);
    ASSERT_TRUE (SAI_STATUS_SUCCESS == sai_rc);

    printf ("Table Creation with Valid Attributes \r\n");
    printf ("Expecting error - SAI_STATUS_SUCCESS.\r\n");

    /* Table Create */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 8,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                             SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
                             SAI_ACL_TABLE_ATTR_FIELD_DSCP,
                             SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        printf ("ACL Table Successfully created with ID 0x%" PRIx64 " \r\n",
                acl_table_id);
    }

    printf ("Table Get for Table Id 0x%" PRIx64 " \r\n", acl_table_id);
    printf ("Expecting error - SAI_STATUS_SUCCESS.\r\n");

    /* Table Get */
    sai_rc = sai_test_acl_table_get (acl_table_id, p_attr_list_get, 2,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_TABLE_ATTR_PRIORITY);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Table Get with invalid Table Id */
    sai_rc = sai_test_acl_table_get ((acl_table_id + 1), p_attr_list_get, 1,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE);
    EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_ID, sai_rc);

    /* Table Get with out of range Table Attribute */
    sai_rc = sai_test_acl_table_get (acl_table_id, p_attr_list_get, 1,
                             (SAI_ACL_TABLE_ATTR_GROUP_ID + 1));
    EXPECT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_rc);

    /* Table Get with invalid Table Id */
    sai_rc = sai_test_acl_table_get (acl_table_id, p_attr_list_get, 1,
                             (SAI_ACL_TABLE_ATTR_FIELD_START - 1));
    EXPECT_EQ (SAI_STATUS_UNKNOWN_ATTRIBUTE_0, sai_rc);

    sai_rc = sai_test_acl_table_remove (acl_table_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_test_acl_table_free_attr_list(p_attr_list_get);
}

TEST_F(saiACLTableTest, table_with_invalid_object_type)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t          acl_table_id = 0;
    sai_object_id_t          acl_table_invalid_id = 0;
    sai_attribute_t      *p_attr_list_get = NULL;
    unsigned int          test_attr_count = 4;

    sai_rc = sai_test_acl_table_create_attr_list(&p_attr_list_get,
                                                 test_attr_count);
    ASSERT_TRUE (SAI_STATUS_SUCCESS == sai_rc);


    printf ("Table Creation with Valid Attributes \r\n");
    printf ("Expecting error - SAI_STATUS_SUCCESS.\r\n");

    /* Table Create */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 8,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                             SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
                             SAI_ACL_TABLE_ATTR_FIELD_DSCP,
                             SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    if (sai_rc == SAI_STATUS_SUCCESS) {
        printf ("ACL Table Successfully created with ID 0x%" PRIx64 " \r\n",
                acl_table_id);
    }

    printf ("Table Get for Invalid Table Id 0x%" PRIx64 " \r\n",
             acl_table_invalid_id);
    printf ("Expecting error - SAI_STATUS_INVALID_OBJECT_TYPE.\r\n");

    sai_rc = sai_test_acl_table_remove (acl_table_invalid_id);

    EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_TYPE, sai_rc);

    /* Table Get with invalid Table Id */
    sai_rc = sai_test_acl_table_get (acl_table_invalid_id, p_attr_list_get, 1,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE);
    EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_TYPE, sai_rc);

    sai_rc = sai_test_acl_table_remove (acl_table_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_test_acl_table_free_attr_list(p_attr_list_get);
}

TEST_F(saiACLTableTest, table_with_fix_size)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t          acl_table_id = 0, acl_table_id_2 = 0;
    sai_object_id_t          acl_rule_id = 0;
    sai_object_id_t          acl_rule_list[SAI_ACL_TEST_TABLE_SIZE] = {0};
    unsigned int             rule_count = 0;
    unsigned int             rule_priority = 0, rule_created = 0;
    sai_attribute_t         *p_attr_list_get = NULL;
    unsigned int             test_attr_count = 1;
    unsigned int             test_table_size = SAI_ACL_TEST_TABLE_SIZE;

    sai_rc = sai_test_acl_table_create_attr_list(&p_attr_list_get,
                                                 test_attr_count);
    ASSERT_TRUE (SAI_STATUS_SUCCESS == sai_rc);

    printf ("Table Creation with Fix Table Size \r\n");

    /* Ingress Table Create */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 9,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                             SAI_ACL_TABLE_ATTR_SIZE,
                             (SAI_ACL_TABLE_MAX_SIZE + 1),
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                             SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
                             SAI_ACL_TABLE_ATTR_FIELD_DSCP,
                             SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL);

    /*
     * Handle the case where the size specified more than max size
     * is supported in a NPU.
     */
    if (SAI_STATUS_SUCCESS == sai_rc) {
        sai_rc = sai_test_acl_table_remove (acl_table_id);

        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    } else {
        EXPECT_EQ (SAI_STATUS_INSUFFICIENT_RESOURCES, sai_rc);
    }

    /* Ingress Table Create */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 9,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                             SAI_ACL_TABLE_ATTR_SIZE,
                             test_table_size,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                             SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
                             SAI_ACL_TABLE_ATTR_FIELD_DSCP,
                             SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_table_get (acl_table_id, p_attr_list_get, 1,
                             SAI_ACL_TABLE_ATTR_SIZE);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    EXPECT_EQ (p_attr_list_get[0].value.u32, test_table_size);

    for (rule_count = 0; rule_count < (test_table_size + 1); rule_count++) {

         printf("\r\n Creating ACL Rule with increasing priority %d\r\n", rule_priority);

         sai_rc = sai_test_acl_rule_create (&acl_rule_id, 4,
                                SAI_ACL_ENTRY_ATTR_TABLE_ID, acl_table_id,
                                SAI_ACL_ENTRY_ATTR_PRIORITY, rule_priority,
                                SAI_ACL_ENTRY_ATTR_ADMIN_STATE, true,
                                SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE,
                                1, 34825, 0xffff);

         if (sai_rc == SAI_STATUS_TABLE_FULL) {
             EXPECT_EQ (rule_count, test_table_size);
             break;
         } else {
             EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
         }

         rule_priority++;
         acl_rule_list[rule_created] = acl_rule_id;
         rule_created++;
    }

    for (rule_count = 0; rule_count < rule_created; rule_count++) {

         if (acl_rule_list[rule_count] != 0) {
             sai_rc = sai_test_acl_rule_remove (acl_rule_list[rule_count]);
             EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
             acl_rule_list[rule_count] = 0;
         }
    }

    rule_created = 0;

    /* Add the rules in decreasing priority */
    for (rule_count = 0; rule_count < (test_table_size + 1); rule_count++) {

         printf("\r\n Creating ACL Rule with decreasing priority %d\r\n", rule_priority);

         sai_rc = sai_test_acl_rule_create (&acl_rule_id, 4,
                                SAI_ACL_ENTRY_ATTR_TABLE_ID, acl_table_id,
                                SAI_ACL_ENTRY_ATTR_PRIORITY, rule_priority,
                                SAI_ACL_ENTRY_ATTR_ADMIN_STATE, true,
                                SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE,
                                1, 34825, 0xffff);

         if (sai_rc == SAI_STATUS_TABLE_FULL) {
             EXPECT_EQ (rule_count, test_table_size);
             break;
         } else {
             EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
         }

         rule_priority--;
         acl_rule_list[rule_created] = acl_rule_id;
         rule_created++;
    }

    for (rule_count = 0; rule_count < rule_created; rule_count++) {

         if (acl_rule_list[rule_count] != 0) {
             sai_rc = sai_test_acl_rule_remove (acl_rule_list[rule_count]);
             EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
             acl_rule_list[rule_count] = 0;
         }
    }

    /* Test the Table deletion with Rule still present for Fix sized tables */

    sai_rc = sai_test_acl_rule_create (&acl_rule_id, 4,
                                SAI_ACL_ENTRY_ATTR_TABLE_ID, acl_table_id,
                                SAI_ACL_ENTRY_ATTR_PRIORITY, rule_priority,
                                SAI_ACL_ENTRY_ATTR_ADMIN_STATE, true,
                                SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE,
                                1, 34825, 0xffff);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_table_remove (acl_table_id);
    EXPECT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);

    sai_rc = sai_test_acl_rule_remove (acl_rule_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_table_remove (acl_table_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Compare Table creation with both fix and dynamic size tables */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 9,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                             SAI_ACL_TABLE_ATTR_SIZE,
                             (SAI_ACL_TABLE_MAX_SIZE - test_table_size),
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                             SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
                             SAI_ACL_TABLE_ATTR_FIELD_DSCP,
                             SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_table_get (acl_table_id, p_attr_list_get, 1,
                             SAI_ACL_TABLE_ATTR_SIZE);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    EXPECT_EQ (p_attr_list_get[0].value.u32,
                    SAI_ACL_TABLE_MAX_SIZE - test_table_size);

    /* Ingress Table Create with size more than allowed */
    sai_rc = sai_test_acl_table_create (&acl_table_id_2, 9,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 2,
                             SAI_ACL_TABLE_ATTR_SIZE,
                             (test_table_size + 1),
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                             SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
                             SAI_ACL_TABLE_ATTR_FIELD_DSCP,
                             SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL);
    EXPECT_EQ (SAI_STATUS_INSUFFICIENT_RESOURCES, sai_rc);

    sai_rc = sai_test_acl_table_remove (acl_table_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_test_acl_table_free_attr_list(p_attr_list_get);
}

TEST_F(saiACLTableTest, table_with_virtual_priority)
{
    sai_status_t             sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t          acl_table_id = 0;
    sai_object_id_t          acl_table_id_group_1 = 0, acl_table_id_group_2 = 0;
    sai_object_id_t          acl_table_id_group_3 = 0;
    sai_object_id_t          acl_rule_id_1 = 0, acl_rule_id_2 = 0;
    sai_object_id_t          acl_rule_id_3 = 0, acl_rule_id_4 = 0;
    sai_attribute_t         *p_attr_list_get = NULL;
    unsigned int             test_attr_count = 1;
    sai_object_id_t          parent_group_id = 0;
    sai_object_id_t          group_1_counter_id = 0;
    sai_object_id_t          group_2_counter_id = 0;

    sai_rc = sai_test_acl_table_create_attr_list(&p_attr_list_get,
                                                 test_attr_count);
    ASSERT_TRUE (SAI_STATUS_SUCCESS == sai_rc);

    /* Table Create */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 4,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_SIZE, 100,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Table Get */
    sai_rc = sai_test_acl_table_get (acl_table_id, p_attr_list_get, 1,
                             SAI_ACL_TABLE_ATTR_GROUP_ID);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    parent_group_id = p_attr_list_get[0].value.oid;

    /* Table Create with invalid group id */
    sai_rc = sai_test_acl_table_create (&acl_table_id_group_1, 4,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 2,
                             SAI_ACL_TABLE_ATTR_GROUP_ID, 0,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP);
    EXPECT_EQ (sai_test_invalid_attr_status_code (SAI_STATUS_INVALID_ATTR_VALUE_0, 2), sai_rc);

    /* Table grouping but with different stage */
    sai_rc = sai_test_acl_table_create (&acl_table_id_group_1, 4,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_EGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 2,
                             SAI_ACL_TABLE_ATTR_GROUP_ID,
                             parent_group_id,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP);
    EXPECT_EQ (sai_test_invalid_attr_status_code (SAI_STATUS_INVALID_ATTR_VALUE_0, 2), sai_rc);

    /* Table Grouping */
    sai_rc = sai_test_acl_table_create (&acl_table_id_group_1, 6,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 2,
                             SAI_ACL_TABLE_ATTR_GROUP_ID,
                             parent_group_id,
                             SAI_ACL_TABLE_ATTR_SIZE, 200,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Table Get */
    sai_rc = sai_test_acl_table_get (acl_table_id, p_attr_list_get, 1,
                             SAI_ACL_TABLE_ATTR_GROUP_ID);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (p_attr_list_get[0].value.oid, parent_group_id);

    /* Create Rule in each of the grouped tables and associate counter to verify grouping*/

    /* Create a counter and associate to rule */
    sai_rc = sai_test_acl_counter_create (&group_1_counter_id, 2,
                                          SAI_ACL_COUNTER_ATTR_TABLE_ID, acl_table_id,
                                          SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT, true);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_counter_create (&group_2_counter_id, 2,
                                          SAI_ACL_COUNTER_ATTR_TABLE_ID, acl_table_id_group_1,
                                          SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT, true);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_rule_create (&acl_rule_id_1, 5,
                                SAI_ACL_ENTRY_ATTR_TABLE_ID, acl_table_id,
                                SAI_ACL_ENTRY_ATTR_PRIORITY, 5,
                                SAI_ACL_ENTRY_ATTR_ADMIN_STATE, true,
                                SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE,
                                1, 34825, 0xffff,
                                SAI_ACL_ENTRY_ATTR_ACTION_COUNTER, 1,
                                group_1_counter_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_rule_create (&acl_rule_id_2, 5,
                                SAI_ACL_ENTRY_ATTR_TABLE_ID, acl_table_id_group_1,
                                SAI_ACL_ENTRY_ATTR_PRIORITY, 6,
                                SAI_ACL_ENTRY_ATTR_ADMIN_STATE, true,
                                SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE,
                                1, 34825, 0xffff,
                                SAI_ACL_ENTRY_ATTR_ACTION_COUNTER, 1,
                                group_2_counter_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /*
     * Since both of them share same qualifiers and are virtually grouped,
     * only the rule with higher priority should be hit
     */

    /* Create and group other tables without size */
    sai_rc = sai_test_acl_table_create (&acl_table_id_group_2, 4,
                                SAI_ACL_TABLE_ATTR_ACL_STAGE,
                                SAI_ACL_STAGE_INGRESS,
                                SAI_ACL_TABLE_ATTR_PRIORITY, 3,
                                SAI_ACL_TABLE_ATTR_GROUP_ID,
                                parent_group_id,
                                SAI_ACL_TABLE_ATTR_FIELD_SRC_IP);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_table_create (&acl_table_id_group_3, 5,
                                SAI_ACL_TABLE_ATTR_ACL_STAGE,
                                SAI_ACL_STAGE_INGRESS,
                                SAI_ACL_TABLE_ATTR_PRIORITY, 4,
                                SAI_ACL_TABLE_ATTR_GROUP_ID,
                                parent_group_id,
                                SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                                SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_rule_create (&acl_rule_id_3, 4,
                                SAI_ACL_ENTRY_ATTR_TABLE_ID, acl_table_id_group_2,
                                SAI_ACL_ENTRY_ATTR_PRIORITY, 6,
                                SAI_ACL_ENTRY_ATTR_ADMIN_STATE, true,
                                SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP,
                                1, &src_ip_data, &src_ip_mask);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_rule_create (&acl_rule_id_4, 5,
                                SAI_ACL_ENTRY_ATTR_TABLE_ID, acl_table_id_group_3,
                                SAI_ACL_ENTRY_ATTR_PRIORITY, 6,
                                SAI_ACL_ENTRY_ATTR_ADMIN_STATE, true,
                                SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP,
                                1, &src_ip_data, &src_ip_mask,
                                SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC,
                                1, &src_mac_data, &src_mac_mask);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_test_acl_table_free_attr_list(p_attr_list_get);

    sai_rc = sai_test_acl_rule_remove (acl_rule_id_1);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_rule_remove (acl_rule_id_2);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_rule_remove (acl_rule_id_3);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_rule_remove (acl_rule_id_4);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_counter_remove (group_1_counter_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_counter_remove (group_2_counter_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_table_remove (acl_table_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_table_remove (acl_table_id_group_1);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_table_remove (acl_table_id_group_2);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_table_remove (acl_table_id_group_3);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

TEST_F(saiACLTableTest, acl_group_member)
{
    unsigned int i = 0;
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t acl_rule_id = 0;
    sai_object_id_t acl_table_id = 0;
    sai_object_id_t acl_table_id1 = 0;
    sai_object_id_t acl_table_group_id = 0;
    sai_object_id_t acl_table_group_mem_id = 0;
    sai_object_id_t acl_table_group_mem_id1 = 0;
    sai_attribute_t new_attr_list[5];
    sai_attribute_t group_mem_attr[5];
    sai_attribute_t set_attr;
    sai_attribute_t get_attr;
    sai_s32_list_t  table_list;
    int32_t         *list = NULL;
    unsigned int attr_count = 0;

    /* Table Create */
    sai_rc = sai_test_acl_table_create (&acl_table_id, 8,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 1,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                             SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
                             SAI_ACL_TABLE_ATTR_FIELD_DSCP,
                             SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_table_create (&acl_table_id1, 8,
                             SAI_ACL_TABLE_ATTR_ACL_STAGE,
                             SAI_ACL_STAGE_INGRESS,
                             SAI_ACL_TABLE_ATTR_PRIORITY, 2,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
                             SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
                             SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
                             SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
                             SAI_ACL_TABLE_ATTR_FIELD_DSCP,
                             SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    new_attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    new_attr_list[attr_count].value.s32 = SAI_ACL_STAGE_INGRESS;

    attr_count ++;
    new_attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_TYPE;
    new_attr_list[attr_count].value.s32 = SAI_ACL_TABLE_GROUP_TYPE_PARALLEL;

    attr_count ++;

    table_list.count = 2;
    list = (int32_t *) calloc (table_list.count, sizeof (int32_t));
    list[0] = SAI_ACL_BIND_POINT_TYPE_PORT;
    list[1] = SAI_ACL_BIND_POINT_TYPE_SWITCH;
    table_list.list = list;

    new_attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    new_attr_list[attr_count].value.s32list = table_list;

    attr_count ++;
    ASSERT_EQ(SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->
            create_acl_table_group(&acl_table_group_id,
                saiACLTest ::sai_acl_get_global_switch_id(),
                attr_count,
                (const sai_attribute_t *)new_attr_list));

    free(list);

    attr_count = 1;
    get_attr.id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;

    table_list.count = 1;
    list = (int32_t *) calloc (table_list.count, sizeof (int32_t));
    table_list.list = list;

    get_attr.value.s32list = table_list;

    ASSERT_EQ(SAI_STATUS_BUFFER_OVERFLOW, p_sai_acl_api_tbl->
              get_acl_table_group_attribute(acl_table_group_id,
                                     attr_count,
                                     &get_attr));

    table_list.count = get_attr.value.s32list.count;
    table_list.list = (int32_t *) calloc (table_list.count, sizeof (int32_t));
    ASSERT_EQ(SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->
              get_acl_table_group_attribute(acl_table_group_id,
                                     attr_count,
                                     &get_attr));

    for(i = 0; i <  get_attr.value.s32list.count; i ++)
    {
        printf("Bind point %d \r\n",  get_attr.value.s32list.list[i]);
    }

    free(list);

    attr_count = 0;
    group_mem_attr[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID;
    group_mem_attr[attr_count].value.oid = acl_table_group_id;

    attr_count ++;

    group_mem_attr[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID;
    group_mem_attr[attr_count].value.oid = acl_table_id;

    attr_count ++;
    group_mem_attr[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY;
    group_mem_attr[attr_count].value.s32 = 5;

    attr_count ++;
    ASSERT_EQ(SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->
            create_acl_table_group_member(&acl_table_group_mem_id,
                saiACLTest ::sai_acl_get_global_switch_id(),
                attr_count,
                (const sai_attribute_t *)group_mem_attr));

    attr_count = 0;
    group_mem_attr[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID;
    group_mem_attr[attr_count].value.oid = acl_table_group_id;

    attr_count ++;

    group_mem_attr[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID;
    group_mem_attr[attr_count].value.oid = acl_table_id1;

    attr_count ++;
    group_mem_attr[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY;
    group_mem_attr[attr_count].value.s32 = 6;

    attr_count ++;
    ASSERT_EQ(SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->
            create_acl_table_group_member(&acl_table_group_mem_id1,
                saiACLTest ::sai_acl_get_global_switch_id(),
                attr_count,
                (const sai_attribute_t *)group_mem_attr));

    printf("Acl table id 0x%" PRIx64 "\r\n", acl_table_id);
    printf("Acl table group id 0x%" PRIx64 "\r\n", acl_table_group_id);
    printf("Acl table group mem id 0x%" PRIx64 "\r\n", acl_table_group_mem_id);
    printf("Acl table group mem id 0x%" PRIx64 "\r\n", acl_table_group_mem_id1);

    attr_count  = 0;
    get_attr.id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY;

    ASSERT_EQ(SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->
              get_acl_table_group_member_attribute(acl_table_group_mem_id1,
                                                   1,
                                                   &get_attr));

    ASSERT_EQ(get_attr.value.s32,6);

    sai_rc = sai_test_acl_rule_create (&acl_rule_id, 4,
                                SAI_ACL_ENTRY_ATTR_TABLE_ID, acl_table_id,
                                SAI_ACL_ENTRY_ATTR_PRIORITY, 5,
                                SAI_ACL_ENTRY_ATTR_ADMIN_STATE, true,
                                SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE,
                                1, 34825, 0xffff);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    set_attr.id = SAI_PORT_ATTR_INGRESS_ACL;
    set_attr.value.oid = acl_table_group_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS, p_sai_port_api_tbl->set_port_attribute(
                                                          port_id_1,
                                                       (const sai_attribute_t *)&set_attr));


    set_attr.id = SAI_ACL_TABLE_GROUP_ATTR_TYPE;
    set_attr.value.s32 = SAI_ACL_TABLE_GROUP_TYPE_PARALLEL;
    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0, p_sai_acl_api_tbl->set_acl_table_group_attribute(
                                                          acl_table_group_id,
                                                       (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY;
    set_attr.value.s32 = 10;
    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0, p_sai_acl_api_tbl->set_acl_table_group_member_attribute(
                                                          acl_table_group_mem_id,
                                                       (const sai_attribute_t *)&set_attr));
    sai_rc = sai_test_acl_rule_remove(acl_rule_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_acl_table_remove (acl_table_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE, p_sai_acl_api_tbl->
              remove_acl_table_group(acl_table_group_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->
              remove_acl_table_group_member(acl_table_group_mem_id));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE, p_sai_acl_api_tbl->
              remove_acl_table_group(acl_table_group_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->
              remove_acl_table_group_member(acl_table_group_mem_id1));

    ASSERT_EQ(SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->
              remove_acl_table_group(acl_table_group_id));


}

int main (int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

