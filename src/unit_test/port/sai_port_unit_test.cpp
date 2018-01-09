/*
 * filename: sai_port_unit_test.cpp
 * (c) Copyright 2015 Dell Inc. All Rights Reserved.
 */

/*
 * SAI PORT UNIT TEST :- Covers the test cases for all Public API's in SAI PORT module.
 * It begins with default switch Init and covers all the physical switch ports,
 * Port Attribute Get/Set and port statistics counter get APIs.
 * As Google UT framework, doesn't support dynamic input at run time, all port related
 * functionalities wouldn't be covered in this UT.
 *
 * Validated Ports:
 * For few critical Attributes like Speed, Admin state this UT would test for all possible
 * valid ports in the switch; for the remaining attributes it would just test with port id 1.
 *
 * STATS Get:
 * As packets of all possible cannot be injected during the UT runtime, coverage of statistics UT
 * would be more of finding the STAT Id's supported by the NPU and validating the stats get API.
 *
 * Setup Environment for Port UT:
 * No specific setup/connection of ports is needed for the port UT to run.
 *
 * For port oper state get to succeed, internal loopback mode will be used,
 * as time taken for a link to come UP after Admin state set might vary depending on the NPU,
 * and would need adding some sleep in between these calls. Need to re-visit about adding a
 * sleep to validate the actual port Link status instead of using Internal Loopback.
 *
 */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include "sai_port_breakout_test_utils.h"
#include "sai_bridge_unit_test_utils.h"
extern "C" {
#include "sai.h"
#include "saiswitch.h"
#include "saiport.h"
#include "saitypes.h"
#include <inttypes.h>
}

static sai_object_id_t switch_id =0;

typedef struct sai_hw_port_info_t {
    uint32_t hw_lane;
    sai_object_id_t port_id;
    bool port_valid;
}sai_hw_port_info;

#define SAI_MAX_PORTS  256
#define SAI_EEE_DEFAULT_IDLE_TIME 2560
#define SAI_EEE_DEFAULT_WAKE_TIME 5
#define SAI_EEE_COPPER_PORT_MAX 48
#define SAI_PORT_FOUR_LANES 4
#define SAI_PORT_TWO_LANES 2
#define SAI_PORT_ONE_LANE 1
#define SAI_PORT_OUI_CODE_1 0x123456
#define SAI_PORT_OUI_CODE_2 0x654321
#define SAI_PORT_SPEED_HUNDRED_GIG 100000
#define SAI_PORT_SPEED_FIFTY_GIG 50000
#define SAI_PORT_SPEED_FORTY_GIG 40000
#define SAI_PORT_SPEED_TWENTY_FIVE_GIG 25000
#define SAI_PORT_SPEED_TEN_GIG 10000

static uint32_t port_count = 0;
static sai_hw_port_info port_info[SAI_MAX_PORTS] = {0};

/* global port id used for UT*/
static sai_object_id_t gport_id = 0;

#define SAI_PORT_CAP_SPEED_MAX 1
#define SAI_PORT_MAX_ARRAY_INDEX 252

#define LOG_PRINT(msg, ...) \
    printf(msg, ##__VA_ARGS__)

/* Callback function stubs to be passed during SDK init. They callbacks will not be
 * validated as part of this UT.
 */

/*Port state change callback.
*/
static inline void sai_port_state_evt_callback(uint32_t count,
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
static inline void sai_fdb_evt_callback(uint32_t count, sai_fdb_event_notification_data_t *data)
{
}

/*Switch operstate callback.
*/
static inline void sai_switch_operstate_callback(sai_switch_oper_status_t switchstate)
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

static inline sai_status_t sai_get_port_discard (sai_port_api_t *port_api_table,
                                                 sai_object_id_t port_id,
                                                 bool            is_untag,
                                                 bool           *value)
{
    sai_attribute_t attr;
    sai_status_t    rc;

    memset (&attr, 0, sizeof (sai_attribute_t));

    if (is_untag) {
        attr.id = SAI_PORT_ATTR_DROP_UNTAGGED;
    }
    else {
        attr.id = SAI_PORT_ATTR_DROP_TAGGED;
    }

    rc = port_api_table->get_port_attribute (port_id, 1, &attr);
    *value = attr.value.booldata;

    return rc;
}

static inline sai_status_t sai_set_port_discard (sai_port_api_t *port_api_table,
                                                 sai_object_id_t port_id,
                                                 bool            is_untag,
                                                 bool            value)
{
    sai_attribute_t attr;

    memset (&attr, 0, sizeof (sai_attribute_t));

    if (is_untag) {
        attr.id = SAI_PORT_ATTR_DROP_UNTAGGED;
    }
    else {
        attr.id = SAI_PORT_ATTR_DROP_TAGGED;
    }

    attr.value.booldata = value;

    return (port_api_table->set_port_attribute (port_id, &attr));
}

/*
 * API query is done while running the first test case and
 * the method table is stored in sai_port_api_table so
 * that its available for the rest of the test cases which
 * use the method table
 */
class portTest : public ::testing::Test
{
    public:
        void sai_port_supported_speed_check(sai_object_id_t port_id, uint32_t speed, bool *supported);
        bool sai_port_speed_set_get(sai_object_id_t port_id, uint32_t speed);
        bool sai_port_attr_set_get(sai_object_id_t port_id, sai_attribute_t *sai_attr);
        bool sai_internal_loopback_set_get(sai_object_id_t port_id,
                                           sai_port_internal_loopback_mode_t lb_mode);
        void sai_test_fdb_learn_mode(sai_port_fdb_learning_mode_t mode);
        void sai_port_create_speed_set(uint32_t speed);
        static void sai_get_port_hw_lane_map_table();
        void sai_check_breakout_mode_supported(sai_object_id_t port_id,
                    sai_port_breakout_mode_type_t mode, bool * supported);
        sai_status_t sai_set_flex_port_configuration(unsigned int port,
                                         sai_port_breakout_mode_type_t breakout_mode,
                                         uint32_t speed);

    protected:
        static void SetUpTestCase(void)
        {
            sai_attribute_t sai_attr_set[7];
            uint32_t attr_count = 7;

            memset(sai_attr_set,0, sizeof(sai_attr_set));
            /*
             * Query and populate the SAI Switch API Table.
             */
            EXPECT_EQ(SAI_STATUS_SUCCESS, sai_api_query
                    (SAI_API_SWITCH, (static_cast<void**>
                                      (static_cast<void*>(&sai_switch_api_table)))));

            ASSERT_TRUE(sai_switch_api_table != NULL);

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

            ASSERT_TRUE(sai_switch_api_table->create_switch != NULL);

            EXPECT_EQ (SAI_STATUS_SUCCESS,
                    (sai_switch_api_table->create_switch (&switch_id , attr_count,
                                                          sai_attr_set)));

            ASSERT_EQ(NULL,sai_api_query(SAI_API_PORT,
                        (static_cast<void**>(static_cast<void*>(&sai_port_api_table)))));

            ASSERT_TRUE(sai_port_api_table != NULL);

            EXPECT_TRUE(sai_port_api_table->set_port_attribute != NULL);
            EXPECT_TRUE(sai_port_api_table->get_port_attribute != NULL);
            EXPECT_TRUE(sai_port_api_table->get_port_stats != NULL);

            portTest::sai_get_port_hw_lane_map_table();
            gport_id = port_info[0].port_id;
            unsigned int port_idx = 0;

            /*print port dump*/
            LOG_PRINT("Port dump list :\n");
            for(port_idx = 0; port_idx < port_count; port_idx++) {
                printf("port_idx:%d port_id:0x%" PRIx64 " hw_lane:%d isvalid:%s \n",
                    port_idx,port_info[port_idx].port_id,port_info[port_idx].hw_lane,
                                    (port_info[port_idx].port_valid==true)?"True":"False");
            }
        }

        static sai_switch_api_t *sai_switch_api_table;
        static sai_port_api_t* sai_port_api_table;
        static sai_bridge_api_t* p_sai_bridge_api_tbl;
};

sai_switch_api_t* portTest ::sai_switch_api_table = NULL;
sai_port_api_t* portTest ::sai_port_api_table = NULL;

void portTest::sai_port_supported_speed_check(sai_object_id_t port_id, uint32_t speed, bool *supported)
{
    uint32_t count = 0;
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_get;

    *supported = false;
    memset(&sai_attr_get, 0, sizeof(sai_attr_get));
    sai_attr_get.id = SAI_PORT_ATTR_SUPPORTED_SPEED;

    sai_attr_get.value.s32list.count = SAI_PORT_CAP_SPEED_MAX;
    sai_attr_get.value.s32list.list = (int32_t *)calloc (SAI_PORT_CAP_SPEED_MAX,
                                                         sizeof(int32_t));
    ASSERT_TRUE(sai_attr_get.value.s32list.list != NULL);

    do {
        ret = sai_port_api_table->get_port_attribute(port_id, 1, &sai_attr_get);

        /* Handles buffer overflow case, by reallocating required memory */
        if(ret == SAI_STATUS_BUFFER_OVERFLOW ) {
            free(sai_attr_get.value.s32list.list);
            LOG_PRINT("port 0x%" PRIx64 " count of supported speeds is %d \r\n", port_id, sai_attr_get.value.s32list.count);
            sai_attr_get.value.s32list.list = (int32_t *)calloc (sai_attr_get.value.s32list.count,
                                                                 sizeof(int32_t));
            ASSERT_TRUE(sai_attr_get.value.s32list.list != NULL);
            ret = sai_port_api_table->get_port_attribute(port_id, 1, &sai_attr_get);
        }

        if(ret != SAI_STATUS_SUCCESS) {
            *supported = false;
            LOG_PRINT("port 0x%" PRIx64 " supported speed get failed\n", port_id);
            break;
        }

        for (count = 0; count < sai_attr_get.value.s32list.count; count++) {
            if (speed == (uint32_t)sai_attr_get.value.s32list.list[count]) {
                *supported = true;
                break;
            }
        }

    } while(0);

    free(sai_attr_get.value.s32list.list);
}


/*
 * Validates Port type get for all the ports in the switch
 * UT PASS case: port should be logical
 */
TEST_F(portTest, logical_port_type_get)
{
    unsigned int port_idx = 0;

    for(port_idx = 0; port_idx < port_count; port_idx++) {
        if(port_info[port_idx].port_valid != true) {
            continue;
        }
        EXPECT_TRUE(sai_port_type_logical(port_info[port_idx].port_id,
                                          sai_port_api_table));
    }
}

/*
 * Validates Port type get for CPU port in the switch
 * UT PASS case: port type should be CPU
 */
TEST_F(portTest, cpu_port_type_get)
{
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));
    sai_attr_get.id = SAI_SWITCH_ATTR_CPU_PORT;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1, &sai_attr_get));

    sai_object_id_t cpu_port = sai_attr_get.value.oid;

    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));
    sai_attr_get.id = SAI_PORT_ATTR_TYPE;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(cpu_port, 1, &sai_attr_get));

    ASSERT_EQ(SAI_PORT_TYPE_CPU, sai_attr_get.value.s32);
}

