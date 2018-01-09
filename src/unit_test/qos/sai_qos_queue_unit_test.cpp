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
* @file  sai_qos_queue_unit_test.cpp
*
* @brief This file contains tests for qos queue APIs
*
*************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "stdarg.h"
#include "gtest/gtest.h"

extern "C" {
#include "sai_qos_unit_test_utils.h"
#include "sai.h"
#include "saistatus.h"
#include <inttypes.h>
#include <string.h>
}

static const unsigned int default_port = 0;
static sai_object_id_t  default_port_id = 0;
static sai_object_id_t  cpu_port_id = 0;
static sai_object_id_t switch_id = 0;

/* SAI initialization */
void SetUpTestCase (void)
{

    /*
     * Query and populate the SAI Switch API Table.
     */
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_api_query
            (SAI_API_SWITCH, (static_cast<void**>
                              (static_cast<void*>(&p_sai_switch_api_table)))));

    ASSERT_TRUE (p_sai_switch_api_table != NULL);

    ASSERT_TRUE (p_sai_switch_api_table->remove_switch != NULL);
    ASSERT_TRUE (p_sai_switch_api_table->set_switch_attribute != NULL);
    ASSERT_TRUE (p_sai_switch_api_table->get_switch_attribute != NULL);

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

    ASSERT_TRUE(p_sai_switch_api_table->create_switch != NULL);

    EXPECT_EQ (SAI_STATUS_SUCCESS,
            (p_sai_switch_api_table->create_switch (&switch_id , attr_count,
                                                    sai_attr_set)));

    printf("Switch Init success \r\n");

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_api_query(SAI_API_QUEUE,
                (static_cast<void**>(static_cast<void*>(&p_sai_qos_queue_api_table)))));

    ASSERT_TRUE (p_sai_qos_queue_api_table != NULL);

    ASSERT_TRUE (p_sai_qos_queue_api_table->create_queue != NULL);
    ASSERT_TRUE (p_sai_qos_queue_api_table->remove_queue != NULL);
    ASSERT_TRUE (p_sai_qos_queue_api_table->set_queue_attribute != NULL);
    ASSERT_TRUE (p_sai_qos_queue_api_table->get_queue_attribute != NULL);
    ASSERT_TRUE (p_sai_qos_queue_api_table->get_queue_stats != NULL);

    ASSERT_EQ (NULL, sai_api_query(SAI_API_PORT,
                (static_cast<void**>(static_cast<void*>(&p_sai_port_api_table)))));

    ASSERT_TRUE (p_sai_port_api_table != NULL);

    ASSERT_TRUE (p_sai_port_api_table->set_port_attribute != NULL);
    ASSERT_TRUE (p_sai_port_api_table->get_port_attribute != NULL);
    ASSERT_TRUE (p_sai_port_api_table->get_port_stats != NULL);

    sai_attribute_t sai_port_attr;
    uint32_t * port_count = sai_qos_update_port_count();
    sai_status_t ret = SAI_STATUS_SUCCESS;

    memset (&sai_port_attr, 0, sizeof (sai_port_attr));

    sai_port_attr.id = SAI_SWITCH_ATTR_PORT_LIST;
    sai_port_attr.value.objlist.count = 256;
    sai_port_attr.value.objlist.list  = sai_qos_update_port_list();

    ret = p_sai_switch_api_table->get_switch_attribute(0,1,&sai_port_attr);
    *port_count = sai_port_attr.value.objlist.count;

    ASSERT_EQ(SAI_STATUS_SUCCESS,ret);
}

/*
 * Validate get port queue id list get.
 */
TEST (saiQosQueueTest, port_queue_id_list_get)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    unsigned int max_ports = 0;
    unsigned int max_queues = 0;
    unsigned int port_index = 0;
    sai_object_id_t  port_id = SAI_NULL_OBJECT_ID;

    max_ports = sai_qos_max_ports_get();

    for (port_index = 0; port_index < max_ports; port_index++)
    {
        port_id = sai_qos_port_id_get (port_index);

        sai_rc = sai_test_port_max_number_queues_get (port_id,
                                                      &max_queues);
        ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

        sai_object_id_t queue_id_list[max_queues];

        sai_rc = sai_test_port_queue_id_list_get (port_id, max_queues,
                                                  &queue_id_list[0]);
        ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }
}

/*
 * Validate get cpu port queue id list get.
 */
TEST (saiQosQueueTest, cpu_port_queue_id_list_get)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    unsigned int max_queues = 0;

    sai_rc = sai_test_port_max_number_queues_get (cpu_port_id,
                                                  &max_queues);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_object_id_t queue_id_list[max_queues];

    sai_rc = sai_test_port_queue_id_list_get (cpu_port_id, max_queues,
                                              &queue_id_list[0]);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}


/*
 * Validate queue set attributes.
 */
