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
 * filename: sai_mirror_unit_test.cpp
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include "stdarg.h"
#include "sai_mirror_unit_test.h"

extern "C" {
#include "sai.h"
#include "saiacl.h"
#include "saistatus.h"
#include "saitypes.h"
#include "saiswitch.h"
#include "saimirror.h"
#include "sai_mirror_api.h"
}
#define SAI_LOCAL_SPAN_NO_OF_MANDAT_ATTRIB 2
#define SAI_REMOTE_SPAN_NO_OF_MANDAT_ATTRIB 5
#define SAI_ER_SPAN_NO_OF_MANDAT_ATTRIB 13

TEST_F(mirrorTest, span_set) {
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  session_id = 0;
    sai_attribute_t attr[SAI_LOCAL_SPAN_NO_OF_MANDAT_ATTRIB + 1] = {0};
    sai_object_list_t obj_list;
    sai_object_id_t *sessions = NULL;

    attr[0].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[0].value.oid = sai_monitor_port;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[1].value.s32 = SAI_MIRROR_SESSION_TYPE_LOCAL;

    sai_rc = sai_test_mirror_session_create (&session_id, SAI_LOCAL_SPAN_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (attr, 0, sizeof(attr));
    attr[0].id = SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id,
            SAI_LOCAL_SPAN_NO_OF_MANDAT_ATTRIB ,attr);


    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (attr[0].value.s32, SAI_MIRROR_SESSION_TYPE_LOCAL);
    EXPECT_EQ (attr[1].value.oid, sai_monitor_port);

    memset (attr, 0, sizeof(attr));
    attr[0].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[0].value.oid = sai_second_monitor_port;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, attr);


    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    memset (attr, 0, sizeof(attr));
    attr[0].id = SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, SAI_LOCAL_SPAN_NO_OF_MANDAT_ATTRIB ,attr);


    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (attr[0].value.s32, SAI_MIRROR_SESSION_TYPE_LOCAL);
    EXPECT_EQ (attr[1].value.oid, sai_second_monitor_port);

    obj_list.count = 1;
    sessions = (sai_object_id_t *) calloc (1, sizeof(sai_object_id_t));
    sessions[0] = session_id;
    obj_list.list = sessions;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 0;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    free (sessions);
}

TEST_F(mirrorTest, span_lag_set) {
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  session_id = 0;
    sai_attribute_t attr[SAI_LOCAL_SPAN_NO_OF_MANDAT_ATTRIB + 1] = {0};
    sai_object_id_t lag_id = 0;
    sai_object_id_t port_arr[2];
    sai_object_list_t lag_port_list;
    sai_attribute_t lag_attr;
    sai_object_list_t obj_list;
    sai_object_id_t *sessions = NULL;

    lag_port_list.count = 2;
    port_arr[0] = sai_monitor_port;
    port_arr[1] = sai_second_monitor_port;
    lag_port_list.list = port_arr;

    lag_attr.id = SAI_LAG_ATTR_PORT_LIST;
    lag_attr.value.objlist = lag_port_list;

    EXPECT_EQ (SAI_STATUS_SUCCESS,
                  p_sai_lag_api_table->create_lag(&lag_id, switch_id, 1, &lag_attr));

    attr[0].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[0].value.oid = lag_id;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[1].value.s32 = SAI_MIRROR_SESSION_TYPE_LOCAL;

    sai_rc = sai_test_mirror_session_create (&session_id, SAI_LOCAL_SPAN_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 1;
    sessions = (sai_object_id_t *) calloc (1, sizeof(sai_object_id_t));
    sessions[0] = session_id;
    obj_list.list = sessions;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 0;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    EXPECT_EQ (SAI_STATUS_SUCCESS,
                  p_sai_lag_api_table->remove_lag(lag_id));

    free (sessions);
}

TEST_F(mirrorTest, span_multiple_port_set) {
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  session_id = 0;
    sai_attribute_t attr[3] = {0};
    sai_object_list_t obj_list;
    sai_object_id_t *sessions = NULL;

    attr[0].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[0].value.oid = sai_monitor_port;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[1].value.s32 = SAI_MIRROR_SESSION_TYPE_LOCAL;

    sai_rc = sai_test_mirror_session_create (&session_id, SAI_LOCAL_SPAN_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (attr, 0, sizeof(attr));
    attr[0].id = SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id,
                                   SAI_LOCAL_SPAN_NO_OF_MANDAT_ATTRIB ,attr);


    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (attr[0].value.s32, SAI_MIRROR_SESSION_TYPE_LOCAL);
    EXPECT_EQ (attr[1].value.oid, sai_monitor_port);

    obj_list.count = 1;
    sessions = (sai_object_id_t *) calloc (1, sizeof(sai_object_id_t));
    sessions[0] = session_id;
    obj_list.list = sessions;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_ingress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], session_id);

    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_egress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], session_id);

    obj_list.count = 1;
    sessions[0] = session_id;
    obj_list.list = sessions;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_ingress_port_get(sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], session_id);

    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_egress_port_get(sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], session_id);

    obj_list.count = 0;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 1;
    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_ingress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], 0);

    obj_list.count = 0;
    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 1;
    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_ingress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], 0);

    sai_rc = sai_test_mirror_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);

    obj_list.count = 0;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 1;
    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_ingress_port_get(sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], 0);

    sai_rc = sai_test_mirror_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);

    obj_list.count = 0;
    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 1;
    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_egress_port_get(sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], 0);

    sai_rc = sai_test_mirror_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    free (sessions);
}

