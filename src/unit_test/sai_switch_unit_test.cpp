/*
 * filename: std_cfg_file_gtest.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/*
 * sai_switch_unit_test.cpp
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include "sai_port_breakout_test_utils.h"


extern "C" {
#include "sai.h"
#include <inttypes.h>
}
#define SAI_MAX_PORTS 256


static sai_object_id_t cpu_port = 0;
static sai_object_id_t switch_id = 0;

#define LOG_PRINT(msg, ...) \
    printf(msg, ##__VA_ARGS__)

#include <iostream>
#include <map>

typedef std::map<std::string, std::string> sai_kv_pair_t;
typedef std::map<std::string, std::string>::iterator kv_iter;
static sai_kv_pair_t kvpair;

const char* profile_get_value(sai_switch_profile_id_t profile_id,
                                 const char* variable)
{
    kv_iter kviter;
    std::string key;

    if (variable ==  NULL)
        return NULL;

    key = variable;

    kviter = kvpair.find(key);
    if (kviter == kvpair.end()) {
        return NULL;
    }
    return kviter->second.c_str();
}

int profile_get_next_value(sai_switch_profile_id_t profile_id,
                           const char** variable,
                           const char** value)
{
    kv_iter kviter;
    std::string key;

    if (variable == NULL || value == NULL) {
        return -1;
    }
    if (*variable == NULL) {
            if (kvpair.size() < 1) {
            return -1;
        }
        kviter = kvpair.begin();
    } else {
        key = *variable;
        kviter = kvpair.find(key);
        if (kviter == kvpair.end()) {
            return -1;
        }
        kviter++;
        if (kviter == kvpair.end()) {
            return -1;
        }
    }
    *variable = (char *)kviter->first.c_str();
    *value = (char *)kviter->second.c_str();
    return 0;
}


void kv_populate(void)
{
    kvpair["SAI_FDB_TABLE_SIZE"] = "32768";
    kvpair["SAI_L3_NEIGHBOR_TABLE_SIZE"] = "16384";
    kvpair["SAI_L3_ROUTE_TABLE_SIZE"] = "131072";
    kvpair["SAI_NUM_CPU_QUEUES"] = "43";
    kvpair["SAI_INIT_CONFIG_FILE"] = "/etc/opx/sai/init.xml";
    kvpair["SAI_NUM_ECMP_MEMBERS"] = "64";
}

/*
 * Pass the service method table and do API intialize.
 */
TEST(sai_unit_test, api_init)
{
    sai_service_method_table_t  sai_service_method_table;
    kv_populate();

    sai_service_method_table.profile_get_value = profile_get_value;
    sai_service_method_table.profile_get_next_value = profile_get_next_value;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_api_initialize(0, &sai_service_method_table));
}
/* Callback functions to be passed during SDK init.
*/

/*Port state change callback.
*/
void sai_port_state_evt_callback(uint32_t count,
                                 sai_port_oper_status_notification_t *data)
{
    uint32_t port_idx = 0;
    for(port_idx = 0; port_idx < count; port_idx++) {
        LOG_PRINT("port 0x%" PRIx64 " State callback: Link state is %d\r\n",
                  data[port_idx].port_id, data[port_idx].port_state);
    }
}

/*FDB event callback.
*/
void sai_fdb_evt_callback(uint32_t count, sai_fdb_event_notification_data_t *data)
{
}
/*Switch operstate callback.
*/
void sai_switch_operstate_callback(sai_switch_oper_status_t switchstate)
{
    LOG_PRINT("Switch Operation state is %d\r\n", switchstate);
}

/* Packet event callback
 */
static inline void sai_packet_event_callback (const void *buffer,
                                              sai_size_t buffer_size,
                                              uint32_t attr_count,
                                              const sai_attribute_t *attr_list)
{
}

/*
 * Switch shutdown callback.
 */
void  sai_switch_shutdown_callback()
{
}

/*
 * NPU SDK init sequence.
 */
