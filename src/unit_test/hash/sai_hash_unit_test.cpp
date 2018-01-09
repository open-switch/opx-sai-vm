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
* @file  sai_hash_unit_test.cpp
*
* @brief This file contains test case functions for the SAI Hash functionality.
*
*************************************************************************/

#include "gtest/gtest.h"

#include "sai_hash_unit_test.h"
#include "sai_udf_unit_test.h"

extern "C" {
#include "sai.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saihash.h"
#include "saiudf.h"
}

static sai_object_id_t switch_id =0;
/*
 * Validates Hash fields for Default ECMP Hash object
 */
TEST_F (saiHashTest, default_ecmp_hash_fields_get)
{
    sai_attribute_t    switch_attr;
    sai_status_t       status;
    const unsigned int count = 1;
    sai_s32_list_t     native_list = {default_native_fld_count,
                                      default_native_fields};

    /* Get the default ECMP Hash object id value */
    switch_attr.id = SAI_SWITCH_ATTR_ECMP_HASH;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &switch_attr);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the default Hash object fields */
    sai_test_hash_verify (switch_attr.value.oid, &native_list, NULL);
}

/*
 * Validates Hash fields for Default LAG Hash object
 */
TEST_F (saiHashTest, default_lag_hash_fields_get)
{
    sai_attribute_t    switch_attr;
    sai_status_t       status;
    const unsigned int count = 1;
    sai_s32_list_t     native_list = {default_native_fld_count,
                                      default_native_fields};

    /* Get the default LAG Hash object id value */
    switch_attr.id = SAI_SWITCH_ATTR_LAG_HASH;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &switch_attr);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the default Hash object fields */
    sai_test_hash_verify (switch_attr.value.oid, &native_list, NULL);
}

/*
 * Validates setting new hash field SAI_NATIVE_HASH_FIELD_VLAN_ID to
 * Default LAG Hash object
 */
