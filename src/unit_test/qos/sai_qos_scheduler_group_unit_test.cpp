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

/**
* @file  sai_qos_scheduler_group_unit_test.cpp
*
* @brief This file contains tests for qos scheduler group APIs
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
#define SAI_MAX_GROUPS_PER_LEVEL 10
#define SAI_TEST_GROUPS_PER_LEVEL 3
#define SAI_MAX_HIERARCHY_LEVELS 10
#define SAI_MAX_QUEUES_PER_PORT  40
#define SAI_MAX_UC_QUEUES_PER_SG 10
#define SAI_MAX_MC_QUEUES_PER_SG 10

#define SAI_MAX_QUEUES_PER_TEST  2

bool   is_fixed_hqos;
static const unsigned int default_port = 0;
static sai_object_id_t    default_port_id  = 0;
static sai_object_id_t    sg_id_list[SAI_MAX_HIERARCHY_LEVELS][SAI_MAX_GROUPS_PER_LEVEL];
static sai_object_id_t    queue_id_list[SAI_MAX_QUEUES_PER_PORT];
static unsigned int max_queues = 0;
static unsigned int hqos_levels = 0;
static unsigned int leaf_level = 0;
static sai_object_id_t default_sched_id = SAI_NULL_OBJECT_ID;
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
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_api_query(SAI_API_SCHEDULER_GROUP,
                (static_cast<void**>(static_cast<void*>(&p_sai_qos_sg_api_table)))));

    ASSERT_TRUE (p_sai_qos_sg_api_table != NULL);

    ASSERT_TRUE (p_sai_qos_sg_api_table->create_scheduler_group != NULL);
    ASSERT_TRUE (p_sai_qos_sg_api_table->remove_scheduler_group != NULL);
    ASSERT_TRUE (p_sai_qos_sg_api_table->set_scheduler_group_attribute != NULL);
    ASSERT_TRUE (p_sai_qos_sg_api_table->get_scheduler_group_attribute != NULL);

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

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query(SAI_API_QUEUE,
                    (static_cast<void**>(static_cast<void*>(&p_sai_qos_queue_api_table)))));

    ASSERT_TRUE (p_sai_qos_queue_api_table != NULL);

    ASSERT_TRUE (p_sai_qos_queue_api_table->create_queue != NULL);
    ASSERT_TRUE (p_sai_qos_queue_api_table->remove_queue != NULL);
    ASSERT_TRUE (p_sai_qos_queue_api_table->set_queue_attribute != NULL);
    ASSERT_TRUE (p_sai_qos_queue_api_table->get_queue_attribute != NULL);
    ASSERT_TRUE (p_sai_qos_queue_api_table->get_queue_stats != NULL);

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

static sai_status_t sai_test_sched_group_create (sai_object_id_t *p_group_id,
                                          unsigned int attr_count, ...)
{
    sai_status_t       sai_rc;
    sai_attribute_t   *p_attr_list = NULL;
    sai_attribute_t   *p_attr = NULL;
    va_list            varg_list;
    unsigned int       index;

    p_attr_list = (sai_attribute_t *) calloc (attr_count,
                                              sizeof (sai_attribute_t));
    if (p_attr_list == NULL) {

        printf ("%s(): Memory alloc failed for attribute list.\n", __FUNCTION__);

        return SAI_STATUS_NO_MEMORY;
    }

    va_start (varg_list, attr_count);

    for (index = 0, p_attr = p_attr_list;
         index < attr_count; ++index, ++p_attr) {

        p_attr->id = va_arg (varg_list, unsigned int);

        switch (p_attr->id) {

            case SAI_SCHEDULER_GROUP_ATTR_PORT_ID:
                p_attr->value.oid = va_arg (varg_list, uint64_t);

                printf ("Attr Index: %d, Set Scheduler group port oid: 0x%" PRIx64 ".\n",
                        index, p_attr->value.oid);
                break;

            case SAI_SCHEDULER_GROUP_ATTR_LEVEL:
                p_attr->value.u32 = va_arg (varg_list, unsigned int);

                printf ("Attr Index: %d, Set Scheduler group level to value: %d.\n",
                        index, p_attr->value.s32);

                break;

            case SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID:
                p_attr->value.oid = va_arg (varg_list, uint64_t);

                printf ("Attr Index: %d, Set Scheduler group scheduler oid: 0x%" PRIx64 ".\n",
                        index, p_attr->value.oid);
                break;

            case SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE:
                p_attr->value.oid = va_arg (varg_list, uint64_t);

                printf ("Attr Index: %d, Set Scheduler group parent oid: 0x%" PRIx64 ".\n",
                        index, p_attr->value.oid);
                break;


            default:
                p_attr->value.u64 = va_arg (varg_list, unsigned int);
                printf ("Attr Index: %d, Set unknown Attr Id: %d to value: %ld.\n",
                        index, p_attr->id, p_attr->value.u64);
                break;
        }
    }

    va_end (varg_list);

    sai_rc = p_sai_qos_sg_api_table->create_scheduler_group (p_group_id, switch_id, attr_count,
                                                          p_attr_list);

    if (sai_rc != SAI_STATUS_SUCCESS) {

        printf ("SAI Scheduler Group Creation failed with error: %d.\n", sai_rc);

    } else {

        printf ("SAI Scheduler Group Creation success, Group Id: 0x%" PRIx64 ".\n",
                (*p_group_id));
    }

    free (p_attr_list);

    return sai_rc;
}

static sai_status_t sai_test_sched_group_remove (sai_object_id_t sg_id)
{
    sai_status_t sai_rc;

    sai_rc = p_sai_qos_sg_api_table->remove_scheduler_group (sg_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        printf ("SAI Scheduler Group Remove failed with error: %d for "\
                "sched group id 0x%" PRIx64 "\r\n", sai_rc, sg_id);
    } else {
        printf ("SAI Scheduler Group Remove success for Group Id: 0x%" PRIx64 ".\r\n",
                sg_id);
    }

    return sai_rc;
}

static void sai_test_sched_group_attr_list_print (unsigned int attr_count,
                                                  sai_attribute_t *p_attr_list)
{
    unsigned int     attr_index;
    sai_attribute_t *p_attr = NULL;
    unsigned int     idx;

    printf ("Printing SAI Scheduler Group attribute list..\n");
    for (attr_index = 0, p_attr = p_attr_list;
         attr_index < attr_count; ++attr_index, ++p_attr) {

        switch (p_attr->id) {
            case SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT:
                printf ("Index: %d, Active Group child Count: %d.\n",
                        attr_index, p_attr->value.u32);
                break;


            case SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST:
                printf ("Index: %d, Scheduler Group child List count: %d, "
                        "Child list Ids: \n", attr_index,
                        p_attr->value.objlist.count);

                for (idx = 0; idx < p_attr->value.objlist.count; idx++)
                {
                    printf ("0x%" PRIx64 " ", p_attr->value.objlist.list[idx]);
                }
                printf ("\n");
                break;

            case SAI_SCHEDULER_GROUP_ATTR_PORT_ID:
                printf ("Index: %d, Scheduler Group port: 0x%" PRIx64 ".\n", attr_index,
                        p_attr->value.oid);

                break;

            case SAI_SCHEDULER_GROUP_ATTR_LEVEL:
                printf ("Index: %d, Scheduler Group Hierarchy level: %d.\n",
                        attr_index,
                        p_attr->value.u32);
                break;

            case SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS:
                printf ("Index: %d, Scheduler Group max childs: %d.\n",
                        attr_index,
                        p_attr->value.u32);
                break;


            case SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID:
                printf ("Index: %d, Scheduler Group scheduler profile: 0x%" PRIx64 ".\n",
                        attr_index,
                        p_attr->value.oid);
                break;

            case SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE:
                printf ("Index: %d, Scheduler Group scheduler parent: 0x%" PRIx64 ".\n",
                        attr_index,
                        p_attr->value.oid);
                break;

            default:
                printf ("Index: %d, Scheduler group unknown Attr Id: %d, Value: %ld.\n",
                        attr_index, p_attr->id, p_attr->value.u64);
                break;
        }
    }
}

/* This API is not for SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST */
static sai_status_t sai_test_sched_group_attr_get (sai_object_id_t sg_id,
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

    sai_rc = p_sai_qos_sg_api_table->get_scheduler_group_attribute (sg_id,
                                                                    attr_count,
                                                                    p_attr_list);

    if (sai_rc != SAI_STATUS_SUCCESS) {

        printf ("SAI Scheduler Group Get attribute failed with error: %d.\n", sai_rc);

    } else {

        printf ("SAI Scheduler group Get attribute success.\n");

        sai_test_sched_group_attr_list_print (attr_count, p_attr_list);
    }

    return sai_rc;
}