TEST_F(mirrorTest, multiple_span_set) {
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  span_session_id = 0;
    sai_object_id_t  rspan_session_id = 0;
    sai_attribute_t attr[10] = {0};
    sai_object_list_t obj_list;
    sai_object_id_t *sessions = NULL;

    attr[0].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[0].value.oid = sai_monitor_port;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[1].value.s32 = SAI_MIRROR_SESSION_TYPE_LOCAL;

    sai_rc = sai_test_mirror_session_create (&span_session_id,
                                             SAI_LOCAL_SPAN_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (attr, 0, sizeof(attr));
    attr[0].id = SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (span_session_id,
                                   SAI_LOCAL_SPAN_NO_OF_MANDAT_ATTRIB ,attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (attr[0].value.s32, SAI_MIRROR_SESSION_TYPE_LOCAL);
    EXPECT_EQ (attr[1].value.oid, sai_monitor_port);

    attr[0].id =  SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[0].value.s32 = SAI_MIRROR_SESSION_TYPE_REMOTE;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[1].value.oid = sai_second_monitor_port;
    attr[2].id = SAI_MIRROR_SESSION_ATTR_VLAN_TPID;
    attr[2].value.u16 = 0x8100;
    attr[3].id = SAI_MIRROR_SESSION_ATTR_VLAN_ID;
    attr[3].value.u16 = 2;
    attr[4].id = SAI_MIRROR_SESSION_ATTR_VLAN_PRI;
    attr[4].value.u8 = 2;

    sai_rc = sai_test_mirror_session_create (&rspan_session_id, SAI_REMOTE_SPAN_NO_OF_MANDAT_ATTRIB,
                                             attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (attr, 0, sizeof(attr));
    attr[0].id = SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[2].id = SAI_MIRROR_SESSION_ATTR_VLAN_TPID;
    attr[3].id = SAI_MIRROR_SESSION_ATTR_VLAN_ID;
    attr[4].id = SAI_MIRROR_SESSION_ATTR_VLAN_PRI;

    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (rspan_session_id, 5,attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (attr[0].value.s32, SAI_MIRROR_SESSION_TYPE_REMOTE);
    EXPECT_EQ (attr[1].value.oid, sai_second_monitor_port);
    EXPECT_EQ (attr[2].value.u16, 33024);
    EXPECT_EQ (attr[3].value.u16, 2);
    EXPECT_EQ (attr[4].value.u8, 2);

    obj_list.count = 2;
    sessions = (sai_object_id_t *) calloc (obj_list.count, sizeof(sai_object_id_t));
    sessions[0] = span_session_id;
    sessions[1] = rspan_session_id;
    obj_list.list = sessions;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 1;
    sessions = (sai_object_id_t *) realloc (sessions, obj_list.count * sizeof(sai_object_id_t));
    sessions[0] = span_session_id;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 0;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_destroy (span_session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_destroy (rspan_session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    free (sessions);
}

TEST_F(mirrorTest, invalid_attrib_value) {
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  session_id = 0;
    sai_attribute_t attr[3] = {0};

    attr[0].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[0].value.oid = sai_invalid_port;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[1].value.s32 = SAI_MIRROR_SESSION_TYPE_LOCAL;

    sai_rc = sai_test_mirror_session_create (&session_id, SAI_LOCAL_SPAN_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_INVALID_ATTR_VALUE_0, sai_rc);
}

TEST_F(mirrorTest, rspan_attrib_set) {
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  session_id = 0;
    sai_attribute_t attr[SAI_REMOTE_SPAN_NO_OF_MANDAT_ATTRIB + 1] = {0};
    sai_attribute_t set_attr,get_attr;
    sai_object_list_t obj_list;
    sai_object_id_t *sessions = NULL;

    memset (&set_attr, 0, sizeof(set_attr));
    memset (&get_attr, 0, sizeof(get_attr));

    attr[0].id = SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[0].value.u8 = SAI_MIRROR_SESSION_TYPE_REMOTE;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[1].value.oid = sai_monitor_port;
    attr[2].id = SAI_MIRROR_SESSION_ATTR_VLAN_TPID;
    attr[2].value.u16 = 0x8100;
    attr[3].id = SAI_MIRROR_SESSION_ATTR_VLAN_ID;
    attr[3].value.u16 = SAI_MIRROR_VLAN_2;
    attr[4].id = SAI_MIRROR_SESSION_ATTR_VLAN_PRI;
    attr[4].value.u8 = 2;

    sai_rc = sai_test_mirror_session_create (&session_id, SAI_REMOTE_SPAN_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 1;
    sessions = (sai_object_id_t *) calloc (1, sizeof(sai_object_id_t));
    sessions[0] = session_id;
    obj_list.list = sessions;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_ingress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], session_id);

    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_egress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], session_id);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_TOS;
    set_attr.value.u8 = 10;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_rc);

    set_attr.id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    set_attr.value.oid =  sai_second_monitor_port;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.oid,  sai_second_monitor_port);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_VLAN_TPID;
    set_attr.value.u16 = 0x8101;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id =  SAI_MIRROR_SESSION_ATTR_VLAN_TPID;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.u16, 33025);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_VLAN_ID;
    set_attr.value.u16 = SAI_MIRROR_VLAN_3;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id =  SAI_MIRROR_SESSION_ATTR_VLAN_ID;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.u16, SAI_MIRROR_VLAN_3);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_VLAN_PRI;
    set_attr.value.u8 = 4;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id =  SAI_MIRROR_SESSION_ATTR_VLAN_PRI;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.u8, 4);

    obj_list.count = 0;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 1;
    sai_rc = sai_test_mirror_session_ingress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], 0);

    sai_rc = sai_test_mirror_session_egress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], 0);

    obj_list.count = 1;
    sessions[0] = session_id;
    obj_list.list = sessions;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_ingress_port_get(sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], session_id);

    obj_list.count = 0;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 1;
    sessions[0] = session_id;
    obj_list.list = sessions;
    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_egress_port_get(sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], session_id);

    obj_list.count = 0;
    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

