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
 * sai_lag_unit_test.cpp
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include <inttypes.h>

#include "sai_bridge_unit_test_utils.h"

extern "C" {
#include "sai.h"
#include "sailag.h"
#include "saitypes.h"
#include "saibridge.h"
#include "sai_l2_unit_test_defs.h"
}

#define SAI_MAX_PORTS     256
#define SAI_LAG_UT_DEFAULT_VLAN 1

uint32_t bridge_port_count = 0;
sai_object_id_t bridge_port_list[SAI_MAX_PORTS] = {0};
sai_object_id_t default_bridge_id = 0;

#include "sai_lag_unit_test.h"

sai_lag_api_t* lagInit ::sai_lag_api_table = NULL;
sai_vlan_api_t* lagInit ::sai_vlan_api_table = NULL;
sai_virtual_router_api_t* lagInit ::sai_vrf_api_table = NULL;
sai_bridge_api_t* lagInit ::p_sai_bridge_api_tbl = NULL;
sai_port_api_t* lagInit ::p_sai_port_api_tbl = NULL;
sai_router_interface_api_t* lagInit ::sai_router_intf_api_table = NULL;
sai_object_id_t lagInit ::port_id_1 = 0;
sai_object_id_t lagInit ::port_id_2 = 0;
sai_object_id_t lagInit ::port_id_invalid = 0;
sai_object_id_t lagInit ::default_vlan_id = 0;
sai_object_id_t lagInit ::bridge_port_id_1 = 0;
sai_object_id_t lagInit ::bridge_port_id_2 = 0;

sai_status_t sai_lag_ut_create_bridge_port(sai_bridge_api_t *p_sai_bridge_api_tbl,
                                           sai_object_id_t switch_id,
                                           sai_object_id_t port_id,
                                           sai_object_id_t *bridge_port_id)
{
    return sai_bridge_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id,
                                            true, bridge_port_id);
}

sai_status_t sai_lag_ut_remove_bridge_port(sai_bridge_api_t *p_sai_bridge_api_tbl,
                                           sai_object_id_t switch_id,
                                           sai_object_id_t bridge_port_id)
{
    return sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, bridge_port_id, true);
}

sai_status_t sai_lag_get_ut_port_from_bridge_port(sai_bridge_api_t *p_sai_bridge_api_tbl,
                                                  sai_object_id_t bridge_port_id,
                                                  sai_object_id_t *port_id)
{
    sai_attribute_t bridge_port_attr[1];
    sai_status_t    sai_rc;

    bridge_port_attr[0].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;

    sai_rc = p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id, 1,
                                                             bridge_port_attr);

    *port_id = bridge_port_attr[0].value.oid;
    return sai_rc;

}

sai_status_t sai_lag_ut_add_port_to_lag (sai_lag_api_t   *lag_api,
                                         sai_object_id_t *member_id,
                                         sai_object_id_t  lag_id,
                                         sai_object_id_t  port_id,
                                         bool is_ing_disable_present,
                                         bool ing_disable,
                                         bool is_egr_disable_present,
                                         bool egr_disable)
{
    sai_attribute_t attr_list [4];
    uint32_t        count = 0;

    memset (attr_list, 0, sizeof (attr_list));

    attr_list [count].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attr_list [count].value.oid = lag_id;
    count++;

    attr_list [count].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attr_list [count].value.oid = port_id;
    count++;

    if (is_ing_disable_present) {
        attr_list [count].id = SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE;
        attr_list [count].value.booldata = ing_disable;
        count++;
    }

    if (is_egr_disable_present) {
        attr_list [count].id = SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE;
        attr_list [count].value.booldata = egr_disable;
        count++;
    }

    return lag_api->create_lag_member (member_id, switch_id, count, attr_list);
}

