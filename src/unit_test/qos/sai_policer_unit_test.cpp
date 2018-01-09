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
* @file  sai_policer_unit_test.cpp
*
* @brief This file contains tests for policer APIs
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
#include "saiacl.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saiswitch.h"
#include <inttypes.h>
}

#define LOG_PRINT(msg, ...) \
    printf(msg, ##__VA_ARGS__)

static sai_object_id_t switch_id = 0;
/*
 * API query is done while running the first test case and
 * the method table is stored in sai_switch_api_table so
 * that its available for the rest of the test cases which
 * use the method table
 */
class policer : public ::testing::Test
{
    protected:
        static void SetUpTestCase()
        {

            sai_attribute_t sai_attr_set[7];
            uint32_t attr_count = 7;

            memset(sai_attr_set,0, sizeof(sai_attr_set));

            ASSERT_EQ(SAI_STATUS_SUCCESS,sai_api_query(SAI_API_SWITCH,
                        (static_cast<void**>(static_cast<void*>(&sai_switch_api_table)))));

            ASSERT_TRUE(sai_switch_api_table != NULL);

            EXPECT_TRUE(sai_switch_api_table->create_switch != NULL);
            EXPECT_TRUE(sai_switch_api_table->remove_switch != NULL);
            EXPECT_TRUE(sai_switch_api_table->set_switch_attribute != NULL);
            EXPECT_TRUE(sai_switch_api_table->get_switch_attribute != NULL);

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


            EXPECT_EQ (SAI_STATUS_SUCCESS,
                    (sai_switch_api_table->create_switch (&switch_id , attr_count,
                                                          sai_attr_set)));


            ASSERT_EQ(SAI_STATUS_SUCCESS,sai_api_query(SAI_API_POLICER,
                        (static_cast<void**>(static_cast<void*>(&sai_policer_api_table)))));

            ASSERT_TRUE(sai_policer_api_table != NULL);

            EXPECT_TRUE(sai_policer_api_table->create_policer != NULL);
            EXPECT_TRUE(sai_policer_api_table->remove_policer  != NULL);
            EXPECT_TRUE(sai_policer_api_table->set_policer_attribute != NULL);
            EXPECT_TRUE(sai_policer_api_table->get_policer_attribute != NULL);

            ASSERT_EQ(NULL,sai_api_query(SAI_API_PORT,
                        (static_cast<void**>(static_cast<void*>(&sai_port_api_table)))));

            ASSERT_TRUE(sai_port_api_table != NULL);

            EXPECT_TRUE(sai_port_api_table->set_port_attribute != NULL);
            EXPECT_TRUE(sai_port_api_table->get_port_attribute != NULL);
            EXPECT_TRUE(sai_port_api_table->get_port_stats != NULL);

            EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query (SAI_API_ACL,
                        (static_cast<void**>(static_cast<void*>(&sai_acl_api_table)))));

        }


        static sai_switch_api_t* sai_switch_api_table;
        static sai_policer_api_t* sai_policer_api_table;
        static sai_port_api_t* sai_port_api_table;
        static sai_acl_api_t* sai_acl_api_table;
        static const unsigned int test_port_id = 30;
        static const unsigned int test_port_id_1 = 31;
};

sai_switch_api_t* policer ::sai_switch_api_table = NULL;
sai_policer_api_t* policer ::sai_policer_api_table = NULL;
sai_port_api_t* policer::sai_port_api_table = NULL;
sai_acl_api_t* policer::sai_acl_api_table = NULL;

/* Create a storm control policer.
 * Get attributes.
 * Apply on port and remove it.
 */
