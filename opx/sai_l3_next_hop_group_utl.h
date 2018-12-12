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
* @file sai_l3_next_hop_group_utl.h
*
* @brief This file contains header file definitions for SAI Next Hop Group
*        utility functionality.
*
*************************************************************************/

#include "saistatus.h"
#include "saitypes.h"
#include "std_assert.h"
#include "sai_map_utl.h"

sai_status_t sai_next_hop_map_insert (sai_object_id_t nh_grp_id,
                                      sai_object_id_t nh_id,
                                      sai_object_id_t member_id);

sai_status_t sai_next_hop_map_remove (sai_object_id_t nh_grp_id,
                                      sai_object_id_t nh_id,
                                      sai_object_id_t member_id);

sai_status_t sai_next_hop_map_get_ids_from_member_id (sai_object_id_t member_id,
                                                      sai_object_id_t *out_nh_grp_id,
                                                      sai_object_id_t *out_nh_id);

sai_status_t sai_next_hop_map_get_member_list (sai_object_id_t  nh_grp_id,
                                               uint32_t        *in_out_nh_count,
                                               sai_object_id_t *out_member_list);

sai_status_t sai_next_hop_map_get_count (sai_object_id_t  nh_grp_id,
                                         uint32_t        *p_out_count);

void sai_fib_next_hop_grp_member_gen_info_init(void);
