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
* @file  sai_qos_buffer_unit_test.cpp
*
* @brief This file contains tests for qos buffer APIs
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
#include "saibuffer.h"
#include "saiport.h"
#include "saiqueue.h"
#include <inttypes.h>
}

static sai_object_id_t switch_id =0;

#define LOG_PRINT(msg, ...) \
    printf(msg, ##__VA_ARGS__)
#define PG_ALL 255
#define QUEUE_ALL 255

unsigned int sai_total_buffer_size = 0;
unsigned int sai_buffer_pool_test_size_1 = 0;
unsigned int sai_buffer_pool_test_size_2 = 0;
unsigned int sai_buffer_pool_test_size_3 = 0;

unsigned int sai_buffer_profile_test_size_1 = 2048; // bytes
unsigned int sai_buffer_profile_test_size_2 = 20480;
unsigned int sai_buffer_profile_test_size_3 = 8192;
unsigned int sai_buffer_profile_test_size_4 = 4096;
unsigned int sai_buffer_profile_test_size_5 = 5120;

/* NOTE: For TH Set this to true and 4 */
#define MAX_SWITCH_TILES 1
/*
 * API query is done while running the first test case and
 * the method table is stored in sai_switch_api_table so
 * that its available for the rest of the test cases which
 * use the method table
 */
class qos_buffer : public ::testing::Test
{
    protected:
        static void SetUpTestCase()
        {
            sai_attribute_t get_attr;
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


            ASSERT_EQ(SAI_STATUS_SUCCESS,sai_api_query(SAI_API_BUFFERS,
                        (static_cast<void**>(static_cast<void*>(&sai_buffer_api_table)))));

            ASSERT_TRUE(sai_buffer_api_table != NULL);

            EXPECT_TRUE(sai_buffer_api_table->create_buffer_pool != NULL);
            EXPECT_TRUE(sai_buffer_api_table->remove_buffer_pool != NULL);
            EXPECT_TRUE(sai_buffer_api_table->set_buffer_pool_attr != NULL);
            EXPECT_TRUE(sai_buffer_api_table->get_buffer_pool_attr != NULL);
            EXPECT_TRUE(sai_buffer_api_table->set_ingress_priority_group_attr != NULL);
            EXPECT_TRUE(sai_buffer_api_table->get_ingress_priority_group_attr != NULL);
            EXPECT_TRUE(sai_buffer_api_table->create_buffer_profile != NULL);
            EXPECT_TRUE(sai_buffer_api_table->remove_buffer_profile != NULL);
            EXPECT_TRUE(sai_buffer_api_table->set_buffer_profile_attr != NULL);
            EXPECT_TRUE(sai_buffer_api_table->get_buffer_profile_attr != NULL);

            ASSERT_EQ(NULL,sai_api_query(SAI_API_PORT,
                        (static_cast<void**>(static_cast<void*>(&sai_port_api_table)))));

            ASSERT_TRUE(sai_port_api_table != NULL);

            EXPECT_TRUE(sai_port_api_table->set_port_attribute != NULL);
            EXPECT_TRUE(sai_port_api_table->get_port_attribute != NULL);
            EXPECT_TRUE(sai_port_api_table->get_port_stats != NULL);


            ASSERT_EQ(SAI_STATUS_SUCCESS,sai_api_query(SAI_API_QOS_MAPS,
                        (static_cast<void**>(static_cast<void*>(&sai_qos_map_api_table)))));

            ASSERT_TRUE(sai_qos_map_api_table != NULL);

            EXPECT_TRUE(sai_qos_map_api_table->create_qos_map != NULL);
            EXPECT_TRUE(sai_qos_map_api_table->remove_qos_map != NULL);
            EXPECT_TRUE(sai_qos_map_api_table->set_qos_map_attribute != NULL);
            EXPECT_TRUE(sai_qos_map_api_table->get_qos_map_attribute != NULL);


            ASSERT_EQ(NULL,sai_api_query(SAI_API_QUEUE,
                        (static_cast<void**>(static_cast<void*>(&sai_queue_api_table)))));

            ASSERT_TRUE(sai_queue_api_table != NULL);
            p_sai_qos_queue_api_table = sai_queue_api_table;

            EXPECT_TRUE(sai_queue_api_table->set_queue_attribute != NULL);
            EXPECT_TRUE(sai_queue_api_table->get_queue_attribute != NULL);
            EXPECT_TRUE(sai_queue_api_table->get_queue_stats != NULL);
            EXPECT_TRUE(sai_queue_api_table->create_queue != NULL);
            EXPECT_TRUE(sai_queue_api_table->remove_queue != NULL);
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

            memset(&get_attr, 0, sizeof(get_attr));
            get_attr.id = SAI_SWITCH_ATTR_TOTAL_BUFFER_SIZE;
            ASSERT_EQ(SAI_STATUS_SUCCESS,
                    sai_switch_api_table->get_switch_attribute(switch_id,1, &get_attr));
            sai_total_buffer_size = (get_attr.value.u32)*1024;
            sai_total_buffer_size = sai_total_buffer_size/MAX_SWITCH_TILES;
            sai_buffer_pool_test_size_1 = (sai_total_buffer_size/10);
            sai_buffer_pool_test_size_2 = (sai_total_buffer_size/5);
            sai_buffer_pool_test_size_3 = 1023;
            printf("Total buffer size - %d bytes\r\n", sai_total_buffer_size);
            printf("sai_buffer_pool_test_size_1 - %d bytes\r\n", sai_buffer_pool_test_size_1);
            printf("sai_buffer_pool_test_size_2 - %d bytes\r\n", sai_buffer_pool_test_size_2);
            printf("sai_buffer_pool_test_size_3 - %d bytes\r\n", sai_buffer_pool_test_size_3);
        }

        static sai_switch_api_t* sai_switch_api_table;
        static sai_buffer_api_t* sai_buffer_api_table;
        static sai_port_api_t* sai_port_api_table;
        static sai_queue_api_t* sai_queue_api_table;
        static sai_qos_map_api_t* sai_qos_map_api_table;

        static const unsigned int test_port_id = 0;
        static const unsigned int test_port_id_1 = 1;
        static const unsigned int test_port_id_2 = 3;

};

sai_switch_api_t* qos_buffer ::sai_switch_api_table = NULL;
sai_buffer_api_t* qos_buffer ::sai_buffer_api_table = NULL;
sai_port_api_t* qos_buffer ::sai_port_api_table = NULL;
sai_queue_api_t* qos_buffer ::sai_queue_api_table = NULL;
sai_qos_map_api_t* qos_buffer ::sai_qos_map_api_table = NULL;

sai_status_t sai_break_test()
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    printf ("SAI STATUS in sai_break_test %d", sai_rc);
    return sai_rc;
}


TEST_F(qos_buffer, buffer_pool_create_test)
{
    sai_attribute_t get_attr;
    sai_attribute_t create_attr[4];
    sai_object_id_t pool_id = 0;

    create_attr[0].id = SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE;
    create_attr[0].value.u32 = SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC;

    create_attr[1].id = SAI_BUFFER_POOL_ATTR_SIZE;
    create_attr[1].value.u32 = sai_buffer_pool_test_size_1;

    /** Mandatory attribute cases */
    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              sai_buffer_api_table->create_buffer_pool (&pool_id, switch_id, 2,
                                      (const sai_attribute_t *)create_attr));

    create_attr[1].id = SAI_BUFFER_POOL_ATTR_TYPE;
    create_attr[1].value.u32 = SAI_BUFFER_POOL_TYPE_INGRESS;

    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              sai_buffer_api_table->create_buffer_pool (&pool_id, switch_id, 2,
                                       (const sai_attribute_t *)create_attr));

    create_attr[2].id = SAI_BUFFER_POOL_ATTR_SIZE;
    create_attr[2].value.u32 = sai_buffer_pool_test_size_1;

    create_attr[3].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    create_attr[3].value.u32 = sai_buffer_pool_test_size_1;

    ASSERT_EQ(SAI_STATUS_CODE(SAI_STATUS_CODE(SAI_STATUS_INVALID_ATTRIBUTE_0)+3),
              sai_buffer_api_table->create_buffer_pool (&pool_id, switch_id, 4,
                                       (const sai_attribute_t *)create_attr));
    create_attr[0].id = SAI_BUFFER_POOL_ATTR_SIZE;
    create_attr[0].value.u32 = sai_buffer_pool_test_size_1;

    create_attr[1].id = SAI_BUFFER_POOL_ATTR_TYPE;
    create_attr[1].value.u32 = SAI_BUFFER_POOL_TYPE_INGRESS;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->create_buffer_pool (&pool_id, switch_id, 2,
                                       (const sai_attribute_t *)create_attr));

    /** Check if default mode is dynamic */
    get_attr.id = SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr(pool_id,
                                     1, &get_attr));
    ASSERT_EQ (get_attr.value.s32, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (pool_id));

    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_buffer_api_table->remove_buffer_pool (pool_id));
}