TEST_F(policer, storm_control)
{
    sai_attribute_t new_attr_list[5];
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[3];
    sai_object_id_t policer_id = SAI_NULL_OBJECT_ID;
    unsigned int attr_count = 0;

    new_attr_list[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
    new_attr_list[attr_count].value.s32 = 0;

    attr_count ++;

    new_attr_list[attr_count].id = SAI_POLICER_ATTR_MODE;
    new_attr_list[attr_count].value.s32 = SAI_POLICER_MODE_STORM_CONTROL ;
    attr_count ++;


    new_attr_list[attr_count].id = SAI_POLICER_ATTR_PIR;
    new_attr_list[attr_count].value.u64 = 200;

    attr_count ++;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_policer_api_table->create_policer
              (&policer_id, switch_id, attr_count, (const sai_attribute_t *)new_attr_list));



    set_attr.id = SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID;
    set_attr.value.oid = policer_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID;
    set_attr.value.oid = policer_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    set_attr.id = SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID;
    set_attr.value.oid = policer_id;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_PIR;
    set_attr.value.u64 = 400;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    attr_count = 0;
    get_attr[attr_count].id = SAI_POLICER_ATTR_PIR;
    attr_count ++;

    get_attr[attr_count].id = SAI_POLICER_ATTR_MODE;
    attr_count ++;

    get_attr[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->
              get_policer_attribute(policer_id, attr_count,
                                    get_attr));

    printf("PIR value is 0x%" PRIx64 " mode %d type %d\r\n",get_attr[0].value.u64,
           get_attr[1].value.s32, get_attr[2].value.s32);
    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,sai_policer_api_table->remove_policer(policer_id));

    set_attr.id = SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,sai_policer_api_table->remove_policer(policer_id));

    set_attr.id = SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,sai_policer_api_table->remove_policer(policer_id));


    set_attr.id = SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_port_api_table->set_port_attribute
              (sai_qos_port_id_get(test_port_id), (const sai_attribute_t *)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->remove_policer(policer_id));
}

/*
 * Invalid attribute tests for storm control policer.
 */
