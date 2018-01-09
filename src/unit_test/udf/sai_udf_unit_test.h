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
* @file  sai_udf_unit_test_utils.h
*
* @brief This file contains class definition, utility and helper
*        function prototypes for testing the SAI UDF functionalities.
*
*************************************************************************/

#ifndef __SAI_UDF_UNIT_TEST_H__
#define __SAI_UDF_UNIT_TEST_H__

#include "gtest/gtest.h"

extern "C" {
#include "saitypes.h"
#include "saistatus.h"
#include "saiswitch.h"
#include "saiudf.h"
}

class saiUdfTest : public ::testing::Test
{
    public:
        /* Method to setup the UDF API table pointers. */
        static void sai_test_udf_api_table_get (void);

        /* Methods for UDF Group functionality SAI API testing. */
        static sai_status_t sai_test_udf_group_create (
                                               sai_object_id_t *udf_group_id,
                                               unsigned int attr_count, ...);
        static sai_status_t sai_test_udf_group_remove (
                                               sai_object_id_t udf_group_id);
        static sai_status_t sai_test_udf_group_get (sai_object_id_t udf_group_id,
                                                    sai_attribute_t *p_attr_list,
                                                    unsigned int attr_count, ...);
        static void sai_test_udf_group_verify (sai_object_id_t udf_group_id,
                                               sai_udf_group_type_t type,
                                               uint16_t udf_len,
                                               const sai_object_list_t *udf_list);

        /* Methods for UDF Match functionality SAI API testing. */
        static sai_status_t sai_test_udf_match_create (sai_object_id_t *match_id,
                                                       unsigned int attr_count, ...);
        static sai_status_t sai_test_udf_match_remove (sai_object_id_t match_id);
        static sai_status_t sai_test_udf_match_get (sai_object_id_t udf_match_id,
                                                    sai_attribute_t *p_attr_list,
                                                    unsigned int attr_count, ...);
        void sai_test_udf_match_verify (sai_object_id_t udf_match_id,
                                        unsigned int attr_count, ...);

        /* Methods for UDF functionality SAI API testing. */
        static sai_status_t sai_test_udf_create (sai_object_id_t *udf_id,
                                                 sai_u8_list_t *udf_hash_mask,
                                                 unsigned int attr_count, ...);
        static sai_status_t sai_test_udf_remove (sai_object_id_t udf_id);
        static sai_status_t sai_test_udf_get (sai_object_id_t udf_id,
                                              sai_attribute_t *p_attr_list,
                                              unsigned int attr_count, ...);
        static void sai_test_udf_verify (sai_object_id_t udf_id,
                                         sai_object_id_t match_id,
                                         sai_object_id_t group_id,
                                         sai_udf_base_t base, uint16_t offset);
        static void sai_test_udf_hash_mask_verify (sai_object_id_t udf_id,
                                                   sai_u8_list_t *hash_mask);

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

        /* Method for retrieving the UDF API table pointer */
        static inline sai_udf_api_t* udf_api_tbl_get (void)
        {
            return p_sai_udf_api_tbl;
        }

    protected:
        static void SetUpTestCase (void);

        static const unsigned int max_udf_grp_attr_count = 3;
        static const unsigned int dflt_udf_grp_attr_count = 2;
        static const unsigned int max_udf_match_attr_count = 4;
        static const unsigned int default_udf_match_attr_count = 0;
        static const unsigned int max_udf_attr_count = 5;
        static const unsigned int default_udf_attr_count = 3;

        static const unsigned int default_udf_len = 2; /* bytes */

        static sai_object_id_t   dflt_udf_group_id;

    private:

        static void sai_test_udf_api_attr_value_fill (sai_attribute_t *p_attr,
                                                      sai_u8_list_t *hash_mask,
                                                      unsigned long attr_val);
        static void sai_test_udf_api_attr_value_print (sai_attribute_t *p_attr);

        static void sai_test_udf_group_attr_value_fill (sai_attribute_t *p_attr,
                                                        unsigned long attr_val);
        static void sai_test_udf_group_attr_value_print (sai_attribute_t *p_attr);

        static void sai_test_udf_match_attr_value_fill (sai_attribute_t *p_attr,
                                                        unsigned int data,
                                                        unsigned int mask);
        static void sai_test_udf_match_attr_value_print (sai_attribute_t *p_attr);

        static bool sai_test_find_udf_id_in_list (sai_object_id_t udf_id,
                                       const sai_object_list_t *p_udf_list);

        static sai_switch_api_t *p_sai_switch_api_tbl;
        static sai_udf_api_t    *p_sai_udf_api_tbl;
};

#endif /* __SAI_UDF_UNIT_TEST_H__ */