sai_status_t sai_lag_ut_get_lag_attr (sai_lag_api_t   *lag_api,
                                      sai_object_id_t  member_id,
                                      bool            *ing_disable,
                                      bool            *egr_disable)
{
    sai_status_t    rc;
    sai_attribute_t attr_list [4];
    uint32_t        count = 0;

    memset (attr_list, 0, sizeof (attr_list));

    attr_list [count].id = SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE;
    count++;

    attr_list [count].id = SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE;
    count++;

    rc = lag_api->get_lag_member_attribute (member_id, count, attr_list);

    if (ing_disable)
        *ing_disable = attr_list [0].value.booldata;

    if (egr_disable)
        *egr_disable = attr_list [1].value.booldata;

    return rc;
}

sai_status_t sai_lag_ut_set_lag_attr (sai_lag_api_t   *lag_api,
                                      sai_object_id_t member_id,
                                      bool is_ing_disable_present,
                                      bool ing_disable,
                                      bool is_egr_disable_present,
                                      bool egr_disable)
{
    sai_status_t    rc;
    sai_attribute_t attr;

    memset (&attr, 0, sizeof (attr));

    if (is_ing_disable_present) {
        attr.id = SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE;
        attr.value.booldata = ing_disable;

        rc = lag_api->set_lag_member_attribute (member_id, &attr);

        if (rc != SAI_STATUS_SUCCESS) {
            return rc;
        }
    }

    if (is_egr_disable_present) {
        attr.id = SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE;
        attr.value.booldata = egr_disable;

        rc = lag_api->set_lag_member_attribute (member_id, &attr);

        if (rc != SAI_STATUS_SUCCESS) {
            return rc;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_lag_ut_remove_port_from_lag (sai_lag_api_t   *lag_api,
                                              sai_object_id_t  member_id)
{
    return lag_api->remove_lag_member (member_id);
}

sai_status_t
sai_lag_ut_create_router_interface (sai_lag_api_t              *lag_api,
                                    sai_router_interface_api_t *router_intf_api_table,
                                    sai_vlan_api_t             *vlan_api_table,
                                    sai_bridge_api_t           *p_sai_bridge_api_tbl,
                                    sai_object_id_t            *rif_id,
                                    sai_object_id_t             vrf_id,
                                    sai_object_id_t             default_vlan_id,
                                    sai_object_id_t             port_id)
{
    sai_status_t rc;
    sai_attribute_t attr_list [4];
    uint32_t count = 0;
    int port_count = 0;
    int it=0;
    int cur_it=0;
    sai_object_type_t obj_type;
    sai_attribute_t get_attr;
    sai_object_id_t bridge_port_id;

    attr_list[0].id = SAI_VLAN_ATTR_MEMBER_LIST;
    attr_list[0].value.objlist.count = 0;

    /* Getting the number of ports using count as 0 */
    rc = vlan_api_table->get_vlan_attribute(default_vlan_id,1,&attr_list[0]);
    if(SAI_STATUS_BUFFER_OVERFLOW != rc) {
        printf("Unable to get the VLAN member list count for"
                " VLAN obj id:%lu, rc:%d\r\n",default_vlan_id,rc);
        return SAI_STATUS_FAILURE;
    }

    port_count = attr_list[0].value.objlist.count;
    printf("VLAN member list count for VLAN obj id:%lu is %d\r\n",
            default_vlan_id,attr_list[0].value.objlist.count);

    obj_type = sai_object_type_query(port_id);

    sai_object_id_t cur_list[10] = {0};
    int cur_count = 1;
    if(obj_type == SAI_OBJECT_TYPE_PORT) {
        cur_count = 1;
        cur_list[0] = port_id;
    } else if(obj_type == SAI_OBJECT_TYPE_LAG) {
        get_attr.id = SAI_LAG_ATTR_PORT_LIST;
        get_attr.value.objlist.list = cur_list;
        get_attr.value.objlist.count = 10;
        lag_api->get_lag_attribute(port_id,1, &get_attr);
        cur_count = get_attr.value.objlist.count;
    }
    {
        sai_object_id_t list[port_count];

        /* Getting the list of ports in default vlan */
        attr_list[0].value.objlist.count = port_count;
        attr_list[0].value.objlist.list = list;
        rc = vlan_api_table->get_vlan_attribute(default_vlan_id,1,&attr_list[0]);
        if(SAI_STATUS_SUCCESS != rc) {
            printf("Unable to get the VLAN member list for"
                    " VLAN obj id:%lu, rc:%d\r\n",default_vlan_id,rc);
            return SAI_STATUS_FAILURE;
        }

        attr_list[0].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
        attr_list[0].value.oid = 0;
        /* Finding the vlan member id of the port id */
        for(cur_it = 0; cur_it < cur_count; cur_it++) {
            printf("Port count %d, Cur idx %d Cur member %lu",cur_count, cur_it, cur_list[cur_it]);
            for(it=0; it<port_count; it++) {

                rc = vlan_api_table->get_vlan_member_attribute(list[it],1,&attr_list[0]);
                if(SAI_STATUS_SUCCESS != rc) {
                    printf("Unable to get the VLAN member port id for "
                            "member:%lu in VLAN obj:%lu, rc:%d\r\n",
                            list[it],default_vlan_id,rc);
                }


                bridge_port_id = attr_list[0].value.oid;
                attr_list[0].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;

                rc  = p_sai_bridge_api_tbl->get_bridge_port_attribute(bridge_port_id,
                                                                      1, attr_list);
                if(SAI_STATUS_SUCCESS != rc) {
                    printf("Unable to get the port id for "
                           "bridge port:%lu  rc:%d\r\n",  bridge_port_id, rc);
                }

                if(attr_list[0].value.oid == cur_list[cur_it]) {
                    rc = vlan_api_table->remove_vlan_member(list[it]);
                    if(SAI_STATUS_SUCCESS != rc) {
                        printf("Failed member remove for VLAN member:%lu "
                                "in VLAN obj:%lu, rc:%d\r\n",
                                list[it],default_vlan_id,rc);
                        return SAI_STATUS_FAILURE;
                    }
                    break;
                }
            }
            if(it == port_count) {
                printf("Unable to find port:%lu as member in VLAN obj:%lu",
                        cur_list[cur_it],default_vlan_id);
                return SAI_STATUS_FAILURE;
            }
        }
    }
    attr_list [count].id = SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID;
    attr_list [count].value.oid = vrf_id;
    count++;

    attr_list [count].id = SAI_ROUTER_INTERFACE_ATTR_TYPE;
    attr_list [count].value.s32 = SAI_ROUTER_INTERFACE_TYPE_PORT;
    count++;

    attr_list [count].id = SAI_ROUTER_INTERFACE_ATTR_PORT_ID;
    attr_list [count].value.oid = port_id;
    count++;

    rc = router_intf_api_table->create_router_interface (rif_id, switch_id, count,
                                                         attr_list);
    return rc;
}

sai_status_t
sai_lag_ut_remove_router_interface (sai_router_interface_api_t *router_intf_api_table,
                                    sai_object_id_t             rif_id)
{
    return (router_intf_api_table->remove_router_interface (rif_id));
}

sai_status_t
sai_lag_ut_create_virtual_router (sai_virtual_router_api_t *vrf_api_table,
                                  sai_object_id_t          *vrf_id)
{
    sai_attribute_t attr;

    attr.id = SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS;
    memset (attr.value.mac, 0x2, sizeof (sai_mac_t));

    return (vrf_api_table->create_virtual_router (vrf_id, switch_id, 1, &attr));
}

sai_status_t
sai_lag_ut_remove_virtual_router (sai_virtual_router_api_t *vrf_api_table,
                                  sai_object_id_t           vrf_id)
{
    return (vrf_api_table->remove_virtual_router (vrf_id));
}

/*
 * LAG create and delete
 */
TEST_F(lagInit, create_remove_lag)
{
    sai_object_id_t lag_id = 0;
    sai_object_id_t lag_bridge_port = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->create_lag(&lag_id, switch_id,0, NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_id, &lag_bridge_port));

    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              sai_lag_api_table->remove_lag(lag_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_bridge_port));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->remove_lag(lag_id));

    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_lag_api_table->remove_lag(lag_id));

}

