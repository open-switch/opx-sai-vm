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

/*
 * sai_l2mc_unit_test.cpp
 *
 */

#include "gtest/gtest.h"

extern "C" {
#include "sai.h"
#include "saivlan.h"
#include "saitypes.h"
#include "sai_switch_utils.h"
#include "sai_l2_unit_test_defs.h"
#include "sai_l2mc_common.h"
#include "sai_l2mc_api.h"
#include "sai_vlan_api.h"
#include <arpa/inet.h>
}

#define SAI_MAX_PORTS  256

static uint32_t port_count = 0;
static sai_object_id_t port_list[SAI_MAX_PORTS] = {0};
static sai_object_id_t switch_id = 0;

/*
 * API query is done while running the first test case and
 * the method table is stored in sai_l2mc_group_api_table so
 * that its available for the rest of the test cases which
 * use the method table
 */
class l2mcInit : public ::testing::Test
{
    protected:
        static void SetUpTestCase()
        {
            sai_switch_api_t *p_sai_switch_api_tbl = NULL;

            /*
             * Query and populate the SAI Switch API Table.
             */
            EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
                    (SAI_API_SWITCH, (static_cast<void**>
                                      (static_cast<void*>(&p_sai_switch_api_tbl)))));

            ASSERT_TRUE (p_sai_switch_api_tbl != NULL);

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

            ASSERT_EQ(NULL,sai_api_query(SAI_API_VLAN,
                        (static_cast<void**>(static_cast<void*>(&sai_vlan_api_table)))));
            ASSERT_EQ(NULL,sai_api_query(SAI_API_L2MC_GROUP,
                        (static_cast<void**>(static_cast<void*>(&sai_l2mc_group_api_table)))));

            ASSERT_TRUE(sai_l2mc_group_api_table != NULL);

            EXPECT_TRUE(sai_l2mc_group_api_table->create_l2mc_group != NULL);
            EXPECT_TRUE(sai_l2mc_group_api_table->remove_l2mc_group != NULL);
            EXPECT_TRUE(sai_l2mc_group_api_table->set_l2mc_group_attribute != NULL);
            EXPECT_TRUE(sai_l2mc_group_api_table->get_l2mc_group_attribute != NULL);
            EXPECT_TRUE(sai_l2mc_group_api_table->create_l2mc_group_member != NULL);
            EXPECT_TRUE(sai_l2mc_group_api_table->remove_l2mc_group_member != NULL);


            ASSERT_EQ(NULL,sai_api_query(SAI_API_L2MC,
                        (static_cast<void**>(static_cast<void*>(&sai_l2mc_api_table)))));

            ASSERT_TRUE(sai_l2mc_api_table != NULL);

            EXPECT_TRUE(sai_l2mc_api_table->create_l2mc_entry != NULL);
            EXPECT_TRUE(sai_l2mc_api_table->remove_l2mc_entry != NULL);
            EXPECT_TRUE(sai_l2mc_api_table->set_l2mc_entry_attribute != NULL);
            EXPECT_TRUE(sai_l2mc_api_table->get_l2mc_entry_attribute != NULL);

            sai_attribute_t sai_port_attr;
            sai_status_t ret = SAI_STATUS_SUCCESS;

            memset (&sai_port_attr, 0, sizeof (sai_port_attr));

            sai_port_attr.id = SAI_SWITCH_ATTR_PORT_LIST;
            sai_port_attr.value.objlist.count = SAI_MAX_PORTS;
            sai_port_attr.value.objlist.list  = port_list;

            ret = p_sai_switch_api_tbl->get_switch_attribute(0,1,&sai_port_attr);
            port_count = sai_port_attr.value.objlist.count;
            EXPECT_EQ (SAI_STATUS_SUCCESS,ret);
            port_id_1 = port_list[0];
            port_id_2 = port_list[port_count-1];
            port_id_invalid = port_id_2 + 1;
        }

        static sai_l2mc_group_api_t* sai_l2mc_group_api_table;
        static sai_l2mc_api_t* sai_l2mc_api_table;
        static sai_vlan_api_t* sai_vlan_api_table;
        static sai_object_id_t  port_id_1;
        static sai_object_id_t  port_id_2;
        static sai_object_id_t  port_id_invalid;
        sai_status_t sai_test_ipmc_entry_create ( unsigned int ip_family,
                                                    const char * grp_str,
                                                    const char * src_str,
                                                    sai_object_id_t bv_id,
                                                    sai_object_id_t l2mc_grp_id);
        sai_status_t sai_test_ipmc_entry_remove ( unsigned int ip_family,
                                                    const char * grp_str,
                                                    const char * src_str,
                                                    sai_object_id_t bv_id);
};

