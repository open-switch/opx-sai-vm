/************************************************************************
* LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file  sai_qos_maps_unit_test.cpp
*
* @brief This file contains tests for qos maps APIs
*
*************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"

extern "C" {
#include "sai_qos_unit_test_utils.h"
#include "sai.h"
#include "saiqosmaps.h"
#include <inttypes.h>
}

#define UC_Q_INDEX    3
#define MC_Q_INDEX    13
#define DEFAULT_TC    2
#define DEFAULT_PG    7
#define NUM_PRIO      8
#define DFLT_Q_INDEX  0

static sai_object_id_t switch_id =0;

#define LOG_PRINT(msg, ...) \
    printf(msg, ##__VA_ARGS__)

/*
 * API query is done while running the first test case and
 * the method table is stored in sai_switch_api_table so
 * that its available for the rest of the test cases which
 * use the method table
 */
class qosMap : public ::testing::Test
{
    protected:
        static void SetUpTestCase()
        {
            ASSERT_EQ(SAI_STATUS_SUCCESS,sai_api_query(SAI_API_SWITCH,
                        (static_cast<void**>(static_cast<void*>(&sai_switch_api_table)))));

            ASSERT_TRUE(sai_switch_api_table != NULL);

            EXPECT_TRUE(sai_switch_api_table->remove_switch != NULL);
            EXPECT_TRUE(sai_switch_api_table->set_switch_attribute != NULL);
            EXPECT_TRUE(sai_switch_api_table->get_switch_attribute != NULL);


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

            ASSERT_TRUE(sai_switch_api_table->create_switch != NULL);

            EXPECT_EQ (SAI_STATUS_SUCCESS,
                    (sai_switch_api_table->create_switch (&switch_id , attr_count,
                                                          sai_attr_set)));


            ASSERT_EQ(SAI_STATUS_SUCCESS,sai_api_query(SAI_API_QOS_MAPS,
                        (static_cast<void**>(static_cast<void*>(&sai_qos_map_api_table)))));

            ASSERT_TRUE(sai_qos_map_api_table != NULL);

            EXPECT_TRUE(sai_qos_map_api_table->create_qos_map != NULL);
            EXPECT_TRUE(sai_qos_map_api_table->remove_qos_map != NULL);
            EXPECT_TRUE(sai_qos_map_api_table->set_qos_map_attribute != NULL);
            EXPECT_TRUE(sai_qos_map_api_table->get_qos_map_attribute != NULL);

            ASSERT_EQ(NULL,sai_api_query(SAI_API_PORT,
                        (static_cast<void**>(static_cast<void*>(&sai_port_api_table)))));

            ASSERT_TRUE(sai_port_api_table != NULL);

            EXPECT_TRUE(sai_port_api_table->set_port_attribute != NULL);
            EXPECT_TRUE(sai_port_api_table->get_port_attribute != NULL);
            EXPECT_TRUE(sai_port_api_table->get_port_stats != NULL);
            sai_attribute_t sai_port_attr;
            uint32_t * port_count = sai_qos_update_port_count();
            sai_status_t ret = SAI_STATUS_SUCCESS;

            memset (&sai_port_attr, 0, sizeof (sai_port_attr));

            sai_port_attr.id = SAI_SWITCH_ATTR_PORT_LIST;
            sai_port_attr.value.objlist.count = 256;
            sai_port_attr.value.objlist.list  = sai_qos_update_port_list();

            ret = sai_switch_api_table->get_switch_attribute(0,1,&sai_port_attr);
            *port_count = sai_port_attr.value.objlist.count;

            ASSERT_EQ(SAI_STATUS_SUCCESS,ret);
        }

        static sai_switch_api_t* sai_switch_api_table;
        static sai_qos_map_api_t* sai_qos_map_api_table;
        static sai_port_api_t* sai_port_api_table;
        static const unsigned int test_port_id = 30;
        static const unsigned int test_port_id_1 = 31;
};

sai_switch_api_t* qosMap ::sai_switch_api_table = NULL;
sai_qos_map_api_t* qosMap ::sai_qos_map_api_table = NULL;
sai_port_api_t* qosMap ::sai_port_api_table = NULL;
/*
 * Apply the same map more than once on the same port.
 * Expect SAI_STATUS_ITEM_ALREADY_EXISTS error for the second call.
 */
TEST_F(qosMap, duplicate_map)
{
    sai_attribute_t attr;
    sai_attribute_t set_attr;
    sai_object_id_t map_id = 0;
    unsigned int attr_count = 1;

    attr.id = SAI_QOS_MAP_ATTR_TYPE;
    attr.value.s32 = SAI_QOS_MAP_TYPE_DOT1P_TO_TC_AND_COLOR;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)&attr));


    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP;
    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP;
    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

}

/*
 * Replace map of the same type with a different map-id.
 * Both should return SAI_STATUS_SUCCESS status.
 */