/*
 * SAI LAG Default attribute test
 */
TEST_F(lagInit, lag_default_attributes)
{
    sai_object_id_t lag_id = 0;
    sai_object_id_t lag_bridge_port = 0;
    sai_attribute_t attr;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->create_lag(&lag_id, switch_id,0, NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_id, &lag_bridge_port));

    attr.id =  SAI_LAG_ATTR_PORT_VLAN_ID;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->get_lag_attribute(lag_id,1, &attr));

    /*Check if default PVID is 1*/
    ASSERT_EQ(attr.value.u16, 1);

    attr.id =  SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->get_lag_attribute(lag_id,1, &attr));

    /*Check if default vlan priority by default is 0*/
    ASSERT_EQ(attr.value.u8, 0);

    attr.id =  SAI_LAG_ATTR_DROP_UNTAGGED;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->get_lag_attribute(lag_id,1, &attr));

    /*Check if default drop untagged is false*/
    ASSERT_EQ(attr.value.booldata, false);

    attr.id = SAI_LAG_ATTR_DROP_TAGGED;
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->get_lag_attribute(lag_id,1, &attr));

    /*Check if default drop tagged is false*/
    ASSERT_EQ(attr.value.booldata, false);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_bridge_port));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->remove_lag(lag_id));

    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_lag_api_table->remove_lag(lag_id));

}