sai_l2mc_group_api_t* l2mcInit ::sai_l2mc_group_api_table = NULL;
sai_l2mc_api_t* l2mcInit ::sai_l2mc_api_table = NULL;
sai_vlan_api_t* l2mcInit ::sai_vlan_api_table = NULL;
sai_object_id_t l2mcInit ::port_id_1 = 0;
sai_object_id_t l2mcInit ::port_id_2 = 0;
sai_object_id_t l2mcInit ::port_id_invalid = 0;

/*
 * L2mc Group create and delete
 */
TEST_F(l2mcInit, create_remove_l2mc_group)
{
    sai_attribute_t attr;
    sai_object_id_t l2mc_obj_id = SAI_NULL_OBJECT_ID;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->create_l2mc_group(&l2mc_obj_id,0,0,&attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->remove_l2mc_group(l2mc_obj_id));

    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_l2mc_group_api_table->remove_l2mc_group(l2mc_obj_id));
}

/*
 * L2mc group add port and remove port
 */
TEST_F(l2mcInit, l2mc_group_port_test)
{
    sai_attribute_t attr[SAI_GTEST_VLAN_MEMBER_ATTR_COUNT];
    sai_object_id_t l2mc_obj_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t l2mc_mem_obj_id1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t l2mc_mem_obj_id2 = SAI_NULL_OBJECT_ID;
    sai_object_id_t l2mc_mem_obj_fail_id1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t l2mc_mem_obj_fail_id2 = SAI_NULL_OBJECT_ID;
    uint32_t attr_count = 0;

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->create_l2mc_group(&l2mc_obj_id,0,attr_count,attr));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID;
    attr[attr_count].value.oid = l2mc_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID;
    attr[attr_count].value.oid = port_id_1;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->create_l2mc_group_member(&l2mc_mem_obj_id1,
                                                    0,
                                                    attr_count,
                                                    attr));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID;
    attr[attr_count].value.oid = l2mc_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID;
    attr[attr_count].value.oid = port_id_2;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->create_l2mc_group_member(&l2mc_mem_obj_id2,
                                                    0,
                                                    attr_count,
                                                    attr));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              sai_l2mc_group_api_table->remove_l2mc_group(l2mc_obj_id));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_l2mc_group_api_table->get_l2mc_group_member_attribute(l2mc_mem_obj_id1,
                                                    attr_count,
                                                    attr));
    ASSERT_EQ(attr[0].value.oid,l2mc_obj_id);

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_l2mc_group_api_table->get_l2mc_group_member_attribute(l2mc_mem_obj_id1,
                                                    attr_count,
                                                    attr));
    ASSERT_EQ(attr[0].value.oid,port_id_1);

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
            sai_l2mc_group_api_table->get_l2mc_group_member_attribute(l2mc_mem_obj_id2,
                                                    attr_count,
                                                    attr));
    ASSERT_EQ(attr[0].value.oid,port_id_2);

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID;
    attr[attr_count].value.oid = l2mc_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID;
    attr[attr_count].value.oid = port_id_1;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_ITEM_ALREADY_EXISTS,
              sai_l2mc_group_api_table->create_l2mc_group_member(&l2mc_mem_obj_fail_id1,
                                                    0,
                                                    attr_count,
                                                    attr));
    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID;
    attr[attr_count].value.oid = l2mc_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID;
    attr[attr_count].value.oid = port_id_2;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_ITEM_ALREADY_EXISTS,
              sai_l2mc_group_api_table->create_l2mc_group_member(&l2mc_mem_obj_fail_id2,
                                                    0,
                                                    attr_count,
                                                    attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->remove_l2mc_group_member(l2mc_mem_obj_id1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->remove_l2mc_group_member(l2mc_mem_obj_id2));

    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_l2mc_group_api_table->remove_l2mc_group_member(l2mc_mem_obj_id1));

    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_l2mc_group_api_table->remove_l2mc_group_member(l2mc_mem_obj_id2));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID;
    attr[attr_count].value.oid = l2mc_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID;
    attr[attr_count].value.oid = port_id_1;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->create_l2mc_group_member(&l2mc_mem_obj_fail_id1,
                                                    0,
                                                    attr_count,
                                                    attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
                  sai_l2mc_group_api_table->remove_l2mc_group_member(l2mc_mem_obj_fail_id1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->remove_l2mc_group(l2mc_obj_id));
}