TEST_F(qosMap, map_replace_test)
{
    sai_attribute_t attr;
    sai_attribute_t set_attr;
    sai_object_id_t map_id = 0;
    sai_object_id_t map_id1 = 0;
    unsigned int attr_count = 1;

    attr.id = SAI_QOS_MAP_ATTR_TYPE;
    attr.value.s32 = SAI_QOS_MAP_TYPE_DOT1P_TO_TC_AND_COLOR;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)&attr));


    attr.id = SAI_QOS_MAP_ATTR_TYPE;
    attr.value.s32 = SAI_QOS_MAP_TYPE_DOT1P_TO_TC_AND_COLOR;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id1, switch_id, attr_count, (const sai_attribute_t *)&attr));
    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP;

    set_attr.value.oid = map_id1;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

}

/*
 * Create, Set, Get and apply on the port.
 * Apply on two ports.
 * Remove from one of the ports and try to remove the map.
 * Expect - SAI_STATUS_OBJECT_IN_USE.
 * Remove from the other port and then remove the map.
 * Expect - SAI_STATUS_SUCCESS
 */
TEST_F(qosMap, dot1p_to_tc_and_color)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_qos_map_list_t map_list;
    sai_object_id_t map_id = 0;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;
    int dot1p_to_check = 6;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DOT1P_TO_TC_AND_COLOR;

    attr_count ++;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dot1p = 0;
    map_list.list[loop_idx].value.tc = 1;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 1;
    map_list.list[loop_idx].value.tc  = 2;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_RED;

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);
    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 16;
    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dot1p %d tc %d %d color\r\n",get_attr.value.qosmap.list[loop_idx].key.dot1p,
               get_attr.value.qosmap.list[loop_idx].value.tc,
               get_attr.value.qosmap.list[loop_idx].value.color);
    }
    free(map_list.list);
    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));
    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));
    attr_count = 0;
    set_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 4;
    map_list.list = (sai_qos_map_t *)calloc(4, sizeof(sai_qos_map_t));

    loop_idx = 0;
    map_list.list[loop_idx].key.dot1p = 4;
    map_list.list[loop_idx].value.tc = 2;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 5;
    map_list.list[loop_idx].value.tc  = 6;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_RED;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 7;
    map_list.list[loop_idx].value.tc  = 6;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_YELLOW;
    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 6;
    map_list.list[loop_idx].value.tc  = DEFAULT_TC;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    set_attr.value.qosmap.count = map_list.count;
    set_attr.value.qosmap.list = map_list.list;
    attr_count ++;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (map_id,&set_attr));
    free(map_list.list);

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 16;
    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dot1p %d tc %d %d color\r\n",get_attr.value.qosmap.list[loop_idx].key.dot1p,
               get_attr.value.qosmap.list[loop_idx].value.tc,
               get_attr.value.qosmap.list[loop_idx].value.color);
    }

    ASSERT_EQ(get_attr.value.qosmap.list[dot1p_to_check].value.tc, DEFAULT_TC);
    ASSERT_EQ(get_attr.value.qosmap.list[dot1p_to_check].value.color, SAI_PACKET_COLOR_GREEN);
    free(map_list.list);
    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP;

    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE, sai_qos_map_api_table->remove_qos_map(map_id));

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id));
}

/*
 * Create, Set, Get and apply on the port.
 * Apply on two ports.
 * Remove from one of the ports and try to remove the map.
 * Expect - SAI_STATUS_OBJECT_IN_USE.
 * Remove from the other port and then remove the map.
 * Expect - SAI_STATUS_SUCCESS
 */
TEST_F(qosMap, dot1p_to_tc)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_qos_map_list_t map_list;
    sai_object_id_t map_id = 0;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;
    int dot1p_to_check = 6;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DOT1P_TO_TC;

    attr_count ++;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dot1p = 0;
    map_list.list[loop_idx].value.tc = 1;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 1;
    map_list.list[loop_idx].value.tc  = 2;

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);
    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 16;
    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dot1p %d tc %d \r\n",get_attr.value.qosmap.list[loop_idx].key.dot1p,
               get_attr.value.qosmap.list[loop_idx].value.tc);
    }
    free(map_list.list);
    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));
    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));
    attr_count = 0;
    set_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 4;
    map_list.list = (sai_qos_map_t *)calloc(4, sizeof(sai_qos_map_t));

    loop_idx = 0;
    map_list.list[loop_idx].key.dot1p = 4;
    map_list.list[loop_idx].value.tc = 2;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 5;
    map_list.list[loop_idx].value.tc  = 6;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 7;
    map_list.list[loop_idx].value.tc  = 6;
    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 6;
    map_list.list[loop_idx].value.tc  = DEFAULT_TC;

    set_attr.value.qosmap.count = map_list.count;
    set_attr.value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (map_id,&set_attr));
    free(map_list.list);

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 16;
    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dot1p %d tc %d \r\n",get_attr.value.qosmap.list[loop_idx].key.dot1p,
               get_attr.value.qosmap.list[loop_idx].value.tc);
    }

    ASSERT_EQ(get_attr.value.qosmap.list[dot1p_to_check].value.tc, DEFAULT_TC);
    free(map_list.list);
    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP;

    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE, sai_qos_map_api_table->remove_qos_map(map_id));

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id));
}
/*
 * Create, Set, Get and apply on the port.
 * Apply on two ports.
 * Remove from one of the ports and try to remove the map.
 * Expect - SAI_STATUS_OBJECT_IN_USE.
 * Remove from the other port and then remove the map.
 * Expect - SAI_STATUS_SUCCESS
 */