TEST_F(qos_buffer, buffer_pool_attr_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr;
    sai_object_id_t pool_id = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id, sai_buffer_pool_test_size_1,
                                     SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    set_attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    set_attr.value.u32 = sai_buffer_pool_test_size_2;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attr(pool_id,
                                     (const sai_attribute_t *)&set_attr));

    get_attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr(pool_id,
                                     1, &get_attr));
    ASSERT_EQ (get_attr.value.u32, sai_buffer_pool_test_size_2);

    set_attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    set_attr.value.u32 = 3;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attr(pool_id,
                                     (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    set_attr.value.u32 = sai_buffer_pool_test_size_1;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attr(pool_id,
                                     (const sai_attribute_t *)&set_attr));

    printf("Update back to sai_buffer_pool_test_size_1\r\n");
    set_attr.id = SAI_BUFFER_POOL_ATTR_TYPE;
    set_attr.value.u32 = SAI_BUFFER_POOL_TYPE_EGRESS;
    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_pool_attr(pool_id,
                                     (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE;
    set_attr.value.u32 = SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC;
    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_pool_attr(pool_id,
                                     (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    set_attr.value.u32 = sai_buffer_pool_test_size_2;
    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_pool_attr(pool_id,
                                     (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (pool_id));
}

TEST_F(qos_buffer, max_ing_num_buffer_pool_test)
{
    sai_attribute_t get_attr;
    unsigned int num_pools = 0;
    unsigned int pool_idx = 0;

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_INGRESS_BUFFER_POOL_NUM;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_switch_api_table->get_switch_attribute(switch_id,1, &get_attr));

    num_pools = get_attr.value.u32;
    sai_object_id_t pool_id[num_pools+1];

    for(pool_idx = 0; pool_idx < num_pools; pool_idx++) {
        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
                  &pool_id[pool_idx], sai_buffer_pool_test_size_2,
                  SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));
    }
    ASSERT_EQ(SAI_STATUS_INSUFFICIENT_RESOURCES, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[num_pools], sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_TYPE_INGRESS,
              SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));
    for(pool_idx = 0; pool_idx < num_pools; pool_idx++) {
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  sai_buffer_api_table->remove_buffer_pool (pool_id[pool_idx]));
    }

    memset(pool_id, 0, (num_pools+1) * sizeof(sai_object_id_t));

    for(pool_idx = 0; pool_idx < num_pools; pool_idx++) {

        printf("Create buffer pool %x \r\n",pool_idx);
        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
                  &pool_id[pool_idx], sai_buffer_pool_test_size_2,
                  SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));
    }

    for(pool_idx = 0; pool_idx < num_pools; pool_idx++) {
        if (pool_id[pool_idx]) {
            ASSERT_EQ(SAI_STATUS_SUCCESS,
                      sai_buffer_api_table->remove_buffer_pool (pool_id[pool_idx]));
        }
    }
}

TEST_F(qos_buffer, max_egr_num_buffer_pool_test)
{
    sai_attribute_t get_attr;
    unsigned int num_pools = 0;
    unsigned int pool_idx = 0;

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_EGRESS_BUFFER_POOL_NUM;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_switch_api_table->get_switch_attribute(switch_id,1, &get_attr));

    num_pools = get_attr.value.u32;
    sai_object_id_t pool_id[num_pools+1];

    for(pool_idx = 0; pool_idx < num_pools; pool_idx++) {
        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[pool_idx],
                    sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_TYPE_EGRESS,
                    SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));
    }
    ASSERT_EQ(SAI_STATUS_INSUFFICIENT_RESOURCES, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[num_pools], sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_TYPE_EGRESS,
              SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    for(pool_idx = 0; pool_idx < num_pools; pool_idx++) {
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  sai_buffer_api_table->remove_buffer_pool (pool_id[pool_idx]));
    }

}

TEST_F(qos_buffer, max_buffer_size_test)
{
    unsigned int buf_size = 0;
    sai_object_id_t pool_id[4];
    sai_attribute_t set_attr;
    unsigned int pool_idx = 0;

    buf_size = sai_total_buffer_size*2;

    ASSERT_EQ(SAI_STATUS_INSUFFICIENT_RESOURCES, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[0], buf_size, SAI_BUFFER_POOL_TYPE_EGRESS,
              SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_INSUFFICIENT_RESOURCES, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[2], buf_size, SAI_BUFFER_POOL_TYPE_INGRESS,
              SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    buf_size = (sai_total_buffer_size*3)/4;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[0], buf_size, SAI_BUFFER_POOL_TYPE_EGRESS,
              SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[2], buf_size, SAI_BUFFER_POOL_TYPE_INGRESS,
              SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_INSUFFICIENT_RESOURCES, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[1], buf_size, SAI_BUFFER_POOL_TYPE_EGRESS,
              SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_INSUFFICIENT_RESOURCES, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[3], buf_size, SAI_BUFFER_POOL_TYPE_INGRESS,
              SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (pool_id[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (pool_id[2]));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[1], buf_size, SAI_BUFFER_POOL_TYPE_EGRESS,
              SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[3], buf_size, SAI_BUFFER_POOL_TYPE_INGRESS,
              SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    buf_size = sai_total_buffer_size/4;
    set_attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    set_attr.value.u32 = buf_size;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attr(pool_id[1],
                                     (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attr(pool_id[3],
                                     (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[0], buf_size, SAI_BUFFER_POOL_TYPE_EGRESS,
              SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
              &pool_id[2], buf_size, SAI_BUFFER_POOL_TYPE_INGRESS,
              SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    for(pool_idx = 0; pool_idx < 4; pool_idx++) {
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  sai_buffer_api_table->remove_buffer_pool (pool_id[pool_idx]));
    }
}

TEST_F(qos_buffer, buffer_profile_create_test)
{
    sai_attribute_t create_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id, sai_buffer_pool_test_size_1,
                                     SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    create_attr[0].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    create_attr[0].value.u32 = sai_buffer_profile_test_size_1;

    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              sai_buffer_api_table->create_buffer_profile (&profile_id, switch_id, 1,
                                      (const sai_attribute_t *)create_attr));

    create_attr[0].id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    create_attr[0].value.oid = pool_id;

    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              sai_buffer_api_table->create_buffer_profile (&profile_id, switch_id, 1,
                                      (const sai_attribute_t *)create_attr));

    create_attr[0].value.oid = SAI_NULL_OBJECT_ID;
    create_attr[1].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    create_attr[1].value.u32 = sai_buffer_profile_test_size_1;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTR_VALUE_0,
              sai_buffer_api_table->create_buffer_profile (&pool_id, switch_id, 2,
                                       (const sai_attribute_t *)create_attr));

    create_attr[0].value.oid = pool_id;
    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              sai_buffer_api_table->create_buffer_profile (&profile_id, switch_id, 2,
                                      (const sai_attribute_t *)create_attr));

    create_attr[2].id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    create_attr[2].value.u8 = 1;

    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              sai_buffer_api_table->create_buffer_profile (&profile_id, switch_id, 3,
                                      (const sai_attribute_t *)create_attr));

    create_attr[2].id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    create_attr[2].value.u32 = sai_buffer_profile_test_size_3;

    create_attr[3].id = SAI_BUFFER_PROFILE_ATTR_XOFF_TH;
    create_attr[3].value.u32 = sai_buffer_profile_test_size_1;

    ASSERT_EQ(SAI_STATUS_CODE(SAI_STATUS_CODE(SAI_STATUS_INVALID_ATTRIBUTE_0)+3),
              sai_buffer_api_table->create_buffer_profile (&profile_id, switch_id, 4,
                                      (const sai_attribute_t *)create_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->create_buffer_profile (&profile_id, switch_id, 3,
                                      (const sai_attribute_t *)create_attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              sai_buffer_api_table->remove_buffer_pool (pool_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_profile(profile_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (pool_id));
}

TEST_F(qos_buffer, egr_buffer_profile_attr_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t pool_id_2 = 0;
    sai_object_id_t pool_id_3 = 0;
    sai_object_id_t pool_id_4 = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1, sai_buffer_pool_test_size_1,
                                     SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_3, sai_buffer_pool_test_size_2,
                                     SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_4, sai_buffer_pool_test_size_3,
                                     SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_2, sai_buffer_pool_test_size_2,
                                     SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_profile(sai_buffer_api_table, &profile_id, 0x13 ,pool_id_1,
                                        sai_buffer_profile_test_size_1, 0,
                                        0, sai_buffer_profile_test_size_3, 0,0));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_XON_TH;
    set_attr.value.u32 = sai_buffer_profile_test_size_1;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_2;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                               (const sai_attribute_t *)&set_attr));

    set_attr.value.oid = pool_id_3;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                               (const sai_attribute_t *)&set_attr));

    printf("Update pool id \r\n");
    set_attr.value.oid = pool_id_4;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                               (const sai_attribute_t *)&set_attr));
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    set_attr.value.u8 = 1;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                              (const sai_attribute_t *)&set_attr));
    printf("Update XON threshold \r\n");
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    set_attr.value.u32 = sai_buffer_profile_test_size_3;

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    printf("Get XON threshold \r\n");
    get_attr[0].id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    get_attr[1].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    get_attr[2].id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_profile_attr (profile_id, 3, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, pool_id_1);
    EXPECT_EQ (get_attr[1].value.u32, sai_buffer_profile_test_size_1);
    EXPECT_EQ (get_attr[2].value.u32, sai_buffer_profile_test_size_3);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_2));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_3));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_4));
}

TEST_F(qos_buffer, ing_buffer_profile_attr_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t pool_id_2 = 0;
    sai_object_id_t pool_id_3 = 0;
    sai_object_id_t pool_id_4 = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1, sai_buffer_pool_test_size_1,
                                     SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_3, sai_buffer_pool_test_size_2,
                                     SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_4, sai_buffer_pool_test_size_3,
                                     SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id_2, sai_buffer_pool_test_size_1,
                                     SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_profile(sai_buffer_api_table, &profile_id, 0xb ,pool_id_1,
                                        sai_buffer_profile_test_size_1, 0, 1, 0, 0, 0));
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    set_attr.value.u32 = sai_buffer_profile_test_size_2;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_XOFF_TH;
    set_attr.value.u32 = sai_buffer_profile_test_size_1;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_2;
    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                               (const sai_attribute_t *)&set_attr));
    set_attr.value.oid = pool_id_3;
    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                               (const sai_attribute_t *)&set_attr));
    set_attr.value.oid = pool_id_4;
    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                               (const sai_attribute_t *)&set_attr));
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    set_attr.value.u8 = 2;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                              (const sai_attribute_t *)&set_attr));
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    set_attr.value.u32 = sai_buffer_profile_test_size_3;
    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0,
               sai_buffer_api_table->set_buffer_profile_attr (profile_id,
                                              (const sai_attribute_t *)&set_attr));
    get_attr[0].id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    get_attr[1].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    get_attr[2].id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    get_attr[3].id = SAI_BUFFER_PROFILE_ATTR_XOFF_TH;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_profile_attr (profile_id, 4,
                                                              get_attr));
    EXPECT_EQ (get_attr[0].value.oid, pool_id_1);
    EXPECT_EQ (get_attr[1].value.u32, sai_buffer_profile_test_size_2);
    EXPECT_EQ (get_attr[2].value.u8, 2);
    EXPECT_EQ (get_attr[3].value.u32, sai_buffer_profile_test_size_1);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_2));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_3));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_4));
}

