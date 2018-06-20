/*
 * filename:
"src/unit_test/sai_bridge_unit_test.cpp
 * (c) Copyright 2017 Dell Inc. All Rights Reserved.
 */

/*
 * sai_bridge_unit_test.cpp
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include <inttypes.h>

#include "sai_bridge_unit_test_utils.h"
#include "sai_bridge_unit_test.h"

extern "C" {
#include "sai.h"
#include "saitypes.h"
#include "saibridge.h"
#include "saiswitch.h"
#include "sailag.h"
#include "saifdb.h"
#include "saivlan.h"
}


uint32_t bridge_port_count = 0;
sai_object_id_t bridge_port_list[SAI_MAX_BRIDGE_PORTS] = {0};
sai_object_id_t switch_id = 0;
sai_object_id_t default_bridge_id = 0;
sai_object_id_t port_list[SAI_MAX_BRIDGE_PORTS] = {0};
uint32_t port_count = 0;
sai_bridge_api_t* bridgeTest::p_sai_bridge_api_tbl = NULL;
sai_switch_api_t* bridgeTest::p_sai_switch_api_tbl = NULL;
sai_lag_api_t* bridgeTest::p_sai_lag_api_tbl = NULL;
sai_fdb_api_t* bridgeTest::p_sai_fdb_api_tbl = NULL;
sai_l2mc_group_api_t* bridgeTest::p_sai_l2mc_group_api_tbl = NULL;
sai_vlan_api_t* bridgeTest::p_sai_vlan_api_tbl = NULL;


static bool sai_bridge_ut_check_if_port_has_bridge_port(sai_bridge_api_t* p_sai_bridge_api_tbl,
                                                        sai_object_id_t port_id)
{
    uint32_t bridge_port_idx = 0;
    sai_attribute_t attr;

    for(bridge_port_idx = 0 ; bridge_port_idx < bridge_port_count; bridge_port_idx++) {
        attr.id = SAI_BRIDGE_PORT_ATTR_PORT_ID;

        p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_list[bridge_port_idx], 1,
                                                        &attr);
        if(attr.value.oid == port_id) {
            printf("Port 0x%" PRIx64 " - Bridge port 0x%" PRIx64 "\r\n",port_id,
                   bridge_port_list[bridge_port_idx]);
            return true;
        }
    }
    printf("Port 0x%" PRIx64 " has no bridge port\r\n", port_id);
    return false;

}


/*
 * Bridge defaults
 */
TEST_F(bridgeTest, test_defaults)
{
    uint32_t port_idx = 0;

    ASSERT_NE(default_bridge_id, SAI_NULL_OBJECT_ID);

    for(port_idx =0; port_idx < port_count; port_idx++) {
        ASSERT_EQ(true,
                  sai_bridge_ut_check_if_port_has_bridge_port(p_sai_bridge_api_tbl,
                                                              port_list[port_idx]));
    }
}

TEST_F(bridgeTest, 1q_bridge_create)
{
    sai_attribute_t attr;
    sai_object_id_t bridge_id = SAI_NULL_OBJECT_ID;

    /* First try to override the switch attribute */
    attr.id = SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_switch_api_tbl->set_switch_attribute(switch_id, &attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
               p_sai_switch_api_tbl->get_switch_attribute(switch_id,1,&attr));

    ASSERT_EQ(default_bridge_id, attr.value.oid);

    /* Create 1Q Bridge */
    attr.id = SAI_BRIDGE_ATTR_TYPE;
    attr.value.s32 = SAI_BRIDGE_TYPE_1Q;

    ASSERT_EQ(SAI_STATUS_ITEM_ALREADY_EXISTS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              p_sai_bridge_api_tbl->remove_bridge(default_bridge_id));
}

TEST_F(bridgeTest, 1q_bridge_attribute_set)
{
    sai_attribute_t attr;
    sai_attribute_t get_attr;
    sai_status_t    sai_rc;

    attr.id = SAI_BRIDGE_ATTR_TYPE;
    attr.value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_attribute(default_bridge_id, &attr));

    attr.id = SAI_BRIDGE_ATTR_PORT_LIST;
    attr.value.objlist.count = bridge_port_count;
    attr.value.objlist.list = bridge_port_list;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_attribute(default_bridge_id, &attr));

    attr.id = SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES;
    attr.value.s32 = 10;
    sai_rc = p_sai_bridge_api_tbl->set_bridge_attribute(default_bridge_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_attribute(default_bridge_id, 1, &get_attr));
        ASSERT_EQ(attr.value.s32, get_attr.value.s32);
        printf("Attribute SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES is set to 10 successfully\r\n");
    }

    attr.id = SAI_BRIDGE_ATTR_LEARN_DISABLE;
    attr.value.booldata = true;
    sai_rc = p_sai_bridge_api_tbl->set_bridge_attribute(default_bridge_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting SAI_BRIDGE_ATTR_LEARN_DISABLE\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_ATTR_LEARN_DISABLE;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_attribute(default_bridge_id, 1, &get_attr));
        ASSERT_EQ(attr.value.booldata, get_attr.value.booldata);
        printf("Attribute SAI_BRIDGE_ATTR_LEARN_DISABLE is set to true successfully\r\n");
    }

    attr.id = SAI_BRIDGE_ATTR_END;
    ASSERT_EQ(SAI_STATUS_UNKNOWN_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_attribute(default_bridge_id, &attr));

}

