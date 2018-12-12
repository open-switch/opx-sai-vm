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

/*
 * filename: std_cfg_file_gtest.cpp
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

