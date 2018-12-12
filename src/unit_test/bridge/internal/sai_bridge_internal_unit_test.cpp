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
 * sai_bridge_internal_unit_test.cpp
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include <inttypes.h>

#include "sai_bridge_unit_test_utils.h"
#include "sai_bridge_unit_test.h"

extern "C" {
#include "sai.h"
#include "saitypes.h"
#include "saibridge.h"
#include "saiswitch.h"
#include "sai_bridge_main.h"
#include "sai_bridge_common.h"
}

static sai_object_id_t          cb_bridge_port_id = 0;
static sai_bridge_port_event_t  cb_event = SAI_BRIDGE_PORT_EVENT_CREATE;
static sai_object_id_t          cb_lag_id = 0;
static bool                     cb_lag_add_ports = false;
static uint32_t                 cb_lag_port_count = 0;
static sai_object_id_t          cb_lag_port_list[SAI_DEBUG_MAX_UT_PORTS] = {0};
static uint_t                   bridge_port_cb_count = 0;

static sai_status_t sai_bridge_port_notification_handler(sai_object_id_t bridge_port_id,
                                                       const sai_bridge_port_notif_t *data)
{
     cb_bridge_port_id = bridge_port_id;
     cb_event = data->event;
     if(cb_event == SAI_BRIDGE_PORT_EVENT_LAG_MODIFY) {

         cb_lag_id = data->lag_id;
         cb_lag_add_ports = data->lag_add_port;
         cb_lag_port_count = data->lag_port_mod_list->count;

         if(data->lag_port_mod_list->count > SAI_DEBUG_MAX_UT_PORTS) {
             printf("Error - max port allowed count is %d\r\n", SAI_DEBUG_MAX_UT_PORTS);
             return SAI_STATUS_FAILURE;
         }
         memcpy(cb_lag_port_list, data->lag_port_mod_list->list,
                 sizeof(sai_object_id_t)*(data->lag_port_mod_list->count));
     }
     bridge_port_cb_count++;
     return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_bridge_port_validate_params(sai_object_id_t bridge_port_id,
                                                    sai_bridge_port_event_t event,
                                                    sai_object_id_t lag_id,
                                                    bool lag_add_ports,
                                                    uint_t lag_port_count,
                                                    sai_object_id_t *lag_port_list)
{
    uint32_t port_idx = 0;

    if (bridge_port_id != cb_bridge_port_id) {
        return SAI_STATUS_FAILURE;
    }

    if (event != cb_event) {
        return SAI_STATUS_FAILURE;
    }

    if(event == SAI_BRIDGE_PORT_EVENT_LAG_MODIFY) {
        if(lag_id != cb_lag_id) {
            return SAI_STATUS_FAILURE;
        }
        if(lag_port_count != cb_lag_port_count) {
            return SAI_STATUS_FAILURE;
        }
        if(lag_add_ports != cb_lag_add_ports) {
            return SAI_STATUS_FAILURE;
        }
        for(port_idx = 0; port_idx < lag_port_count; port_idx++) {
            if(lag_port_list[port_idx] != cb_lag_port_list[port_idx]) {
                return SAI_STATUS_FAILURE;
            }
        }
    }
    return SAI_STATUS_SUCCESS;
}

TEST_F(bridgeTest, tests_bridge_create_1q_callbacks)
{
    uint32_t        type_bmp = SAI_BRIDGE_PORT_TYPE_TO_BMP(SAI_BRIDGE_PORT_TYPE_PORT);
    sai_object_id_t port_id;
    sai_object_id_t bridge_port_id;
    sai_attribute_t attr[3];

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_event_cb_register(SAI_MODULE_BRIDGE, type_bmp,
                                                sai_bridge_port_notification_handler));

    bridge_port_cb_count = 0;
    bridge_port_id = bridge_port_list[0];

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_get_port_from_bridge_port(p_sai_bridge_api_tbl, bridge_port_id,
                                                      &port_id));

    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_PORT;


    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = port_id;

    attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[2].value.oid = default_bridge_id;

    ASSERT_EQ(SAI_STATUS_ITEM_ALREADY_EXISTS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id, switch_id, 3, attr));

    /* Check if no callback for duplicate create*/
    ASSERT_EQ(bridge_port_cb_count, 0);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               bridge_port_list[0], true));
    ASSERT_EQ(bridge_port_cb_count, 1);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(bridge_port_list[0], SAI_BRIDGE_PORT_EVENT_REMOVE,
                                              0, 0, 0, NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id,
                                               true, &bridge_port_id));

    ASSERT_EQ(bridge_port_cb_count, 2);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(bridge_port_id, SAI_BRIDGE_PORT_EVENT_CREATE,
                                              0, 0, 0, NULL));

    bridge_port_list[0] = bridge_port_id;
}

