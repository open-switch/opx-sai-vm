/*
 * filename:
"src/unit_test/sai_bridge_unit_test.cpp
 * (c) Copyright 2015 Dell Inc. All Rights Reserved.
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

extern "C" {
#include "sai.h"
#include "saitypes.h"
#include "saibridge.h"
#include "saiswitch.h"
#include "saivlan.h"
}

#define SAI_MAX_BRIDGE_PORTS  256

sai_status_t sai_bridge_ut_get_bridge_port_from_port(sai_switch_api_t *p_sai_switch_api_tbl,
                                                     sai_bridge_api_t *p_sai_bridge_api_tbl,
                                                     sai_object_id_t switch_id,
                                                     sai_object_id_t port_id,
                                                     sai_object_id_t *bridge_port_id)
{
    uint32_t idx;
    sai_attribute_t attr;
    sai_object_id_t bridge_id;
    sai_object_id_t bp_list[256];
    uint32_t        bp_count;
    sai_status_t    sai_rc;

    attr.id = SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID;

    sai_rc = p_sai_switch_api_tbl->get_switch_attribute(switch_id,1,&attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }
    bridge_id = attr.value.oid;

    attr.id = SAI_BRIDGE_ATTR_PORT_LIST;
    attr.value.objlist.count = 256;
    attr.value.objlist.list = bp_list;

    sai_rc = p_sai_bridge_api_tbl->get_bridge_attribute(bridge_id, 1, &attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }
    bp_count = attr.value.objlist.count;


    for(idx = 0; idx < bp_count; idx++) {
        attr.id = SAI_BRIDGE_PORT_ATTR_PORT_ID;

         p_sai_bridge_api_tbl->get_bridge_port_attribute(bp_list[idx], 1, &attr);
         if(attr.value.oid == port_id) {
             *bridge_port_id = bp_list[idx];
             return SAI_STATUS_SUCCESS;
         }
    }
    return SAI_STATUS_ITEM_NOT_FOUND;
}


sai_status_t sai_bridge_ut_get_port_from_bridge_port(sai_bridge_api_t *p_sai_bridge_api_tbl,
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

sai_object_id_t sai_bridge_ut_get_def_vlan_member_from_bridge_port(sai_vlan_api_t   *p_sai_vlan_api_tbl,
                                                                   sai_object_id_t   switch_id,
                                                                   sai_object_id_t   bridge_port_id)
{
    sai_switch_api_t *p_sai_switch_api_tbl = NULL;
    sai_object_id_t   def_vlan_id;
    sai_attribute_t   attr;
    sai_attribute_t   vlan_mem_attr;
    sai_status_t      sai_rc;
    sai_object_id_t   vlan_mem_list[SAI_MAX_BRIDGE_PORTS];
    uint32_t          idx = 0;

    sai_api_query(SAI_API_SWITCH, (static_cast<void**>
                                 (static_cast<void*>(&p_sai_switch_api_tbl))));

    attr.id = SAI_SWITCH_ATTR_DEFAULT_VLAN_ID;

    sai_rc = p_sai_switch_api_tbl->get_switch_attribute(switch_id, 1, &attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return SAI_NULL_OBJECT_ID;
    }
    def_vlan_id = attr.value.oid;

    attr.id = SAI_VLAN_ATTR_MEMBER_LIST;
    attr.value.objlist.count = SAI_MAX_BRIDGE_PORTS;
    attr.value.objlist.list = vlan_mem_list;

    sai_rc = p_sai_vlan_api_tbl->get_vlan_attribute(def_vlan_id, 1, &attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return SAI_NULL_OBJECT_ID;
    }

    for(idx = 0; idx < attr.value.objlist.count; idx++) {
       memset(&vlan_mem_attr, 0, sizeof(vlan_mem_attr));
       vlan_mem_attr.id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;

       sai_rc = p_sai_vlan_api_tbl->get_vlan_member_attribute (vlan_mem_list[idx], 1,
                                                               &vlan_mem_attr);
       if(vlan_mem_attr.value.oid == bridge_port_id) {
           return vlan_mem_list[idx];
       }
    }
    return SAI_NULL_OBJECT_ID;
}

sai_status_t sai_bridge_ut_remove_bridge_port_from_def_vlan(sai_object_id_t switch_id,
                                                            sai_object_id_t bridge_port_id)
{
    sai_vlan_api_t   *p_sai_vlan_api_tbl = NULL;
    sai_object_id_t   vlan_member_id = SAI_NULL_OBJECT_ID;

    sai_api_query(SAI_API_VLAN, (static_cast<void**>
                        (static_cast<void*>(&p_sai_vlan_api_tbl))));

    vlan_member_id = sai_bridge_ut_get_def_vlan_member_from_bridge_port(p_sai_vlan_api_tbl,
                                                                        switch_id,
                                                                        bridge_port_id);

    if(vlan_member_id == SAI_NULL_OBJECT_ID) {
        return SAI_STATUS_FAILURE;
    }
    return p_sai_vlan_api_tbl->remove_vlan_member(vlan_member_id);
}

sai_status_t sai_bridge_ut_add_bridge_port_to_def_vlan(sai_object_id_t switch_id,
                                                       sai_object_id_t bridge_port_id)
{
    sai_vlan_api_t   *p_sai_vlan_api_tbl = NULL;
    sai_switch_api_t *p_sai_switch_api_tbl = NULL;
    sai_object_id_t   vlan_member_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t   def_vlan_id;
    sai_attribute_t   attr[2];
    sai_status_t      sai_rc;

    sai_api_query(SAI_API_SWITCH, (static_cast<void**>
                                 (static_cast<void*>(&p_sai_switch_api_tbl))));

    sai_api_query(SAI_API_VLAN, (static_cast<void**>
                        (static_cast<void*>(&p_sai_vlan_api_tbl))));

    attr[0].id = SAI_SWITCH_ATTR_DEFAULT_VLAN_ID;

    sai_rc = p_sai_switch_api_tbl->get_switch_attribute(switch_id, 1, attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }
    def_vlan_id = attr[0].value.oid;

    attr[0].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr[0].value.oid  = def_vlan_id;

    attr[1].id= SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr[1].value.oid = bridge_port_id;

    return p_sai_vlan_api_tbl->create_vlan_member(&vlan_member_id, switch_id,
                                                  2, attr);

}

sai_object_id_t sai_bridge_ut_get_def_stp_port_from_bridge_port(sai_stp_api_t   *p_sai_stp_api_tbl,
                                                                sai_object_id_t   switch_id,
                                                                sai_object_id_t   bridge_port_id)
{
    sai_switch_api_t *p_sai_switch_api_tbl = NULL;
    sai_object_id_t   def_stp_id;
    sai_attribute_t   attr;
    sai_attribute_t   stp_port_attr;
    sai_status_t      sai_rc;
    sai_object_id_t   stp_port_list[SAI_MAX_BRIDGE_PORTS];
    uint32_t          idx = 0;

    sai_api_query(SAI_API_SWITCH, (static_cast<void**>
                                 (static_cast<void*>(&p_sai_switch_api_tbl))));

    attr.id = SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID;

    sai_rc = p_sai_switch_api_tbl->get_switch_attribute(switch_id, 1, &attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return SAI_NULL_OBJECT_ID;
    }
    def_stp_id = attr.value.oid;

    attr.id = SAI_STP_ATTR_PORT_LIST;
    attr.value.objlist.count = SAI_MAX_BRIDGE_PORTS;
    attr.value.objlist.list = stp_port_list;

    sai_rc = p_sai_stp_api_tbl->get_stp_attribute(def_stp_id, 1, &attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return SAI_NULL_OBJECT_ID;
    }

    for(idx = 0; idx < attr.value.objlist.count; idx++) {
       memset(&stp_port_attr, 0, sizeof(stp_port_attr));
       stp_port_attr.id = SAI_STP_PORT_ATTR_BRIDGE_PORT;

       sai_rc = p_sai_stp_api_tbl->get_stp_port_attribute (stp_port_list[idx], 1,
                                                               &stp_port_attr);
       if(stp_port_attr.value.oid == bridge_port_id) {
           return stp_port_list[idx];
       }
    }
    return SAI_NULL_OBJECT_ID;
}

sai_status_t sai_bridge_ut_add_bridge_port_to_def_stp(sai_object_id_t switch_id,
                                                      sai_object_id_t bridge_port_id)
{
    sai_stp_api_t    *p_sai_stp_api_tbl = NULL;
    sai_switch_api_t *p_sai_switch_api_tbl = NULL;
    sai_object_id_t   stp_port_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t   def_stp_id;
    sai_attribute_t   attr[3];
    sai_status_t      sai_rc;

    sai_api_query(SAI_API_SWITCH, (static_cast<void**>
                                 (static_cast<void*>(&p_sai_switch_api_tbl))));

    sai_api_query(SAI_API_STP, (static_cast<void**>
                        (static_cast<void*>(&p_sai_stp_api_tbl))));

    attr[0].id = SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID;

    sai_rc = p_sai_switch_api_tbl->get_switch_attribute(switch_id, 1, attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }
    def_stp_id = attr[0].value.oid;

    attr[0].id = SAI_STP_PORT_ATTR_STP;
    attr[0].value.oid  = def_stp_id;

    attr[1].id = SAI_STP_PORT_ATTR_BRIDGE_PORT;
    attr[1].value.oid = bridge_port_id;

    attr[2].id = SAI_STP_PORT_ATTR_STATE;
    attr[2].value.oid = SAI_STP_PORT_STATE_FORWARDING;

    return p_sai_stp_api_tbl->create_stp_port(&stp_port_id, switch_id, 3, attr);
}

sai_status_t sai_bridge_ut_remove_bridge_port_from_def_stp(sai_object_id_t switch_id,
                                                            sai_object_id_t bridge_port_id)
{
    sai_stp_api_t   *p_sai_stp_api_tbl = NULL;
    sai_object_id_t   stp_port_id = SAI_NULL_OBJECT_ID;

    sai_api_query(SAI_API_STP, (static_cast<void**>
                        (static_cast<void*>(&p_sai_stp_api_tbl))));

    stp_port_id = sai_bridge_ut_get_def_stp_port_from_bridge_port(p_sai_stp_api_tbl,
                                                                  switch_id,
                                                                  bridge_port_id);

    if(stp_port_id == SAI_NULL_OBJECT_ID) {
        return SAI_STATUS_FAILURE;
    }
    return p_sai_stp_api_tbl->remove_stp_port(stp_port_id);
}

sai_status_t sai_bridge_ut_create_bridge_port(sai_bridge_api_t * p_sai_bridge_api_tbl,
                                              sai_object_id_t switch_id,
                                              sai_object_id_t port_id,
                                              bool def_vlan_add,
                                              sai_object_id_t *bridge_port_id)
{
    sai_status_t    sai_rc;
    sai_attribute_t bridge_port_attr[2];

    bridge_port_attr[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    bridge_port_attr[0].value.s32 = SAI_BRIDGE_PORT_TYPE_PORT;

    bridge_port_attr[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    bridge_port_attr[1].value.oid = port_id;

    sai_rc =  p_sai_bridge_api_tbl->create_bridge_port(bridge_port_id, switch_id,
                                                       2, bridge_port_attr);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }
    if(def_vlan_add) {
        sai_rc = sai_bridge_ut_add_bridge_port_to_def_vlan(switch_id, *bridge_port_id);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            p_sai_bridge_api_tbl->remove_bridge_port(*bridge_port_id);
        }
    }
    if(sai_rc == SAI_STATUS_SUCCESS) {
        sai_rc = sai_bridge_ut_add_bridge_port_to_def_stp(switch_id, *bridge_port_id);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            p_sai_bridge_api_tbl->remove_bridge_port(*bridge_port_id);
        }
    }
    return sai_rc;
}

sai_status_t sai_bridge_ut_remove_bridge_port(sai_bridge_api_t * p_sai_bridge_api_tbl,
                                              sai_object_id_t switch_id,
                                              sai_object_id_t bridge_port_id,
                                              bool def_vlan_remove)
{
    sai_status_t sai_rc;
    if(def_vlan_remove) {
        sai_rc = sai_bridge_ut_remove_bridge_port_from_def_vlan(switch_id, bridge_port_id);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            return sai_rc;
        }
    }
    sai_rc = sai_bridge_ut_remove_bridge_port_from_def_stp(switch_id, bridge_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    sai_rc = p_sai_bridge_api_tbl->remove_bridge_port(bridge_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        if(def_vlan_remove) {
            sai_bridge_ut_add_bridge_port_to_def_vlan(switch_id, bridge_port_id);
        }
        sai_bridge_ut_add_bridge_port_to_def_stp(switch_id, bridge_port_id);
    }
    return sai_rc;
}

sai_status_t sai_bridge_ut_clean_bridge_port_configs(sai_object_id_t switch_id,
                                                     sai_object_id_t port_id)
{
    sai_object_id_t   bridge_port_id;
    sai_bridge_api_t *p_sai_bridge_api_tbl = NULL;
    sai_switch_api_t *p_sai_switch_api_tbl = NULL;
    sai_status_t      sai_rc;

    sai_api_query(SAI_API_SWITCH, (static_cast<void**>
                                 (static_cast<void*>(&p_sai_switch_api_tbl))));

    sai_api_query(SAI_API_BRIDGE, (static_cast<void**>
                                 (static_cast<void*>(&p_sai_bridge_api_tbl))));

    sai_rc = sai_bridge_ut_get_bridge_port_from_port(p_sai_switch_api_tbl,p_sai_bridge_api_tbl,
                                                     switch_id, port_id, &bridge_port_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        printf("Error %d in getting bridge port for port 0x%" PRIx64 "\r\n", sai_rc, port_id);
    }
    return sai_bridge_ut_remove_bridge_port(p_sai_bridge_api_tbl, switch_id, bridge_port_id, true);
}

sai_status_t sai_bridge_ut_init_bridge_port_configs(sai_object_id_t switch_id,
                                                    sai_object_id_t port_id)
{
    sai_object_id_t   bridge_port_id;
    sai_bridge_api_t *p_sai_bridge_api_tbl = NULL;
    sai_api_query(SAI_API_BRIDGE, (static_cast<void**>
                 (static_cast<void*>(&p_sai_bridge_api_tbl))));

    return sai_bridge_ut_create_bridge_port(p_sai_bridge_api_tbl, switch_id, port_id,
                                            true, &bridge_port_id);
}