TEST_F(bridgeTest, 1q_bridge_stats)
{
    sai_status_t      sai_rc;
    sai_bridge_stat_t bridge_stat[] = {SAI_BRIDGE_STAT_IN_OCTETS, SAI_BRIDGE_STAT_IN_PACKETS,
                                       SAI_BRIDGE_STAT_OUT_OCTETS, SAI_BRIDGE_STAT_OUT_PACKETS};

    unsigned int      num_stats = sizeof(bridge_stat)/ sizeof(sai_bridge_stat_t);
    unsigned int      stat_idx = 0;
    uint64_t          val = 0;

    for(stat_idx = 0; stat_idx < num_stats; stat_idx++) {
        sai_rc = p_sai_bridge_api_tbl->get_bridge_stats(default_bridge_id, 1,
                                                        &bridge_stat[stat_idx],&val);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            printf("Stat id %d  get returns error %d for default bridge\r\n",
                   bridge_stat[stat_idx], sai_rc);
        } else {
            printf("Stat id %d value %ld for default bridge\r\n", bridge_stat[stat_idx], val);
        }
    }

    for(stat_idx = 0; stat_idx < num_stats; stat_idx++) {
        sai_rc = p_sai_bridge_api_tbl->clear_bridge_stats(default_bridge_id, 1,
                                                        &bridge_stat[stat_idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            printf("Stat id %d clear returns error %d for default bridge\r\n",
                   bridge_stat[stat_idx], sai_rc);
        } else {
            ASSERT_EQ(SAI_STATUS_SUCCESS, p_sai_bridge_api_tbl->get_bridge_stats(default_bridge_id, 1,
                                                            &bridge_stat[stat_idx],&val));
            ASSERT_EQ(val, 0);
        }
    }

}

TEST_F(bridgeTest, 1q_bridge_port_create_remove)
{
    sai_object_id_t port_id;
    sai_object_id_t bridge_port_id;
    sai_attribute_t attr[2];

    bridge_port_id = bridge_port_list[0];

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_get_port_from_bridge_port(p_sai_bridge_api_tbl, bridge_port_id,
                                                      &port_id));


    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_PORT;

    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id, switch_id, 1, attr));

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = port_id;

    ASSERT_EQ(SAI_STATUS_ITEM_ALREADY_EXISTS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id, switch_id, 2, attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               bridge_port_list[0], true));


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id,
                                               true, &bridge_port_id));

    bridge_port_list[0] = bridge_port_id;
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 "",
           port_id, bridge_port_id);
}

TEST_F(bridgeTest, 1q_bridge_port_attribute_set)
{
    sai_attribute_t attr;
    sai_attribute_t get_attr;
    sai_status_t    sai_rc;
    sai_object_id_t bridge_port_id = bridge_port_list[0];

    attr.id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr.value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    attr.id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr.value.oid = port_list[1];

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    attr.id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr.value.oid = SAI_BRIDGE_GTEST_VLAN;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    attr.id = SAI_BRIDGE_PORT_ATTR_RIF_ID;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    attr.id = SAI_BRIDGE_PORT_ATTR_TUNNEL_ID;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    attr.id = SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES;
    attr.value.s32 = 10;

    sai_rc = p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id, 1, &get_attr));
        ASSERT_EQ(attr.value.s32, get_attr.value.s32);
        printf("Attribute SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES is set to 10 successfully\r\n");
    }

    attr.id = SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE;
    attr.value.s32 = SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DROP;

    sai_rc = p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id, 1, &get_attr));
        ASSERT_EQ(attr.value.s32, get_attr.value.s32);
        printf("Attribute SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE is set to drop successfully\r\n");
    }
    attr.id = SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION;
    attr.value.s32 = SAI_PACKET_ACTION_DROP;

    sai_rc = p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting "
               "SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id, 1, &get_attr));
        ASSERT_EQ(attr.value.s32, get_attr.value.s32);
        printf("Attribute SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION "
               "is set to drop successfully\r\n");
    }

    attr.id = SAI_BRIDGE_PORT_ATTR_ADMIN_STATE;
    attr.value.booldata = true;

    sai_rc = p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting SAI_BRIDGE_PORT_ATTR_ADMIN_STATE\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_PORT_ATTR_ADMIN_STATE;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id, 1, &get_attr));
        ASSERT_EQ(attr.value.booldata, get_attr.value.booldata);
        printf("Attribute SAI_BRIDGE_PORT_ATTR_ADMIN_STATE is set to true successfully\r\n");
        attr.value.booldata = false;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));
    }

    attr.id = SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING;
    attr.value.booldata = true;

    sai_rc = p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id, 1, &get_attr));
        ASSERT_EQ(attr.value.booldata, get_attr.value.booldata);
        printf("Attribute SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING is set to true successfully\r\n");
    }

    attr.id = SAI_BRIDGE_PORT_ATTR_EGRESS_FILTERING;
    attr.value.booldata = true;

    sai_rc = p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting SAI_BRIDGE_PORT_ATTR_EGRESS_FILTERING\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_PORT_ATTR_EGRESS_FILTERING;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id, 1, &get_attr));
        ASSERT_EQ(attr.value.booldata, get_attr.value.booldata);
        printf("Attribute SAI_BRIDGE_PORT_ATTR_EGRESS_FILTERING is set to true successfully\r\n");
    }

    attr.id = SAI_BRIDGE_PORT_ATTR_END;
    ASSERT_EQ(SAI_STATUS_UNKNOWN_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

}

