/*
 * filename:
"src/unit_test/sai_bridge_unit_test.h
 * (c) Copyright 2017 Dell Inc. All Rights Reserved.
 */

/*
 * sai_bridge_unit_test.h
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include <inttypes.h>

#include "sai_bridge_unit_test_utils.h"

extern "C" {
#include "sai.h"
#include "saitypes.h"
#include "saibridge.h"
#include "saiswitch.h"
#include "sailag.h"
#include "saifdb.h"
#include "saivlan.h"
}

extern uint32_t bridge_port_count;
extern sai_object_id_t bridge_port_list[SAI_MAX_BRIDGE_PORTS];
extern sai_object_id_t switch_id;
extern sai_object_id_t default_bridge_id;
extern sai_object_id_t port_list[SAI_MAX_BRIDGE_PORTS];
extern uint32_t port_count;

/*
 *
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
                                                  switchstate)
{
}

/* Packet event callback
 */
static inline void sai_packet_event_callback (const void *buffer,
                                              sai_size_t buffer_size,
                                              uint32_t attr_count,
                                              const sai_attribute_t *attr_list)
{
}


static inline void  sai_switch_shutdown_callback (void)
{
}
class bridgeTest : public ::testing::Test
{
    protected:
        static void SetUpTestCase()
        {

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

            sai_api_query (SAI_API_BRIDGE, (static_cast<void**>
                        (static_cast<void*>(&p_sai_bridge_api_tbl))));

            ASSERT_TRUE (p_sai_bridge_api_tbl != NULL);

            sai_api_query (SAI_API_LAG, (static_cast<void**>
                        (static_cast<void*>(&p_sai_lag_api_tbl))));

            ASSERT_TRUE (p_sai_lag_api_tbl != NULL);

            sai_api_query (SAI_API_FDB, (static_cast<void**>
                        (static_cast<void*>(&p_sai_fdb_api_tbl))));

            ASSERT_TRUE (p_sai_fdb_api_tbl != NULL);

            ASSERT_EQ(NULL,sai_api_query(SAI_API_L2MC_GROUP,
                        (static_cast<void**>(static_cast<void*>(&p_sai_l2mc_group_api_tbl)))));

            ASSERT_TRUE (p_sai_l2mc_group_api_tbl != NULL);

            ASSERT_EQ(NULL,sai_api_query(SAI_API_VLAN,
                        (static_cast<void**>(static_cast<void*>(&p_sai_vlan_api_tbl)))));


            EXPECT_TRUE(p_sai_bridge_api_tbl->create_bridge != NULL);
            EXPECT_TRUE(p_sai_bridge_api_tbl->remove_bridge != NULL);
            EXPECT_TRUE(p_sai_bridge_api_tbl->set_bridge_attribute != NULL);
            EXPECT_TRUE(p_sai_bridge_api_tbl->get_bridge_attribute != NULL);
            EXPECT_TRUE(p_sai_bridge_api_tbl->get_bridge_stats != NULL);
            EXPECT_TRUE(p_sai_bridge_api_tbl->clear_bridge_stats != NULL);
            EXPECT_TRUE(p_sai_bridge_api_tbl->create_bridge_port != NULL);
            EXPECT_TRUE(p_sai_bridge_api_tbl->remove_bridge_port != NULL);
            EXPECT_TRUE(p_sai_bridge_api_tbl->set_bridge_port_attribute != NULL);
            EXPECT_TRUE(p_sai_bridge_api_tbl->get_bridge_port_attribute != NULL);
            EXPECT_TRUE(p_sai_bridge_api_tbl->get_bridge_port_stats != NULL);
            EXPECT_TRUE(p_sai_bridge_api_tbl->clear_bridge_port_stats != NULL);

            sai_attribute_t attr;
            sai_status_t ret;
            memset (&attr, 0, sizeof (attr));

            attr.id = SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID;

            ret = p_sai_switch_api_tbl->get_switch_attribute(switch_id,1,&attr);
            ASSERT_EQ (SAI_STATUS_SUCCESS, ret);

            default_bridge_id = attr.value.oid;

            attr.id = SAI_BRIDGE_ATTR_PORT_LIST;
            attr.value.objlist.count = SAI_MAX_BRIDGE_PORTS;
            attr.value.objlist.list = bridge_port_list;

            ret = p_sai_bridge_api_tbl->get_bridge_attribute(default_bridge_id, 1, &attr);
            ASSERT_EQ (SAI_STATUS_SUCCESS, ret);

            bridge_port_count = attr.value.objlist.count;

            memset (&attr, 0, sizeof (attr));

            attr.id = SAI_SWITCH_ATTR_PORT_LIST;
            attr.value.objlist.count = SAI_MAX_BRIDGE_PORTS;
            attr.value.objlist.list  = port_list;

            ret = p_sai_switch_api_tbl->get_switch_attribute(0,1,&attr);
            port_count = attr.value.objlist.count;

            EXPECT_EQ (SAI_STATUS_SUCCESS,ret);
        }

        static sai_bridge_api_t* p_sai_bridge_api_tbl;
        static sai_switch_api_t *p_sai_switch_api_tbl;
        static sai_lag_api_t *p_sai_lag_api_tbl;
        static sai_fdb_api_t *p_sai_fdb_api_tbl;
        static sai_l2mc_group_api_t *p_sai_l2mc_group_api_tbl;
        static sai_vlan_api_t *p_sai_vlan_api_tbl;
};