TEST_F(qos_buffer, buffer_profile_pg_basic_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t pg_obj = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_TYPE_INGRESS,
                                  SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0x6b ,pool_id_1, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_ingress_priority_group_attr (pg_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id);

    ASSERT_EQ (SAI_STATUS_OBJECT_IN_USE,
               sai_buffer_api_table->remove_buffer_profile (profile_id));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
}

TEST_F(qos_buffer, buffer_profile_pool_size_test)
{
    sai_object_id_t pool_id = 0;
    sai_object_id_t profile_id = 0;
    sai_attribute_t set_attr;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t pg_obj = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_create_buffer_pool(sai_buffer_api_table, &pool_id, sai_buffer_pool_test_size_3,
                                     SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    /** Create buffer profile with size as more than pool size and check insuffcient resources is returned*/
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0x6b ,pool_id, sai_buffer_pool_test_size_3, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj));
    /** Create with small space in pool and check insuffcient resources is returned*/
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_INSUFFICIENT_RESOURCES, sai_buffer_api_table->
               set_ingress_priority_group_attr (pg_obj, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id));
}
TEST_F(qos_buffer, buffer_profile_pg_size_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t pool_id_2 = 0;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t pg_obj = SAI_NULL_OBJECT_ID;
    int shared_size = sai_buffer_pool_test_size_1 * MAX_SWITCH_TILES;
    int shared_size_2 = sai_buffer_pool_test_size_2 * MAX_SWITCH_TILES;
    int shared_size_3 = sai_buffer_pool_test_size_3 * MAX_SWITCH_TILES;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1,
              sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_2,
              sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0x6b ,pool_id_1, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_buffer_pool_attr (pool_id_1, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.u32,
              (shared_size - (sai_buffer_profile_test_size_2 + sai_buffer_profile_test_size_1)));

    /** Change size of PG */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    set_attr.value.u32 = sai_buffer_profile_test_size_1;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               set_buffer_profile_attr (profile_id, (const sai_attribute_t *)&set_attr));
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr (
                                               pool_id_1, 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (shared_size - (sai_buffer_profile_test_size_1 + sai_buffer_profile_test_size_1)));

    /** Change Xoff size of PG */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_XOFF_TH;
    set_attr.value.u32 = sai_buffer_profile_test_size_2;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               set_buffer_profile_attr (profile_id, (const sai_attribute_t *)&set_attr));
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr (
                                               pool_id_1, 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (shared_size - (sai_buffer_profile_test_size_1 + sai_buffer_profile_test_size_2)));

    /** Move PG to a different pool, not vlaid */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_2;
    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_profile_attr (
                                              profile_id, (const sai_attribute_t *)&set_attr));
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_pool_attr (pool_id_1, 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (shared_size - (sai_buffer_profile_test_size_1 + sai_buffer_profile_test_size_2)));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_pool_attr (pool_id_2, 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32, shared_size_2);

    /** Change pool size */
    set_attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    set_attr.value.u32 = sai_buffer_pool_test_size_3;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attr (
                                              pool_id_2, (const sai_attribute_t *)&set_attr));
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_buffer_pool_attr (pool_id_2, 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32, shared_size_3);

    /** Remove PG from pool */
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_buffer_pool_attr (pool_id_1, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.u32, shared_size);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_2));
}

TEST_F(qos_buffer, pg_buffer_profile_replace_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id[5] = {0};
    sai_object_id_t pool_id[4] = {0};
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t pg_obj = SAI_NULL_OBJECT_ID;
    int shared_size_1 = sai_buffer_pool_test_size_1 * MAX_SWITCH_TILES;
    int shared_size_2 = sai_buffer_pool_test_size_2 * MAX_SWITCH_TILES;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[0],
              sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[1],
              sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[2],
              sai_buffer_pool_test_size_3, SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[3],
              sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[0],
              0x6b ,pool_id[0], sai_buffer_profile_test_size_2, 0, 1, 0,
              sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[1],
              0x6b ,pool_id[0], sai_buffer_profile_test_size_1, 0, 1, 0,
              sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[2],
              0x6b ,pool_id[1], sai_buffer_profile_test_size_1, 0, 1, 0,
              sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[3],
              0x73 ,pool_id[2], sai_buffer_profile_test_size_2, 0, 0, sai_buffer_profile_test_size_3,
              sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));

    /* profile with egress pool */
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[4],
              0xb ,pool_id[3], sai_buffer_profile_test_size_2, 0, 1, 0, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id[0];
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    set_attr.value.oid =  profile_id[3];
    ASSERT_EQ (SAI_STATUS_INVALID_PARAMETER, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    set_attr.value.oid =  profile_id[4];
    ASSERT_EQ (SAI_STATUS_INVALID_PARAMETER, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    set_attr.value.oid = profile_id[1];
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_ingress_priority_group_attr (
                                               pg_obj, 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.oid, profile_id[1]);

        memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr (
                                               pool_id[0], 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (shared_size_1 - (sai_buffer_profile_test_size_1 + sai_buffer_profile_test_size_1)));

    set_attr.value.oid = profile_id[2];
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr (
                                               pool_id[0], 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.u32, shared_size_1);

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_pool_attr (pool_id[1], 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (shared_size_2 - (sai_buffer_profile_test_size_1 + sai_buffer_profile_test_size_1)));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[0]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[1]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[2]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[3]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[4]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[0]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[1]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[2]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[3]));
}

TEST_F(qos_buffer, buffer_profile_pg_profile_th_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id_1 = 0;
    sai_object_id_t profile_id_2 = 0;
    sai_object_id_t profile_id_3 = 0;
    sai_object_id_t profile_id_4 = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t pool_id_2 = 0;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t pg_obj = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_TYPE_INGRESS,
                                  SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_2,
                                  sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_TYPE_INGRESS,
                                  SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    /* Local profile is static while pool is dynamic */
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_1,
                                  0x17 ,pool_id_1, sai_buffer_profile_test_size_2,
                                  SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC, 0,
                                  sai_buffer_profile_test_size_3, 0, 0));

    /* Local profile is dynamic while pool is static */
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_2,
                                  0xf ,pool_id_2, sai_buffer_profile_test_size_2,
                                  SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC, 1, 0, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_3,
                                  0xb ,pool_id_1, sai_buffer_profile_test_size_2,
                                  SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC, 1, 0, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_4,
                                  0x13 ,pool_id_2, sai_buffer_profile_test_size_2,
                                  SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC, 0, sai_buffer_profile_test_size_3, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj));
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id_1;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));
    /* Reseevred 1MB */
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_ingress_priority_group_attr (pg_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_1);

    printf("Replace with profile 2 \r\n");
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id_2;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));
   // reserved is 1MB
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_ingress_priority_group_attr (pg_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_2);

    printf("Replace with profile 3 \r\n");

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id_3;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));
    // reseve 1MB
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_ingress_priority_group_attr (pg_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_3);
    printf("After profile 3 \r\n");

    /* Error should be returned since profile th is not enabled*/
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    set_attr.value.u32 = 0;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));
    printf("modify pool id \r\n");

    /* Set profile th mode and change profile value. Setting to 0 which should be setting to unlimited value */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE;
    set_attr.value.oid = SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));

    printf(" SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH \r\n");
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    set_attr.value.u32 = 0;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));
    printf("modify pool id \r\n");
    /* Change buffer pool to a differnt type since profile th is enabled */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_2;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));

    printf("Applying profile id 4 \r\n");
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id_4;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_ingress_priority_group_attr (pg_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_4);

    /* Error should be returned since profile th is not enabled*/
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    set_attr.value.u32 = 2;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));
    /* Set profile th mode and change profile value. */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE;
    set_attr.value.oid = SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    set_attr.value.u32 = 2;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));

    /* Change buffer pool to a differnt type since profile th is enabled */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_1;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));

    printf("Removing profile from pg \r\n");
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_obj, (const sai_attribute_t *)&set_attr));


    printf("After De attch profile from all the pools \r\n");
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_2));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_3));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_4));

    printf("After removing buffer profiles \r\n");
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_2));
    printf("After removing pools pools \r\n");
}

TEST_F(qos_buffer, buffer_profile_queue_basic_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t queue_obj = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_TYPE_EGRESS,
                                  SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0xb ,pool_id_1, sai_buffer_profile_test_size_2, 0, 1, 0, 0, 0));


    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id, &queue_obj));
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->
               get_queue_attribute (queue_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id);

    ASSERT_EQ (SAI_STATUS_OBJECT_IN_USE,
               sai_buffer_api_table->remove_buffer_profile (profile_id));

    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
}