TEST_F(bridgeTest, 1q_bridge_port_remove_condition)
{
    sai_attribute_t attr;
    sai_object_id_t bridge_port_id = bridge_port_list[0];
    sai_object_id_t port_id = port_list[0];
    sai_attribute_t fdb_attr[4];
    sai_fdb_entry_t fdb_entry;


    attr.id = SAI_BRIDGE_PORT_ATTR_ADMIN_STATE;
    attr.value.booldata = true;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    ASSERT_NE(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));

    fdb_attr[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
    fdb_attr[0].value.s32 = SAI_FDB_ENTRY_TYPE_STATIC;

    fdb_attr[1].id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID;
    fdb_attr[1].value.oid = bridge_port_id;

    fdb_attr[2].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION;
    fdb_attr[2].value.s32 = SAI_PACKET_ACTION_DROP;

    fdb_entry.bv_id = SAI_BRIDGE_GTEST_VLAN_OBJ;
    fdb_entry.mac_address[5] = 0xe;


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_fdb_api_tbl->create_fdb_entry(&fdb_entry, 3, fdb_attr));

    ASSERT_NE(SAI_STATUS_SUCCESS,
              sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               bridge_port_id, true));
    attr.value.booldata = false;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               bridge_port_id, true));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_fdb_api_tbl->remove_fdb_entry(&fdb_entry));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               bridge_port_id, true));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               port_id, true, &bridge_port_id));

    bridge_port_list[0] = bridge_port_id;
}

TEST_F(bridgeTest, 1q_bridge_port_stats)
{
    sai_status_t           sai_rc;
    sai_bridge_port_stat_t bridge_port_stat[] = {SAI_BRIDGE_PORT_STAT_IN_OCTETS,
                                                 SAI_BRIDGE_PORT_STAT_IN_PACKETS,
                                                 SAI_BRIDGE_PORT_STAT_OUT_OCTETS,
                                                 SAI_BRIDGE_PORT_STAT_OUT_PACKETS};

    unsigned int           num_stats = sizeof(bridge_port_stat)/ sizeof(sai_bridge_port_stat_t);
    unsigned int           stat_idx = 0;
    uint64_t               val = 0;

    for(stat_idx = 0; stat_idx < num_stats; stat_idx++) {
        sai_rc = p_sai_bridge_api_tbl->get_bridge_port_stats(bridge_port_list[0], 1,
                                                        &bridge_port_stat[stat_idx],&val);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            printf("Stat id %d  get returns error %d for bridge port 0x%" PRIx64 "\r\n",
                   bridge_port_stat[stat_idx], sai_rc, bridge_port_list[0]);
        } else {
            printf("Stat id %d value %ld for default bridge port 0x%" PRIx64 "\r\n",
                  bridge_port_stat[stat_idx], val, bridge_port_list[0]);
        }
    }

    for(stat_idx = 0; stat_idx < num_stats; stat_idx++) {
        sai_rc = p_sai_bridge_api_tbl->clear_bridge_port_stats(bridge_port_list[0], 1,
                                                        &bridge_port_stat[stat_idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            printf("Stat id %d clear returns error %d for default bridge_port 0x%" PRIx64 "\r\n",
                   bridge_port_stat[stat_idx], sai_rc, bridge_port_list[0]);
        } else {
            ASSERT_EQ(SAI_STATUS_SUCCESS,
                      p_sai_bridge_api_tbl->get_bridge_port_stats(bridge_port_list[0], 1,
                                                            &bridge_port_stat[stat_idx],&val));
            ASSERT_EQ(val, 0);
        }
    }
}


TEST_F(bridgeTest, 1d_bridge_create_remove)
{
    sai_attribute_t attr;
    sai_object_id_t bridge_id = SAI_NULL_OBJECT_ID;

    attr.id = SAI_BRIDGE_ATTR_LEARN_DISABLE;
    attr.value.booldata = true;

    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &attr));

    attr.id = SAI_BRIDGE_ATTR_TYPE;
    attr.value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));

    ASSERT_EQ(SAI_STATUS_INVALID_OBJECT_ID,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));
}