TEST_F(qosMap, dot1p_to_color)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_qos_map_list_t map_list;
    sai_object_id_t map_id = 0;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;
    int dot1p_to_check = 6;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR;

    attr_count ++;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dot1p = 0;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 1;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_RED;

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);
    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 16;
    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dot1p %d color %d\r\n",get_attr.value.qosmap.list[loop_idx].key.dot1p,
               get_attr.value.qosmap.list[loop_idx].value.color);
    }
    free(map_list.list);
    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP;
    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));
    attr_count = 0;
    set_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 4;
    map_list.list = (sai_qos_map_t *)calloc(4, sizeof(sai_qos_map_t));

    loop_idx = 0;
    map_list.list[loop_idx].key.dot1p = 4;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 5;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_RED;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 7;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_YELLOW;
    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 6;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    set_attr.value.qosmap.count = map_list.count;
    set_attr.value.qosmap.list = map_list.list;
    attr_count ++;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (map_id,&set_attr));
    free(map_list.list);

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 16;
    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dot1p %d color %d\r\n",get_attr.value.qosmap.list[loop_idx].key.dot1p,
               get_attr.value.qosmap.list[loop_idx].value.color);
    }

    ASSERT_EQ(get_attr.value.qosmap.list[dot1p_to_check].value.color, SAI_PACKET_COLOR_GREEN);
    free(map_list.list);
    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP;

    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE, sai_qos_map_api_table->remove_qos_map(map_id));

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id));
}

/*
 * Create, Set, Get and apply on the port.
 * Apply on two ports.
 * Remove from one of the ports and try to remove the map.
 * Expect - SAI_STATUS_OBJECT_IN_USE.
 * Remove from the other port and then remove the map.
 * Expect - SAI_STATUS_SUCCESS
 */
TEST_F(qosMap, dot1p_to_tc_map_or_color_map)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_qos_map_list_t map_list;
    sai_object_id_t tc_map_id = 0;
    sai_object_id_t color_map_id = 0;
    sai_object_id_t tc_map_id_1 = 0;
    sai_object_id_t color_map_id_1 = 0;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;
    int dot1p_to_check = 6;

    /* Dot1p to tc */
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DOT1P_TO_TC;

    attr_count ++;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dot1p = 0;
    map_list.list[loop_idx].value.tc = 1;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 1;
    map_list.list[loop_idx].value.tc  = 2;

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&tc_map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);

    /* Dot1p to color */
    attr_count = loop_idx = 0;
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR;

    attr_count ++;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dot1p = 0;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 1;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&color_map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);

    /* Dot1p to tc */
    attr_count = loop_idx = 0;
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DOT1P_TO_TC;

    attr_count ++;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dot1p = 0;
    map_list.list[loop_idx].value.tc = 5;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 1;
    map_list.list[loop_idx].value.tc  = 6;

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&tc_map_id_1, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);

    /* Dot1p to color */
    attr_count = loop_idx = 0;
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR;

    attr_count ++;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dot1p = 0;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_YELLOW;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 1;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_RED;

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&color_map_id_1, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);

    printf("Appply maps on ports \r\n");
    /* Apply it on ports */
    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP;
    set_attr.value.oid = tc_map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP;
    set_attr.value.oid = color_map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    /* Modify the existing */
    attr_count = 0;
    set_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 4;
    map_list.list = (sai_qos_map_t *)calloc(4, sizeof(sai_qos_map_t));

    loop_idx = 0;
    map_list.list[loop_idx].key.dot1p = 4;
    map_list.list[loop_idx].value.tc = 2;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 5;
    map_list.list[loop_idx].value.tc  = 6;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_RED;

    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 7;
    map_list.list[loop_idx].value.tc  = 6;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_YELLOW;
    loop_idx ++;

    map_list.list[loop_idx].key.dot1p = 6;
    map_list.list[loop_idx].value.tc  = DEFAULT_TC;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    set_attr.value.qosmap.count = map_list.count;
    set_attr.value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (tc_map_id,&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (color_map_id,&set_attr));

    free(map_list.list);

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 16;
    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (tc_map_id, 1, &get_attr));

    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dot1p %d tc %d\r\n",get_attr.value.qosmap.list[loop_idx].key.dot1p,
               get_attr.value.qosmap.list[loop_idx].value.tc);
    }

    ASSERT_EQ(get_attr.value.qosmap.list[dot1p_to_check].value.tc, DEFAULT_TC);
    free(map_list.list);

    map_list.count = 16;
    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (color_map_id, 1, &get_attr));

    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dot1p %d color %d\r\n",get_attr.value.qosmap.list[loop_idx].key.dot1p,
               get_attr.value.qosmap.list[loop_idx].value.color);
    }

    ASSERT_EQ(get_attr.value.qosmap.list[dot1p_to_check].value.color, SAI_PACKET_COLOR_GREEN);
    free(map_list.list);

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    /* Modify the maps on port */

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP;
    set_attr.value.oid = tc_map_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP;
    set_attr.value.oid = color_map_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP;
    set_attr.value.oid = tc_map_id_1;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP;
    set_attr.value.oid = color_map_id_1;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(tc_map_id));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(tc_map_id_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(color_map_id));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(color_map_id_1));
}
/*
 * Create, Set, Get and apply on the port.
 * Apply on two ports.
 * Remove from one of the ports and try to remove the map.
 * Expect - SAI_STATUS_OBJECT_IN_USE.
 * Remove from the other port and then remove the map.
 * Expect - SAI_STATUS_SUCCESS
 */