static sai_status_t sai_test_sched_group_child_count_get (
                                                     sai_object_id_t  sg_id,
                                                     unsigned int *child_count)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t    attr  = {0};
    unsigned int       attr_count = 1;

    sai_rc = sai_test_sched_group_attr_get (sg_id, &attr, attr_count,
                                            SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        printf("Failed to get child count on sg :0x%" PRIx64 ". \n", sg_id);
        return SAI_STATUS_FAILURE;
    }
    *child_count = attr.value.u32;

    sai_test_sched_group_attr_list_print (attr_count, &attr);
    return sai_rc;
}

static sai_status_t sai_test_sched_group_child_id_list_get (
                                                     sai_object_id_t  sg_id,
                                                     unsigned int child_count,
                                                     sai_object_id_t *p_child_id_list)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t    attr  = {0};
    unsigned int       attr_count = 1;

    attr.value.objlist.count = child_count;
    attr.value.objlist.list = p_child_id_list;

    if (attr.value.objlist.list == NULL) {

        printf ("%s(): Memory alloc failed for attribute list.\n", __FUNCTION__);

        return SAI_STATUS_NO_MEMORY;
    }

    sai_rc = sai_test_sched_group_attr_get (sg_id, &attr, attr_count,
                                            SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        printf("Failed to get child list on sg :0x%" PRIx64 ". \n", sg_id);
        return SAI_STATUS_FAILURE;
    }

    sai_test_sched_group_attr_list_print (attr_count, &attr);
    return sai_rc;
}

static void sai_sched_group_verify_after_removal (sai_object_id_t sg_id)
{
    sai_status_t  sai_rc;

    if (sg_id != 0) {
        printf("Verify presence of sg :0x%" PRIx64 ". \n", sg_id);
        sai_rc =  sai_test_sched_group_remove (sg_id);
        EXPECT_EQ (SAI_STATUS_INVALID_OBJECT_ID, sai_rc);
    }
}