TEST_F(lagInit, lag_attributes_on_create)
{
    sai_object_id_t lag_id = 0;
    sai_object_id_t lag_bridge_port = 0;
    sai_attribute_t get_attr[4];
    sai_attribute_t set_attr[4];

    set_attr[0].id =  SAI_LAG_ATTR_PORT_VLAN_ID;
    set_attr[0].value.u16 = 10;

    set_attr[1].id =  SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY;
    set_attr[1].value.u8 = 5;

    set_attr[2].id =  SAI_LAG_ATTR_DROP_UNTAGGED;
    set_attr[2].value.booldata =  true;

    set_attr[3].id =  SAI_LAG_ATTR_DROP_TAGGED;
    set_attr[3].value.booldata =  false;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->create_lag(&lag_id, switch_id,4, set_attr));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_id, &lag_bridge_port));


    get_attr[0].id = set_attr[0].id;
    get_attr[1].id = set_attr[1].id;
    get_attr[2].id = set_attr[2].id;
    get_attr[3].id = set_attr[3].id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->get_lag_attribute(lag_id,4, get_attr));

    ASSERT_EQ(set_attr[0].value.u16, get_attr[0].value.u16);
    ASSERT_EQ(set_attr[1].value.u8, get_attr[1].value.u8);
    ASSERT_EQ(set_attr[2].value.booldata, get_attr[2].value.booldata);
    ASSERT_EQ(set_attr[3].value.booldata, get_attr[3].value.booldata);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_bridge_port));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->remove_lag(lag_id));

}

TEST_F(lagInit, lag_attributes)
{
    sai_object_id_t lag_id = 0;
    sai_object_id_t lag_bridge_port = 0;
    sai_attribute_t get_attr[4];
    sai_attribute_t set_attr[4];
    uint_t          idx = 0;

    set_attr[0].id =  SAI_LAG_ATTR_PORT_VLAN_ID;
    set_attr[0].value.u16 = 10;

    set_attr[1].id =  SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY;
    set_attr[1].value.u8 = 5;

    set_attr[2].id =  SAI_LAG_ATTR_DROP_UNTAGGED;
    set_attr[2].value.booldata =  false;

    set_attr[3].id =  SAI_LAG_ATTR_DROP_TAGGED;
    set_attr[3].value.booldata =  true;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->create_lag(&lag_id, switch_id,0, NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_id, &lag_bridge_port));

    for(idx = 0; idx < 4; idx++) {
        ASSERT_EQ(SAI_STATUS_SUCCESS,
                  sai_lag_api_table->set_lag_attribute(lag_id, &set_attr[idx]));
    }

    get_attr[0].id = set_attr[0].id;
    get_attr[1].id = set_attr[1].id;
    get_attr[2].id = set_attr[2].id;
    get_attr[3].id = set_attr[3].id;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->get_lag_attribute(lag_id,4, get_attr));

    ASSERT_EQ(set_attr[0].value.u16, get_attr[0].value.u16);
    ASSERT_EQ(set_attr[1].value.u8, get_attr[1].value.u8);
    ASSERT_EQ(set_attr[2].value.booldata, get_attr[2].value.booldata);
    ASSERT_EQ(set_attr[3].value.booldata, get_attr[3].value.booldata);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_bridge_port));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->remove_lag(lag_id));

}

