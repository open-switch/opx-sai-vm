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
* @file  sai_qos_scheduler_unit_test.cpp
*
* @brief This file contains tests for qos scheduler APIs
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
}

#define SAI_MAX_QUEUES_PER_PORT  40
static const unsigned int default_port = 0;
static sai_object_id_t  default_port_id  = 0;
static sai_object_id_t  cpu_port_id = 0;
static sai_object_id_t  queue_id_list[SAI_MAX_QUEUES_PER_PORT];
unsigned int            max_queues = 0;
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

    ASSERT_TRUE (p_sai_qos_queue_api_table->set_queue_attribute != NULL);
    ASSERT_TRUE (p_sai_qos_queue_api_table->get_queue_attribute != NULL);
    ASSERT_TRUE (p_sai_qos_queue_api_table->get_queue_stats != NULL);

    ASSERT_EQ (NULL, sai_api_query(SAI_API_PORT,
                (static_cast<void**>(static_cast<void*>(&p_sai_port_api_table)))));

    ASSERT_TRUE (p_sai_port_api_table != NULL);

    ASSERT_TRUE (p_sai_port_api_table->set_port_attribute != NULL);
    ASSERT_TRUE (p_sai_port_api_table->get_port_attribute != NULL);
    ASSERT_TRUE (p_sai_port_api_table->get_port_stats != NULL);

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_api_query(SAI_API_SCHEDULER,
                (static_cast<void**>(static_cast<void*>(&p_sai_scheduler_api_table)))));

    ASSERT_TRUE (p_sai_scheduler_api_table != NULL);

    ASSERT_TRUE (p_sai_scheduler_api_table->create_scheduler!= NULL);
    ASSERT_TRUE (p_sai_scheduler_api_table->remove_scheduler!= NULL);
    ASSERT_TRUE (p_sai_scheduler_api_table->set_scheduler_attribute != NULL);
    ASSERT_TRUE (p_sai_scheduler_api_table->get_scheduler_attribute != NULL);

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

static void sai_test_scheduler_attr_list_print (unsigned int attr_count,
                                                  sai_attribute_t *p_attr_list)
{
    unsigned int     attr_index;
    sai_attribute_t *p_attr = NULL;

    printf ("Printing SAI Scheduler attribute list..\n");
    for (attr_index = 0, p_attr = p_attr_list;
         attr_index < attr_count; ++attr_index, ++p_attr) {

        switch (p_attr->id) {

            case SAI_SCHEDULER_ATTR_SCHEDULING_TYPE:
                printf ("Index: %d, Scheduler algorithem %d.\n",
                        attr_index, p_attr->value.s32);
                break;

            case SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT:
                printf ("Index: %d, Scheduler weight %d.\n",
                        attr_index, p_attr->value.u8);
                break;

            case SAI_SCHEDULER_ATTR_METER_TYPE:
                printf ("Index: %d, Scheduler shape type %d.\n",
                        attr_index, p_attr->value.u8);
                break;

            case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE:
                printf ("Index: %d, Scheduler min bw rate %" PRIu64 ".\n",
                        attr_index, p_attr->value.u64);
                break;

            case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE:
                printf ("Index: %d, Scheduler min bw burst %" PRIu64 ".\n",
                        attr_index, p_attr->value.u64);
                break;

            case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE:
                printf ("Index: %d, Scheduler max bw rate %" PRIu64 ".\n",
                        attr_index, p_attr->value.u64);
                break;

            case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE:
                printf ("Index: %d, Scheduler max bw burst %" PRIu64 ".\n",
                        attr_index, p_attr->value.u64);
                break;

            default:
                printf ("Index: %d, Scheduler unknown Attr Id: %d, Value: %ld.\n",
                        attr_index, p_attr->id, p_attr->value.u64);
                break;
        }
    }
}

static sai_status_t sai_test_scheduler_attr_get (sai_object_id_t sched_id,
                                            sai_attribute_t *p_attr_list,
                                            unsigned int attr_count, ...)
{
    sai_status_t         sai_rc;
    va_list              varg_list;
    unsigned int         index;

    va_start (varg_list, attr_count);

    for (index = 0; index < attr_count; index++) {

        p_attr_list [index].id = va_arg (varg_list, unsigned int);
    }

    va_end (varg_list);

    sai_rc = p_sai_scheduler_api_table->get_scheduler_attribute (sched_id,
                                                                 attr_count,
                                                                 p_attr_list);

    if (sai_rc != SAI_STATUS_SUCCESS) {

        printf ("SAI Scheduler Get attribute failed with error: %d.\n", sai_rc);

    } else {

        printf ("SAI Scheduler Get attribute success.\n");

        sai_test_scheduler_attr_list_print (attr_count, p_attr_list);
    }

    return sai_rc;
}