static sai_status_t sai_sched_group_max_hierarchy_level_get(void)
{
    sai_status_t  sai_rc;

    sai_rc = sai_test_switch_max_number_hierarchy_levels_get (&hqos_levels);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    EXPECT_EQ (1, (hqos_levels > 0));

    leaf_level = hqos_levels  - 1;
    return sai_rc;
}

static sai_status_t sai_sched_group_test_hierarchy_get (void)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    unsigned int     level = 0;
    unsigned int     group = 0;

    unsigned int    attr_count = 1;
    sai_attribute_t attr_list[attr_count];
    unsigned int    max_sg_count = 0;
    unsigned int    i = 0;

    sai_rc = sai_test_port_sched_group_id_count_get(default_port_id,
                                                    &max_sg_count);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_object_id_t sg_id_list_all[max_sg_count];

    memset (&sg_id_list[0][0], 0, sizeof(sg_id_list));

    sai_rc = sai_test_port_sched_group_id_list_get (default_port_id, max_sg_count,
                                                    &sg_id_list_all[0]);
    for (i = 0; i < max_sg_count; i++) {
        sai_rc = sai_test_sched_group_attr_get (sg_id_list_all[i], &attr_list[0], attr_count,
                                                SAI_SCHEDULER_GROUP_ATTR_LEVEL);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            return SAI_STATUS_FAILURE;
        }

        level = attr_list[0].value.u32;
        if (level < SAI_MAX_HIERARCHY_LEVELS) {
            for (group = 0; group < SAI_MAX_GROUPS_PER_LEVEL; group++) {
                if (sg_id_list[level][group])
                    continue;
                sg_id_list[level][group] = sg_id_list_all[i];
                break;
            }
        }
    }

    for (level = 0; level < SAI_MAX_HIERARCHY_LEVELS; level++) {
        for (group = 0; group < SAI_MAX_GROUPS_PER_LEVEL; group++) {
            if (sg_id_list[level][group])
                printf ("sg_id_list[%d][%d] sg :0x%" PRIx64 " \r\n",
                         level, group, sg_id_list[level][group]);
        }
    }

    unsigned int     max_queues = 0;

    sai_rc = sai_test_port_max_number_queues_get (default_port_id,
                                                  &max_queues);
    sai_rc = sai_test_port_queue_id_list_get (default_port_id, max_queues,
                                              &queue_id_list[0]);
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_sched_group_create_test_hierarchy (void)
{
    sai_object_id_t  parent_sg_id = SAI_NULL_OBJECT_ID;
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    unsigned int     level = 0;
    unsigned int     group = 0;
    unsigned int     queue = 0;
    unsigned int     max_childs = 20;

    printf("Create default Scheduler profile\r\n");
    /* Create Schduler Profile */
    sai_rc = sai_test_scheduler_create (&default_sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_DWRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    printf("Create Scheduler groups at all levels \r\n");
    /* Root/Level 0 */

    parent_sg_id = default_port_id;
    sai_rc = sai_test_sched_group_create (
                      &sg_id_list[level][group], 4,
                      SAI_SCHEDULER_GROUP_ATTR_PORT_ID, default_port_id,
                      SAI_SCHEDULER_GROUP_ATTR_LEVEL, level,
                      SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS, max_childs,
                      SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE, parent_sg_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    parent_sg_id = sg_id_list[0][0];

    /* Create SAI_TEST_GROUPS_PER_LEVEL at each level */
    /* Add childs SG's at level N+1 to parent level N */
    for (level = 1; level < hqos_levels; level++) {
        for (group = 0; group < SAI_TEST_GROUPS_PER_LEVEL; group++) {
            sai_rc = sai_test_sched_group_create (
                              &sg_id_list[level][group], 5,
                              SAI_SCHEDULER_GROUP_ATTR_PORT_ID, default_port_id,
                              SAI_SCHEDULER_GROUP_ATTR_LEVEL, level,
                              SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS, max_childs,
                              SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE, parent_sg_id,
                              SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID, default_sched_id);

            EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
        }
        parent_sg_id = sg_id_list[level][0];
    }

    for (queue = 0; queue < SAI_MAX_UC_QUEUES_PER_SG; queue++)
    {
         sai_rc = sai_test_queue_create(
                           &queue_id_list[queue], 4,
                           SAI_QUEUE_ATTR_PORT, default_port_id,
                           SAI_QUEUE_ATTR_TYPE, SAI_QUEUE_TYPE_UNICAST,
                           SAI_QUEUE_ATTR_INDEX, queue,
                           SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE, parent_sg_id);

         EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }

    for (queue = 0; queue < SAI_MAX_MC_QUEUES_PER_SG; queue++)
    {
         sai_rc = sai_test_queue_create(
                           &queue_id_list[SAI_MAX_UC_QUEUES_PER_SG + queue], 4,
                           SAI_QUEUE_ATTR_PORT, default_port_id,
                           SAI_QUEUE_ATTR_TYPE, SAI_QUEUE_TYPE_MULTICAST,
                           SAI_QUEUE_ATTR_INDEX, queue,
                           SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE, parent_sg_id);

         EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }

    for (queue = 0; queue < SAI_MAX_UC_QUEUES_PER_SG; queue++)
    {
         sai_rc = sai_test_queue_remove(queue_id_list[queue]);
         EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
         queue_id_list[queue] = 0;
    }

    for (queue = 0; queue < SAI_MAX_MC_QUEUES_PER_SG; queue++)
    {
         sai_rc = sai_test_queue_remove(queue_id_list[SAI_MAX_UC_QUEUES_PER_SG + queue]);
         EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
         queue_id_list[SAI_MAX_UC_QUEUES_PER_SG + queue] = 0;
    }

    for (queue = 0; queue < SAI_MAX_UC_QUEUES_PER_SG; queue++)
    {
         sai_rc = sai_test_queue_create(
                           &queue_id_list[queue], 4,
                           SAI_QUEUE_ATTR_PORT, default_port_id,
                           SAI_QUEUE_ATTR_TYPE, SAI_QUEUE_TYPE_UNICAST,
                           SAI_QUEUE_ATTR_INDEX, queue,
                           SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE, parent_sg_id);

         EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }

    for (queue = 0; queue < SAI_MAX_MC_QUEUES_PER_SG; queue++)
    {
         sai_rc = sai_test_queue_create(
                           &queue_id_list[SAI_MAX_UC_QUEUES_PER_SG + queue], 4,
                           SAI_QUEUE_ATTR_PORT, default_port_id,
                           SAI_QUEUE_ATTR_TYPE, SAI_QUEUE_TYPE_MULTICAST,
                           SAI_QUEUE_ATTR_INDEX, queue,
                           SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE, parent_sg_id);

         EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }
    return sai_rc;
}

static sai_status_t sai_sched_group_remove_test_hierarchy (void)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    int              level = 0;
    unsigned int     group = 0;
    unsigned int     queue = 0;

    if (is_fixed_hqos)
        return sai_rc;

    /* Remove queues as child */
    printf("Remove Queues from parent SG's at leaf level\r\n");
    for (queue = 0; queue < SAI_MAX_UC_QUEUES_PER_SG; queue++) {
        sai_rc = sai_test_queue_remove(queue_id_list[queue]);
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }

    for (queue = 0; queue < SAI_MAX_MC_QUEUES_PER_SG; queue++) {
        sai_rc = sai_test_queue_remove(
                     queue_id_list[SAI_MAX_UC_QUEUES_PER_SG + queue]);
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }

    printf("Remove SG's created at all levels\r\n");
    for (level = (int) hqos_levels - 1; level >= 0; level--) {
        for (group = 0; group < SAI_TEST_GROUPS_PER_LEVEL; group++) {
            if (sg_id_list[level][group] != 0) {
            printf("Remove SG 0x%" PRIx64 " at level %d and group %d \r\n",
                   sg_id_list[level][group], level, group);
            sai_rc = sai_test_sched_group_remove(sg_id_list[level][group]);
            EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
            sg_id_list[level][group] = 0;
            }
        }
    }

    sai_test_scheduler_remove (default_sched_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    return sai_rc;
}

/*
 * Validate get port scheduler group id list get.
 */
TEST (saiQosSchedulerGroupTest, port_sched_group_id_list_get)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    unsigned int max_ports = 0;
    unsigned int max_sg_count = 0;
    unsigned int port_index = 0;
    sai_object_id_t  port_id = SAI_NULL_OBJECT_ID;

    max_ports = sai_qos_max_ports_get();

    for (port_index = 0; port_index < max_ports; port_index++)
    {
        port_id = sai_qos_port_id_get (port_index);

        sai_rc = sai_test_port_sched_group_id_count_get(port_id,
                                                      &max_sg_count);
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

        sai_object_id_t sg_id_list[max_sg_count];

        sai_rc = sai_test_port_sched_group_id_list_get (port_id, max_sg_count,
                                                        &sg_id_list[0]);
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }
}

/*
 * Validate scheduler group create, remove and duplicate delete cases in
 * all hierarchy levels.
 */
TEST (saiQosSchedulerGroupTest, sched_group_create_and_remove)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    int level = 0;
    unsigned int max_childs = 1;
    sai_object_id_t parent_sg_id = default_port_id;
    sai_object_id_t sched_id = SAI_NULL_OBJECT_ID;

    if (is_fixed_hqos)
        return;

    printf("Create default Scheduler profile\r\n");
    /* Create Schduler Profile */
    sai_rc = sai_test_scheduler_create (&sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_DWRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_sched_group_create (
                      &sg_id_list[level][0], 4,
                      SAI_SCHEDULER_GROUP_ATTR_PORT_ID, default_port_id,
                      SAI_SCHEDULER_GROUP_ATTR_LEVEL, level,
                      SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS, max_childs,
                      SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE, parent_sg_id);
    parent_sg_id = sg_id_list[0][0];

    for (level = 1; level < (int) hqos_levels; level++) {
        sai_rc = sai_test_sched_group_create (
                          &sg_id_list[level][0], 5,
                          SAI_SCHEDULER_GROUP_ATTR_PORT_ID, default_port_id,
                          SAI_SCHEDULER_GROUP_ATTR_LEVEL, level,
                          SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS, max_childs,
                          SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE, parent_sg_id,
                          SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID, sched_id);
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
        parent_sg_id = sg_id_list[level][0];
    }

    for (level = (int)hqos_levels - 1; (level >= 0) && (sg_id_list[level][0] != 0) ; level--) {
        sai_rc =  sai_test_sched_group_remove (sg_id_list[level][0]);
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
        printf("Removed Scheduler group at level %d\r\n", level);
    }

    for (level = 0; level < (int)hqos_levels; level++) {
        printf("Verify Scheduler group at level %d\r\n", level);
        sai_sched_group_verify_after_removal (sg_id_list[level][0]);
        sg_id_list[level][0] = 0;
    }

    sai_rc = sai_test_scheduler_remove(sched_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate scheduler group create mandatory attribuite test.
 */
TEST (saiQosSchedulerGroupTest, sched_group_create_and_mandatory_attribute)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    if (is_fixed_hqos)
        return;

    /* Create Level 1 node */
    sai_rc = sai_test_sched_group_create (&sg_id_list[1][0], 1,
                                          SAI_SCHEDULER_GROUP_ATTR_PORT_ID,
                                          default_port_id);
    EXPECT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING, sai_rc);

    sai_rc = sai_test_sched_group_create (&sg_id_list[1][0], 1,
                                          SAI_SCHEDULER_GROUP_ATTR_LEVEL, 1);
    EXPECT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING, sai_rc);

    sai_rc = sai_test_sched_group_create (&sg_id_list[1][0], 2,
                                          SAI_SCHEDULER_GROUP_ATTR_LEVEL, 1,
                                          SAI_SCHEDULER_GROUP_ATTR_PORT_ID,
                                          default_port_id);
    EXPECT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING, sai_rc);
}

