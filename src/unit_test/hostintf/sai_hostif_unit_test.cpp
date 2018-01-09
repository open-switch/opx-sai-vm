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
* @file ai_hostif_unit_test.cpp
*
* @brief This file contains the google unit test cases for testing the
*        SAI host interface functionality.
*
*************************************************************************/
extern "C" {
#include "sai.h"
#include "saihostintf.h"
#include "saitypes.h"
}


#include "gtest/gtest.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#define SAI_MAX_PORTS  256

static sai_object_id_t switch_id =0;
static unsigned int port_count = 0;
static sai_object_id_t port_list[SAI_MAX_PORTS] = {0};

static inline void sai_packet_event_callback (const void *buffer,
                                              sai_size_t buffer_size,
                                              uint32_t attr_count,
                                              const sai_attribute_t *attr_list)
{
    unsigned int index = 0;

    printf("Pkt Size = %lu attr_count = %u\n",buffer_size, attr_count);
    for (index = 0; index < attr_count; index++) {
        printf("Attribute id=%u\n",attr_list[index].id);
        if (SAI_HOSTIF_PACKET_ATTR_INGRESS_PORT == attr_list[index].id) {
            printf("Ingress Port object 0x%" PRIx64 ".\n",attr_list[index].value.oid);
        } else if (SAI_HOSTIF_PACKET_ATTR_HOSTIF_TRAP_ID == attr_list[index].id) {
            printf("Trap id = 0x%" PRIx64 "\n", attr_list[index].value.oid);
        }
    }
}

/*
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

static inline void  sai_switch_shutdown_callback (void)
{

}

/*
 * API query is done while running the first test case and
 * the method table is stored in sai_hostif_api_table so
 * that its available for the rest of the test cases which
 * use the method table
 */
class hostIntfInit : public ::testing::Test
{
    protected:
        static void SetUpTestCase()
        {
            sai_attribute_t sai_attr_set[7];
            uint32_t attr_count = 7;

            memset(sai_attr_set,0, sizeof(sai_attr_set));

            /*
             * Query and populate the SAI Switch API Table.
             */
            EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
                    (SAI_API_SWITCH, (static_cast<void**>
                                      (static_cast<void*>(&p_sai_switch_api_tbl)))));

            ASSERT_TRUE (p_sai_switch_api_tbl != NULL);

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
            ASSERT_EQ(NULL,sai_api_query(SAI_API_HOST_INTERFACE,
                        (static_cast<void**>(static_cast<void*>(&sai_hostif_api_table)))));

            ASSERT_TRUE(sai_hostif_api_table != NULL);

            EXPECT_TRUE(sai_hostif_api_table->create_hostif != NULL);
            EXPECT_TRUE(sai_hostif_api_table->remove_hostif != NULL);
            EXPECT_TRUE(sai_hostif_api_table->set_hostif_attribute != NULL);
            EXPECT_TRUE(sai_hostif_api_table->get_hostif_attribute !=NULL);
            EXPECT_TRUE(sai_hostif_api_table->create_hostif_trap_group !=NULL);
            EXPECT_TRUE(sai_hostif_api_table->remove_hostif_trap_group !=NULL);
            EXPECT_TRUE(sai_hostif_api_table->set_trap_group_attribute !=NULL);
            EXPECT_TRUE(sai_hostif_api_table->get_trap_group_attribute !=NULL);
            EXPECT_TRUE(sai_hostif_api_table->set_trap_attribute !=NULL);
            EXPECT_TRUE(sai_hostif_api_table->get_trap_attribute !=NULL);
            EXPECT_TRUE(sai_hostif_api_table->set_user_defined_trap_attribute !=NULL);
            EXPECT_TRUE(sai_hostif_api_table->get_user_defined_trap_attribute !=NULL);
            EXPECT_TRUE(sai_hostif_api_table->recv_packet !=NULL);
            EXPECT_TRUE(sai_hostif_api_table->send_packet !=NULL);
            sai_attribute_t sai_port_attr;
            sai_status_t ret = SAI_STATUS_SUCCESS;

                memset (&sai_port_attr, 0, sizeof (sai_port_attr));

            sai_port_attr.id = SAI_SWITCH_ATTR_PORT_LIST;
            sai_port_attr.value.objlist.count = SAI_MAX_PORTS;
            sai_port_attr.value.objlist.list  = port_list;