static void sai_scheduler_verify_after_removal (sai_object_id_t sched_id)
{
    sai_status_t  sai_rc;

    sai_rc =  sai_test_scheduler_remove (sched_id);

    ASSERT_EQ (SAI_STATUS_INVALID_OBJECT_ID, sai_rc);
}

/*
 * Validate scheduler create, remove and duplicate delete.
 */
TEST (saiQosSchedulerTest, scheduler_create_and_remove)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t sched_id = SAI_NULL_OBJECT_ID;

    sai_rc = sai_test_scheduler_create (&sched_id, 0);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_scheduler_verify_after_removal (sched_id);

    sai_rc = sai_test_scheduler_create (&sched_id, 2,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_DWRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_scheduler_create (&sched_id, 3,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_scheduler_create (&sched_id, 3,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_scheduler_verify_after_removal (sched_id);
}

/*
 * Validate scheduler get attributes.
 */
TEST (saiQosSchedulerTest, schededuler_attribute_get)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  sched_id = SAI_NULL_OBJECT_ID;
    unsigned int     attr_count = 7;
    sai_attribute_t  attr_list[attr_count];

    sai_rc = sai_test_scheduler_create (&sched_id, 0);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_scheduler_attr_get (sched_id, &attr_list[0], attr_count,
                                            SAI_SCHEDULER_ATTR_SCHEDULING_TYPE,
                                            SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT,
                                            SAI_SCHEDULER_ATTR_METER_TYPE,
                                            SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE,
                                            SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE,
                                            SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE,
                                            SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE);

    ASSERT_EQ (SAI_SCHEDULING_TYPE_WRR, attr_list[0].value.s32);
    ASSERT_EQ (1, attr_list[1].value.u8);
    ASSERT_EQ (SAI_METER_TYPE_BYTES, attr_list[2].value.s32);
    ASSERT_EQ (0, attr_list[3].value.u64);
    ASSERT_EQ (0, attr_list[4].value.u64);
    ASSERT_EQ (0, attr_list[5].value.u64);
    ASSERT_EQ (0, attr_list[6].value.u64);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_scheduler_create (&sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_DWRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_scheduler_attr_get (sched_id, &attr_list[0], attr_count,
                                            SAI_SCHEDULER_ATTR_SCHEDULING_TYPE,
                                            SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT,
                                            SAI_SCHEDULER_ATTR_METER_TYPE,
                                            SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE,
                                            SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE,
                                            SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE,
                                            SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE);


    ASSERT_EQ (SAI_SCHEDULING_TYPE_DWRR, attr_list[0].value.s32);
    ASSERT_EQ (10, attr_list[1].value.u8);
    ASSERT_EQ (SAI_METER_TYPE_BYTES, attr_list[2].value.s32);
    ASSERT_EQ (2048, attr_list[3].value.u64);
    ASSERT_EQ (1024, attr_list[4].value.u64);
    ASSERT_EQ (20480, attr_list[5].value.u64);
    ASSERT_EQ (10240, attr_list[6].value.u64);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate scheduler set attributes.
 */
TEST (saiQosSchedulerTest, scheduler_attribute_set)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t  attr;
    unsigned int     attr_count = 7;
    sai_attribute_t  attr_list[attr_count];
    sai_object_id_t  sched_id = SAI_NULL_OBJECT_ID;

    sai_rc = sai_test_scheduler_create (&sched_id, 0);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_SCHEDULING_TYPE;
    attr.value.s32 = SAI_SCHEDULING_TYPE_DWRR;

    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT;
    attr.value.u8 = 10;

    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_METER_TYPE;
    attr.value.s32 = SAI_METER_TYPE_BYTES;

    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE;
    attr.value.u64 = 2048;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE;
    attr.value.u64 = 1024;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE;
    attr.value.u64 = 20480;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE;
    attr.value.u64 = 10240;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_scheduler_attr_get (sched_id, &attr_list[0], attr_count,
                                            SAI_SCHEDULER_ATTR_SCHEDULING_TYPE,
                                            SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT,
                                            SAI_SCHEDULER_ATTR_METER_TYPE,
                                            SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE,
                                            SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE,
                                            SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE,
                                            SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE);

    ASSERT_EQ (SAI_SCHEDULING_TYPE_DWRR, attr_list[0].value.s32);
    ASSERT_EQ (10, attr_list[1].value.u8);
    ASSERT_EQ (SAI_METER_TYPE_BYTES, attr_list[2].value.s32);
    ASSERT_EQ (2048, attr_list[3].value.u64);
    ASSERT_EQ (1024, attr_list[4].value.u64);
    ASSERT_EQ (20480, attr_list[5].value.u64);
    ASSERT_EQ (10240, attr_list[6].value.u64);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate scheduler apply on queue.
 */
TEST (saiQosSchedulerTest, scheduler_on_queue)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t  attr;
    sai_object_id_t  sched_id = SAI_NULL_OBJECT_ID;
    unsigned int     queue_index = 0;
    sai_object_id_t  queue_id = SAI_NULL_OBJECT_ID;

    sai_rc = sai_test_port_max_number_queues_get (default_port_id,
                                                  &max_queues);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Scheduler DWRR + SHAPE */
    sai_rc = sai_test_scheduler_create (&sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_DWRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 10000000,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 1000000);

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = sched_id;

    /* Apply Scheduler on all queues */
    for (queue_index = 0; queue_index < max_queues; queue_index++) {
        queue_id = queue_id_list[queue_index];
        sai_rc = p_sai_qos_queue_api_table->set_queue_attribute (queue_id, &attr);
        ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);

    /* Delete Scheduler from all queues */
    attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    for (queue_index = 0; queue_index < max_queues; queue_index++) {
        queue_id = queue_id_list[queue_index];
        sai_rc = p_sai_qos_queue_api_table->set_queue_attribute (queue_id, &attr);
        ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate Strict priority + Shaping scheduler on queue.
 */
TEST (saiQosSchedulerTest, scheduler_strict_priority_on_queue)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t  attr;
    sai_object_id_t  sched_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t  queue_id = SAI_NULL_OBJECT_ID;

    sai_rc = sai_test_port_max_number_queues_get (default_port_id,
                                                  &max_queues);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Test Strict priority + shape scheduler on queue */
    sai_rc = sai_test_scheduler_create (&sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_STRICT,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    queue_id = queue_id_list[0];

    attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = sched_id;
    sai_rc = p_sai_qos_queue_api_table->set_queue_attribute (queue_id, &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* SP + NO MAX Rate & Burst */
    attr.id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE;
    attr.value.u64 = 0;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE;
    attr.value.u64 = 0;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* SP + NO SHAPE */
    attr.id = SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE;
    attr.value.u64 = 0;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE;
    attr.value.u64 = 0;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    sai_rc = p_sai_qos_queue_api_table->set_queue_attribute (queue_id, &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Scheduler replace on queue.
 */
TEST (saiQosSchedulerTest, scheduler_replace_on_queue)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t  attr;
    sai_object_id_t  queue_id = SAI_NULL_OBJECT_ID;

    sai_rc = sai_test_port_max_number_queues_get (default_port_id,
                                                  &max_queues);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Modify Scheduler on queue */
    sai_object_id_t  sched_id_1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t  sched_id_2 = SAI_NULL_OBJECT_ID;

    sai_rc = sai_test_scheduler_create (&sched_id_1, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_WRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_scheduler_create (&sched_id_2, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_DWRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 100,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 4098,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 409800,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 20480);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    queue_id = queue_id_list[0];
    attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;

    attr.value.oid = sched_id_1;
    sai_rc = p_sai_qos_queue_api_table->set_queue_attribute (queue_id, &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.value.oid = sched_id_2;
    sai_rc = p_sai_qos_queue_api_table->set_queue_attribute (queue_id, &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id_1);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id_2);
    ASSERT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);

    attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    sai_rc = p_sai_qos_queue_api_table->set_queue_attribute (queue_id, &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id_2);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate scheduler appply on port.
 */

TEST (saiQosSchedulerTest, scheduler_on_port)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  sched_id = SAI_NULL_OBJECT_ID;
    sai_attribute_t  attr;

    sai_rc = sai_test_scheduler_create (&sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_DWRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID;
    attr.value.oid = sched_id;

    sai_rc = p_sai_port_api_table->set_port_attribute (default_port_id,
                                                       &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);

    attr.id = SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    sai_rc = p_sai_port_api_table->set_port_attribute (default_port_id,
                                                       &attr);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate scheduler replace on port.
 */

TEST (saiQosSchedulerTest, scheduler_replace_on_port)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t  attr;

    sai_object_id_t  sched_id_1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t  sched_id_2 = SAI_NULL_OBJECT_ID;

    sai_rc = sai_test_scheduler_create (&sched_id_1, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_WRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_scheduler_create (&sched_id_2, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_DWRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 100,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 4098,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 409800,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 20480);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID;
    attr.value.oid = sched_id_1;
    sai_rc = p_sai_port_api_table->set_port_attribute (default_port_id,
                                                       &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID;
    attr.value.oid = sched_id_2;
    sai_rc = p_sai_port_api_table->set_port_attribute (default_port_id,
                                                       &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id_1);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id_2);
    ASSERT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);

    attr.id = SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;
    sai_rc = p_sai_port_api_table->set_port_attribute (default_port_id,
                                                       &attr);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id_2);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate Modifying scheduler parmas after applyinig on queue and port.
 */

TEST (saiQosSchedulerTest, scheduler_modify)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t  attr;
    unsigned int     queue_index = 0;
    sai_object_id_t  sched_id = SAI_NULL_OBJECT_ID;

    sai_rc = sai_test_port_max_number_queues_get (default_port_id,
                                                  &max_queues);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_scheduler_create (&sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_WRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = sched_id;

    sai_object_id_t queue_id = queue_id_list[queue_index];
    sai_rc = p_sai_qos_queue_api_table->set_queue_attribute (queue_id, &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID;
    attr.value.oid = sched_id;

    sai_rc = p_sai_port_api_table->set_port_attribute (default_port_id,
                                                       &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Modify */
    attr.id = SAI_SCHEDULER_ATTR_SCHEDULING_TYPE;
    attr.value.s32 = SAI_SCHEDULING_TYPE_STRICT;

    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT;
    attr.value.u8 = 50;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_METER_TYPE;
    attr.value.s32 = SAI_METER_TYPE_PACKETS;

    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Remove */
    attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    sai_rc = p_sai_qos_queue_api_table->set_queue_attribute (queue_id, &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;
    sai_rc = p_sai_port_api_table->set_port_attribute (default_port_id,
                                                       &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate scheduler appply on CPU port queues.
 */
TEST (saiQosSchedulerTest, scheduler_on_cpu_port_queue)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t  attr;
    unsigned int     max_queues = 0;
    sai_object_id_t  sched_id = SAI_NULL_OBJECT_ID;
    unsigned int     queue_index = 0;
    sai_object_id_t  queue_id = SAI_NULL_OBJECT_ID;

    sai_rc = sai_test_port_max_number_queues_get (cpu_port_id,
                                                  &max_queues);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_object_id_t queue_id_list[max_queues];

    sai_rc = sai_test_port_queue_id_list_get (cpu_port_id, max_queues,
                                              &queue_id_list[0]);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Scheduler DWRR + SHAPE */
    sai_rc = sai_test_scheduler_create (&sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_DWRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = sched_id;

    /* Apply Scheduler on all queues */
    for (queue_index = 0; queue_index < max_queues; queue_index++) {
        queue_id = queue_id_list[queue_index];
        sai_rc = p_sai_qos_queue_api_table->set_queue_attribute (queue_id, &attr);
        ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }

    /* Test Packet mode */
    attr.id = SAI_SCHEDULER_ATTR_METER_TYPE;
    attr.value.s32 = SAI_METER_TYPE_PACKETS;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE;
    attr.value.u64 = 4000;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE;
    attr.value.u64 = 200;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE;
    attr.value.u64 = 500;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE;
    attr.value.u64 = 200;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);

    attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    for (queue_index = 0; queue_index < max_queues; queue_index++) {
        queue_id = queue_id_list[queue_index];
        sai_rc = p_sai_qos_queue_api_table->set_queue_attribute (queue_id, &attr);
        ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}


/*
 * Validate scheduler apply on CPU port.
 */

TEST (saiQosSchedulerTest, scheduler_on_cpu_port)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  sched_id = SAI_NULL_OBJECT_ID;
    sai_attribute_t  attr;

    sai_rc = sai_test_scheduler_create (&sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_DWRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID;
    attr.value.oid = sched_id;

    sai_rc = p_sai_port_api_table->set_port_attribute (cpu_port_id, &attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    sai_rc = p_sai_port_api_table->set_port_attribute (cpu_port_id, &attr);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}


int main (int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    SetUpTestCase ();
    default_port_id = sai_qos_port_id_get (default_port);
    sai_test_cpu_port_id_get(&cpu_port_id);
    sai_test_port_queue_count_and_id_list_get(default_port_id, &max_queues,
                                              &queue_id_list[0]);

    return RUN_ALL_TESTS();
}