TEST_F(bridgeTest, 1d_bridge_attribute_set)
{
    sai_attribute_t attr;
    sai_attribute_t get_attr;
    sai_status_t    sai_rc;
    sai_object_id_t bridge_id = SAI_NULL_OBJECT_ID;

    attr.id = SAI_BRIDGE_ATTR_TYPE;
    attr.value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &attr));

    attr.id = SAI_BRIDGE_ATTR_TYPE;
    attr.value.s32 = SAI_BRIDGE_TYPE_1Q;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id, &attr));

    attr.id = SAI_BRIDGE_ATTR_PORT_LIST;
    attr.value.objlist.count = bridge_port_count;
    attr.value.objlist.list = bridge_port_list;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id, &attr));

    attr.id = SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES;

    attr.value.s32 = 10;
    sai_rc = p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_attribute(bridge_id, 1, &get_attr));
        ASSERT_EQ(attr.value.s32, get_attr.value.s32);
        printf("Attribute SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES is set to 10 successfully\r\n");
    }

    attr.id = SAI_BRIDGE_ATTR_LEARN_DISABLE;
    attr.value.booldata = true;
    sai_rc = p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting SAI_BRIDGE_ATTR_LEARN_DISABLE\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_ATTR_LEARN_DISABLE;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_attribute(bridge_id, 1, &get_attr));
        ASSERT_EQ(attr.value.booldata, get_attr.value.booldata);
        printf("Attribute SAI_BRIDGE_ATTR_LEARN_DISABLE is set to true successfully\r\n");
    }

    attr.id = SAI_BRIDGE_ATTR_END;
    ASSERT_EQ(SAI_STATUS_UNKNOWN_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id, &attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));

}

TEST_F(bridgeTest, 1d_bridge_stats)
{
    sai_object_id_t   bridge_id;
    sai_status_t      sai_rc;
    sai_attribute_t   attr;
    sai_bridge_stat_t bridge_stat[] = {SAI_BRIDGE_STAT_IN_OCTETS, SAI_BRIDGE_STAT_IN_PACKETS,
                                       SAI_BRIDGE_STAT_OUT_OCTETS, SAI_BRIDGE_STAT_OUT_PACKETS};
    unsigned int      num_stats = sizeof(bridge_stat)/ sizeof(sai_bridge_stat_t);
    unsigned int      stat_idx = 0;
    uint64_t          val = 0;

    attr.id = SAI_BRIDGE_ATTR_TYPE;
    attr.value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &attr));

    for(stat_idx = 0; stat_idx < num_stats; stat_idx++) {
        sai_rc = p_sai_bridge_api_tbl->get_bridge_stats(bridge_id, 1,
                                                        &bridge_stat[stat_idx],&val);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            printf("Stat id %d  get returns error %d for bridge 0x%" PRIx64 "\r\n",
                   bridge_stat[stat_idx], sai_rc, bridge_id);
        } else {
            printf("Stat id %d value %ld for bridge 0x%" PRIx64 "\r\n",
                  bridge_stat[stat_idx], val, bridge_id);
        }
    }

    for(stat_idx = 0; stat_idx < num_stats; stat_idx++) {
        sai_rc = p_sai_bridge_api_tbl->clear_bridge_stats(bridge_id, 1,
                                                        &bridge_stat[stat_idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            printf("Stat id %d clear returns error %d for bridge 0x%" PRIx64 "\r\n",
                   bridge_stat[stat_idx], sai_rc, bridge_id);
        } else {
            ASSERT_EQ(SAI_STATUS_SUCCESS, p_sai_bridge_api_tbl->get_bridge_stats(bridge_id, 1,
                                                            &bridge_stat[stat_idx],&val));
            ASSERT_EQ(val, 0);
        }
    }

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));

}

TEST_F(bridgeTest, 1d_bridge_port_create_remove)
{
    sai_object_id_t port_id = port_list[0];
    sai_vlan_id_t   vlan_id = SAI_BRIDGE_GTEST_VLAN;
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t bridge_port_id_1 = SAI_NULL_OBJECT_ID;
    sai_attribute_t attr[4];
    sai_object_id_t bridge_id = SAI_NULL_OBJECT_ID;

    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &attr[0]));


    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = port_id;


    attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[2].value.oid = bridge_id;

    ASSERT_EQ(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id, switch_id, 3, attr));

    attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr[3].value.u16 = vlan_id;

    attr[2].value.oid = default_bridge_id;

    ASSERT_NE(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id, switch_id, 4, attr));

    attr[2].value.oid = bridge_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id, switch_id, 4, attr));
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           port_id, bridge_port_id, vlan_id);

    ASSERT_EQ(SAI_STATUS_ITEM_ALREADY_EXISTS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id_1, switch_id, 4, attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));

    ASSERT_EQ(SAI_STATUS_INVALID_OBJECT_ID,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));
    /* Check again for non duplicate creation */

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id_1, switch_id, 4, attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));

}