/*
 * Validate add/remove childs to parent SG.
 * - Create Root/Level 0 scheduler group.
 * - Create 2 scheduler group at level 1 ... N-1.
 * - Add all scheduler groups at level 1 to parent scheduler groups at level 0.
 * - Remove child scheduler groups at level 1 from parent scheduler group at level 0.
 */
TEST (saiQosSchedulerGroupTest, modify_parent)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t  attr = {0};
    unsigned int     child_count = 0;
    sai_object_id_t  child_list[SAI_TEST_GROUPS_PER_LEVEL] = {0};

    if (is_fixed_hqos)
        return;

    printf("Create Scheduler groups hierarchy at all levels \r\n");
    sai_rc = sai_sched_group_create_test_hierarchy();
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Move SG from Level 2 between Level 1 nodes*/
    if (hqos_levels >= 2) {
        printf("Move L2.0 from L1.0 to L1.1 \r\n");

        printf("Before movement \r\n");
        sai_rc = sai_test_sched_group_child_count_get(sg_id_list[1][1], &child_count);
        printf("Child count of L1.1 is %u\r\n", child_count);

        if (child_count > 0)
        {
            printf("Child List of L1.1 \r\n");
            sai_rc = sai_test_sched_group_child_id_list_get(sg_id_list[1][1],
                                                            child_count,
                                                            child_list);
        }

        sai_rc = sai_test_sched_group_child_count_get(sg_id_list[1][0], &child_count);
        printf("Child count of L1.0 is %u\r\n", child_count);

        memset(child_list, 0, sizeof(child_list));
        if (child_count > 0)
        {
            printf("Child List of L1.0 \r\n");
            sai_rc = sai_test_sched_group_child_id_list_get(sg_id_list[1][0],
                                                            child_count,
                                                            child_list);
        }

        attr.id = SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE;
        attr.value.oid = sg_id_list[1][1];

        sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id_list[2][0],
                                                                        &attr);
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

        printf("After movement \r\n");
        sai_rc = sai_test_sched_group_child_count_get(sg_id_list[1][1], &child_count);
        printf("Child count of L1.1 is %u\r\n", child_count);

        if (child_count > 0)
        {
            printf("Child List of L1.1 \r\n");
            sai_rc = sai_test_sched_group_child_id_list_get(sg_id_list[1][1],
                                                            child_count,
                                                            child_list);
        }

        sai_rc = sai_test_sched_group_child_count_get(sg_id_list[1][0], &child_count);
        printf("Child count of L1.0 is %u\r\n", child_count);

        memset(child_list, 0, sizeof(child_list));
        if (child_count > 0)
        {
            printf("Child List of L1.0 \r\n");
            sai_rc = sai_test_sched_group_child_id_list_get(sg_id_list[1][0],
                                                            child_count,
                                                            child_list);
        }
    }

    sai_rc = sai_sched_group_remove_test_hierarchy();
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate scheduler group get attributes.
 */