TEST_F(qos_buffer, buffer_profile_queue_profile_th_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id_1 = 0;
    sai_object_id_t profile_id_2 = 0;
    sai_object_id_t profile_id_3 = 0;
    sai_object_id_t profile_id_4 = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t pool_id_2 = 0;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t queue_obj = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_TYPE_EGRESS,
                                  SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_2,
                                  sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_TYPE_EGRESS,
                                  SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    /* Local profile is static while pool is dynamic */
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_1,
                                  0x17 ,pool_id_1, sai_buffer_profile_test_size_2,
                                  SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC, 0,
                                  sai_buffer_profile_test_size_3, 0, 0));

    /* Local profile is dynamic while pool is static */
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_2,
                                  0xf ,pool_id_2, sai_buffer_profile_test_size_2,
                                  SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC, 1, 0, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_3,
                                  0xb ,pool_id_1, sai_buffer_profile_test_size_2,
                                  0, 1, 0, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_4,
                                  0x13 ,pool_id_2, sai_buffer_profile_test_size_2,
                                  0, 0, sai_buffer_profile_test_size_3, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id, &queue_obj));
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id_1;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->
               get_queue_attribute (queue_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_1);

    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id_2;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->
               get_queue_attribute (queue_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_2);

    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id_3;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->
               get_queue_attribute (queue_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_3);

    /* Error should be returned since profile th is not enabled*/
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    set_attr.value.u32 = 0;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));

    /* Set profile th mode and change profile value. Setting to 0 which should be setting to unlimited value */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE;
    set_attr.value.oid = SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    set_attr.value.u32 = 0;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));

    /* Change buffer pool to a differnt type since profile th is enabled */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_2;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_profile_attr (profile_id_3,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id_4;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->
               get_queue_attribute (queue_obj, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_4);

    /* Error should be returned since profile th is not enabled*/
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    set_attr.value.u32 = 2;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));

    /* Set profile th mode and change profile value. */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_THRESHOLD_MODE;
    set_attr.value.oid = SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    set_attr.value.u32 = 2;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));

    /* Change buffer pool to a differnt type since profile th is enabled */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_1;

    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_profile_attr (profile_id_4,
                                              (const sai_attribute_t *)&set_attr));
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_2));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_3));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_4));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_2));
}

TEST_F(qos_buffer, buffer_profile_queue_size_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t pool_id_2 = 0;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t queue_obj = SAI_NULL_OBJECT_ID;

    int shared_size_2 = sai_buffer_pool_test_size_2 * MAX_SWITCH_TILES;
    int shared_size_1 = sai_buffer_pool_test_size_1 * MAX_SWITCH_TILES;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1,
              sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_2,
              sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0xb ,pool_id_1, sai_buffer_profile_test_size_1, 0, 1, 0, 0,0));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id, &queue_obj));

    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_buffer_pool_attr (pool_id_1, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.u32,
              (shared_size_2 - sai_buffer_profile_test_size_1));

    printf("Modify SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE to > 1MB  + 1  \r\n");
    /** Change size of QUEUE */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    set_attr.value.u32 = sai_buffer_profile_test_size_2;

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               set_buffer_profile_attr (profile_id, (const sai_attribute_t *)&set_attr));
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr (
                                               pool_id_1, 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (shared_size_2 - sai_buffer_profile_test_size_2));

    /** Move QUEUE to a different pool */
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    set_attr.value.oid = pool_id_2;
    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_buffer_api_table->set_buffer_profile_attr (
                                              profile_id, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_pool_attr (pool_id_1, 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32, (shared_size_2 - sai_buffer_profile_test_size_2));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_pool_attr (pool_id_2, 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32, (shared_size_1));

    /** Change pool size */
    set_attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    set_attr.value.u32 = sai_buffer_pool_test_size_2;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attr (
                                              pool_id_2, (const sai_attribute_t *)&set_attr));
    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_buffer_pool_attr (pool_id_2, 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32, (shared_size_2));

    /** Remove QUEUE from pool */
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_buffer_pool_attr (pool_id_2, 1, get_attr));

    EXPECT_EQ (get_attr[0].value.u32, shared_size_2);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_2));
}

TEST_F(qos_buffer, queue_buffer_profile_replace_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id[5] = {0};
    sai_object_id_t pool_id[4] = {0};
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t queue_obj = SAI_NULL_OBJECT_ID;

    int shared_size_2 = sai_buffer_pool_test_size_2 * MAX_SWITCH_TILES;
    int shared_size_1 = sai_buffer_pool_test_size_1 * MAX_SWITCH_TILES;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[0],
              sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[1],
              sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[2],
              sai_buffer_pool_test_size_3, SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id[3],
              sai_buffer_pool_test_size_2, SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[0],
              0xb ,pool_id[0], sai_buffer_profile_test_size_2, 0, 1, 0, 0, 0));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[1],
              0xb ,pool_id[0], sai_buffer_profile_test_size_1, 0, 1, 0, 0, 0));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[2],
              0xb ,pool_id[1], sai_buffer_profile_test_size_1, 0, 1, 0, 0, 0));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[3],
              0x13 ,pool_id[2], sai_buffer_profile_test_size_2, 0, 0, sai_buffer_profile_test_size_3,0,0));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id[4],
              0xb ,pool_id[3], sai_buffer_profile_test_size_2, 0, 1, 0, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id, &queue_obj));

    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id[0];
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    set_attr.value.oid =  profile_id[3];
    ASSERT_EQ (SAI_STATUS_INVALID_PARAMETER, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    set_attr.value.oid =  profile_id[4];
    ASSERT_EQ (SAI_STATUS_INVALID_PARAMETER, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));
    set_attr.value.oid = profile_id[1];
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->get_queue_attribute (
                                               queue_obj, 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.oid, profile_id[1]);

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr (
                                               pool_id[0], 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (shared_size_1 - sai_buffer_profile_test_size_1));

    set_attr.value.oid = profile_id[2];
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->get_buffer_pool_attr (
                                               pool_id[0], 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.u32, shared_size_1);

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_BUFFER_POOL_ATTR_SHARED_SIZE;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->get_buffer_pool_attr (pool_id[1], 1, get_attr));
    EXPECT_EQ (get_attr[0].value.u32,
              (shared_size_2 - sai_buffer_profile_test_size_1));

    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_obj, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[0]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[1]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[2]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[3]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_profile (profile_id[4]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[0]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[1]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[2]));
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->remove_buffer_pool (pool_id[3]));
}

TEST_F (qos_buffer, ingress_buffer_pool_stats_get)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t pg_id;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id = 0;
    sai_attribute_t set_attr;
    sai_buffer_pool_stat_t counter_id[] =
                   {SAI_BUFFER_POOL_STAT_CURR_OCCUPANCY_BYTES,
                    SAI_BUFFER_POOL_STAT_WATERMARK_BYTES
                    };
    uint64_t counter_val;
    unsigned int idx = 0;
    unsigned int num_counters;

    num_counters =  sizeof(counter_id)/sizeof(sai_buffer_pool_stat_t);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_TYPE_INGRESS,
                                  SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0x6b ,pool_id, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_id));
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_id, (const sai_attribute_t *)&set_attr));

    for(idx = 0; idx < num_counters; idx++) {
        sai_rc = sai_buffer_api_table->get_buffer_pool_stats(pool_id, &counter_id[idx],
                                                            1, &counter_val);
        if(sai_rc == SAI_STATUS_SUCCESS) {
             printf("Get counter ID %d is supported. Val:0x%" PRIx64 "\r\n",counter_id[idx],counter_val);
        } else if( sai_rc == SAI_STATUS_NOT_SUPPORTED) {
             printf("Get counter ID %d is not supported.\r\n",counter_id[idx]);
        } else {
             printf("Get counter ID %d get returned error %d\r\n",counter_id[idx], sai_rc);
        }
    }
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_id, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id));

}

TEST_F (qos_buffer, egress_buffer_pool_stats_get)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t queue_id;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id = 0;
    sai_attribute_t set_attr;
    sai_buffer_pool_stat_t counter_id[] =
                   {SAI_BUFFER_POOL_STAT_CURR_OCCUPANCY_BYTES,
                    SAI_BUFFER_POOL_STAT_WATERMARK_BYTES
                    };
    uint64_t counter_val;
    unsigned int idx = 0;
    unsigned int num_counters;

    num_counters =  sizeof(counter_id)/sizeof(sai_buffer_pool_stat_t);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_TYPE_EGRESS,
                                  SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0xb ,pool_id, sai_buffer_profile_test_size_2, 0, 1, 0, 0, 0));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id, &queue_id));


    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_id, (const sai_attribute_t *)&set_attr));


    for(idx = 0; idx < num_counters; idx++) {
        sai_rc = sai_buffer_api_table->get_buffer_pool_stats(pool_id, &counter_id[idx],
                                                            1, &counter_val);
        if(sai_rc == SAI_STATUS_SUCCESS) {
             printf("Get counter ID %d is supported. Val:0x%" PRIx64 "\r\n",counter_id[idx],counter_val);
        } else if( sai_rc == SAI_STATUS_NOT_SUPPORTED) {
             printf("Get counter ID %d is not supported.\r\n",counter_id[idx]);
        } else {
             printf("Get counter ID %d get returned error %d\r\n",counter_id[idx], sai_rc);
        }
    }
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                                               (queue_id, (const sai_attribute_t *)&set_attr));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id));

}
TEST_F (qos_buffer, pg_stats_get)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t pg_id;
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id = 0;
    sai_attribute_t set_attr;

    sai_ingress_priority_group_stat_t counter_id[] =
                   {SAI_INGRESS_PRIORITY_GROUP_STAT_PACKETS,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_WATERMARK_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_WATERMARK_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_WATERMARK_BYTES
                    };
    uint64_t counter_val;
    unsigned int idx = 0;
    unsigned int num_counters;

    num_counters =  sizeof(counter_id)/sizeof(sai_ingress_priority_group_stat_t);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_TYPE_INGRESS,
                                  SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0x6b ,pool_id, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_id));
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_id, (const sai_attribute_t *)&set_attr));
    for(idx = 0; idx < num_counters; idx++) {
        sai_rc = sai_buffer_api_table->get_ingress_priority_group_stats(pg_id, &counter_id[idx],
                                                            1, &counter_val);
        if(sai_rc == SAI_STATUS_SUCCESS) {
             printf("Get counter ID %d is supported. Val:0x%" PRIx64 "\r\n",counter_id[idx],counter_val);
        } else if( sai_rc == SAI_STATUS_NOT_SUPPORTED) {
             printf("Get counter ID %d is not supported.\r\n",counter_id[idx]);
        } else {
             printf("Get counter ID %d get returned error %d\r\n",counter_id[idx], sai_rc);
        }
    }
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_id, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id));

}