            ret = p_sai_switch_api_tbl->get_switch_attribute(0,1,&sai_port_attr);
            port_count = sai_port_attr.value.objlist.count;
            EXPECT_EQ (SAI_STATUS_SUCCESS,ret);
        }
        static sai_switch_api_t *p_sai_switch_api_tbl;
        static sai_hostif_api_t *sai_hostif_api_table;
        static sai_status_t sai_test_create_trapgroup(sai_object_id_t *group,
                                                bool admin_state,
                                                unsigned int cpu_queue);
};

sai_hostif_api_t* hostIntfInit::sai_hostif_api_table = NULL;
sai_switch_api_t *hostIntfInit::p_sai_switch_api_tbl = NULL;

#define SAI_MAX_TRAP_GROUP_ATTRS 4
#define SAI_GTEST_CPU_QUEUE_1 1
#define SAI_GTEST_CPU_QUEUE_2 2

sai_status_t hostIntfInit :: sai_test_create_trapgroup(sai_object_id_t *group,
                                                       bool admin_state,
                                                       unsigned int cpu_queue)
{
    sai_attribute_t attr_list[SAI_MAX_TRAP_GROUP_ATTRS] = {0};
    unsigned int attr_count = 0;
    sai_status_t rc = SAI_STATUS_FAILURE;

    attr_list[attr_count].id = SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE;
    attr_list[attr_count].value.booldata = admin_state;
    attr_count++;

    attr_list[attr_count].id = SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE;
    attr_list[attr_count].value.u32 = cpu_queue;
    attr_count++;

    rc = sai_hostif_api_table->create_hostif_trap_group(group, switch_id,
                                                        attr_count, attr_list);
    return rc;
}

TEST_F(hostIntfInit, default_trap_group_operations)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_attribute_t attr = {0};
    sai_object_id_t trap_group_oid = 0;

    memset(&attr, 0, sizeof(sai_attribute_t));

    attr.id = SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP;
    rc = p_sai_switch_api_tbl->get_switch_attribute(switch_id,1, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);
    /*Default trap group object should not be NULL*/
    ASSERT_NE (attr.value.oid, SAI_NULL_OBJECT_ID);
    trap_group_oid = attr.value.oid;

    /*Default trap group object is read only and cannot be set*/
    /*Trying a negative test case*/
    attr.id = SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP;
    rc = p_sai_switch_api_tbl->set_switch_attribute(switch_id,&attr);
    ASSERT_NE (rc, SAI_STATUS_SUCCESS);

    /* Default trap group points to CPU queue 0 and the key for trap group is
     * the cpu queue. So creating another trap group with cpu queue 0 would
     * not be allowed. Testing this case.
     */
    rc = sai_test_create_trapgroup(&trap_group_oid, true, 0);
    ASSERT_NE (rc, SAI_STATUS_SUCCESS);

    /*Default trap group object cannot be removed*/
    /*Trying a negative test case*/
    rc = sai_hostif_api_table->remove_hostif_trap_group(trap_group_oid);
    ASSERT_NE (rc, SAI_STATUS_SUCCESS);
}

TEST_F(hostIntfInit, create_remove_trapgroup)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_object_id_t trap_group_oid = 0;

    rc = sai_test_create_trapgroup(&trap_group_oid, true, SAI_GTEST_CPU_QUEUE_1);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    rc = sai_hostif_api_table->remove_hostif_trap_group(trap_group_oid);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);
}