/*
 * Validates Port Operational Status get for all the valid ports in the switch
 * UT PASS case: port should be Link UP
 *
 * For port oper state get to succeed, internal loopback mode will be used,
 * as time taken for a link to come UP after Admin state set might vary depending on the NPU,
 * and would need adding some sleep in between these calls. Need to re-visit about adding a
 * sleep to validate the actual port Link status instead of using Internal Loopback.
 */
TEST_F(portTest, oper_status_get)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_get;
    sai_attribute_t sai_attr_set;
    unsigned int port_idx = 0;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    /* Check port Oper state as UP by enabling the internal Loopback */
    for(port_idx = 0; port_idx < port_count; port_idx++) {
        if(port_info[port_idx].port_valid != true) {
            continue;
        }
        /* Enable link admin state */
        sai_attr_set.id = SAI_PORT_ATTR_ADMIN_STATE;
        sai_attr_set.value.booldata = true;

        ret = sai_port_api_table->set_port_attribute(port_info[port_idx].port_id, &sai_attr_set);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        /* Loopback set to PHY */
        sai_attr_set.id = SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE;
        sai_attr_set.value.s32 = SAI_PORT_INTERNAL_LOOPBACK_MODE_PHY;

        ret = sai_port_api_table->set_port_attribute(port_info[port_idx].port_id, &sai_attr_set);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_get.id = SAI_PORT_ATTR_OPER_STATUS;
        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }
        EXPECT_EQ(SAI_PORT_OPER_STATUS_UP, sai_attr_get.value.s32);
    }
}


/*
 * Set port attribute with given value and get the same attribute
 * @TODO: use this API in all applicable places
 */
bool portTest::sai_port_attr_set_get(sai_object_id_t port_id, sai_attribute_t *sai_attr)
{

    if(sai_port_api_table->set_port_attribute(port_id, sai_attr) != SAI_STATUS_SUCCESS) {
        return false;
    }

    memset(&sai_attr->value, 0, sizeof(sai_attribute_value_t));

    if(sai_port_api_table->get_port_attribute(port_id, 1, sai_attr) != SAI_STATUS_SUCCESS) {
        return false;
    }

    return true;
}

/*
 * Validates if the port is able to get/set a given port speed
 */
bool portTest::sai_port_speed_set_get(sai_object_id_t port_id, uint32_t speed)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_SPEED;
    sai_attr_set.value.u32 = speed;

    if(sai_port_api_table->set_port_attribute(port_id, &sai_attr_set) != SAI_STATUS_SUCCESS) {
        return false;
    }

    sai_attr_get.id = SAI_PORT_ATTR_SPEED;

    if(sai_port_api_table->get_port_attribute(port_id, 1, &sai_attr_get) != SAI_STATUS_SUCCESS) {
        return false;
    }

    if(speed != sai_attr_get.value.u32) {
        return false;
    }

    return true;
}

/*
 * Validates 40G Port speed capability for all the valid ports in the switch
 * UT PASS case: port should be able to set get 40G speed
 */
TEST_F(portTest, speed_40g_set_get)
{
    unsigned int port_idx = 0;
    uint32_t speed = 40000;
    bool supported = false;

    for(port_idx = 0; port_idx < port_count; port_idx++) {
        if(port_info[port_idx].port_valid != true) {
            continue;
        }
        sai_port_supported_speed_check(port_info[port_idx].port_id, speed, &supported);
        if (supported == true) {
            EXPECT_TRUE(sai_port_speed_set_get(port_info[port_idx].port_id, speed));
        }
    }
}

/*
 * Validates 10G Port speed capability for all the valid ports in the switch
 * UT PASS case: port should be able to set get 10G speed
 */
TEST_F(portTest, speed_10g_set_get)
{
    unsigned int port_idx = 0;
    uint32_t speed = 10000;
    bool supported = false;

    for(port_idx = 0; port_idx < port_count; port_idx++) {
        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_port_supported_speed_check(port_info[port_idx].port_id, speed, &supported);
        if (supported == true) {
            EXPECT_TRUE(sai_port_speed_set_get(port_info[port_idx].port_id, speed));
        }
    }
}

/*
 * Validates 1G Port speed capability for all the valid ports in the switch
 * UT PASS case: port should be able to set get 1G speed
 */
TEST_F(portTest, speed_1g_set_get)
{
    unsigned int port_idx = 0;
    uint32_t speed = 1000;
    bool supported = false;

    for(port_idx = 0; port_idx < port_count; port_idx++) {

        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_port_supported_speed_check(port_info[port_idx].port_id, speed, &supported);
        if (supported == true) {
            EXPECT_TRUE(sai_port_speed_set_get(port_info[port_idx].port_id, speed));
        }
    }
}

/*
 * Validates 100M Port speed capability for all the valid ports in the switch
 * UT PASS case: port should be able to set get 100M speed
 */
TEST_F(portTest, speed_100m_set_get)
{
    unsigned int port_idx = 0;
    uint32_t speed = 100;
    bool supported = false;

    for(port_idx = 0; port_idx < port_count; port_idx++) {

        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_port_supported_speed_check(port_info[port_idx].port_id, speed, &supported);
        if (supported == true) {
            EXPECT_TRUE(sai_port_speed_set_get(port_info[port_idx].port_id, speed));
        }
    }
}

/*
 * Validates if port admin state can be enabled for all the valid ports in the switch
 * UT PASS case: port admin state should get enabled
 */
TEST_F(portTest, admin_state_set_get)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;
    unsigned int port_idx = 0;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    for(port_idx = 0; port_idx < port_count; port_idx++) {

        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_attr_set.id = SAI_PORT_ATTR_ADMIN_STATE;
        sai_attr_set.value.booldata = true;

        ret = sai_port_api_table->set_port_attribute(port_info[port_idx].port_id, &sai_attr_set);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_get.id = SAI_PORT_ATTR_ADMIN_STATE;
        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }
        EXPECT_TRUE(sai_attr_get.value.booldata);
    }
}

/*
 * Validates if port EEE state can be enabled for all the copper ports in the switch
 * UT PASS case: port EEE state should set/get enabled
 */
