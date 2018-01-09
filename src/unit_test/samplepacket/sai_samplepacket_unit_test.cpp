/*
 * filename: sai_samplepacket_unit_test.cpp
 * (c) Copyright 2015 Dell Inc. All Rights Reserved.
 */

#include "gtest/gtest.h"
#include "stdarg.h"
#include "sai_samplepacket_unit_test.h"

extern "C" {
#include "sai.h"
#include "saiacl.h"
#include "saistatus.h"
#include "saitypes.h"
#include "saiswitch.h"
#include "saisamplepacket.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
}
#define SAI_SAMPLE_NO_OF_MANDAT_ATTRIB 1

TEST_F(samplepacketTest, sample_set) {
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  session_id = 0;
    sai_attribute_t attr[SAI_SAMPLE_NO_OF_MANDAT_ATTRIB + 1] = {0};
    sai_attribute_t port_attr;
    sai_object_id_t samplepacket_port = port_id_2;

    attr[0].id =  SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    attr[0].value.u32 = 100;

    sai_rc = sai_test_samplepacket_session_create (&session_id, SAI_SAMPLE_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (attr, 0, sizeof(attr));
    attr[0].id = SAI_SAMPLEPACKET_ATTR_TYPE;
    attr[1].id = SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    sai_rc = p_sai_samplepacket_api_tbl->get_samplepacket_attribute (session_id,
            SAI_SAMPLE_NO_OF_MANDAT_ATTRIB + 1 ,attr);


    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (attr[0].value.s32, SAI_SAMPLEPACKET_TYPE_SLOW_PATH);
    EXPECT_EQ (attr[1].value.u32, 100);

    memset (attr, 0, sizeof(attr));

    sai_rc = sai_test_samplepacket_session_ingress_port_add (samplepacket_port, session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    port_attr.id = SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE;

    sai_rc = p_sai_port_api_tbl->get_port_attribute (samplepacket_port, 1, &port_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (port_attr.value.oid, session_id);

    sai_rc = sai_test_samplepacket_session_egress_port_add (samplepacket_port, session_id);

    port_attr.id = SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE;

    sai_rc = p_sai_port_api_tbl->get_port_attribute (samplepacket_port, 1, &port_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (port_attr.value.oid, session_id);

    attr[0].id =  SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    attr[0].value.oid = 1024;
    sai_rc = p_sai_samplepacket_api_tbl->set_samplepacket_attribute (session_id, attr);


    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    memset (attr, 0, sizeof(attr));
    attr[0].id = SAI_SAMPLEPACKET_ATTR_TYPE;
    attr[1].id =  SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    sai_rc = p_sai_samplepacket_api_tbl->get_samplepacket_attribute (session_id, SAI_SAMPLE_NO_OF_MANDAT_ATTRIB + 1 ,attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (attr[0].value.s32, SAI_SAMPLEPACKET_TYPE_SLOW_PATH);
    EXPECT_EQ (attr[1].value.oid, 1024);

    attr[0].id =  SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    attr[0].value.oid = 512;
    sai_rc = p_sai_samplepacket_api_tbl->set_samplepacket_attribute (session_id, attr);


    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    memset (attr, 0, sizeof(attr));
    attr[0].id = SAI_SAMPLEPACKET_ATTR_TYPE;
    attr[1].id =  SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    sai_rc = p_sai_samplepacket_api_tbl->get_samplepacket_attribute (session_id, SAI_SAMPLE_NO_OF_MANDAT_ATTRIB + 1 ,attr);


    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (attr[0].value.s32, SAI_SAMPLEPACKET_TYPE_SLOW_PATH);
    EXPECT_EQ (attr[1].value.oid, 512);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_samplepacket_session_ingress_port_add (samplepacket_port, SAI_NULL_OBJECT_ID);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_samplepacket_session_egress_port_add (samplepacket_port, SAI_NULL_OBJECT_ID);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    port_attr.id = SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE;

    sai_rc = p_sai_port_api_tbl->get_port_attribute (samplepacket_port, 1, &port_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (port_attr.value.oid, SAI_NULL_OBJECT_ID);

    sai_rc = sai_test_samplepacket_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

TEST_F(samplepacketTest, object_in_use) {
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_object_id_t  session_id = 0;
    sai_object_id_t  session_id_dup = 0;
    sai_attribute_t attr[SAI_SAMPLE_NO_OF_MANDAT_ATTRIB + 1] = {0};
    sai_attribute_t port_attr;
    sai_object_id_t samplepacket_port = port_id_2;

    attr[0].id =  SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    attr[0].value.u32 = 100;

    sai_rc = sai_test_samplepacket_session_create (&session_id, SAI_SAMPLE_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr[0].id =  SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    attr[0].value.u32 = 200;

    sai_rc = sai_test_samplepacket_session_create (&session_id_dup, SAI_SAMPLE_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    memset (attr, 0, sizeof(attr));

    sai_rc = sai_test_samplepacket_session_ingress_port_add (samplepacket_port, session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    port_attr.id = SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE;

    sai_rc = p_sai_port_api_tbl->get_port_attribute (samplepacket_port, 1, &port_attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    EXPECT_EQ (port_attr.value.oid, session_id);

    sai_rc = sai_test_samplepacket_session_ingress_port_add (samplepacket_port, session_id_dup);

    EXPECT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);

    sai_rc = sai_test_samplepacket_session_ingress_port_add (samplepacket_port, SAI_NULL_OBJECT_ID);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);


    sai_rc = sai_test_samplepacket_session_ingress_port_add (samplepacket_port, session_id_dup);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_samplepacket_session_ingress_port_add (samplepacket_port, SAI_NULL_OBJECT_ID);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_samplepacket_session_egress_port_add (samplepacket_port, session_id_dup);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_samplepacket_session_egress_port_add (samplepacket_port, SAI_NULL_OBJECT_ID);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    sai_rc = sai_test_samplepacket_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_samplepacket_session_destroy (session_id_dup);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

TEST_F(samplepacketTest, flow_based) {
    sai_object_id_t acl_table_id;
    sai_object_id_t acl_rule_id;
    sai_object_id_t acl_rule_id2;
    sai_object_id_t session_id;
    sai_attribute_t rule_attr[10] = {0};
    sai_attribute_t table_attr[12] = {0};
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr[SAI_SAMPLE_NO_OF_MANDAT_ATTRIB + 1] = {0};

    attr[0].id =  SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    attr[0].value.u32 = 2048;

    sai_rc = sai_test_samplepacket_session_create (&session_id, SAI_SAMPLE_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

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
    sai_rc = p_sai_acl_api_tbl->create_acl_table (&acl_table_id, switch_id, 12,
                                                  table_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    rule_attr[0].value.oid = acl_table_id;
    rule_attr[1].id =  SAI_ACL_ENTRY_ATTR_PRIORITY;
    rule_attr[1].value.u32 = 1;
    rule_attr[2].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    rule_attr[2].value.booldata= true;
    rule_attr[3].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    rule_attr[3].value.aclfield.enable = true;
    rule_attr[3].value.aclfield.data.objlist.count = 1;
    rule_attr[3].value.aclfield.data.objlist.list = (sai_object_id_t *) calloc(
                                                            1, sizeof(sai_object_id_t));
    rule_attr[3].value.aclfield.data.objlist.list[0] = port_id_1;
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE;
    rule_attr[4].value.aclaction.enable = true;
    rule_attr[4].value.aclaction.parameter.oid = session_id;
    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    EXPECT_EQ (SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->remove_acl_entry(acl_rule_id));
    free (rule_attr[3].value.aclfield.data.objlist.list);

    /* Without INPORTS */
    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    rule_attr[0].value.oid = acl_table_id;
    rule_attr[1].id =  SAI_ACL_ENTRY_ATTR_PRIORITY;
    rule_attr[1].value.u32 = 1;
    rule_attr[2].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    rule_attr[2].value.booldata= true;
    rule_attr[3].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    rule_attr[3].value.aclfield.enable = true;
    rule_attr[3].value.aclfield.data.u8 = 17;
    rule_attr[3].value.aclfield.mask.u8 = 0xff;
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE;
    rule_attr[4].value.aclaction.enable = true;
    rule_attr[4].value.aclaction.parameter.oid = session_id;
    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    /* Without INPORTS */
    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    rule_attr[0].value.oid = acl_table_id;
    rule_attr[1].id =  SAI_ACL_ENTRY_ATTR_PRIORITY;
    rule_attr[1].value.u32 = 1;
    rule_attr[2].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    rule_attr[2].value.booldata= true;
    rule_attr[3].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    rule_attr[3].value.aclfield.enable = true;
    rule_attr[3].value.aclfield.data.u8 = 17;
    rule_attr[3].value.aclfield.mask.u8 = 0xff;
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE;
    rule_attr[4].value.aclaction.enable = true;
    rule_attr[4].value.aclaction.parameter.oid = session_id;
    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id2, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    EXPECT_EQ (SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->remove_acl_entry(acl_rule_id));
    EXPECT_EQ (SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->remove_acl_entry(acl_rule_id2));

    EXPECT_EQ (SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->remove_acl_table (acl_table_id));

    sai_rc = sai_test_samplepacket_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

TEST_F(samplepacketTest, flow_based_set) {
    sai_object_id_t acl_table_id;
    sai_object_id_t acl_rule_id;
    sai_object_id_t acl_rule_id2;
    sai_object_id_t session_id;
    sai_object_id_t session_id_dup;
    sai_attribute_t rule_attr[10] = {0};
    sai_attribute_t set_rule_attr;
    sai_attribute_t table_attr[12] = {0};
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr[SAI_SAMPLE_NO_OF_MANDAT_ATTRIB + 1] = {0};

    attr[0].id =  SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    attr[0].value.u32 = 2048;

    sai_rc = sai_test_samplepacket_session_create (&session_id, SAI_SAMPLE_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr[0].id =  SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    attr[0].value.u32 = 100;

    sai_rc = sai_test_samplepacket_session_create (&session_id_dup, SAI_SAMPLE_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
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
    sai_rc = p_sai_acl_api_tbl->create_acl_table (&acl_table_id, switch_id, 12,
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
    rule_attr[3].value.aclfield.data.objlist.list[0] = port_id_1;
    rule_attr[3].value.aclfield.enable = true;
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE;
    rule_attr[4].value.aclaction.enable = true;
    rule_attr[4].value.aclaction.parameter.oid = session_id;
    rule_attr[5].id = SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE;
    rule_attr[5].value.aclaction.enable = true;
    rule_attr[5].value.aclaction.parameter.oid = session_id;
    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id, switch_id, 6, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    free (rule_attr[3].value.aclfield.data.objlist.list);

    set_rule_attr.id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    set_rule_attr.value.aclfield.data.objlist.count = 1;
    set_rule_attr.value.aclfield.data.objlist.list = (sai_object_id_t *) calloc(
                                                            1, sizeof(sai_object_id_t));
    set_rule_attr.value.aclfield.data.objlist.list[0] = port_id_2;
    set_rule_attr.value.aclfield.enable = true;
    sai_rc = p_sai_acl_api_tbl->set_acl_entry_attribute (acl_rule_id, &set_rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    free (set_rule_attr.value.aclfield.data.objlist.list);

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
    rule_attr[3].value.aclfield.data.objlist.list[0] = port_id_1;
    rule_attr[3].value.aclfield.enable = true;
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE;
    rule_attr[4].value.aclaction.enable = true;
    rule_attr[4].value.aclaction.parameter.oid = session_id_dup;
    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id2, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    free (rule_attr[3].value.aclfield.data.objlist.list);

    set_rule_attr.id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    set_rule_attr.value.aclfield.data.objlist.count = 1;
    set_rule_attr.value.aclfield.data.objlist.list = (sai_object_id_t *) calloc(
                                                            1, sizeof(sai_object_id_t));
    set_rule_attr.value.aclfield.data.objlist.list[0] = port_id_1;
    sai_rc = p_sai_acl_api_tbl->set_acl_entry_attribute (acl_rule_id, &set_rule_attr);
    EXPECT_EQ (SAI_STATUS_FAILURE, sai_rc);
    free (set_rule_attr.value.aclfield.data.objlist.list);

    set_rule_attr.id = SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE;
    set_rule_attr.value.aclaction.enable = false;
    set_rule_attr.value.aclaction.parameter.oid = session_id;
    sai_rc = p_sai_acl_api_tbl->set_acl_entry_attribute (acl_rule_id, &set_rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    set_rule_attr.id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    set_rule_attr.value.aclfield.data.objlist.count = 1;
    set_rule_attr.value.aclfield.data.objlist.list = (sai_object_id_t *) calloc(
                                                            1, sizeof(sai_object_id_t));
    set_rule_attr.value.aclfield.data.objlist.list[0] = port_id_1;
    set_rule_attr.value.aclfield.enable = true;
    sai_rc = p_sai_acl_api_tbl->set_acl_entry_attribute (acl_rule_id, &set_rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    free (set_rule_attr.value.aclfield.data.objlist.list);

    set_rule_attr.id = SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE;
    set_rule_attr.value.aclaction.enable = true;
    set_rule_attr.value.aclaction.parameter.oid = session_id;
    sai_rc = p_sai_acl_api_tbl->set_acl_entry_attribute (acl_rule_id, &set_rule_attr);
    EXPECT_EQ (SAI_STATUS_FAILURE, sai_rc);

    set_rule_attr.id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    set_rule_attr.value.aclfield.enable = true;
    set_rule_attr.value.aclfield.data.objlist.count = 1;
    set_rule_attr.value.aclfield.data.objlist.list = (sai_object_id_t *) calloc(
                                                            1, sizeof(sai_object_id_t));
    set_rule_attr.value.aclfield.data.objlist.list[0] = port_id_2;
    sai_rc = p_sai_acl_api_tbl->set_acl_entry_attribute (acl_rule_id, &set_rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    free (set_rule_attr.value.aclfield.data.objlist.list);

    set_rule_attr.id = SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE;
    set_rule_attr.value.aclaction.enable = true;
    set_rule_attr.value.aclaction.parameter.oid = session_id;
    sai_rc = p_sai_acl_api_tbl->set_acl_entry_attribute (acl_rule_id, &set_rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    EXPECT_EQ (SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->remove_acl_entry(acl_rule_id));
    EXPECT_EQ (SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->remove_acl_entry(acl_rule_id2));

    EXPECT_EQ (SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->remove_acl_table (acl_table_id));

    sai_rc = sai_test_samplepacket_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_samplepacket_session_destroy (session_id_dup);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

TEST_F(samplepacketTest, flow_based_and_port_based) {
    sai_object_id_t acl_table_id;
    sai_object_id_t acl_rule_id;
    sai_object_id_t session_id;
    sai_attribute_t rule_attr[10] = {0};
    sai_attribute_t table_attr[12] = {0};
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr[SAI_SAMPLE_NO_OF_MANDAT_ATTRIB + 1] = {0};

    attr[0].id =  SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    attr[0].value.u32 = 2048;

    sai_rc = sai_test_samplepacket_session_create (&session_id, SAI_SAMPLE_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = sai_test_samplepacket_session_ingress_port_add (port_id_1, session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

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
    sai_rc = p_sai_acl_api_tbl->create_acl_table (&acl_table_id, switch_id, 12, table_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    rule_attr[0].value.oid = acl_table_id;
    rule_attr[1].id =  SAI_ACL_ENTRY_ATTR_PRIORITY;
    rule_attr[1].value.u32 = 1;
    rule_attr[2].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    rule_attr[2].value.booldata = true;
    rule_attr[3].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    rule_attr[3].value.aclfield.enable = true;
    rule_attr[3].value.aclfield.data.objlist.count = 1;
    rule_attr[3].value.aclfield.data.objlist.list = (sai_object_id_t *) calloc(
                                                            1, sizeof(sai_object_id_t));
    rule_attr[3].value.aclfield.data.objlist.list[0] = port_id_1;
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE;
    rule_attr[4].value.aclaction.enable = true;
    rule_attr[4].value.aclaction.parameter.oid = session_id;
    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);

    sai_rc = sai_test_samplepacket_session_ingress_port_add (port_id_1, SAI_NULL_OBJECT_ID);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    EXPECT_EQ (SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->remove_acl_entry(acl_rule_id));
    EXPECT_EQ (SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->remove_acl_table (acl_table_id));

    free (rule_attr[3].value.aclfield.data.objlist.list);

    sai_rc = sai_test_samplepacket_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}

TEST_F(samplepacketTest, flow_based_duplicate) {
    sai_object_id_t acl_table_id;
    sai_object_id_t acl_rule_id_1;
    sai_object_id_t acl_rule_id_2;
    sai_object_id_t acl_rule_id_3;
    sai_object_id_t acl_rule_id_4;
    sai_object_id_t session_id;
    sai_object_id_t session_id_dup;
    sai_object_id_t session_id_dup2;
    sai_attribute_t rule_attr[10] = {0};
    sai_attribute_t table_attr[12] = {0};
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr[SAI_SAMPLE_NO_OF_MANDAT_ATTRIB + 1] = {0};

    attr[0].id =  SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    attr[0].value.u32 = 2048;

    sai_rc = sai_test_samplepacket_session_create (&session_id, SAI_SAMPLE_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr[0].id =  SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    attr[0].value.u32 = 1024;

    sai_rc = sai_test_samplepacket_session_create (&session_id_dup, SAI_SAMPLE_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    attr[0].id =  SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE;
    attr[0].value.u32 = 4096;

    sai_rc = sai_test_samplepacket_session_create (&session_id_dup2, SAI_SAMPLE_NO_OF_MANDAT_ATTRIB, attr);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
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
    sai_rc = p_sai_acl_api_tbl->create_acl_table (&acl_table_id, switch_id, 12, table_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    rule_attr[0].value.oid = acl_table_id;
    rule_attr[1].id =  SAI_ACL_ENTRY_ATTR_PRIORITY;
    rule_attr[1].value.u32 = 1;
    rule_attr[2].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    rule_attr[2].value.booldata= true;
    rule_attr[3].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    rule_attr[3].value.aclfield.enable = true;
    rule_attr[3].value.aclfield.data.objlist.count = 1;
    rule_attr[3].value.aclfield.data.objlist.list = (sai_object_id_t *) calloc(
                                                            1, sizeof(sai_object_id_t));
    rule_attr[3].value.aclfield.data.objlist.list[0] = port_id_1;
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE;
    rule_attr[4].value.aclaction.enable = true;
    rule_attr[4].value.aclaction.parameter.oid = session_id;
    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id_1, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    free (rule_attr[3].value.aclfield.data.objlist.list);

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    rule_attr[0].value.oid = acl_table_id;
    rule_attr[1].id =  SAI_ACL_ENTRY_ATTR_PRIORITY;
    rule_attr[1].value.u32 = 1;
    rule_attr[2].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    rule_attr[2].value.booldata= true;
    rule_attr[3].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    rule_attr[3].value.aclfield.enable = true;
    rule_attr[3].value.aclfield.data.objlist.count = 1;
    rule_attr[3].value.aclfield.data.objlist.list = (sai_object_id_t *) calloc(
                                                            1, sizeof(sai_object_id_t));
    rule_attr[3].value.aclfield.data.objlist.list[0] = port_id_2;
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE;
    rule_attr[4].value.aclaction.enable = true;
    rule_attr[4].value.aclaction.parameter.oid = session_id_dup;
    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id_2, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    free (rule_attr[3].value.aclfield.data.objlist.list);

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    rule_attr[0].value.oid = acl_table_id;
    rule_attr[1].id =  SAI_ACL_ENTRY_ATTR_PRIORITY;
    rule_attr[1].value.u32 = 1;
    rule_attr[2].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    rule_attr[2].value.booldata= true;
    rule_attr[3].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS;
    rule_attr[3].value.aclfield.enable = true;
    rule_attr[3].value.objlist.count = 2;
    rule_attr[3].value.objlist.list = (sai_object_id_t *) calloc(
                                                            2, sizeof(sai_object_id_t));
    rule_attr[3].value.objlist.list[0] = port_id_1;
    rule_attr[3].value.objlist.list[1] = port_id_2;
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE;
    rule_attr[4].value.aclaction.enable = true;
    rule_attr[4].value.aclaction.parameter.oid = session_id_dup;
    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id_3, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_FAILURE, sai_rc);
    free (rule_attr[3].value.objlist.list);

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    rule_attr[0].value.oid = acl_table_id;
    rule_attr[1].id =  SAI_ACL_ENTRY_ATTR_PRIORITY;
    rule_attr[1].value.u32 = 1;
    rule_attr[2].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    rule_attr[2].value.booldata= true;
    rule_attr[3].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    rule_attr[3].value.aclfield.enable = true;
    rule_attr[3].value.aclfield.data.u8 = 17;
    rule_attr[3].value.aclfield.mask.u8 = 0xff;
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE;
    rule_attr[4].value.aclaction.enable = true;
    rule_attr[4].value.aclaction.parameter.oid = session_id;
    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id_3, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_FAILURE, sai_rc);

    rule_attr[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    rule_attr[0].value.oid = acl_table_id;
    rule_attr[1].id =  SAI_ACL_ENTRY_ATTR_PRIORITY;
    rule_attr[1].value.u32 = 1;
    rule_attr[2].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    rule_attr[2].value.booldata= true;
    rule_attr[3].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    rule_attr[3].value.aclfield.enable = true;
    rule_attr[3].value.aclfield.data.u8 = 17;
    rule_attr[3].value.aclfield.mask.u8 = 0xff;
    rule_attr[4].id = SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE;
    rule_attr[4].value.aclaction.enable = true;
    rule_attr[4].value.aclaction.parameter.oid = session_id_dup;
    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id_4, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_FAILURE, sai_rc);

    rule_attr[4].value.aclaction.parameter.oid = session_id;
    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id_4, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_FAILURE, sai_rc);

    rule_attr[4].value.aclaction.parameter.oid = session_id_dup2;
    sai_rc = p_sai_acl_api_tbl->create_acl_entry (&acl_rule_id_4, switch_id, 5, rule_attr);
    EXPECT_EQ (SAI_STATUS_FAILURE, sai_rc);

    sai_rc = sai_test_samplepacket_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_OBJECT_IN_USE, sai_rc);

    EXPECT_EQ (SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->remove_acl_entry(acl_rule_id_1));
    EXPECT_EQ (SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->remove_acl_entry(acl_rule_id_2));
    EXPECT_EQ (SAI_STATUS_SUCCESS, p_sai_acl_api_tbl->remove_acl_table (acl_table_id));

    sai_rc = sai_test_samplepacket_session_destroy (session_id);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    sai_rc = sai_test_samplepacket_session_destroy (session_id_dup);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
    sai_rc = sai_test_samplepacket_session_destroy (session_id_dup2);

    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_rc);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
