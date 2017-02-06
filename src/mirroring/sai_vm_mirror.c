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

/**
 * @file sai_vm_mirror.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI MIRROR object in VM environment.
 */

#include "sai_vm_event_log.h"
#include "sai_npu_mirror.h"
#include "sai_mirror_defs.h"
#include "sai_oid_utils.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include <stddef.h>

static sai_status_t sai_npu_mirror_init (void)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_mirror_session_create (
sai_mirror_session_info_t *p_session_info, sai_npu_object_id_t *npu_object_id)
{
    STD_ASSERT (p_session_info != NULL);
    STD_ASSERT (npu_object_id != NULL);

    /*
     * @TODO Generate Mirror ID.
     */

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_mirror_session_destroy (sai_object_id_t session_id)
{
    /*
     * @TODO Free the Mirror ID that was generated upon session creation.
     */

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_mirror_session_port_add (sai_object_id_t session_id,
                                                     sai_object_id_t mirror_port,
                                                     sai_mirror_direction_t direction)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_mirror_session_port_remove (
sai_object_id_t session_id, sai_object_id_t mirror_port,
sai_mirror_direction_t direction)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_mirror_session_set (sai_object_id_t session_id,
                                                sai_mirror_type_t span_type,
                                                const sai_attribute_t *attr)
{
    STD_ASSERT (attr != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_mirror_session_get (sai_object_id_t session_id,
                                                sai_mirror_type_t span_type,
                                                uint32_t attr_count,
                                                sai_attribute_t *attr_list)
{
    STD_ASSERT (attr_list != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_mirror_session_validate_attributes (sai_object_id_t session_id,
                                                                const sai_attribute_t *attr_list,
                                                                uint32_t attr_index)
{
    STD_ASSERT (attr_list != NULL);

    return SAI_STATUS_SUCCESS;
}


static sai_npu_mirror_api_t sai_vm_mirror_api_table = {
    sai_npu_mirror_init,
    sai_npu_mirror_session_create,
    sai_npu_mirror_session_destroy,
    sai_npu_mirror_session_port_add,
    sai_npu_mirror_session_port_remove,
    sai_npu_mirror_session_set,
    sai_npu_mirror_session_get,
    sai_npu_mirror_session_validate_attributes
};

sai_npu_mirror_api_t* sai_vm_mirror_api_query (void)
{
    return &sai_vm_mirror_api_table;
}

