/************************************************************************
 * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
 *
 * This source code is confidential, proprietary, and contains trade
 * secrets that are the sole property of Dell Inc.
 * Copy and/or distribution of this source code or disassembly or reverse
 * engineering of the resultant object code are strictly forbidden without
 * the written consent of Dell Inc.
 *
 ************************************************************************/
/*
 * @file sai_vm_mirror.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI MIRROR object in VM environment.
 *************************************************************************/
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

