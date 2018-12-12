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
* @file sai_l3_next_hop_group_utl.c
*
* @brief This file contains utility routines for SAI Next Hop Group
*        functionality.
*
*************************************************************************/

#include "saistatus.h"
#include "saitypes.h"
#include "std_assert.h"
#include "sai_map_utl.h"
#include "sai_l3_next_hop_group_utl.h"

#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

sai_status_t sai_next_hop_map_insert (sai_object_id_t nh_grp_id,
                                      sai_object_id_t nh_id,
                                      sai_object_id_t member_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    sai_status_t   rc;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    /* Insert the memberId in the nhGrp to memberId list */
    key.type = SAI_MAP_TYPE_NH_GRP_2_MEMBER_LIST;
    key.id1  = nh_grp_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = member_id;

    rc = sai_map_insert (&key, &value);

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    /* Create the memberId --> {nhGrpId, nhId} mapping */
    key.type = SAI_MAP_TYPE_NH_MEMBER_2_GRP_INFO;
    key.id1  = member_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = nh_grp_id;
    data.val2 = nh_id;

    rc = sai_map_insert (&key, &value);

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }

    return rc;
}

sai_status_t sai_next_hop_map_remove (sai_object_id_t nh_grp_id,
                                      sai_object_id_t nh_id,
                                      sai_object_id_t member_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    uint32_t       count = 0;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    /* Remove the memberId from the nhGrp to memberId list */
    key.type = SAI_MAP_TYPE_NH_GRP_2_MEMBER_LIST;
    key.id1  = nh_grp_id;

    value.count = 1;
    value.data  = &data;

    data.val1 = member_id;

    sai_map_delete_elements (&key, &value, SAI_MAP_VAL_FILTER_VAL1);

    if (sai_map_get_val_count (&key, &count) == SAI_STATUS_SUCCESS) {
        if (count == 0) {
            sai_map_delete (&key);
        }
    }

    memset (&key, 0, sizeof (key));

    /* Create the memberId --> {nhGrpId, nhId} mapping */
    key.type = SAI_MAP_TYPE_NH_MEMBER_2_GRP_INFO;
    key.id1  = member_id;

    sai_map_delete (&key);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_next_hop_map_get_ids_from_member_id (sai_object_id_t member_id,
                                                      sai_object_id_t *out_nh_grp_id,
                                                      sai_object_id_t *out_nh_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    sai_status_t   rc;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_NH_MEMBER_2_GRP_INFO;
    key.id1  = member_id;

    value.count = 1;
    value.data  = &data;

    rc = sai_map_get (&key, &value);

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }

    *out_nh_grp_id = data.val1;
    *out_nh_id     = data.val2;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_next_hop_map_get_member_list (sai_object_id_t  nh_grp_id,
                                               uint32_t        *in_out_nh_count,
                                               sai_object_id_t *out_member_list)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data [*in_out_nh_count];
    sai_status_t   rc;
    uint32_t       index;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data [0], 0, (*in_out_nh_count) * sizeof (sai_map_data_t));

    key.type = SAI_MAP_TYPE_NH_GRP_2_MEMBER_LIST;
    key.id1  = nh_grp_id;

    value.count = *in_out_nh_count;
    value.data  = data;

    rc = sai_map_get (&key, &value);

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }

    if (value.count > (*in_out_nh_count)) {
        return SAI_STATUS_FAILURE;
    }

    *in_out_nh_count = value.count;

    for (index = 0; index < value.count; index++) {
        out_member_list [index] = data [index].val1;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_next_hop_map_get_count (sai_object_id_t  nh_grp_id,
                                         uint32_t        *p_out_count)
{
    sai_map_key_t  key;
    sai_status_t   rc;

    memset (&key, 0, sizeof (key));

    key.type = SAI_MAP_TYPE_NH_GRP_2_MEMBER_LIST;
    key.id1  = nh_grp_id;

    rc = sai_map_get_val_count (&key, p_out_count);

    return rc;
}