TEST_F(mirrorTest, erspan_set) {
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  session_id = 0;
    sai_attribute_t attr[SAI_ER_SPAN_NO_OF_MANDAT_ATTRIB + 1] = {0};
    sai_object_list_t obj_list;
    sai_object_id_t *sessions = NULL;

    attr[0].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[0].value.oid = sai_monitor_port;
    attr[1].id = SAI_MIRROR_SESSION_ATTR_VLAN_TPID;
    attr[1].value.u16 = 0x8100;
    attr[2].id = SAI_MIRROR_SESSION_ATTR_VLAN_ID;
    attr[2].value.u16 = SAI_MIRROR_VLAN_2;
    attr[3].id = SAI_MIRROR_SESSION_ATTR_VLAN_PRI;
    attr[3].value.u8 = 2;
    attr[4].id = SAI_MIRROR_SESSION_ATTR_ERSPAN_ENCAPSULATION_TYPE;
    attr[4].value.s32 = SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL;
    attr[5].id = SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION;
    attr[5].value.u8 = 4;
    attr[6].id = SAI_MIRROR_SESSION_ATTR_TOS;
    attr[6].value.u16 = 2;
    attr[7].id = SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS;
    attr[7].value.ipaddr.addr.ip4 = 2;
    attr[7].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr[8].id = SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS;
    attr[8].value.ipaddr.addr.ip4 = 2;
    attr[8].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr[9].id = SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS;
    attr[9].value.mac[0] = 34;
    attr[9].value.mac[1] = 34;
    attr[9].value.mac[2] = 34;
    attr[9].value.mac[3] = 34;
    attr[9].value.mac[4] = 34;
    attr[9].value.mac[5] = 34;
    attr[10].id = SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS;
    attr[10].value.mac[0] = 68;
    attr[10].value.mac[1] = 68;
    attr[10].value.mac[2] = 68;
    attr[10].value.mac[3] = 68;
    attr[10].value.mac[4] = 68;
    attr[10].value.mac[5] = 68;
    attr[11].id = SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE;
    attr[11].value.u16 = 35006;
    attr[12].id =  SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[12].value.s32 = SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE;

    sai_rc = sai_test_mirror_session_create (&session_id, SAI_ER_SPAN_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (attr, 0, sizeof(attr));
    attr[0].id = SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[1].id = SAI_MIRROR_SESSION_ATTR_VLAN_TPID;
    attr[2].id = SAI_MIRROR_SESSION_ATTR_VLAN_ID;
    attr[3].id = SAI_MIRROR_SESSION_ATTR_VLAN_PRI;
    attr[4].id = SAI_MIRROR_SESSION_ATTR_ERSPAN_ENCAPSULATION_TYPE;
    attr[5].id = SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION;
    attr[6].id = SAI_MIRROR_SESSION_ATTR_TOS;
    attr[7].id = SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS;
    attr[8].id = SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS;
    attr[9].id = SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS;
    attr[10].id = SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS;
    attr[11].id = SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE;
    attr[12].id = SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[13].id = SAI_MIRROR_SESSION_ATTR_TTL;

    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id,
                                   SAI_ER_SPAN_NO_OF_MANDAT_ATTRIB + 1, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (attr[0].value.oid, sai_monitor_port);
    EXPECT_EQ (attr[12].value.s32, SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE);
    EXPECT_EQ (attr[1].value.u16, 33024);
    EXPECT_EQ (attr[2].value.u16, 2);
    EXPECT_EQ (attr[3].value.u8, 2);
    EXPECT_EQ (attr[4].value.s32 , SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL);
    EXPECT_EQ (attr[5].value.u8 , 4);
    EXPECT_EQ (attr[6].value.u16 , 2);
    EXPECT_EQ (attr[7].value.ipaddr.addr.ip4 , 2);
    EXPECT_EQ (attr[7].value.ipaddr.addr_family , SAI_IP_ADDR_FAMILY_IPV4);
    EXPECT_EQ (attr[8].value.ipaddr.addr.ip4 , 2);
    EXPECT_EQ (attr[8].value.ipaddr.addr_family , SAI_IP_ADDR_FAMILY_IPV4);
    EXPECT_EQ (attr[9].value.mac[0] , 34);
    EXPECT_EQ (attr[9].value.mac[1] , 34);
    EXPECT_EQ (attr[9].value.mac[2] , 34);
    EXPECT_EQ (attr[9].value.mac[3] , 34);
    EXPECT_EQ (attr[9].value.mac[4] , 34);
    EXPECT_EQ (attr[9].value.mac[5] , 34);
    EXPECT_EQ (attr[10].value.mac[0] , 68);
    EXPECT_EQ (attr[10].value.mac[1] , 68);
    EXPECT_EQ (attr[10].value.mac[2] , 68);
    EXPECT_EQ (attr[10].value.mac[3] , 68);
    EXPECT_EQ (attr[10].value.mac[4] , 68);
    EXPECT_EQ (attr[10].value.mac[5] , 68);
    EXPECT_EQ (attr[11].value.u16 , 35006);
    EXPECT_EQ (attr[13].value.u16 , 255);

    obj_list.count = 1;
    sessions = (sai_object_id_t *) calloc (1, sizeof(sai_object_id_t));
    sessions[0] = session_id;
    obj_list.list = sessions;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_ingress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], session_id);

    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_egress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], session_id);

    obj_list.count = 0;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 1;
    sai_rc = sai_test_mirror_session_ingress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], 0);

    sai_rc = sai_test_mirror_session_egress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], 0);

    obj_list.count = 1;
    sessions[0] = session_id;
    obj_list.list = sessions;
    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_egress_port_get(sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], session_id);

    obj_list.count = 0;
    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_second_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    free (sessions);
}