TEST_F (saiHashTest, default_lag_hash_fields_set)
{
    sai_attribute_t    switch_attr;
    sai_status_t       status;
    const unsigned int count = 1;
    const unsigned int field_count = default_native_fld_count + 1;
    const unsigned int new_field_index = field_count - 1;
    int32_t            field_list [field_count] = {0};
    sai_attribute_t    attr_list [count];
    sai_s32_list_t     native_list = {field_count, field_list};

    /* Get the default LAG Hash object id value */
    switch_attr.id = SAI_SWITCH_ATTR_LAG_HASH;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &switch_attr);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Get the default Hash object fields */
    attr_list[0].value.s32list = native_list;
    status = sai_test_hash_attr_get (switch_attr.value.oid, attr_list, count,
                                     SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    EXPECT_EQ (attr_list[0].value.s32list.count, (field_count - 1));

    /* Set the VLAN Id field in native fields list */
    field_list[new_field_index] = SAI_NATIVE_HASH_FIELD_VLAN_ID;

    status = sai_test_hash_attr_set (switch_attr.value.oid, &native_list, NULL,
                                     SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the new field is returned in default Hash object fields */
    sai_test_hash_verify (switch_attr.value.oid, &native_list, NULL);
}

/*
 * Validates removing a hash field from Default ECMP Hash object
 */
TEST_F (saiHashTest, default_ecmp_hash_fields_set)
{
    sai_attribute_t    switch_attr;
    sai_status_t       status;
    const unsigned int count = 1;
    const unsigned int field_count = default_native_fld_count;
    int32_t            field_list [field_count] = {0};
    sai_attribute_t    attr_list [count];
    sai_s32_list_t     native_list = {field_count, field_list};

    /* Get the default ECMP Hash object id value */
    switch_attr.id = SAI_SWITCH_ATTR_ECMP_HASH;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &switch_attr);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Get the default Hash object fields */
    attr_list[0].value.s32list = native_list;
    status = sai_test_hash_attr_get (switch_attr.value.oid, attr_list, count,
                                     SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    EXPECT_EQ (attr_list[0].value.s32list.count, field_count);

    /* Reduce the count in the native fields list */
    native_list.count = field_count - 1;
    native_list.list [native_list.count] = -1;

    status = sai_test_hash_attr_set (switch_attr.value.oid, &native_list, NULL,
                                     SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the removed field is not returned in default Hash object fields */
    sai_test_hash_verify (switch_attr.value.oid, &native_list, NULL);

    native_list.count = default_native_fld_count;
    native_list.list  = default_native_fields;

    /* ReSet the native fields list with default values */
    status = sai_test_hash_attr_set (switch_attr.value.oid, &native_list, NULL,
                                     SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates resetting all fields in Default LAG Hash object
 */
TEST_F (saiHashTest, default_lag_hash_all_fields_remove)
{
    sai_attribute_t    switch_attr;
    sai_status_t       status;
    const unsigned int count = 1;
    const unsigned int field_count = 0;
    int32_t            field_list [field_count];
    sai_s32_list_t     native_list = {field_count, field_list};

    /* Get the default ECMP Hash object id value */
    switch_attr.id = SAI_SWITCH_ATTR_ECMP_HASH;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &switch_attr);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Set the native fields list with empty list */
    status = sai_test_hash_attr_set (switch_attr.value.oid, &native_list, NULL,
                                     SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the empty list is returned in default Hash object fields */
    sai_test_hash_verify (switch_attr.value.oid, &native_list, NULL);

    native_list.count = default_native_fld_count;
    native_list.list  = default_native_fields;

    /* ReSet the native fields list with default values */
    status = sai_test_hash_attr_set (switch_attr.value.oid, &native_list, NULL,
                                     SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the fields are  returned in default Hash object fields */
    sai_test_hash_verify (switch_attr.value.oid, &native_list, NULL);
}

/*
 * Validates that resetting default Hash object is not allowed
 */
TEST_F (saiHashTest, default_hash_object_remove)
{
    sai_attribute_t    switch_attr;
    sai_status_t       status;

    /* Set the default LAG Hash object id value to NULL */
    switch_attr.id        = SAI_SWITCH_ATTR_LAG_HASH;
    switch_attr.value.oid = SAI_NULL_OBJECT_ID;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_NE (SAI_STATUS_SUCCESS, status);

    /* Set the default ECMP Hash object id value to NULL */
    switch_attr.id        = SAI_SWITCH_ATTR_ECMP_HASH;
    switch_attr.value.oid = SAI_NULL_OBJECT_ID;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_NE (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates new Hash object create and remove with native fields
 */
TEST_F (saiHashTest, hash_object_create_remove_native_fields)
{
    sai_status_t       status;
    sai_object_id_t    hash_id;
    const unsigned int attr_count = 1;
    const unsigned int field_count = 5;
    int32_t            field_list [field_count] =
    { SAI_NATIVE_HASH_FIELD_SRC_IP, SAI_NATIVE_HASH_FIELD_DST_IP,
      SAI_NATIVE_HASH_FIELD_L4_SRC_PORT, SAI_NATIVE_HASH_FIELD_L4_DST_PORT,
      SAI_NATIVE_HASH_FIELD_IP_PROTOCOL };
    sai_s32_list_t     native_list = {field_count, field_list};

    status = sai_test_hash_create (&hash_id, &native_list, NULL, attr_count,
                                   SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Hash object fields */
    sai_test_hash_verify (hash_id, &native_list, NULL);

    /* Remove the Hash object */
    status = sai_test_hash_remove (hash_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates IPV4 Hash object create with 5 tuple native fields.
 * Verifies that the hash object cannot be removed when set in Switch attr.
 */
TEST_F (saiHashTest, ipv4_ecmp_hash_object)
{
    sai_status_t       status;
    sai_attribute_t    switch_attr;
    sai_object_id_t    hash_id;
    const unsigned int attr_count = 1;
    const unsigned int field_count = 5;
    int32_t            field_list [field_count] =
    { SAI_NATIVE_HASH_FIELD_SRC_IP, SAI_NATIVE_HASH_FIELD_DST_IP,
      SAI_NATIVE_HASH_FIELD_L4_SRC_PORT, SAI_NATIVE_HASH_FIELD_L4_DST_PORT,
      SAI_NATIVE_HASH_FIELD_IP_PROTOCOL };
    sai_s32_list_t     native_list = {field_count, field_list};

    status = sai_test_hash_create (&hash_id, &native_list, NULL, attr_count,
                                   SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Hash object fields */
    sai_test_hash_verify (hash_id, &native_list, NULL);

    /* Set the IPV4 ECMP Hash object id value */
    switch_attr.id        = SAI_SWITCH_ATTR_ECMP_HASH_IPV4;
    switch_attr.value.oid = hash_id;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Get the IPV4 ECMP Hash object id value */
    switch_attr.value.oid = SAI_NULL_OBJECT_ID;
    switch_attr.id = SAI_SWITCH_ATTR_ECMP_HASH_IPV4;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,attr_count, &switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    EXPECT_EQ (switch_attr.value.oid, hash_id);

    /* Verify that the hash object cannot be removed now */
    status = sai_test_hash_remove (hash_id);

    EXPECT_EQ (SAI_STATUS_OBJECT_IN_USE, status);

    /* Reset the IPV4 ECMP Hash object */
    switch_attr.id        = SAI_SWITCH_ATTR_ECMP_HASH_IPV4;
    switch_attr.value.oid = SAI_NULL_OBJECT_ID;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify IPV4 ECMP Hash object is set to NULL value */
    switch_attr.id = SAI_SWITCH_ATTR_ECMP_HASH_IPV4;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,attr_count, &switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    EXPECT_EQ (switch_attr.value.oid, SAI_NULL_OBJECT_ID);

    /* Remove the Hash object */
    status = sai_test_hash_remove (hash_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates setting new fields to IPV4 Hash object when it is set
 * in switch attribute
 */
TEST_F (saiHashTest, ipv4_ecmp_hash_object_set_attr)
{
    sai_status_t       status;
    sai_attribute_t    switch_attr;
    sai_object_id_t    hash_id;
    const unsigned int attr_count = 1;
    const unsigned int field_count = 5;
    int32_t            field_list [field_count + 1] =
    { SAI_NATIVE_HASH_FIELD_SRC_IP, SAI_NATIVE_HASH_FIELD_DST_IP,
      SAI_NATIVE_HASH_FIELD_L4_SRC_PORT, SAI_NATIVE_HASH_FIELD_L4_DST_PORT,
      SAI_NATIVE_HASH_FIELD_IP_PROTOCOL };
    sai_s32_list_t     native_list = {field_count, field_list};

    status = sai_test_hash_create (&hash_id, &native_list, NULL, attr_count,
                                   SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Hash object fields */
    sai_test_hash_verify (hash_id, &native_list, NULL);

    /* Set the IPV4 ECMP Hash object id value */
    switch_attr.id        = SAI_SWITCH_ATTR_ECMP_HASH_IPV4;
    switch_attr.value.oid = hash_id;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Set the new hash field SAI_NATIVE_HASH_FIELD_IN_PORT */
    native_list.count = field_count + 1;
    native_list.list [field_count] = SAI_NATIVE_HASH_FIELD_IN_PORT;

    status = sai_test_hash_attr_set (hash_id, &native_list, NULL,
                                     SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Hash object fields include the new field */
    sai_test_hash_verify (hash_id, &native_list, NULL);

    /* Reset the IPV4 ECMP Hash object */
    switch_attr.id        = SAI_SWITCH_ATTR_ECMP_HASH_IPV4;
    switch_attr.value.oid = SAI_NULL_OBJECT_ID;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the Hash object */
    status = sai_test_hash_remove (hash_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates IPV6 Hash object create with 5 tuple native fields.
 * Verifies that the hash object cannot be removed when set in Switch attr.
 */
TEST_F (saiHashTest, ipv6_ecmp_hash_object)
{
    sai_status_t       status;
    sai_attribute_t    switch_attr;
    sai_object_id_t    hash_id;
    const unsigned int attr_count = 1;
    const unsigned int field_count = 5;
    int32_t            field_list [field_count] =
    { SAI_NATIVE_HASH_FIELD_SRC_IP, SAI_NATIVE_HASH_FIELD_DST_IP,
      SAI_NATIVE_HASH_FIELD_L4_SRC_PORT, SAI_NATIVE_HASH_FIELD_L4_DST_PORT,
      SAI_NATIVE_HASH_FIELD_IP_PROTOCOL };
    sai_s32_list_t     native_list = {field_count, field_list};

    status = sai_test_hash_create (&hash_id, &native_list, NULL, attr_count,
                                   SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Hash object fields */
    sai_test_hash_verify (hash_id, &native_list, NULL);

    /* Set the IPV4 ECMP Hash object id value */
    switch_attr.id        = SAI_SWITCH_ATTR_ECMP_HASH_IPV6;
    switch_attr.value.oid = hash_id;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Get the IPV4 ECMP Hash object id value */
    switch_attr.value.oid = SAI_NULL_OBJECT_ID;
    switch_attr.id = SAI_SWITCH_ATTR_ECMP_HASH_IPV6;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,attr_count, &switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    EXPECT_EQ (switch_attr.value.oid, hash_id);

    /* Verify that the hash object cannot be removed now */
    status = sai_test_hash_remove (hash_id);

    EXPECT_EQ (SAI_STATUS_OBJECT_IN_USE, status);

    /* Reset the IPV4 ECMP Hash object */
    switch_attr.id        = SAI_SWITCH_ATTR_ECMP_HASH_IPV6;
    switch_attr.value.oid = SAI_NULL_OBJECT_ID;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify IPV4 ECMP Hash object is set to NULL value */
    switch_attr.id = SAI_SWITCH_ATTR_ECMP_HASH_IPV6;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,attr_count, &switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    EXPECT_EQ (switch_attr.value.oid, SAI_NULL_OBJECT_ID);

    /* Remove the Hash object */
    status = sai_test_hash_remove (hash_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates setting new fields to IPV6 Hash object when it is set
 * in switch attribute
 */
TEST_F (saiHashTest, ipv6_ecmp_hash_object_set_attr)
{
    sai_status_t       status;
    sai_attribute_t    switch_attr;
    sai_object_id_t    hash_id;
    const unsigned int attr_count = 1;
    const unsigned int field_count = 5;
    int32_t            field_list [field_count + 1] =
    { SAI_NATIVE_HASH_FIELD_SRC_IP, SAI_NATIVE_HASH_FIELD_DST_IP,
      SAI_NATIVE_HASH_FIELD_L4_SRC_PORT, SAI_NATIVE_HASH_FIELD_L4_DST_PORT,
      SAI_NATIVE_HASH_FIELD_IP_PROTOCOL };
    sai_s32_list_t     native_list = {field_count, field_list};

    status = sai_test_hash_create (&hash_id, &native_list, NULL, attr_count,
                                   SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Hash object fields */
    sai_test_hash_verify (hash_id, &native_list, NULL);

    /* Set the IPV4 ECMP Hash object id value */
    switch_attr.id        = SAI_SWITCH_ATTR_ECMP_HASH_IPV6;
    switch_attr.value.oid = hash_id;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Set the new hash field SAI_NATIVE_HASH_FIELD_IN_PORT */
    native_list.count = field_count + 1;
    native_list.list [field_count] = SAI_NATIVE_HASH_FIELD_IN_PORT;

    status = sai_test_hash_attr_set (hash_id, &native_list, NULL,
                                     SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Hash object fields include the new field */
    sai_test_hash_verify (hash_id, &native_list, NULL);

    /* Reset the IPV4 ECMP Hash object */
    switch_attr.id        = SAI_SWITCH_ATTR_ECMP_HASH_IPV6;
    switch_attr.value.oid = SAI_NULL_OBJECT_ID;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the Hash object */
    status = sai_test_hash_remove (hash_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates new Hash object create and remove with UDF fields
 */
TEST_F (saiHashTest, hash_object_create_remove_udf_fields)
{
    sai_status_t       status;
    sai_object_id_t    hash_id;
    const unsigned int attr_count = 1;
    const unsigned int udf_count = 2;
    sai_object_id_t    udf_group_id [udf_count];
    sai_object_list_t  udf_group_list = {udf_count, udf_group_id};
    unsigned int       udf_length = 2;

    for (unsigned int count = 0; count < udf_count; count++)
    {
        /* Create UDF Hash UDF Group */
        status = saiUdfTest::sai_test_udf_group_create (&udf_group_id[count],
                                                dflt_udf_grp_attr_count,
                                                SAI_UDF_GROUP_ATTR_TYPE,
                                                SAI_UDF_GROUP_TYPE_HASH,
                                                SAI_UDF_GROUP_ATTR_LENGTH, udf_length);

        ASSERT_EQ (SAI_STATUS_SUCCESS, status);
    }

    status = sai_test_hash_create (&hash_id, NULL, &udf_group_list, attr_count,
                                   SAI_HASH_ATTR_UDF_GROUP_LIST);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Hash object fields */
    sai_test_hash_verify (hash_id, NULL, &udf_group_list);

    /* Remove the Hash object */
    status = sai_test_hash_remove (hash_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    for (unsigned int count = 0; count < udf_count; count++)
    {
        /* Remove the UDF Hash UDF Group */
        status = saiUdfTest::sai_test_udf_group_remove (udf_group_id[count]);

        EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    }
}

/*
 * Validates IPV4 Hash object create with UDF field matching SIP address offset.
 */
TEST_F (saiHashTest, ipv4_lag_hash_object_udf_field)
{
    sai_status_t       status;
    sai_attribute_t    switch_attr;
    sai_object_id_t    hash_id;
    const unsigned int attr_count = 1;
    const unsigned int udf_count = 1;
    sai_object_id_t    udf_group_id;
    sai_object_list_t  udf_group_list = {udf_count, &udf_group_id};
    unsigned int       udf_length = 2;
    sai_object_id_t    udf_match_id;
    sai_object_id_t    udf_id;

    /* Create UDF Match to match IP packets */
    status = saiUdfTest::sai_test_udf_match_create (&udf_match_id, attr_count,
                                                    SAI_UDF_MATCH_ATTR_L2_TYPE,
                                                    0x0800, 0xFFFF);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create UDF Hash UDF Group */
    status = saiUdfTest::sai_test_udf_group_create (&udf_group_id,
                                                    dflt_udf_grp_attr_count,
                                                    SAI_UDF_GROUP_ATTR_TYPE,
                                                    SAI_UDF_GROUP_TYPE_HASH,
                                                    SAI_UDF_GROUP_ATTR_LENGTH,
                                                    udf_length);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Create UDF to match SIP offset in IP packets */
    status = saiUdfTest::sai_test_udf_create (&udf_id, NULL, 3,
                                              SAI_UDF_ATTR_MATCH_ID, udf_match_id,
                                              SAI_UDF_ATTR_GROUP_ID, udf_group_id,
                                              SAI_UDF_ATTR_OFFSET, 28);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_hash_create (&hash_id, NULL, &udf_group_list, attr_count,
                                   SAI_HASH_ATTR_UDF_GROUP_LIST);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Hash object fields */
    sai_test_hash_verify (hash_id, NULL, &udf_group_list);

    /* Set the IPV4 LAG Hash object id value */
    switch_attr.id        = SAI_SWITCH_ATTR_LAG_HASH_IPV4;
    switch_attr.value.oid = hash_id;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Set the IPV4 ECMP Hash object id value */
    switch_attr.id        = SAI_SWITCH_ATTR_ECMP_HASH_IPV4;
    switch_attr.value.oid = hash_id;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Get the IPV4 LAG Hash object id value */
    switch_attr.value.oid = SAI_NULL_OBJECT_ID;
    switch_attr.id = SAI_SWITCH_ATTR_LAG_HASH_IPV4;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,attr_count, &switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    EXPECT_EQ (switch_attr.value.oid, hash_id);

    /* Reset the IPV4 ECMP Hash object */
    switch_attr.id        = SAI_SWITCH_ATTR_ECMP_HASH_IPV4;
    switch_attr.value.oid = SAI_NULL_OBJECT_ID;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Reset the IPV4 LAG Hash object */
    switch_attr.id        = SAI_SWITCH_ATTR_LAG_HASH_IPV4;
    switch_attr.value.oid = SAI_NULL_OBJECT_ID;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the Hash object */
    status = sai_test_hash_remove (hash_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the UDF object */
    status = saiUdfTest::sai_test_udf_remove (udf_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the UDF Match object */
    status = saiUdfTest::sai_test_udf_match_remove (udf_match_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the UDF Hash UDF Group */
    status = saiUdfTest::sai_test_udf_group_remove (udf_group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates setting and removing UDF hash field in Default ECMP Hash object
 */
TEST_F (saiHashTest, default_ecmp_udf_hash_fields_set)
{
    sai_attribute_t    switch_attr;
    sai_status_t       status;
    const unsigned int count = 1;
    const unsigned int field_count = 0;
    sai_s32_list_t     native_list = {field_count, NULL};
    const unsigned int udf_count = 1;
    sai_object_id_t    udf_group_id;
    sai_object_list_t  udf_group_list = {udf_count, &udf_group_id};
    unsigned int       udf_length = 4;

    /* Get the default ECMP Hash object id value */
    switch_attr.id = SAI_SWITCH_ATTR_ECMP_HASH;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &switch_attr);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the native fields list in the Hash object */
    status = sai_test_hash_attr_set (switch_attr.value.oid, &native_list, NULL,
                                     SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Set the UDF Hash fields */
    status = saiUdfTest::sai_test_udf_group_create (
                                        &udf_group_id, dflt_udf_grp_attr_count,
                                        SAI_UDF_GROUP_ATTR_TYPE,
                                        SAI_UDF_GROUP_TYPE_HASH,
                                        SAI_UDF_GROUP_ATTR_LENGTH, udf_length);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_hash_attr_set (switch_attr.value.oid, NULL, &udf_group_list,
                                     SAI_HASH_ATTR_UDF_GROUP_LIST);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF fields are set in default Hash object */
    sai_test_hash_verify (switch_attr.value.oid, NULL, &udf_group_list);

    /* Remove the UDF Hash fields now */
    udf_group_list.count    = 0;
    status = sai_test_hash_attr_set (switch_attr.value.oid, NULL, &udf_group_list,
                                     SAI_HASH_ATTR_UDF_GROUP_LIST);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the UDF fields are removed in default Hash object */
    sai_test_hash_verify (switch_attr.value.oid, &native_list, &udf_group_list);

    native_list.count = default_native_fld_count;
    native_list.list  = default_native_fields;

    /* ReSet the native fields list with default values */
    status = sai_test_hash_attr_set (switch_attr.value.oid, &native_list, NULL,
                                     SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the UDF Group now */
    status = saiUdfTest::sai_test_udf_group_remove (udf_group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates Hash object created with both Native and UDF fields
 * and can be set in Switch attr. Verifies removing the hash fields as well.
 */
TEST_F (saiHashTest, ipv4_hash_object_native_and_udf_field)
{
    sai_status_t       status;
    sai_attribute_t    switch_attr;
    sai_object_id_t    hash_id;
    const unsigned int attr_count = 2;
    const unsigned int udf_count = 1;
    sai_object_id_t    udf_group_id;
    sai_object_list_t  udf_group_list = {udf_count, &udf_group_id};
    unsigned int       udf_length = 4;
    const unsigned int field_count = 5;
    int32_t            field_list [field_count + 1] =
    { SAI_NATIVE_HASH_FIELD_SRC_IP, SAI_NATIVE_HASH_FIELD_DST_IP,
      SAI_NATIVE_HASH_FIELD_L4_SRC_PORT, SAI_NATIVE_HASH_FIELD_L4_DST_PORT,
      SAI_NATIVE_HASH_FIELD_IP_PROTOCOL };
    sai_s32_list_t     native_list = {field_count, field_list};

    /* Create UDF Hash UDF Group */
    status = saiUdfTest::sai_test_udf_group_create (&udf_group_id,
                                                    dflt_udf_grp_attr_count,
                                                    SAI_UDF_GROUP_ATTR_TYPE,
                                                    SAI_UDF_GROUP_TYPE_HASH,
                                                    SAI_UDF_GROUP_ATTR_LENGTH,
                                                    udf_length);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_hash_create (&hash_id, &native_list, &udf_group_list,
                                   attr_count,
                                   SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST,
                                   SAI_HASH_ATTR_UDF_GROUP_LIST);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Hash object fields */
    sai_test_hash_verify (hash_id, &native_list, &udf_group_list);

    /* Set the IPV4 LAG Hash object id value */
    switch_attr.id        = SAI_SWITCH_ATTR_LAG_HASH_IPV4;
    switch_attr.value.oid = hash_id;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the UDF Fields */
    udf_group_list.count = 0;
    status = sai_test_hash_attr_set (hash_id, NULL, &udf_group_list,
                                     SAI_HASH_ATTR_UDF_GROUP_LIST);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the Native fields */
    native_list.count = 0;
    status = sai_test_hash_attr_set (hash_id, &native_list, NULL,
                                     SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify the Hash object fields */
    sai_test_hash_verify (hash_id, &native_list, &udf_group_list);

    /* Reset the IPV4 LAG Hash object */
    switch_attr.id        = SAI_SWITCH_ATTR_LAG_HASH_IPV4;
    switch_attr.value.oid = SAI_NULL_OBJECT_ID;

    status = switch_api_tbl_get()->set_switch_attribute (switch_id,&switch_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the Hash object */
    status = sai_test_hash_remove (hash_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the UDF Hash UDF Group */
    status = saiUdfTest::sai_test_udf_group_remove (udf_group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates Invalid UDF Group objects are not allowed in Hash object
 * UDF Group list
 */
TEST_F (saiHashTest, hash_object_invalid_udf_group_list)
{
    sai_status_t       status;
    sai_object_id_t    hash_id;
    const unsigned int attr_count = 1;
    const unsigned int udf_count = 1;
    sai_object_id_t    udf_group_id;
    sai_object_list_t  udf_group_list = {udf_count, &udf_group_id};
    unsigned int       udf_length = 2;

    /* Create UDF Group of type Generic */
    status = saiUdfTest::sai_test_udf_group_create (
                                        &udf_group_id, dflt_udf_grp_attr_count,
                                        SAI_UDF_GROUP_ATTR_TYPE,
                                        SAI_UDF_GROUP_TYPE_GENERIC,
                                        SAI_UDF_GROUP_ATTR_LENGTH, udf_length);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    status = sai_test_hash_create (&hash_id, NULL, &udf_group_list, attr_count,
                                   SAI_HASH_ATTR_UDF_GROUP_LIST);

    /* Verify Hash creation fails */
    EXPECT_NE (SAI_STATUS_SUCCESS, status);

    /* Remove the UDF Group object */
    status = saiUdfTest::sai_test_udf_group_remove (udf_group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    udf_group_id = SAI_NULL_OBJECT_ID;

    /* Create with NULL object id */
    status = sai_test_hash_create (&hash_id, NULL, &udf_group_list, attr_count,
                                   SAI_HASH_ATTR_UDF_GROUP_LIST);

    /* Verify Hash creation fails */
    EXPECT_NE (SAI_STATUS_SUCCESS, status);

    /* Create UDF Group of type Hash */
    status = saiUdfTest::sai_test_udf_group_create (
                                        &udf_group_id, dflt_udf_grp_attr_count,
                                        SAI_UDF_GROUP_ATTR_TYPE,
                                        SAI_UDF_GROUP_TYPE_GENERIC,
                                        SAI_UDF_GROUP_ATTR_LENGTH, udf_length);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Remove the UDF Group object */
    status = saiUdfTest::sai_test_udf_group_remove (udf_group_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    /* Verify Hash creation fails for the non-existing group id */
    status = sai_test_hash_create (&hash_id, NULL, &udf_group_list, attr_count,
                                   SAI_HASH_ATTR_UDF_GROUP_LIST);

    /* Verify Hash creation fails */
    EXPECT_NE (SAI_STATUS_SUCCESS, status);
}

/*
 * Validates Get API returns Buffer Overflow for Hash field list
 */
TEST_F (saiHashTest, hash_field_list_get_buffer_overflow)
{
    sai_attribute_t    switch_attr;
    sai_status_t       status;
    const unsigned int count = 1;
    const unsigned int attr_count = 1;
    const unsigned int field_count = default_native_fld_count;
    sai_attribute_t    attr_list [attr_count];

    /* Get the default ECMP Hash object id value */
    switch_attr.id = SAI_SWITCH_ATTR_ECMP_HASH;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &switch_attr);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    /* Get the default Hash object fields */
    attr_list[0].value.s32list.count = 0;
    status = sai_test_hash_attr_get (switch_attr.value.oid, attr_list, attr_count,
                                     SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST);

    ASSERT_EQ (SAI_STATUS_BUFFER_OVERFLOW, status);

    EXPECT_EQ (attr_list[0].value.s32list.count, field_count);
}

/*
 * Validates default values for Hash algorithm and Hash seed attributes
 */
TEST_F (saiHashTest, default_hash_seed_algorithm_get)
{
    sai_attribute_t  get_attr;
    const unsigned int count = 1;
    sai_status_t       status;

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    EXPECT_EQ (get_attr.value.s32, SAI_HASH_ALGORITHM_CRC);

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    EXPECT_EQ (get_attr.value.s32, SAI_HASH_ALGORITHM_CRC);

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    EXPECT_EQ (get_attr.value.u32, 0);

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED;

    status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    EXPECT_EQ (get_attr.value.u32, 0);
}

/*
 * Validates set and get for Hash algorithm and Hash seed attributes
 */
TEST_F (saiHashTest, default_hash_seed_algorithm_set)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr;
    const unsigned int hash_attr_count = 2;
    unsigned int       count = 1;
    sai_status_t       status;
    unsigned int       index;

    sai_attr_id_t   algorithm_attr [hash_attr_count] = {
        SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM,
        SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM,
    };
    sai_attr_id_t   seed_attr [hash_attr_count] = {
        SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED,
        SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED,
    };

    memset(&get_attr, 0, sizeof(get_attr));
    memset(&set_attr, 0, sizeof(set_attr));

    for (index = 0; index < hash_attr_count; index++) {

        set_attr.id = get_attr.id = algorithm_attr [index];

        set_attr.value.s32 = SAI_HASH_ALGORITHM_CRC;
        status = switch_api_tbl_get()->set_switch_attribute (switch_id,&set_attr);
        EXPECT_EQ (SAI_STATUS_SUCCESS, status);

        status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &get_attr);
        EXPECT_EQ (get_attr.value.s32, SAI_HASH_ALGORITHM_CRC);

        set_attr.value.s32 = SAI_HASH_ALGORITHM_XOR;
        status = switch_api_tbl_get()->set_switch_attribute (switch_id,&set_attr);
        EXPECT_EQ (SAI_STATUS_SUCCESS, status);

        status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &get_attr);
        EXPECT_EQ (get_attr.value.s32, SAI_HASH_ALGORITHM_XOR);

        set_attr.value.s32 = SAI_HASH_ALGORITHM_RANDOM;
        status = switch_api_tbl_get()->set_switch_attribute (switch_id,&set_attr);
        EXPECT_EQ (SAI_STATUS_SUCCESS, status);

        /* Reset back to default value */
        set_attr.value.s32 = SAI_HASH_ALGORITHM_CRC;
        status = switch_api_tbl_get()->set_switch_attribute (switch_id,&set_attr);
        EXPECT_EQ (SAI_STATUS_SUCCESS, status);

        status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &get_attr);
        EXPECT_EQ (get_attr.value.s32, SAI_HASH_ALGORITHM_CRC);

        set_attr.id = get_attr.id = seed_attr [index];
        set_attr.value.u32 = 12345;
        status = switch_api_tbl_get()->set_switch_attribute (switch_id,&set_attr);
        EXPECT_EQ (SAI_STATUS_SUCCESS, status);

        status = switch_api_tbl_get()->get_switch_attribute (switch_id,count, &get_attr);
        EXPECT_EQ (get_attr.value.u32, 12345);

        /* Reset back to default value */
        set_attr.value.u32 = 0;
        status = switch_api_tbl_get()->set_switch_attribute (switch_id,&set_attr);
        EXPECT_EQ (SAI_STATUS_SUCCESS, status);
    }
}

int main (int argc, char **argv)
{
    ::testing::InitGoogleTest (&argc, argv);

    return RUN_ALL_TESTS ();
}
