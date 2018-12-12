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
* @file sai_tunnel_api_utils.h
*
* @brief This private header file contains SAI Tunnel util functions
*        data structure definitions
*
****************************************************************************/
#ifndef _SAI_TUNNEL_API_UTILS_H_
#define _SAI_TUNNEL_API_UTILS_H_

#include "saitunnel.h"
#include "saitypes.h"
#include "sai_common_utils.h"
#include "sai_tunnel_util.h"
#include "sai_l3_common.h"

/* Function declarations*/
void dn_sai_tunnel_map_init(void);

void dn_sai_tunnel_map_entry_init(void);

void dn_sai_tunnel_term_obj_api_fill (sai_tunnel_api_t *api_table);

void dn_sai_tunnel_map_obj_api_fill (sai_tunnel_api_t *api_table);

void dn_sai_tunnel_map_entry_obj_api_fill (sai_tunnel_api_t *api_table);

sai_status_t dn_sai_tunnel_attr_list_validate (sai_object_type_t obj_type,
                                               uint32_t attr_count,
                                               const sai_attribute_t *attr_list,
                                               dn_sai_operations_t op_type);

sai_status_t sai_tunnel_encap_nh_setup(sai_ip_address_t *p_remote_ip,
                                       dn_sai_tunnel_t *p_tunnel);

sai_status_t sai_tunnel_encap_nh_teardown(sai_ip_address_t *p_remote_ip,
                                          dn_sai_tunnel_t *p_tunnel);

/* Accessor functions for Tunnel API global info */
static inline uint8_t *dn_sai_tunnel_obj_id_bitmap (void)
{
    return (dn_sai_tunnel_access_global_config()->tunnel_obj_id_bitmap);
}

static inline uint8_t *dn_sai_tunnel_term_obj_id_bitmap (void)
{
    return (dn_sai_tunnel_access_global_config()->tunnel_term_id_bitmap);
}

#endif /* _SAI_TUNNEL_API_UTILS_H_ */