TEST_F(portTest, eee_state_set_get)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;
    unsigned int port_idx = 0;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    for(port_idx = 0; port_idx < SAI_EEE_COPPER_PORT_MAX; port_idx++) {

        if(port_info[port_idx].port_valid != true) {
            continue;
        }
        sai_attr_set.id = SAI_PORT_ATTR_EEE_ENABLE;
        sai_attr_set.value.booldata = true;
        ret = sai_port_api_table->set_port_attribute(port_info[port_idx].port_id, &sai_attr_set);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_get.id =  SAI_PORT_ATTR_EEE_ENABLE;
        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }


        EXPECT_TRUE(sai_attr_get.value.booldata);

    }
}

/*
 * Validates if port EEE time can be set for all the copper ports in the switch
 * UT PASS case: port EEE timer set/get to default value.
 */
TEST_F(portTest, eee_timer_set_get)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_get;
    sai_attribute_t sai_attr_set;
    unsigned int port_idx = 0;

    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));

    for(port_idx = 0; port_idx < SAI_EEE_COPPER_PORT_MAX; port_idx++) {

        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_attr_set.id = SAI_PORT_ATTR_EEE_IDLE_TIME;
        sai_attr_set.value.u16 = SAI_EEE_DEFAULT_IDLE_TIME;
        ret = sai_port_api_table->set_port_attribute(port_info[port_idx].port_id, &sai_attr_set);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_get.id = SAI_PORT_ATTR_EEE_IDLE_TIME;
        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id,1, &sai_attr_get);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }


        EXPECT_EQ(SAI_EEE_DEFAULT_IDLE_TIME, sai_attr_get.value.u16);

        sai_attr_set.id =  SAI_PORT_ATTR_EEE_WAKE_TIME;
        sai_attr_set.value.u16 = SAI_EEE_DEFAULT_WAKE_TIME;
        ret = sai_port_api_table->set_port_attribute(port_info[port_idx].port_id, &sai_attr_set);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }
        sai_attr_get.id =  SAI_PORT_ATTR_EEE_WAKE_TIME;
        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }


        EXPECT_EQ(SAI_EEE_DEFAULT_WAKE_TIME, sai_attr_get.value.u16);

    }
}

/*
 * Validates if port location LED can be enabled for all the valid ports in the switch
 * UT PASS case: port location LED set/get
 */
TEST_F(portTest, location_led_set_get)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;
    unsigned int port_idx = 0;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    for(port_idx = 0; port_idx < port_count; port_idx++) {

        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_attr_set.id = SAI_PORT_ATTR_LOCATION_LED;
        sai_attr_set.value.booldata = true;

        ret = sai_port_api_table->set_port_attribute(port_info[port_idx].port_id, &sai_attr_set);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_get.id = SAI_PORT_ATTR_LOCATION_LED;
        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        EXPECT_TRUE(sai_attr_get.value.booldata);

    }
}

/*
 * Validates if the port default VLAN can be set to 100.
 * UT PASS case: port should be set with default VLAN as 100
 */
TEST_F(portTest, default_vlan_set_get)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;
    sai_vlan_id_t vlan_id = 100;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_PORT_VLAN_ID;
    sai_attr_set.value.u16 = vlan_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(gport_id, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_PORT_VLAN_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(gport_id, 1, &sai_attr_get));

    ASSERT_EQ(vlan_id, sai_attr_get.value.u16);
}

/*
 * Validates if the port default VLAN priority can be set to 1.
 * UT PASS case: port should be set with default VLAN priority as 1.
 */
TEST_F(portTest, default_vlan_prio_set_get)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY;
    sai_attr_set.value.u8 = 1;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(gport_id, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(gport_id, 1, &sai_attr_get));

    ASSERT_EQ(1, sai_attr_get.value.u8);
}

/*
 * Validates if the port drop untagged capability can be enabled
 * UT PASS case: port drop untagged capability should get enabled
 */
TEST_F(portTest, drop_untagged_set_get)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_DROP_UNTAGGED;
    sai_attr_set.value.booldata = true;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(gport_id, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_DROP_UNTAGGED;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(gport_id, 1, &sai_attr_get));

    ASSERT_TRUE(sai_attr_get.value.booldata);
}

/*
 * Validates if the port drop tagged capability can be enabled
 * UT PASS case: port drop tagged capability should get enabled
 */
TEST_F(portTest, drop_tagged_set_get)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_DROP_TAGGED;
    sai_attr_set.value.booldata = true;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(gport_id, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_DROP_TAGGED;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(gport_id, 1, &sai_attr_get));

    ASSERT_TRUE(sai_attr_get.value.booldata);
}

/*
 * Validate if setting drop untagged is not affecting drop tagged,
 * and vice-versa.
 */
TEST_F(portTest, drop_untagged_and_tagged_set)
{
    bool drop_untagged_val;
    bool drop_tagged_val;

    /*
     * drop_untagged = false
     * drop_tagged   = false
     */
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_set_port_discard (sai_port_api_table, gport_id, true, false));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_set_port_discard (sai_port_api_table, gport_id, false, false));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_get_port_discard (sai_port_api_table, gport_id, true,
                                    &drop_untagged_val));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_get_port_discard (sai_port_api_table, gport_id, false,
                                    &drop_tagged_val));
    ASSERT_FALSE(drop_untagged_val);
    ASSERT_FALSE(drop_tagged_val);

    /*
     * drop_untagged = false
     * drop_tagged   = true
     */
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_set_port_discard (sai_port_api_table, gport_id, true, false));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_set_port_discard (sai_port_api_table, gport_id, false, true));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_get_port_discard (sai_port_api_table, gport_id, true,
                                    &drop_untagged_val));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_get_port_discard (sai_port_api_table, gport_id, false,
                                    &drop_tagged_val));
    ASSERT_FALSE(drop_untagged_val);
    ASSERT_TRUE(drop_tagged_val);

    /*
     * drop_untagged = true
     * drop_tagged   = false
     */
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_set_port_discard (sai_port_api_table, gport_id, true, true));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_set_port_discard (sai_port_api_table, gport_id, false, false));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_get_port_discard (sai_port_api_table, gport_id, true,
                                    &drop_untagged_val));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_get_port_discard (sai_port_api_table, gport_id, false,
                                    &drop_tagged_val));
    ASSERT_TRUE(drop_untagged_val);
    ASSERT_FALSE(drop_tagged_val);

    /*
     * drop_untagged = true
     * drop_tagged   = true
     */
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_set_port_discard (sai_port_api_table, gport_id, true, true));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_set_port_discard (sai_port_api_table, gport_id, false, true));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_get_port_discard (sai_port_api_table, gport_id, true,
                                    &drop_untagged_val));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_get_port_discard (sai_port_api_table, gport_id, false,
                                    &drop_tagged_val));
    ASSERT_TRUE(drop_untagged_val);
    ASSERT_TRUE(drop_tagged_val);
}

/*
 * Validates if a given looback mode gets configured correctly on a given port
 */
bool portTest::sai_internal_loopback_set_get(sai_object_id_t port_id,
                                             sai_port_internal_loopback_mode_t lb_mode)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE;
    sai_attr_set.value.s32 = lb_mode;

    if(sai_port_api_table->set_port_attribute(port_id, &sai_attr_set) != SAI_STATUS_SUCCESS) {
        return false;
    }

    sai_attr_get.id = SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE;

    if(sai_port_api_table->get_port_attribute(port_id, 1, &sai_attr_get) != SAI_STATUS_SUCCESS) {
        return false;
    }
    if(lb_mode != sai_attr_get.value.s32) {
        return false;
    }

    return true;
}

/*
 * Validates internal loopback mode set/get for all the valid ports in the switch
 * UT PASS case: port internal loopback mode should get set to PHY/MAC/NONE
 */
TEST_F(portTest, internal_loopback_set_get)
{
    unsigned int port_idx = 0;

    for(port_idx = 0; port_idx < port_count; port_idx++) {

        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        EXPECT_TRUE(sai_internal_loopback_set_get(port_info[port_idx].port_id, SAI_PORT_INTERNAL_LOOPBACK_MODE_PHY));
        EXPECT_TRUE(sai_internal_loopback_set_get(port_info[port_idx].port_id, SAI_PORT_INTERNAL_LOOPBACK_MODE_MAC));
        EXPECT_TRUE(sai_internal_loopback_set_get(port_info[port_idx].port_id, SAI_PORT_INTERNAL_LOOPBACK_MODE_NONE));
    }
}

/*
 * Validates if port autoneg can be enabled for all the valid ports
 * in the switch
 */