TEST_F(qosMap, dscp_to_tc)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_qos_map_list_t map_list;
    sai_object_id_t map_id = 0;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;
    int dscp_to_check = 6;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DSCP_TO_TC;

    attr_count ++;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dscp = 0;
    map_list.list[loop_idx].value.tc = 1;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 1;
    map_list.list[loop_idx].value.tc  = 2;

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);
    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 64;
    map_list.list = (sai_qos_map_t *)calloc(64, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dscp %d tc %d \r\n",get_attr.value.qosmap.list[loop_idx].key.dscp,
               get_attr.value.qosmap.list[loop_idx].value.tc);
    }
    free(map_list.list);
    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));
    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));
    attr_count = 0;
    set_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 4;
    map_list.list = (sai_qos_map_t *)calloc(4, sizeof(sai_qos_map_t));

    loop_idx = 0;
    map_list.list[loop_idx].key.dscp = 4;
    map_list.list[loop_idx].value.tc = 2;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 5;
    map_list.list[loop_idx].value.tc  = 6;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 7;
    map_list.list[loop_idx].value.tc  = 6;
    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 6;
    map_list.list[loop_idx].value.tc  = DEFAULT_TC;

    set_attr.value.qosmap.count = map_list.count;
    set_attr.value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (map_id,&set_attr));
    free(map_list.list);

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 64;
    map_list.list = (sai_qos_map_t *)calloc(64, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dscp %d tc %d \r\n",get_attr.value.qosmap.list[loop_idx].key.dscp,
               get_attr.value.qosmap.list[loop_idx].value.tc);
    }

    ASSERT_EQ(get_attr.value.qosmap.list[dscp_to_check].value.tc, DEFAULT_TC);
    free(map_list.list);
    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP;

    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE, sai_qos_map_api_table->remove_qos_map(map_id));

    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id));
}
/*
 * Create, Set, Get and apply on the port.
 * Apply on two ports.
 * Remove from one of the ports and try to remove the map.
 * Expect - SAI_STATUS_OBJECT_IN_USE.
 * Remove from the other port and then remove the map.
 * Expect - SAI_STATUS_SUCCESS
 */
TEST_F(qosMap, dscp_to_color)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_qos_map_list_t map_list;
    sai_object_id_t map_id = 0;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;
    int dscp_to_check = 6;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DSCP_TO_COLOR;

    attr_count ++;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dscp = 0;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 1;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_RED;

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);
    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 64;
    map_list.list = (sai_qos_map_t *)calloc(64, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dscp %d color %d\r\n",get_attr.value.qosmap.list[loop_idx].key.dscp,
               get_attr.value.qosmap.list[loop_idx].value.color);
    }
    free(map_list.list);
    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP;
    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));
    attr_count = 0;
    set_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 4;
    map_list.list = (sai_qos_map_t *)calloc(4, sizeof(sai_qos_map_t));

    loop_idx = 0;
    map_list.list[loop_idx].key.dscp = 4;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 5;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_RED;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 7;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_YELLOW;
    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 6;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    set_attr.value.qosmap.count = map_list.count;
    set_attr.value.qosmap.list = map_list.list;
    attr_count ++;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (map_id,&set_attr));
    free(map_list.list);

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 64;
    map_list.list = (sai_qos_map_t *)calloc(64, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dscp %d color %d\r\n",get_attr.value.qosmap.list[loop_idx].key.dscp,
               get_attr.value.qosmap.list[loop_idx].value.color);
    }

    ASSERT_EQ(get_attr.value.qosmap.list[dscp_to_check].value.color, SAI_PACKET_COLOR_GREEN);
    free(map_list.list);
    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP;

    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE, sai_qos_map_api_table->remove_qos_map(map_id));

    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id));
}

/*
 * Create, Set, Get and apply on the port.
 * Apply on two ports.
 * Remove from one of the ports and try to remove the map.
 * Expect - SAI_STATUS_OBJECT_IN_USE.
 * Remove from the other port and then remove the map.
 * Expect - SAI_STATUS_SUCCESS
 */