TEST_F(lagInit, lag_member_new_api_test)
{
    sai_object_id_t lag_id_1 = 0;
    sai_object_id_t lag_id_2 = 0;
    sai_object_id_t member_id_1 = 0;
    sai_object_id_t member_id_2 = 0;
    sai_object_id_t tmp_member_id = 0;
    sai_attribute_t attr_list [4];
    uint32_t        count;
    sai_object_id_t lag_bridge_port_id_1 = 0;
    sai_object_id_t lag_bridge_port_id_2 = 0;

    /* Create Lag lag_id_1 and add port_id_1 and port_id_2 as members */
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->create_lag(&lag_id_1, switch_id, 0, NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_id_1, &lag_bridge_port_id_1));

    ASSERT_NE(SAI_STATUS_SUCCESS,
              sai_lag_ut_add_port_to_lag (sai_lag_api_table, &member_id_1,
                                          lag_id_1, port_id_1,
                                          false, false, false, false));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, bridge_port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_add_port_to_lag (sai_lag_api_table, &member_id_1,
                                          lag_id_1, port_id_1,
                                          false, false, false, false));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, bridge_port_id_2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_add_port_to_lag (sai_lag_api_table, &member_id_2,
                                          lag_id_1, port_id_2,
                                          false, false, true, true));

    /* Check if same port cannot be added to two lags */
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->create_lag (&lag_id_2, switch_id, 0, NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_id_2, &lag_bridge_port_id_2));

    ASSERT_EQ(SAI_STATUS_INVALID_PORT_MEMBER,
              sai_lag_ut_add_port_to_lag (sai_lag_api_table, &tmp_member_id,
                                          lag_id_2, port_id_1,
                                          false, false, false, false));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_bridge_port_id_2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->remove_lag(lag_id_2));

    /* Check if the addition of a lag member fails if it is already a member */
    ASSERT_EQ(SAI_STATUS_ITEM_ALREADY_EXISTS,
              sai_lag_ut_add_port_to_lag (sai_lag_api_table, &tmp_member_id,
                                          lag_id_1, port_id_1,
                                          false, false, false, false));

    /* Check if invalid port is not allowed to be added to the lag */
    ASSERT_EQ(SAI_STATUS_INVALID_PARAMETER,
              sai_lag_ut_add_port_to_lag (sai_lag_api_table, &tmp_member_id,
                                          lag_id_1, port_id_invalid,
                                          false, false, false, false));

    /* Check if member removal with an invalid member_id fails */
    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_lag_ut_remove_port_from_lag (sai_lag_api_table, 0));

    /* Do not remove bridge port until all members are removed from lag */
    ASSERT_EQ(SAI_STATUS_OBJECT_IN_USE,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_bridge_port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_port_from_lag (sai_lag_api_table, member_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id_1, &bridge_port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_port_from_lag (sai_lag_api_table, member_id_2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id_2, &bridge_port_id_2));
    /* Check if member removal fails, if it is not a member of the lag */
    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_lag_ut_remove_port_from_lag (sai_lag_api_table, member_id_1));

    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_lag_ut_remove_port_from_lag (sai_lag_api_table, member_id_2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_bridge_port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->remove_lag(lag_id_1));

    /* Check if member addition fails if the lag is not created. */
    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_lag_ut_add_port_to_lag (sai_lag_api_table, &tmp_member_id,
                                          lag_id_1, port_id_1,
                                          false, false, false, false));

    ASSERT_EQ(SAI_STATUS_ITEM_NOT_FOUND,
              sai_lag_ut_add_port_to_lag (sai_lag_api_table, &tmp_member_id,
                                          lag_id_1, port_id_2,
                                          false, false, false, false));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->create_lag (&lag_id_1, switch_id, 0, NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_id_1, &lag_bridge_port_id_1));

    /* Check if member addition fails, if mandatory arguments are not present */
    memset (attr_list, 0, sizeof (attr_list));
    count = 0;

    attr_list [count].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attr_list [count].value.oid = lag_id_1;
    count++;

    ASSERT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
               sai_lag_api_table->create_lag_member (&tmp_member_id, switch_id,
                                                     count, attr_list));

    count = 0;
    attr_list [count].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attr_list [count].value.oid = port_id_1;
    count++;

    ASSERT_EQ (SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING,
               sai_lag_api_table->create_lag_member (&tmp_member_id, switch_id,
                                                     count, attr_list));

    count = 0;
    attr_list [count].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attr_list [count].value.oid = lag_id_1;
    count++;

    attr_list [count].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attr_list [count].value.oid = port_id_1;
    count++;

    attr_list [count].id = SAI_LAG_MEMBER_ATTR_END + 1000;
    attr_list [count].value.oid = 0;
    count++;

    /* Check if member addition fails if unknown attribute is present. */
    ASSERT_EQ (SAI_STATUS_UNKNOWN_ATTRIBUTE_0+SAI_STATUS_CODE(2),
               sai_lag_api_table->create_lag_member (&tmp_member_id, switch_id,
                                                     count, attr_list));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_bridge_port_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_api_table->remove_lag(lag_id_1));
}