TEST_F(portTest, autoneg_state_set_get)
{
    sai_attribute_t sai_attr;
    unsigned int port_idx = 0;

    memset(&sai_attr, 0, sizeof(sai_attribute_t));

    for(port_idx = 0; port_idx < port_count; port_idx++) {

        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        EXPECT_TRUE(sai_internal_loopback_set_get(port_info[port_idx].port_id, SAI_PORT_INTERNAL_LOOPBACK_MODE_NONE));

        sai_attr.id = SAI_PORT_ATTR_AUTO_NEG_MODE;

        /* Enable auto-neg and check */
        sai_attr.value.booldata = true;
        EXPECT_TRUE(sai_port_attr_set_get(port_info[port_idx].port_id, &sai_attr));
        EXPECT_TRUE(sai_attr.value.booldata);

        /* Disable auto-neg and check */
        sai_attr.value.booldata = false;
        EXPECT_TRUE(sai_port_attr_set_get(port_info[port_idx].port_id, &sai_attr));
        EXPECT_FALSE(sai_attr.value.booldata);
    }
}

/*
 * Validates if port full duplex can be enabled for all the valid ports
 * in the switch
 */
TEST_F(portTest, fullduplex_state_set_get)
{
    sai_attribute_t sai_attr;
    unsigned int port_idx = 0;

    memset(&sai_attr, 0, sizeof(sai_attribute_t));

    for(port_idx = 0; port_idx < port_count; port_idx++) {

        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_attr.id = SAI_PORT_ATTR_FULL_DUPLEX_MODE;

        /* Enable full duplex and check */
        sai_attr.value.booldata = true;
        EXPECT_TRUE(sai_port_attr_set_get(port_info[port_idx].port_id, &sai_attr));
        EXPECT_TRUE(sai_attr.value.booldata);

        /* Disable full duplex and check */
        sai_attr.value.booldata = false;
        EXPECT_TRUE(sai_port_attr_set_get(port_info[port_idx].port_id, &sai_attr));
        EXPECT_FALSE(sai_attr.value.booldata);
    }
}

/*
 * Validates and prints the HW lane list for all the valid logical ports in the switch
 * UT PASS case: hw lane list get API for all ports should succeed without error.
 */
TEST_F(portTest, hw_lane_list_get)
{
    unsigned int port_idx = 0, lane = 0;
    sai_status_t ret = SAI_STATUS_SUCCESS;

    sai_attribute_t sai_attr_get;
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));
    sai_attr_get.id = SAI_PORT_ATTR_HW_LANE_LIST;

    sai_attr_get.value.u32list.count = 1;
    sai_attr_get.value.u32list.list = (uint32_t *)calloc (1, sizeof(uint32_t));

    ASSERT_TRUE(sai_attr_get.value.u32list.list != NULL);

    for(port_idx = 0; port_idx < port_count; port_idx++) {

        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);

        /* Handles buffer overflow case, by reallocating required memory */
        if(ret == SAI_STATUS_BUFFER_OVERFLOW ) {
            free(sai_attr_get.value.u32list.list);
            sai_attr_get.value.u32list.list = (uint32_t *)calloc (sai_attr_get.value.u32list.count,
                                                                  sizeof(uint32_t));
            ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        }

        if(ret != SAI_STATUS_SUCCESS) {
            free(sai_attr_get.value.u32list.list);
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        for(lane = 0; lane < sai_attr_get.value.u32list.count; lane++ ) {
            LOG_PRINT("port 0x%" PRIx64 " lane %d is %d \r\n", port_info[port_idx].port_id, lane,
                      sai_attr_get.value.u32list.list[lane]);
        }
    }

    if (ret == SAI_STATUS_SUCCESS)
        free(sai_attr_get.value.u32list.list);
}

/*
 * Validates and prints the supported breakout modes for all the valid logical ports in the switch
 * UT PASS case: supported breakout mode get API for all ports should succeed without error.
 */
TEST_F(portTest, supported_breakout_mode_get)
{
    unsigned int port_idx = 0, count = 0 ;
    sai_status_t ret = SAI_STATUS_FAILURE;

    sai_attribute_t sai_attr_get;
    memset(&sai_attr_get, 0, sizeof(sai_attr_get));
    sai_attr_get.id = SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE_TYPE;

    sai_attr_get.value.s32list.count = SAI_PORT_BREAKOUT_MODE_TYPE_MAX;
    sai_attr_get.value.s32list.list = (int32_t *)calloc (SAI_PORT_BREAKOUT_MODE_TYPE_MAX,
                                                         sizeof(int32_t));
    ASSERT_TRUE(sai_attr_get.value.s32list.list != NULL);

    for(port_idx = 0; port_idx < port_count; port_idx++) {

        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);

        /* Handles buffer overflow case, by reallocating required memory */
        if(ret == SAI_STATUS_BUFFER_OVERFLOW ) {
            free(sai_attr_get.value.s32list.list);
            sai_attr_get.value.s32list.list = (int32_t *)calloc (sai_attr_get.value.s32list.count,
                                                                 sizeof(int32_t));
            ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        }

        if(ret != SAI_STATUS_SUCCESS) {
            free(sai_attr_get.value.s32list.list);
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        for(count = 0; count < sai_attr_get.value.s32list.count; count++ ) {
            LOG_PRINT("port 0x%" PRIx64 " mode %d is %d \r\n", port_info[port_idx].port_id, count,
                      sai_attr_get.value.s32list.list[count]);
        }
    }

    free(sai_attr_get.value.u32list.list);
}

/*
 * Validates and prints the current breakout modes for all the valid logical ports in the switch
 * UT PASS case: current breakout mode get API for all ports should succeed without error.
 */
TEST_F(portTest, current_breakout_mode_get)
{
    unsigned int port_idx = 0;

    sai_attribute_t sai_attr_get;
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));
    sai_attr_get.id = SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE;

    for(port_idx = 0; port_idx < port_count; port_idx++) {

        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get));

        LOG_PRINT("port 0x%" PRIx64 " current breakout mode %d \r\n", port_info[port_idx].port_id, sai_attr_get.value.s32);
    }
}

/*
 * Validates if the port update DSCP can be enabled on the port
 * UT PASS case: Update DSCP should get enabled on the port
 */
TEST_F(portTest, update_dscp_set_get)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_UPDATE_DSCP;
    sai_attr_set.value.booldata = true;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(gport_id, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_UPDATE_DSCP;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(gport_id, 1, &sai_attr_get));

    ASSERT_TRUE(sai_attr_get.value.booldata);
}

/*
 * Validates if the flood storm control can be enabled on the port
 * UT PASS case: flood storm control should get enabled on the port
 */
TEST_F(portTest, flood_storm_control_set_get)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID;
    sai_attr_set.value.booldata = true;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(gport_id, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(gport_id, 1, &sai_attr_get));

    ASSERT_TRUE(sai_attr_get.value.booldata);
}

/*
 * Validates if the broadcast storm control can be enabled on the port
 * UT PASS case: broadcast storm control should get enabled on the port
 */
TEST_F(portTest, broadcast_storm_set_get)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID;
    sai_attr_set.value.booldata = true;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(gport_id, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(gport_id, 1, &sai_attr_get));

    ASSERT_TRUE(sai_attr_get.value.booldata);
}

/*
 * Validates if the multicast storm control can be enabled on the port
 * UT PASS case: multicast storm control should get enabled on the port
 */
TEST_F(portTest, multicast_storm_set_get)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_set.id = SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID;
    sai_attr_set.value.booldata = true;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(gport_id, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(gport_id, 1, &sai_attr_get));

    ASSERT_TRUE(sai_attr_get.value.booldata);
}

/*
 * Validates if the port MTU can be set to 1000 for all valid ports in the switch
 * UT PASS case: MTU should get set to 1000 on all valid ports
 */
TEST_F(portTest, mtu_set_get)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    unsigned int port_idx = 0, mtu = 1000;

    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    for(port_idx = 0; port_idx < port_count; port_idx++) {

        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_attr_set.id = SAI_PORT_ATTR_MTU;
        sai_attr_set.value.u32 = mtu;
        ret = sai_port_api_table->set_port_attribute(port_info[port_idx].port_id, &sai_attr_set);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_get.id = SAI_PORT_ATTR_MTU;
        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        EXPECT_EQ(mtu, sai_attr_get.value.u32);
    }
}

