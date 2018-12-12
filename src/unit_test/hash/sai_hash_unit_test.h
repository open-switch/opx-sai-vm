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
* @file  sai_hash_unit_test.h
*
* @brief This file contains class definition, utility and helper
*        function prototypes for testing the SAI Hash functionality.
*
*************************************************************************/

#ifndef __SAI_HASH_UNIT_TEST_H__
#define __SAI_HASH_UNIT_TEST_H__

#include "gtest/gtest.h"

extern "C" {
#include "saitypes.h"
#include "saistatus.h"
#include "saiswitch.h"
#include "saihash.h"
}

class saiHashTest : public ::testing::Test
{
    public:
        /* Method to setup the Hash API table pointers. */
        static void sai_test_hash_api_table_query (void);

        /* Methods for Hash functionality SAI API testing. */
        static sai_status_t sai_test_hash_create (
                                              sai_object_id_t *hash_id,
                                              sai_s32_list_t *native_fld_list,
                                              sai_object_list_t *udf_group_list,
                                              unsigned int attr_count, ...);
        static sai_status_t sai_test_hash_remove (sai_object_id_t hash_id);
        static sai_status_t sai_test_hash_attr_get (sai_object_id_t hash_id,
                                                    sai_attribute_t *p_attr_list,
                                                    unsigned int attr_count, ...);
        static sai_status_t sai_test_hash_attr_set (sai_object_id_t hash_id,
                                                    sai_s32_list_t *native_fld_list,
                                                    sai_object_list_t *udf_group_list,
                                                    sai_attr_id_t attr_id);
        static void sai_test_hash_verify (sai_object_id_t hash_id,
                                          const sai_s32_list_t *native_fld_list,
                                          const sai_object_list_t *udf_grp_list);
        static bool sai_test_find_native_field_in_list (
                                                 sai_int32_t field_id,
                                                 const sai_s32_list_t *p_field_list);
        static bool sai_test_find_udf_grp_id_in_list (
                                               sai_object_id_t udf_grp_id,
                                               const sai_object_list_t *p_udf_grp_list);

        /* Util for converting to attribute index based status code */
        static inline sai_status_t sai_test_invalid_attr_status_code (
                                                       sai_status_t status,
                                                       unsigned int attr_index)
        {
            return (status + SAI_STATUS_CODE (attr_index));
        }

        /* Method for retrieving the switch API table pointer */
        static inline sai_switch_api_t* switch_api_tbl_get (void)
        {
            return p_sai_switch_api_tbl;
        }

        /* Method for retrieving the HASH API table pointer */
        static inline sai_hash_api_t* hash_api_tbl_get (void)
        {
            return p_sai_hash_api_tbl;
        }

    protected:
        static void SetUpTestCase (void);

        static const unsigned int max_hash_attr_count = 2;
        static const unsigned int dflt_hash_attr_count = 1;
        static const unsigned int dflt_udf_grp_attr_count = 2;
        static const unsigned int default_native_fld_count = 4;
        static int32_t default_native_fields [default_native_fld_count];
        static const int max_native_fld_count = 12;
        static const char *native_fields_str [max_native_fld_count];

    private:

        static void sai_test_hash_attr_value_fill (sai_attribute_t *p_attr,
                                                   sai_s32_list_t *native_fld_list,
                                                   sai_object_list_t *udf_group_list);

        static void sai_test_hash_attr_value_print (sai_attribute_t *p_attr);

        static sai_object_id_t   default_lag_hash_obj;
        static sai_object_id_t   default_ecmp_hash_obj;

        static sai_switch_api_t *p_sai_switch_api_tbl;
        static sai_hash_api_t  *p_sai_hash_api_tbl;
};

#endif /* __SAI_HASH_UNIT_TEST_H__ */