TEST_F(bridgeTest, tests_bridge_create_1d_callbacks)
{
    uint32_t        type_bmp = SAI_BRIDGE_PORT_TYPE_TO_BMP(SAI_BRIDGE_PORT_TYPE_SUB_PORT);
    sai_object_id_t port_id = port_list[0];
    sai_vlan_id_t   vlan_id = SAI_BRIDGE_GTEST_VLAN;
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t bridge_port_id_1 = SAI_NULL_OBJECT_ID;
    sai_attribute_t attr[4];
    sai_object_id_t bridge_id = SAI_NULL_OBJECT_ID;

    bridge_port_cb_count = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_event_cb_register(SAI_MODULE_BRIDGE, type_bmp,
                                                sai_bridge_port_notification_handler));


    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &attr[0]));


    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = port_id;


    attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[2].value.oid = bridge_id;

    attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr[3].value.u16 = vlan_id;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id, switch_id, 4, attr));
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           port_id, bridge_port_id, vlan_id);

    ASSERT_EQ(bridge_port_cb_count, 1);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(bridge_port_id, SAI_BRIDGE_PORT_EVENT_CREATE,
                                              0, 0, 0, NULL));

    ASSERT_EQ(SAI_STATUS_ITEM_ALREADY_EXISTS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id_1, switch_id, 4, attr));

    ASSERT_EQ(bridge_port_cb_count, 1);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));

    ASSERT_EQ(bridge_port_cb_count, 2);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(bridge_port_id, SAI_BRIDGE_PORT_EVENT_REMOVE,
                                              0, 0, 0, NULL));

    ASSERT_EQ(SAI_STATUS_INVALID_OBJECT_ID,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));

    ASSERT_EQ(bridge_port_cb_count, 2);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));
}

/* Register for a type and make sure don't get callbacks for other types */

TEST_F(bridgeTest, tests_bridge_port_type_callback)
{
    uint32_t        type_bmp = SAI_BRIDGE_PORT_TYPE_TO_BMP(SAI_BRIDGE_PORT_TYPE_PORT);
    sai_object_id_t port_id = port_list[0];
    sai_vlan_id_t   vlan_id = SAI_BRIDGE_GTEST_VLAN;
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_attribute_t attr[4];
    sai_object_id_t bridge_id = SAI_NULL_OBJECT_ID;

    bridge_port_cb_count = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_event_cb_register(SAI_MODULE_BRIDGE, type_bmp,
                                                sai_bridge_port_notification_handler));


    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &attr[0]));


    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = port_id;


    attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[2].value.oid = bridge_id;

    attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr[3].value.u16 = vlan_id;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id, switch_id, 4, attr));
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           port_id, bridge_port_id, vlan_id);

    ASSERT_EQ(bridge_port_cb_count, 0);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));

    ASSERT_EQ(bridge_port_cb_count, 0);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));

}

