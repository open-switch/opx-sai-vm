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
 * sai_lag_unit_test_internal.cpp
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include <inttypes.h>

extern "C" {
#include "sai.h"
#include "sailag.h"
#include "saitypes.h"
#include "sai_lag_callback.h"
#include "sai_l2_unit_test_defs.h"
#include "sai_lag_unit_test.h"
#include "sai_l3_api_utils.h"
#include "sai_lag_debug.h"
}

static sai_object_id_t     g_lag_id;
static sai_object_id_t     g_rif_id;
static sai_object_list_t   g_port_list;
static sai_object_id_t     g_ports [SAI_MAX_PORTS];
static sai_lag_operation_t g_lag_operation;

static sai_status_t sai_lag_ut_rif_callback (sai_object_id_t          lag_id,
                                             sai_lag_operation_t      operation,
                                             const sai_object_list_t *port_list)
{
    sai_object_id_t          rif_id = 0;

    if((operation != SAI_LAG_OPER_ADD_PORTS) && (operation != SAI_LAG_OPER_DEL_PORTS)) {
        return SAI_STATUS_SUCCESS;
    }
    if (port_list->count > (sizeof (g_ports) / sizeof (sai_object_id_t))) {
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    sai_fib_get_rif_id_from_lag_id(lag_id, & rif_id);
    memset (g_ports, 0, sizeof (g_ports));
    g_port_list.list  = g_ports;

    g_lag_id          = lag_id;
    g_rif_id          = rif_id;
    g_lag_operation   = operation;
    g_port_list.count = port_list->count;
    memcpy (g_port_list.list, port_list->list,
            port_list->count * sizeof (sai_object_id_t));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t
sai_lag_ut_validate_rif_resp (sai_object_id_t     lag_id,
                              sai_object_id_t     rif_id,
                              sai_object_id_t     port_id,
                              sai_lag_operation_t operation)
{
    uint32_t index;

    if (lag_id != g_lag_id) {
        return SAI_STATUS_FAILURE;
    }

    if (rif_id != g_rif_id) {
        return SAI_STATUS_FAILURE;
    }

    if (operation != g_lag_operation) {
        return SAI_STATUS_FAILURE;
    }

    if (lag_id != g_lag_id) {
        return SAI_STATUS_FAILURE;
    }

    for (index = 0; index < g_port_list.count; index++) {
        if (port_id == g_port_list.list [index]) {
            return SAI_STATUS_SUCCESS;
        }
    }

    return SAI_STATUS_FAILURE;
}

TEST_F(lagInit, rif_callback)
{
    sai_object_id_t lag_id_1 = 0;
    sai_object_id_t member_id_1 = 0;
    sai_object_id_t member_id_2 = 0;
    sai_object_id_t vrf_id;
    sai_object_id_t rif_id;
    sai_object_id_t lag_bridge_port_id = 0;


    sai_lag_event_callback_register (SAI_MODULE_ROUTER_INTERFACE, sai_lag_ut_rif_callback);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_api_table->create_lag (&lag_id_1, switch_id,0, NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_id_1, &lag_bridge_port_id));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_create_virtual_router (sai_vrf_api_table, &vrf_id));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_create_router_interface (sai_lag_api_table,
                                                   sai_router_intf_api_table,
                                                   sai_vlan_api_table,
                                                   p_sai_bridge_api_tbl,
                                                   &rif_id, vrf_id,
                                                   default_vlan_id,
                                                   lag_id_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, bridge_port_id_1));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_add_port_to_lag (sai_lag_api_table, &member_id_1,
                                           lag_id_1, port_id_1,
                                           true, true, false, false));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_validate_rif_resp(lag_id_1, rif_id,
                                            port_id_1, SAI_LAG_OPER_ADD_PORTS));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, bridge_port_id_2));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_add_port_to_lag (sai_lag_api_table, &member_id_2,
                                           lag_id_1, port_id_2,
                                           false, false, true, true));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_validate_rif_resp(lag_id_1, rif_id,
                                            port_id_2, SAI_LAG_OPER_ADD_PORTS));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_remove_port_from_lag (sai_lag_api_table,
                                                member_id_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id_1, &bridge_port_id_1));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_validate_rif_resp(lag_id_1, rif_id,
                                            port_id_1, SAI_LAG_OPER_DEL_PORTS));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_remove_port_from_lag(sai_lag_api_table, member_id_2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id_2, &bridge_port_id_2));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_validate_rif_resp(lag_id_1, rif_id,
                                            port_id_2, SAI_LAG_OPER_DEL_PORTS));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_remove_router_interface (sai_router_intf_api_table,
                                                   rif_id));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_remove_virtual_router (sai_vrf_api_table, vrf_id));
}

TEST_F(lagInit, lag_attributes_internal)
{
    sai_object_id_t lag_id = 0;
    sai_object_id_t lag_bridge_port = 0;
    sai_attribute_t set_attr[4];
    uint_t          idx = 0;
    sai_object_id_t member_id_1 = 0;
    sai_object_id_t member_id_2 = 0;

    set_attr[0].id =  SAI_LAG_ATTR_PORT_VLAN_ID;
    set_attr[0].value.u16 = 10;

    set_attr[1].id =  SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY;
    set_attr[1].value.u8 = 5;

    set_attr[2].id =  SAI_LAG_ATTR_DROP_UNTAGGED;
    set_attr[2].value.booldata =  false;

    set_attr[3].id =  SAI_LAG_ATTR_DROP_TAGGED;
    set_attr[3].value.booldata =  true;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->create_lag(&lag_id, switch_id, 4, set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_id, &lag_bridge_port));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, bridge_port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_add_port_to_lag (sai_lag_api_table, &member_id_1,
                                          lag_id, port_id_1,
                                          false, false, false, false));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_debug_validate_lag_attribute_set_on_members(lag_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, bridge_port_id_2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_add_port_to_lag (sai_lag_api_table, &member_id_2,
                                          lag_id, port_id_2,
                                          false, false, false, false));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_debug_validate_lag_attribute_set_on_members(lag_id));


    set_attr[0].value.u16 = 20;
    set_attr[1].value.u8 = 3;
    set_attr[2].value.booldata =  true;
    set_attr[3].value.booldata =  false;

    for(idx = 0; idx < 4; idx++) {
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  sai_lag_api_table->set_lag_attribute(lag_id, &set_attr[idx]));
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  sai_debug_validate_lag_attribute_set_on_members(lag_id));
    }

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_port_from_lag (sai_lag_api_table, member_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id_1, &bridge_port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_debug_validate_lag_attribute_set_on_members(lag_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_port_from_lag (sai_lag_api_table, member_id_2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id_2, &bridge_port_id_2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_debug_validate_lag_attribute_set_on_members(lag_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_bridge_port));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->remove_lag(lag_id));
}
