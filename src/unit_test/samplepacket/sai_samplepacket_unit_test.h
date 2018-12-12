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
 * filename: sai_samplepacket_unit_test.h
 */

#ifndef __SAI_SAMPLEPACKET_UNIT_TEST_H__
#define __SAI_SAMPLEPACKET_UNIT_TEST_H__

#include "gtest/gtest.h"

extern "C" {
#include "saiswitch.h"
#include "saiacl.h"
#include "saisamplepacket.h"
#include "saitypes.h"
}

class samplepacketTest : public ::testing::Test
{
    public:
        static void SetUpTestCase();


        static sai_switch_api_t         *p_sai_switch_api_tbl;
        static sai_samplepacket_api_t         *p_sai_samplepacket_api_tbl;
        static sai_port_api_t           *p_sai_port_api_tbl;
        static sai_acl_api_t           *p_sai_acl_api_tbl;
        static sai_status_t sai_test_samplepacket_session_create (sai_object_id_t *p_session_id,
                uint32_t attr_count,
                sai_attribute_t *p_attr_list);
        static sai_status_t sai_test_samplepacket_session_ingress_port_add (sai_object_id_t port_id,
        sai_object_id_t sample_object);
        static sai_status_t sai_test_samplepacket_session_egress_port_add (sai_object_id_t port_id,
        sai_object_id_t sample_object);
        static sai_status_t sai_test_samplepacket_session_ingress_port_get (sai_object_id_t port_id,
        sai_object_id_t sample_object);
        static sai_status_t sai_test_samplepacket_session_egress_port_get (sai_object_id_t port_id,
        sai_object_id_t sample_object);
        static sai_status_t sai_test_samplepacket_session_destroy (sai_object_id_t session_id);
        static void sai_test_samplepacket_port_breakout (void);
        static sai_object_id_t port_id_1;
        static sai_object_id_t port_id_2;
        static sai_object_id_t switch_id;

};
#endif /* __SAI_SAMPLEPACKET_UNIT_TEST_H__ */