TEST_F(mirrorTest, erspan_invalid_attrib) {
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  session_id = 0;
    sai_attribute_t attr[15] = {0};

    attr[0].id = 20;
    attr[0].value.u8 = 7;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[1].value.oid = sai_monitor_port;
    attr[2].id = SAI_MIRROR_SESSION_ATTR_VLAN_TPID;
    attr[2].value.u16 = 0x8100;
    attr[3].id = SAI_MIRROR_SESSION_ATTR_VLAN_ID;
    attr[3].value.u16 = SAI_MIRROR_VLAN_2;
    attr[4].id = SAI_MIRROR_SESSION_ATTR_VLAN_PRI;
    attr[4].value.u8 = 2;
    attr[5].id = SAI_MIRROR_SESSION_ATTR_ERSPAN_ENCAPSULATION_TYPE;
    attr[5].value.s32 = SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL;
    attr[6].id = SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION;
    attr[6].value.u8 = 4;
    attr[7].id = SAI_MIRROR_SESSION_ATTR_TOS;
    attr[7].value.u16 = 2;
    attr[8].id = SAI_MIRROR_SESSION_ATTR_TTL;
    attr[8].value.u8 = 2;
    attr[9].id = SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS;
    attr[9].value.ipaddr.addr.ip4 = 2;
    attr[9].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr[10].id = SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS;
    attr[10].value.ipaddr.addr.ip4 = 2;
    attr[10].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr[11].id = SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS;
    attr[11].value.mac[0] = 34;
    attr[11].value.mac[1] = 34;
    attr[11].value.mac[2] = 34;
    attr[11].value.mac[3] = 34;
    attr[11].value.mac[4] = 34;
    attr[11].value.mac[5] = 34;
    attr[12].id = SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS;
    attr[12].value.mac[0] = 68;
    attr[12].value.mac[1] = 68;
    attr[12].value.mac[2] = 68;
    attr[12].value.mac[3] = 68;
    attr[12].value.mac[4] = 68;
    attr[12].value.mac[5] = 68;
    attr[13].id =  SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[13].value.s32 = SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE;

    sai_rc = sai_test_mirror_session_create (&session_id, 14, attr);

    EXPECT_EQ (SAI_STATUS_CODE(abs(SAI_STATUS_UNKNOWN_ATTRIBUTE_0)), sai_rc);
}