void sdkinit()
{
    sai_switch_api_t* sai_switch_api = NULL;
    sai_service_method_table_t  sai_service_method_table;
    kv_populate();

    sai_service_method_table.profile_get_value = profile_get_value;
    sai_service_method_table.profile_get_next_value = profile_get_next_value;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_api_initialize(0, &sai_service_method_table));

    sai_api_query(SAI_API_SWITCH,
                  (static_cast<void**>
                   (static_cast<void*>(&sai_switch_api))));


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

    ASSERT_TRUE(sai_switch_api != NULL);
    ASSERT_TRUE(sai_switch_api->create_switch != NULL);

    EXPECT_EQ (SAI_STATUS_SUCCESS,
               (sai_switch_api->create_switch (&switch_id , attr_count,
                                                         sai_attr_set)));

}


/*
 * API query is done while running the first test case and
 * the method table is stored in sai_switch_api_table so
 * that its available for the rest of the test cases which
 * use the method table
 */
class switchInit : public ::testing::Test
{
    public:
        bool sai_miss_action_test(sai_switch_attr_t attr, sai_packet_action_t action);
        bool sai_switch_max_port_get(uint32_t *max_port);

    protected:
        static void SetUpTestCase()
        {
            ASSERT_EQ(NULL,sai_api_query(SAI_API_SWITCH,
                                         (static_cast<void**>(static_cast<void*>(&sai_switch_api_table)))));

            ASSERT_TRUE(sai_switch_api_table != NULL);

            EXPECT_TRUE(sai_switch_api_table->create_switch != NULL);
            EXPECT_TRUE(sai_switch_api_table->remove_switch != NULL);
            EXPECT_TRUE(sai_switch_api_table->set_switch_attribute != NULL);
            EXPECT_TRUE(sai_switch_api_table->get_switch_attribute != NULL);

            sdkinit();

            ASSERT_EQ(NULL,sai_api_query(SAI_API_PORT,
                                         (static_cast<void**>(static_cast<void*>(&sai_port_api_table)))));

            ASSERT_TRUE(sai_port_api_table != NULL);

            EXPECT_TRUE(sai_port_api_table->set_port_attribute != NULL);
            EXPECT_TRUE(sai_port_api_table->get_port_attribute != NULL);
            EXPECT_TRUE(sai_port_api_table->get_port_stats != NULL);

            /* Get and update the CPU port */
            sai_attribute_t get_attr;
            memset(&get_attr, 0, sizeof(get_attr));
            get_attr.id = SAI_SWITCH_ATTR_CPU_PORT;

            EXPECT_EQ(SAI_STATUS_SUCCESS,
                      sai_switch_api_table->get_switch_attribute(switch_id,1, &get_attr));

            cpu_port = get_attr.value.oid;
        }

        static sai_switch_api_t* sai_switch_api_table;
        static sai_port_api_t* sai_port_api_table;
};

sai_switch_api_t* switchInit ::sai_switch_api_table = NULL;
sai_port_api_t* switchInit ::sai_port_api_table = NULL;

/*
 * Get the values for the list of attribute passed.
 */
TEST_F(switchInit, attr_get)
{
    sai_attribute_t sai_attr[3];

    sai_attr[0].id = SAI_SWITCH_ATTR_SWITCHING_MODE;
    sai_attr[0].value.s32 = SAI_SWITCH_SWITCHING_MODE_CUT_THROUGH;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->set_switch_attribute(switch_id,
                                    (const sai_attribute_t*)&sai_attr[0]));

    sai_attr[0].id = SAI_SWITCH_ATTR_SWITCHING_MODE;

    ASSERT_EQ(SAI_STATUS_SUCCESS,sai_switch_api_table->
              get_switch_attribute(switch_id,1,&sai_attr[0]));

    ASSERT_EQ(SAI_SWITCH_SWITCHING_MODE_CUT_THROUGH,sai_attr[0].value.s32);
}

/*
 * Get the switch oper state
 */
TEST_F(switchInit, oper_status_get)
{
    sai_attribute_t get_attr;

    get_attr.id = SAI_SWITCH_ATTR_OPER_STATUS;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1, &get_attr));

    ASSERT_EQ(SAI_SWITCH_OPER_STATUS_UP, get_attr.value.s32);
}