TEST_F(portTest, max_port_mtu_set_get)
{
    unsigned int port_idx = 0;
    unsigned int max_mtu = 0;

    sai_attribute_t sai_attr;

    memset(&sai_attr, 0, sizeof(sai_attribute_t));

    sai_attr.id = SAI_SWITCH_ATTR_PORT_MAX_MTU;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_switch_api_table->get_switch_attribute(switch_id,1, &sai_attr));

    EXPECT_NE(sai_attr.value.u32, 0);
    max_mtu = sai_attr.value.u32;

    LOG_PRINT("Maximum PORT mtu is %d \r\n", max_mtu);

    memset(&sai_attr, 0, sizeof(sai_attribute_t));

    for(port_idx = 0; port_idx < port_count; port_idx++) {

        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_attr.id = SAI_PORT_ATTR_MTU;
        sai_attr.value.u32 = max_mtu;
        EXPECT_TRUE(sai_port_attr_set_get(port_info[port_idx].port_id, &sai_attr));

        LOG_PRINT("PORT %d mtu is %d \r\n", port_idx, sai_attr.value.u32);

        EXPECT_EQ(max_mtu, sai_attr.value.u32);
    }
}

/*
 * Set all supported port media type one by one and verify configured value
 * is set by reading back the media type.
 * UT PASS case: Media type set to configured type
 */
TEST_F(portTest, media_type_set_get)
{
    sai_int32_t media_type = 0;
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    for(media_type = SAI_PORT_MEDIA_TYPE_NOT_PRESENT;
        media_type <= SAI_PORT_MEDIA_TYPE_COPPER; media_type++) {
        memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
        memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

        sai_attr_set.id = SAI_PORT_ATTR_MEDIA_TYPE;
        sai_attr_set.value.s32 = media_type;

        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  sai_port_api_table->set_port_attribute(gport_id, &sai_attr_set));

        sai_attr_get.id = SAI_PORT_ATTR_MEDIA_TYPE;

        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  sai_port_api_table->get_port_attribute(gport_id, 1, &sai_attr_get));

        ASSERT_EQ(media_type, sai_attr_get.value.s32);
    }
}


/*
 * Port All Statistics Get: Tests only the support of statistics counters;
 * not the stats collection functionality
 */
TEST_F(portTest, all_stats_get)
{
    uint64_t counters[1] = {0};
    int32_t counter = 0;
    sai_port_stat_t counter_ids[1];
    sai_status_t status = SAI_STATUS_FAILURE;

    for(counter = SAI_PORT_STAT_IF_IN_OCTETS;
        counter <= SAI_PORT_STAT_PFC_7_TX_PKTS; counter++)
    {
        counter_ids[0] = (sai_port_stat_t)counter;
        status = sai_port_api_table->get_port_stats(gport_id, counter_ids, 1, counters);

        if(status == SAI_STATUS_SUCCESS) {
            LOG_PRINT("port 0x%" PRIx64 " stat id %d value is %ld \r\n", gport_id, counter_ids[0], counters[0]);
        } else if (status == (SAI_STATUS_ATTR_NOT_SUPPORTED_0 + counter)) {
            LOG_PRINT("port 0x%" PRIx64 " stat id %d not implemented \r\n", gport_id, counter_ids[0]);
        }

        EXPECT_EQ(SAI_STATUS_SUCCESS, status);
    }
}

/*
 * Port EEE Statistics Get: Tests only the copper port EEE statistics counters;
 * not the stats collection functionality
 */
TEST_F(portTest, eee_stats_get)
{
    uint64_t counters[1] = {0};
    int32_t counter = 0;
    unsigned int port_idx = 0;
    sai_port_stat_t counter_ids[1];
    sai_status_t status = SAI_STATUS_FAILURE;

    for(port_idx = 0; port_idx < SAI_EEE_COPPER_PORT_MAX; port_idx++) {
        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        for(counter = SAI_PORT_STAT_EEE_TX_EVENT_COUNT;
                counter <= SAI_PORT_STAT_EEE_RX_DURATION; counter++)
        {
            counter_ids[0] = (sai_port_stat_t)counter;
            status = sai_port_api_table->get_port_stats(port_info[port_idx].port_id, counter_ids, 1, counters);

            if(status == SAI_STATUS_SUCCESS) {
                LOG_PRINT("port 0x%" PRIx64 " EEE stat id %d value is %ld \r\n", port_info[port_idx].port_id, counter_ids[0], counters[0]);
            } else if (status == (SAI_STATUS_ATTR_NOT_SUPPORTED_0 + counter)) {
                LOG_PRINT("port 0x%" PRIx64 "EEE stat id %d not implemented \r\n", port_info[port_idx].port_id, counter_ids[0]);
            }

            EXPECT_EQ(SAI_STATUS_SUCCESS, status);
        }
    }
}

/*
 * Clear EEE statistic counters
 */
TEST_F(portTest, eee_stat_clear)
{
    int32_t counter = 0;
    unsigned int port_idx = 0;
    sai_port_stat_t counter_ids[1];
    sai_status_t status = SAI_STATUS_FAILURE;

    for(port_idx = 0; port_idx < SAI_EEE_COPPER_PORT_MAX; port_idx++) {
        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        for(counter = SAI_PORT_STAT_EEE_TX_EVENT_COUNT;
                counter <= SAI_PORT_STAT_EEE_RX_DURATION; counter++)
        {
            counter_ids[0] = (sai_port_stat_t)counter;
            status = sai_port_api_table->clear_port_stats(port_info[port_idx].port_id, counter_ids, 1);

            if(status == SAI_STATUS_SUCCESS) {
                LOG_PRINT("port 0x%" PRIx64 " EEE stat id %d \r\n", port_info[port_idx].port_id, counter_ids[0]);
            } else if (status == (SAI_STATUS_ATTR_NOT_SUPPORTED_0 + counter)) {
                LOG_PRINT("port 0x%" PRIx64 " EEE stat id %d not implemented \r\n", port_info[port_idx].port_id, counter_ids[0]);
            }

            EXPECT_EQ(SAI_STATUS_SUCCESS, status);
        }
    }
}

/*
 * Clear port's all statistic counters one by one
 */
TEST_F(portTest, port_stat_clear)
{
    int32_t counter = 0;
    sai_port_stat_t counter_ids[1];
    sai_status_t status = SAI_STATUS_FAILURE;

    for(counter = SAI_PORT_STAT_IF_IN_OCTETS;
        counter <= SAI_PORT_STAT_PFC_7_TX_PKTS; counter++)
    {
        counter_ids[0] = (sai_port_stat_t)counter;
        status = sai_port_api_table->clear_port_stats(gport_id, counter_ids, 1);

        if(status == SAI_STATUS_SUCCESS) {
            LOG_PRINT("port 0x%" PRIx64 " stat id %d \r\n", gport_id, counter_ids[0]);
        } else if (status == (SAI_STATUS_ATTR_NOT_SUPPORTED_0 + counter)) {
            LOG_PRINT("port 0x%" PRIx64 " stat id %d not implemented \r\n", gport_id, counter_ids[0]);
        }

        EXPECT_EQ(SAI_STATUS_SUCCESS, status);
    }
}

/*
 * Clear port's all statistic counters at one shot
 */
TEST_F(portTest, port_all_stat_clear)
{
    sai_status_t status = SAI_STATUS_FAILURE;
    unsigned int port_idx = 0;

    for(port_idx = 0; port_idx < port_count; port_idx++) {
        if(port_info[port_idx].port_valid != true) {
            continue;
        }

        status = sai_port_api_table->clear_port_all_stats(gport_id);
        if (status == SAI_STATUS_SUCCESS) {
            LOG_PRINT("port 0x%" PRIx64 " stat cleared\r\n", gport_id);
        } else {
            LOG_PRINT("port 0x%" PRIx64 " stat clear failed %d\r\n", gport_id, status);
        }
        EXPECT_EQ(SAI_STATUS_SUCCESS, status);
    }
}

int port_egress_block_port_list_count_get(sai_object_id_t port_id,
                                          sai_port_api_t *sai_port_api_table)
{
    sai_object_id_t port_arr[SAI_MAX_PORTS];
    sai_object_list_t egress_block_port_list;
    sai_attribute_t attr;
    sai_status_t ret = SAI_STATUS_FAILURE;
    egress_block_port_list.list = port_arr;
    egress_block_port_list.count = SAI_MAX_PORTS;

    memset(&attr, 0, sizeof(sai_attribute_t));
    attr.id = SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST;
    attr.value.objlist = egress_block_port_list;

    ret = sai_port_api_table->get_port_attribute(port_id, 1, &attr);
    if (ret != SAI_STATUS_SUCCESS) {
        LOG_PRINT("Port 0x%" PRIx64 " get egress block port list failed with err: %d\r\n",
                  port_id, ret);
        return -1;
    }

    return (int) attr.value.objlist.count;
}

