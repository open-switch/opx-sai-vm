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
 * sai_vlan_unit_test.cpp
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
#include "saivlan.h"
#include "saitypes.h"
#include "saibridge.h"
#include "saiswitch.h"
#include "sai_switch_utils.h"
#include "sai_l2_unit_test_defs.h"
#include "sai_vlan_common.h"
#include "sai_vlan_api.h"
}

#define SAI_MAX_PORTS  256

static uint32_t bridge_port_count = 0;
static sai_object_id_t bridge_port_list[SAI_MAX_PORTS] = {0};
static sai_object_id_t switch_id = 0;
static sai_object_id_t default_bridge_id = 0;
static sai_object_id_t default_vlan_id = 0;

/*
 * API query is done while running the first test case and
 * the method table is stored in sai_vlan_api_table so
 * that its available for the rest of the test cases which
 * use the method table
 */
class vlanTest : public ::testing::Test
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

            sai_api_query (SAI_API_BRIDGE, (static_cast<void**>
                        (static_cast<void*>(&p_sai_bridge_api_tbl))));

            ASSERT_TRUE (p_sai_bridge_api_tbl != NULL);

            sai_attribute_t attr;
            sai_status_t ret;
            memset (&attr, 0, sizeof (attr));

            attr.id = SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID;

            ret = p_sai_switch_api_tbl->get_switch_attribute(switch_id,1,&attr);
            ASSERT_EQ (SAI_STATUS_SUCCESS, ret);

            default_bridge_id = attr.value.oid;

            attr.id = SAI_BRIDGE_ATTR_PORT_LIST;
            attr.value.objlist.count = SAI_MAX_PORTS;
            attr.value.objlist.list = bridge_port_list;

            ret = p_sai_bridge_api_tbl->get_bridge_attribute(default_bridge_id, 1, &attr);
            ASSERT_EQ (SAI_STATUS_SUCCESS, ret);

            bridge_port_count = attr.value.objlist.count;

            bridge_port_id_1 = bridge_port_list[0];
            bridge_port_id_2 = bridge_port_list[bridge_port_count-1];
            bridge_port_id_invalid = SAI_NULL_OBJECT_ID;
        }

        static sai_vlan_api_t* sai_vlan_api_table;
        static sai_object_id_t  bridge_port_id_1;
        static sai_object_id_t  bridge_port_id_2;
        static sai_object_id_t  bridge_port_id_invalid;
        static sai_bridge_api_t *p_sai_bridge_api_tbl;
        static sai_switch_api_t *p_sai_switch_api_tbl;
};

sai_bridge_api_t* vlanTest::p_sai_bridge_api_tbl = NULL;
sai_switch_api_t* vlanTest::p_sai_switch_api_tbl = NULL;
sai_vlan_api_t* vlanTest::sai_vlan_api_table = NULL;
sai_object_id_t vlanTest ::bridge_port_id_1 = 0;
sai_object_id_t vlanTest ::bridge_port_id_2 = 0;
sai_object_id_t vlanTest ::bridge_port_id_invalid = 0;


bool sai_vlan_ut_all_bridge_ports_in_default_vlan(sai_vlan_api_t *sai_vlan_api_table,
                                                  uint32_t count, sai_object_id_t *vlan_member_list)
{
    uint32_t bp_idx = 0;
    uint32_t vm_idx = 0;
    sai_attribute_t attr;

    for(bp_idx = 0 ; bp_idx < bridge_port_count;  bp_idx++) {
        for(vm_idx = 0; vm_idx < count; vm_idx++) {
            attr.id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
            sai_vlan_api_table->get_vlan_member_attribute(vlan_member_list[vm_idx],1,&attr);
            if(attr.value.oid == bridge_port_list[bp_idx]) {
                printf("Vlan member 0x%" PRIx64 " Bridge port 0x%" PRIx64 "\r\n",
                       vlan_member_list[vm_idx],bridge_port_list[bp_idx]);
                break;
            }
        }
        if(vm_idx == count) {
            printf("Bridge port id 0x%" PRIx64 " is not in default vlan\r\n",
                   bridge_port_list[bp_idx]);
            return false;
        }
    }
    return true;
}

