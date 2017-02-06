/*
 * Copyright (c) 2016 Dell Inc.
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
 * @file sai_routing_db_api.cpp
 *
 * @brief This file contains the function definitions for updating the
 *        SQL DB tables related to the SAI routing objects in VM
 *        environment.
 */

#include "sai_vm_db_utils.h"
#include "sai_routing_db_api.h"
#include "db_sql_ops.h"
#include "sai_vm_defs.h"
#include "sai_vm_event_log.h"
#include "sai_l3_common.h"
#include "sai_l3_util.h"

#include "sairouter.h"
#include "sairouterintf.h"
#include "saineighbor.h"
#include "sainexthop.h"
#include "sainexthopgroup.h"
#include "sairoute.h"
#include "saitypes.h"
#include "saistatus.h"

#include "ds_common_types.h"
#include "std_mac_utils.h"
#include "std_error_codes.h"
#include "std_assert.h"
#include <string.h>
#include <inttypes.h>

#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>

using namespace std;

static inline const char *ip_addr_to_str (int type, void *ip_addr,
                                           char *buffer, int size)
{
    const char* result = inet_ntop (type, ip_addr, buffer, size);

    return result;
}

static sai_object_id_t sai_vm_route_node_nh_id_get (sai_fib_route_t *p_route)
{
    if (!p_route) {
        return 0;
    }

    if ((p_route->nh_type == SAI_OBJECT_TYPE_NEXT_HOP) &&
        (p_route->nh_info.nh_node)) {
        return (p_route->nh_info.nh_node)->next_hop_id;
    } else if ((p_route->nh_type == SAI_OBJECT_TYPE_NEXT_HOP_GROUP) &&
               (p_route->nh_info.group_node)) {
        return (p_route->nh_info.group_node)->key.group_id;
    } else {
        return 0;
    }
}