TEST_F(portTest, port_egress_block_port_list_set)
{
    sai_object_id_t port_arr[1];
    sai_object_list_t egress_block_port_list;
    sai_attribute_t attr;
    int egress_block_port_count = 0;

    port_arr[0] = port_info[1].port_id;

    egress_block_port_list.list = port_arr;
    egress_block_port_list.count = 1;

    egress_block_port_count = port_egress_block_port_list_count_get(gport_id,
                                                                    sai_port_api_table);
    memset(&attr, 0, sizeof(sai_attribute_t));
    attr.id = SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST;
    attr.value.objlist = egress_block_port_list;

    EXPECT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(gport_id, &attr));
    EXPECT_EQ((egress_block_port_count+1),
              port_egress_block_port_list_count_get(gport_id, sai_port_api_table));
}

TEST_F(portTest, port_egress_block_port_list_get)
{
    sai_object_id_t port_arr[SAI_MAX_PORTS];
    sai_object_list_t egress_block_port_list;
    sai_attribute_t attr;
    unsigned int port_idx = 0;

    egress_block_port_list.list = port_arr;
    egress_block_port_list.count = SAI_MAX_PORTS;

    memset(&attr, 0, sizeof(sai_attribute_t));
    attr.id = SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST;
    attr.value.objlist = egress_block_port_list;

    EXPECT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(gport_id, 1, &attr));

    for(port_idx = 0; port_idx < attr.value.objlist.count; port_idx++) {
        LOG_PRINT("Port 0x%" PRIx64 " \r\n", attr.value.objlist.list[port_idx]);
    }
}

TEST_F(portTest, port_egress_block_port_list_clear)
{
    sai_object_id_t port_arr[0];
    sai_object_list_t egress_block_port_list;
    sai_attribute_t attr;

    egress_block_port_list.list = port_arr;
    egress_block_port_list.count = 0;

    memset(&attr, 0, sizeof(sai_attribute_t));
    attr.id = SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST;
    attr.value.objlist = egress_block_port_list;

    EXPECT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(gport_id, &attr));
    EXPECT_EQ(0,
              port_egress_block_port_list_count_get(gport_id, sai_port_api_table));
}

/*
 * Validates if the port flow control can be set to Tx only
 * UT PASS case: Flow control should get enabled for Tx only
 */
TEST_F(portTest, flow_control_test)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_get.id = SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(gport_id, 1, &sai_attr_get));

    ASSERT_EQ(SAI_PORT_FLOW_CONTROL_MODE_DISABLE, sai_attr_get.value.s32);

    sai_attr_set.id = SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE;
    sai_attr_set.value.s32 = SAI_PORT_FLOW_CONTROL_MODE_BOTH_ENABLE;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(gport_id, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(gport_id, 1, &sai_attr_get));

    ASSERT_EQ(SAI_PORT_FLOW_CONTROL_MODE_BOTH_ENABLE, sai_attr_get.value.s32);

    sai_attr_set.value.s32 = SAI_PORT_FLOW_CONTROL_MODE_DISABLE;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(gport_id, &sai_attr_set));

}

TEST_F(portTest, pfc_bitmap_test)
{
    sai_attribute_t sai_attr_set;
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    sai_attr_get.id = SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(gport_id, 1, &sai_attr_get));

    ASSERT_EQ(0, sai_attr_get.value.u8);

    sai_attr_set.id = SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL;
    sai_attr_set.value.u8 = 0x4f;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(gport_id, &sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->get_port_attribute(gport_id, 1, &sai_attr_get));

    ASSERT_EQ(0x4f, sai_attr_get.value.u8);

    sai_attr_set.value.u8 = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_port_api_table->set_port_attribute(gport_id, &sai_attr_set));

}

void  portTest::sai_get_port_hw_lane_map_table()
{
    sai_attribute_t sai_port_attr,sai_lane_attr;
    uint8_t count = 0, lane = 0;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    sai_object_id_t port_list[SAI_MAX_PORTS] = {0};
    uint8_t lane_count = 0;

    memset (&sai_port_attr, 0, sizeof (sai_port_attr));
    memset (&sai_lane_attr, 0, sizeof (sai_lane_attr));

    sai_port_attr.id = SAI_SWITCH_ATTR_PORT_LIST;
    sai_port_attr.value.objlist.count = SAI_MAX_PORTS;
    sai_port_attr.value.objlist.list  = port_list;

    sai_switch_api_table->get_switch_attribute(switch_id,1,&sai_port_attr);
    port_count = sai_port_attr.value.objlist.count;

    sai_lane_attr.id = SAI_PORT_ATTR_HW_LANE_LIST;

    sai_lane_attr.value.u32list.count = 1;
    sai_lane_attr.value.u32list.list = (uint32_t *)calloc (1, sizeof(uint32_t));

    ASSERT_TRUE(sai_lane_attr.value.u32list.list != NULL);

    for (count = 0;count < port_count;count ++) {
        ret = sai_port_api_table->get_port_attribute(port_list[count], 1, &sai_lane_attr);

        /* Handles buffer overflow case, by reallocating required memory */
        if(ret == SAI_STATUS_BUFFER_OVERFLOW ) {
            free(sai_lane_attr.value.u32list.list);
            sai_lane_attr.value.u32list.list = (uint32_t *)calloc (sai_lane_attr.value.u32list.count,
                    sizeof(uint32_t));
            ret = sai_port_api_table->get_port_attribute(port_list[count], 1, &sai_lane_attr);
        }

        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        for(lane = 0; lane < sai_lane_attr.value.u32list.count; lane++ ) {
            port_info[lane_count].hw_lane = sai_lane_attr.value.u32list.list[lane];
            port_info[lane_count].port_id = port_list[count];
            if(lane == 0) {
                port_info[lane_count].port_valid = true;
            }
            lane_count ++;
            LOG_PRINT("port 0x%" PRIx64 " lane %d is %d \r\n", port_list[count], lane,
                    sai_lane_attr.value.u32list.list[lane]);
        }
    }
    free(sai_lane_attr.value.u32list.list);
}

void portTest::sai_check_breakout_mode_supported(sai_object_id_t port,
                                                sai_port_breakout_mode_type_t mode,
                                                bool *supported)
{
    sai_attribute_t sai_attr_get;
    sai_status_t ret = SAI_STATUS_FAILURE;
    uint8_t count;
    *supported = false;

    memset(&sai_attr_get, 0, sizeof(sai_attr_get));
    sai_attr_get.id = SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE_TYPE;

    sai_attr_get.value.s32list.count = SAI_PORT_BREAKOUT_MODE_TYPE_MAX;
    sai_attr_get.value.s32list.list = (int32_t *)calloc (SAI_PORT_BREAKOUT_MODE_TYPE_MAX,
            sizeof(int32_t));
    ASSERT_TRUE(sai_attr_get.value.s32list.list != NULL);


    ret = sai_port_api_table->get_port_attribute(port, 1, &sai_attr_get);

    /* Handles buffer overflow case, by reallocating required memory */
    if(ret == SAI_STATUS_BUFFER_OVERFLOW ) {
        free(sai_attr_get.value.s32list.list);
        sai_attr_get.value.s32list.list = (int32_t *)calloc (sai_attr_get.value.s32list.count,
                sizeof(int32_t));
        ret = sai_port_api_table->get_port_attribute(port, 1, &sai_attr_get);
    }
    if(ret != SAI_STATUS_SUCCESS) {
        EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
        *supported = false;
        return;
    }
    for(count = 0; count < sai_attr_get.value.s32list.count; count++ ) {
        if(sai_attr_get.value.s32list.list[count] == mode) {
            *supported = true;
        }
    }

    free(sai_attr_get.value.u32list.list);
}

