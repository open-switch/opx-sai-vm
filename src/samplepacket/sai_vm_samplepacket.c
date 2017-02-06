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
 * @file sai_vm_samplepacket.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI SamplePacket object in VM environment.
 */

#include "sai_npu_samplepacket.h"

#include "saitypes.h"
#include "saistatus.h"
#include "std_assert.h"

static sai_status_t sai_npu_samplepacket_init (void)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_samplepacket_session_create (dn_sai_samplepacket_session_info_t *p_session_info,
                                                         sai_npu_object_id_t *npu_object_id)
{
    STD_ASSERT (npu_object_id != NULL);
    STD_ASSERT (p_session_info != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_samplepacket_session_destroy (sai_object_id_t session_id)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_samplepacket_session_port_add (dn_sai_samplepacket_session_info_t *p_session_info,
                                                           sai_object_id_t samplepacket_port,
                                                           sai_samplepacket_direction_t direction)
{
    STD_ASSERT (p_session_info != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_samplepacket_session_port_remove (sai_object_id_t session_id,
                                                              sai_object_id_t samplepacket_port,
                                                              sai_samplepacket_direction_t direction)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_samplepacket_session_set (dn_sai_samplepacket_session_info_t *p_session_info,
                                                      const sai_attribute_t *attr)
{
    STD_ASSERT (attr != NULL);
    STD_ASSERT (p_session_info != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_samplepacket_session_get (dn_sai_samplepacket_session_info_t *p_session_info,
                                                      uint32_t attr_count,
                                                      sai_attribute_t *attr_list)
{
    STD_ASSERT (p_session_info != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_samplepacket_session_acl_port_add (dn_sai_samplepacket_session_info_t *p_session_info,
                                                               sai_object_id_t samplepacket_port,
                                                               sai_samplepacket_direction_t direction)
{
    STD_ASSERT (p_session_info != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_samplepacket_session_acl_port_remove (sai_object_id_t session_id,
                                                                  sai_object_id_t samplepacket_port,
                                                                  sai_samplepacket_direction_t direction)
{
    return SAI_STATUS_SUCCESS;
}

static sai_npu_samplepacket_api_t sai_vm_samplepacket_api_table = {
    sai_npu_samplepacket_init,
    sai_npu_samplepacket_session_create,
    sai_npu_samplepacket_session_destroy,
    sai_npu_samplepacket_session_port_add,
    sai_npu_samplepacket_session_port_remove,
    sai_npu_samplepacket_session_set,
    sai_npu_samplepacket_session_get,
    sai_npu_samplepacket_session_acl_port_add,
    sai_npu_samplepacket_session_acl_port_remove
};

sai_npu_samplepacket_api_t* sai_vm_samplepacket_api_query (void)
{
    return &sai_vm_samplepacket_api_table;
}

