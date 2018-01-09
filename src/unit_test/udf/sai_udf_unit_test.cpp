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
* @file  sai_udf_unit_test_utils.cpp
*
* @brief This file contains utility and helper function definitions for
*        testing the SAI UDF functionalities.
*
*************************************************************************/

#include "gtest/gtest.h"

#include "sai_udf_unit_test.h"

extern "C" {
#include "sai.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saiudf.h"
}

/*
 * Validates UDF Group creation and removal for Generic type
 */
TEST_F (saiUdfTest, create_and_remove_generic_udf_group)
{
    sai_status_t       status;
    sai_object_id_t    udf_group_id = SAI_NULL_OBJECT_ID;
    const unsigned int udf_length = 4;

    status = sai_test_udf_group_create (&udf_group_id, dflt_udf_grp_attr_count,
                                        SAI_UDF_GROUP_ATTR_TYPE,
                                        SAI_UDF_GROUP_TYPE_GENERIC,
                                        SAI_UDF_GROUP_ATTR_LENGTH, udf_length);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF Group object using get API */
    sai_test_udf_group_verify (udf_group_id, SAI_UDF_GROUP_TYPE_GENERIC, udf_length,
                               NULL);
    status = sai_test_udf_group_remove (udf_group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates UDF Group creation and removal for Hash type
 */
TEST_F (saiUdfTest, create_and_remove_hash_udf_group)
{
    sai_status_t       status;
    sai_object_id_t    udf_group_id = SAI_NULL_OBJECT_ID;
    const unsigned int udf_length = 2;

    status = sai_test_udf_group_create (&udf_group_id, dflt_udf_grp_attr_count,
                                        SAI_UDF_GROUP_ATTR_TYPE,
                                        SAI_UDF_GROUP_TYPE_HASH,
                                        SAI_UDF_GROUP_ATTR_LENGTH, udf_length);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF Group object using get API */
    sai_test_udf_group_verify (udf_group_id, SAI_UDF_GROUP_TYPE_HASH, udf_length,
                               NULL);
    status = sai_test_udf_group_remove (udf_group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates UDF Match creation with no match type attribute.
 */
TEST_F (saiUdfTest, create_and_remove_default_udf_match)
{
    sai_status_t     status;
    sai_object_id_t  udf_match_id = SAI_NULL_OBJECT_ID;

    status = sai_test_udf_match_create (&udf_match_id,
                                        default_udf_match_attr_count);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF Match object using get API */
    sai_test_udf_match_verify (udf_match_id, max_udf_match_attr_count,
                               SAI_UDF_MATCH_ATTR_L2_TYPE, 0, 0,
                               SAI_UDF_MATCH_ATTR_L3_TYPE, 0, 0,
                               SAI_UDF_MATCH_ATTR_GRE_TYPE, 0, 0,
                               SAI_UDF_MATCH_ATTR_PRIORITY, 0);

    status = sai_test_udf_match_remove (udf_match_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates UDF Match creation with Priority attribute.
 */
TEST_F (saiUdfTest, set_udf_match_priority)
{
    sai_status_t     status;
    const unsigned int match_attr_count = 2;
    sai_object_id_t  udf_match_id = SAI_NULL_OBJECT_ID;

    status = sai_test_udf_match_create (&udf_match_id, match_attr_count,
                                        SAI_UDF_MATCH_ATTR_L2_TYPE,
                                        0x0806, 0xFFFF,
                                        SAI_UDF_MATCH_ATTR_PRIORITY, 3);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF Match object using get API */
    sai_test_udf_match_verify (udf_match_id, match_attr_count,
                               SAI_UDF_MATCH_ATTR_L2_TYPE,
                               0x0806, 0xFFFF,
                               SAI_UDF_MATCH_ATTR_PRIORITY, 3);

    status = sai_test_udf_match_remove (udf_match_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates UDF creation for matching lower order 2 bytes of inner source ip
 * field for GRE encapsulated IPv4 packets
 */
TEST_F (saiUdfTest, create_and_remove_inner_sip_udf)
{
    sai_status_t       status;
    sai_object_id_t    udf_match_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t    udf_id = SAI_NULL_OBJECT_ID;
    sai_object_list_t  udf_list = {1, &udf_id};
    const unsigned int match_attr_count = 3;
    const unsigned int offset = 56; /* 18 + 20 + 4 + 14 */

    status = sai_test_udf_match_create (&udf_match_id, match_attr_count,
                                        SAI_UDF_MATCH_ATTR_L2_TYPE,
                                        0x0800, 0xFFFF,
                                        SAI_UDF_MATCH_ATTR_L3_TYPE,
                                        0x2F, 0xFF,
                                        SAI_UDF_MATCH_ATTR_GRE_TYPE,
                                        0x0800, 0xFFFF);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF Match object using get API */
    sai_test_udf_match_verify (udf_match_id, match_attr_count,
                               SAI_UDF_MATCH_ATTR_L2_TYPE,
                               0x0800, 0xFFFF,
                               SAI_UDF_MATCH_ATTR_L3_TYPE,
                               0x2F, 0xFF,
                               SAI_UDF_MATCH_ATTR_GRE_TYPE,
                               0x0800, 0xFFFF);

    status = sai_test_udf_create (udf_list.list, NULL, default_udf_attr_count,
                                  SAI_UDF_ATTR_MATCH_ID, udf_match_id,
                                  SAI_UDF_ATTR_GROUP_ID, dflt_udf_group_id,
                                  SAI_UDF_ATTR_OFFSET, offset);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF object using get API */
    sai_test_udf_verify (udf_id, udf_match_id, dflt_udf_group_id,
                         SAI_UDF_BASE_L2, offset);

    /* Verify the UDF Group contains the UDF object */
    sai_test_udf_group_verify (dflt_udf_group_id, SAI_UDF_GROUP_TYPE_GENERIC,
                               default_udf_len, &udf_list);

    status = sai_test_udf_remove (udf_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_match_remove (udf_match_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates UDF creation for matching SRC ECID field of the 802.1BR E-TAG
 */
TEST_F (saiUdfTest, create_and_remove_src_ecid_udf)
{
    sai_status_t       status;
    sai_object_id_t    udf_match_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t    udf_id = SAI_NULL_OBJECT_ID;
    sai_object_list_t  udf_list = {1, &udf_id};
    const unsigned int match_attr_count = 1;
    const unsigned int offset = 14;

    status = sai_test_udf_match_create (&udf_match_id, match_attr_count,
                                        SAI_UDF_MATCH_ATTR_L2_TYPE,
                                        0x893F, 0xFFFF);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF Match object using get API */
    sai_test_udf_match_verify (udf_match_id, match_attr_count,
                               SAI_UDF_MATCH_ATTR_L2_TYPE, 0x893F, 0xFFFF);

    status = sai_test_udf_create (udf_list.list, NULL, default_udf_attr_count,
                                  SAI_UDF_ATTR_MATCH_ID, udf_match_id,
                                  SAI_UDF_ATTR_GROUP_ID, dflt_udf_group_id,
                                  SAI_UDF_ATTR_OFFSET, offset);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF object using get API */
    sai_test_udf_verify (udf_id, udf_match_id, dflt_udf_group_id,
                         SAI_UDF_BASE_L2, offset);

    /* Verify the UDF Group contains the UDF object */
    sai_test_udf_group_verify (dflt_udf_group_id, SAI_UDF_GROUP_TYPE_GENERIC,
                               default_udf_len, &udf_list);

    status = sai_test_udf_remove (udf_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_match_remove (udf_match_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates UDF creation for a Hash UDF group. Creates UDF to extract 4 bytes
 * inner source ip field of GRE encapsulated IPv4 packets for hashing.
 */
TEST_F (saiUdfTest, create_and_remove_hash_udf)
{
    sai_status_t       status;
    sai_object_id_t    udf_match_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t    udf_group_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t    udf_id = SAI_NULL_OBJECT_ID;
    sai_object_list_t  udf_list = {1, &udf_id};
    const unsigned int udf_length = 4;
    const unsigned int match_attr_count = 3;
    const unsigned int offset = 54; /* 18 + 20 + 4 + 12 */
    uint8_t            hash_mask [udf_length] = {0xFF, 0xFF, 0xFF, 0xFF};
    sai_u8_list_t      hash_mask_list = {udf_length, hash_mask};

    status = sai_test_udf_group_create (&udf_group_id, dflt_udf_grp_attr_count,
                                        SAI_UDF_GROUP_ATTR_TYPE,
                                        SAI_UDF_GROUP_TYPE_HASH,
                                        SAI_UDF_GROUP_ATTR_LENGTH, udf_length);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_match_create (&udf_match_id, match_attr_count,
                                        SAI_UDF_MATCH_ATTR_L2_TYPE,
                                        0x0800, 0xFFFF,
                                        SAI_UDF_MATCH_ATTR_L3_TYPE,
                                        0x2F, 0xFF,
                                        SAI_UDF_MATCH_ATTR_GRE_TYPE,
                                        0x0800, 0xFFFF);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF Match object using get API */
    sai_test_udf_match_verify (udf_match_id, match_attr_count,
                               SAI_UDF_MATCH_ATTR_L2_TYPE,
                               0x0800, 0xFFFF,
                               SAI_UDF_MATCH_ATTR_L3_TYPE,
                               0x2F, 0xFF,
                               SAI_UDF_MATCH_ATTR_GRE_TYPE,
                               0x0800, 0xFFFF);

    status = sai_test_udf_create (udf_list.list, NULL, default_udf_attr_count,
                                  SAI_UDF_ATTR_MATCH_ID, udf_match_id,
                                  SAI_UDF_ATTR_GROUP_ID, udf_group_id,
                                  SAI_UDF_ATTR_OFFSET, offset);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF object using get API */
    sai_test_udf_verify (udf_id, udf_match_id, udf_group_id,
                         SAI_UDF_BASE_L2, offset);

    /* Verify the default UDF Hash mask using get API */
    sai_test_udf_hash_mask_verify (udf_id, &hash_mask_list);

    /* Verify the UDF Group contains the UDF object */
    sai_test_udf_group_verify (udf_group_id, SAI_UDF_GROUP_TYPE_HASH, udf_length,
                               &udf_list);

    status = sai_test_udf_remove (udf_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_group_remove (udf_group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_match_remove (udf_match_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates Hash mask attribute for UDF object during UDF creation.
 * Creates the same UDF to extract 4 bytes inner source ip field of GRE
 * encapsulated IPv4 packets. Set hash mask to apply only the higher order
 * 2 bytes of the ip address for hashing.
 */
TEST_F (saiUdfTest, create_udf_with_udf_hash_mask)
{
    sai_status_t       status;
    sai_object_id_t    udf_group_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t    udf_match_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t    udf_id = SAI_NULL_OBJECT_ID;
    const unsigned int udf_length = 4;
    const unsigned int match_attr_count = 3;
    const unsigned int udf_attr_count = 5;
    const unsigned int offset = 36; /* 20 + 4 + 12 */
    uint8_t            hash_mask [udf_length] = {0xFF, 0xFF, 0, 0};
    sai_u8_list_t      hash_mask_list = {udf_length, hash_mask};


    status = sai_test_udf_group_create (&udf_group_id, dflt_udf_grp_attr_count,
                                        SAI_UDF_GROUP_ATTR_TYPE,
                                        SAI_UDF_GROUP_TYPE_HASH,
                                        SAI_UDF_GROUP_ATTR_LENGTH, udf_length);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_match_create (&udf_match_id, match_attr_count,
                                        SAI_UDF_MATCH_ATTR_L2_TYPE,
                                        0x0800, 0xFFFF,
                                        SAI_UDF_MATCH_ATTR_L3_TYPE,
                                        0x2F, 0xFF,
                                        SAI_UDF_MATCH_ATTR_GRE_TYPE,
                                        0x0800, 0xFFFF);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF Match object using get API */
    sai_test_udf_match_verify (udf_match_id, match_attr_count,
                               SAI_UDF_MATCH_ATTR_L2_TYPE,
                               0x0800, 0xFFFF,
                               SAI_UDF_MATCH_ATTR_L3_TYPE,
                               0x2F, 0xFF,
                               SAI_UDF_MATCH_ATTR_GRE_TYPE,
                               0x0800, 0xFFFF);

    status = sai_test_udf_create (&udf_id, &hash_mask_list, udf_attr_count,
                                  SAI_UDF_ATTR_MATCH_ID, udf_match_id,
                                  SAI_UDF_ATTR_GROUP_ID, udf_group_id,
                                  SAI_UDF_ATTR_BASE, SAI_UDF_BASE_L3,
                                  SAI_UDF_ATTR_OFFSET, offset,
                                  SAI_UDF_ATTR_HASH_MASK);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF object using get API */
    sai_test_udf_verify (udf_id, udf_match_id, udf_group_id,
                         SAI_UDF_BASE_L3, offset);

    /* Verify the UDF Hash mask bytes using get API */
    sai_test_udf_hash_mask_verify (udf_id, &hash_mask_list);

    status = sai_test_udf_remove (udf_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_group_remove (udf_group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_match_remove (udf_match_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates Hash mask attribute for UDF object in set API.
 * Creates UDF to lookup VLAN Tag of the packets and
 * sets hash mask attribute to extract VLAN ID for hashing.
 */
TEST_F (saiUdfTest, set_udf_hash_mask)
{
    sai_status_t       status;
    sai_object_id_t    udf_group_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t    udf_match_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t    udf_id = SAI_NULL_OBJECT_ID;
    const unsigned int udf_length = 2;
    const unsigned int match_attr_count = 1;
    const unsigned int udf_attr_count = 3;
    const unsigned int offset = 14;
    uint8_t            set_value [udf_length] = {0x0F, 0xFF};
    uint8_t            reset_value [udf_length] = {0, 0};
    sai_attribute_t    attr;

    memset (&attr, 0, sizeof (sai_attribute_t));

    attr.id = SAI_UDF_ATTR_HASH_MASK;

    status = sai_test_udf_group_create (&udf_group_id, dflt_udf_grp_attr_count,
                                        SAI_UDF_GROUP_ATTR_TYPE,
                                        SAI_UDF_GROUP_TYPE_HASH,
                                        SAI_UDF_GROUP_ATTR_LENGTH, udf_length);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_match_create (&udf_match_id, match_attr_count,
                                        SAI_UDF_MATCH_ATTR_L2_TYPE,
                                        0x8100, 0xFFFF);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_create (&udf_id, NULL, udf_attr_count,
                                  SAI_UDF_ATTR_MATCH_ID, udf_match_id,
                                  SAI_UDF_ATTR_GROUP_ID, udf_group_id,
                                  SAI_UDF_ATTR_OFFSET, offset);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Set UDF Hash mask to the UDF */
    attr.value.u8list.count = udf_length;
    attr.value.u8list.list  = set_value;
    status = udf_api_tbl_get()->set_udf_attribute (udf_id, &attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF Hash mask bytes using get API */
    sai_test_udf_hash_mask_verify (udf_id, &attr.value.u8list);

    /* Reset the UDF Hash mask to stop hashing based on VLAN Id */
    attr.value.u8list.list  = reset_value;
    status = udf_api_tbl_get()->set_udf_attribute (udf_id, &attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF Hash mask bytes are reset using get API */
    sai_test_udf_hash_mask_verify (udf_id, &attr.value.u8list);

    status = sai_test_udf_remove (udf_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_group_remove (udf_group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_match_remove (udf_match_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Verify Mandatory attribute missing case for UDF Group creation
 */
TEST_F (saiUdfTest, udf_group_mandatory_attr_missing)
{
    sai_status_t       status;
    sai_object_id_t    udf_group_id = SAI_NULL_OBJECT_ID;
    const unsigned int udf_length = 2;
    const unsigned int attr_count = 1;

    /* Create without the UDF length attr */
    status = sai_test_udf_group_create (&udf_group_id, attr_count,
                                        SAI_UDF_GROUP_ATTR_TYPE,
                                        SAI_UDF_GROUP_TYPE_HASH);

    EXPECT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING, status);

    /* Create without the UDF Group type attr and
     * verify a generic UDF group is created by default */
    status = sai_test_udf_group_create (&udf_group_id, attr_count,
                                        SAI_UDF_GROUP_ATTR_LENGTH, udf_length);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF Group object using get API */
    sai_test_udf_group_verify (udf_group_id, SAI_UDF_GROUP_TYPE_GENERIC, udf_length,
                               NULL);
    status = sai_test_udf_group_remove (udf_group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Invalid attribute values for UDF Group creation
 */
TEST_F (saiUdfTest, udf_group_invalid_attr_value)
{
    sai_status_t       status;
    sai_object_id_t    udf_group_id = SAI_NULL_OBJECT_ID;
    const int          invalid_type = 0xFF;
    const unsigned int udf_length = 2;

    /* Check if invalid group type is not allowed */
    status = sai_test_udf_group_create (&udf_group_id, dflt_udf_grp_attr_count,
                                        SAI_UDF_GROUP_ATTR_TYPE, invalid_type,
                                        SAI_UDF_GROUP_ATTR_LENGTH, udf_length);

    EXPECT_EQ ((sai_test_invalid_attr_status_code (SAI_STATUS_INVALID_ATTR_VALUE_0,
                0)), status);

    /* Check if 0 UDF length is not allowed */
    status = sai_test_udf_group_create (&udf_group_id, dflt_udf_grp_attr_count,
                                        SAI_UDF_GROUP_ATTR_TYPE,
                                        SAI_UDF_GROUP_TYPE_GENERIC,
                                        SAI_UDF_GROUP_ATTR_LENGTH, 0);

    EXPECT_EQ ((sai_test_invalid_attr_status_code (SAI_STATUS_INVALID_ATTR_VALUE_0,
                1)), status);
}

/*
 * Verify Mandatory attribute missing case for UDF creation
 */
TEST_F (saiUdfTest, udf_mandatory_attr_missing)
{
    sai_status_t       status;
    sai_object_id_t    udf_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t    udf_match_id = SAI_NULL_OBJECT_ID;
    const unsigned int offset = 0;
    const unsigned int attr_count = default_udf_attr_count - 1;

    /* Create without the UDF Match Id attr */
    status = sai_test_udf_create (&udf_id, NULL, attr_count,
                                  SAI_UDF_ATTR_GROUP_ID, dflt_udf_group_id,
                                  SAI_UDF_ATTR_OFFSET, offset);

    EXPECT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING, status);

    /* Create without the UDF Group Id attr */
    status = sai_test_udf_match_create (&udf_match_id,
                                        default_udf_match_attr_count);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_create (&udf_id, NULL, attr_count,
                                  SAI_UDF_ATTR_MATCH_ID, udf_match_id,
                                  SAI_UDF_ATTR_OFFSET, offset);

    EXPECT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING, status);

    /* Create without the UDF offset attr */
    status = sai_test_udf_create (&udf_id, NULL, attr_count,
                                  SAI_UDF_ATTR_GROUP_ID, dflt_udf_group_id,
                                  SAI_UDF_ATTR_MATCH_ID, udf_match_id);

    EXPECT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING, status);

    status = sai_test_udf_match_remove (udf_match_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Invalid attribute values for UDF creation
 */
TEST_F (saiUdfTest, udf_invalid_attr_value)
{
    sai_status_t           status;
    sai_object_id_t        udf_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t        udf_match_id = SAI_NULL_OBJECT_ID;
    const unsigned int     offset = 12;
    const int              invalid_base = 0xFF;
    const sai_object_id_t  invalid_obj_id = SAI_NULL_OBJECT_ID;
    const unsigned int     attr_count = default_udf_attr_count + 1;

    /* Check if invalid match id is not allowed */
    status = sai_test_udf_create (&udf_id, NULL, attr_count,
                                  SAI_UDF_ATTR_MATCH_ID, invalid_obj_id,
                                  SAI_UDF_ATTR_GROUP_ID, dflt_udf_group_id,
                                  SAI_UDF_ATTR_BASE, SAI_UDF_BASE_L2,
                                  SAI_UDF_ATTR_OFFSET, offset);

    EXPECT_EQ ((sai_test_invalid_attr_status_code (SAI_STATUS_INVALID_ATTR_VALUE_0,
                0)), status);

    /* Check if invalid group id is not allowed */
    status = sai_test_udf_match_create (&udf_match_id,
                                        default_udf_match_attr_count);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_create (&udf_id, NULL, attr_count,
                                  SAI_UDF_ATTR_MATCH_ID, udf_match_id,
                                  SAI_UDF_ATTR_GROUP_ID, invalid_obj_id,
                                  SAI_UDF_ATTR_BASE, SAI_UDF_BASE_L2,
                                  SAI_UDF_ATTR_OFFSET, offset);

    EXPECT_EQ ((sai_test_invalid_attr_status_code (SAI_STATUS_INVALID_ATTR_VALUE_0,
                1)), status);

    /* Check if invalid udf base is not allowed */
    status = sai_test_udf_create (&udf_id, NULL, attr_count,
                                  SAI_UDF_ATTR_MATCH_ID, udf_match_id,
                                  SAI_UDF_ATTR_GROUP_ID, dflt_udf_group_id,
                                  SAI_UDF_ATTR_BASE, invalid_base,
                                  SAI_UDF_ATTR_OFFSET, offset);

    EXPECT_EQ ((sai_test_invalid_attr_status_code (SAI_STATUS_INVALID_ATTR_VALUE_0,
                2)), status);

    status = sai_test_udf_match_remove (udf_match_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates if Hash mask attribute is not allowed to be set for UDF
 * object of generic UDF Group.
 */
TEST_F (saiUdfTest, udf_hash_mask_for_generic_udf_group)
{
    sai_status_t       status;
    sai_object_id_t    udf_match_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t    udf_id = SAI_NULL_OBJECT_ID;
    const unsigned int offset = 14;
    const unsigned int match_attr_count = 1;
    const unsigned int udf_attr_count = 4;
    const uint32_t     byte_count = 2;
    uint8_t            byte_value [byte_count] = {0x21, 0x21};
    sai_u8_list_t      hash_mask_list = {byte_count, byte_value};

    status = sai_test_udf_match_create (&udf_match_id, match_attr_count,
                                        SAI_UDF_MATCH_ATTR_L2_TYPE,
                                        0x8100, 0xFFFF);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_create (&udf_id, &hash_mask_list, udf_attr_count,
                                  SAI_UDF_ATTR_MATCH_ID, udf_match_id,
                                  SAI_UDF_ATTR_GROUP_ID, dflt_udf_group_id,
                                  SAI_UDF_ATTR_OFFSET, offset,
                                  SAI_UDF_ATTR_HASH_MASK);

    EXPECT_TRUE (SAI_STATUS_SUCCESS != status);
    EXPECT_TRUE (udf_id == SAI_NULL_OBJECT_ID);

    status = sai_test_udf_match_remove (udf_match_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates if Hash mask of size not same as the UDF length is not allowed.
 */
TEST_F (saiUdfTest, invalid_udf_hash_mask)
{
    sai_status_t       status;
    sai_object_id_t    udf_group_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t    udf_match_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t    udf_id = SAI_NULL_OBJECT_ID;
    const unsigned int udf_length = 4;
    const unsigned int match_attr_count = 1;
    const unsigned int udf_attr_count = 4;
    const unsigned int offset = 14;
    const uint32_t     byte_count = 2;
    uint8_t            byte_value [byte_count] = {0x21, 0x21};
    sai_u8_list_t      hash_mask_list = {byte_count, byte_value};

    status = sai_test_udf_group_create (&udf_group_id, dflt_udf_grp_attr_count,
                                        SAI_UDF_GROUP_ATTR_TYPE,
                                        SAI_UDF_GROUP_TYPE_HASH,
                                        SAI_UDF_GROUP_ATTR_LENGTH, udf_length);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_match_create (&udf_match_id, match_attr_count,
                                        SAI_UDF_MATCH_ATTR_L2_TYPE,
                                        0x8100, 0xFFFF);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF Match object using get API */
    sai_test_udf_match_verify (udf_match_id, match_attr_count,
                               SAI_UDF_MATCH_ATTR_L2_TYPE, 0x8100, 0xFFFF);

    status = sai_test_udf_create (&udf_id, &hash_mask_list, udf_attr_count,
                                  SAI_UDF_ATTR_MATCH_ID, udf_match_id,
                                  SAI_UDF_ATTR_GROUP_ID, udf_group_id,
                                  SAI_UDF_ATTR_OFFSET, offset,
                                  SAI_UDF_ATTR_HASH_MASK);

    EXPECT_TRUE (SAI_STATUS_SUCCESS != status);
    EXPECT_TRUE (udf_id == SAI_NULL_OBJECT_ID);

    status = sai_test_udf_group_remove (udf_group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_udf_match_remove (udf_match_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

int main (int argc, char **argv)
{
    ::testing::InitGoogleTest (&argc, argv);

    return RUN_ALL_TESTS ();
}