sai_status_t sai_test_mcast_entry_fill( sai_l2mc_entry_t *l2mc_entry,
unsigned int af, const char * grp_str, const char * src_str, sai_object_id_t bv_id)
{

    l2mc_entry->bv_id = bv_id;
    l2mc_entry->destination.addr_family = l2mc_entry->source.addr_family =
        (sai_ip_addr_family_t )af;

    if(af == SAI_IP_ADDR_FAMILY_IPV4) {
        inet_pton(AF_INET, grp_str, (void *)&l2mc_entry->destination.addr.ip4);
        inet_pton(AF_INET, src_str, (void *)&l2mc_entry->source.addr.ip4);
    } else if (af == SAI_IP_ADDR_FAMILY_IPV6) {
        inet_pton(AF_INET6, grp_str, (void *)&l2mc_entry->destination.addr.ip6);
        inet_pton(AF_INET6, src_str, (void *)&l2mc_entry->source.addr.ip6);
    }
    return SAI_STATUS_SUCCESS;

}
sai_status_t l2mcInit::sai_test_ipmc_entry_create ( unsigned int ip_family,
                                                    const char * grp_str,
                                                    const char * src_str,
                                                    sai_object_id_t bv_id,
                                                    sai_object_id_t l2mc_grp_id)
{
    sai_attribute_t attr[SAI_GTEST_VLAN_MEMBER_ATTR_COUNT];
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_l2mc_entry_t l2mc_entry;
    uint32_t attr_count = 0;

    memset(&l2mc_entry, 0, sizeof(l2mc_entry));

    sai_rc = sai_test_mcast_entry_fill (&l2mc_entry, ip_family, grp_str, src_str, bv_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf ("failed to fill the l2mc_entry\n");
        return sai_rc;
    }

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_L2MC_ENTRY_ATTR_PACKET_ACTION;
    attr[attr_count].value.s32 = SAI_PACKET_ACTION_FORWARD;
    attr_count++;
    attr[attr_count].id = SAI_L2MC_ENTRY_ATTR_OUTPUT_GROUP_ID;
    attr[attr_count].value.oid = l2mc_grp_id;
    attr_count++;

    sai_rc = sai_l2mc_api_table->create_l2mc_entry (&l2mc_entry, attr_count, attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf ("failed to install the l2mc_entry %d\n",sai_rc);
    }else {
        printf("SAI L2mc Entry insalled\n");
    }
    return sai_rc;
}

sai_status_t l2mcInit::sai_test_ipmc_entry_remove ( unsigned int ip_family,
                                                    const char * grp_str,
                                                    const char * src_str,
                                                    sai_object_id_t bv_id)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    sai_l2mc_entry_t l2mc_entry;

    memset(&l2mc_entry, 0, sizeof(l2mc_entry));

    sai_rc = sai_test_mcast_entry_fill (&l2mc_entry, ip_family, grp_str, src_str, bv_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf ("failed to fill the l2mc_entry\n");
        return sai_rc;
    }

    sai_rc = sai_l2mc_api_table->remove_l2mc_entry (&l2mc_entry);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf ("failed to Remove the l2mc_entry %d\n",sai_rc);
    }else {
        printf("SAI L2mc Entry Removed\n");
    }
    return sai_rc;
}
/*
 * VLAN get port test
 */
TEST_F(l2mcInit, l2mc_entry_add_and_remove)
{
    sai_attribute_t attr[SAI_GTEST_VLAN_MEMBER_ATTR_COUNT];
    sai_object_id_t l2mc_obj_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t vlan_obj_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t l2mc_mem_obj_id1 = SAI_NULL_OBJECT_ID;
    sai_object_id_t l2mc_mem_obj_id2 = SAI_NULL_OBJECT_ID;
    const char *grp_str = "225.1.1.1";
    const char *src_str = "10.1.1.1";
    sai_ip_addr_family_t family = SAI_IP_ADDR_FAMILY_IPV4;
    uint32_t attr_count = 0;

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_VLAN_ATTR_VLAN_ID;
    attr[attr_count].value.u16 = SAI_GTEST_VLAN;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_vlan_api_table->create_vlan(&vlan_obj_id,0,1,(const sai_attribute_t *)&attr));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->create_l2mc_group(&l2mc_obj_id,0,attr_count,attr));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID;
    attr[attr_count].value.oid = l2mc_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID;
    attr[attr_count].value.oid = port_id_1;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->create_l2mc_group_member(&l2mc_mem_obj_id1,
                                                    0,
                                                    attr_count,
                                                    attr));

    attr_count = 0;
    memset(&attr,0,sizeof(attr));
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID;
    attr[attr_count].value.oid = l2mc_obj_id;
    attr_count++;
    attr[attr_count].id = SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID;
    attr[attr_count].value.oid = port_id_2;
    attr_count++;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->create_l2mc_group_member(&l2mc_mem_obj_id2,
                                                    0,
                                                    attr_count,
                                                    attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_test_ipmc_entry_create(family, grp_str, src_str,
                  (sai_object_id_t) SAI_GTEST_VLAN_OBJ,
                  l2mc_obj_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->remove_l2mc_group_member(l2mc_mem_obj_id1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->remove_l2mc_group_member(l2mc_mem_obj_id2));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              sai_l2mc_group_api_table->remove_l2mc_group(l2mc_obj_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_test_ipmc_entry_remove(family, grp_str, src_str,
                  (sai_object_id_t) SAI_GTEST_VLAN_OBJ));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_l2mc_group_api_table->remove_l2mc_group(l2mc_obj_id));
}