TEST (saiQosQueueTest, queue_attribute_set)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t  attr;
    unsigned int     max_queues = 0;

    sai_rc = sai_test_port_max_number_queues_get (default_port_id,
                                                  &max_queues);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_object_id_t queue_id_list[max_queues];

    sai_rc = sai_test_port_queue_id_list_get (default_port_id, max_queues,
                                              &queue_id_list[0]);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_object_id_t  queue_id = queue_id_list[0];
    attr.id = SAI_QUEUE_ATTR_TYPE;
    attr.value.s32 = SAI_QUEUE_TYPE_MULTICAST;

    sai_rc = p_sai_qos_queue_api_table->set_queue_attribute (queue_id,
                                                             &attr);
    EXPECT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_rc);
}

/*
 * Validate Queue get attributes.
 */
TEST (saiQosQueueTest, queue_attribute_get)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t  attr;
    unsigned int     max_queues = 0;

    sai_rc = sai_test_port_max_number_queues_get (default_port_id,
                                                  &max_queues);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_object_id_t queue_id_list[max_queues];

    sai_rc = sai_test_port_queue_id_list_get (default_port_id, max_queues,
                                              &queue_id_list[0]);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_object_id_t  queue_id = queue_id_list[0];

    attr.id = SAI_QUEUE_ATTR_TYPE;
    sai_rc = p_sai_qos_queue_api_table->get_queue_attribute (queue_id, 1,
                                                             &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (SAI_QUEUE_TYPE_UNICAST, attr.value.s32);

}
/*
 * Validate queue statistics
 */
TEST (saiQosQueueTest, queue_stats_get)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    unsigned int     max_queues = 0;
    int              counter_id = 0;
    uint64_t counter_val;

    sai_rc = sai_test_port_max_number_queues_get (default_port_id,
                                                  &max_queues);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_object_id_t queue_id_list[max_queues];

    sai_rc = sai_test_port_queue_id_list_get (default_port_id, max_queues,
                                              &queue_id_list[0]);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_object_id_t  queue_id = queue_id_list[0];

    for (counter_id = SAI_QUEUE_STAT_PACKETS;
         counter_id <= SAI_QUEUE_STAT_SHARED_WATERMARK_BYTES; counter_id++) {

        sai_rc = p_sai_qos_queue_api_table->get_queue_stats(queue_id,
                                                           (const sai_queue_stat_t*)&counter_id,
                                                           1, &counter_val);
        if(sai_rc == SAI_STATUS_SUCCESS) {
             printf("Counter ID %d is supported. Val:0x%" PRIx64 "\r\n",counter_id,counter_val);
        } else if( sai_rc == SAI_STATUS_NOT_SUPPORTED) {
             printf("Counter ID %d is not supported.\r\n",counter_id);
        } else {
             printf("Counter ID %d get returned error %d\r\n",counter_id, sai_rc);
        }
    }

}

TEST (saiQosQueueTest, clear_queue_stats)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    unsigned int     max_queues = 0;
    int              counter_id = 0;
    uint64_t counter_val;

    sai_rc = sai_test_port_max_number_queues_get (default_port_id,
                                                  &max_queues);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_object_id_t queue_id_list[max_queues];

    sai_rc = sai_test_port_queue_id_list_get (default_port_id, max_queues,
                                              &queue_id_list[0]);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_object_id_t  queue_id = queue_id_list[0];
    for (counter_id = SAI_QUEUE_STAT_PACKETS;
         counter_id <= SAI_QUEUE_STAT_SHARED_WATERMARK_BYTES; counter_id++) {

        sai_rc = p_sai_qos_queue_api_table->clear_queue_stats(queue_id,
                                                            (const sai_queue_stat_t*)&counter_id, 1);
        if(sai_rc == SAI_STATUS_SUCCESS) {
            sai_rc = p_sai_qos_queue_api_table->get_queue_stats (queue_id,
                                                                (const sai_queue_stat_t*)&counter_id,
                                                                 1, &counter_val);
            if(sai_rc != SAI_STATUS_SUCCESS) {
                printf("Counter ID %d can be cleared but not get. Error %d\r\n",
                       counter_id, sai_rc);
            } else {
                if(counter_val == 0) {
                    printf("Counter ID %d cleared.\r\n", counter_id);
                }
            }
        } else if (sai_rc == SAI_STATUS_NOT_SUPPORTED) {
             printf("Counter ID %d is not supported.\r\n",counter_id);
        } else if (sai_rc == SAI_STATUS_INVALID_PARAMETER) {
             printf ("Counter ID %d cannot be cleared.\r\n",counter_id);
        }else {
             printf("Counter ID %d get returned error %d\r\n",counter_id, sai_rc);
        }
    }
}

int main (int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    SetUpTestCase ();
    default_port_id = sai_qos_port_id_get (default_port);
    sai_test_cpu_port_id_get(&cpu_port_id);

    return RUN_ALL_TESTS();
}