TEST_F(bridgeTest, tests_multiple_bridge_port_type_register)
{
    uint32_t        type_bmp = ((SAI_BRIDGE_PORT_TYPE_TO_BMP(SAI_BRIDGE_PORT_TYPE_PORT)) |
                               (SAI_BRIDGE_PORT_TYPE_TO_BMP(SAI_BRIDGE_PORT_TYPE_SUB_PORT)));
    sai_object_id_t port_id = port_list[0];
    sai_vlan_id_t   vlan_id = SAI_BRIDGE_GTEST_VLAN;
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_attribute_t attr[4];
    sai_object_id_t bridge_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t bridge_1q_port_id;
    sai_object_id_t port_1q_id;

    bridge_1q_port_id = bridge_port_list[0];

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_get_port_from_bridge_port(p_sai_bridge_api_tbl, bridge_1q_port_id,
                                                      &port_1q_id));

    bridge_port_cb_count = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_event_cb_register(SAI_MODULE_BRIDGE, type_bmp,
                                                sai_bridge_port_notification_handler));

    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &attr[0]));


    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = port_id;


    attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[2].value.oid = bridge_id;

    attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr[3].value.u16 = vlan_id;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id, switch_id, 4, attr));
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           port_id, bridge_port_id, vlan_id);

    ASSERT_EQ(bridge_port_cb_count, 1);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(bridge_port_id, SAI_BRIDGE_PORT_EVENT_CREATE,
                                              0, 0, 0, NULL));

    ASSERT_EQ(bridge_port_cb_count, 1);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));

    ASSERT_EQ(bridge_port_cb_count, 2);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(bridge_port_id, SAI_BRIDGE_PORT_EVENT_REMOVE,
                                              0, 0, 0, NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               bridge_port_list[0], true));

    ASSERT_EQ(bridge_port_cb_count, 3);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(bridge_port_list[0], SAI_BRIDGE_PORT_EVENT_REMOVE,
                                              0, 0, 0, NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_1q_id,
                                               true, &bridge_1q_port_id));
    ASSERT_EQ(bridge_port_cb_count, 4);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(bridge_1q_port_id, SAI_BRIDGE_PORT_EVENT_CREATE,
                                              0, 0, 0, NULL));

    bridge_port_list[0] = bridge_1q_port_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));

}

TEST_F(bridgeTest, lag_bridge_port_callbacks)
{
    uint32_t        type_bmp = SAI_BRIDGE_PORT_TYPE_TO_BMP(SAI_BRIDGE_PORT_TYPE_PORT);
    sai_object_id_t lag_id;
    sai_object_id_t port_id1;
    sai_object_id_t port_id2;
    sai_object_id_t lag_1d_bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t lag_1q_bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t bridge_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t lag_member_id1;
    sai_object_id_t lag_member_id2;
    sai_vlan_id_t   vlan_id = SAI_BRIDGE_GTEST_VLAN;
    sai_attribute_t attr[4];

    bridge_port_cb_count = 0;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_event_cb_register(SAI_MODULE_BRIDGE, type_bmp,
                                                sai_bridge_port_notification_handler));

    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &attr[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_get_port_from_bridge_port(p_sai_bridge_api_tbl, bridge_port_list[0],
                                                      &port_id1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_get_port_from_bridge_port(p_sai_bridge_api_tbl, bridge_port_list[1],
                                                      &port_id2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               bridge_port_list[0], true));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(bridge_port_list[0], SAI_BRIDGE_PORT_EVENT_REMOVE,
                                              0, 0, 0, NULL));
    ASSERT_EQ(bridge_port_cb_count, 1);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               bridge_port_list[1], true));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(bridge_port_list[1], SAI_BRIDGE_PORT_EVENT_REMOVE,
                                              0, 0, 0, NULL));
    ASSERT_EQ(bridge_port_cb_count, 2);


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->create_lag(&lag_id, switch_id, 0, NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_id,
                                               true, &lag_1q_bridge_port_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(lag_1q_bridge_port_id, SAI_BRIDGE_PORT_EVENT_CREATE,
                                              0, 0, 0, NULL));

    ASSERT_EQ(bridge_port_cb_count, 3);

    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = lag_id;

    attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[2].value.oid = bridge_id;

    attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr[3].value.u16 = vlan_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&lag_1d_bridge_port_id, switch_id, 4, attr));
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           lag_id, lag_1d_bridge_port_id, vlan_id);


    ASSERT_EQ(bridge_port_cb_count, 3);

    attr[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attr[0].value.oid = lag_id;

    attr[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attr[1].value.oid = port_id1;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->create_lag_member (&lag_member_id1, switch_id, 2, attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(lag_1q_bridge_port_id, SAI_BRIDGE_PORT_EVENT_LAG_MODIFY,
                                              lag_id, true, 1, &port_id1));

    ASSERT_EQ(bridge_port_cb_count, 4);

    attr[1].value.oid = port_id2;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->create_lag_member (&lag_member_id2, switch_id, 2, attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(lag_1q_bridge_port_id, SAI_BRIDGE_PORT_EVENT_LAG_MODIFY,
                                              lag_id, true, 1, &port_id2));

    ASSERT_EQ(bridge_port_cb_count, 5);

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              p_sai_lag_api_tbl->remove_lag(lag_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->remove_lag_member(lag_member_id1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(lag_1q_bridge_port_id, SAI_BRIDGE_PORT_EVENT_LAG_MODIFY,
                                              lag_id, false, 1, &port_id1));

    ASSERT_EQ(bridge_port_cb_count, 6);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->remove_lag_member(lag_member_id2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(lag_1q_bridge_port_id, SAI_BRIDGE_PORT_EVENT_LAG_MODIFY,
                                              lag_id, false, 1, &port_id2));

    ASSERT_EQ(bridge_port_cb_count, 7);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id1,
                                               true, &bridge_port_list[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(bridge_port_list[0], SAI_BRIDGE_PORT_EVENT_CREATE,
                                              0, 0, 0, NULL));

    ASSERT_EQ(bridge_port_cb_count, 8);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id2,
                                               true, &bridge_port_list[1]));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(bridge_port_list[1], SAI_BRIDGE_PORT_EVENT_CREATE,
                                              0, 0, 0, NULL));

    ASSERT_EQ(bridge_port_cb_count, 9);

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              p_sai_lag_api_tbl->remove_lag(lag_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               lag_1q_bridge_port_id, true));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_validate_params(lag_1q_bridge_port_id, SAI_BRIDGE_PORT_EVENT_REMOVE,
                                              0, 0, 0, NULL));

    ASSERT_EQ(bridge_port_cb_count, 10);

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              p_sai_lag_api_tbl->remove_lag(lag_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(lag_1d_bridge_port_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->remove_lag(lag_id));

    ASSERT_EQ(bridge_port_cb_count, 10);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));

}

