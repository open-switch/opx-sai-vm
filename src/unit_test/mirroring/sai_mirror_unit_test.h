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
 * filename: sai_mirror_unit_test.h
 */

#ifndef __SAI_MIRROR_UNIT_TEST_H__
#define __SAI_MIRROR_UNIT_TEST_H__

#include "gtest/gtest.h"

extern "C" {
#include "saiswitch.h"
#include "saimirror.h"
#include "saiacl.h"
#include "saivlan.h"
#include "sailag.h"
#include "saitypes.h"
#include "std_type_defs.h"
}

#define SAI_MIRROR_VLAN_2 2
#define SAI_MIRROR_VLAN_3 3

class mirrorTest : public ::testing::Test
{
    public:
        static void SetUpTestCase();


        static sai_switch_api_t         *p_sai_switch_api_tbl;
        static sai_mirror_api_t         *p_sai_mirror_api_tbl;
        static sai_port_api_t           *p_sai_port_api_tbl;
        static sai_acl_api_t            *p_sai_acl_api_tbl;
        static sai_vlan_api_t           *p_sai_vlan_api_tbl;
        static sai_lag_api_t            *p_sai_lag_api_table;
        static sai_object_id_t           sai_mirror_first_port;
        static sai_object_id_t           sai_mirror_second_port;
        static sai_object_id_t           sai_mirror_third_port;
        static sai_object_id_t           sai_monitor_port;
        static sai_object_id_t           sai_second_monitor_port;
        static sai_object_id_t           sai_invalid_port;
        static sai_object_id_t           sai_mirror_vlan_2;
        static sai_object_id_t           sai_mirror_vlan_3;
        static sai_object_id_t           switch_id;

        static sai_object_id_t          sai_mirror_port_id_get (uint32_t port_index);
        static sai_object_id_t          sai_mirror_invalid_port_id_get ();

        static sai_status_t sai_test_mirror_session_create (sai_object_id_t *p_session_id,
                uint32_t attr_count,
                sai_attribute_t *p_attr_list);
        static sai_status_t sai_test_mirror_session_ingress_port_add (sai_object_id_t port_id,
        sai_object_list_t *session_list);
        static sai_status_t sai_test_mirror_session_egress_port_add (sai_object_id_t port_id,
        sai_object_list_t *session_list);
        static sai_status_t sai_test_mirror_session_ingress_port_get (sai_object_id_t port_id,
        sai_object_list_t *session_list);
        static sai_status_t sai_test_mirror_session_egress_port_get (sai_object_id_t port_id,
        sai_object_list_t *session_list);
        static sai_status_t sai_test_mirror_session_destroy (sai_object_id_t session_id);
        void sai_test_mirror_port_breakout (sai_object_id_t session_id);

        static void TearDownTestCase();
};
#endif /* __SAI_MIRROR_UNIT_TEST_H__ */