TEST_F(mirrorTest, erspan_mandat_miss) {
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  session_id = 0;
    sai_attribute_t attr[15] = {0};

    attr[0].id =  SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[0].value.s32 = SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[1].value.oid = sai_monitor_port;
    attr[2].id = SAI_MIRROR_SESSION_ATTR_VLAN_TPID;
    attr[2].value.u16 = 0x8100;
    attr[3].id = SAI_MIRROR_SESSION_ATTR_VLAN_ID;
    attr[3].value.u16 = 2;
    attr[4].id = SAI_MIRROR_SESSION_ATTR_VLAN_PRI;
    attr[4].value.u8 = 2;
    attr[5].id = SAI_MIRROR_SESSION_ATTR_ERSPAN_ENCAPSULATION_TYPE;
    attr[5].value.s32 = SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL;
    attr[6].id = SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION;
    attr[6].value.u8 = 4;
    attr[7].id = SAI_MIRROR_SESSION_ATTR_TOS;
    attr[7].value.u16 = 2;
    attr[8].id = SAI_MIRROR_SESSION_ATTR_TTL;
    attr[8].value.u8 = 2;
    attr[9].id = SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS;
    attr[9].value.ipaddr.addr.ip4 = 2;
    attr[9].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr[10].id = SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS;
    attr[10].value.ipaddr.addr.ip4 = 2;
    attr[10].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr[11].id = SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS;
    attr[11].value.mac[0] = 34;
    attr[11].value.mac[1] = 34;
    attr[11].value.mac[2] = 34;
    attr[11].value.mac[3] = 34;
    attr[11].value.mac[4] = 34;
    attr[11].value.mac[5] = 34;
    attr[12].id = SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS;
    attr[12].value.mac[0] = 68;
    attr[12].value.mac[1] = 68;
    attr[12].value.mac[2] = 68;
    attr[12].value.mac[3] = 68;
    attr[12].value.mac[4] = 68;
    attr[12].value.mac[5] = 68;

    sai_rc = sai_test_mirror_session_create (&session_id, 13, attr);

    EXPECT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING, sai_rc);
}