TEST_F(vlanTest, default_vlan_test)
{
    sai_attribute_t   attr;
    sai_object_id_t vlan_member_list[SAI_MAX_PORTS];

    attr.id = SAI_SWITCH_ATTR_DEFAULT_VLAN_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_switch_api_tbl->get_switch_attribute(switch_id,1,&attr));


    default_vlan_id = attr.value.oid;
    attr.id = SAI_VLAN_ATTR_MEMBER_LIST;
    attr.value.objlist.count = SAI_MAX_PORTS;
    attr.value.objlist.list = vlan_member_list;

    EXPECT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->get_vlan_attribute(default_vlan_id,1,
                                                     &attr));
    ASSERT_EQ(true,
              sai_vlan_ut_all_bridge_ports_in_default_vlan(sai_vlan_api_table,
                                                           attr.value.objlist.count,
                                                           vlan_member_list));
}
/*
 * VLAN create and delete
 */
TEST_F(vlanTest, create_remove_vlan)
{
    sai_attribute_t attr;
    sai_object_id_t vlan_obj_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t vlan_obj_id_fail = SAI_NULL_OBJECT_ID;

    attr.id = SAI_VLAN_ATTR_VLAN_ID;
    attr.value.u16 = SAI_GTEST_VLAN;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->create_vlan(&vlan_obj_id,0,1,&attr));

    attr.id = SAI_VLAN_ATTR_VLAN_ID;
    attr.value.u16 = SAI_GTEST_VLAN;
    ASSERT_EQ(SAI_STATUS_ITEM_ALREADY_EXISTS,
              sai_vlan_api_table->create_vlan(&vlan_obj_id_fail,0,1,&attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->remove_vlan(vlan_obj_id));

    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_vlan_api_table->remove_vlan(vlan_obj_id));

    vlan_obj_id = SAI_NULL_OBJECT_ID;
    attr.id = SAI_VLAN_ATTR_VLAN_ID;
    attr.value.u16 = SAI_GTEST_INVALID_VLAN_ID;
    ASSERT_EQ(SAI_STATUS_INVALID_VLAN_ID,
              sai_vlan_api_table->create_vlan(&vlan_obj_id,0,1,&attr));

    ASSERT_EQ(SAI_STATUS_INVALID_OBJECT_ID,
              sai_vlan_api_table->remove_vlan(vlan_obj_id));
}

/*
 * VLAN add port and remove port
 */
