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
 * filename: sai_mirror_unit_test_utils.cpp
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "gtest/gtest.h"
#include "stdarg.h"
#include "sai_mirror_unit_test.h"
#include "sai_port_breakout_test_utils.h"

extern "C" {
#include "sai.h"
#include "saistatus.h"
#include "saitypes.h"
#include "saiswitch.h"
#include "saimirror.h"
#include "sai_mirror_api.h"
#include "sai_oid_utils.h"
}

#define SAI_MAX_PORTS 106

static uint32_t port_count = 0;

static sai_object_id_t port_list[SAI_MAX_PORTS] = {0};

/*
 * Stubs for Callback functions to be passed from adaptor host/application
 * upon switch initialization API call.
 */
static inline void sai_port_state_evt_callback (uint32_t count,
                                                sai_port_oper_status_notification_t *data)
{
}

static inline void sai_fdb_evt_callback(uint32_t count, sai_fdb_event_notification_data_t *data)
{
}

static inline void sai_switch_operstate_callback (sai_switch_oper_status_t
                                                  switchstate) {
}

/* Packet event callback
 */
static inline void sai_packet_event_callback (const void *buffer,
                                              sai_size_t buffer_size,
                                              uint32_t attr_count,
                                              const sai_attribute_t *attr_list)
{
}

static inline void  sai_switch_shutdown_callback (void) {
}

sai_object_id_t mirrorTest ::sai_mirror_port_id_get (uint32_t port_index)
{
    if(port_index >= port_count) {
        return 0;
    }

    return port_list [port_index];
}

sai_object_id_t mirrorTest ::sai_mirror_invalid_port_id_get ()
{
    return (port_list[port_count-1] + 1);
}