TEST_F(bridgeTest, lag_bridge_port_create_remove)
{
    sai_object_id_t lag_id;
    sai_object_id_t port_id1;
    sai_object_id_t port_id2;
    sai_object_id_t lag_1d_bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t lag_1q_bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t bridge_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t lag_member_id1;
    sai_object_id_t lag_member_id2;
    sai_vlan_id_t   vlan_id = SAI_BRIDGE_GTEST_VLAN;
    sai_attribute_t attr[4];

    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &attr[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_get_port_from_bridge_port(p_sai_bridge_api_tbl, bridge_port_list[0],
                                                      &port_id1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_get_port_from_bridge_port(p_sai_bridge_api_tbl, bridge_port_list[1],
                                                      &port_id2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               bridge_port_list[0], true));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               bridge_port_list[1], true));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->create_lag(&lag_id, switch_id, 0, NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_id,
                                               true, &lag_1q_bridge_port_id));

    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = lag_id;

    attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[2].value.oid = bridge_id;

    attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr[3].value.u16 = vlan_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&lag_1d_bridge_port_id, switch_id, 4, attr));
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           lag_id, lag_1d_bridge_port_id, vlan_id);

    attr[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attr[0].value.oid = lag_id;

    attr[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attr[1].value.oid = port_id1;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->create_lag_member (&lag_member_id1, switch_id, 2, attr));

    attr[1].value.oid = port_id2;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->create_lag_member (&lag_member_id2, switch_id, 2, attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              p_sai_lag_api_tbl->remove_lag(lag_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->remove_lag_member(lag_member_id1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->remove_lag_member(lag_member_id2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id1,
                                               true, &bridge_port_list[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id2,
                                               true, &bridge_port_list[1]));
    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              p_sai_lag_api_tbl->remove_lag(lag_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id,
                                               lag_1q_bridge_port_id, true));
    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              p_sai_lag_api_tbl->remove_lag(lag_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(lag_1d_bridge_port_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_lag_api_tbl->remove_lag(lag_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));
}

TEST_F(bridgeTest, 1d_bridge_port_list_get)
{
    sai_object_id_t port_id = port_list[0];
    sai_object_id_t port_id_1 = port_list[1];
    sai_vlan_id_t   vlan_id = SAI_BRIDGE_GTEST_VLAN;
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t bridge_port_id_1 = SAI_NULL_OBJECT_ID;
    sai_attribute_t attr[4];
    sai_object_id_t bridge_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t tmp_bridge_port_list[2];
    sai_attribute_t get_attr;

    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &attr[0]));


    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = port_id;


    attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[2].value.oid = bridge_id;


    attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr[3].value.u16 = vlan_id;

    get_attr.id = SAI_BRIDGE_ATTR_PORT_LIST;
    get_attr.value.objlist.count = 2;
    get_attr.value.objlist.list = tmp_bridge_port_list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->get_bridge_attribute(bridge_id, 1, &get_attr));

    ASSERT_EQ(get_attr.value.objlist.count, 0);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id, switch_id, 4, attr));

    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           port_id, bridge_port_id, vlan_id);

    get_attr.value.objlist.count = 2;
    get_attr.value.objlist.list = tmp_bridge_port_list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->get_bridge_attribute(bridge_id, 1, &get_attr));

    ASSERT_EQ(get_attr.value.objlist.count, 1);
    ASSERT_EQ(tmp_bridge_port_list[0], bridge_port_id);

    attr[1].value.oid = port_id_1;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id_1, switch_id, 4, attr));

    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           port_id_1, bridge_port_id_1, vlan_id);

    get_attr.value.objlist.count = 2;
    get_attr.value.objlist.list = tmp_bridge_port_list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->get_bridge_attribute(bridge_id, 1, &get_attr));

    ASSERT_EQ(get_attr.value.objlist.count, 2);

    ASSERT_TRUE(((bridge_port_id_1 == tmp_bridge_port_list[0]) &&
                 (bridge_port_id == tmp_bridge_port_list[1])) ||
                ((bridge_port_id_1 == tmp_bridge_port_list[1]) &&
                 (bridge_port_id == tmp_bridge_port_list[0])));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));

    get_attr.value.objlist.count = 2;
    get_attr.value.objlist.list = tmp_bridge_port_list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->get_bridge_attribute(bridge_id, 1, &get_attr));

    ASSERT_EQ(get_attr.value.objlist.count, 1);
    ASSERT_EQ(tmp_bridge_port_list[0], bridge_port_id_1);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id_1));

    get_attr.value.objlist.count = 2;
    get_attr.value.objlist.list = tmp_bridge_port_list;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->get_bridge_attribute(bridge_id, 1, &get_attr));

    ASSERT_EQ(get_attr.value.objlist.count, 0);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));

}