TEST_F(vlanTest, vlan_port_test)
{
    sai_attribute_t attr[SAI_GTEST_VLAN_MEMBER_ATTR_COUNT];
    sai_object_id_t vlan_obj_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t vlan_mem_obj_id1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t vlan_mem_obj_id2 = SAI_NULL_OBJECT_ID;
    sai_object_id_t vlan_mem_obj_fail_id1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t vlan_mem_obj_fail_id2 = SAI_NULL_OBJECT_ID;
    uint32_t attr_count = 0;

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_ATTR_VLAN_ID;
    attr[attr_count].value.u16 = SAI_GTEST_VLAN;
    attr_count++;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->create_vlan(&vlan_obj_id,0,attr_count,attr));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr[attr_count].value.oid = vlan_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr[attr_count].value.oid = bridge_port_id_1;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr[attr_count].value.u32 = SAI_VLAN_TAGGING_MODE_UNTAGGED;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->create_vlan_member(&vlan_mem_obj_id1,
                                                    0,
                                                    attr_count,
                                                    attr));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr[attr_count].value.oid = vlan_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr[attr_count].value.oid = bridge_port_id_2;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr[attr_count].value.u32 = SAI_VLAN_TAGGING_MODE_TAGGED;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->create_vlan_member(&vlan_mem_obj_id2,
                                                    0,
                                                    attr_count,
                                                    attr));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_vlan_api_table->get_vlan_member_attribute(vlan_mem_obj_id1,
                                                    attr_count,
                                                    attr));
    ASSERT_EQ(attr[attr_count].value.u32,SAI_VLAN_TAGGING_MODE_UNTAGGED);

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_vlan_api_table->get_vlan_member_attribute(vlan_mem_obj_id2,
                                                    attr_count,
                                                    attr));
    attr_count = 0;
    ASSERT_EQ(attr[0].value.u32,SAI_VLAN_TAGGING_MODE_TAGGED);

    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr[attr_count].value.u32 = SAI_VLAN_TAGGING_MODE_TAGGED;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_vlan_api_table->set_vlan_member_attribute(vlan_mem_obj_id1,
                                                    &attr[attr_count]));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr[attr_count].value.u32 = SAI_VLAN_TAGGING_MODE_UNTAGGED;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_vlan_api_table->set_vlan_member_attribute(vlan_mem_obj_id2,
                                                    &attr[attr_count]));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_vlan_api_table->get_vlan_member_attribute(vlan_mem_obj_id1,
                                                    attr_count,
                                                    attr));
    attr_count = 0;
    ASSERT_EQ(attr[attr_count].value.u32,SAI_VLAN_TAGGING_MODE_TAGGED);

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_vlan_api_table->get_vlan_member_attribute(vlan_mem_obj_id2,
                                                    attr_count,
                                                    attr));
    attr_count = 0;
    ASSERT_EQ(attr[attr_count].value.u32,SAI_VLAN_TAGGING_MODE_UNTAGGED);

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr[attr_count].value.oid = vlan_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr[attr_count].value.oid = bridge_port_id_1;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr[attr_count].value.u32 = SAI_VLAN_TAGGING_MODE_TAGGED;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_ITEM_ALREADY_EXISTS,
              sai_vlan_api_table->create_vlan_member(&vlan_mem_obj_fail_id1,
                                                    0,
                                                    attr_count,
                                                    attr));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr[attr_count].value.oid = vlan_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr[attr_count].value.oid = bridge_port_id_2;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr[attr_count].value.u32 = SAI_VLAN_TAGGING_MODE_UNTAGGED;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_ITEM_ALREADY_EXISTS,
              sai_vlan_api_table->create_vlan_member(&vlan_mem_obj_fail_id2,
                                                    0,
                                                    attr_count,
                                                    attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->remove_vlan_member(vlan_mem_obj_id1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->remove_vlan_member(vlan_mem_obj_id2));

    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_vlan_api_table->remove_vlan_member(vlan_mem_obj_id1));

    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_vlan_api_table->remove_vlan_member(vlan_mem_obj_id2));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr[attr_count].value.oid = bridge_port_id_invalid;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr[attr_count].value.oid = vlan_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr[attr_count].value.u32 = SAI_VLAN_TAGGING_MODE_TAGGED;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTR_VALUE_0,
              sai_vlan_api_table->create_vlan_member(&vlan_mem_obj_fail_id1,
                                                    0,
                                                    attr_count,
                                                    attr));

    ASSERT_EQ(SAI_STATUS_INVALID_OBJECT_ID,
                  sai_vlan_api_table->remove_vlan_member(vlan_mem_obj_fail_id1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->remove_vlan(vlan_obj_id));
}
/*
 * VLAN get port test
 */