TEST_F(mirrorTest, erspan_attrib_set) {
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  session_id = 0;
    sai_attribute_t attr[SAI_ER_SPAN_NO_OF_MANDAT_ATTRIB + 1] = {0};
    sai_attribute_t set_attr,get_attr;
    sai_object_list_t obj_list;
    sai_object_id_t *sessions = NULL;

    memset (&set_attr, 0, sizeof(set_attr));
    memset (&get_attr, 0, sizeof(get_attr));

    attr[0].id = SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[0].value.u8 = SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[1].value.oid = sai_monitor_port;
    attr[2].id = SAI_MIRROR_SESSION_ATTR_VLAN_TPID;
    attr[2].value.u16 = 0x8100;
    attr[3].id = SAI_MIRROR_SESSION_ATTR_VLAN_ID;
    attr[3].value.u16 = 2;
    attr[4].id = SAI_MIRROR_SESSION_ATTR_VLAN_PRI;
    attr[4].value.u8 = 2;
    attr[5].id = SAI_MIRROR_SESSION_ATTR_ERSPAN_ENCAPSULATION_TYPE;
    attr[5].value.s32 = SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL;
    attr[6].id = SAI_MIRROR_SESSION_ATTR_IPHDR_VERSION;
    attr[6].value.u8 = 4;
    attr[7].id = SAI_MIRROR_SESSION_ATTR_TOS;
    attr[7].value.u16 = 2;
    attr[8].id = SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE;
    attr[8].value.u16 = 0x88BE;
    attr[9].id = SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS;
    attr[9].value.ipaddr.addr.ip4 = 0x20000000;
    attr[9].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr[10].id = SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS;
    attr[10].value.ipaddr.addr.ip4 = 0x40000000;
    attr[10].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr[11].id = SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS;
    attr[11].value.mac[0] = 34;
    attr[11].value.mac[1] = 34;
    attr[11].value.mac[2] = 34;
    attr[11].value.mac[3] = 34;
    attr[11].value.mac[4] = 34;
    attr[11].value.mac[5] = 34;
    attr[12].id = SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS;
    attr[12].value.mac[0] = 68;
    attr[12].value.mac[1] = 68;
    attr[12].value.mac[2] = 68;
    attr[12].value.mac[3] = 68;
    attr[12].value.mac[4] = 68;
    attr[12].value.mac[5] = 68;

    sai_rc = sai_test_mirror_session_create (&session_id, SAI_ER_SPAN_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 1;
    sessions = (sai_object_id_t *) calloc (1, sizeof(sai_object_id_t));
    sessions[0] = session_id;
    obj_list.list = sessions;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_ingress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], session_id);

    memset (obj_list.list, 0, obj_list.count * sizeof(sai_object_id_t));
    sai_rc = sai_test_mirror_session_egress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], session_id);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_TYPE;
    set_attr.value.u8 = SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_rc);

    set_attr.id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    set_attr.value.oid =  sai_second_monitor_port;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.oid,  sai_second_monitor_port);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_VLAN_TPID;
    set_attr.value.u16 = 0x8101;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id =  SAI_MIRROR_SESSION_ATTR_VLAN_TPID;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.u16, 33025);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_VLAN_ID;
    set_attr.value.u16 = 3;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id =  SAI_MIRROR_SESSION_ATTR_VLAN_ID;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.u16, 3);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_VLAN_PRI;
    set_attr.value.u8 = 4;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id =  SAI_MIRROR_SESSION_ATTR_VLAN_PRI;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.u8, 4);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_ERSPAN_ENCAPSULATION_TYPE;
    set_attr.value.u16 = 2;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_rc);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_TTL;
    set_attr.value.u8 = 254;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_MIRROR_SESSION_ATTR_TTL;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.u8, 254);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_TOS;
    set_attr.value.u8 = 7;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id =  SAI_MIRROR_SESSION_ATTR_TOS;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.u8, 7);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE;
    set_attr.value.u16 = 35006;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id =  SAI_MIRROR_SESSION_ATTR_GRE_PROTOCOL_TYPE;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.u16, 35006);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS;
    set_attr.value.ipaddr.addr.ip4 = 04;
    set_attr.value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id =  SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.ipaddr.addr.ip4, 04);
    EXPECT_EQ (get_attr.value.ipaddr.addr_family, SAI_IP_ADDR_FAMILY_IPV4);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS;
    set_attr.value.ipaddr.addr.ip4 = 01;
    set_attr.value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id =  SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.ipaddr.addr.ip4, 01);
    EXPECT_EQ (get_attr.value.ipaddr.addr_family, SAI_IP_ADDR_FAMILY_IPV4);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS;
    set_attr.value.mac[0] = 04;
    set_attr.value.mac[1] = 04;
    set_attr.value.mac[2] = 04;
    set_attr.value.mac[3] = 04;
    set_attr.value.mac[4] = 04;
    set_attr.value.mac[5] = 04;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.mac[0] , 04);
    EXPECT_EQ (get_attr.value.mac[1] , 04);
    EXPECT_EQ (get_attr.value.mac[2] , 04);
    EXPECT_EQ (get_attr.value.mac[3] , 04);
    EXPECT_EQ (get_attr.value.mac[4] , 04);
    EXPECT_EQ (get_attr.value.mac[5] , 04);

    set_attr.id = SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS;
    set_attr.value.mac[0] = 02;
    set_attr.value.mac[1] = 02;
    set_attr.value.mac[2] = 02;
    set_attr.value.mac[3] = 02;
    set_attr.value.mac[4] = 02;
    set_attr.value.mac[5] = 02;
    sai_rc = p_sai_mirror_api_tbl->set_mirror_session_attribute (session_id, &set_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (&get_attr, 0, sizeof(get_attr));
    get_attr.id =  SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS;
    sai_rc = p_sai_mirror_api_tbl->get_mirror_session_attribute (session_id, 1, &get_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (get_attr.value.mac[0] , 02);
    EXPECT_EQ (get_attr.value.mac[1] , 02);
    EXPECT_EQ (get_attr.value.mac[2] , 02);
    EXPECT_EQ (get_attr.value.mac[3] , 02);
    EXPECT_EQ (get_attr.value.mac[4] , 02);
    EXPECT_EQ (get_attr.value.mac[5] , 02);

    obj_list.count = 0;
    sai_rc = sai_test_mirror_session_ingress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_mirror_session_egress_port_add (sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    obj_list.count = 1;
    sai_rc = sai_test_mirror_session_ingress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], 0);

    sai_rc = sai_test_mirror_session_egress_port_get(sai_mirror_first_port, &obj_list);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (obj_list.list[0], 0);

    sai_rc = sai_test_mirror_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    free (sessions);

}