TEST (saiQosSchedulerGroupTest, sched_group_attribute_get)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  sg_id;
    unsigned int     attr_count = 4;
    sai_attribute_t  attr_list[attr_count];
    sai_object_id_t  sched_id = SAI_NULL_OBJECT_ID;

    if (is_fixed_hqos)
        return;

    printf("Create default Scheduler profile\r\n");
    /* Create Schduler Profile */
    sai_rc = sai_test_scheduler_create (&sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_DWRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_sched_group_create (
                      &sg_id, 5,
                      SAI_SCHEDULER_GROUP_ATTR_PORT_ID, default_port_id,
                      SAI_SCHEDULER_GROUP_ATTR_LEVEL, 0,
                      SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS, 2,
                      SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE, default_port_id,
                      SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID, sched_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_sched_group_attr_get (sg_id, &attr_list[0], attr_count,
                                            SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT,
                                            SAI_SCHEDULER_GROUP_ATTR_PORT_ID,
                                            SAI_SCHEDULER_GROUP_ATTR_LEVEL,
                                            SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID);

    EXPECT_EQ (0, attr_list[0].value.u32);
    EXPECT_EQ (default_port_id, attr_list[1].value.oid);
    EXPECT_EQ (0, attr_list[2].value.u32);
    EXPECT_EQ (sched_id, attr_list[3].value.oid);

    sai_rc =  sai_test_sched_group_remove (sg_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_scheduler_remove(sched_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate scheduler group child list get attribute.
 */
TEST (saiQosSchedulerGroupTest, sched_group_child_list_attribute_get)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    unsigned int     queue = 0;
    unsigned int     child = 0;
    unsigned int     level = 0;
    unsigned int     next_level = 0;
    sai_object_id_t  child_list[SAI_MAX_GROUPS_PER_LEVEL];
    sai_object_id_t  queue_list[SAI_MAX_QUEUES_PER_PORT];

    if (is_fixed_hqos == false)
        sai_rc = sai_sched_group_create_test_hierarchy();

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Get Child List and Validate */
    for (level = 0; level < leaf_level; level++) {

        next_level = level + 1;
        sai_rc = sai_test_sched_group_child_id_list_get (sg_id_list[level][0],
                                                         SAI_MAX_GROUPS_PER_LEVEL,
                                                         &child_list[0]);
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

        for (child = 0; child < (is_fixed_hqos == true ?
             SAI_MAX_GROUPS_PER_LEVEL : SAI_TEST_GROUPS_PER_LEVEL); child++) {
            if (sg_id_list[next_level][child]) {
                EXPECT_EQ (sg_id_list[next_level][child], child_list[child]);
            }
        }
    }

    sai_rc = sai_test_sched_group_child_count_get(sg_id_list[leaf_level][0],
                                                  &max_queues);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_sched_group_child_id_list_get (sg_id_list[leaf_level][0],
                                                     max_queues,
                                                     &queue_list[0]);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    for (queue = 0; queue < SAI_MAX_QUEUES_PER_TEST; queue++) {
        EXPECT_EQ (queue_id_list[queue], queue_list[queue]);
    }

    sai_rc = sai_sched_group_remove_test_hierarchy();
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate scheduler group set attributes.
 */
TEST (saiQosSchedulerGroupTest, sched_group_attribute_set)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  sg_id;
    sai_attribute_t  attr;
    sai_object_id_t  child_id;
    sai_object_id_t  sched_id = SAI_NULL_OBJECT_ID;

    if (is_fixed_hqos)
        return;

    printf("Create default Scheduler profile\r\n");
    /* Create Schduler Profile */
    sai_rc = sai_test_scheduler_create (&sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_DWRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_sched_group_create (
                      &sg_id, 5,
                      SAI_SCHEDULER_GROUP_ATTR_PORT_ID, default_port_id,
                      SAI_SCHEDULER_GROUP_ATTR_LEVEL, 0,
                      SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS,1,
                      SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE, default_port_id,
                      SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID, sched_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Validate CREATE_ONLY Attributes's */
    attr.id = SAI_SCHEDULER_GROUP_ATTR_PORT_ID;
    attr.value.oid = default_port_id;

    sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id,
                                                                    &attr);
    EXPECT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_rc);

    attr.id = SAI_SCHEDULER_GROUP_ATTR_LEVEL;
    attr.value.u32 = 1;
    sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id,
                                                                    &attr);
    EXPECT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_rc);

    /* Validate READ ONLY Attribute */
    attr.id = SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT;
    attr.value.objlist.count = 1;
    attr.value.objlist.list = &child_id;

    sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id,
                                                                    &attr);
    EXPECT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0, sai_rc);

    sai_rc =  sai_test_sched_group_remove (sg_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_scheduler_remove(sched_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate scheduler profile on scheduler group.
 * - Create Hiearchy
 * - Set Scheduler profile
 * - Replace with new profile.
 * - Delete the profile.
 * - Delete Hierarchy.
 */

TEST (saiQosSchedulerGroupTest, sched_group_scheduler_profile_attribute_set)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    unsigned int     level = 0;
    unsigned int     group = 0;
    sai_attribute_t  attr;

    if (is_fixed_hqos == false) {
        sai_rc = sai_sched_group_create_test_hierarchy();
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

        sai_rc =  sai_test_scheduler_remove (default_sched_id);
        EXPECT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);
    }

    /* Set scheduler in SG's for all levels, except ROOT */
    attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;


    sai_object_id_t  sched_id_2 = SAI_NULL_OBJECT_ID;
    /* Create New Schduler Group */
    sai_rc = sai_test_scheduler_create (&sched_id_2, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_DWRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 100,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 4098,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 409800,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 20480);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Set new scheduler in SG's for all levels, except ROOT */
    attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = sched_id_2;

    for (level = 1; level < hqos_levels; level++) {
        for (group = 0; (group < SAI_TEST_GROUPS_PER_LEVEL) &&
                        (sg_id_list[level][group] != 0); group++) {
                sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id_list[level][group],
                                                                                &attr);
                if (sai_rc == SAI_STATUS_SUCCESS) {
                    printf ("Successfully Set Scheduler profile id 0x%" PRIx64 " for SG  0x%" PRIx64 ".\n",
                            attr.value.oid, sg_id_list[level][group]);
                }
                EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
        }
    }

    sai_rc =  sai_test_scheduler_remove (sched_id_2);
    EXPECT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);

    /* Remove Scheduler ID from ALL SG's */
    attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    for (level = 1; level < hqos_levels; level++) {
        for (group = 0; (group < SAI_TEST_GROUPS_PER_LEVEL) &&
                        (sg_id_list[level][group] != 0); group++) {
            sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id_list[level][group],
                                                                            &attr);
            if (sai_rc == SAI_STATUS_SUCCESS) {
                printf ("Succesfully ReSet Scheduler profile id 0x%" PRIx64 " for SG  0x%" PRIx64 ".\n",
                        attr.value.oid, sg_id_list[level][group]);
            }
            EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
        }
    }

    sai_rc =  sai_test_scheduler_remove (sched_id_2);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_sched_group_remove_test_hierarchy();
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate Modifying scheduler parmas after applyinig on
 * scheduler group.
 */