TEST_F(qosMap, dscp_to_tc_map_or_color_map)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_qos_map_list_t map_list;
    sai_object_id_t tc_map_id = 0;
    sai_object_id_t color_map_id = 0;
    sai_object_id_t tc_map_id_1 = 0;
    sai_object_id_t color_map_id_1 = 0;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;
    int dscp_to_check = 6;

    /* Dot1p to tc */
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DSCP_TO_TC;

    attr_count ++;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dscp = 0;
    map_list.list[loop_idx].value.tc = 1;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 1;
    map_list.list[loop_idx].value.tc  = 2;

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&tc_map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);

    /* Dot1p to color */
    attr_count = loop_idx = 0;
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DSCP_TO_COLOR;

    attr_count ++;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dscp = 0;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 1;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&color_map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);

    /* Dot1p to tc */
    attr_count = loop_idx = 0;
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DSCP_TO_TC;

    attr_count ++;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dscp = 0;
    map_list.list[loop_idx].value.tc = 5;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 1;
    map_list.list[loop_idx].value.tc  = 6;

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&tc_map_id_1, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);

    /* Dot1p to color */
    attr_count = loop_idx = 0;
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DSCP_TO_COLOR;

    attr_count ++;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dscp = 0;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_YELLOW;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 1;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_RED;

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&color_map_id_1, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);

    printf("Appply maps on ports \r\n");
    /* Apply it on ports */
    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP;
    set_attr.value.oid = tc_map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP;
    set_attr.value.oid = color_map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    /* Modify the existing */
    attr_count = 0;
    set_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 4;
    map_list.list = (sai_qos_map_t *)calloc(4, sizeof(sai_qos_map_t));

    loop_idx = 0;
    map_list.list[loop_idx].key.dscp = 4;
    map_list.list[loop_idx].value.tc = 2;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 5;
    map_list.list[loop_idx].value.tc  = 6;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_RED;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 7;
    map_list.list[loop_idx].value.tc  = 6;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_YELLOW;
    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 6;
    map_list.list[loop_idx].value.tc  = DEFAULT_TC;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    set_attr.value.qosmap.count = map_list.count;
    set_attr.value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (tc_map_id,&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (color_map_id,&set_attr));

    free(map_list.list);

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 64;
    map_list.list = (sai_qos_map_t *)calloc(64, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (tc_map_id, 1, &get_attr));

    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dscp %d tc %d\r\n",get_attr.value.qosmap.list[loop_idx].key.dscp,
               get_attr.value.qosmap.list[loop_idx].value.tc);
    }

    ASSERT_EQ(get_attr.value.qosmap.list[dscp_to_check].value.tc, DEFAULT_TC);
    free(map_list.list);

    map_list.count = 64;
    map_list.list = (sai_qos_map_t *)calloc(64, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (color_map_id, 1, &get_attr));

    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dscp %d color %d\r\n",get_attr.value.qosmap.list[loop_idx].key.dscp,
               get_attr.value.qosmap.list[loop_idx].value.color);
    }

    ASSERT_EQ(get_attr.value.qosmap.list[dscp_to_check].value.color, SAI_PACKET_COLOR_GREEN);
    free(map_list.list);

    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    /* Modify the maps on port */

    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP;
    set_attr.value.oid = tc_map_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP;
    set_attr.value.oid = color_map_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP;
    set_attr.value.oid = tc_map_id_1;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP;
    set_attr.value.oid = color_map_id_1;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(tc_map_id));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(tc_map_id_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(color_map_id));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(color_map_id_1));
}


/*
 * Create, Set, Get and apply on the port.
 * Apply on two ports.
 * Remove from one of the ports and try to remove the map.
 * Expect - SAI_STATUS_OBJECT_IN_USE.
 * Remove from the other port and then remove the map.
 * Expect - SAI_STATUS_SUCCESS
 */
TEST_F(qosMap, tc_and_color_to_dot1p)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_qos_map_list_t map_list;
    sai_object_id_t map_id = 0;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;
    unsigned int tc_to_check = DEFAULT_TC;
    sai_packet_color_t color_to_check = SAI_PACKET_COLOR_GREEN;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P;

    attr_count ++;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.tc = 1;
    map_list.list[loop_idx].key.color = SAI_PACKET_COLOR_GREEN;
    map_list.list[loop_idx].value.dot1p = 0;

    loop_idx ++;

    map_list.list[loop_idx].value.dot1p = 1;
    map_list.list[loop_idx].key.tc  = 2;
    map_list.list[loop_idx].key.color = SAI_PACKET_COLOR_RED;

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);
    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 48;
    map_list.list = (sai_qos_map_t *)calloc(48, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("tc %d color %d dot1p:%d\r\n",get_attr.value.qosmap.list[loop_idx].key.tc,
               get_attr.value.qosmap.list[loop_idx].key.color,
               get_attr.value.qosmap.list[loop_idx].value.dot1p);
    }
    free(map_list.list);
    set_attr.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));
    set_attr.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));
    attr_count = 0;
    set_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 4;
    map_list.list = (sai_qos_map_t *)calloc(4, sizeof(sai_qos_map_t));

    loop_idx = 0;
    map_list.list[loop_idx].key.tc = 4;
    map_list.list[loop_idx].key.color = SAI_PACKET_COLOR_GREEN;
    map_list.list[loop_idx].value.dot1p = 7;

    loop_idx ++;

    map_list.list[loop_idx].key.tc = 5;
    map_list.list[loop_idx].key.color  = SAI_PACKET_COLOR_RED;
    map_list.list[loop_idx].value.dot1p = 6;

    loop_idx ++;

    map_list.list[loop_idx].key.tc = 7;
    map_list.list[loop_idx].key.color  = SAI_PACKET_COLOR_YELLOW;
    map_list.list[loop_idx].value.dot1p = 3;
    loop_idx ++;

    map_list.list[loop_idx].key.tc = DEFAULT_TC;
    map_list.list[loop_idx].key.color  = SAI_PACKET_COLOR_GREEN;
    map_list.list[loop_idx].value.dot1p = 1;

    set_attr.value.qosmap.count = map_list.count;
    set_attr.value.qosmap.list = map_list.list;
    attr_count ++;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (map_id,&set_attr));
    free(map_list.list);

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 48;
    map_list.list = (sai_qos_map_t *)calloc(48, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("tc %d color %d dot1p :%d\r\n",get_attr.value.qosmap.list[loop_idx].key.tc,
               get_attr.value.qosmap.list[loop_idx].key.color,
               get_attr.value.qosmap.list[loop_idx].value.dot1p);
    }

    for (loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++) {
        if ((get_attr.value.qosmap.list[loop_idx].key.tc == tc_to_check) &&
            (get_attr.value.qosmap.list[loop_idx].key.color == color_to_check)) {
            ASSERT_EQ(get_attr.value.qosmap.list[loop_idx].value.dot1p, 1);
            break;
        }
    }
    free(map_list.list);
    set_attr.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP;

    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE, sai_qos_map_api_table->remove_qos_map(map_id));

    set_attr.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id));
}
/*
 * Sets the map type attribute.
 * Sets invalid and out of range value.
 * Expect - SAI_STATUS_INVALID_ATTRIBUTE_0.
 */