TEST_F(policer, storm_control_invalid_attr)
{
    sai_attribute_t new_attr_list[5];
    sai_object_id_t policer_id1 = SAI_NULL_OBJECT_ID;
    unsigned int attr_count = 0;

    new_attr_list[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
    new_attr_list[attr_count].value.s32 = 0;

    attr_count ++;

    new_attr_list[attr_count].id = SAI_POLICER_ATTR_MODE;
    new_attr_list[attr_count].value.s32 = SAI_POLICER_MODE_STORM_CONTROL ;
    attr_count ++;


    new_attr_list[attr_count].id = SAI_POLICER_ATTR_PIR;
    new_attr_list[attr_count].value.u64 = 200;

    attr_count ++;

    new_attr_list[attr_count].id = SAI_POLICER_ATTR_PBS;
    new_attr_list[attr_count].value.u64 = 200;

    attr_count ++;
    ASSERT_EQ(SAI_STATUS_INVALID_PARAMETER, sai_policer_api_table->create_policer
              (&policer_id1, switch_id, attr_count, (const sai_attribute_t *)new_attr_list));
}

/*
 * Mandatory attribute missing for policer create.
 */
TEST_F(policer, mandatory_attr_missing)
{
    sai_attribute_t new_attr_list[5];
    sai_object_id_t policer_id1 = SAI_NULL_OBJECT_ID;
    unsigned int attr_count = 0;

    new_attr_list[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
    new_attr_list[attr_count].value.s32 = 0;

    attr_count ++;

    new_attr_list[attr_count].id = SAI_POLICER_ATTR_MODE;
    new_attr_list[attr_count].value.s32 = SAI_POLICER_MODE_TR_TCM;
    attr_count ++;

    new_attr_list[attr_count].id = SAI_POLICER_ATTR_PBS;
    new_attr_list[attr_count].value.u64 = 200;

    attr_count ++;
    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING, sai_policer_api_table->create_policer
              (&policer_id1, switch_id, attr_count, (const sai_attribute_t *)new_attr_list));
}

/*
 * Create a policer.
 * Apply on acl.
 * Replace with a new policer.
 * Modify and get attributes
 */
TEST_F(policer, acl)
{
    sai_attribute_t attr_list[10];
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[3];
    unsigned int attr_count = 0;
    unsigned int idx = 0;
    sai_object_id_t acl_table_id;
    sai_object_id_t acl_rule_id;
    sai_object_id_t policer_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t policer_id1 = SAI_NULL_OBJECT_ID;
    sai_attribute_t rule_attr[10] = {0};
    sai_attribute_t table_attr[12] = {0};
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    attr_list[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
    attr_list[attr_count].value.s32 = 0;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_CIR;
    attr_list[attr_count].value.u64 = 100;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_MODE;
    attr_list[attr_count].value.s32 = SAI_POLICER_MODE_TR_TCM;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_PIR;
    attr_list[attr_count].value.u64 = 0;

    attr_count ++;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_policer_api_table->create_policer
              (&policer_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    attr_count  = 0;
    attr_list[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
    attr_list[attr_count].value.s32 = 0;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_CIR;
    attr_list[attr_count].value.u64 = 100;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_MODE;
    attr_list[attr_count].value.s32 = SAI_POLICER_MODE_TR_TCM;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_PIR;
    attr_list[attr_count].value.u64 = 500;

    attr_count ++;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_policer_api_table->create_policer
              (&policer_id1, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    table_attr[0].id = SAI_ACL_TABLE_ATTR_ACL_STAGE;
    table_attr[0].value.s32= 0;
    table_attr[1].id =  SAI_ACL_TABLE_ATTR_PRIORITY;
    table_attr[1].value.u32 = 1;
    table_attr[2].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
    table_attr[3].id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
    table_attr[4].id = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
    table_attr[5].id = SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE;
    table_attr[6].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID;
    table_attr[7].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI;
    table_attr[8].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI;
    table_attr[9].id = SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
    table_attr[10].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    table_attr[11].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    sai_rc = sai_acl_api_table->create_acl_table (&acl_table_id, switch_id, 12,
                                                  table_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    rule_attr[0].value.oid = acl_table_id;
    rule_attr[1].id =  SAI_ACL_ENTRY_ATTR_PRIORITY;
    rule_attr[1].value.u32 = 1;
    rule_attr[2].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    rule_attr[2].value.booldata= true;
    rule_attr[3].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    rule_attr[3].value.aclfield.data.objlist.count = 1;
    rule_attr[3].value.aclfield.data.objlist.list = (sai_object_id_t *) calloc(
                                                            1, sizeof(sai_object_id_t));
    rule_attr[3].value.aclfield.data.objlist.list[0] = sai_qos_port_id_get(test_port_id);
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
    rule_attr[4].value.aclaction.enable = false;
    rule_attr[4].value.aclaction.parameter.oid = policer_id;
    sai_rc = sai_acl_api_table->create_acl_entry (&acl_rule_id, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);


    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_CIR;
    set_attr.value.u64 = 400;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_rc);


    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_YELLOW_PACKET_ACTION;
    set_attr.value.s32 = SAI_PACKET_ACTION_FORWARD;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));


    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_RED_PACKET_ACTION;;
    set_attr.value.s32 = SAI_PACKET_ACTION_DROP;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));


    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_GREEN_PACKET_ACTION;
    set_attr.value.s32 = SAI_PACKET_ACTION_FORWARD;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
    rule_attr[0].value.aclaction.enable = true;
    rule_attr[0].value.aclaction.parameter.oid = policer_id;
    sai_rc = sai_acl_api_table->set_acl_entry_attribute(acl_rule_id, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
    rule_attr[0].value.aclaction.enable = true;
    rule_attr[0].value.aclaction.parameter.oid = policer_id1;
    sai_rc = sai_acl_api_table->set_acl_entry_attribute(acl_rule_id, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_RED_PACKET_ACTION;;
    set_attr.value.s32 = SAI_PACKET_ACTION_DROP;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id1, (const sai_attribute_t *)&set_attr));

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_RED_PACKET_ACTION;;
    set_attr.value.s32 = SAI_PACKET_ACTION_FORWARD;
    attr_count ++;


    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id1, (const sai_attribute_t *)&set_attr));
    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_CIR;
    set_attr.value.u64 = 545;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id1, (const sai_attribute_t *)&set_attr));

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
    rule_attr[0].value.aclaction.enable = false;
    rule_attr[0].value.aclaction.parameter.oid = policer_id1;
    sai_rc = sai_acl_api_table->set_acl_entry_attribute(acl_rule_id, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr_count = 0;
    get_attr[attr_count].id = SAI_POLICER_ATTR_RED_PACKET_ACTION;
    attr_count ++;

    get_attr[attr_count].id = SAI_POLICER_ATTR_YELLOW_PACKET_ACTION;
    attr_count ++;

    get_attr[attr_count].id = SAI_POLICER_ATTR_GREEN_PACKET_ACTION;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->
              get_policer_attribute(policer_id1, attr_count,
                                    get_attr));

    for(idx = 0; idx < attr_count; idx ++){
        printf("Id %d action %d",get_attr[idx].id,
               get_attr[idx].value.s32);
    }


    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_CIR;
    set_attr.value.u64 = 800;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id1, (const sai_attribute_t *)&set_attr));

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
    rule_attr[0].value.aclaction.enable = true;
    rule_attr[0].value.aclaction.parameter.oid = policer_id1;
    sai_rc = sai_acl_api_table->set_acl_entry_attribute(acl_rule_id, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
    rule_attr[0].value.aclaction.enable = true;
    rule_attr[0].value.aclaction.parameter.oid = policer_id1;
    sai_rc = sai_acl_api_table->set_acl_entry_attribute(acl_rule_id, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);


    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,sai_policer_api_table->remove_policer(policer_id1));


    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
    rule_attr[0].value.aclaction.enable = false;
    rule_attr[0].value.aclaction.parameter.oid = policer_id1;
    sai_rc = sai_acl_api_table->set_acl_entry_attribute(acl_rule_id, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->remove_policer(policer_id1));
    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->remove_policer(policer_id));

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_acl_api_table->remove_acl_entry(acl_rule_id));
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_acl_api_table->remove_acl_table(acl_table_id));
    free (rule_attr[3].value.aclfield.data.objlist.list);
}