TEST_F(bridgeTest, 1d_bridge_port_attribute_set)
{
    sai_attribute_t attr;
    sai_attribute_t get_attr;
    sai_status_t    sai_rc;
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_attribute_t c_attr[4];
    sai_object_id_t bridge_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t port_id = port_list[0];
    sai_vlan_id_t   vlan_id = SAI_BRIDGE_GTEST_VLAN;

    c_attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    c_attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &c_attr[0]));

    c_attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    c_attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    c_attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    c_attr[1].value.oid = port_id;


    c_attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    c_attr[2].value.oid = bridge_id;


    c_attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    c_attr[3].value.u16 = vlan_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id, switch_id, 4, c_attr));


    attr.id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr.value.s32 = SAI_BRIDGE_PORT_TYPE_TUNNEL;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    attr.id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr.value.oid = port_list[1];

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    attr.id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr.value.oid = SAI_BRIDGE_GTEST_VLAN;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    attr.id = SAI_BRIDGE_PORT_ATTR_RIF_ID;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    attr.id = SAI_BRIDGE_PORT_ATTR_TUNNEL_ID;

    ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    attr.id = SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES;
    attr.value.s32 = 10;

    sai_rc = p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id, 1, &get_attr));
        ASSERT_EQ(attr.value.s32, get_attr.value.s32);
        printf("Attribute SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES is set to 10 successfully\r\n");
    }

    attr.id = SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE;
    attr.value.s32 = SAI_BRIDGE_PORT_FDB_LEARNING_MODE_DROP;

    sai_rc = p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id, 1, &get_attr));
        ASSERT_EQ(attr.value.s32, get_attr.value.s32);
        printf("Attribute SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE is set to drop successfully\r\n");
    }
    attr.id = SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION;
    attr.value.s32 = SAI_PACKET_ACTION_DROP;

    sai_rc = p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting "
               "SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id, 1, &get_attr));
        ASSERT_EQ(attr.value.s32, get_attr.value.s32);
        printf("Attribute SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION "
               "is set to drop successfully\r\n");
    }

    attr.id = SAI_BRIDGE_PORT_ATTR_ADMIN_STATE;
    attr.value.booldata = true;

    sai_rc = p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting SAI_BRIDGE_PORT_ATTR_ADMIN_STATE\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_PORT_ATTR_ADMIN_STATE;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id, 1, &get_attr));
        ASSERT_EQ(attr.value.booldata, get_attr.value.booldata);
        printf("Attribute SAI_BRIDGE_PORT_ATTR_ADMIN_STATE is set to true successfully\r\n");
        attr.value.booldata = false;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));
    }

    attr.id = SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING;
    attr.value.booldata = true;

    sai_rc = p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d is returned on setting SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING\r\n",
               sai_rc);
    } else {
        get_attr.id = SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING;
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id, 1, &get_attr));
        ASSERT_EQ(attr.value.booldata, get_attr.value.booldata);
        printf("Attribute SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING is set to true successfully\r\n");
    }

    attr.id = SAI_BRIDGE_PORT_ATTR_END;
    ASSERT_EQ(SAI_STATUS_UNKNOWN_ATTRIBUTE_0,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));

}

TEST_F(bridgeTest, 1d_bridge_port_remove_condition)
{
    sai_attribute_t attr;
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_attribute_t c_attr[4];
    sai_object_id_t bridge_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t port_id = port_list[0];
    sai_vlan_id_t   vlan_id = SAI_BRIDGE_GTEST_VLAN;
    sai_attribute_t fdb_attr[4];
    sai_fdb_entry_t fdb_entry;

    c_attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    c_attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &c_attr[0]));

    c_attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    c_attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    c_attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    c_attr[1].value.oid = port_id;


    c_attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    c_attr[2].value.oid = bridge_id;


    c_attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    c_attr[3].value.u16 = vlan_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id, switch_id, 4, c_attr));

    attr.id = SAI_BRIDGE_PORT_ATTR_ADMIN_STATE;
    attr.value.booldata = true;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    ASSERT_NE(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));

    fdb_attr[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
    fdb_attr[0].value.s32 = SAI_FDB_ENTRY_TYPE_STATIC;

    fdb_attr[1].id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID;
    fdb_attr[1].value.oid = bridge_port_id;

    fdb_attr[2].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION;
    fdb_attr[2].value.s32 = SAI_PACKET_ACTION_DROP;

    fdb_entry.bv_id = bridge_id;
    fdb_entry.mac_address[5] = 0xe;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_fdb_api_tbl->create_fdb_entry(&fdb_entry, 3, fdb_attr));

    ASSERT_NE(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));

    attr.value.booldata = false;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->set_bridge_port_attribute(bridge_port_id, &attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_fdb_api_tbl->remove_fdb_entry(&fdb_entry));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));

}


TEST_F(bridgeTest, 1d_bridge_port_stats)
{
    sai_status_t           sai_rc;
    sai_bridge_port_stat_t bridge_port_stat[] = {SAI_BRIDGE_PORT_STAT_IN_OCTETS,
                                                 SAI_BRIDGE_PORT_STAT_IN_PACKETS,
                                                 SAI_BRIDGE_PORT_STAT_OUT_OCTETS,
                                                 SAI_BRIDGE_PORT_STAT_OUT_PACKETS};

    unsigned int           num_stats = sizeof(bridge_port_stat)/ sizeof(sai_bridge_port_stat_t);
    unsigned int           stat_idx = 0;
    uint64_t               val = 0;
    sai_object_id_t        port_id = port_list[0];
    sai_vlan_id_t          vlan_id = SAI_BRIDGE_GTEST_VLAN;
    sai_object_id_t        bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_attribute_t        attr[4];
    sai_object_id_t        bridge_id = SAI_NULL_OBJECT_ID;

    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &attr[0]));

    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = port_id;

    attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[2].value.oid = bridge_id;

    attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr[3].value.u16 = vlan_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id, switch_id, 4, attr));
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           port_id, bridge_port_id, vlan_id);

    for(stat_idx = 0; stat_idx < num_stats; stat_idx++) {
        sai_rc = p_sai_bridge_api_tbl->get_bridge_port_stats(bridge_port_id, 1,
                                                        &bridge_port_stat[stat_idx],&val);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            printf("Stat id %d  get returns error %d for bridge port 0x%" PRIx64 "\r\n",
                   bridge_port_stat[stat_idx], sai_rc, bridge_port_id);
        } else {
            printf("Stat id %d value %ld for bridge port 0x%" PRIx64 "\r\n",
                  bridge_port_stat[stat_idx], val, bridge_port_id);
        }
    }

    for(stat_idx = 0; stat_idx < num_stats; stat_idx++) {
        sai_rc = p_sai_bridge_api_tbl->clear_bridge_port_stats(bridge_port_id, 1,
                                                        &bridge_port_stat[stat_idx]);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            printf("Stat id %d clear returns error %d for bridge_port 0x%" PRIx64 "\r\n",
                   bridge_port_stat[stat_idx], sai_rc, bridge_port_id);
        } else {
            ASSERT_EQ(SAI_STATUS_SUCCESS,
                      p_sai_bridge_api_tbl->get_bridge_port_stats(bridge_port_id, 1,
                                                            &bridge_port_stat[stat_idx],&val));
            ASSERT_EQ(val, 0);
        }
    }

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));

}

