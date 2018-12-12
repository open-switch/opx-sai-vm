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
 * filename: sai_fdb_unit_test.cpp
 */

/*
 * sai_fdb_unit_test.cpp
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include "inttypes.h"

extern "C" {
#include "sai.h"
#include "saifdb.h"
#include "saitypes.h"
#include "saiswitch.h"
#include "sai_l2_unit_test_defs.h"
#include "sai_fdb_main.h"
#include "sai_fdb_unit_test.h"
}

#define MAX_FDB_NOTIFICATIONS 50
sai_fdb_internal_notification_data_t notification_data[MAX_FDB_NOTIFICATIONS];
uint_t fdb_num_notifications;
bool notification_wait = true;

static inline void sai_set_test_registered_entry(uint8_t last_octet,sai_fdb_entry_t* fdb_entry)
{
    memset(fdb_entry,0, sizeof(sai_fdb_entry_t));
    fdb_entry->mac_address[5] = last_octet;
    fdb_entry->bv_id = SAI_GTEST_VLAN_OBJ;
}

sai_status_t sai_fdb_test_internal_callback(uint_t num_notifications,
                                                   sai_fdb_internal_notification_data_t *data)
{
    fdb_num_notifications = num_notifications;
    printf("Received :%d notifications\r\n",num_notifications);
    memcpy(notification_data, data, sizeof(sai_fdb_internal_notification_data_t)*num_notifications);
    notification_wait = false;
    return SAI_STATUS_SUCCESS;
}

void fdbInit ::sai_fdb_create_registered_entry (sai_fdb_entry_t fdb_entry,
                                                  sai_fdb_entry_type_t type,
                                                  sai_object_id_t bridge_port_id,
                                                  sai_packet_action_t action)
{
    sai_attribute_t attr_list[SAI_MAX_FDB_TEST_ATTRIBUTES];

    memset(attr_list,0, sizeof(attr_list));
    attr_list[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
    attr_list[0].value.s32 = type;

    attr_list[1].id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID;
    attr_list[1].value.oid = bridge_port_id;

    attr_list[2].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION;
    attr_list[2].value.s32 = action;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_fdb_api_table->create_fdb_entry(
                                 (const sai_fdb_entry_t*)&fdb_entry,
                                 SAI_MAX_FDB_TEST_ATTRIBUTES,
                                 (const sai_attribute_t*)attr_list));
}

TEST_F(fdbInit, sai_fdb_single_notification_callback)
{
    sai_fdb_entry_t fdb_entry;


    sai_set_test_registered_entry(0xa,&fdb_entry);
    sai_l2_fdb_register_internal_callback(sai_fdb_test_internal_callback);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_l2_register_fdb_entry(&fdb_entry));
    sai_fdb_create_registered_entry(fdb_entry,SAI_FDB_ENTRY_TYPE_DYNAMIC, bridge_port_id_1, SAI_PACKET_ACTION_FORWARD);

    while(notification_wait) {
        usleep(1);
    }
    notification_wait = true;

    ASSERT_EQ( notification_data[0].bridge_port_id, bridge_port_id_1);
    ASSERT_EQ( notification_data[0].fdb_event, SAI_FDB_EVENT_LEARNED);
    ASSERT_EQ( 0, memcmp(&fdb_entry,&notification_data[0].fdb_entry, sizeof(sai_fdb_entry_t)));
    ASSERT_EQ( 1, fdb_num_notifications);

    fdb_num_notifications = 0;
    memset(notification_data, 0, sizeof(notification_data));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_fdb_api_table->remove_fdb_entry(
                                 (const sai_fdb_entry_t*)&fdb_entry));

    while(notification_wait) {
        usleep(1);
    }
    notification_wait = true;

    ASSERT_EQ( notification_data[0].fdb_event, SAI_FDB_EVENT_FLUSHED);
    ASSERT_EQ( 0, memcmp(&fdb_entry,&notification_data[0].fdb_entry, sizeof(sai_fdb_entry_t)));
    ASSERT_EQ( 1, fdb_num_notifications);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_l2_deregister_fdb_entry(&fdb_entry));
}

TEST_F(fdbInit, sai_fdb_multi_notification_callback)
{
    sai_fdb_entry_t fdb_entry1;
    sai_fdb_entry_t fdb_entry2;
    sai_attribute_t flush_attr[1];

    sai_set_test_registered_entry(0xa,&fdb_entry1);
    sai_set_test_registered_entry(0xb,&fdb_entry2);

    sai_l2_fdb_register_internal_callback(sai_fdb_test_internal_callback);

    sai_fdb_create_registered_entry(fdb_entry1, SAI_FDB_ENTRY_TYPE_DYNAMIC, bridge_port_id_1, SAI_PACKET_ACTION_FORWARD);
    sai_fdb_create_registered_entry(fdb_entry2, SAI_FDB_ENTRY_TYPE_DYNAMIC, bridge_port_id_2, SAI_PACKET_ACTION_FORWARD);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_l2_register_fdb_entry(&fdb_entry1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_l2_register_fdb_entry(&fdb_entry2));

    flush_attr[0].id = SAI_FDB_FLUSH_ATTR_BV_ID;
    flush_attr[0].value.oid =  SAI_GTEST_VLAN_OBJ;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_fdb_api_table->flush_fdb_entries(switch_id, 1,
                                 (const sai_attribute_t*)flush_attr));

    while(notification_wait) {
        usleep(1);
    }
    notification_wait = true;

    ASSERT_EQ( 2, fdb_num_notifications);

    ASSERT_EQ( notification_data[0].fdb_event, SAI_FDB_EVENT_FLUSHED);
    ASSERT_EQ( notification_data[1].fdb_event, SAI_FDB_EVENT_FLUSHED);

    fdb_num_notifications = 0;
    memset(notification_data, 0, sizeof(notification_data));


    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_l2_deregister_fdb_entry(&fdb_entry1));

    sai_fdb_create_registered_entry(fdb_entry1, SAI_FDB_ENTRY_TYPE_DYNAMIC, bridge_port_id_1, SAI_PACKET_ACTION_FORWARD);
    sai_fdb_create_registered_entry(fdb_entry2, SAI_FDB_ENTRY_TYPE_DYNAMIC, bridge_port_id_2, SAI_PACKET_ACTION_FORWARD);

    while(notification_wait) {
        usleep(1);
    }
    notification_wait = true;
    ASSERT_EQ( notification_data[0].bridge_port_id, bridge_port_id_2);
    ASSERT_EQ( notification_data[0].fdb_event, SAI_FDB_EVENT_LEARNED);
    ASSERT_EQ( 0, memcmp(&fdb_entry2,&notification_data[0].fdb_entry, sizeof(sai_fdb_entry_t)));
    ASSERT_EQ( 1, fdb_num_notifications);

    fdb_num_notifications = 0;
    memset(notification_data, 0, sizeof(notification_data));

    flush_attr[0].id = SAI_FDB_FLUSH_ATTR_BV_ID;
    flush_attr[0].value.oid =  SAI_GTEST_VLAN_OBJ;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_fdb_api_table->flush_fdb_entries(switch_id, 1,
                                 (const sai_attribute_t*)flush_attr));

    while(notification_wait) {
        usleep(1);
    }
    notification_wait = true;

    ASSERT_EQ( 1, fdb_num_notifications);
    ASSERT_EQ( notification_data[0].fdb_event, SAI_FDB_EVENT_FLUSHED);
    ASSERT_EQ( 0, memcmp(&fdb_entry2,&notification_data[0].fdb_entry, sizeof(sai_fdb_entry_t)));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_l2_deregister_fdb_entry(&fdb_entry2));
}

TEST_F(fdbInit, sai_fdb_port_change_notification_callback)
{
    sai_fdb_entry_t fdb_entry;
    sai_attribute_t set_attr;


    sai_set_test_registered_entry(0xa,&fdb_entry);
    sai_fdb_create_registered_entry(fdb_entry,SAI_FDB_ENTRY_TYPE_DYNAMIC, bridge_port_id_1, SAI_PACKET_ACTION_FORWARD);
    sai_l2_fdb_register_internal_callback(sai_fdb_test_internal_callback);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_l2_register_fdb_entry(&fdb_entry));

    set_attr.id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID;
    set_attr.value.oid = bridge_port_id_2;

    EXPECT_EQ(SAI_STATUS_SUCCESS,
              sai_fdb_api_table->set_fdb_entry_attribute(
                                 (const sai_fdb_entry_t*)&fdb_entry,
                                 (const sai_attribute_t*)&set_attr));

    while(notification_wait) {
        usleep(1);
    }
    notification_wait = true;

    ASSERT_EQ( notification_data[0].bridge_port_id, bridge_port_id_2);
    ASSERT_EQ( notification_data[0].fdb_event, SAI_FDB_EVENT_LEARNED);
    ASSERT_EQ( 0, memcmp(&fdb_entry,&notification_data[0].fdb_entry, sizeof(sai_fdb_entry_t)));
    ASSERT_EQ( 1, fdb_num_notifications);

    fdb_num_notifications = 0;
    memset(notification_data, 0, sizeof(notification_data));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_fdb_api_table->remove_fdb_entry(
                                 (const sai_fdb_entry_t*)&fdb_entry));

    while(notification_wait) {
        usleep(1);
    }
    notification_wait = true;

    ASSERT_EQ( notification_data[0].fdb_event, SAI_FDB_EVENT_FLUSHED);
    ASSERT_EQ( 0, memcmp(&fdb_entry,&notification_data[0].fdb_entry, sizeof(sai_fdb_entry_t)));
    ASSERT_EQ( 1, fdb_num_notifications);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_l2_deregister_fdb_entry(&fdb_entry));
}
