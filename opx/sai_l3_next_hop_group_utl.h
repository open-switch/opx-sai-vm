
/************************************************************************
* LEGALESE:   "Copyright (c) 2016, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
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