TEST_F(lagInit, lag_member_attribute_new_api_test)
{
    sai_object_id_t lag_id_1 = 0;
    sai_object_id_t member_id_1 = 0;
    sai_object_id_t member_id_2 = 0;
    bool            ing_disable;
    bool            egr_disable;
    sai_object_id_t lag_bridge_port_id = 0;

    /*
     * Create a lag with members assigned default ingress/egress disable
     * attribute values.
     */
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_api_table->create_lag (&lag_id_1, switch_id, 0, NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_id_1, &lag_bridge_port_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, bridge_port_id_1));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_add_port_to_lag (sai_lag_api_table, &member_id_1,
                                           lag_id_1, port_id_1,
                                           false, false, false, false));
    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, bridge_port_id_2));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_add_port_to_lag (sai_lag_api_table, &member_id_2,
                                           lag_id_1, port_id_2,
                                           false, false, false, false));

    /*
     * Check if the get attribute returns correct values for ingress/egress
     * disable attributes.
     */
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_get_lag_attr (sai_lag_api_table, member_id_1,
                                        &ing_disable, &egr_disable));
    ASSERT_FALSE (ing_disable);
    ASSERT_FALSE (egr_disable);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_get_lag_attr (sai_lag_api_table, member_id_2,
                                        &ing_disable, &egr_disable));
    ASSERT_FALSE (ing_disable);
    ASSERT_FALSE (egr_disable);

    /*
     * Modify the attributes and check if the get attribute returns
     * correct values for ingress/egress disable attributes.
     */
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_set_lag_attr (sai_lag_api_table, member_id_1,
                                        true, true, true, false));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_get_lag_attr (sai_lag_api_table, member_id_1,
                                        &ing_disable, &egr_disable));
    ASSERT_TRUE (ing_disable);
    ASSERT_FALSE (egr_disable);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_set_lag_attr (sai_lag_api_table, member_id_2,
                                        false, false, true, true));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_get_lag_attr (sai_lag_api_table, member_id_2,
                                        &ing_disable, &egr_disable));
    ASSERT_FALSE (ing_disable);
    ASSERT_TRUE (egr_disable);

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_set_lag_attr (sai_lag_api_table, member_id_2,
                                        false, false, true, true));
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_get_lag_attr (sai_lag_api_table, member_id_2,
                                        &ing_disable, &egr_disable));
    ASSERT_FALSE (ing_disable);
    ASSERT_TRUE (egr_disable);

    /* Check if LAG cannot be removed until lag members are removed*/
    ASSERT_EQ (SAI_STATUS_OBJECT_IN_USE,
               sai_lag_api_table->remove_lag (lag_id_1));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_remove_port_from_lag(sai_lag_api_table, member_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id_1, &bridge_port_id_1));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_remove_port_from_lag(sai_lag_api_table, member_id_2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id_2, &bridge_port_id_2));


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_bridge_port_id));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_api_table->remove_lag (lag_id_1));

}