void mirrorTest ::SetUpTestCase (void) {

    sai_attribute_t sai_attr_set[7];
    uint32_t attr_count = 7;

    memset(sai_attr_set,0, sizeof(sai_attr_set));
    /*
     * Query and populate the SAI Switch API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
               (SAI_API_SWITCH, (static_cast<void**>
                                 (static_cast<void*>(&p_sai_switch_api_tbl)))));

    ASSERT_TRUE (p_sai_switch_api_tbl != NULL);

    sai_attr_set[0].id = SAI_SWITCH_ATTR_INIT_SWITCH;
    sai_attr_set[0].value.booldata = 1;

    sai_attr_set[1].id = SAI_SWITCH_ATTR_SWITCH_PROFILE_ID;
    sai_attr_set[1].value.u32 = 0;

    sai_attr_set[2].id = SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY;
    sai_attr_set[2].value.ptr = (void *)sai_fdb_evt_callback;

    sai_attr_set[3].id = SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY;
    sai_attr_set[3].value.ptr = (void *)sai_port_state_evt_callback;

    sai_attr_set[4].id = SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY;
    sai_attr_set[4].value.ptr = (void *)sai_packet_event_callback;

    sai_attr_set[5].id = SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY;
    sai_attr_set[5].value.ptr = (void *)sai_switch_operstate_callback;

    sai_attr_set[6].id = SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY;
    sai_attr_set[6].value.ptr = (void *)sai_switch_shutdown_callback;

    ASSERT_TRUE(p_sai_switch_api_tbl->create_switch != NULL);

    EXPECT_EQ (SAI_STATUS_SUCCESS,
               (p_sai_switch_api_tbl->create_switch (&switch_id , attr_count,
                                                         sai_attr_set)));

    /*
     * Query and populate the SAI Port API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
               (SAI_API_PORT, (static_cast<void**>
                                         (static_cast<void*>
                                          (&p_sai_port_api_tbl)))));

    ASSERT_TRUE (p_sai_port_api_tbl != NULL);

    /*
     * Query and populate the SAI Mirror API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
               (SAI_API_MIRROR, (static_cast<void**>
                                         (static_cast<void*>
                                          (&p_sai_mirror_api_tbl)))));

    ASSERT_TRUE (p_sai_mirror_api_tbl != NULL);

    /*
     * Query and populate the SAI Acl API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
               (SAI_API_ACL, (static_cast<void**>
                                         (static_cast<void*>
                                          (&p_sai_acl_api_tbl)))));

    ASSERT_TRUE (p_sai_acl_api_tbl != NULL);

    /*
     * Query and populate the SAI Acl API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
               (SAI_API_VLAN, (static_cast<void**>
                                         (static_cast<void*>
                                          (&p_sai_vlan_api_tbl)))));

    ASSERT_TRUE (p_sai_vlan_api_tbl != NULL);

    /*
     * Query and populate the SAI Acl API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
               (SAI_API_LAG, (static_cast<void**>
                                         (static_cast<void*>
                                          (&p_sai_lag_api_table)))));

    ASSERT_TRUE (p_sai_lag_api_table != NULL);

    sai_attribute_t sai_port_attr;
    sai_status_t ret = SAI_STATUS_SUCCESS;

    memset (&sai_port_attr, 0, sizeof (sai_port_attr));

    sai_port_attr.id = SAI_SWITCH_ATTR_PORT_LIST;
    sai_port_attr.value.objlist.count = SAI_MAX_PORTS;
    sai_port_attr.value.objlist.list  = port_list;

    ret = p_sai_switch_api_tbl->get_switch_attribute(0,1,&sai_port_attr);
    port_count = sai_port_attr.value.objlist.count;
    EXPECT_EQ (SAI_STATUS_SUCCESS,ret);

    sai_mirror_first_port = port_list[port_count - 2];
    sai_mirror_second_port = sai_mirror_port_id_get (17);
    sai_mirror_third_port = sai_mirror_port_id_get (9);
    sai_monitor_port = port_list[port_count - 1];
    sai_second_monitor_port = sai_mirror_port_id_get (13);
    sai_invalid_port = sai_mirror_invalid_port_id_get();

    {
        sai_attribute_t attr;

        attr.id = SAI_VLAN_ATTR_VLAN_ID;
        attr.value.u16 = SAI_MIRROR_VLAN_2;
        EXPECT_EQ(SAI_STATUS_SUCCESS,
                p_sai_vlan_api_tbl->create_vlan(&sai_mirror_vlan_2,0,1,&attr));

        attr.id = SAI_VLAN_ATTR_VLAN_ID;
        attr.value.u16 = SAI_MIRROR_VLAN_3;
        EXPECT_EQ(SAI_STATUS_SUCCESS,
                p_sai_vlan_api_tbl->create_vlan(&sai_mirror_vlan_3,0,1,&attr));
    }
}

sai_switch_api_t* mirrorTest ::p_sai_switch_api_tbl = NULL;
sai_port_api_t* mirrorTest ::p_sai_port_api_tbl = NULL;
sai_mirror_api_t* mirrorTest ::p_sai_mirror_api_tbl = NULL;
sai_acl_api_t* mirrorTest ::p_sai_acl_api_tbl = NULL;
sai_vlan_api_t* mirrorTest ::p_sai_vlan_api_tbl = NULL;
sai_lag_api_t* mirrorTest ::p_sai_lag_api_table = NULL;
sai_object_id_t mirrorTest ::sai_mirror_first_port = 0;
sai_object_id_t mirrorTest ::sai_mirror_second_port = 0;
sai_object_id_t mirrorTest ::sai_mirror_third_port = 0;
sai_object_id_t mirrorTest ::sai_monitor_port = 0;
sai_object_id_t mirrorTest ::sai_second_monitor_port = 0;
sai_object_id_t mirrorTest ::sai_invalid_port = 0;
sai_object_id_t mirrorTest ::sai_mirror_vlan_2 = 0;
sai_object_id_t mirrorTest ::sai_mirror_vlan_3 = 0;
sai_object_id_t mirrorTest ::switch_id = 0;

sai_status_t mirrorTest ::sai_test_mirror_session_create (sai_object_id_t *p_session_id,
        uint32_t attr_count,
        sai_attribute_t *p_attr_list)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;


    printf ("Testing Session Create API with attribute count: %d\r\n", attr_count);

    sai_rc = p_sai_mirror_api_tbl->create_mirror_session (p_session_id,switch_id,
                                                          attr_count,p_attr_list);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        printf ("SAI Mirror session Creation API failed with error: %d\r\n", sai_rc);
    } else {
        printf ("SAI Mirror session Creation API success, session ID: %lu\r\n", *p_session_id);
    }

    return sai_rc;
}

sai_status_t mirrorTest ::sai_test_mirror_session_destroy (sai_object_id_t session_id) {
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    sai_rc = p_sai_mirror_api_tbl->remove_mirror_session (session_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        printf ("Mirror Session destroy API failed with error: %d\r\n", sai_rc);
    } else {
        printf ("Mirror Session API success for session id: %lu\r\n", session_id);
    }

    return sai_rc;
}

sai_status_t mirrorTest ::sai_test_mirror_session_ingress_port_add (sai_object_id_t port_id,
            sai_object_list_t *obj_list) {

    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    attr.id = SAI_PORT_ATTR_INGRESS_MIRROR_SESSION;
    memcpy (&attr.value.objlist,obj_list,sizeof (sai_object_list_t));
    sai_rc = p_sai_port_api_tbl->set_port_attribute (port_id,&attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        printf ("Set port attribute API failed with error: %d\r\n", sai_rc);
    } else {
        printf ("Set port attribute API success for port id: %lu\r\n", port_id);
    }

    return sai_rc;
}

sai_status_t mirrorTest ::sai_test_mirror_session_ingress_port_get (sai_object_id_t port_id,
            sai_object_list_t *obj_list) {

    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    attr.id = SAI_PORT_ATTR_INGRESS_MIRROR_SESSION;
    memcpy (&attr.value.objlist,obj_list,sizeof (sai_object_list_t));
    sai_rc = p_sai_port_api_tbl->get_port_attribute (port_id, 1, &attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        printf ("Get port attribute API failed with error: %d\r\n", sai_rc);
    } else {
        printf ("Get port attribute API success for port id: %lu\r\n", port_id);
    }

    return sai_rc;
}

sai_status_t mirrorTest ::sai_test_mirror_session_egress_port_add (sai_object_id_t port_id,
            sai_object_list_t *obj_list) {

    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    attr.id = SAI_PORT_ATTR_EGRESS_MIRROR_SESSION;
    memcpy (&attr.value.objlist,obj_list,sizeof (sai_object_list_t));
    sai_rc = p_sai_port_api_tbl->set_port_attribute (port_id,&attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        printf ("Set port attribute API failed with error: %d\r\n", sai_rc);
    } else {
        printf ("Set port attribute API success for port id: %lu\r\n", port_id);
    }

    return sai_rc;
}

sai_status_t mirrorTest ::sai_test_mirror_session_egress_port_get (sai_object_id_t port_id,
            sai_object_list_t *obj_list) {

    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    attr.id = SAI_PORT_ATTR_EGRESS_MIRROR_SESSION;

    memcpy (&attr.value.objlist,obj_list,sizeof (sai_object_list_t));
    sai_rc = p_sai_port_api_tbl->get_port_attribute (port_id, 1, &attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        printf ("Get port attribute API failed with error: %d\r\n", sai_rc);
    } else {
        printf ("Get port attribute API success for port id: %lu\r\n", port_id);
    }

    return sai_rc;
}

void mirrorTest ::TearDownTestCase (void)
{
    EXPECT_EQ (SAI_STATUS_SUCCESS,p_sai_vlan_api_tbl->remove_vlan(sai_mirror_vlan_2));
    EXPECT_EQ (SAI_STATUS_SUCCESS,p_sai_vlan_api_tbl->remove_vlan(sai_mirror_vlan_3));
}