TEST_F(bridgeTest, test_lag_vlan_callback)
{
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_vlan_lag_callback_npu_dump());
}

TEST_F(bridgeTest, test_lag_stp_callback)
{
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_stp_lag_callback_npu_dump());
}
TEST_F(bridgeTest, test_lag_mcast_callback)
{
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_mcast_lag_callback_npu_dump());
}
TEST_F(bridgeTest, test_admin_state_change)
{
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_port_admin_state_change_npu_dump());
}

TEST_F(bridgeTest, 1d_multicast_flood_groups_internal)
{
    sai_object_id_t        port_id_1 = port_list[0];
    sai_object_id_t        port_id_2 = port_list[1];
    sai_vlan_id_t          vlan_id = SAI_BRIDGE_GTEST_VLAN;
    sai_object_id_t        bridge_port_id_1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t        bridge_port_id_2 = SAI_NULL_OBJECT_ID;
    sai_attribute_t        attr[4];
    sai_object_id_t        bridge_id_1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t        l2mc_obj_id_1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t        l2mc_obj_id_2 = SAI_NULL_OBJECT_ID;
    sai_object_id_t        l2mc_member_id = SAI_NULL_OBJECT_ID;
    unsigned int           idx = 0;


    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id_1, switch_id, 1, &attr[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->create_l2mc_group(&l2mc_obj_id_1,switch_id,0,attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->create_l2mc_group(&l2mc_obj_id_2,switch_id,0,attr));

    printf("++++++++++++++++\r\n");
    printf("+ Default dump +\r\n");
    printf("++++++++++++++++\r\n");

    sai_bridge_dump_multicast_info(bridge_id_1);

    printf("+++++++++++++++++++++++++++++++\r\n");
    printf("+ Setting flood group to none +\r\n");
    printf("+++++++++++++++++++++++++++++++\r\n");

    attr[0].id  = SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE;
    attr[1].id  = SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE;
    attr[2].id  = SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE;

    for(idx = 0; idx< 3; idx++) {
        attr[idx].value.s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_NONE;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[idx]));
    }
    sai_bridge_dump_multicast_info(bridge_id_1);

    printf("++++++++++++++++++++++++++++++++++++\r\n");
    printf("+ Setting flood group to sub ports +\r\n");
    printf("++++++++++++++++++++++++++++++++++++\r\n");

    attr[0].id  = SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE;
    attr[1].id  = SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE;
    attr[2].id  = SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE;

    for(idx = 0; idx< 3; idx++) {
        attr[idx].value.s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_SUB_PORTS;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[idx]));
    }
    sai_bridge_dump_multicast_info(bridge_id_1);


    printf("+++++++++++++++++++++++++++++++++++++\r\n");
    printf("+ Setting flood group to L2MC Group +\r\n");
    printf("+++++++++++++++++++++++++++++++++++++\r\n");

    attr[0].id  = SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE;
    attr[1].id  = SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE;
    attr[2].id  = SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE;

    for(idx = 0; idx< 3; idx++) {
        attr[idx].value.s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[idx]));
    }
    sai_bridge_dump_multicast_info(bridge_id_1);

    printf("++++++++++++++++++++++++++++++++++++++++++++++\r\n");
    printf("+ Adding L2MC group before flood type is set +\r\n");
    printf("++++++++++++++++++++++++++++++++++++++++++++++\r\n");

    attr[0].id = SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_SUB_PORTS;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    attr[0].id = SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP;
    attr[0].value.oid = l2mc_obj_id_1;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    sai_bridge_dump_multicast_info(bridge_id_1);
    attr[0].value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));
    printf("Removing the group\r\n");
    sai_bridge_dump_multicast_info(bridge_id_1);

    printf("+++++++++++++++++++++++++++++++++++++++++++++\r\n");
    printf("+ Adding L2MC group After flood type is set +\r\n");
    printf("+++++++++++++++++++++++++++++++++++++++++++++\r\n");

    attr[0].id = SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    attr[0].id = SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP;
    attr[0].value.oid = l2mc_obj_id_1;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    sai_bridge_dump_multicast_info(bridge_id_1);

    attr[0].value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    sai_bridge_dump_multicast_info(bridge_id_1);

    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
    printf("+ Adding L2MC group After Bridge port is created is set +\r\n");
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");

    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = port_id_1;

    attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[2].value.oid = bridge_id_1;

    attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr[3].value.u16 = vlan_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id_1, switch_id, 4, attr));
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           port_id_1, bridge_port_id_1, vlan_id);

    attr[1].value.oid = port_id_2;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id_2, switch_id, 4, attr));
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           port_id_2, bridge_port_id_2, vlan_id);

    attr[0].id = SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP;
    attr[0].value.oid = l2mc_obj_id_1;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));


    sai_bridge_dump_multicast_info(bridge_id_1);

    attr[0].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID;
    attr[0].value.oid = l2mc_obj_id_1;
    attr[1].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID;
    attr[1].value.oid = bridge_port_id_1;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->create_l2mc_group_member(&l2mc_member_id, switch_id,
                                                                 2, attr));

    printf("++++++++++++++++++++++++++++++++++++\r\n");
    printf("+ Adding member to mutlicast group +\r\n");
    printf("++++++++++++++++++++++++++++++++++++\r\n");

    sai_bridge_dump_multicast_info(bridge_id_1);

    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
    printf("+ Adding two different L2MC groups for different flood types +\r\n");
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");

    attr[0].id = SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP;
    attr[0].value.oid = l2mc_obj_id_2;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    sai_bridge_dump_multicast_info(bridge_id_1);

    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
    printf("+ Setting one of the flood control types to sub ports +\r\n");
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");

    attr[0].id = SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_SUB_PORTS;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    sai_bridge_dump_multicast_info(bridge_id_1);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->remove_l2mc_group_member(l2mc_member_id));

    printf("++++++++++++++++++++++++++++++++++++\r\n");
    printf("+ After Removing L2MC group member +\r\n");
    printf("++++++++++++++++++++++++++++++++++++\r\n");

    sai_bridge_dump_multicast_info(bridge_id_1);
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id_2));

    printf("++++++++++++++++++++++++++++++\r\n");
    printf("+ After removing bridge port +\r\n");
    printf("++++++++++++++++++++++++++++++\r\n");

    sai_bridge_dump_multicast_info(bridge_id_1);
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->remove_l2mc_group(l2mc_obj_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->remove_l2mc_group(l2mc_obj_id_2));

}