TEST_F(lagInit, port_attributes_on_lag_member_remove)
{
    sai_object_id_t lag_id_1 = 0;
    sai_object_id_t member_id_1 = 0;
    sai_object_id_t member_id_2 = 0;
    sai_object_id_t lag_bridge_port_id = 0;
    sai_attribute_t get_attr[4];
    sai_attribute_t set_attr[4];
    uint32_t        idx = 0;


    /*
     * Create a lag with members assigned default ingress/egress disable
     * attribute values.
     */
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_api_table->create_lag (&lag_id_1, switch_id, 0, NULL));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_id_1, &lag_bridge_port_id));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, bridge_port_id_1));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_add_port_to_lag (sai_lag_api_table, &member_id_1,
                                           lag_id_1, port_id_1,
                                           false, false, false, false));

    set_attr[0].id =  SAI_PORT_ATTR_PORT_VLAN_ID;
    set_attr[0].value.u16 = 10;

    set_attr[1].id =  SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY;
    set_attr[1].value.u8 = 5;

    set_attr[2].id =  SAI_PORT_ATTR_DROP_UNTAGGED;
    set_attr[2].value.booldata =  false;

    set_attr[3].id =  SAI_PORT_ATTR_DROP_TAGGED;
    set_attr[3].value.booldata =  true;

    for(idx = 0; idx < 4; idx++) {
        ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
                  p_sai_port_api_tbl->set_port_attribute(port_id_1, &set_attr[idx]));

        ASSERT_EQ(SAI_STATUS_INVALID_ATTRIBUTE_0,
                  p_sai_port_api_tbl->get_port_attribute(port_id_1, 1, &set_attr[idx]));
    }


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, bridge_port_id_2));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_add_port_to_lag (sai_lag_api_table, &member_id_2,
                                           lag_id_1, port_id_2,
                                           false, false, false, false));


    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_remove_port_from_lag(sai_lag_api_table, member_id_1));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id_1, &bridge_port_id_1));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_ut_remove_port_from_lag(sai_lag_api_table, member_id_2));

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id_2, &bridge_port_id_2));


    ASSERT_EQ(SAI_STATUS_SUCCESS,
              sai_lag_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, lag_bridge_port_id));

    get_attr[0].id =  SAI_PORT_ATTR_PORT_VLAN_ID;
    get_attr[1].id =  SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY;
    get_attr[2].id =  SAI_PORT_ATTR_DROP_UNTAGGED;
    get_attr[3].id =  SAI_PORT_ATTR_DROP_TAGGED;

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_port_api_tbl->get_port_attribute(port_id_1, 4, get_attr));
    ASSERT_EQ(get_attr[0].value.u16, 1);
    ASSERT_EQ(get_attr[1].value.u8, 0);
    ASSERT_EQ(get_attr[2].value.booldata, false);
    ASSERT_EQ(get_attr[3].value.booldata, false);

    ASSERT_EQ(SAI_STATUS_SUCCESS,
              p_sai_port_api_tbl->get_port_attribute(port_id_2, 4, get_attr));
    ASSERT_EQ(get_attr[0].value.u16, 1);
    ASSERT_EQ(get_attr[1].value.u8, 0);
    ASSERT_EQ(get_attr[2].value.booldata, false);
    ASSERT_EQ(get_attr[3].value.booldata, false);
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_lag_api_table->remove_lag (lag_id_1));

}
