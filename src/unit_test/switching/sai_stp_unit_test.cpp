/*
 * filename: sai_stp_unit_test.cpp
 * (c) Copyright 2015 Dell Inc. All Rights Reserved.
 */

#include "gtest/gtest.h"
#include "sai_stp_unit_test.h"

extern "C" {
#include "sai.h"
#include "saistatus.h"
#include "saitypes.h"
#include "saiswitch.h"
#include "saistp.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "stdarg.h"
}

#define SAI_STP_NO_OF_ATTRIB 1
#define SAI_STP_NO_OF_PORT_ATTRIB 3
#define SAI_STP_DEF_VLAN 1
#define SAI_STP_VLAN_2 2
#define SAI_STP_VLAN_3 3
#define SAI_STP_VLAN_10 10
#define SAI_MAX_PORTS 106

static uint32_t bridge_port_count = 0;
static sai_object_id_t switch_id =0;
static sai_object_id_t default_bridge_id =0;

static sai_object_id_t bridge_port_list[SAI_MAX_PORTS] = {0};

sai_object_id_t stpTest ::sai_stp_bridge_port_id_get (uint32_t port_index)
{
    if(port_index >= bridge_port_count) {
        return 0;
    }

    return bridge_port_list [port_index];
}

sai_object_id_t stpTest ::sai_stp_invalid_port_id_get ()
{
    return (SAI_NULL_OBJECT_ID);
}

static inline void sai_port_state_evt_callback (uint32_t count,
                                                sai_port_oper_status_notification_t *data)
{
}

static inline void sai_fdb_evt_callback (uint32_t count, sai_fdb_event_notification_data_t *data)
{
}

static inline void sai_switch_operstate_callback (sai_switch_oper_status_t
                                                  switchstate) {
}

/* Packet event callback
 */
static inline void sai_packet_event_callback (const void *buffer,
                                              sai_size_t buffer_size,
                                              uint32_t attr_count,
                                              const sai_attribute_t *attr_list)
{
}

static inline void  sai_switch_shutdown_callback (void) {
}