TEST_F(qos_buffer, pg_stats_clear)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t port_id = sai_qos_port_id_get (0);
    sai_object_id_t pg_id;
    sai_object_id_t profile_id = 0;
    sai_object_id_t pool_id = 0;
    sai_attribute_t set_attr;
    sai_ingress_priority_group_stat_t counter_id[] =
                   {SAI_INGRESS_PRIORITY_GROUP_STAT_PACKETS,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_WATERMARK_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_WATERMARK_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_WATERMARK_BYTES
                    };
    unsigned int idx = 0;
    unsigned int num_counters;

    num_counters =  sizeof(counter_id)/sizeof(sai_ingress_priority_group_stat_t);

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_TYPE_INGRESS,
                                  SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0x6b ,pool_id, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_id));
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_id, (const sai_attribute_t *)&set_attr));

    for(idx = 0; idx < num_counters; idx++) {
        sai_rc = sai_buffer_api_table->clear_ingress_priority_group_stats(pg_id, 1, &counter_id[idx]);
        if(sai_rc == SAI_STATUS_SUCCESS) {
             printf("Clear counter ID %d is supported\r\n",counter_id[idx]);
        } else if( sai_rc == SAI_STATUS_NOT_SUPPORTED) {
             printf("Clear counter ID %d is not supported.\r\n",counter_id[idx]);
        } else {
             printf("Clear counter ID %d get returned error %d\r\n",counter_id[idx], sai_rc);
        }
    }

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attr
                                               (pg_id, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id));
}


sai_status_t
sai_create_dot1p_to_tc_map (sai_qos_map_api_t* sai_qos_map_api_table,
                            sai_object_id_t *map_id)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_qos_map_list_t map_list;
    sai_status_t sai_rc;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_DOT1P_TO_TC_AND_COLOR;
    attr_count ++;

    map_list.count = 8;
    map_list.list = (sai_qos_map_t *)calloc(8, sizeof(sai_qos_map_t));

    for ( ; loop_idx < map_list.count; ++loop_idx)
    {
        map_list.list[loop_idx].key.dot1p = loop_idx;
        map_list.list[loop_idx].value.tc = loop_idx;
        map_list.list[loop_idx].value.color = SAI_PACKET_COLOR_GREEN;
    }

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;
    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    sai_rc = sai_qos_map_api_table->create_qos_map
              (map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list);

    free(map_list.list);

    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        return sai_rc;
    }

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 16;
    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    sai_rc = sai_qos_map_api_table->get_qos_map_attribute
                 (*map_id, 1, &get_attr);

    printf("Number of entries in QOS MAP : %d\r\n", get_attr.value.qosmap.count);
    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        free(map_list.list);
        return sai_rc;
    }

    for (loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("dot1p %d tc %d %d color\r\n",get_attr.value.qosmap.list[loop_idx].key.dot1p,
               get_attr.value.qosmap.list[loop_idx].value.tc,
               get_attr.value.qosmap.list[loop_idx].value.color);
    }
    free(map_list.list);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_create_tc_to_pg_map(sai_qos_map_api_t* sai_qos_map_api_table,
                        bool pause_enable,
                        bool is_default,
                        sai_object_id_t *map_id)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_qos_map_list_t map_list;
    sai_status_t sai_rc;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;
    unsigned int pg_id = 0;

    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP;

    attr_count ++;
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 8;
    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    pg_id = 7;
    for (loop_idx = 0; loop_idx < map_list.count; loop_idx ++)
    {
        map_list.list[loop_idx].key.tc = loop_idx;
        if (pause_enable == true)
        {
            map_list.list[loop_idx].value.pg = 7;
        }
        else
        {
            if (is_default) {
                map_list.list[loop_idx].value.pg = loop_idx;
            } else {
                map_list.list[loop_idx].value.pg = pg_id;
                if (pg_id == 0)
                    pg_id = 7;
                --pg_id;
            }
        }
    }

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    sai_rc = sai_qos_map_api_table->create_qos_map
              (map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list);

    free(map_list.list);
    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        return sai_rc;
    }

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 8;
    map_list.list = (sai_qos_map_t *)calloc(8, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;


    sai_rc = sai_qos_map_api_table->get_qos_map_attribute
                 (*map_id, 1, &get_attr);

    printf("Number of entries in QOS MAP : %d\r\n", get_attr.value.qosmap.count);
    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        free(map_list.list);

        map_list.count = get_attr.value.qosmap.count;
        map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

        get_attr.value.qosmap.count = map_list.count;
        get_attr.value.qosmap.list = map_list.list;

        sai_rc = sai_qos_map_api_table->get_qos_map_attribute (*map_id, 1, &get_attr);
    }

    for (loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("tc %d pg %d\r\n",get_attr.value.qosmap.list[loop_idx].key.tc,
               get_attr.value.qosmap.list[loop_idx].value.pg);
    }
    free(map_list.list);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_create_tc_to_queue_map(sai_qos_map_api_t* sai_qos_map_api_table,
                           sai_object_id_t *map_id)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_qos_map_list_t map_list;
    sai_status_t sai_rc;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;

    attr_count = 0;
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_TC_TO_QUEUE;

    attr_count ++;
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 8;
    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    for (loop_idx = 0; loop_idx < map_list.count; loop_idx ++)
    {
        map_list.list[loop_idx].key.tc = loop_idx;
        map_list.list[loop_idx].value.queue_index = loop_idx;
    }

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    sai_rc = sai_qos_map_api_table->create_qos_map
              (map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list);

    free(map_list.list);
    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        return sai_rc;
    }

    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 16;
    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    sai_rc = sai_qos_map_api_table->get_qos_map_attribute
                  (*map_id, 1, &get_attr);

    printf("Number of entries in QOS MAP : %d\r\n", get_attr.value.qosmap.count);

    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        free(map_list.list);

        map_list.count = get_attr.value.qosmap.count;

        map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

        get_attr.value.qosmap.count = map_list.count;
        get_attr.value.qosmap.list = map_list.list;

        sai_rc =  sai_qos_map_api_table->get_qos_map_attribute
                   (*map_id, 1, &get_attr);
    }

    for (loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("tc %d queue %d\r\n",get_attr.value.qosmap.list[loop_idx].key.tc,
               get_attr.value.qosmap.list[loop_idx].value.queue_index);
    }
    free(map_list.list);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_create_pfc_prio_to_queue_map (sai_qos_map_api_t* sai_qos_map_api_table,
                                 sai_object_id_t *map_id)
{
    sai_attribute_t attr_list[3];
    sai_attribute_t get_attr;
    sai_qos_map_list_t map_list;
    sai_status_t sai_rc;
    unsigned int attr_count = 0;
    unsigned int loop_idx = 0;

    attr_count = 0;
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE;

    attr_count ++;
    attr_list[attr_count].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 8;
    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    for (loop_idx = 0; loop_idx < map_list.count; loop_idx ++)
    {
        map_list.list[loop_idx].key.prio = loop_idx;
        map_list.list[loop_idx].value.queue_index = loop_idx;
    }

    attr_list[attr_count].value.qosmap.count = map_list.count;
    attr_list[attr_count].value.qosmap.list = map_list.list;
    attr_count ++;

    sai_rc = sai_qos_map_api_table->create_qos_map
              (map_id, switch_id, attr_count, (const sai_attribute_t *)attr_list);

    free(map_list.list);
    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        return sai_rc;
    }
    get_attr.id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;

    map_list.count = 16;
    map_list.list = (sai_qos_map_t *)calloc(16, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    sai_rc = sai_qos_map_api_table->get_qos_map_attribute
                  (*map_id, 1, &get_attr);
    free(map_list.list);

    map_list.count = get_attr.value.qosmap.count;

    map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

    get_attr.value.qosmap.count = map_list.count;
    get_attr.value.qosmap.list = map_list.list;

    sai_rc =  sai_qos_map_api_table->get_qos_map_attribute
                   (*map_id, 1, &get_attr);

    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        free(map_list.list);

        map_list.count = get_attr.value.qosmap.count;

        map_list.list = (sai_qos_map_t *)calloc(map_list.count, sizeof(sai_qos_map_t));

        get_attr.value.qosmap.count = map_list.count;
        get_attr.value.qosmap.list = map_list.list;

        sai_rc =  sai_qos_map_api_table->get_qos_map_attribute
                   (*map_id, 1, &get_attr);
    }

    for (loop_idx = 0; loop_idx < get_attr.value.qosmap.count; loop_idx ++)
    {
        printf("pfc prio %d queue %d\r\n",get_attr.value.qosmap.list[loop_idx].key.prio,
               get_attr.value.qosmap.list[loop_idx].value.queue_index);
    }
    free(map_list.list);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_enable_llfc(sai_port_api_t* sai_port_api_table,
                unsigned int port_id)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr;
    sai_status_t sai_rc;

    memset(&set_attr, 0, sizeof(set_attr));
    memset(&get_attr, 0, sizeof(get_attr));

    set_attr.id = SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE;
    set_attr.value.s32 = SAI_PORT_FLOW_CONTROL_MODE_BOTH_ENABLE;

    sai_rc = sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(port_id), (const sai_attribute_t *)&set_attr);

    get_attr.id = SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE;
    sai_rc = sai_port_api_table->get_port_attribute (sai_qos_port_id_get(port_id), 1, &get_attr);
    if (sai_rc == SAI_STATUS_SUCCESS) {
        printf("Link level Flow control set is 0x%x and fetched is 0x%x \r\n",
                            set_attr.value.s32, get_attr.value.s32);
    }

    return sai_rc;
}