TEST (saiQosSchedulerGroupTest, sched_group_modify_scheduler_attributes)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t  attr;
    sai_object_id_t  sched_id = SAI_NULL_OBJECT_ID;

    sai_rc = sai_test_scheduler_create (&sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_WRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    if (is_fixed_hqos == false) {
        sai_rc = sai_sched_group_create_test_hierarchy();
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }

    attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = sched_id;

    /* Apply scheduler on Level 1 SG */
    sai_object_id_t  sg_id = sg_id_list[1][0];
    sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id,
                                                                    &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Modify */
    attr.id = SAI_SCHEDULER_ATTR_SCHEDULING_TYPE;
    attr.value.s32 = SAI_SCHEDULING_TYPE_STRICT;

    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT;
    attr.value.u8 = 50;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_METER_TYPE;
    attr.value.s32 = SAI_METER_TYPE_PACKETS;

    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Remove */
    attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id,
                                                                    &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_sched_group_remove_test_hierarchy();
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate strict priority scheduler apply on scheduler group.
 */
TEST (saiQosSchedulerGroupTest, scheduler_strict_priority_on_scheduler_group)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  sched_id = SAI_NULL_OBJECT_ID;
    sai_attribute_t  attr;

    sai_rc = sai_test_scheduler_create (&sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_STRICT,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    if (is_fixed_hqos == false) {
        sai_rc = sai_sched_group_create_test_hierarchy();
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }

    attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = sched_id;

    /* Apply Scheduler to Level 1 node */
    sai_object_id_t  sg_id = sg_id_list[1][0];
    sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id, &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    if (hqos_levels > 2)
    {
        /* Apply Scheduler to Level 2.0 node */
        sg_id = sg_id_list[2][0];
        sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id, &attr);
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }
    /* SP + NO MAX Rate & Burst */
    attr.id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE;
    attr.value.u64 = 0;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE;
    attr.value.u64 = 0;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* SP + NO SHAPE */
    attr.id = SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE;
    attr.value.u64 = 0;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE;
    attr.value.u64 = 0;
    sai_rc = p_sai_scheduler_api_table->set_scheduler_attribute (sched_id,
                                                                 &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    if (hqos_levels > 2) {
    sg_id = sg_id_list[2][0];
    sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id, &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }
    sg_id = sg_id_list[1][0];
    sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id, &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc =  sai_test_scheduler_remove (sched_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_sched_group_remove_test_hierarchy();
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate scheduler group set attributes along with create.
 */
TEST (saiQosSchedulerGroupTest, sched_group_attribute_create_set)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  sg_id = SAI_NULL_OBJECT_ID;

    if (is_fixed_hqos)
        return;

    sai_rc = sai_test_sched_group_create (&sg_id, 5,
                                          SAI_SCHEDULER_GROUP_ATTR_PORT_ID,
                                          default_port_id,
                                          SAI_SCHEDULER_GROUP_ATTR_LEVEL, 0,
                                          SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS,1,
                                          SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT, 10,
                                          SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE,
                                          default_port_id);
    EXPECT_EQ (sai_test_invalid_attr_status_code(SAI_STATUS_INVALID_ATTRIBUTE_0, 3), sai_rc);
}

/*
 * Validate scheduler group inuse test case.
 */
TEST (saiQosSchedulerGroupTest, sched_group_inuse)
{
    sai_status_t     sai_rc = SAI_STATUS_SUCCESS;
    unsigned int     level = 0;
    unsigned int     group = 0;
    sai_attribute_t  attr;

    if (is_fixed_hqos == false) {
        sai_rc = sai_sched_group_create_test_hierarchy();
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
        if (sg_id_list[0][0] != 0) {
            sai_rc =  sai_test_sched_group_remove (sg_id_list[0][0]);
            EXPECT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);
        }
    }

    sai_object_id_t  sched_id = SAI_NULL_OBJECT_ID;

    /* Create Schduler Profile */
    sai_rc = sai_test_scheduler_create (&sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_DWRR,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 2048,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 20480,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 10240);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Set scheduler in SG's for all levels, except ROOT */
    attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = sched_id;

    for (level = 1; level < hqos_levels; level++) {
        for (group = 0; (group < SAI_TEST_GROUPS_PER_LEVEL) &&
                        (sg_id_list[level][group] != 0); group++) {
             sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id_list[level][group],
                                                                                &attr);
             EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
             if (sai_rc == SAI_STATUS_SUCCESS) {
                 printf ("Set Schduler profile id 0x%" PRIx64 " for SG  0x%" PRIx64 ".\n",
                         attr.value.oid, sg_id_list[level][group]);
             }
        }
    }

    /* Remove Scheduler ID from ALL SG's */
    attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    for (level = 1; level < hqos_levels; level++) {
        for (group = 0; (group < SAI_TEST_GROUPS_PER_LEVEL) &&
                        (sg_id_list[level][group] != 0); group++) {
            sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id_list[level][group],
                                                                            &attr);
            EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

            if (sai_rc == SAI_STATUS_SUCCESS) {
                printf ("ReSet Schduler profile id 0x%" PRIx64 " for SG  0x%" PRIx64 ".\n",
                        attr.value.oid, sg_id_list[level][group]);
            }
        }
    }

    if (is_fixed_hqos == false) {
        sai_rc =  sai_test_sched_group_remove (sg_id_list[1][0]);
        EXPECT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);

        sai_rc = sai_sched_group_remove_test_hierarchy();
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }
    sai_rc =  sai_test_scheduler_remove (sched_id);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