TEST_F(qosMap, map_invalid_attribute_test)
{
    sai_attribute_t attr;
    sai_attribute_t set_attr;
    sai_object_id_t map_id = 0;
    unsigned int attr_count = 1;
    unsigned int loop_idx = 0;
    sai_qos_map_list_t map_list;

    attr.id = SAI_QOS_MAP_ATTR_TYPE;
    attr.value.s32 = SAI_QOS_MAP_TYPE_DOT1P_TO_TC_AND_COLOR;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)&attr));

    set_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;
    map_list.count = 1;
    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dot1p = 78;
    map_list.list[loop_idx].value.tc = 1;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_YELLOW;

    set_attr.value.qosmap.count = map_list.count;
    set_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTR_VALUE_0,
              sai_qos_map_api_table->set_qos_map_attribute(map_id, &set_attr));

    set_attr.id = SAI_QOS_MAP_ATTR_TYPE;
    set_attr.value.s32 = SAI_QOS_MAP_TYPE_DSCP_TO_TC_AND_COLOR;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              sai_qos_map_api_table->set_qos_map_attribute(map_id, &set_attr));
    free(map_list.list);
}

/*
 * Create, set, get dscp map.
 * apply on port and remove
 */
TEST_F(qosMap, dscp_to_tc_and_color_map_get)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t attr;
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_object_id_t map_id = 0;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;
    int dscp_to_check = 12;
    sai_qos_map_list_t map_list;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DSCP_TO_TC_AND_COLOR;

    attr_count ++;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    /*
     * Set the values for the created map.
     */
    attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 5;
    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.dscp = 0;
    map_list.list[loop_idx].value.tc = 1;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 1;
    map_list.list[loop_idx].value.tc  = 2;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_RED;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 5;
    map_list.list[loop_idx].value.tc  = 3;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_RED;


    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 10;
    map_list.list[loop_idx].value.tc  = 7;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_YELLOW;

    loop_idx ++;

    map_list.list[loop_idx].key.dscp = 12;
    map_list.list[loop_idx].value.tc  = DEFAULT_TC;
    map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_YELLOW;

    attr.value.qosmap.count = map_list.count;
    attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (map_id, &attr));

    free(map_list.list);

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 16;
    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_BUFFER_OVERFLOW,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));

    map_list.count = get_attr.value.qosmap.count;
    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
    for(loop_idx = 0; loop_idx < map_list.count; loop_idx ++)
    {
        printf("dscp %d tc %d %d color\r\n",get_attr.value.qosmap.list[loop_idx].key.dscp,
               get_attr.value.qosmap.list[loop_idx].value.tc,
               get_attr.value.qosmap.list[loop_idx].value.color);
    }

    ASSERT_EQ(get_attr.value.qosmap.list[dscp_to_check].value.tc, DEFAULT_TC);
    ASSERT_EQ(get_attr.value.qosmap.list[dscp_to_check].value.color, SAI_PACKET_COLOR_YELLOW);
    free(map_list.list);

    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id));
}

/*
 * Create, set, get tc to dscp map.
 * apply on port and remove
 */
