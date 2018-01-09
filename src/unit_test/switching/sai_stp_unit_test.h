/*
 * filename: sai_stp_unit_test.h
 * (c) Copyright 2015 Dell Inc. All Rights Reserved.
 */

#ifndef __SAI_STP_UNIT_TEST_H__
#define __SAI_STP_UNIT_TEST_H__

#include "gtest/gtest.h"

extern "C" {
#include "saiswitch.h"
#include "saivlan.h"
#include "saistp.h"
#include "saitypes.h"
#include "saibridge.h"
}

class stpTest : public ::testing::Test
{
    public:
        static void SetUpTestCase();

        static sai_object_id_t          sai_stp_bridge_port_id_get (uint32_t port_index);
        static sai_object_id_t          sai_stp_invalid_port_id_get ();

        static sai_switch_api_t         *p_sai_switch_api_tbl;
        static sai_stp_api_t            *p_sai_stp_api_tbl;
        static sai_vlan_api_t           *p_sai_vlan_api_tbl;
        static sai_bridge_api_t         *p_sai_bridge_api_tbl;
        static sai_object_id_t          sai_bridge_port_id_1;
        static sai_object_id_t          sai_invalid_port_id;

};
#endif /* __SAI_STP_UNIT_TEST_H__ */