/*
 * Validate strict priority setting for more than one children
 * Apply a strict priority scheduler on more than one children at a level
 * Retreive the applied scheduler id to validate
 * Revert by applying a WRR scheduler on a SP child
 *
 */
TEST (saiQosSchedulerGroupTest, two_strict_priority)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  sp_sched_id[3] = {SAI_NULL_OBJECT_ID,
                                       SAI_NULL_OBJECT_ID, SAI_NULL_OBJECT_ID} ;
    sai_object_id_t  wrr_sched_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t  sg_id = SAI_NULL_OBJECT_ID;
    sai_attribute_t  attr;
    sai_attribute_t  get_attr;

    if (is_fixed_hqos == false) {
        sai_rc = sai_sched_group_create_test_hierarchy();
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }

    for (int iLoop = 0; iLoop < 3; iLoop++) {
        sai_rc = sai_test_scheduler_create (&sp_sched_id[iLoop], 6,
                                            SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_STRICT,
                                            SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                            SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 512,
                                            SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                            SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 1024,
                                            SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 2048);
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    }

    sai_rc = sai_test_scheduler_create (&wrr_sched_id, 7,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_TYPE, SAI_SCHEDULING_TYPE_WRR,
                                        SAI_SCHEDULER_ATTR_METER_TYPE, SAI_METER_TYPE_BYTES,
                                        SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT, 10,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE, 512,
                                        SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, 1024,
                                        SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE, 2048);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    get_attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
    attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = sp_sched_id[0];

    /* Apply SP scheduler on L0 nodes. The order of sg node is made intentionally random */
    sg_id = sg_id_list[1][0];
    sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id, &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = p_sai_qos_sg_api_table->get_scheduler_group_attribute (sg_id, 1, &get_attr);
    EXPECT_EQ (sp_sched_id[0], get_attr.value.oid);

    attr.value.oid = sp_sched_id[1];
    sg_id = sg_id_list[1][2];
    sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id, &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = p_sai_qos_sg_api_table->get_scheduler_group_attribute (sg_id, 1, &get_attr);
    EXPECT_EQ (sp_sched_id[1], get_attr.value.oid);

    attr.value.oid = sp_sched_id[2];
    sg_id = sg_id_list[1][1];
    sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id, &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = p_sai_qos_sg_api_table->get_scheduler_group_attribute (sg_id, 1, &get_attr);
    EXPECT_EQ (sp_sched_id[2], get_attr.value.oid);

    if (hqos_levels > 2) {
        /* Apply SP scheduler on L1 nodes */
        sg_id = sg_id_list[2][2];
        attr.value.oid = sp_sched_id[1];
        sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id, &attr);
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
        sai_rc = p_sai_qos_sg_api_table->get_scheduler_group_attribute (sg_id, 1, &get_attr);
        EXPECT_EQ (sp_sched_id[1], get_attr.value.oid);

        attr.value.oid = sp_sched_id[0];
        sg_id = sg_id_list[2][0];
        sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id, &attr);
        EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

        sai_rc = p_sai_qos_sg_api_table->get_scheduler_group_attribute (sg_id, 1, &get_attr);
        EXPECT_EQ (sp_sched_id[0], get_attr.value.oid);
    }
    attr.id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = wrr_sched_id;

    sg_id = sg_id_list[1][0];
    sai_rc = p_sai_qos_sg_api_table->set_scheduler_group_attribute (sg_id, &attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = p_sai_qos_sg_api_table->get_scheduler_group_attribute (sg_id, 1, &get_attr);
    EXPECT_EQ (wrr_sched_id, get_attr.value.oid);
}

int main (int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    SetUpTestCase ();
    default_port_id = sai_qos_port_id_get (default_port);
    sai_sched_group_max_hierarchy_level_get();
    sai_sched_group_test_hierarchy_get();
    /* TODO - Get this by switch attribute, TH set this to true */
    is_fixed_hqos = false;
    return RUN_ALL_TESTS();
}