sai_status_t portTest::sai_set_flex_port_configuration(unsigned int port,
                                         sai_port_breakout_mode_type_t breakout_mode,
                                         uint32_t speed)
{
    sai_attribute_t sai_attr_set[2];
    sai_attribute_t sai_attr_get;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    int32_t current_mode = 0;
    uint32_t current_speed = 0;
    unsigned int count = 0;
    uint32_t port_lane_list[SAI_PORT_FOUR_LANES] = {0};

    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));
    memset(sai_attr_set, 0, sizeof(sai_attr_set));

    sai_attr_get.id = SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE;
    ret = sai_port_api_table->get_port_attribute(port_info[port].port_id, 1, &sai_attr_get);
    EXPECT_EQ(SAI_STATUS_SUCCESS, ret);

    current_mode = sai_attr_get.value.s32;
    if (current_mode == breakout_mode){
        memset(&sai_attr_get, 0, sizeof(sai_attribute_t));
        sai_attr_get.id = SAI_PORT_ATTR_SPEED;
        ret = sai_port_api_table->get_port_attribute(port_info[port].port_id, 1, &sai_attr_get);
        EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
        current_speed = sai_attr_get.value.u32;
        if (current_speed == speed) {
            return SAI_STATUS_SUCCESS;
        }
    }

    if(current_mode == SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE) {
        EXPECT_EQ(SAI_STATUS_SUCCESS,
                  sai_bridge_ut_clean_bridge_port_configs(switch_id, port_info[port].port_id));
        ret = sai_port_api_table->remove_port(port_info[port].port_id);
    } else if (current_mode == SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE) {
        for (count = 0; count < SAI_PORT_TWO_LANES; count ++) {
            EXPECT_EQ(SAI_STATUS_SUCCESS,
                      sai_bridge_ut_clean_bridge_port_configs(switch_id,
                      port_info[port+(count*SAI_PORT_FOUR_LANES)].port_id));
            ret = sai_port_api_table->remove_port(port_info[port+(count*SAI_PORT_FOUR_LANES)].port_id);
            if (ret != SAI_STATUS_SUCCESS){
                break;
            }
        }
    } else {
        for (count = 0; count < SAI_PORT_FOUR_LANES; count++) {
            EXPECT_EQ(SAI_STATUS_SUCCESS,
                  sai_bridge_ut_clean_bridge_port_configs(switch_id,
                  port_info[port+count].port_id));
            ret = sai_port_api_table->remove_port(port_info[port+count].port_id);
            if (ret != SAI_STATUS_SUCCESS){
                break;
            }
        }
    }
    EXPECT_EQ(SAI_STATUS_SUCCESS, ret);

    if (SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE == breakout_mode) {
        for (count = 0; count < SAI_PORT_FOUR_LANES; count++) {
            sai_attr_set[0].id = SAI_PORT_ATTR_HW_LANE_LIST;
            sai_attr_set[0].value.u32list.count = 1;
            sai_attr_set[0].value.u32list.list = &port_info[port+count].hw_lane;

            sai_attr_set[1].id = SAI_PORT_ATTR_SPEED;
            sai_attr_set[1].value.u32 = speed;
            ret = sai_port_api_table->create_port(&port_info[port + count].port_id,switch_id,2,sai_attr_set);
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            EXPECT_EQ(SAI_STATUS_SUCCESS,
                      sai_bridge_ut_init_bridge_port_configs(switch_id,
                                          port_info[port + count].port_id));
        }
    } else if (SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE == breakout_mode) {
        sai_attr_set[0].id = SAI_PORT_ATTR_HW_LANE_LIST;
        sai_attr_set[0].value.u32list.count = 4;
        for (count = 0; count < SAI_PORT_FOUR_LANES; count++) {
            port_lane_list[count] = port_info[port+count].hw_lane;
        }
        sai_attr_set[0].value.u32list.list = port_lane_list;

        sai_attr_set[1].id = SAI_PORT_ATTR_SPEED;
        sai_attr_set[1].value.u32 = speed;
        ret = sai_port_api_table->create_port(&port_info[port].port_id,switch_id,2,sai_attr_set);
        EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
        EXPECT_EQ(SAI_STATUS_SUCCESS,
                sai_bridge_ut_init_bridge_port_configs(switch_id,
                    port_info[port].port_id));
    } else if (SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE == breakout_mode) {
        for (count = 0; count < SAI_PORT_TWO_LANES; count++) {
            if (count == 1) {
                port = port + 1;
            }
            sai_attr_set[0].id = SAI_PORT_ATTR_HW_LANE_LIST;
            sai_attr_set[0].value.u32list.count = 2;
            sai_attr_set[0].value.u32list.list = &port_info[port + count].hw_lane;
            sai_attr_set[0].value.u32list.list = &port_info[port + count + 1].hw_lane;

            sai_attr_set[1].id = SAI_PORT_ATTR_SPEED;
            sai_attr_set[1].value.u32 = speed;
            ret = sai_port_api_table->create_port(&port_info[port + count * SAI_PORT_TWO_LANES].port_id,
                                                  switch_id,2,sai_attr_set);
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            EXPECT_EQ(SAI_STATUS_SUCCESS,
                    sai_bridge_ut_init_bridge_port_configs(switch_id,
                        port_info[port + count*SAI_PORT_TWO_LANES].port_id));
        }
    }

    return ret;
}

TEST_F(portTest, disable_port)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_set;
    unsigned int port_idx = 0;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));

    /* Check port Oper state as UP by enabling the internal Loopback */
    for(port_idx = 0; port_idx < port_count; port_idx++) {
        if(port_info[port_idx].port_valid != true) {
            continue;
        }
        /* Enable link admin state */
        sai_attr_set.id = SAI_PORT_ATTR_ADMIN_STATE;
        sai_attr_set.value.booldata = false;

        ret = sai_port_api_table->set_port_attribute(port_info[port_idx].port_id, &sai_attr_set);
        if(ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }
    }
}
TEST_F(portTest, breakout_4x10_set)
{

    sai_attribute_t sai_attr_set[2];
    sai_attribute_t sai_attr_get;
    unsigned int port_idx = 0,count =0;
    bool supported = false;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    int32_t current_mode = 0;


    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));
    memset(sai_attr_set, 0, sizeof(sai_attr_set));

    for (port_idx = 0; port_idx < SAI_MAX_PORTS; port_idx++) {
        if (port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_port_supported_speed_check(port_info[port_idx].port_id, SAI_PORT_SPEED_TEN_GIG, &supported);
        if (supported != true) {
            continue;
        }

        sai_check_breakout_mode_supported(port_info[port_idx].port_id,SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE,
                &supported);
        if (supported != true) {
            continue;
        }
        sai_attr_get.id = SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE;
        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        EXPECT_EQ(SAI_STATUS_SUCCESS, ret);

        /* This ports index is not possible in real time
         * added this check for avoid Out-of-bounds access */
        if(port_idx > SAI_PORT_MAX_ARRAY_INDEX) {
            continue;
        }

        current_mode = sai_attr_get.value.s32;
        if (current_mode == SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE){
            continue;
        }

        if (current_mode == SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE) {
            EXPECT_EQ(SAI_STATUS_SUCCESS,
                    sai_bridge_ut_clean_bridge_port_configs(switch_id, port_info[port_idx].port_id));
            ret = sai_port_api_table->remove_port(port_info[port_idx].port_id);
        } else { /*SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE*/
            for (count = 0; count < SAI_PORT_TWO_LANES; count ++) {
                EXPECT_EQ(SAI_STATUS_SUCCESS,
                        sai_bridge_ut_clean_bridge_port_configs(switch_id,
                        port_info[port_idx+(count*SAI_PORT_TWO_LANES)].port_id));
                ret = sai_port_api_table->remove_port(port_info[port_idx+(count*SAI_PORT_TWO_LANES)].port_id);
                if(ret != SAI_STATUS_SUCCESS){
                    break;
                }
            }
        }

        EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
        for (count = 0; count < SAI_PORT_FOUR_LANES; count++) {
            sai_attr_set[0].id = SAI_PORT_ATTR_HW_LANE_LIST;
            sai_attr_set[0].value.u32list.count = 1;
            sai_attr_set[0].value.u32list.list = &port_info[port_idx+count].hw_lane;

            sai_attr_set[1].id = SAI_PORT_ATTR_SPEED;
            sai_attr_set[1].value.u32 = SAI_PORT_SPEED_TEN_GIG;
            ret = sai_port_api_table->create_port(&port_info[port_idx+count].port_id,switch_id,2,sai_attr_set);
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            EXPECT_EQ(SAI_STATUS_SUCCESS,
                    sai_bridge_ut_init_bridge_port_configs(switch_id,
                        port_info[port_idx + count].port_id));
        }
    }
}