/*
 * Get the current switch maximum temperature
 */
TEST_F(switchInit, temp_get)
{
    sai_attribute_t get_attr;

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_MAX_TEMP;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1, &get_attr));

    LOG_PRINT("Switch current max temperature is %d \r\n", get_attr.value.s32);
}

/*
 * Get the maximum ports count
 */
TEST_F(switchInit, max_port_get)
{
    sai_attribute_t get_attr;

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_PORT_NUMBER;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1, &get_attr));

    LOG_PRINT("Maximum port number in the switch is %d \r\n", get_attr.value.u32);
}

/*
 * Get the CPU port
 */
TEST_F(switchInit, cpu_port_get)
{
    sai_attribute_t get_attr;

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_CPU_PORT;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1, &get_attr));

    cpu_port = get_attr.value.oid;
    LOG_PRINT("CPU port obj id is 0x%" PRIx64 " \r\n", get_attr.value.oid);
}

/*
 * Validates if the CPU port default VLAN can be set to 100.
 * UT PASS case: port should be set with default VLAN as 100
 */
TEST_F(switchInit, cpu_default_vlan_set_get)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;
    sai_vlan_id_t vlan_id = 100;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_PORT_VLAN_ID;
    sai_attr_set.value.u16 = vlan_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(cpu_port, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_PORT_VLAN_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(cpu_port, 1, &sai_attr_get));

    ASSERT_EQ(vlan_id, sai_attr_get.value.u16);
}

/*
 * Validates if the CPU port default VLAN priority can be set to 1.
 * UT PASS case: port should be set with default VLAN priority as 1.
 */
TEST_F(switchInit, cpu_default_vlan_prio_set_get)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY;
    sai_attr_set.value.u8 = 1;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(cpu_port, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(cpu_port, 1, &sai_attr_get));

    ASSERT_EQ(1, sai_attr_get.value.u8);
}

/*
 * Validates if the CPU port ingress filter can be enabled
 * UT PASS case: port ingress filter should get enabled
 */
TEST_F(switchInit, cpu_ingress_filter_set_get)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY;
    sai_attr_set.value.booldata = true;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(cpu_port, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(cpu_port, 1, &sai_attr_get));

    ASSERT_EQ(true, sai_attr_get.value.booldata);
}

/*
 * Validates if the CPU port drop untagged capability can be enabled
 * UT PASS case: port drop untagged capability should get enabled
 */
TEST_F(switchInit, cpu_drop_untagged_set_get)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_DROP_UNTAGGED;
    sai_attr_set.value.booldata = true;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(cpu_port, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_DROP_UNTAGGED;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(cpu_port, 1, &sai_attr_get));

    ASSERT_TRUE(sai_attr_get.value.booldata);
}

/*
 * Validates if the CPU port drop tagged capability can be enabled
 * UT PASS case: port drop tagged capability should get enabled
 */
TEST_F(switchInit, cpu_drop_tagged_set_get)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_DROP_TAGGED;
    sai_attr_set.value.booldata = true;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(cpu_port, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_DROP_TAGGED;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(cpu_port, 1, &sai_attr_get));

    ASSERT_TRUE(sai_attr_get.value.booldata);
}


/* Switch Max port get - returns the maximum ports in the switch */
bool switchInit::sai_switch_max_port_get(uint32_t *max_port)
{
    sai_attribute_t sai_get_attr;

    memset(&sai_get_attr, 0, sizeof(sai_attribute_t));
    sai_get_attr.id = SAI_SWITCH_ATTR_PORT_NUMBER;

    if(sai_switch_api_table->get_switch_attribute(switch_id,1, &sai_get_attr) != SAI_STATUS_SUCCESS) {
        return false;
    }

    *max_port = sai_get_attr.value.u32;

    return true;
}

/*
 * Get and print the list of valid logical ports in the switch
 */