sai_status_t sai_router_create_db_entry (sai_object_id_t vrf_id,
                                         sai_fib_vrf_t *p_vrf_node)
{
    char   mac_addr [SAI_VM_MAX_BUFSZ];
    uint_t npu_vrf_id = (uint_t) sai_uoid_npu_obj_id_get (vrf_id);

    STD_ASSERT (p_vrf_node != NULL);

    string vr_id_str = std::to_string (npu_vrf_id);
    string v4_state_str = (p_vrf_node->v4_admin_state)? "1" : "0" ;
    string v6_state_str = (p_vrf_node->v6_admin_state)? "1" : "0" ;
    string ttl1_action_str = std::to_string (p_vrf_node->ttl0_1_pkt_action);
    string ip_options_str = std::to_string (p_vrf_node->ip_options_pkt_action);

    std_mac_to_string ((const hal_mac_addr_t *)p_vrf_node->src_mac, mac_addr,
                       sizeof (mac_addr));
    string mac_addr_str = string (mac_addr);

    string insert_str =
        string ("( ") + vr_id_str + ", " + v4_state_str + ", " + v6_state_str +
        ",\"" + mac_addr_str + "\", " + ttl1_action_str + " , " +
        ip_options_str +  string (")");

    if (db_sql_insert (sai_vm_get_db_handle(), "SAI_ROUTER", insert_str.c_str())
        != STD_ERR_OK) {
        SAI_VM_DB_LOG_TRACE ("Error inserting router entry with vr_id %s.",
                             vr_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_router_delete_db_entry (sai_object_id_t vrf_id)
{
    uint_t npu_vrf_id = (uint_t) sai_uoid_npu_obj_id_get (vrf_id);
    string vr_id_str = std::to_string (npu_vrf_id);

    string delete_str = string ("( vr_id=") + vr_id_str + string (")");

    if (db_sql_delete (sai_vm_get_db_handle(), "SAI_ROUTER", delete_str.c_str())
        != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error deleting router entry with vr_id %s.",
                           vr_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_router_set_db_entry (sai_fib_vrf_t *p_vrf, uint_t attr_flag)
{
    string attr_str;
    string value_str;
    char   mac_addr [SAI_VM_MAX_BUFSZ];
    uint_t npu_vrf_id = 0;

    STD_ASSERT (p_vrf != NULL);

    npu_vrf_id = (uint_t) sai_uoid_npu_obj_id_get (p_vrf->vrf_id);
    string vr_id_str = std::to_string (npu_vrf_id);

    if (attr_flag & SAI_FIB_SRC_MAC_ATTR_FLAG) {
        std_mac_to_string (&p_vrf->src_mac, mac_addr, sizeof (mac_addr));

        attr_str = "SRC_MAC_ADDRESS";
        value_str = "\""+ string (mac_addr) + "\"";
    } else if (attr_flag & SAI_FIB_V4_ADMIN_STATE_ATTR_FLAG) {
        attr_str = "ADMIN_V4_STATE";
        value_str = (p_vrf->v4_admin_state)? "1" : "0" ;
    } else if (attr_flag & SAI_FIB_V6_ADMIN_STATE_ATTR_FLAG) {
        attr_str = "ADMIN_V6_STATE";
        value_str = (p_vrf->v6_admin_state)? "1" : "0" ;
    } else if (attr_flag & SAI_FIB_IP_OPTIONS_ATTR_FLAG) {
        attr_str = "VIOLATION_IP_OPTIONS";
        value_str = std::to_string (p_vrf->ip_options_pkt_action);
    } else if (attr_flag & SAI_FIB_TTL_VIOLATION_ATTR_FLAG) {
        attr_str = "VIOLATION_TTL1_ACTION";
        value_str = std::to_string (p_vrf->ttl0_1_pkt_action);
    } else {
        /* Stub for any optional attributes */
        SAI_VM_DB_LOG_TRACE ("Attribute flag 0x%x is not set for VRF object.",
                             attr_flag);

        return SAI_STATUS_SUCCESS;
    }

    string cond_str = string ("( vr_id=") + vr_id_str + string (")");

    if (db_sql_set_attribute (sai_vm_get_db_handle(),"SAI_ROUTER",
                              attr_str.c_str(), value_str.c_str(),
                              cond_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error setting router entry with vr_id: %s attr: %s "
                           "value: %s", vr_id_str.c_str(), attr_str.c_str(),
                           value_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_routerintf_create_db_entry (sai_object_id_t rif_id,
                                             sai_fib_router_interface_t *p_rif)
{
    string port_str = "NULL";
    string vlan_str = "NULL";
    char   mac_addr [SAI_VM_MAX_BUFSZ];
    uint_t npu_vrf_id = 0;
    uint_t npu_rif_id = 0;
    uint_t npu_port_id = 0;

    STD_ASSERT (p_rif != NULL);

    npu_rif_id = (uint_t) sai_uoid_npu_obj_id_get (rif_id);
    string rif_id_str = std::to_string (npu_rif_id);

    npu_vrf_id = (uint_t) sai_uoid_npu_obj_id_get (p_rif->vrf_id);
    string vrf_str = std::to_string (npu_vrf_id);

    string type_str = std::to_string (p_rif->type);

    if (p_rif->type == SAI_ROUTER_INTERFACE_TYPE_VLAN) {
        vlan_str = std::to_string (p_rif->attachment.vlan_id);
    } else {
        npu_port_id =
            (uint_t) sai_uoid_npu_obj_id_get (p_rif->attachment.port_id);
        port_str = std::to_string (npu_port_id);
    }

    std_mac_to_string ((const hal_mac_addr_t *)p_rif->src_mac, mac_addr,
                       sizeof (mac_addr));
    string mac_addr_str = string (mac_addr);

    string v4_state_str = (p_rif->v4_admin_state)? "1" : "0";
    string v6_state_str = (p_rif->v6_admin_state)? "1" : "0";
    string mtu_str = std::to_string (p_rif->mtu);

    string insert_str = string ("( ") + rif_id_str + ", " + vrf_str + ", " +
        type_str + ", " + port_str + ", " + vlan_str + ",\"" + mac_addr_str +
        "\", " + v4_state_str + " , " + v6_state_str + ", " + mtu_str +
        string(")");

    if (db_sql_insert (sai_vm_get_db_handle(), "SAI_ROUTER_INTF",
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting routerintf entry with rif_id %s.",
                           rif_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_routerintf_delete_db_entry (sai_object_id_t rif_id)
{
    uint_t npu_rif_id = (uint_t) sai_uoid_npu_obj_id_get (rif_id);

    string rif_id_str = std::to_string (npu_rif_id);

    string delete_str = string ("( rif_id=") + rif_id_str + string (")");

    if (db_sql_delete (sai_vm_get_db_handle(), "SAI_ROUTER_INTF",
                       delete_str.c_str()) != STD_ERR_OK) {
       SAI_VM_DB_LOG_ERR ("Error deleting router intf entry with rif_id %s.",
                          rif_id_str.c_str());

       return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_routerintf_set_db_entry (sai_fib_router_interface_t *p_rif,
                                          uint_t attr_flag)
{
    string attr_str;
    string value_str;
    char   mac_addr [SAI_VM_MAX_BUFSZ];
    uint_t npu_rif_id = 0;

    STD_ASSERT (p_rif != NULL);

    npu_rif_id = (uint_t) sai_uoid_npu_obj_id_get (p_rif->rif_id);
    string rif_id_str = std::to_string (npu_rif_id);

    if (attr_flag & SAI_FIB_SRC_MAC_ATTR_FLAG) {
        std_mac_to_string ((const hal_mac_addr_t *)p_rif->src_mac, mac_addr,
                           sizeof (mac_addr));

        attr_str = "SRC_MAC_ADDRESS";
        value_str = "\""+ string (mac_addr)+ "\"";
    } else if (attr_flag & SAI_FIB_V4_ADMIN_STATE_ATTR_FLAG) {
        attr_str = "ADMIN_V4_STATE";
        value_str = (p_rif->v4_admin_state)? "1" : "0";
    } else if (attr_flag & SAI_FIB_V6_ADMIN_STATE_ATTR_FLAG) {
        attr_str = "ADMIN_V6_STATE";
        value_str = (p_rif->v6_admin_state)? "1" : "0";
    } else if (attr_flag & SAI_FIB_MTU_ATTR_FLAG) {
        attr_str = "MTU";
        value_str = std::to_string (p_rif->mtu);
    } else {
        /* Stub for any optional attributes */
        SAI_VM_DB_LOG_TRACE ("Attribute flag 0x%x is not set for RIF object.",
                             attr_flag);

        return SAI_STATUS_SUCCESS;
    }

    string cond_str = string ("( rif_id=") + rif_id_str + string (")");

    if (db_sql_set_attribute (sai_vm_get_db_handle(), "SAI_ROUTER_INTF",
                              attr_str.c_str(), value_str.c_str(),
                              cond_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error setting router intf entry with rif_id %s, "
                           "attr: %s, value: %s.", rif_id_str.c_str(),
                           attr_str.c_str(), value_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_route_create_db_entry (sai_fib_route_t *p_route)
{
    string          ip_addr_str;
    string          nh_id_str = "NULL";
    sai_object_id_t nh_obj_id = 0;
    uint_t          npu_vrf_id = 0;
    uint_t          npu_nh_id = 0;
    char            buff [SAI_VM_MAX_BUFSZ];

    STD_ASSERT (p_route != NULL);

    npu_vrf_id = (uint_t) sai_uoid_npu_obj_id_get (p_route->vrf_id);
    string vrf_str =std::to_string (npu_vrf_id);
    string prefix_len_str = std::to_string (p_route->prefix_len);
    string meta_data = std::to_string (p_route->meta_data);

    if (p_route->key.prefix.addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
        ip_addr_str = ip_addr_to_str (AF_INET,
                                      (void *)&(p_route->key.prefix.addr.ip4),
                                      buff, sizeof (buff));
    }
    else if (p_route->key.prefix.addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
        ip_addr_str = ip_addr_to_str (AF_INET6,
                                      (void *)&(p_route->key.prefix.addr.ip6),
                                      buff, sizeof (buff));
    }

    if ((nh_obj_id = sai_vm_route_node_nh_id_get (p_route)) != 0) {
        npu_nh_id = (uint_t) sai_uoid_npu_obj_id_get (nh_obj_id);
        nh_id_str = std::to_string (npu_nh_id);
    }

    string pkt_action_str = std::to_string (p_route->packet_action);
    string trap_prio_str = std::to_string (p_route->trap_priority);

    string insert_str = string ("( ") + vrf_str + ",\"" + ip_addr_str +
        "\", " + prefix_len_str + " , " + pkt_action_str + ", " + trap_prio_str
        + " , " + nh_id_str + ", " + meta_data + string(")");

    if (db_sql_insert (sai_vm_get_db_handle(), "SAI_ROUTE",
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting route entry with vr_id: %s, "
                           "ip addr: %s, prefix_len: %s.", vrf_str.c_str(),
                           ip_addr_str.c_str(), prefix_len_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_route_delete_db_entry (sai_fib_route_t *p_route)
{
    uint_t npu_vrf_id = 0;
    string ip_addr_str;
    char   buff [SAI_VM_MAX_BUFSZ];

    STD_ASSERT (p_route != NULL);

    npu_vrf_id = (uint_t) sai_uoid_npu_obj_id_get (p_route->vrf_id);
    string vrf_str =std::to_string (npu_vrf_id);
    string prefix_len_str = std::to_string (p_route->prefix_len);

    if (p_route->key.prefix.addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
        ip_addr_str = ip_addr_to_str (AF_INET,
                                      (void *)&(p_route->key.prefix.addr.ip4),
                                      buff, sizeof (buff));
    }
    else if (p_route->key.prefix.addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
        ip_addr_str = ip_addr_to_str (AF_INET6,
                                      (void *)&(p_route->key.prefix.addr.ip6),
                                      buff, sizeof (buff));
    }

    string delete_str = string ("( vr_id=") + vrf_str + " AND ip_addr =\"" +
        ip_addr_str + "\" AND prefix_len=" + prefix_len_str + string(")");

    if (db_sql_delete (sai_vm_get_db_handle(), "SAI_ROUTE",
                       delete_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error deleting route entry with vr_id: %s, "
                           "ip addr: %s and prefix_len: %s", vrf_str.c_str(),
                           ip_addr_str.c_str(), prefix_len_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_route_set_db_entry (sai_fib_route_t *p_route, uint_t attr_flag)
{
    string          ip_addr_str;
    sai_object_id_t nh_obj_id = 0;
    uint_t          npu_vrf_id = 0;
    uint_t          npu_nh_id = 0;
    string          attr_str;
    string          value_str;
    char            buff [SAI_VM_MAX_BUFSZ];

    STD_ASSERT (p_route != NULL);

    npu_vrf_id = (uint_t) sai_uoid_npu_obj_id_get (p_route->vrf_id);
    string vrf_str =std::to_string (npu_vrf_id);
    string prefix_len_str = std::to_string (p_route->prefix_len);

    if (p_route->key.prefix.addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
        ip_addr_str = ip_addr_to_str (AF_INET,
                                      (void *)&(p_route->key.prefix.addr.ip4),
                                      buff, sizeof (buff));
    }
    else if (p_route->key.prefix.addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
        ip_addr_str = ip_addr_to_str (AF_INET6,
                                      (void *)&(p_route->key.prefix.addr.ip6),
                                      buff, sizeof (buff));
    }

    if (attr_flag == SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION) {
        attr_str = string ("PACKET_ACTION");
        value_str = std::to_string (p_route->packet_action);
    } else if (attr_flag == SAI_ROUTE_ENTRY_ATTR_TRAP_PRIORITY) {
        attr_str = string ("TRAP_PRIORITY");
        value_str = std::to_string (p_route->trap_priority);
    } else if (attr_flag == SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID) {
        nh_obj_id = sai_vm_route_node_nh_id_get (p_route);

        npu_nh_id = (uint_t) sai_uoid_npu_obj_id_get (nh_obj_id);

        attr_str = string ("NEXT_HOP_ID");
        value_str = std::to_string (npu_nh_id);
    } else if (attr_flag == SAI_ROUTE_ENTRY_ATTR_META_DATA) {
        attr_str = string ("META_DATA");
        value_str = std::to_string (p_route->meta_data);
    } else {
        SAI_VM_DB_LOG_ERR ("Attribute %d is not valid for Route object.",
                           attr_flag);

        return SAI_STATUS_FAILURE;
    }

    string cond_str = string ("( vr_id=") + vrf_str + " AND ip_addr=\"" +
        ip_addr_str + "\" AND prefix_len=" + prefix_len_str + string (")");

    if (db_sql_set_attribute (sai_vm_get_db_handle(), "SAI_ROUTE",
                              attr_str.c_str(), value_str.c_str(),
                              cond_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error setting route entry with vr_id: %s, ip addr: "
                           "%s, prefix_len: %s, attr: %s, value: %s.",
                           vrf_str.c_str(), ip_addr_str.c_str(),
                           prefix_len_str.c_str(), attr_str.c_str(),
                           value_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_neighbor_create_db_entry (sai_fib_nh_t *p_neighbor)
{
    sai_ip_address_t *p_nh_ip = NULL;
    string            ip_addr_str;
    uint_t            npu_rif_id = 0;
    char              buff [SAI_VM_MAX_BUFSZ];
    char              mac_addr [SAI_VM_MAX_BUFSZ];

    STD_ASSERT (p_neighbor != NULL);

    npu_rif_id = (uint_t) sai_uoid_npu_obj_id_get (p_neighbor->key.rif_id);

    string rif_id_str = std::to_string (npu_rif_id);
    string pkt_action_str = std::to_string (p_neighbor->packet_action);
    string no_host_route_str = (p_neighbor->no_host_route) ? "1" : "0" ;
    string meta_data = std::to_string (p_neighbor->meta_data);

    p_nh_ip = &p_neighbor->key.info.ip_nh.ip_addr;

    if (p_nh_ip->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
        ip_addr_str = ip_addr_to_str (AF_INET, (void *)&(p_nh_ip->addr.ip4),
                                      buff, sizeof (buff));
    }
    else if (p_nh_ip->addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
        ip_addr_str = ip_addr_to_str (AF_INET6, (void *)&(p_nh_ip->addr.ip6),
                                      buff, sizeof (buff));
    }

    std_mac_to_string ((const hal_mac_addr_t *)p_neighbor->mac_addr, mac_addr,
                       sizeof (mac_addr));
    string mac_addr_str = string (mac_addr);

    string insert_str = string ("( ") + rif_id_str + ",\""+ ip_addr_str +
        "\", \"" + mac_addr_str + "\", " + pkt_action_str + " , " + no_host_route_str +
        ", " + meta_data + string(")");

    if (db_sql_insert (sai_vm_get_db_handle(), "SAI_NEIGHBOR",
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting neighbor entry with rif_id: %s and "
                           "ip address: %s.", rif_id_str.c_str(),
                           ip_addr_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_neighbor_delete_db_entry (sai_fib_nh_t *p_neighbor)
{
    sai_ip_address_t *p_nh_ip = NULL;
    string            ip_addr_str;
    uint_t            npu_rif_id = 0;
    char              buff [SAI_VM_MAX_BUFSZ];

    STD_ASSERT (p_neighbor != NULL);

    npu_rif_id = (uint_t) sai_uoid_npu_obj_id_get (p_neighbor->key.rif_id);

    string rif_id_str = std::to_string (npu_rif_id);

    p_nh_ip = &p_neighbor->key.info.ip_nh.ip_addr;

    if (p_nh_ip->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
        ip_addr_str = ip_addr_to_str (AF_INET, (void *)&(p_nh_ip->addr.ip4),
                                      buff, sizeof (buff));
    }
    else if (p_nh_ip->addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
        ip_addr_str = ip_addr_to_str (AF_INET6, (void *)&(p_nh_ip->addr.ip6),
                                      buff, sizeof (buff));
    }

    string delete_str = string ("( rif_id=") + rif_id_str + " AND ip_addr=\"" +
        ip_addr_str + "\"" + string (")");

    if (db_sql_delete (sai_vm_get_db_handle() , "SAI_NEIGHBOR",
                       delete_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error deleting neighbor entry with rif_id: %s and "
                           "ip addr: %s", rif_id_str.c_str(),
                           ip_addr_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_neighbor_set_db_entry (sai_fib_nh_t *p_neighbor,
                                        uint_t attr_flags)
{
    sai_ip_address_t *p_nh_ip = NULL;
    uint_t            npu_rif_id = 0;
    string            ip_addr_str;
    string            attr_str;
    string            value_str;
    char              buff [SAI_VM_MAX_BUFSZ];
    char              mac_addr [SAI_VM_MAX_BUFSZ];

    STD_ASSERT (p_neighbor != NULL);

    npu_rif_id = (uint_t) sai_uoid_npu_obj_id_get (p_neighbor->key.rif_id);

    string rif_id_str = std::to_string (npu_rif_id);

    p_nh_ip = &p_neighbor->key.info.ip_nh.ip_addr;

    if (p_nh_ip->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
        ip_addr_str = ip_addr_to_str (AF_INET, (void *)&(p_nh_ip->addr.ip4),
                                      buff, sizeof (buff));
    }
    else if (p_nh_ip->addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
        ip_addr_str = ip_addr_to_str (AF_INET6, (void *)&(p_nh_ip->addr.ip6),
                                      buff, sizeof (buff));
    }

    if (attr_flags & SAI_FIB_NEIGHBOR_PKT_ACTION_ATTR_FLAG) {
        attr_str = string ("PACKET_ACTION");
        value_str = std::to_string (p_neighbor->packet_action);
    } else if (attr_flags & SAI_FIB_NEIGHBOR_DEST_MAC_ATTR_FLAG) {
        std_mac_to_string ((const hal_mac_addr_t *)p_neighbor->mac_addr,
                           mac_addr, sizeof (mac_addr));

        attr_str = string ("DST_MAC_ADDRESS");
        value_str = string ("\"") + string (mac_addr) + string ("\"");
    } else if (attr_flags & SAI_FIB_NEIGHBOR_NO_HOST_ROUTE_ATTR_FLAG) {
        attr_str  = string ("NO_HOST_ROUTE");
        value_str = (p_neighbor->no_host_route) ? "1" : "0" ;
    } else if (attr_flags & SAI_FIB_NEIGHBOR_META_DATA_ATTR_FLAG) {
        attr_str  = string ("META_DATA");
        value_str = std::to_string (p_neighbor->meta_data);
    } else {
        /* Stub for any optional attributes */
        SAI_VM_DB_LOG_TRACE ("Attribute flag 0x%x is not set for Neighbor "
                             "object.", attr_flags);

        return SAI_STATUS_SUCCESS;
    }


    string cond_str = string ("( rif_id=") + rif_id_str + " AND ip_addr=\""+
        ip_addr_str + "\"" + string (")");

    if (db_sql_set_attribute (sai_vm_get_db_handle(), "SAI_NEIGHBOR",
                              attr_str.c_str(), value_str.c_str(),
                              cond_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error setting neighbor entry with rif_id: %s and "
                           "ip addr: %s, attr: %s, value: %s.",
                           rif_id_str.c_str(), ip_addr_str.c_str(),
                           attr_str.c_str(), value_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_nexthop_create_db_entry (sai_object_id_t nh_id,
                                          sai_fib_nh_t *p_next_hop)
{
    sai_ip_address_t *p_nh_ip = NULL;
    uint_t            npu_nh_id = 0;
    uint_t            npu_rif_id = 0;
    string            ip_addr_str;
    char              buff [SAI_VM_MAX_BUFSZ];

    STD_ASSERT (p_next_hop != NULL);

    p_nh_ip = &p_next_hop->key.info.ip_nh.ip_addr;

    if (p_nh_ip->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
        ip_addr_str = ip_addr_to_str (AF_INET, (void *)&(p_nh_ip->addr.ip4),
                                      buff, sizeof (buff));
    }
    else if (p_nh_ip->addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
        ip_addr_str = ip_addr_to_str (AF_INET6, (void *)&(p_nh_ip->addr.ip6),
                                      buff, sizeof (buff));
    }

    npu_nh_id = (uint_t) sai_uoid_npu_obj_id_get (nh_id);
    string nh_id_str = std::to_string (npu_nh_id);

    npu_rif_id = (uint_t) sai_uoid_npu_obj_id_get (p_next_hop->key.rif_id);
    string rif_id_str = std::to_string (npu_rif_id);

    string type_str = std::to_string (p_next_hop->key.nh_type);

    string insert_str = string ("( ") + nh_id_str + ", " + type_str + ", \"" +
        ip_addr_str + "\", " + rif_id_str + string (")");

    if (db_sql_insert (sai_vm_get_db_handle(), "SAI_NEXT_HOP",
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting nexthop entry with nh_id: %s.",
                           nh_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_nexthop_delete_db_entry (sai_object_id_t nh_id)
{
    uint_t npu_nh_id = sai_uoid_npu_obj_id_get (nh_id);

    string nh_id_str = std::to_string (npu_nh_id);

    string delete_str = string ("( nhop_id=") + nh_id_str + string (")");

    if (db_sql_delete (sai_vm_get_db_handle(), "SAI_NEXT_HOP",
                       delete_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error deleting nexthop entry with nh_id: %s.",
                           nh_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_nh_group_set_db_entry_nh_count (sai_object_id_t grp_id,
                                                        uint_t nh_count)
{
    string attr_str = "NEXT_HOP_COUNT";
    string value_str = std::to_string (nh_count);

    uint_t npu_grp_id = (uint_t) sai_uoid_npu_obj_id_get (grp_id);

    string nh_grp_id_str = std::to_string (npu_grp_id);
    string cond_str = string ("( nhop_group_id=") + nh_grp_id_str + string(")");

    if (db_sql_set_attribute (sai_vm_get_db_handle(), "SAI_NEXT_HOP_GROUP",
                              attr_str.c_str(), value_str.c_str(),
                              cond_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error setting NH Group entry with grp_id: %s, "
                           "attr: %s, value: %s.", nh_grp_id_str.c_str(),
                           attr_str.c_str(), value_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_nh_group_list_db_populate (sai_object_id_t nh_grp_id,
                                                   uint_t nh_count,
                                                   sai_fib_nh_t *ap_next_hop [])
{
    uint_t idx = 0;
    string insert_str;
    string nh_id_str;
    uint_t npu_nh_id = 0;
    uint_t npu_nh_grp_id = (uint_t) sai_uoid_npu_obj_id_get (nh_grp_id);

    string nh_grp_id_str = std::to_string (npu_nh_grp_id);

    STD_ASSERT (ap_next_hop != NULL);

    for (idx = 0; idx < nh_count; idx ++) {
        npu_nh_id =
            (uint_t) sai_uoid_npu_obj_id_get (ap_next_hop [idx]->next_hop_id);

        nh_id_str = std::to_string (npu_nh_id);

        insert_str =
            string ("( ") + nh_grp_id_str + ", " + nh_id_str + string (")");

        if (db_sql_insert (sai_vm_get_db_handle(), "SAI_NEXT_HOP_GROUP_LIST",
                           insert_str.c_str()) != STD_ERR_OK) {
            SAI_VM_DB_LOG_ERR ("Error inserting entry for NH GRP ID: %s, "
                               "NH ID: %s to SAI_NEXT_HOP_GROUP_LIST table.",
                               nh_grp_id_str.c_str(), nh_id_str.c_str());

            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_nh_group_add_nh_list_to_db_entry (sai_object_id_t nh_grp_id,
                                                   uint_t nh_count_added,
                                                   sai_fib_nh_t *ap_next_hop [],
                                                   uint_t total_nh_count)
{
    STD_ASSERT (ap_next_hop != NULL);

    if (sai_nh_group_list_db_populate (nh_grp_id, nh_count_added, ap_next_hop)
        != SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error adding Next-hops to the NEXT_HOP_GROUP_LIST "
                           "for nh_grp_id: 0x%" PRIx64 ".", nh_grp_id);

        return SAI_STATUS_FAILURE;
    }

    if (sai_nh_group_set_db_entry_nh_count (nh_grp_id, total_nh_count) !=
        SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error updating NH count to the NH Group entry for "
                           "nh_grp_id: 0x%" PRIx64 ".", nh_grp_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_nh_group_delete_nh_list_from_db_entry (
sai_object_id_t nh_grp_id, uint_t nh_count_deleted,
sai_fib_nh_t *ap_next_hop [], uint_t total_nh_count)
{
    uint_t idx = 0;
    string delete_str;
    string nh_id_str;
    uint_t npu_nh_id = 0;
    uint_t npu_nh_grp_id = (uint_t) sai_uoid_npu_obj_id_get (nh_grp_id);

    string nh_grp_id_str = std::to_string (npu_nh_grp_id);

    STD_ASSERT (ap_next_hop != NULL);

    for (idx = 0; idx < nh_count_deleted; idx ++) {
        npu_nh_id =
            (uint_t) sai_uoid_npu_obj_id_get (ap_next_hop [idx]->next_hop_id);

        nh_id_str = std::to_string (npu_nh_id);

        delete_str = string ("( nhop_group_id=") + nh_grp_id_str +
            " AND NEXT_HOP_ID =" + nh_id_str + string (")") + " limit 1";

        if (db_sql_delete (sai_vm_get_db_handle(), "SAI_NEXT_HOP_GROUP_LIST",
                           delete_str.c_str()) != STD_ERR_OK) {
            SAI_VM_DB_LOG_ERR ("Error deleting NEXT_HOP_GROUP_LIST entry with "
                               "NH GRP ID: %s, NH ID: %s.",
                               nh_grp_id_str.c_str(), nh_id_str.c_str());

            return SAI_STATUS_FAILURE;
        }
    }

    if (sai_nh_group_set_db_entry_nh_count (nh_grp_id, total_nh_count) !=
        SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error updating NH count to the NH Group entry for "
                           "nh_grp_id: 0x%" PRIx64 ".", nh_grp_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_nh_group_create_db_entry (sai_object_id_t nh_grp_id,
                                           sai_next_hop_group_type_t type,
                                           uint_t nh_count,
                                           sai_fib_nh_t *ap_next_hop [])
{
    string nh_count_str = std::to_string (nh_count);
    string nh_grp_type_str = std::to_string (type);
    uint_t npu_nh_grp_id = (uint_t) sai_uoid_npu_obj_id_get (nh_grp_id);

    string nh_grp_id_str = std::to_string (npu_nh_grp_id);

    STD_ASSERT (ap_next_hop != NULL);

    string insert_str =
        string ("( ") + nh_grp_id_str + ", " + nh_count_str + ", " +
        nh_grp_type_str + string (")");

    if (db_sql_insert (sai_vm_get_db_handle(), "SAI_NEXT_HOP_GROUP",
                       insert_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error inserting NH group entry with nh_grp_id: %s.",
                           nh_grp_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    if (sai_nh_group_list_db_populate (nh_grp_id, nh_count, ap_next_hop) !=
        SAI_STATUS_SUCCESS) {
        SAI_VM_DB_LOG_ERR ("Error adding Next-hops to the NEXT_HOP_GROUP_LIST "
                           "for nh_grp_id: %s.", nh_grp_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_nh_group_delete_db_entry (sai_object_id_t nh_grp_id)
{
    uint_t npu_nh_grp_id = (uint_t) sai_uoid_npu_obj_id_get (nh_grp_id);

    string grp_id_str = std::to_string (npu_nh_grp_id);
    string delete_str = string ("( nhop_group_id=") + grp_id_str + string(")");

    if (db_sql_delete (sai_vm_get_db_handle(), "SAI_NEXT_HOP_GROUP",
                       delete_str.c_str()) != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error deleting NH Group entry with nh_grp_id: %s.",
                           grp_id_str.c_str());

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}
