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
 * @file sai_vm_samplepacket.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI SamplePacket object in VM environment.
 *************************************************************************/

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