/*
 * Create trtcm policer and apply on acl.
 */
TEST_F(policer, trtcm_acl)
{
    sai_attribute_t attr_list[10];
    sai_attribute_t set_attr;
    unsigned int attr_count = 0;
    sai_object_id_t acl_table_id;
    sai_object_id_t acl_rule_id;
    sai_object_id_t policer_id = SAI_NULL_OBJECT_ID;
    sai_attribute_t rule_attr[10] = {0};
    sai_attribute_t table_attr[12] = {0};
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    attr_list[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
    attr_list[attr_count].value.s32 = 0;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_CIR;
    attr_list[attr_count].value.u64 = 1000;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_MODE;
    attr_list[attr_count].value.s32 = SAI_POLICER_MODE_TR_TCM;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_PIR;
    attr_list[attr_count].value.u64 = 2000;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_RED_PACKET_ACTION;
    attr_list[attr_count].value.s32 = SAI_PACKET_ACTION_DROP;

    attr_count ++;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_policer_api_table->create_policer
              (&policer_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    table_attr[0].id = SAI_ACL_TABLE_ATTR_ACL_STAGE;
    table_attr[0].value.s32= 0;
    table_attr[1].id =  SAI_ACL_TABLE_ATTR_PRIORITY;
    table_attr[1].value.u32 = 1;
    table_attr[2].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
    table_attr[3].id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
    table_attr[4].id = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
    table_attr[5].id = SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE;
    table_attr[6].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID;
    table_attr[7].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI;
    table_attr[8].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI;
    table_attr[9].id = SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
    table_attr[10].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    table_attr[11].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    sai_rc = sai_acl_api_table->create_acl_table (&acl_table_id, switch_id, 12,
                                                  table_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    rule_attr[0].value.oid = acl_table_id;
    rule_attr[1].id =  SAI_ACL_ENTRY_ATTR_PRIORITY;
    rule_attr[1].value.u32 = 1;
    rule_attr[2].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    rule_attr[2].value.booldata= true;
    rule_attr[3].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    rule_attr[3].value.aclfield.data.objlist.count = 1;
    rule_attr[3].value.aclfield.data.objlist.list = (sai_object_id_t *) calloc(
                                                            1, sizeof(sai_object_id_t));
    rule_attr[3].value.aclfield.data.objlist.list[0] = sai_qos_port_id_get(test_port_id_1);
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
    rule_attr[4].value.aclaction.enable = true;
    rule_attr[4].value.aclaction.parameter.oid = policer_id;
    sai_rc = sai_acl_api_table->create_acl_entry (&acl_rule_id, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_YELLOW_PACKET_ACTION;;
    set_attr.value.s32 = SAI_PACKET_ACTION_DROP;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_GREEN_PACKET_ACTION;;
    set_attr.value.s32 = SAI_PACKET_ACTION_DROP;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_GREEN_PACKET_ACTION;;
    set_attr.value.s32 = SAI_PACKET_ACTION_FORWARD;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_YELLOW_PACKET_ACTION;;
    set_attr.value.s32 = SAI_PACKET_ACTION_FORWARD;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_RED_PACKET_ACTION;;
    set_attr.value.s32 = SAI_PACKET_ACTION_FORWARD;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_CIR;
    set_attr.value.u64 = 1500;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
    rule_attr[0].value.aclaction.enable = false;
    rule_attr[0].value.aclaction.parameter.oid = policer_id;
    sai_rc = sai_acl_api_table->set_acl_entry_attribute(acl_rule_id, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->remove_policer(policer_id));

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_acl_api_table->remove_acl_entry(acl_rule_id));
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_acl_api_table->remove_acl_table(acl_table_id));
    free (rule_attr[3].value.aclfield.data.objlist.list);

}


/*
 * Create srtcm policer and apply on acl.
 */
TEST_F(policer, srtcm_acl)
{
    sai_attribute_t attr_list[10];
    sai_attribute_t set_attr;
    unsigned int attr_count = 0;
    sai_object_id_t acl_table_id;
    sai_object_id_t acl_rule_id;
    sai_object_id_t policer_id = SAI_NULL_OBJECT_ID;
    sai_attribute_t rule_attr[10] = {0};
    sai_attribute_t table_attr[12] = {0};
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    attr_list[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
    attr_list[attr_count].value.s32 = 0;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_CIR;
    attr_list[attr_count].value.u64 = 1500;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_MODE;
    attr_list[attr_count].value.s32 = SAI_POLICER_MODE_SR_TCM;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_CBS;
    attr_list[attr_count].value.u64 = 2000;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_PBS;
    attr_list[attr_count].value.u64 = 1000;

    attr_count ++;

    attr_list[attr_count].id = SAI_POLICER_ATTR_RED_PACKET_ACTION;
    attr_list[attr_count].value.s32 = SAI_PACKET_ACTION_DROP;

    attr_count ++;
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_policer_api_table->create_policer
              (&policer_id, switch_id, attr_count, (const sai_attribute_t *)attr_list));

    table_attr[0].id = SAI_ACL_TABLE_ATTR_ACL_STAGE;
    table_attr[0].value.s32= 0;
    table_attr[1].id =  SAI_ACL_TABLE_ATTR_PRIORITY;
    table_attr[1].value.u32 = 1;
    table_attr[2].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
    table_attr[3].id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
    table_attr[4].id = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
    table_attr[5].id = SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE;
    table_attr[6].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID;
    table_attr[7].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI;
    table_attr[8].id = SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI;
    table_attr[9].id = SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
    table_attr[10].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    table_attr[11].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    sai_rc = sai_acl_api_table->create_acl_table (&acl_table_id, switch_id, 12,
                                                  table_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    rule_attr[0].value.oid = acl_table_id;
    rule_attr[1].id =  SAI_ACL_ENTRY_ATTR_PRIORITY;
    rule_attr[1].value.u32 = 1;
    rule_attr[2].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    rule_attr[2].value.booldata= true;
    rule_attr[3].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    rule_attr[3].value.aclfield.data.objlist.count = 1;
    rule_attr[3].value.aclfield.data.objlist.list = (sai_object_id_t *) calloc(
                                                            1, sizeof(sai_object_id_t));
    rule_attr[3].value.aclfield.data.objlist.list[0] = sai_qos_port_id_get(test_port_id_1);
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
    rule_attr[4].value.aclaction.enable = true;
    rule_attr[4].value.aclaction.parameter.oid = policer_id;
    sai_rc = sai_acl_api_table->create_acl_entry (&acl_rule_id, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_YELLOW_PACKET_ACTION;;
    set_attr.value.s32 = SAI_PACKET_ACTION_DROP;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_GREEN_PACKET_ACTION;;
    set_attr.value.s32 = SAI_PACKET_ACTION_DROP;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_GREEN_PACKET_ACTION;;
    set_attr.value.s32 = SAI_PACKET_ACTION_FORWARD;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_YELLOW_PACKET_ACTION;;
    set_attr.value.s32 = SAI_PACKET_ACTION_FORWARD;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_RED_PACKET_ACTION;;
    set_attr.value.s32 = SAI_PACKET_ACTION_FORWARD;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    attr_count = 0;
    set_attr.id = SAI_POLICER_ATTR_CIR;
    set_attr.value.u64 = 1500;
    attr_count ++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->set_policer_attribute
              (policer_id, (const sai_attribute_t *)&set_attr));

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
    rule_attr[0].value.aclaction.enable = false;
    rule_attr[0].value.aclaction.parameter.oid = policer_id;
    sai_rc = sai_acl_api_table->set_acl_entry_attribute(acl_rule_id, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_policer_api_table->remove_policer(policer_id));

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_acl_api_table->remove_acl_entry(acl_rule_id));
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_acl_api_table->remove_acl_table(acl_table_id));
    free (rule_attr[3].value.aclfield.data.objlist.list);

}
int main (int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