TEST_F(qosMap, tc_and_color_to_dscp_map)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t attr;
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_object_id_t map_id = 0;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;
    unsigned int tc_to_check = DEFAULT_TC;
    sai_packet_color_t color_to_check = SAI_PACKET_COLOR_YELLOW;
    sai_qos_map_list_t map_list;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP;

    attr_count ++;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    /*
     * Set the values for the created map.
     */
    attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 5;
    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.tc = 1;
    map_list.list[loop_idx].key.color = SAI_PACKET_COLOR_GREEN;
    map_list.list[loop_idx].value.dscp = 0;

    loop_idx ++;

    map_list.list[loop_idx].key.tc  = 2;
    map_list.list[loop_idx].key.color = SAI_PACKET_COLOR_RED;
    map_list.list[loop_idx].value.dscp = 1;

    loop_idx ++;

    map_list.list[loop_idx].key.tc  = 3;
    map_list.list[loop_idx].key.color = SAI_PACKET_COLOR_RED;
    map_list.list[loop_idx].value.dscp = 5;


    loop_idx ++;

    map_list.list[loop_idx].key.tc  = 7;
    map_list.list[loop_idx].key.color = SAI_PACKET_COLOR_YELLOW;
    map_list.list[loop_idx].value.dscp = 10;

    loop_idx ++;

    map_list.list[loop_idx].key.tc  = DEFAULT_TC;
    map_list.list[loop_idx].key.color = SAI_PACKET_COLOR_YELLOW;
    map_list.list[loop_idx].value.dscp = 12;

    attr.value.qosmap.count = map_list.count;
    attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (map_id, &attr));

    free(map_list.list);

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 1;
    map_list.list = (sai_qos_map_t *)calloc(1, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_BUFFER_OVERFLOW,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));

    free(map_list.list);
    map_list.count = get_attr.value.qosmap.count;
    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
    for(loop_idx = 0; loop_idx < map_list.count; loop_idx ++)
    {
        printf("tc %d color %d dscp %d\r\n",get_attr.value.qosmap.list[loop_idx].key.tc,
               get_attr.value.qosmap.list[loop_idx].key.color,
               get_attr.value.qosmap.list[loop_idx].value.dscp);
    }

    for (loop_idx = 0; loop_idx < map_list.count; loop_idx ++) {
        if ((get_attr.value.qosmap.list[loop_idx].key.tc == tc_to_check) &&
                (get_attr.value.qosmap.list[loop_idx].key.color == color_to_check)) {
            ASSERT_EQ(get_attr.value.qosmap.list[loop_idx].value.dscp, 12);
            break;
        }
    }
    free(map_list.list);

    set_attr.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id));
}
/*
 * Create, get, set a tc_to_queue map.
 * Apply on port and remove.
 */
TEST_F(qosMap, tc_to_queue_map_set_get)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_object_id_t map_id = 0;
    sai_qos_map_list_t map_list;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_TC_TO_QUEUE;

    attr_count ++;
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 1;
    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.tc = 2;
    map_list.list[loop_idx].value.queue_index = UC_Q_INDEX;

    loop_idx ++;


    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);

    get_attr.id = SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP;

    get_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&get_attr));

    get_attr.id = SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP;

    get_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&get_attr));
    set_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 3;
    map_list.list = (sai_qos_map_t *)calloc(3, sizeof(sai_qos_map_t));

    loop_idx = 0;

    map_list.list[loop_idx].key.tc = DEFAULT_TC;
    map_list.list[loop_idx].value.queue_index  = UC_Q_INDEX;

    loop_idx ++;

    map_list.list[loop_idx].key.tc = DEFAULT_TC;
    map_list.list[loop_idx].value.queue_index  = MC_Q_INDEX;

    set_attr.value.qosmap.count = map_list.count;
    set_attr.value.qosmap.list = map_list.list;
    attr_count ++;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (map_id,&set_attr));

    free(map_list.list);

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 16;
    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;


    ASSERT_EQ(SAI_STATUS_BUFFER_OVERFLOW,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));

    map_list.count = get_attr.value.qosmap.count;

    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));

    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("tc %d qindex %d\r\n",get_attr.value.qosmap.list[loop_idx].key.tc,
               get_attr.value.qosmap.list[loop_idx].value.queue_index);
    }

    ASSERT_EQ(get_attr.value.qosmap.list[DEFAULT_TC].value.queue_index, UC_Q_INDEX);
    free(map_list.list);

    set_attr.id = SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP;

    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));
    set_attr.id = SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP;

    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id));


    ASSERT_EQ(SAI_STATUS_INVALID_OBJECT_ID,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
}

/*
 * Create a empty map and then populate with values.
 */
TEST_F(qosMap, empty_map)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_object_id_t map_id = 0;
    sai_qos_map_list_t map_list;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_TC_TO_QUEUE;

    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    set_attr.id = SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP;

    set_attr.value.oid = map_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));


    set_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    loop_idx = 0;

    map_list.list[loop_idx].key.tc = DEFAULT_TC;
    map_list.list[loop_idx].value.queue_index  = UC_Q_INDEX;

    loop_idx ++;

    map_list.list[loop_idx].key.tc = DEFAULT_TC;
    map_list.list[loop_idx].value.queue_index  = MC_Q_INDEX;

    set_attr.value.qosmap.count = map_list.count;
    set_attr.value.qosmap.list = map_list.list;
    attr_count ++;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (map_id,&set_attr));

    free(map_list.list);

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 16;
    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;


    ASSERT_EQ(SAI_STATUS_BUFFER_OVERFLOW,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));

    free(map_list.list);
    map_list.count = get_attr.value.qosmap.count;

    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));

    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("tc %d qindex %d\r\n",get_attr.value.qosmap.list[loop_idx].key.tc,
               get_attr.value.qosmap.list[loop_idx].value.queue_index);

    }

    ASSERT_EQ(get_attr.value.qosmap.list[DEFAULT_TC].value.queue_index, UC_Q_INDEX);

    free(map_list.list);

    set_attr.id = SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP;

    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id));
}