TEST_F(bridgeTest, 1d_multicast_flood_type)
{
    unsigned int           idx = 0;
    sai_object_id_t        port_id = port_list[0];
    sai_vlan_id_t          vlan_id = SAI_BRIDGE_GTEST_VLAN;
    sai_object_id_t        bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_attribute_t        attr[4];
    sai_object_id_t        bridge_id = SAI_NULL_OBJECT_ID;

    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id, switch_id, 1, &attr[0]));

    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = port_id;

    attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[2].value.oid = bridge_id;

    attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr[3].value.u16 = vlan_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id, switch_id, 4, attr));
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           port_id, bridge_port_id, vlan_id);

    attr[0].id  = SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE;
    attr[1].id  = SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE;
    attr[2].id  = SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->get_bridge_attribute(bridge_id, 3, attr));

    ASSERT_EQ(attr[0].value.s32, SAI_BRIDGE_FLOOD_CONTROL_TYPE_SUB_PORTS);
    ASSERT_EQ(attr[1].value.s32, SAI_BRIDGE_FLOOD_CONTROL_TYPE_SUB_PORTS);
    ASSERT_EQ(attr[2].value.s32, SAI_BRIDGE_FLOOD_CONTROL_TYPE_SUB_PORTS);

    attr[0].value.s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_NONE;
    attr[1].value.s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_NONE;
    attr[2].value.s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_NONE;

    for(idx = 0; idx< 3; idx++) {
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id, &attr[idx]));
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_attribute(bridge_id, 1, &attr[idx]));
        ASSERT_EQ(attr[idx].value.s32, SAI_BRIDGE_FLOOD_CONTROL_TYPE_NONE);

    }

    attr[0].value.s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP;
    attr[1].value.s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP;
    attr[2].value.s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP;

    for(idx = 0; idx< 3; idx++) {
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id, &attr[idx]));
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  p_sai_bridge_api_tbl->get_bridge_attribute(bridge_id, 1, &attr[idx]));
        ASSERT_EQ(attr[idx].value.s32, SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP);

    }

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id));


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id));

}