sai_status_t
sai_enable_pfc(sai_port_api_t* sai_port_api_table,
               unsigned int port_id)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr;
    sai_status_t sai_rc;

    memset(&set_attr, 0, sizeof(set_attr));
    memset(&get_attr, 0, sizeof(get_attr));
    set_attr.id = SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE;
    set_attr.value.s32 = SAI_PORT_FLOW_CONTROL_MODE_DISABLE;

    sai_rc = sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(port_id), (const sai_attribute_t *)&set_attr);

    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        return sai_rc;
    }

    memset(&set_attr, 0, sizeof(set_attr));
    set_attr.id = SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL;
    set_attr.value.u8 = 0xff;

    sai_rc = sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(port_id), (const sai_attribute_t *)&set_attr);

    get_attr.id = SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL;
    sai_rc = sai_port_api_table->get_port_attribute (sai_qos_port_id_get(port_id), 1, &get_attr);
    if (sai_rc == SAI_STATUS_SUCCESS) {
        printf("Priority based Flow control set is 0x%x and fetched is 0x%x\r\n",
                            set_attr.value.u8, get_attr.value.u8);
    }

    return sai_rc;
}

sai_status_t
sai_set_port_max_mtu(sai_switch_api_t* sai_switch_api_table,
                     sai_port_api_t* sai_port_api_table,
                     unsigned int port_id)
{
    unsigned int max_mtu = 0;
    sai_attribute_t sai_attr;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    memset(&sai_attr, 0, sizeof(sai_attribute_t));
    sai_attr.id = SAI_SWITCH_ATTR_PORT_MAX_MTU;

    sai_rc = sai_switch_api_table->get_switch_attribute(switch_id,1, &sai_attr);

    if ((sai_rc != SAI_STATUS_SUCCESS)
                || (sai_attr.value.u32 <= 0))
    {
        return SAI_STATUS_FAILURE;
    }
    max_mtu = sai_attr.value.u32;

    memset(&sai_attr, 0, sizeof(sai_attribute_t));
    sai_attr.id = SAI_PORT_ATTR_MTU;
    sai_attr.value.u32 = max_mtu;

    sai_rc = sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(port_id), (const sai_attribute_t *)&sai_attr);

    return sai_rc;
}

sai_status_t
sai_get_ingress_pg_stats(sai_port_api_t* sai_port_api_table,
                         sai_buffer_api_t* sai_buffer_api_table,
                         unsigned int port_id)
{
    sai_object_id_t pg_id[16];
    sai_ingress_priority_group_stat_t counter_id[] =
                   {SAI_INGRESS_PRIORITY_GROUP_STAT_PACKETS,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_WATERMARK_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_SHARED_WATERMARK_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_CURR_OCCUPANCY_BYTES,
                    SAI_INGRESS_PRIORITY_GROUP_STAT_XOFF_ROOM_WATERMARK_BYTES
                   };
    unsigned int num_counters;
    unsigned int num_pg = 0;
    sai_attribute_t get_attr;
    unsigned int loop_idx = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    uint64_t counter_val = 0;

    num_counters =  sizeof(counter_id)/sizeof(sai_buffer_pool_stat_t);

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_PORT_ATTR_NUMBER_OF_INGRESS_PRIORITY_GROUPS;
    sai_rc = sai_port_api_table->get_port_attribute (sai_qos_port_id_get(port_id), 1, &get_attr);

    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        return sai_rc;
    }

    num_pg = get_attr.value.u32;
    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST;
    get_attr.value.objlist.count = num_pg;
    get_attr.value.objlist.list = pg_id;
    sai_rc = sai_port_api_table->get_port_attribute (sai_qos_port_id_get(port_id), 1, &get_attr);

    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        return sai_rc;
    }

    for (loop_idx = 0; loop_idx < num_pg; loop_idx++)
    {
        printf(" INGRESS PG %d STATISTICS - PG ID %lx\n", loop_idx, pg_id[loop_idx]);
        for (unsigned int idx = 0; idx < num_counters; idx++)
        {
            sai_rc = sai_buffer_api_table->get_ingress_priority_group_stats(pg_id[loop_idx], &counter_id[idx],
                                                            1, &counter_val);
            if (sai_rc == SAI_STATUS_SUCCESS) {
                printf("Get counter ID %d is supported. Val:0x%" PRIx64 "\r\n",counter_id[idx],counter_val);
            } else if( sai_rc == SAI_STATUS_NOT_SUPPORTED) {
                printf("Get counter ID %d is not supported.\r\n",counter_id[idx]);
            } else {
                printf("Get counter ID %d get returned error %d\r\n",counter_id[idx], sai_rc);
            }
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_get_egress_queue_stats(sai_port_api_t* sai_port_api_table,
                           sai_queue_api_t* sai_queue_api_table,
                           unsigned int port_id)
{
    sai_attribute_t get_attr;
    unsigned int loop_idx = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    uint64_t counter_val = 0;
    sai_object_id_t queue_id[50];
    unsigned int num_queue = 0;
    sai_object_id_t queue_obj = SAI_NULL_OBJECT_ID;

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES;
    sai_rc = sai_port_api_table->get_port_attribute (sai_qos_port_id_get(port_id), 1, &get_attr);

    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        return sai_rc;
    }

    num_queue = get_attr.value.u32;

    get_attr.id = SAI_PORT_ATTR_QOS_QUEUE_LIST;
    get_attr.value.objlist.count = num_queue;
    get_attr.value.objlist.list = queue_id;

    sai_rc = sai_port_api_table->get_port_attribute (sai_qos_port_id_get(port_id), 1, &get_attr);

    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        return sai_rc;
    }

    for (loop_idx = 0; loop_idx < num_queue; loop_idx++)
    {
        queue_obj = queue_id[loop_idx];
        printf(" EGRESS QUEUE %d STATISTICS : Q ID %lx\n", loop_idx, queue_obj);
        for (unsigned int cntr_id  = SAI_QUEUE_STAT_PACKETS;
             cntr_id <= SAI_QUEUE_STAT_SHARED_WATERMARK_BYTES; cntr_id++)
        {

            sai_rc = sai_queue_api_table->get_queue_stats(queue_obj,
                                                          (const sai_queue_stat_t*)&cntr_id,
                                                          1, &counter_val);
            if (sai_rc == SAI_STATUS_SUCCESS) {
                 printf("Counter ID %d is supported. Val:0x%" PRIx64 "\r\n", cntr_id, counter_val);
            } else if( sai_rc == SAI_STATUS_NOT_SUPPORTED) {
                 printf("Counter ID %d is not supported.\r\n", cntr_id);
            } else {
                 printf("Counter ID %d get returned error %d\r\n", cntr_id, sai_rc);
            }
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_set_ingress_profile_id_for_pg(sai_port_api_t* sai_port_api_table,
                                  sai_buffer_api_t* sai_buffer_api_table,
                                  unsigned int port_id,
                                  sai_object_id_t ing_profile_id,
                                  unsigned int pg_index)
{
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    unsigned int loop_idx = 0;
    unsigned int num_pg = 0;
    sai_object_id_t pg_id[16];
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_PORT_ATTR_NUMBER_OF_INGRESS_PRIORITY_GROUPS;
    sai_rc = sai_port_api_table->get_port_attribute (sai_qos_port_id_get(port_id), 1, &get_attr);

    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        return sai_rc;
    }

    num_pg = 8;

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST;
    get_attr.value.objlist.count = num_pg;
    get_attr.value.objlist.list = pg_id;
    sai_rc = sai_port_api_table->get_port_attribute (sai_qos_port_id_get(port_id), 1, &get_attr);

    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        return sai_rc;
    }

    memset(&set_attr, 0, sizeof(set_attr));
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = ing_profile_id;

    if (pg_index == PG_ALL) {
        for (loop_idx = 0; loop_idx < num_pg; loop_idx++)
        {
            printf("Setting Buffer profile for pg no %d \r\n", loop_idx);
            sai_rc = sai_buffer_api_table->set_ingress_priority_group_attr
                       (pg_id[loop_idx], (const sai_attribute_t *)&set_attr);
            if (sai_rc != SAI_STATUS_SUCCESS)
                printf("Setting Buffer profile failed for pg no %d \r\n", loop_idx);
        }
    } else {
        if (pg_index > num_pg)
            return SAI_STATUS_FAILURE;
        printf("Setting Buffer profile for pg no %d \r\n", pg_index);
        sai_rc = sai_buffer_api_table->set_ingress_priority_group_attr
                       (pg_id[pg_index], (const sai_attribute_t *)&set_attr);
        if (sai_rc != SAI_STATUS_SUCCESS)
            printf("Setting Buffer profile failed for pg no %d \r\n", pg_index);
    }

    return sai_rc;
}

sai_status_t
sai_set_egress_profile_for_queue (sai_port_api_t* sai_port_api_table,
                                  sai_queue_api_t* sai_queue_api_table,
                                  unsigned int port_id,
                                  sai_object_id_t egr_profile_id,
                                  unsigned int queue_index)
{
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    unsigned int loop_idx = 0;
    unsigned int num_queue = 0;
    sai_object_id_t queue_obj = SAI_NULL_OBJECT_ID;
    sai_object_id_t queue_id[50] = {0};
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    get_attr.id = SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES;
    sai_rc = sai_port_api_table->get_port_attribute (sai_qos_port_id_get(port_id), 1, &get_attr);

    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        return sai_rc;
    }

    num_queue = get_attr.value.u32;

    get_attr.id = SAI_PORT_ATTR_QOS_QUEUE_LIST;
    get_attr.value.objlist.count = num_queue;
    get_attr.value.objlist.list = queue_id;
    sai_rc = sai_port_api_table->get_port_attribute (sai_qos_port_id_get(port_id), 1, &get_attr);

    if (sai_rc != SAI_STATUS_SUCCESS)
    {
        return sai_rc;
    }

    memset(&set_attr, 0, sizeof(set_attr));
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = egr_profile_id;

    if (queue_index == QUEUE_ALL)
    {
        for (loop_idx = 0; loop_idx < num_queue; loop_idx++)
        {
            printf("Setting egress profile for queue no: %d \r\n", loop_idx);
            queue_obj = queue_id[loop_idx];
            sai_rc = sai_queue_api_table->set_queue_attribute
                       (queue_obj, (const sai_attribute_t *)&set_attr);
            if (sai_rc != SAI_STATUS_SUCCESS) {
                printf("Setting Buffer profile failed for queue no %d \r\n", loop_idx);
                break;
            }
        }
    } else {
        if (queue_index > num_queue)
            return SAI_STATUS_FAILURE;
        printf("Setting egress profile for queue no: %d \r\n", queue_index);
        queue_obj = queue_id[queue_index];
        sai_rc = sai_queue_api_table->set_queue_attribute
                       (queue_obj, (const sai_attribute_t *)&set_attr);
        if (sai_rc != SAI_STATUS_SUCCESS)
            printf("Setting Buffer profile failed for queue no %d \r\n", queue_index);
    }
    return sai_rc;
}

