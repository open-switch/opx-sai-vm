/*
 * filename: std_cfg_file_gtest.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/*
 * sai_infra_unit_test.cpp
 *
 */


#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"

extern "C" {
#include "sai.h"
}
/*
 *Obtain the method table for the sai switch api.
 */
TEST(sai_unit_test, api_query)
{
    sai_switch_api_t *sai_switch_api_table = NULL;

    ASSERT_EQ(NULL,sai_api_query(SAI_API_SWITCH,
              (static_cast<void**>(static_cast<void*>(&sai_switch_api_table)))));

    ASSERT_TRUE(sai_switch_api_table != NULL);

    EXPECT_TRUE(sai_switch_api_table->create_switch != NULL);
    EXPECT_TRUE(sai_switch_api_table->remove_switch != NULL);
    EXPECT_TRUE(sai_switch_api_table->set_switch_attribute != NULL);
    EXPECT_TRUE(sai_switch_api_table->get_switch_attribute != NULL);

}

/*
 *Verify if object_type_query returns OBJECT_TYPE_NULL for invalid object id.
 */
TEST(sai_unit_test, sai_object_type_query)
{
    sai_object_id_t  invalid_obj_id = 0;

    ASSERT_EQ(SAI_OBJECT_TYPE_NULL,sai_object_type_query(invalid_obj_id));
}

/*
 * Unintialize the SDK and free up the resources.
 */
TEST(sai_unit_test, api_uninit)
{
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_api_uninitialize());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

