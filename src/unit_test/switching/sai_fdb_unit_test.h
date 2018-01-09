/*
 * filename: sai_fdb_unit_test.cpp
 * (c) Copyright 2015 Dell Inc. All Rights Reserved.
 */

/*
 * sai_fdb_unit_test.h
 *
 */

#ifndef __SAI_FDB_UNIT_TEST_H__
#define __SAI_FDB_UNIT_TEST_H__

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
#include "sailag.h"
#include "saibridge.h"
}

#define SAI_MAX_PORTS  256
#define SAI_MAX_FDB_TEST_ATTRIBUTES 3
static sai_object_id_t switch_id = 0;

extern uint32_t port_count;
extern sai_object_id_t bridge_port_list[];;


static inline void sai_set_test_fdb_entry(sai_fdb_entry_t* fdb_entry,
                                          sai_object_id_t bridge_port_id)
{
    sai_attribute_t attr;
    sai_bridge_api_t *p_sai_bridge_api_tbl = NULL;

    memset(fdb_entry,0, sizeof(sai_fdb_entry_t));
    sai_api_query (SAI_API_BRIDGE, (static_cast<void**>
                  (static_cast<void*>(&p_sai_bridge_api_tbl))));

    ASSERT_TRUE (p_sai_bridge_api_tbl != NULL);

    attr.id = SAI_BRIDGE_PORT_ATTR_TYPE;
    p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id, 1, &attr);

    if(attr.value.s32 == SAI_BRIDGE_PORT_TYPE_PORT) {
        attr.id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
        p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id, 1, &attr);
        if(sai_object_type_query(attr.value.oid) == SAI_OBJECT_TYPE_LAG) {
            fdb_entry->mac_address[5] = 0xb;
        } else {
            fdb_entry->mac_address[5] = 0xa;
        }
    }
    fdb_entry->bv_id = SAI_GTEST_VLAN_OBJ;
}

/*
 * API query is done while running the first test case and
 * the method table is stored in sai_fdb_api_table so
 * that its available for the rest of the test cases which
 * use the method table
 */
class fdbInit : public ::testing::Test
{
    public:
        void sai_fdb_entry_create(sai_fdb_entry_type_t type, sai_object_id_t bridge_port_id,
                sai_packet_action_t action);
        void sai_fdb_create_registered_entry (sai_fdb_entry_t entry,
                sai_fdb_entry_type_t type,
                sai_object_id_t bridge_port_id,
                sai_packet_action_t action);
        static sai_status_t sai_get_fdb_bridge_port_list_get(sai_switch_api_t *p_sai_switch_api_tbl,
                                                             sai_bridge_api_t *p_sai_bridge_api_tbl);
    protected:
        static void SetUpTestCase()
        {
            sai_switch_api_t *p_sai_switch_api_tbl = NULL;

            /*
             * Query and populate the SAI Switch API Table.
             */
            EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
                    (SAI_API_SWITCH, (static_cast<void**>
                                      (static_cast<void*>(&p_sai_switch_api_tbl)))));

            ASSERT_TRUE (p_sai_switch_api_tbl != NULL);

            EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
                    (SAI_API_BRIDGE, (static_cast<void**>
                                      (static_cast<void*>(&p_sai_bridge_api_tbl)))));

            ASSERT_TRUE (p_sai_bridge_api_tbl != NULL);

            ASSERT_EQ(NULL,sai_api_query(SAI_API_FDB,
                        (static_cast<void**>(static_cast<void*>(&sai_fdb_api_table)))));

            ASSERT_TRUE(sai_fdb_api_table != NULL);


            EXPECT_TRUE(sai_fdb_api_table->create_fdb_entry != NULL);
            EXPECT_TRUE(sai_fdb_api_table->remove_fdb_entry != NULL);
            EXPECT_TRUE(sai_fdb_api_table->get_fdb_entry_attribute != NULL);
            EXPECT_TRUE(sai_fdb_api_table->set_fdb_entry_attribute != NULL);
            EXPECT_TRUE(sai_fdb_api_table->flush_fdb_entries != NULL);

            ASSERT_EQ(NULL,sai_api_query(SAI_API_LAG,
                        (static_cast<void**>(static_cast<void*>(&sai_lag_api_table)))));


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

            ASSERT_TRUE(sai_lag_api_table != NULL);

            EXPECT_TRUE(sai_lag_api_table->create_lag != NULL);
            EXPECT_TRUE(sai_lag_api_table->remove_lag != NULL);
            EXPECT_TRUE(sai_lag_api_table->set_lag_attribute != NULL);
            EXPECT_TRUE(sai_lag_api_table->get_lag_attribute != NULL);

            EXPECT_EQ (SAI_STATUS_SUCCESS,sai_get_fdb_bridge_port_list_get(p_sai_switch_api_tbl,
                                                                           p_sai_bridge_api_tbl));
            bridge_port_id_1 = bridge_port_list[0];
            bridge_port_id_2 = bridge_port_list[port_count-1];
            bridge_port_id_3 = bridge_port_list[1];
            bridge_port_id_4 = bridge_port_list[2];
        }
        static sai_bridge_api_t *p_sai_bridge_api_tbl;
        static sai_fdb_api_t* sai_fdb_api_table;
        static sai_lag_api_t* sai_lag_api_table;
        static sai_object_id_t  bridge_port_id_1;
        static sai_object_id_t  bridge_port_id_2;
        static sai_object_id_t  bridge_port_id_3;
        static sai_object_id_t  bridge_port_id_4;
};

#endif