TEST_F(qos_buffer, qos_ing_egr_admn_ctrl_static_test)
{
    sai_attribute_t set_attr;
    sai_object_id_t map_id[3] = { 0 };
    sai_attr_id_t map_name[3] = {
                     SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP,
                     SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP,
                     SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP
                   };
    sai_object_id_t ing_pool_id = 0;
    sai_object_id_t egr_pool_id = 0;
    sai_object_id_t ing_profile_id = 0;
    sai_object_id_t egr_profile_id = 0;
    int             buffer_profile_size = 102400; // 100KB

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_dot1p_to_tc_map
              (sai_qos_map_api_table, &map_id[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_tc_to_pg_map
              (sai_qos_map_api_table, false, true, &map_id[1]));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_tc_to_queue_map
              (sai_qos_map_api_table, &map_id[2]));

    for (int iLoop =0; iLoop < 3; iLoop++)
    {
        memset(&set_attr, 0, sizeof(set_attr));
        set_attr.id = map_name[iLoop];
        set_attr.value.oid = map_id[iLoop];

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
                  (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
                  (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));
    }

    printf("Buffer size of ingress SP is %u and egress SP is %u bytes\n", sai_total_buffer_size,
                                          sai_total_buffer_size);

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
                  &ing_pool_id, sai_total_buffer_size,
                  SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
                  &egr_pool_id, sai_total_buffer_size,
                  SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &ing_profile_id,
              0x17 , ing_pool_id, buffer_profile_size, SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC,
              0, 0, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &egr_profile_id,
                                  0x17 ,egr_pool_id, buffer_profile_size,
                                  SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC, 0,
                                  sai_buffer_profile_test_size_3, 0, 0));

    printf("Setting Buffer profile for port no %u \n", test_port_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id, ing_profile_id, PG_ALL));

    printf("Setting Buffer profile for port no %u \n", test_port_id_1);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id_1, ing_profile_id, PG_ALL));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_set_egress_profile_for_queue (sai_port_api_table, sai_queue_api_table,
                                                test_port_id_2, egr_profile_id, QUEUE_ALL));

    sai_break_test();

    {
        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_get_ingress_pg_stats(sai_port_api_table,
                      sai_buffer_api_table, test_port_id));

        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_get_ingress_pg_stats(sai_port_api_table,
                      sai_buffer_api_table, test_port_id_1));

        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_get_egress_queue_stats(sai_port_api_table,
                           sai_queue_api_table, test_port_id_2));
    }
    sai_break_test();

    printf("Resetting Buffer profile for port no %u \n", test_port_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id, SAI_NULL_OBJECT_ID, PG_ALL));

    printf("Resetting Buffer profile for port no %u \n", test_port_id_1);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id_1, SAI_NULL_OBJECT_ID, PG_ALL));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_set_egress_profile_for_queue (sai_port_api_table, sai_queue_api_table,
                                                test_port_id_2, SAI_NULL_OBJECT_ID, QUEUE_ALL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_profile(ing_profile_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (ing_pool_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_profile(egr_profile_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (egr_pool_id));

    for (int iLoop = 0; ((iLoop < 3) && (map_id[iLoop] != 0)); iLoop++)
    {
        printf("Remove QOS MAP no %d and id %lu \r\n", iLoop, map_id[iLoop]);
        memset(&set_attr, 0, sizeof(set_attr));
        set_attr.id = map_name[iLoop];
        set_attr.value.oid = SAI_NULL_OBJECT_ID;

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id[iLoop]));
    }
}

TEST_F(qos_buffer, qos_ing_egr_admn_ctrl_dynamic_test)
{
    sai_attribute_t set_attr;
    sai_object_id_t map_id[3] = { 0};
    sai_attr_id_t map_name[3] = {
                     SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP,
                     SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP,
                     SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP
                   };
    sai_object_id_t ing_pool_id = 0;
    sai_object_id_t egr_pool_id = 0;
    sai_object_id_t ing_profile_id = 0;
    sai_object_id_t egr_profile_id = 0;
    int             buffer_profile_size = 102400; // 100KB

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_dot1p_to_tc_map
              (sai_qos_map_api_table, &map_id[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_tc_to_pg_map
              (sai_qos_map_api_table, false, true, &map_id[1]));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_tc_to_queue_map
              (sai_qos_map_api_table, &map_id[2]));

    for (int iLoop =0; iLoop < 3; iLoop++)
    {
        memset(&set_attr, 0, sizeof(set_attr));
        set_attr.id = map_name[iLoop];
        set_attr.value.oid = map_id[iLoop];

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
                  (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
                  (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));
    }

    printf("Buffer size of ingress SP is %u and egress SP is %u bytes\n", sai_total_buffer_size,
                                         sai_total_buffer_size);

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
                  &ing_pool_id, sai_total_buffer_size,
                  SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
                  &egr_pool_id, sai_total_buffer_size,
                  SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &ing_profile_id,
              0x17 , ing_pool_id, buffer_profile_size, SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC,
              0, sai_buffer_profile_test_size_2, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &egr_profile_id,
              0xb , egr_pool_id, buffer_profile_size, 0, 1, 0, 0, 0));

    printf("Setting Buffer profile for port no %u \n", test_port_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id, ing_profile_id, PG_ALL));

    printf("Setting Buffer profile for port no %u \n", test_port_id_1);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id_1, ing_profile_id, PG_ALL));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_set_egress_profile_for_queue (sai_port_api_table, sai_queue_api_table,
                                                test_port_id_2, egr_profile_id, QUEUE_ALL));

    sai_break_test();

    {
        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_get_ingress_pg_stats(sai_port_api_table,
                      sai_buffer_api_table, test_port_id));

        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_get_ingress_pg_stats(sai_port_api_table,
                      sai_buffer_api_table, test_port_id_1));

        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_get_egress_queue_stats(sai_port_api_table,
                           sai_queue_api_table, test_port_id_2));
    }

    sai_break_test();

    printf("Resetting Buffer profile for port no %u \n", test_port_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id, SAI_NULL_OBJECT_ID, PG_ALL));

    printf("Resetting Buffer profile for port no %u \n", test_port_id_1);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id_1, SAI_NULL_OBJECT_ID, PG_ALL));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_set_egress_profile_for_queue (sai_port_api_table, sai_queue_api_table,
                                                test_port_id_2, SAI_NULL_OBJECT_ID, QUEUE_ALL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_profile(ing_profile_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (ing_pool_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_profile(egr_profile_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (egr_pool_id));

    for (int iLoop = 0; ((iLoop < 3) && (map_id[iLoop] != 0)); iLoop++)
    {
        printf("Remove QOS MAP no %d and id %lu \r\n", iLoop, map_id[iLoop]);
        memset(&set_attr, 0, sizeof(set_attr));
        set_attr.id = map_name[iLoop];
        set_attr.value.oid = SAI_NULL_OBJECT_ID;

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id[iLoop]));
    }
}