TEST_F(vlanTest, vlan_get_port)
{
    sai_attribute_t attr[SAI_GTEST_VLAN_MEMBER_ATTR_COUNT];
    sai_object_id_t vlan_obj_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t vlan_mem_obj_id1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t vlan_mem_obj_id2 = SAI_NULL_OBJECT_ID;
    sai_object_id_t port_obj_list[SAI_GTEST_VLAN_MEMBER_PORT_COUNT];
    //sai_vlan_member_node_t *vlan_node = NULL;
    uint32_t attr_count = 0;

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_ATTR_VLAN_ID;
    attr[attr_count].value.u16 = SAI_GTEST_VLAN;
    attr_count++;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->create_vlan(&vlan_obj_id,0,attr_count,attr));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr[attr_count].value.oid = vlan_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr[attr_count].value.oid = bridge_port_id_1;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr[attr_count].value.u32 = SAI_VLAN_TAGGING_MODE_UNTAGGED;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->create_vlan_member(&vlan_mem_obj_id1,
                                                    0,
                                                    attr_count,
                                                    attr));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr[attr_count].value.oid = vlan_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr[attr_count].value.oid = bridge_port_id_2;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr[attr_count].value.u32 = SAI_VLAN_TAGGING_MODE_TAGGED;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->create_vlan_member(&vlan_mem_obj_id2,
                                                    0,
                                                    attr_count,
                                                    attr));
    attr_count = 0;
    memset(&attr, 0, sizeof(attr));
    memset(&port_obj_list, 0, sizeof(port_obj_list));

    attr[attr_count].id = SAI_VLAN_ATTR_MEMBER_LIST;
    attr[attr_count].value.objlist.count = SAI_GTEST_VLAN_MEMBER_PORT_COUNT;
    attr[attr_count].value.objlist.list = port_obj_list;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->get_vlan_attribute(vlan_obj_id, 1, attr));

    EXPECT_EQ(attr[0].value.objlist.count, SAI_GTEST_VLAN_MEMBER_PORT_COUNT);
    EXPECT_EQ(vlan_mem_obj_id1, port_obj_list[0]);
    EXPECT_EQ(vlan_mem_obj_id2, port_obj_list[1]);

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_vlan_api_table->get_vlan_member_attribute(
                vlan_mem_obj_id1,
                attr_count,
                attr));
    EXPECT_EQ(attr[0].value.oid, vlan_obj_id);
    EXPECT_EQ(attr[1].value.oid, bridge_port_id_1);
    EXPECT_EQ(attr[2].value.u32, SAI_VLAN_TAGGING_MODE_UNTAGGED);

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_vlan_api_table->get_vlan_member_attribute(
                vlan_mem_obj_id2,
                attr_count,
                attr));
    EXPECT_EQ(attr[0].value.oid, vlan_obj_id);
    EXPECT_EQ(attr[1].value.oid, bridge_port_id_2);
    EXPECT_EQ(attr[2].value.u32, SAI_VLAN_TAGGING_MODE_TAGGED);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->remove_vlan_member(vlan_mem_obj_id1));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->remove_vlan_member(vlan_mem_obj_id2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->remove_vlan(vlan_obj_id));
}
/*
 * VLAN port priority tag testing
 */
TEST_F(vlanTest, vlan_port_priority_tag_test)
{
    sai_attribute_t attr[SAI_GTEST_VLAN_MEMBER_ATTR_COUNT];
    sai_object_id_t vlan_obj_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t vlan_mem_obj_id = SAI_NULL_OBJECT_ID;
    uint32_t attr_count = 0;

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_ATTR_VLAN_ID;
    attr[attr_count].value.u16 = SAI_GTEST_VLAN;
    attr_count++;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->create_vlan(&vlan_obj_id,0,attr_count,attr));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr[attr_count].value.oid = vlan_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr[attr_count].value.oid = bridge_port_id_1;
    attr_count++;
    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr[attr_count].value.u32 = SAI_VLAN_TAGGING_MODE_PRIORITY_TAGGED;
    attr_count++;

    EXPECT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->create_vlan_member(&vlan_mem_obj_id,
                                                    0,
                                                    attr_count,
                                                    attr));

    EXPECT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->remove_vlan_member(vlan_mem_obj_id));

    EXPECT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_vlan_api_table->remove_vlan_member(vlan_mem_obj_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->remove_vlan(vlan_obj_id));
}
/*
 * VLAN test MAX learnt MAC address
 */
TEST_F(vlanTest, test_max_learnt_mac_address)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr;
    sai_attribute_t attr;
    sai_object_id_t vlan_obj_id = SAI_NULL_OBJECT_ID;

    memset(&set_attr, 0, sizeof(sai_attribute_t));
    memset(&get_attr, 0, sizeof(sai_attribute_t));

    attr.id = SAI_VLAN_ATTR_VLAN_ID;
    attr.value.u16 = SAI_GTEST_VLAN;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->create_vlan(&vlan_obj_id,0,1,&attr));

    set_attr.id = SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES;
    set_attr.value.u32 = 100;

    EXPECT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->set_vlan_attribute(vlan_obj_id,
                                                   (const sai_attribute_t*)&set_attr));
    get_attr.id = SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES;
    EXPECT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->get_vlan_attribute(vlan_obj_id,1,
                                                           &get_attr));
    EXPECT_EQ(set_attr.value.u32,get_attr.value.u32);
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->remove_vlan(vlan_obj_id));
}
/*
 * VLAN test learn disable
 */
