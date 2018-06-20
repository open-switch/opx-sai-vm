/*
 * filename: sai_samplepacket_unit_test_utils.cpp
 * (c) Copyright 2015 Dell Inc. All Rights Reserved.
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "gtest/gtest.h"
#include "stdarg.h"
#include "sai_samplepacket_unit_test.h"
#include "sai_port_breakout_test_utils.h"

extern "C" {
#include "sai.h"
#include "saistatus.h"
#include "saitypes.h"
#include "saiswitch.h"
#include "saisamplepacket.h"
#include "sai_samplepacket_api.h"
}

#define SAI_MAX_PORTS  256

static uint32_t port_count = 0;

static sai_object_id_t port_list[SAI_MAX_PORTS] = {0};

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

void samplepacketTest ::SetUpTestCase (void) {


    /*
     * Query and populate the SAI Switch API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
            (SAI_API_SWITCH, (static_cast<void**>
                              (static_cast<void*>(&p_sai_switch_api_tbl)))));

    ASSERT_TRUE (p_sai_switch_api_tbl != NULL);


    sai_attribute_t sai_attr_set[7];
    uint32_t attr_count = 7;

    memset(sai_attr_set,0, sizeof(sai_attr_set));

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
     * Query and populate the SAI SamplePacket API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
            (SAI_API_SAMPLEPACKET, (static_cast<void**>
                                    (static_cast<void*>
                                     (&p_sai_samplepacket_api_tbl)))));

    ASSERT_TRUE (p_sai_samplepacket_api_tbl != NULL);

    /*
     * Query and populate the SAI Acl API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
            (SAI_API_ACL, (static_cast<void**>
                           (static_cast<void*>
                            (&p_sai_acl_api_tbl)))));

    ASSERT_TRUE (p_sai_acl_api_tbl != NULL);

    sai_attribute_t sai_port_attr;
    sai_status_t ret = SAI_STATUS_SUCCESS;

    memset (&sai_port_attr, 0, sizeof (sai_port_attr));

    sai_port_attr.id = SAI_SWITCH_ATTR_PORT_LIST;
    sai_port_attr.value.objlist.count = SAI_MAX_PORTS;
    sai_port_attr.value.objlist.list  = port_list;

    ret = p_sai_switch_api_tbl->get_switch_attribute(0,1,&sai_port_attr);
    port_count = sai_port_attr.value.objlist.count;
    EXPECT_EQ (SAI_STATUS_SUCCESS,ret);
    port_id_1 = port_list[0];
    port_id_2 = port_list[port_count-1];

}

sai_switch_api_t* samplepacketTest ::p_sai_switch_api_tbl = NULL;
sai_port_api_t* samplepacketTest ::p_sai_port_api_tbl = NULL;
sai_samplepacket_api_t* samplepacketTest ::p_sai_samplepacket_api_tbl = NULL;
sai_acl_api_t* samplepacketTest ::p_sai_acl_api_tbl = NULL;
sai_object_id_t samplepacketTest ::port_id_1 = 0;
sai_object_id_t samplepacketTest ::port_id_2 = 0;
sai_object_id_t samplepacketTest ::switch_id = 0;

sai_status_t samplepacketTest ::sai_test_samplepacket_session_create (sai_object_id_t *p_session_id,
        uint32_t attr_count,
        sai_attribute_t *p_attr_list)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;


    printf ("Testing Session Create API with attribute count: %d\r\n", attr_count);

    sai_rc = p_sai_samplepacket_api_tbl->create_samplepacket(p_session_id, switch_id,
                                                                      attr_count, p_attr_list);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        printf ("SAI SamplePacket session Creation API failed with error: %d\r\n", sai_rc);
    } else {
        printf ("SAI SamplePacket session Creation API success, session ID: %lu\r\n", *p_session_id);
    }

    return sai_rc;
}

sai_status_t samplepacketTest ::sai_test_samplepacket_session_destroy (sai_object_id_t session_id) {
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    sai_rc = p_sai_samplepacket_api_tbl->remove_samplepacket(session_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        printf ("SamplePacket Session destroy API failed with error: %d\r\n", sai_rc);
    } else {
        printf ("SamplePacket Session API success for session id: %lu\r\n", session_id);
    }

    return sai_rc;
}

sai_status_t samplepacketTest ::sai_test_samplepacket_session_ingress_port_add (sai_object_id_t port_id,
            sai_object_id_t object) {

    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    attr.id = SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE;
    attr.value.oid = object;

    sai_rc = p_sai_port_api_tbl->set_port_attribute (port_id,&attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        printf ("Set port attribute API failed with error: %d\r\n", sai_rc);
    } else {
        printf ("Set port attribute API success for port id: %lu\r\n", port_id);
    }

    return sai_rc;
}

sai_status_t samplepacketTest ::sai_test_samplepacket_session_egress_port_add (sai_object_id_t port_id,
            sai_object_id_t object) {

    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    attr.id = SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE;
    attr.value.oid = object;

    sai_rc = p_sai_port_api_tbl->set_port_attribute (port_id,&attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        printf ("Set port attribute API failed with error: %d\r\n", sai_rc);
    } else {
        printf ("Set port attribute API success for port id: %lu\r\n", port_id);
    }

    return sai_rc;
}

