
/*
 * filename: sai_lag_unit_test.h
 * (c) Copyright 2015 Dell Inc. All Rights Reserved.
 */

#ifndef __SAI_LAG_UNIT_TEST_H__
#define __SAI_LAG_UNIT_TEST_H__

#include "gtest/gtest.h"

extern "C" {
#include "saiswitch.h"
#include "saivlan.h"
#include "saistp.h"
#include "saitypes.h"
#include "sairouter.h"
#include "saibridge.h"

sai_status_t
sai_lag_ut_create_virtual_router (sai_virtual_router_api_t *vrf_api_table,
                                  sai_object_id_t          *vrf_id);

sai_status_t sai_lag_ut_add_port_to_lag (sai_lag_api_t   *lag_api,
                                         sai_object_id_t *member_id,
                                         sai_object_id_t  lag_id,
                                         sai_object_id_t  port_id,
                                         bool is_ing_disable_present,
                                         bool ing_disable,
                                         bool is_egr_disable_present,
                                         bool egr_disable);

sai_status_t sai_lag_ut_remove_port_from_lag (sai_lag_api_t   *lag_api,
                                              sai_object_id_t  member_id);

sai_status_t
sai_lag_ut_remove_virtual_router (sai_virtual_router_api_t *vrf_api_table,
                                  sai_object_id_t           vrf_id);

sai_status_t
sai_lag_ut_create_router_interface (sai_lag_api_t              *lag_api,
                                    sai_router_interface_api_t *router_intf_api_table,
                                    sai_vlan_api_t             *vlan_api_table,
                                    sai_bridge_api_t           *p_sai_bridge_api_tbl,
                                    sai_object_id_t            *rif_id,
                                    sai_object_id_t             vrf_id,
                                    sai_object_id_t             default_vlan_id,
                                    sai_object_id_t             port_id);

sai_status_t
sai_lag_ut_remove_router_interface (sai_router_interface_api_t *router_intf_api_table,
                                    sai_object_id_t             rif_id);

sai_status_t sai_lag_ut_get_lag_attr (sai_lag_api_t   *lag_api,
                                      sai_object_id_t  member_id,
                                      bool            *ing_disable,
                                      bool            *egr_disable);

sai_status_t sai_lag_ut_set_lag_attr (sai_lag_api_t   *lag_api,
                                      sai_object_id_t member_id,
                                      bool is_ing_disable_present,
                                      bool ing_disable,
                                      bool is_egr_disable_present,
                                      bool egr_disable);

sai_status_t sai_lag_ut_create_bridge_port(sai_bridge_api_t *p_sai_bridge_api_tbl,
                                           sai_object_id_t switch_id,
                                           sai_object_id_t port_id,
                                           sai_object_id_t *bridge_port_id);

sai_status_t sai_lag_ut_remove_bridge_port(sai_bridge_api_t *p_sai_bridge_api_tbl,
                                           sai_object_id_t switch_id,
                                           sai_object_id_t bridge_port_id);

sai_status_t sai_lag_get_ut_port_from_bridge_port(sai_bridge_api_t *p_sai_bridge_api_tbl,
                                                  sai_object_id_t bridge_port_id,
                                                  sai_object_id_t *port_id);
}

#define SAI_MAX_PORTS     256
#define SAI_LAG_UT_DEFAULT_VLAN 1

extern uint32_t bridge_port_count;
extern sai_object_id_t bridge_port_list[SAI_MAX_PORTS];
static sai_object_id_t switch_id = 0;
extern sai_object_id_t default_bridge_id;

/*
 * API query is done while running the first test case and
 * the method table is stored in sai_lag_api_table so
 * that its available for the rest of the test cases which
 * use the method table
 */
class lagInit : public ::testing::Test
{
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

            EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
                    (SAI_API_PORT, (static_cast<void**>
                                      (static_cast<void*>(&p_sai_port_api_tbl)))));

            ASSERT_TRUE (p_sai_port_api_tbl != NULL);

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

            sai_attr_set[0].id = SAI_SWITCH_ATTR_DEFAULT_VLAN_ID;
            EXPECT_EQ (SAI_STATUS_SUCCESS,
                    (p_sai_switch_api_tbl->get_switch_attribute(switch_id, 1,
                                                                &sai_attr_set[0])));

            default_vlan_id = sai_attr_set[0].value.oid;

            ASSERT_EQ(NULL,sai_api_query(SAI_API_LAG,
                        (static_cast<void**>(static_cast<void*>(&sai_lag_api_table)))));

            ASSERT_TRUE(sai_lag_api_table != NULL);

            EXPECT_TRUE(sai_lag_api_table->create_lag != NULL);
            EXPECT_TRUE(sai_lag_api_table->remove_lag != NULL);
            EXPECT_TRUE(sai_lag_api_table->set_lag_attribute != NULL);
            EXPECT_TRUE(sai_lag_api_table->get_lag_attribute != NULL);

            ASSERT_EQ(NULL,sai_api_query(SAI_API_VIRTUAL_ROUTER,
                        (static_cast<void**>(static_cast<void*>(&sai_vrf_api_table)))));

            ASSERT_TRUE(sai_vrf_api_table != NULL);

            EXPECT_TRUE(sai_vrf_api_table->create_virtual_router != NULL);
            EXPECT_TRUE(sai_vrf_api_table->remove_virtual_router != NULL);
            EXPECT_TRUE(sai_vrf_api_table->set_virtual_router_attribute != NULL);
            EXPECT_TRUE(sai_vrf_api_table->get_virtual_router_attribute != NULL);

            ASSERT_EQ(NULL,sai_api_query(SAI_API_VLAN,
                        (static_cast<void**>(static_cast<void*>(&sai_vlan_api_table)))));

            ASSERT_TRUE(sai_vlan_api_table != NULL);

            EXPECT_TRUE(sai_vlan_api_table->create_vlan != NULL);
            EXPECT_TRUE(sai_vlan_api_table->remove_vlan != NULL);
            EXPECT_TRUE(sai_vlan_api_table->set_vlan_attribute != NULL);
            EXPECT_TRUE(sai_vlan_api_table->get_vlan_attribute != NULL);
            EXPECT_TRUE(sai_vlan_api_table->create_vlan_member != NULL);
            EXPECT_TRUE(sai_vlan_api_table->remove_vlan_member != NULL);
            EXPECT_TRUE(sai_vlan_api_table->get_vlan_stats != NULL);
            EXPECT_TRUE(sai_vlan_api_table->set_vlan_member_attribute != NULL);
            EXPECT_TRUE(sai_vlan_api_table->get_vlan_member_attribute != NULL);

            ASSERT_EQ(NULL,sai_api_query(SAI_API_ROUTER_INTERFACE,
                        (static_cast<void**>(static_cast<void*>(&sai_router_intf_api_table)))));

            ASSERT_TRUE(sai_router_intf_api_table != NULL);

            EXPECT_TRUE(sai_router_intf_api_table->create_router_interface != NULL);
            EXPECT_TRUE(sai_router_intf_api_table->remove_router_interface != NULL);
            EXPECT_TRUE(sai_router_intf_api_table->set_router_interface_attribute != NULL);
            EXPECT_TRUE(sai_router_intf_api_table->get_router_interface_attribute != NULL);

            sai_attribute_t attr;
            sai_status_t sai_rc = SAI_STATUS_SUCCESS;

            memset (&attr, 0, sizeof (attr));
            attr.id = SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID;

            sai_rc = p_sai_switch_api_tbl->get_switch_attribute(switch_id,1,&attr);

            ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
            default_bridge_id = attr.value.oid;

            attr.id = SAI_BRIDGE_ATTR_PORT_LIST;
            attr.value.objlist.count = SAI_MAX_PORTS;
            attr.value.objlist.list = bridge_port_list;

            sai_rc = p_sai_bridge_api_tbl->get_bridge_attribute(default_bridge_id, 1, &attr);

            ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
            bridge_port_count = attr.value.objlist.count;
            ASSERT_EQ(SAI_STATUS_SUCCESS,
                      sai_lag_get_ut_port_from_bridge_port(p_sai_bridge_api_tbl,
                                                           bridge_port_list[0], &port_id_1));
            bridge_port_id_1 = bridge_port_list[0];

            ASSERT_EQ(SAI_STATUS_SUCCESS,
                      sai_lag_get_ut_port_from_bridge_port(p_sai_bridge_api_tbl,
                                                           bridge_port_list[2], &port_id_2));

            bridge_port_id_2 = bridge_port_list[2];

            ASSERT_EQ(SAI_STATUS_SUCCESS,
                      sai_lag_get_ut_port_from_bridge_port(p_sai_bridge_api_tbl,
                                                           bridge_port_list[bridge_port_count-1],
                                                          &port_id_invalid));
           port_id_invalid +=1;
        }

        static sai_lag_api_t* sai_lag_api_table;
        static sai_vlan_api_t* sai_vlan_api_table;
        static sai_virtual_router_api_t* sai_vrf_api_table;
        static sai_router_interface_api_t* sai_router_intf_api_table;
        static sai_bridge_api_t* p_sai_bridge_api_tbl;
        static sai_port_api_t*   p_sai_port_api_tbl;
        static sai_object_id_t  port_id_1;
        static sai_object_id_t  port_id_2;
        static sai_object_id_t  port_id_invalid;
        static sai_object_id_t  default_vlan_id;
        static sai_object_id_t  bridge_port_id_1;
        static sai_object_id_t  bridge_port_id_2;

    public:
        sai_lag_api_t *sai_lag_ut_get_lag_api () {return sai_lag_api_table;}
};
#endif /* __SAI_LAG_UNIT_TEST_H__ */