TEST_F(mirrorTest, flow_based) {
    sai_object_id_t acl_table_id;
    sai_object_id_t acl_rule_id;
    sai_object_id_t session_id;
    sai_attribute_t attr[12] = {0};
    sai_attribute_t rule_attr[10] = {0};
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    attr[0].id =  SAI_MIRROR_SESSION_ATTR_MONITOR_PORT;
    attr[0].value.oid = sai_monitor_port;
    attr[1].id =  SAI_MIRROR_SESSION_ATTR_TYPE;
    attr[1].value.s32 = SAI_MIRROR_SESSION_TYPE_LOCAL;

    sai_rc = sai_test_mirror_session_create (&session_id, SAI_LOCAL_SPAN_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr[0].id = SAI_ACL_TABLE_ATTR_ACL_STAGE;
    attr[0].value.s32= 0;
    attr[1].id =  SAI_ACL_TABLE_ATTR_PRIORITY;
    attr[1].value.u32 = 1;
    attr[2].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
    attr[3].id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
    attr[4].id = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
    attr[5].id = SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE;
    attr[6].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID;
    attr[7].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI;
    attr[8].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI;
    attr[9].id = SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
    attr[10].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr[11].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    sai_rc = p_sai_acl_api_tbl->create_acl_table (&acl_table_id, switch_id, 12, attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    rule_attr[0].value.oid = acl_table_id;
    rule_attr[1].id =  SAI_ACL_ENTRY_ATTR_PRIORITY;
    rule_attr[1].value.u32 = 1;
    rule_attr[2].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    rule_attr[2].value.booldata= true;
    rule_attr[3].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    rule_attr[3].value.aclfield.data.objlist.count = 1;
    rule_attr[3].value.aclfield.data.objlist.list = (sai_object_id_t *) calloc(
                                                            1, sizeof(sai_object_id_t));
    rule_attr[3].value.aclfield.data.objlist.list[0] = sai_mirror_second_port;
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS;
    rule_attr[4].value.aclaction.enable = true;
    rule_attr[4].value.aclaction.parameter.objlist.count = 1;
    rule_attr[4].value.aclaction.parameter.objlist.list = (sai_object_id_t *) calloc(
                                                            1, sizeof(sai_object_id_t));
    rule_attr[4].value.aclaction.parameter.objlist.list[0] = session_id;
    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    EXPECT_EQ (SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->remove_acl_entry(acl_rule_id));
    EXPECT_EQ (SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->remove_acl_table (acl_table_id));

    sai_rc = sai_test_mirror_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    free (rule_attr[3].value.aclfield.data.objlist.list);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