TEST_F(portTest, breakin_1x40_set)
{

    sai_attribute_t sai_attr_set[2];
    sai_attribute_t sai_attr_get;
    unsigned int port_idx = 0,count =0;
    bool supported = false;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    int32_t current_mode = 0;
    uint32_t port_lane_list[4] = {0};


    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));
    memset(sai_attr_set, 0, sizeof(sai_attr_set));

    for (port_idx = 0; port_idx < SAI_MAX_PORTS; port_idx++) {
        if (port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_port_supported_speed_check(port_info[port_idx].port_id, SAI_PORT_SPEED_FORTY_GIG, &supported);
        if (supported != true) {
            continue;
        }

        sai_check_breakout_mode_supported(port_info[port_idx].port_id,SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE,
                &supported);
        if (supported != true) {
            continue;
        }
        sai_attr_get.id = SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE;
        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        EXPECT_EQ(SAI_STATUS_SUCCESS, ret);

        /* This ports index is not possible in real time
         * added this check for avoid Out-of-bounds access */
        if(port_idx > SAI_PORT_MAX_ARRAY_INDEX) {
            continue;
        }
        current_mode = sai_attr_get.value.s32;
        if (current_mode == SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE){
            continue;
        }

        if (current_mode == SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE) {
            for (count = 0; count < SAI_PORT_TWO_LANES;count ++) {
                EXPECT_EQ(SAI_STATUS_SUCCESS,
                        sai_bridge_ut_clean_bridge_port_configs(switch_id,
                        port_info[port_idx+(count*SAI_PORT_TWO_LANES)].port_id));
                ret = sai_port_api_table->remove_port(port_info[port_idx+(count*SAI_PORT_TWO_LANES)].port_id);
                if (ret != SAI_STATUS_SUCCESS){
                    break;
                }
            }
        } else {
            for (count = 0; count < SAI_PORT_FOUR_LANES; count++) {
                EXPECT_EQ(SAI_STATUS_SUCCESS,
                        sai_bridge_ut_clean_bridge_port_configs(switch_id,
                        port_info[port_idx+count].port_id));
                ret = sai_port_api_table->remove_port(port_info[port_idx+count].port_id);
                if (ret != SAI_STATUS_SUCCESS){
                    break;
                }
            }
        }
        EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
        sai_attr_set[0].id = SAI_PORT_ATTR_HW_LANE_LIST;
        sai_attr_set[0].value.u32list.count = 4;
        for (count = 0; count < SAI_PORT_FOUR_LANES; count++) {
            port_lane_list[count] = port_info[port_idx+count].hw_lane;
        }
        sai_attr_set[0].value.u32list.list = port_lane_list;

        sai_attr_set[1].id = SAI_PORT_ATTR_SPEED;
        sai_attr_set[1].value.u32 = SAI_PORT_SPEED_FORTY_GIG;
        ret = sai_port_api_table->create_port(&port_info[port_idx].port_id,switch_id,2,sai_attr_set);
        EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
        EXPECT_EQ(SAI_STATUS_SUCCESS,
                sai_bridge_ut_init_bridge_port_configs(switch_id,
                    port_info[port_idx].port_id));
    }
}

TEST_F(portTest, fec_cl91_on_100g_set_get)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_get;
    sai_attribute_t sai_attr_set;
    unsigned int port_idx = 0;
    bool supported = false;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    for (port_idx = 0; port_idx < SAI_MAX_PORTS; port_idx++) {
        if (port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_port_supported_speed_check(port_info[port_idx].port_id,
                            SAI_PORT_SPEED_HUNDRED_GIG, &supported);
        if (supported != true) {
            continue;
        }

        sai_check_breakout_mode_supported(port_info[port_idx].port_id,SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE,
                                                            &supported);
        if (supported != true) {
            continue;
        }

        ret = sai_set_flex_port_configuration(port_idx, SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE,
                                                                   SAI_PORT_SPEED_HUNDRED_GIG);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_set.id = SAI_PORT_ATTR_FEC_MODE;
        sai_attr_set.value.s32 = SAI_PORT_FEC_MODE_RS;

        ret = sai_port_api_table->set_port_attribute(port_info[port_idx].port_id, &sai_attr_set);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_get.id = SAI_PORT_ATTR_FEC_MODE;
        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }
        EXPECT_EQ(SAI_PORT_FEC_MODE_RS, sai_attr_get.value.s32);
    }
}

TEST_F(portTest, fec_cl74_on_25g_set_get)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_get;
    sai_attribute_t sai_attr_set;
    unsigned int port_idx = 0;
    bool supported = false;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    for (port_idx = 0; port_idx < SAI_MAX_PORTS; port_idx++) {
        if (port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_port_supported_speed_check(port_info[port_idx].port_id,
                        SAI_PORT_SPEED_TWENTY_FIVE_GIG, &supported);
        if (supported != true) {
            continue;
        }

        sai_check_breakout_mode_supported(port_info[port_idx].port_id,SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE,
                &supported);
        if (supported != true) {
            continue;
        }

        ret = sai_set_flex_port_configuration(port_idx, SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE,
                                                SAI_PORT_SPEED_TWENTY_FIVE_GIG);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_set.id = SAI_PORT_ATTR_FEC_MODE;
        sai_attr_set.value.s32 = SAI_PORT_FEC_MODE_FC;

        ret = sai_port_api_table->set_port_attribute(port_info[port_idx].port_id, &sai_attr_set);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_get.id = SAI_PORT_ATTR_FEC_MODE;
        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }
        EXPECT_EQ(SAI_PORT_FEC_MODE_FC, sai_attr_get.value.s32);
    }
}

TEST_F(portTest, fec_cl74_on_50g_set_get)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_get;
    sai_attribute_t sai_attr_set;
    unsigned int port_idx = 0;
    bool supported = false;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    for (port_idx = 0; port_idx < SAI_MAX_PORTS; port_idx++) {
        if (port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_port_supported_speed_check(port_info[port_idx].port_id,
                                SAI_PORT_SPEED_FIFTY_GIG, &supported);
        if (supported != true) {
            continue;
        }

        sai_check_breakout_mode_supported(port_info[port_idx].port_id,SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE,
                                            &supported);
        if (supported != true) {
            continue;
        }

        ret = sai_set_flex_port_configuration(port_idx, SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE,
                                                SAI_PORT_SPEED_FIFTY_GIG);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_set.id = SAI_PORT_ATTR_FEC_MODE;
        sai_attr_set.value.s32 = SAI_PORT_FEC_MODE_FC;

        ret = sai_port_api_table->set_port_attribute(port_info[port_idx].port_id, &sai_attr_set);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_get.id = SAI_PORT_ATTR_FEC_MODE;
        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }
        EXPECT_EQ(SAI_PORT_FEC_MODE_FC, sai_attr_get.value.s32);
    }
}

TEST_F(portTest, oui_on_25g_set_get)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_get;
    sai_attribute_t sai_attr_set;
    unsigned int port_idx = 0;
    bool supported = false;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    for (port_idx = 0; port_idx < SAI_MAX_PORTS; port_idx++) {
        if (port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_port_supported_speed_check(port_info[port_idx].port_id,
                                 SAI_PORT_SPEED_TWENTY_FIVE_GIG, &supported);
        if (supported != true) {
            continue;
        }

        sai_check_breakout_mode_supported(port_info[port_idx].port_id, SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE,
                                            &supported);
        if (supported != true) {
            continue;
        }

        ret = sai_set_flex_port_configuration(port_idx, SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE,
                                                            SAI_PORT_SPEED_TWENTY_FIVE_GIG);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_set.id = SAI_PORT_ATTR_ADVERTISED_OUI_CODE;
        sai_attr_set.value.u32 = SAI_PORT_OUI_CODE_1;

        ret = sai_port_api_table->set_port_attribute(port_info[port_idx].port_id, &sai_attr_set);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_get.id = SAI_PORT_ATTR_ADVERTISED_OUI_CODE;
        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }
        EXPECT_EQ(SAI_PORT_OUI_CODE_1, sai_attr_get.value.u32);
    }
}

TEST_F(portTest, oui_on_50g_set_get)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_get;
    sai_attribute_t sai_attr_set;
    unsigned int port_idx = 0;
    bool supported = false;

    memset(&sai_attr_set, 0, sizeof(sai_attribute_t));
    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));

    for (port_idx = 0; port_idx < SAI_MAX_PORTS; port_idx++) {
        if (port_info[port_idx].port_valid != true) {
            continue;
        }

        sai_port_supported_speed_check(port_info[port_idx].port_id, SAI_PORT_SPEED_FIFTY_GIG, &supported);
        if (supported != true) {
            continue;
        }

        sai_check_breakout_mode_supported(port_info[port_idx].port_id, SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE,
                &supported);
        if (supported != true) {
            continue;
        }

        ret = sai_set_flex_port_configuration(port_idx, SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE, SAI_PORT_SPEED_FIFTY_GIG);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_set.id = SAI_PORT_ATTR_ADVERTISED_OUI_CODE;
        sai_attr_set.value.u32 = SAI_PORT_OUI_CODE_2;

        ret = sai_port_api_table->set_port_attribute(port_info[port_idx].port_id, &sai_attr_set);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }

        sai_attr_get.id = SAI_PORT_ATTR_ADVERTISED_OUI_CODE;
        ret = sai_port_api_table->get_port_attribute(port_info[port_idx].port_id, 1, &sai_attr_get);
        if (ret != SAI_STATUS_SUCCESS) {
            EXPECT_EQ(SAI_STATUS_SUCCESS, ret);
            continue;
        }
        EXPECT_EQ(SAI_PORT_OUI_CODE_2, sai_attr_get.value.u32);
    }
}