void stpTest ::SetUpTestCase (void) {
    /*
     * Query and populate the SAI Switch API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
               (SAI_API_SWITCH, (static_cast<void**>
                                 (static_cast<void*>(&p_sai_switch_api_tbl)))));

    ASSERT_TRUE (p_sai_switch_api_tbl != NULL);

    /*
     * Switch Initialization.
     */

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

    ASSERT_TRUE(p_sai_switch_api_tbl->create_switch != NULL);

    EXPECT_EQ (SAI_STATUS_SUCCESS,
               (p_sai_switch_api_tbl->create_switch (&switch_id , attr_count,
                                                         sai_attr_set)));
    /*
     * Query and populate the SAI Virtual Router API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
               (SAI_API_VLAN, (static_cast<void**>
                                         (static_cast<void*>
                                          (&p_sai_vlan_api_tbl)))));

    ASSERT_TRUE (p_sai_vlan_api_tbl != NULL);

    /*
     * Query and populate the SAI Virtual Router API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
               (SAI_API_STP, (static_cast<void**>
                                         (static_cast<void*>
                                          (&p_sai_stp_api_tbl)))));

    ASSERT_TRUE (p_sai_stp_api_tbl != NULL);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
               (SAI_API_BRIDGE, (static_cast<void**>
                                (static_cast<void*>(&p_sai_bridge_api_tbl)))));

    ASSERT_TRUE (p_sai_bridge_api_tbl != NULL);

    sai_attribute_t attr;
    sai_status_t ret = SAI_STATUS_SUCCESS;

    attr.id = SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID;

    ret = p_sai_switch_api_tbl->get_switch_attribute(switch_id,1,&attr);
    ASSERT_EQ (SAI_STATUS_SUCCESS, ret);
    default_bridge_id = attr.value.oid;

    attr.id = SAI_BRIDGE_ATTR_PORT_LIST;
    attr.value.objlist.count = SAI_MAX_PORTS;
    attr.value.objlist.list = bridge_port_list;

    ret = p_sai_bridge_api_tbl->get_bridge_attribute(default_bridge_id, 1, &attr);

    bridge_port_count = attr.value.objlist.count;

    ASSERT_EQ (SAI_STATUS_SUCCESS, ret);
    sai_bridge_port_id_1 = sai_stp_bridge_port_id_get (0);
}

sai_switch_api_t* stpTest ::p_sai_switch_api_tbl = NULL;
sai_bridge_api_t* stpTest ::p_sai_bridge_api_tbl = NULL;
sai_vlan_api_t* stpTest ::p_sai_vlan_api_tbl = NULL;
sai_stp_api_t* stpTest ::p_sai_stp_api_tbl = NULL;
sai_object_id_t stpTest ::sai_bridge_port_id_1 = 0;

bool sai_stp_ut_all_bridge_ports_in_default_stp(sai_stp_api_t *p_sai_stp_api_tbl,
                                                uint32_t count, sai_object_id_t *stp_port_list)
{
    uint32_t bp_idx = 0;
    uint32_t sp_idx = 0;
    sai_attribute_t attr;

    for(bp_idx = 0 ; bp_idx < bridge_port_count;  bp_idx++) {
        for(sp_idx = 0; sp_idx < count; sp_idx++) {
            attr.id = SAI_STP_PORT_ATTR_BRIDGE_PORT;
            p_sai_stp_api_tbl->get_stp_port_attribute(stp_port_list[sp_idx],1,&attr);
            if(attr.value.oid == bridge_port_list[bp_idx]) {
                printf("STPport 0x%" PRIx64 " Bridge port 0x%" PRIx64 "\r\n",
                       stp_port_list[sp_idx],bridge_port_list[bp_idx]);
                break;
            }
        }
        if(sp_idx == count) {
            printf("Bridge port id 0x%" PRIx64 " is not in default stp\r\n",
                   bridge_port_list[bp_idx]);
            return false;
        }
    }
    return true;
}

TEST_F(stpTest, def_stp_get)
{
    sai_attribute_t attr[SAI_STP_NO_OF_ATTRIB] = {0};
    sai_object_id_t def_stp_id = 0;
    sai_vlan_id_t   vlan_id = SAI_STP_VLAN_10;

    attr[0].id =  SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID;
    attr[0].value.u16 = 0;

    EXPECT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,p_sai_switch_api_tbl->
                  set_switch_attribute(switch_id,attr));

    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_switch_api_tbl->
                  get_switch_attribute(switch_id,SAI_STP_NO_OF_ATTRIB, attr));

    def_stp_id = attr[0].value.oid;

    attr[0].id = SAI_STP_ATTR_VLAN_LIST;
    attr[0].value.vlanlist.count = 1;
    attr[0].value.vlanlist.list = &vlan_id;

    EXPECT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,p_sai_stp_api_tbl->
                  set_stp_attribute(def_stp_id, attr));

    attr[0].value.vlanlist.count = 1;
    attr[0].value.vlanlist.list = (sai_vlan_id_t *) calloc (attr[0].value.vlanlist.count,
                                                             sizeof(sai_vlan_id_t));

    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_stp_api_tbl->
                  get_stp_attribute(def_stp_id, SAI_STP_NO_OF_ATTRIB, attr));

    EXPECT_EQ (attr[0].value.vlanlist.count, 1);
    EXPECT_EQ (attr[0].value.vlanlist.list[0], SAI_STP_DEF_VLAN);

    free (attr[0].value.vlanlist.list);
}

TEST_F(stpTest, default_stp_port_test)
{
    sai_attribute_t attr;
    sai_object_id_t stp_member_list[SAI_MAX_PORTS];
    sai_object_id_t default_stp_id;

    attr.id = SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_switch_api_tbl->get_switch_attribute(switch_id,1,&attr));


    default_stp_id = attr.value.oid;
    attr.id = SAI_STP_ATTR_PORT_LIST;
    attr.value.objlist.count = SAI_MAX_PORTS;
    attr.value.objlist.list = stp_member_list;

    EXPECT_EQ(SAI_STATUS_SUCCESS,
              p_sai_stp_api_tbl->get_stp_attribute(default_stp_id,1,
                                                     &attr));
    ASSERT_EQ(true,
              sai_stp_ut_all_bridge_ports_in_default_stp(p_sai_stp_api_tbl,
                                                         attr.value.objlist.count,
                                                         stp_member_list));
}

TEST_F(stpTest, create_stp_group)
{
    sai_attribute_t attr[SAI_STP_NO_OF_ATTRIB] = {0};
    sai_object_id_t stp_id = 0;
    sai_object_id_t def_inst_id = 0;
    sai_object_id_t vlan_obj_id = 0;

    attr[SAI_STP_NO_OF_ATTRIB - 1].id =  SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID;

    EXPECT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,p_sai_switch_api_tbl->
                  set_switch_attribute(switch_id,attr));

    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_switch_api_tbl->
                  get_switch_attribute(switch_id,SAI_STP_NO_OF_ATTRIB, attr));

    def_inst_id = attr[0].value.oid;

    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_stp_api_tbl->
            create_stp(&stp_id,switch_id,0,attr));

    attr[0].id = SAI_VLAN_ATTR_VLAN_ID;
    attr[0].value.u16 = SAI_STP_VLAN_2;
    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_vlan_api_tbl->
            create_vlan(&vlan_obj_id,1,SAI_STP_NO_OF_ATTRIB,attr));

    attr[0].id = SAI_VLAN_ATTR_STP_INSTANCE;
    attr[0].value.oid = stp_id;

    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_vlan_api_tbl->
            set_vlan_attribute(vlan_obj_id,attr));

    EXPECT_EQ(SAI_STATUS_OBJECT_IN_USE, p_sai_stp_api_tbl->
            remove_stp(stp_id));

    attr[0].id = SAI_VLAN_ATTR_STP_INSTANCE;
    attr[0].value.oid = def_inst_id;

    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_vlan_api_tbl->
            set_vlan_attribute(vlan_obj_id,attr));

    EXPECT_EQ(SAI_STATUS_SUCCESS, p_sai_stp_api_tbl->
            remove_stp(stp_id));

    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_vlan_api_tbl->
            remove_vlan(vlan_obj_id));
}

TEST_F(stpTest, stp_port_state)
{
    sai_attribute_t attr[SAI_STP_NO_OF_PORT_ATTRIB] = {0};
    sai_object_id_t stp_id = 0;
    sai_object_id_t stp_port_id_1 = 0;
    sai_object_id_t invalid_port_id = sai_stp_invalid_port_id_get();

    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_stp_api_tbl->
            create_stp(&stp_id,0,0,attr));

    attr[0].id = SAI_STP_PORT_ATTR_STP;
    attr[0].value.oid = stp_id;
    attr[1].id = SAI_STP_PORT_ATTR_BRIDGE_PORT;
    attr[1].value.oid = invalid_port_id;
    attr[2].id = SAI_STP_PORT_ATTR_STATE;
    attr[2].value.s32 = SAI_STP_PORT_STATE_BLOCKING;
    EXPECT_EQ(SAI_STATUS_INVALID_ATTR_VALUE_0+SAI_STATUS_CODE(1), p_sai_stp_api_tbl->
            create_stp_port(&stp_port_id_1, switch_id, SAI_STP_NO_OF_PORT_ATTRIB, attr));

    attr[0].id = SAI_STP_PORT_ATTR_STP;
    attr[0].value.oid = 10;
    attr[1].id = SAI_STP_PORT_ATTR_BRIDGE_PORT;
    attr[1].value.oid = invalid_port_id;
    attr[2].id = SAI_STP_PORT_ATTR_STATE;
    attr[2].value.s32 = SAI_STP_PORT_STATE_BLOCKING;
    EXPECT_EQ(SAI_STATUS_INVALID_ATTR_VALUE_0, p_sai_stp_api_tbl->
            create_stp_port(&stp_port_id_1, switch_id, SAI_STP_NO_OF_PORT_ATTRIB, attr));

    attr[0].id = SAI_STP_PORT_ATTR_STP;
    attr[0].value.oid = stp_id;
    attr[1].id = SAI_STP_PORT_ATTR_BRIDGE_PORT;
    attr[1].value.oid = sai_bridge_port_id_1;
    attr[2].id = SAI_STP_PORT_ATTR_STATE;
    attr[2].value.s32 = 3;
    EXPECT_EQ(SAI_STATUS_INVALID_ATTR_VALUE_0+SAI_STATUS_CODE(2), p_sai_stp_api_tbl->
            create_stp_port(&stp_port_id_1, switch_id, SAI_STP_NO_OF_PORT_ATTRIB, attr));

    EXPECT_EQ(SAI_STATUS_INVALID_OBJECT_ID, p_sai_stp_api_tbl->
            remove_stp_port(stp_port_id_1));

    attr[0].id = SAI_STP_PORT_ATTR_STP;
    attr[0].value.oid = stp_id;
    attr[1].id = SAI_STP_PORT_ATTR_BRIDGE_PORT;
    attr[1].value.oid = sai_bridge_port_id_1;
    attr[2].id = SAI_STP_PORT_ATTR_STATE;
    attr[2].value.s32 = SAI_STP_PORT_STATE_LEARNING;
    EXPECT_EQ(SAI_STATUS_SUCCESS, p_sai_stp_api_tbl->
            create_stp_port(&stp_port_id_1, switch_id, SAI_STP_NO_OF_PORT_ATTRIB, attr));

    attr[0].id = SAI_STP_PORT_ATTR_STATE;
    EXPECT_EQ(SAI_STATUS_SUCCESS, p_sai_stp_api_tbl->
            get_stp_port_attribute(stp_port_id_1,SAI_STP_NO_OF_ATTRIB,attr));
    EXPECT_EQ(attr[0].value.s32,SAI_STP_PORT_STATE_LEARNING);

    attr[0].id = SAI_STP_PORT_ATTR_STATE;
    attr[0].value.s32 = SAI_STP_PORT_STATE_FORWARDING;
    EXPECT_EQ(SAI_STATUS_SUCCESS, p_sai_stp_api_tbl->
            set_stp_port_attribute(stp_port_id_1,&attr[0]));

    attr[0].id = SAI_STP_PORT_ATTR_STATE;
    EXPECT_EQ(SAI_STATUS_SUCCESS, p_sai_stp_api_tbl->
            get_stp_port_attribute(stp_port_id_1,SAI_STP_NO_OF_ATTRIB,attr));
    EXPECT_EQ(attr[0].value.s32,SAI_STP_PORT_STATE_FORWARDING);

    attr[0].id = SAI_STP_PORT_ATTR_STATE;
    attr[0].value.s32 = SAI_STP_PORT_STATE_BLOCKING;
    EXPECT_EQ(SAI_STATUS_SUCCESS, p_sai_stp_api_tbl->
            set_stp_port_attribute(stp_port_id_1,&attr[0]));

    attr[0].id = SAI_STP_PORT_ATTR_STATE;
    EXPECT_EQ(SAI_STATUS_SUCCESS, p_sai_stp_api_tbl->
            get_stp_port_attribute(stp_port_id_1,SAI_STP_NO_OF_ATTRIB,attr));
    EXPECT_EQ(attr[0].value.s32,SAI_STP_PORT_STATE_BLOCKING);

    EXPECT_EQ(SAI_STATUS_OBJECT_IN_USE, p_sai_stp_api_tbl->
            remove_stp(stp_id));

    EXPECT_EQ(SAI_STATUS_SUCCESS, p_sai_stp_api_tbl->
            remove_stp_port(stp_port_id_1));

    EXPECT_EQ(SAI_STATUS_ITEM_NOT_FOUND, p_sai_stp_api_tbl->
            remove_stp_port(stp_port_id_1));

    EXPECT_EQ(SAI_STATUS_SUCCESS, p_sai_stp_api_tbl->
            remove_stp(stp_id));
}

TEST_F(stpTest, delet_vlan)
{
    sai_attribute_t attr[SAI_STP_NO_OF_ATTRIB] = {0};
    sai_object_id_t stp_id = 0;
    sai_object_id_t vlan_obj_id1 = 0;
    sai_object_id_t vlan_obj_id2 = 0;

    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_stp_api_tbl->
            create_stp(&stp_id,0,0,attr));

    attr[0].id = SAI_VLAN_ATTR_VLAN_ID;
    attr[0].value.u16 = SAI_STP_VLAN_2;
    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_vlan_api_tbl->
            create_vlan(&vlan_obj_id1,1,SAI_STP_NO_OF_ATTRIB,attr));

    attr[0].id = SAI_VLAN_ATTR_VLAN_ID;
    attr[0].value.u16 = SAI_STP_VLAN_3;
    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_vlan_api_tbl->
            create_vlan(&vlan_obj_id2,1,SAI_STP_NO_OF_ATTRIB,attr));

    attr[0].id = SAI_VLAN_ATTR_STP_INSTANCE;
    attr[0].value.oid = stp_id;

    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_vlan_api_tbl->
            set_vlan_attribute(vlan_obj_id2,attr));

    attr[0].id = SAI_VLAN_ATTR_STP_INSTANCE;
    attr[0].value.oid = 10;

    EXPECT_EQ(SAI_STATUS_INVALID_OBJECT_ID, p_sai_vlan_api_tbl->
            set_vlan_attribute(vlan_obj_id1, attr));

    attr[0].id = SAI_VLAN_ATTR_STP_INSTANCE;
    attr[0].value.oid = stp_id;

    EXPECT_EQ(SAI_STATUS_SUCCESS, p_sai_vlan_api_tbl->
            set_vlan_attribute(vlan_obj_id1, attr));

    memset (attr, 0, sizeof(attr));
    attr[0].id = SAI_VLAN_ATTR_STP_INSTANCE;

    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_vlan_api_tbl->
            get_vlan_attribute(vlan_obj_id2, SAI_STP_NO_OF_ATTRIB, attr));

    EXPECT_EQ (attr->value.oid, stp_id);

    memset (attr, 0, sizeof(attr));
    attr[0].id = SAI_VLAN_ATTR_STP_INSTANCE;

    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_vlan_api_tbl->
            get_vlan_attribute(vlan_obj_id1, SAI_STP_NO_OF_ATTRIB, attr));

    EXPECT_EQ (attr->value.oid, stp_id);

    memset (attr, 0, sizeof(attr));
    attr[0].id = SAI_STP_ATTR_VLAN_LIST;
    attr[0].value.vlanlist.count = 1;
    attr[0].value.vlanlist.list = (sai_vlan_id_t *) calloc (attr[0].value.vlanlist.count,
                                       sizeof(sai_vlan_id_t));

    EXPECT_EQ(SAI_STATUS_BUFFER_OVERFLOW, p_sai_stp_api_tbl->
            get_stp_attribute(stp_id, SAI_STP_NO_OF_ATTRIB, attr));

    EXPECT_EQ (attr->value.vlanlist.count, SAI_STP_VLAN_2);

    attr[0].id = SAI_STP_ATTR_VLAN_LIST;
    attr[0].value.vlanlist.list = (sai_vlan_id_t *) realloc (attr[0].value.vlanlist.list,
                                       sizeof(sai_vlan_id_t));

    EXPECT_EQ(SAI_STATUS_SUCCESS, p_sai_stp_api_tbl->
            get_stp_attribute(stp_id, SAI_STP_NO_OF_ATTRIB, attr));

    EXPECT_EQ (attr->value.vlanlist.count, 2);
    EXPECT_EQ (attr->value.vlanlist.list[0], SAI_STP_VLAN_2);
    EXPECT_EQ (attr->value.vlanlist.list[1], SAI_STP_VLAN_3);

    free (attr[0].value.vlanlist.list);

    EXPECT_EQ(SAI_STATUS_OBJECT_IN_USE, p_sai_stp_api_tbl->
            remove_stp(stp_id));

    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_vlan_api_tbl->
            remove_vlan(vlan_obj_id2));

    EXPECT_EQ(SAI_STATUS_SUCCESS,p_sai_vlan_api_tbl->
            remove_vlan(vlan_obj_id1));

    EXPECT_EQ(SAI_STATUS_SUCCESS, p_sai_stp_api_tbl->
            remove_stp(stp_id));

    EXPECT_EQ(SAI_STATUS_INVALID_OBJECT_ID, p_sai_stp_api_tbl->
            remove_stp(stp_id));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