TEST_F(switchInit, attr_port_list_get)
{
    sai_attribute_t get_attr;
    uint32_t count = 0;
    sai_status_t ret = SAI_STATUS_FAILURE;

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_PORT_LIST;

    get_attr.value.objlist.count = 1;
    get_attr.value.objlist.list = (sai_object_id_t *) calloc(1,
                                                             sizeof(sai_object_id_t));
    ASSERT_TRUE(get_attr.value.objlist.list != NULL);

    ret = sai_switch_api_table->get_switch_attribute(switch_id,1, &get_attr);

    /* Handles buffer overflow case, by reallocating required memory */
    if(ret == SAI_STATUS_BUFFER_OVERFLOW ) {
        free(get_attr.value.objlist.list);
        get_attr.value.objlist.list = (sai_object_id_t *)calloc (get_attr.value.objlist.count,
                                                                 sizeof(sai_object_id_t));
        ret = sai_switch_api_table->get_switch_attribute(switch_id,1, &get_attr);
    }

    if(ret != SAI_STATUS_SUCCESS) {
        free(get_attr.value.objlist.list);
        ASSERT_EQ(SAI_STATUS_SUCCESS, ret);
    } else {

        LOG_PRINT("Port list count is %d \r\n", get_attr.value.objlist.count);

        for(count = 0; count < get_attr.value.objlist.count; count++) {
            LOG_PRINT("Port list id %d is %lu \r\n", count, get_attr.value.objlist.list[count]);
        }

        free(get_attr.value.objlist.list);
    }
}

/*
 * Test attribute counter thread refresh interval
 */
TEST_F(switchInit, attr_counter_refresh_interval_get)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr;

    memset(&set_attr, 0, sizeof(set_attr));
    set_attr.id = SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL;
    set_attr.value.u32 = 100;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->set_switch_attribute(switch_id,(const sai_attribute_t*)&set_attr));

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1,&get_attr));

    ASSERT_EQ(get_attr.value.u32, set_attr.value.u32);

    /* HW based counter statistics read */
    set_attr.value.u32 = 0;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->set_switch_attribute(switch_id,(const sai_attribute_t*)&set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1,&get_attr));

    ASSERT_EQ(get_attr.value.u32, set_attr.value.u32);
}

/*
 * Test attribute broadcast flood enable
 */
TEST_F(switchInit, attr_bcast_flood_enable)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr;

    memset(&set_attr, 0, sizeof(set_attr));
    set_attr.id = SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE;
    set_attr.value.booldata = true;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->set_switch_attribute(switch_id,(const sai_attribute_t*)&set_attr));

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1,&get_attr));

    ASSERT_EQ(get_attr.value.booldata,set_attr.value.booldata);
}

/*
 * Test attribute multicast flood enable
 */
TEST_F(switchInit, attr_mcast_flood_enable)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr;

    memset(&set_attr, 0, sizeof(set_attr));
    set_attr.id = SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE;
    set_attr.value.booldata = true;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->set_switch_attribute(switch_id,(const sai_attribute_t*)&set_attr));

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1,&get_attr));

    ASSERT_EQ(get_attr.value.booldata,set_attr.value.booldata);
}

/*
 * Test attribute max learned address
 */
TEST_F(switchInit, attr_max_learned_addresses)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr;

    memset(&set_attr, 0, sizeof(set_attr));
    set_attr.id = SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES;
    set_attr.value.u32 = 100;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->set_switch_attribute(switch_id,(const sai_attribute_t*)&set_attr));

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1,&get_attr));

    ASSERT_EQ(get_attr.value.u32, set_attr.value.u32);
}

/*
 * Test attribute FDB Aging time
 */
TEST_F(switchInit, attr_fdb_aging_time)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr;

    memset(&set_attr, 0, sizeof(set_attr));
    set_attr.id = SAI_SWITCH_ATTR_FDB_AGING_TIME;
    set_attr.value.u32 = 10;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->set_switch_attribute(switch_id,(const sai_attribute_t*)&set_attr));

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_FDB_AGING_TIME;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1,&get_attr));

    ASSERT_EQ(get_attr.value.u32, set_attr.value.u32);
}

