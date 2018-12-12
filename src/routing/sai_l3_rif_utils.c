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
* @file sai_l3_rif_utils.c
*
* @brief This file contains function definitions for SAI router interface
*        functionality utility API implementation.
*
*************************************************************************/

#include "sai_map_utl.h"
#include "saitypes.h"
#include "saistatus.h"
#include <stdio.h>
#include <string.h>

sai_status_t sai_fib_lag_rif_mapping_insert (sai_object_id_t lag_id,
                                             sai_object_id_t rif_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_LAG_RIF_INFO;
    key.id1  = lag_id;
    value.count = 1;
    value.data  = &data;

    data.val1 = rif_id;

    return sai_map_insert (&key, &value);
}

sai_status_t sai_fib_lag_rif_mapping_remove (sai_object_id_t lag_id,
                                             sai_object_id_t rif_id)
{
    sai_map_key_t  key;

    memset (&key, 0, sizeof (key));
    key.type = SAI_MAP_TYPE_LAG_RIF_INFO;
    key.id1 = lag_id;

    return sai_map_delete (&key);
}


sai_status_t sai_fib_get_rif_id_from_lag_id (sai_object_id_t lag_id,
                                             sai_object_id_t *rif_id)
{
    sai_map_key_t  key;
    sai_map_val_t  value;
    sai_map_data_t data;
    sai_status_t   rc;

    memset (&key, 0, sizeof (key));
    memset (&value, 0, sizeof (value));
    memset (&data, 0, sizeof (data));

    key.type = SAI_MAP_TYPE_LAG_RIF_INFO;
    key.id1  = lag_id;

    value.count = 1;
    value.data  = &data;

    rc = sai_map_get (&key, &value);

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }

    *rif_id = data.val1;

    return SAI_STATUS_SUCCESS;
}