TEST_F(bridgeTest, 1d_multicast_flood_groups)
{
    sai_object_id_t        port_id_1 = port_list[0];
    sai_object_id_t        port_id_2 = port_list[1];
    sai_object_id_t        port_id_3 = port_list[2];
    sai_vlan_id_t          vlan_id = SAI_BRIDGE_GTEST_VLAN;
    sai_object_id_t        bridge_port_id_1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t        bridge_port_id_2 = SAI_NULL_OBJECT_ID;
    sai_object_id_t        bridge_port_id_3 = SAI_NULL_OBJECT_ID;
    sai_attribute_t        attr[4];
    sai_object_id_t        bridge_id_1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t        bridge_id_2 = SAI_NULL_OBJECT_ID;
    sai_object_id_t        l2mc_obj_id_1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t        l2mc_obj_id_2 = SAI_NULL_OBJECT_ID;
    sai_object_id_t        l2mc_member_id = SAI_NULL_OBJECT_ID;

    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id_1, switch_id, 1, &attr[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id_2, switch_id, 1, &attr[0]));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->create_l2mc_group(&l2mc_obj_id_1,switch_id,0,attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->create_l2mc_group(&l2mc_obj_id_2,switch_id,0,attr));

    /*Case 1: Add l2mc_group before bridge ports are added. Before flood type is set*/

    attr[0].id = SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP;
    attr[0].value.oid = l2mc_obj_id_1;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    attr[0].value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    /*Case 2: Add l2mc_group after flood type is set */

    attr[0].id = SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_L2MC_GROUP;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    attr[0].id = SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP;
    attr[0].value.oid = l2mc_obj_id_1;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));
    attr[0].value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    /*Case 3: Add l2mc group after bridge port is created */
    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = port_id_1;

    attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[2].value.oid = bridge_id_1;

    attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr[3].value.u16 = vlan_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id_1, switch_id, 4, attr));
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           port_id_1, bridge_port_id_1, vlan_id);

    attr[1].value.oid = port_id_2;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id_2, switch_id, 4, attr));
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           port_id_2, bridge_port_id_2, vlan_id);

    attr[2].value.oid = bridge_id_2;
    attr[1].value.oid = port_id_3;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id_3, switch_id, 4, attr));
    printf("Bridge port id 0x%" PRIx64 " is created for port 0x%" PRIx64 " vlan %d\r\n",
           port_id_2, bridge_port_id_3, vlan_id);


    attr[0].id = SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP;
    attr[0].value.oid = l2mc_obj_id_1;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    attr[0].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID;
    attr[0].value.oid = l2mc_obj_id_1;
    attr[1].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID;
    attr[1].value.oid = bridge_port_list[0];

    /*Case 4: Check if creating member fails if bridge port used belongs to a 1Q bridge */
    ASSERT_NE(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->create_l2mc_group_member(&l2mc_member_id, switch_id,
                                                                 2, attr));

    attr[1].value.oid = bridge_port_id_3;

    /*Case 5: Check if creating member fails if bridge port used belongs to a different 1D bridge */
    ASSERT_NE(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->create_l2mc_group_member(&l2mc_member_id, switch_id,
                                                                 2, attr));

    /*Case 6: Check if creating member is success if bridge port used belongs to same bridge */
    attr[1].value.oid = bridge_port_id_1;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->create_l2mc_group_member(&l2mc_member_id, switch_id,
                                                                 2, attr));


    /*Case 7: Check if adding the multicast group to a new bridge fails that is already part of a different bridge*/
    attr[0].id = SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP;
    attr[0].value.oid = l2mc_obj_id_1;
    ASSERT_NE(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_2, &attr[0]));

    /*Case 8: Check if deleting multicast group fails if it is attached to a bridge*/
    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              p_sai_l2mc_group_api_tbl->remove_l2mc_group(l2mc_obj_id_1));


    /*Case 9: Add two different groups for two different flood types*/
    attr[0].id = SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP;
    attr[0].value.oid = l2mc_obj_id_2;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
            p_sai_bridge_api_tbl->set_bridge_attribute(bridge_id_1, &attr[0]));

    ASSERT_NE(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id_2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id_3));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->remove_l2mc_group_member(l2mc_member_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id_2));

    /*Case 10: Check if L2MC group can be deleted after removing the bridge */
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->remove_l2mc_group(l2mc_obj_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_l2mc_group_api_tbl->remove_l2mc_group(l2mc_obj_id_2));

}

TEST_F(bridgeTest, vlan_member_sub_port_check)
{
    sai_object_id_t        port_id_1 = port_list[0];
    sai_object_id_t        bridge_port_id_1 = bridge_port_list[0];
    sai_attribute_t        attr[4];
    sai_vlan_id_t          vlan_id = SAI_BRIDGE_GTEST_VLAN;
    sai_object_id_t        vlan_obj_id;
    sai_object_id_t        vlan_member_id;
    sai_object_id_t        bridge_port_id_2;
    sai_object_id_t        bridge_id_1;

    attr[0].id = SAI_BRIDGE_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_TYPE_1D;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge(&bridge_id_1, switch_id, 1, &attr[0]));

    attr[0].id = SAI_VLAN_ATTR_VLAN_ID;
    attr[0].value.u16 = vlan_id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_vlan_api_tbl->create_vlan(&vlan_obj_id, switch_id, 1, attr));

    attr[0].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr[0].value.oid = vlan_obj_id;

    attr[1].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr[1].value.oid = bridge_port_id_1;

    attr[2].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr[2].value.u32 = SAI_VLAN_TAGGING_MODE_TAGGED;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_vlan_api_tbl->create_vlan_member(&vlan_member_id, switch_id, 3, attr));

    attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_SUB_PORT;

    attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    attr[1].value.oid = port_id_1;

    attr[2].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    attr[2].value.oid = bridge_id_1;

    attr[3].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    attr[3].value.u16 = vlan_id;

    /* Check if bridge port cannot be created if a vlan member already exists on the
       port with same vlan */
    ASSERT_NE(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id_2, switch_id, 4, attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_vlan_api_tbl->remove_vlan_member(vlan_member_id));


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->create_bridge_port(&bridge_port_id_2, switch_id, 4, attr));


    attr[0].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr[0].value.oid = vlan_obj_id;

    attr[1].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr[1].value.oid = bridge_port_id_1;

    attr[2].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr[2].value.u32 = SAI_VLAN_TAGGING_MODE_TAGGED;

    /* Check if vlan member cannot be created if sub port already exists with same port and vlan */
    ASSERT_NE(SAI_STATUS_SUCCESS,
              p_sai_vlan_api_tbl->create_vlan_member(&vlan_member_id, switch_id, 3, attr));


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id_2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_bridge_api_tbl->remove_bridge(bridge_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_vlan_api_tbl->create_vlan_member(&vlan_member_id, switch_id, 3, attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_vlan_api_tbl->remove_vlan_member(vlan_member_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_vlan_api_tbl->remove_vlan(vlan_obj_id));

}