bool switchInit::sai_miss_action_test(sai_switch_attr_t attr, sai_packet_action_t action)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr;

    memset(&set_attr, 0, sizeof(set_attr));
    set_attr.id = attr;
    set_attr.value.s32 = action;
    if(sai_switch_api_table->set_switch_attribute(switch_id,
                                                  (const sai_attribute_t*)&set_attr) != SAI_STATUS_SUCCESS) {
        return false;
    }

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = attr;

    if(sai_switch_api_table->get_switch_attribute(switch_id,1,&get_attr)!= SAI_STATUS_SUCCESS) {
        return false;
    }

    if(get_attr.value.s32 != set_attr.value.s32) {
        return false;
    }
    return true;
}
/*
 * Test attributes miss actions
 */
TEST_F(switchInit, attr_miss_action)
{
    EXPECT_TRUE(sai_miss_action_test(SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION,SAI_PACKET_ACTION_FORWARD));
    EXPECT_TRUE(sai_miss_action_test(SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION,SAI_PACKET_ACTION_TRAP));
    EXPECT_TRUE(sai_miss_action_test(SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION,SAI_PACKET_ACTION_LOG));
    EXPECT_TRUE(sai_miss_action_test(SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION,SAI_PACKET_ACTION_DROP));

    EXPECT_TRUE(sai_miss_action_test(SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION,SAI_PACKET_ACTION_FORWARD));
    EXPECT_TRUE(sai_miss_action_test(SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION,SAI_PACKET_ACTION_TRAP));
    EXPECT_TRUE(sai_miss_action_test(SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION,SAI_PACKET_ACTION_LOG));
    EXPECT_TRUE(sai_miss_action_test(SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION,SAI_PACKET_ACTION_DROP));

    EXPECT_TRUE(sai_miss_action_test(SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION,SAI_PACKET_ACTION_FORWARD));
    EXPECT_TRUE(sai_miss_action_test(SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION,SAI_PACKET_ACTION_TRAP));
    EXPECT_TRUE(sai_miss_action_test(SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION,SAI_PACKET_ACTION_LOG));
    EXPECT_TRUE(sai_miss_action_test(SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION,SAI_PACKET_ACTION_DROP));
}

/*
 * Get the maximum LAG number and max LAG members
 */
TEST_F(switchInit, max_lag_attr)
{
    sai_attribute_t get_attr;

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_NUMBER_OF_LAGS;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              sai_switch_api_table->set_switch_attribute(switch_id,(const sai_attribute_t*)&get_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1, &get_attr));

    EXPECT_NE(get_attr.value.u32, 0);

    LOG_PRINT("Maximum lag number in the switch is %d \r\n", get_attr.value.u32);

    memset(&get_attr, 0, sizeof(get_attr));
    get_attr.id = SAI_SWITCH_ATTR_LAG_MEMBERS;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              sai_switch_api_table->set_switch_attribute(switch_id,(const sai_attribute_t*)&get_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1, &get_attr));

    EXPECT_NE(get_attr.value.u32, 0);

    LOG_PRINT("Maximum number of ports per LAG in the switch is %d \r\n", get_attr.value.u32);
}

TEST_F(switchInit, default_tc_set_get)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;
    uint32_t        tc = 7;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_SWITCH_ATTR_QOS_DEFAULT_TC;
    sai_attr_set.value.u32 = 7;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->set_switch_attribute(switch_id,&sai_attr_set));

    sai_attr_get.id = SAI_SWITCH_ATTR_QOS_DEFAULT_TC;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1, &sai_attr_get));

    ASSERT_EQ(tc, sai_attr_get.value.u32);
}

TEST_F(switchInit, max_port_mtu_get)
{
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_get.id = SAI_SWITCH_ATTR_PORT_MAX_MTU;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1, &sai_attr_get));

    EXPECT_NE(sai_attr_get.value.u32, 0);

    LOG_PRINT("Maximum PORT mtu is %d \r\n", sai_attr_get.value.u32);
}