TEST_F(hostIntfInit, set_get_trapgroup)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_object_id_t trap_group_oid = 0;
    sai_attribute_t attr[2] = {0};

    rc = sai_test_create_trapgroup(&trap_group_oid, true, SAI_GTEST_CPU_QUEUE_2);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    memset(&attr, 0, sizeof(attr));
    attr[0].id = SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE;
    attr[1].id = SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE;

    rc = sai_hostif_api_table->get_trap_group_attribute(trap_group_oid, 2, attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    EXPECT_EQ(attr[0].value.u32, SAI_GTEST_CPU_QUEUE_2);
    EXPECT_EQ(attr[1].value.booldata, true);

    attr[0].id = SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE;
    attr[0].value.u32 = SAI_GTEST_CPU_QUEUE_1;
    rc = sai_hostif_api_table->set_trap_group_attribute(trap_group_oid, attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    memset(&attr, 0, sizeof(attr));
    attr[0].id = SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE;
    attr[1].id = SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE;

    rc = sai_hostif_api_table->get_trap_group_attribute(trap_group_oid, 2, attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    EXPECT_EQ(attr[0].value.u32, SAI_GTEST_CPU_QUEUE_1);
    EXPECT_EQ(attr[1].value.booldata, true);

    rc = sai_hostif_api_table->remove_hostif_trap_group(trap_group_oid);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);
}

TEST_F(hostIntfInit, set_ttl_trap)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_object_id_t trap_group_oid = 0;
    sai_attribute_t attr = {0}, attr_list[2] = {0};

    rc = sai_test_create_trapgroup(&trap_group_oid, true, SAI_GTEST_CPU_QUEUE_2);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr.id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
    attr.value.s32 = SAI_PACKET_ACTION_TRAP;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_TTL_ERROR, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr.id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
    attr.value.oid = trap_group_oid;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_TTL_ERROR, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
    attr_list[1].id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;

    rc = sai_hostif_api_table->get_trap_attribute(SAI_HOSTIF_TRAP_TYPE_TTL_ERROR, 2, attr_list);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    EXPECT_EQ(attr_list[0].value.s32, SAI_PACKET_ACTION_TRAP);
    EXPECT_EQ(attr_list[1].value.oid, trap_group_oid);

    attr.id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_TTL_ERROR, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    rc = sai_hostif_api_table->remove_hostif_trap_group(trap_group_oid);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);
}

TEST_F(hostIntfInit, set_l3_mtu_failure_trap)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_object_id_t trap_group_oid = 0;
    sai_attribute_t attr = {0}, attr_list[2] = {0};

    rc = sai_test_create_trapgroup(&trap_group_oid, true, SAI_GTEST_CPU_QUEUE_1);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr.id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
    attr.value.s32 = SAI_PACKET_ACTION_TRAP;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr.id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
    attr.value.oid = trap_group_oid;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
    attr_list[1].id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;

    rc = sai_hostif_api_table->get_trap_attribute(SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR, 2, attr_list);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    EXPECT_EQ(attr_list[0].value.s32, SAI_PACKET_ACTION_TRAP);
    EXPECT_EQ(attr_list[1].value.oid, trap_group_oid);

    attr.id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    rc = sai_hostif_api_table->remove_hostif_trap_group(trap_group_oid);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);
}

TEST_F(hostIntfInit, set_samplepacket_trap)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_object_id_t trap_group_oid = 0;
    sai_attribute_t attr = {0};

    rc = sai_test_create_trapgroup(&trap_group_oid, true, SAI_GTEST_CPU_QUEUE_1);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr.id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
    attr.value.oid = trap_group_oid;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    memset(&attr, 0, sizeof(sai_attribute_t));
    attr.id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;

    rc = sai_hostif_api_table->get_trap_attribute(SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET, 1, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    EXPECT_EQ(attr.value.oid, trap_group_oid);

    attr.id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    rc = sai_hostif_api_table->remove_hostif_trap_group(trap_group_oid);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);
}

TEST_F(hostIntfInit, set_dhcp_trap)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_object_id_t trap_group_oid = 0;
    sai_object_list_t portlist = {0};
    sai_attribute_t attr = {0}, attr_list[4] = {0};

    rc = sai_test_create_trapgroup(&trap_group_oid, true, SAI_GTEST_CPU_QUEUE_2);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr.id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
    attr.value.oid = trap_group_oid;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_DHCP, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr.id = SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY;
    attr.value.u32 = 5;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_DHCP, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    portlist.count = 2;
    portlist.list = (sai_object_id_t *) calloc(portlist.count, sizeof(sai_object_id_t));
    portlist.list[0] = port_list[port_count - 2];
    portlist.list[1] = port_list[port_count - 1];

    /* TODO -  Until SAI implementaion changes, exclude list will work as enbale list */
    attr.id = SAI_HOSTIF_TRAP_ATTR_EXCLUDE_PORT_LIST;
    attr.value.objlist = portlist;
    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_DHCP, &attr);
    free(portlist.list );
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr.id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
    attr.value.s32 = SAI_PACKET_ACTION_TRAP;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_DHCP, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
    attr_list[1].id = SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY;
    attr_list[2].id = SAI_HOSTIF_TRAP_ATTR_EXCLUDE_PORT_LIST;
    attr_list[2].value.objlist.count = 2;
    attr_list[2].value.objlist.list = (sai_object_id_t *)
                                calloc(attr_list[2].value.objlist.count,
                                                sizeof(sai_object_id_t));
    attr_list[3].id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
    rc = sai_hostif_api_table->get_trap_attribute(SAI_HOSTIF_TRAP_TYPE_DHCP,
                                                  4, attr_list);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);
    EXPECT_EQ (attr_list[0].value.oid, trap_group_oid);
    EXPECT_EQ (attr_list[1].value.u32, 5);
    EXPECT_EQ (attr_list[2].value.objlist.count, 2);
    EXPECT_EQ (attr_list[3].value.s32, SAI_PACKET_ACTION_TRAP);

    attr.id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_DHCP, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    rc = sai_hostif_api_table->remove_hostif_trap_group(trap_group_oid);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

}