/*
 * Create, get, set a tc_to_pg map.
 * Apply on port and remove.
 */
TEST_F(qosMap, tc_to_pg_map_test)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_object_id_t map_id = 0;
    sai_qos_map_list_t map_list;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP;

    attr_count ++;
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 1;
    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.tc = 2;
    map_list.list[loop_idx].value.pg = 5;

    loop_idx ++;


    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);

    memset(&set_attr, 0, sizeof(set_attr));
    set_attr.id = SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->get_port_attribute
              (sai_qos_port_id_get(test_port_id_1), 1, &get_attr));

    ASSERT_EQ(get_attr.value.oid, map_id);
    memset(&set_attr, 0, sizeof(set_attr));
    set_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    loop_idx = 0;

    map_list.list[loop_idx].key.tc = 3;
    map_list.list[loop_idx].value.pg  = 4;

    loop_idx ++;

    map_list.list[loop_idx].key.tc = 1;
    map_list.list[loop_idx].value.pg  = 2;

    set_attr.value.qosmap.count = map_list.count;
    set_attr.value.qosmap.list = map_list.list;
    attr_count ++;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (map_id,&set_attr));

    free(map_list.list);

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 8;
    map_list.list = (sai_qos_map_t *)calloc(8, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;


    ASSERT_EQ(SAI_STATUS_BUFFER_OVERFLOW,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));

    free(map_list.list);
    map_list.count = get_attr.value.qosmap.count;

    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));

    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("tc %d pg %d\r\n",get_attr.value.qosmap.list[loop_idx].key.tc,
               get_attr.value.qosmap.list[loop_idx].value.pg);
    }

    ASSERT_EQ(get_attr.value.qosmap.list[2].value.pg, 5);
    ASSERT_EQ(get_attr.value.qosmap.list[3].value.pg, 4);
    ASSERT_EQ(get_attr.value.qosmap.list[1].value.pg, 2);
    ASSERT_EQ(get_attr.value.qosmap.list[0].value.pg, DEFAULT_PG);
    free(map_list.list);

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE, sai_qos_map_api_table->remove_qos_map(map_id));
    set_attr.id = SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP;

    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id));


    ASSERT_EQ(SAI_STATUS_INVALID_OBJECT_ID,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
}

/*
 * Create, get, set a pfc_pri_to_queue map.
 * Apply on port and remove.
 */
TEST_F(qosMap, pfc_pri_to_queue_map_test)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_object_id_t map_id = 0;
    sai_qos_map_list_t map_list;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;
    unsigned int num_uc = 0;


    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_NUMBER_OF_UNICAST_QUEUES;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_switch_api_table->get_switch_attribute(switch_id,1, &get_attr));
    num_uc = get_attr.value.u32;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE;

    attr_count ++;
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 1;
    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    map_list.list[loop_idx].key.prio = 1;
    map_list.list[loop_idx].value.queue_index = UC_Q_INDEX;

    loop_idx ++;


    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->create_qos_map
              (&map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    free(map_list.list);

    memset(&set_attr, 0, sizeof(set_attr));
    set_attr.id = SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP;

    set_attr.value.oid = map_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->get_port_attribute
              (sai_qos_port_id_get(test_port_id_1), 1, &get_attr));

    ASSERT_EQ(get_attr.value.oid, map_id);
    memset(&set_attr, 0, sizeof(set_attr));
    set_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 2;
    map_list.list = (sai_qos_map_t *)calloc(2, sizeof(sai_qos_map_t));

    loop_idx = 0;

    map_list.list[loop_idx].key.prio = 1;
    map_list.list[loop_idx].value.queue_index  = MC_Q_INDEX;

    loop_idx ++;

    map_list.list[loop_idx].key.prio = 2;
    map_list.list[loop_idx].value.queue_index  = MC_Q_INDEX;

    set_attr.value.qosmap.count = map_list.count;
    set_attr.value.qosmap.list = map_list.list;
    attr_count ++;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->set_qos_map_attribute
              (map_id,&set_attr));

    free(map_list.list);

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 8;
    map_list.list = (sai_qos_map_t *)calloc(8, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;


    ASSERT_EQ(SAI_STATUS_BUFFER_OVERFLOW,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));

    free(map_list.list);
    map_list.count = get_attr.value.qosmap.count;

    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));

    for(loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("pfc_pri %d qindex %d\r\n",get_attr.value.qosmap.list[loop_idx].key.prio,
               get_attr.value.qosmap.list[loop_idx].value.queue_index);
    }

    ASSERT_EQ(get_attr.value.qosmap.list[1].value.queue_index, UC_Q_INDEX);
    ASSERT_EQ(get_attr.value.qosmap.list[NUM_PRIO+1].value.queue_index, (MC_Q_INDEX - num_uc));
    ASSERT_EQ(get_attr.value.qosmap.list[NUM_PRIO+2].value.queue_index, (MC_Q_INDEX - num_uc));
    free(map_list.list);

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE, sai_qos_map_api_table->remove_qos_map(map_id));
    set_attr.id = SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP;

    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id));


    ASSERT_EQ(SAI_STATUS_INVALID_OBJECT_ID,
              sai_qos_map_api_table->get_qos_map_attribute
              (map_id, 1, &get_attr));
}
