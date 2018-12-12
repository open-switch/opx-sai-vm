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

/**
* @file sai_tunnel_encap_nh.c
*
* @brief This file contains implementation for setting up underlay
*        tunnel next hops.
*************************************************************************/
#include "sai_tunnel_api_utils.h"
#include "sai_l3_common.h"
#include "sai_tunnel_util.h"
#include "sai_l3_api_utils.h"

#include "saitypes.h"
#include "saistatus.h"

#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

sai_status_t sai_tunnel_encap_nh_setup(sai_ip_address_t *p_remote_ip,
                                       dn_sai_tunnel_t *p_tunnel)
{
     sai_status_t sai_rc = SAI_STATUS_FAILURE;
     sai_fib_nh_t nh_node;
     sai_fib_nh_t *p_underlay_nh = NULL;
     char ip_addr_str [SAI_FIB_MAX_BUFSZ];

     sai_fib_lock();

     do {

         p_underlay_nh = sai_fib_ip_next_hop_node_get(SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP,
                                                      p_tunnel->underlay_rif,
                                                      p_remote_ip, p_tunnel->tunnel_type);
         if(NULL == p_underlay_nh) {
             /*Create a new underlay tunnel encap*/
             memset(&nh_node, 0, sizeof(nh_node));

             nh_node.key.nh_type = SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP;
             sai_fib_ip_addr_copy(sai_fib_next_hop_ip_addr(&nh_node), p_remote_ip);
             nh_node.key.rif_id      =  p_tunnel->underlay_rif;
             nh_node.key.tunnel_type =  p_tunnel->tunnel_type;
             nh_node.vrf_id          =  p_tunnel->underlay_vrf;
             nh_node.tunnel_id       =  p_tunnel->tunnel_id;

             sai_rc = sai_fib_ip_next_hop_create(&nh_node, &p_underlay_nh);
             if(sai_rc != SAI_STATUS_SUCCESS) {
                 SAI_TUNNEL_LOG_ERR("Failed to create an encap next hop for %s "
                                    "on tunnel 0x%"PRIx64"",sai_ip_addr_to_str(p_remote_ip,
                                     ip_addr_str, sizeof(ip_addr_str)),
                                     p_tunnel->tunnel_id);
                 break;
             }
          } else {
              sai_rc = SAI_STATUS_SUCCESS;
          }

         sai_fib_incr_nh_ref_count(p_underlay_nh);

     } while(0);

     sai_fib_unlock();

     return sai_rc;
}

sai_status_t sai_tunnel_encap_nh_teardown(sai_ip_address_t *p_remote_ip,
                                          dn_sai_tunnel_t *p_tunnel)
{
    sai_fib_nh_t *p_underlay_nh = NULL;
    char ip_addr_str [SAI_FIB_MAX_BUFSZ];
    sai_status_t sai_rc = SAI_STATUS_FAILURE;

    sai_fib_lock();

    do {

        p_underlay_nh = sai_fib_ip_next_hop_node_get(SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP,
                                                     p_tunnel->underlay_rif,
                                                     p_remote_ip, p_tunnel->tunnel_type);
        if(NULL == p_underlay_nh) {

            SAI_TUNNEL_LOG_ERR("Failed to get the encap next hop for %s "
                               "on tunnel 0x%"PRIx64"",sai_ip_addr_to_str(p_remote_ip,
                               ip_addr_str, sizeof(ip_addr_str)),
                               p_tunnel->tunnel_id);
            break;
        }

        sai_fib_decr_nh_ref_count(p_underlay_nh);
        sai_rc = SAI_STATUS_SUCCESS;

        if(p_underlay_nh->ref_count == 0) {
            sai_rc = sai_fib_ip_next_hop_remove(p_underlay_nh);

            if(sai_rc != SAI_STATUS_SUCCESS) {

                SAI_TUNNEL_LOG_ERR("Failed to remove the encap next hop for %s "
                                   "on tunnel 0x%"PRIx64"",sai_ip_addr_to_str(p_remote_ip,
                                   ip_addr_str, sizeof(ip_addr_str)),
                                   p_tunnel->tunnel_id);

                sai_fib_incr_nh_ref_count(p_underlay_nh);
                break;
            }
        }
    } while(0);

    sai_fib_unlock();

    return sai_rc;
}

sai_status_t sai_tunnel_underlay_setup(sai_ip_address_t *p_remote_ip,
                                       sai_object_id_t tunnel_oid)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_tunnel_t *p_tunnel = NULL;

    dn_sai_tunnel_lock();
    do {
        p_tunnel = dn_sai_tunnel_obj_get(tunnel_oid);

        if(NULL == p_tunnel) {
            SAI_TUNNEL_LOG_ERR("Tunnel oid %"PRIx64" not found", tunnel_oid);
            break;
        }

        sai_rc = sai_tunnel_encap_nh_setup(p_remote_ip, p_tunnel);

    } while(0);
    dn_sai_tunnel_unlock();

    return sai_rc;
}

sai_status_t sai_tunnel_underlay_teardown(sai_ip_address_t *p_remote_ip,
                                          sai_object_id_t tunnel_oid)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_tunnel_t *p_tunnel = NULL;

    dn_sai_tunnel_lock();
    do {
        p_tunnel = dn_sai_tunnel_obj_get(tunnel_oid);

        if(NULL == p_tunnel) {
            SAI_TUNNEL_LOG_ERR("Tunnel oid %"PRIx64" not found", tunnel_oid);
            break;
        }

        sai_rc = sai_tunnel_encap_nh_teardown(p_remote_ip, p_tunnel);

    } while(0);
    dn_sai_tunnel_unlock();

    return sai_rc;
}