TEST_F(qos_buffer, qos_ing_egr_admn_ctrl_pause_test)
{
    sai_attribute_t set_attr;
    sai_object_id_t map_id[3] = { 0};
    sai_attr_id_t map_name[3] = {
                     SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP,
                     SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP,
                     SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP
                   };
    sai_object_id_t ing_pool_id = 0;
    sai_object_id_t egr_pool_id = 0;
    sai_object_id_t ing_profile_id = 0;
    sai_object_id_t egr_profile_id = 0;
    int             buffer_profile_size = 102400; // 100KB
    int             xoff_profile_size = 102400; // 100KB
    int             xon_profile_size = 10240; // 10KB

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_dot1p_to_tc_map
              (sai_qos_map_api_table, &map_id[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_tc_to_pg_map
              (sai_qos_map_api_table, true, true, &map_id[1]));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_tc_to_queue_map
              (sai_qos_map_api_table, &map_id[2]));

    for (int iLoop =0; iLoop < 3; iLoop++)
    {
        memset(&set_attr, 0, sizeof(set_attr));
        set_attr.id = map_name[iLoop];
        set_attr.value.oid = map_id[iLoop];

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
                  (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
                  (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));
    }

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_enable_llfc(sai_port_api_table,
                test_port_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_enable_llfc(sai_port_api_table,
                test_port_id_1));

    printf("Buffer size of ingress SP is %u and egress SP is %u cells\n", sai_total_buffer_size,
           sai_total_buffer_size);

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
                  &ing_pool_id, sai_total_buffer_size,
                  SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
                  &egr_pool_id, sai_total_buffer_size,
                  SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &ing_profile_id,
              0x73 , ing_pool_id, buffer_profile_size, 0, 0, sai_buffer_profile_test_size_2,
              xoff_profile_size, xon_profile_size));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &egr_profile_id,
                                  0x17 ,egr_pool_id, buffer_profile_size,
                                  SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC, 0,
                                  sai_buffer_profile_test_size_3, 0, 0));

    printf("Setting Buffer profile for port no %u \n", test_port_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id, ing_profile_id, PG_ALL));

    printf("Setting Buffer profile for port no %u \n", test_port_id_1);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id_1, ing_profile_id, PG_ALL));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_set_egress_profile_for_queue (sai_port_api_table, sai_queue_api_table,
                                                test_port_id_2, egr_profile_id, QUEUE_ALL));

    sai_break_test();

    {
        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_get_ingress_pg_stats(sai_port_api_table,
                      sai_buffer_api_table, test_port_id));

        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_get_ingress_pg_stats(sai_port_api_table,
                      sai_buffer_api_table, test_port_id_1));

        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_get_egress_queue_stats(sai_port_api_table,
                           sai_queue_api_table, test_port_id_2));
    }

    sai_break_test();

    printf("Resetting Buffer profile for port no %u \n", test_port_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id, SAI_NULL_OBJECT_ID, PG_ALL));

    printf("Resetting Buffer profile for port no %u \n", test_port_id_1);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id_1, SAI_NULL_OBJECT_ID, PG_ALL));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_set_egress_profile_for_queue (sai_port_api_table, sai_queue_api_table,
                                                test_port_id_2, SAI_NULL_OBJECT_ID, QUEUE_ALL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_profile(ing_profile_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (ing_pool_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_profile(egr_profile_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (egr_pool_id));

    for (int iLoop = 0; ((iLoop < 3) && (map_id[iLoop] != 0)); iLoop++)
    {
        printf("Remove QOS MAP no %d and id %lu \r\n", iLoop, map_id[iLoop]);
        memset(&set_attr, 0, sizeof(set_attr));
        set_attr.id = map_name[iLoop];
        set_attr.value.oid = SAI_NULL_OBJECT_ID;

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id[iLoop]));
    }
}

TEST_F(qos_buffer, qos_ing_egr_admn_ctrl_pfc_test)
{
    sai_attribute_t get_attr;
    sai_attribute_t set_attr;
    sai_object_id_t map_id[4] = { 0};
    sai_attr_id_t map_name[4] = {
                     SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP,
                     SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP,
                     SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP,
                     SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP
                   };
    sai_object_id_t ing_pool_id = 0;
    sai_object_id_t egr_pool_id = 0;
    sai_object_id_t ing_pfc_pool_id = 0;
    sai_object_id_t egr_pfc_pool_id = 0;
    sai_object_id_t ing_profile_id = 0;
    sai_object_id_t egr_profile_id = 0;
    sai_object_id_t ing_pfc_profile_id = 0;
    sai_object_id_t egr_pfc_profile_id = 0;
    sai_object_id_t queue_id[50] = {0};
    unsigned int num_queue = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    int             buffer_profile_size = 102400; // 100KB
    int             xoff_profile_size = 102400; // 100KB
    int             xon_profile_size = 10240; // 10KB

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_dot1p_to_tc_map
              (sai_qos_map_api_table, &map_id[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_tc_to_pg_map
              (sai_qos_map_api_table, false, false, &map_id[1]));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_pfc_prio_to_queue_map
              (sai_qos_map_api_table, &map_id[2]));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_tc_to_queue_map
              (sai_qos_map_api_table, &map_id[3]));

    for (int iLoop =0; iLoop < 4; iLoop++)
    {
        memset(&set_attr, 0, sizeof(set_attr));
        set_attr.id = map_name[iLoop];
        set_attr.value.oid = map_id[iLoop];

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
                  (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
                  (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
                  (sai_qos_port_id_get(test_port_id_2), (const sai_attribute_t *)&set_attr));
    }

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_enable_pfc(sai_port_api_table,
                test_port_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_enable_pfc(sai_port_api_table,
                test_port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_enable_pfc(sai_port_api_table,
                test_port_id_2));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_set_port_max_mtu(sai_switch_api_table,
                sai_port_api_table, test_port_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_set_port_max_mtu(sai_switch_api_table,
                sai_port_api_table, test_port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_set_port_max_mtu(sai_switch_api_table,
                sai_port_api_table, test_port_id_2));

    printf("Buffer size of ingress SP is %u and egress SP is %u cells\n",
           sai_total_buffer_size, sai_total_buffer_size);

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
                  &ing_pool_id, sai_total_buffer_size/2,
                  SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
                  &egr_pool_id, sai_total_buffer_size/2,
                  SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
                  &ing_pfc_pool_id, sai_total_buffer_size/2,
                  SAI_BUFFER_POOL_TYPE_INGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table,
                  &egr_pfc_pool_id, sai_total_buffer_size/2,
                  SAI_BUFFER_POOL_TYPE_EGRESS, SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &ing_profile_id,
              0x17 , ing_pool_id, 9216, SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC,
              0, 0,
              0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &egr_profile_id,
              0xF , egr_pool_id, 8096, SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC, 0, 0, 0, 0));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &ing_pfc_profile_id,
              0x73 , ing_pfc_pool_id, buffer_profile_size, 0,
              0, sai_buffer_profile_test_size_2, xoff_profile_size, xon_profile_size));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &egr_pfc_profile_id,
                                  0x17 ,egr_pfc_pool_id, buffer_profile_size,
                                  SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC, 0,
                                  sai_buffer_profile_test_size_3, 0, 0));

    printf("Setting PG Buffer profile for port no %u \r\n", test_port_id);

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id, ing_profile_id, PG_ALL));

    printf("Setting PG Buffer profile for port no %u \r\n", test_port_id_1);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id_1, ing_profile_id, PG_ALL));

    printf("Setting PG Buffer profile for port no %u \r\n", test_port_id_2);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id_2, ing_profile_id, PG_ALL));


    printf("Setting Queue Buffer profile for port no %u \r\n", test_port_id_2);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_set_egress_profile_for_queue (sai_port_api_table, sai_queue_api_table,
                                                test_port_id_2, egr_profile_id, QUEUE_ALL));

    printf("Setting PFC PG Buffer profile for port no %u \r\n", test_port_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id, ing_pfc_profile_id, 4));

    printf("Setting PFC PG Buffer profile for port no %u \r\n", test_port_id_1);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id_1, ing_pfc_profile_id, 4));

    sai_break_test();

    printf("Setting PFC Queue Buffer profile for port no %u \r\n", test_port_id_2);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_set_egress_profile_for_queue (sai_port_api_table, sai_queue_api_table,
                                                test_port_id_2, egr_pfc_profile_id, 6));


    {
        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_get_ingress_pg_stats(sai_port_api_table,
                      sai_buffer_api_table, test_port_id));

        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_get_ingress_pg_stats(sai_port_api_table,
                      sai_buffer_api_table, test_port_id_1));

        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_get_egress_queue_stats(sai_port_api_table,
                           sai_queue_api_table, test_port_id_2));
    }
    sai_break_test();

    printf("Resetting Buffer profile for port no %u \n", test_port_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id, SAI_NULL_OBJECT_ID, PG_ALL));

    printf("Resetting Buffer profile for port no %u \n", test_port_id_1);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id_1, SAI_NULL_OBJECT_ID, PG_ALL));

    printf("Resetting Buffer profile for port no %u \n", test_port_id_2);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_set_ingress_profile_id_for_pg(sai_port_api_table,
                                  sai_buffer_api_table, test_port_id_2, SAI_NULL_OBJECT_ID, PG_ALL));

    for (int iLoop = 0; ((iLoop < 4) && (map_id[iLoop] != 0)); iLoop++)
    {
        printf("Remove QOS MAP no %d and id %lu \r\n", iLoop, map_id[iLoop]);
        memset(&set_attr, 0, sizeof(set_attr));
        set_attr.id = map_name[iLoop];
        set_attr.value.oid = SAI_NULL_OBJECT_ID;

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_1), (const sai_attribute_t *)&set_attr));

        ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id_2), (const sai_attribute_t *)&set_attr));

        ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_map_api_table->remove_qos_map(map_id[iLoop]));
    }

    get_attr.id = SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES;
    sai_rc = sai_port_api_table->get_port_attribute (sai_qos_port_id_get(test_port_id_2), 1, &get_attr);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_rc);

    num_queue = get_attr.value.u32;

    get_attr.id = SAI_PORT_ATTR_QOS_QUEUE_LIST;
    get_attr.value.objlist.count = num_queue;
    get_attr.value.objlist.list = queue_id;
    sai_rc = sai_port_api_table->get_port_attribute (sai_qos_port_id_get(test_port_id_2), 1, &get_attr);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_rc);

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_test_queue_remove(queue_id[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_set_egress_profile_for_queue (sai_port_api_table, sai_queue_api_table,
                                                test_port_id_2, SAI_NULL_OBJECT_ID, QUEUE_ALL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_profile(ing_profile_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_profile(ing_pfc_profile_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (ing_pool_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (ing_pfc_pool_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_profile(egr_profile_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_profile(egr_pfc_profile_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (egr_pool_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_buffer_api_table->remove_buffer_pool (egr_pfc_pool_id));

}