TEST_F(vlanTest, test_vlan_learn_disable)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr;
    sai_attribute_t attr;
    sai_object_id_t vlan_obj_id = SAI_NULL_OBJECT_ID;

    memset(&set_attr, 0, sizeof(sai_attribute_t));
    memset(&get_attr, 0, sizeof(sai_attribute_t));

    attr.id = SAI_VLAN_ATTR_VLAN_ID;
    attr.value.u16 = SAI_GTEST_VLAN;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->create_vlan(&vlan_obj_id,0,1,&attr));

    set_attr.id = SAI_VLAN_ATTR_LEARN_DISABLE;
    set_attr.value.booldata = true;

    EXPECT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->set_vlan_attribute(vlan_obj_id,
                                                   (const sai_attribute_t*)&set_attr));
    get_attr.id = SAI_VLAN_ATTR_LEARN_DISABLE;
    EXPECT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->get_vlan_attribute(vlan_obj_id,1,
                                                           &get_attr));
    EXPECT_EQ(set_attr.value.booldata,get_attr.value.booldata);
    set_attr.value.booldata = false;

    EXPECT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->set_vlan_attribute(vlan_obj_id,
                                                   (const sai_attribute_t*)&set_attr));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->remove_vlan(vlan_obj_id));
}

/*
 * VLAN test statistics
 */
TEST_F(vlanTest, vlan_stats)
{
    sai_vlan_stat_t counter_ids[] =
    {SAI_VLAN_STAT_IN_OCTETS,
    SAI_VLAN_STAT_IN_PACKETS,
    SAI_VLAN_STAT_IN_UCAST_PKTS,
    SAI_VLAN_STAT_IN_NON_UCAST_PKTS,
    SAI_VLAN_STAT_IN_DISCARDS,
    SAI_VLAN_STAT_IN_ERRORS,
    SAI_VLAN_STAT_IN_UNKNOWN_PROTOS,
    SAI_VLAN_STAT_OUT_OCTETS,
    SAI_VLAN_STAT_OUT_PACKETS,
    SAI_VLAN_STAT_OUT_UCAST_PKTS,
    SAI_VLAN_STAT_OUT_NON_UCAST_PKTS,
    SAI_VLAN_STAT_OUT_DISCARDS,
    SAI_VLAN_STAT_OUT_ERRORS,
    SAI_VLAN_STAT_OUT_QLEN
    };

    unsigned int num_counter = sizeof(counter_ids)/sizeof(sai_vlan_stat_t);
    uint64_t counter_val = 0;
    unsigned int idx = 0;
    sai_status_t ret_val = 0;
    sai_attribute_t attr;
    sai_object_id_t vlan_obj_id = SAI_NULL_OBJECT_ID;

    attr.id = SAI_VLAN_ATTR_VLAN_ID;
    attr.value.u16 = SAI_GTEST_VLAN;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->create_vlan(&vlan_obj_id,0,1,&attr));

    for(idx = 0; idx < num_counter; idx++) {
        ret_val = sai_vlan_api_table->get_vlan_stats(vlan_obj_id,1,
                                                    (const sai_vlan_stat_t*)
                                                     &counter_ids[idx], &counter_val);
        if(ret_val == SAI_STATUS_SUCCESS) {
            printf("Counter %d val is 0x%" PRIx64 "\r\n",counter_ids[idx], counter_val);
        } else if(ret_val == SAI_STATUS_NOT_SUPPORTED) {
            printf("Counter %d is not supported\r\n",counter_ids[idx]);
        } else {
            printf("Counter %d return value is %d\r\n",counter_ids[idx],ret_val);
        }
    }

    for(idx = 0; idx < num_counter; idx++) {
        ret_val = sai_vlan_api_table->clear_vlan_stats(vlan_obj_id, 1,
                                                      (const sai_vlan_stat_t*)&counter_ids[idx]);
        if(ret_val == SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, sai_vlan_api_table->get_vlan_stats(vlan_obj_id, 1,
                                                                             (const sai_vlan_stat_t*)
                                                                              &counter_ids[idx], &counter_val));
            if(counter_val == 0) {
                printf("Counter %d is cleared\r\n",counter_ids[idx]);
            } else {
                printf("Counter %d has value 0x%" PRIx64 " after clear\r\n",
                       counter_ids[idx], counter_val);
            }
        } else if(ret_val == SAI_STATUS_NOT_SUPPORTED) {
            printf("Counter %d is not supported\r\n",counter_ids[idx]);
        } else {
            printf("Counter %d return value is %d\r\n",counter_ids[idx],ret_val);
        }
    }

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->remove_vlan(vlan_obj_id));

}