TEST_F(hostIntfInit, share_set_trap_group)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_object_id_t trap_group_oid = 0;
    sai_attribute_t attr = {0};

    rc = sai_test_create_trapgroup(&trap_group_oid, true, SAI_GTEST_CPU_QUEUE_1);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr.id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
    attr.value.s32 = SAI_PACKET_ACTION_TRAP;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr.id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
    attr.value.oid = trap_group_oid;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr.id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
    attr.value.s32 = SAI_PACKET_ACTION_TRAP;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_TTL_ERROR, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr.id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
    attr.value.oid = trap_group_oid;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_TTL_ERROR, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr.id = SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE;
    attr.value.u32 = SAI_GTEST_CPU_QUEUE_2;
    rc = sai_hostif_api_table->set_trap_group_attribute(trap_group_oid, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    attr.id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    rc = sai_hostif_api_table->set_trap_attribute(SAI_HOSTIF_TRAP_TYPE_TTL_ERROR, &attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);

    rc = sai_hostif_api_table->remove_hostif_trap_group(trap_group_oid);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);
}

TEST_F(hostIntfInit, send_pkt_pipeline_bypass)
{
    sai_attribute_t sai_attr[2];
    sai_status_t rc = SAI_STATUS_FAILURE;

    unsigned char buffer[] =
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x82, 0x2f,
     0x2e, 0x42, 0x46, 0x74, 0x08, 0x06, 0x00, 0x01,
     0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x82, 0x2f,
     0x2e, 0x42, 0x46, 0x74, 0x0a, 0x00, 0x00, 0x01,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00,
     0x00, 0x02, 0x00, 0x00, 0x00, 0x00};

    sai_attr[0].id = SAI_HOSTIF_PACKET_ATTR_EGRESS_PORT_OR_LAG;
    sai_attr[0].value.oid = port_list[port_count - 1];

    sai_attr[1].id = SAI_HOSTIF_PACKET_ATTR_HOSTIF_TX_TYPE;
    sai_attr[1].value.s32 = SAI_HOSTIF_TX_TYPE_PIPELINE_BYPASS;

    rc = sai_hostif_api_table->send_packet(SAI_NULL_OBJECT_ID, buffer,
                                        sizeof(buffer), 2, &sai_attr[0]);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);
}

TEST_F(hostIntfInit, send_pkt_pipeline_lookup)
{
    sai_attribute_t sai_attr = {0};
    sai_status_t rc = SAI_STATUS_FAILURE;

    unsigned char buffer[] =
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x82, 0x2f,
     0x2e, 0x42, 0x46, 0x74, 0x08, 0x06, 0x00, 0x01,
     0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x82, 0x2f,
     0x2e, 0x42, 0x46, 0x74, 0x0a, 0x00, 0x00, 0x01,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00,
     0x00, 0x02, 0x00, 0x00, 0x00, 0x00};

    sai_attr.id = SAI_HOSTIF_PACKET_ATTR_HOSTIF_TX_TYPE;
    sai_attr.value.s32 = SAI_HOSTIF_TX_TYPE_PIPELINE_LOOKUP;

    rc = sai_hostif_api_table->send_packet(SAI_NULL_OBJECT_ID, buffer,
                                           sizeof(buffer), 1, &sai_attr);
    ASSERT_EQ (rc, SAI_STATUS_SUCCESS);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