TEST_F(bridgeTest, 1d_multicast_flood_groups_on_lag)
{
    sai_object_id_t        port_id_1;
    sai_object_id_t        port_id_2;
    sai_vlan_id_t          vlan_id = SAI_BRIDGE_GTEST_VLAN;
    sai_object_id_t        bridge_port_id_1 = SAI_NULL_OBJECT_ID;
    sai_attribute_t        attr[4];
    sai_object_id_t        bridge_id_1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t        l2mc_obj_id_1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t        l2mc_member_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t        lag_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t        lag_member_id_1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t        lag_member_id_2 = SAI_NULL_OBJECT_ID;


    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id_1, switch_id, 1, &attr[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->create_l2mc_group(&l2mc_obj_id_1,switch_id,0,attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->create_lag(&lag_id,switch_id,0,NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_get_port_from_bridge_port(p_sai_bridge_api_tbl, bridge_port_list[0],
                                                      &port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_get_port_from_bridge_port(p_sai_bridge_api_tbl, bridge_port_list[1],
                                                      &port_id_2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               bridge_port_list[0], true));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               bridge_port_list[1], true));

    printf("++++++++++++++++\r\n");
    printf("+ Default dump +\r\n");
    printf("++++++++++++++++\r\n");

    sai_bridge_dump_multicast_info(bridge_id_1);

    attr[0].id  = SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE;

    attr[0].value.s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    attr[0].id = SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP;
    attr[0].value.oid = l2mc_obj_id_1;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = lag_id;

    attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[2].value.oid = bridge_id_1;

    attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr[3].value.u16 = vlan_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id_1, switch_id, 4, attr));
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           lag_id, bridge_port_id_1, vlan_id);

    attr[0].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID;
    attr[0].value.oid = l2mc_obj_id_1;
    attr[1].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID;
    attr[1].value.oid = bridge_port_id_1;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->create_l2mc_group_member(&l2mc_member_id, switch_id,
                                                                 2, attr));

    printf("++++++++++++++++++++++++++\r\n");
    printf("+ Adding member 1 to LAG +\r\n");
    printf("++++++++++++++++++++++++++\r\n");


    attr[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attr[0].value.oid = lag_id;

    attr[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attr[1].value.oid = port_id_1;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->create_lag_member (&lag_member_id_1, switch_id, 2, attr));


    sai_bridge_dump_multicast_info(bridge_id_1);

    printf("++++++++++++++++++++++++++\r\n");
    printf("+ Adding member 2 to LAG +\r\n");
    printf("++++++++++++++++++++++++++\r\n");


    attr[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attr[0].value.oid = lag_id;

    attr[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attr[1].value.oid = port_id_2;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->create_lag_member (&lag_member_id_2, switch_id, 2, attr));


    sai_bridge_dump_multicast_info(bridge_id_1);



    printf("++++++++++++++++++++++++++++++\r\n");
    printf("+ Removing member 1 from LAG +\r\n");
    printf("++++++++++++++++++++++++++++++\r\n");


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->remove_lag_member (lag_member_id_1));

    sai_bridge_dump_multicast_info(bridge_id_1);

    printf("++++++++++++++++++++++++++++++\r\n");
    printf("+ Removing member 2 from LAG +\r\n");
    printf("++++++++++++++++++++++++++++++\r\n");


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->remove_lag_member (lag_member_id_2));

    sai_bridge_dump_multicast_info(bridge_id_1);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->remove_l2mc_group_member(l2mc_member_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->remove_l2mc_group(l2mc_obj_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->remove_lag(lag_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id_1,
                                               true, &bridge_port_list[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id_2,
                                               true, &bridge_port_list[1]));
}

