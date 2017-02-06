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
 * @file sai_vm_stp.c
 *
 * @brief  This file contains function implementations of NPU handlers
 *         for SAI STP object in VM environment.
 */

#include "sai_npu_stp.h"
#include "sai_vm_event_log.h"
#include "sai_switching_db_api.h"
#include "sai_oid_utils.h"
#include "sai_vlan_api.h"
#include "sai_vlan_common.h"
#include "sai_stp_util.h"
#include "sai_vm_defs.h"
#include "saistp.h"
#include "saivlan.h"
#include "saitypes.h"
#include "saistatus.h"

#include "std_assert.h"

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>


static bool stp_id_in_use[SAI_VM_SWITCH_MAX_STP_INSTANCES];
static int  default_stp_id = 0;

static sai_status_t sai_npu_stp_instance_create (sai_npu_object_id_t *p_stp_id)
{
    uint_t stp_idx;
    STD_ASSERT (p_stp_id != NULL);

    SAI_STP_LOG_TRACE ("STP Instance Creation.");

    for(stp_idx = 0; stp_idx < SAI_VM_SWITCH_MAX_STP_INSTANCES; stp_idx++) {
        if(!stp_id_in_use[stp_idx]) {
            stp_id_in_use[stp_idx] = true;
            *p_stp_id = stp_idx;
            return SAI_STATUS_SUCCESS;
        }
    }

    SAI_STP_LOG_ERR("STP Instance unavailable");
    return SAI_STATUS_INSUFFICIENT_RESOURCES;
}

static sai_status_t sai_npu_stp_instance_remove (sai_object_id_t stp_inst_id)
{
    uint_t stp_idx = (uint_t)sai_uoid_npu_obj_id_get(stp_inst_id);
    SAI_STP_LOG_TRACE ("STP Instance Remove, ID: 0x%"PRIx64".", stp_inst_id);

    if(stp_idx < SAI_VM_SWITCH_MAX_STP_INSTANCES) {
        if(stp_id_in_use[stp_idx]) {
            stp_id_in_use[stp_idx] = false;
            return SAI_STATUS_SUCCESS;
        } else {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
    }

    return SAI_STATUS_INVALID_OBJECT_ID;
}

static sai_status_t sai_npu_stp_default_instance_get (sai_npu_object_id_t *p_default_stp_instance)
{
    STD_ASSERT (p_default_stp_instance != NULL);
    *p_default_stp_instance = default_stp_id;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_stp_init(sai_npu_object_id_t *p_default_stp_instance,
                                     sai_npu_object_id_t *p_l3_stp_instance)
{

    STD_ASSERT (p_default_stp_instance != NULL);
    STD_ASSERT (p_l3_stp_instance != NULL);
    sai_npu_stp_instance_create(p_default_stp_instance);
    default_stp_id = *p_default_stp_instance;
    sai_npu_stp_instance_create(p_l3_stp_instance);
    return SAI_STATUS_SUCCESS;
}
static sai_status_t sai_npu_stp_port_state_set (sai_object_id_t stp_inst_id,
                                                sai_object_id_t port_id,
                                                sai_port_stp_state_t port_state)
{

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_stp_port_state_get (sai_object_id_t stp_inst_id,
                                                sai_object_id_t port_id,
                                                sai_port_stp_state_t *port_state)
{
    STD_ASSERT (port_state != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_stp_vlan_add (sai_object_id_t stp_inst_id,
                                          sai_vlan_id_t vlan_id)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    if (!sai_is_valid_vlan_id (vlan_id)) {
        SAI_VLAN_LOG_ERR ("vlan ID %d is not valid.", vlan_id);

        return SAI_STATUS_INVALID_VLAN_ID;
    }

    /* Update VLAN record in DB with STP instance info. */
    attr.id = SAI_VLAN_ATTR_STP_INSTANCE;
    attr.value.oid = stp_inst_id;

    sai_rc = sai_vlan_set_db_entry (vlan_id, &attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR ("Error updating VLAN entry in DB with STP instance"
                             " object ID: 0x%"PRIx64" for vlan ID: %d.",
                             stp_inst_id, vlan_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_stp_vlan_remove (sai_object_id_t stp_inst_id,
                                             sai_vlan_id_t vlan_id)
{

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_stp_attribute_get (sai_object_id_t stp_inst_id,
                                               uint32_t attr_count,
                                               sai_attribute_t *attr_list)
{
    STD_ASSERT (attr_list != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_vlan_stp_get (sai_vlan_id_t vlan_id,
                                          sai_npu_object_id_t *p_stp_id)
{
    STD_ASSERT (p_stp_id != NULL);


    return SAI_STATUS_SUCCESS;
}

static sai_npu_stp_api_t sai_vm_stp_api_table = {
    sai_npu_stp_init,
    sai_npu_stp_default_instance_get,
    sai_npu_stp_instance_create,
    sai_npu_stp_instance_remove,
    sai_npu_stp_vlan_add,
    sai_npu_stp_vlan_remove,
    sai_npu_stp_attribute_get,
    sai_npu_stp_port_state_set,
    sai_npu_stp_port_state_get,
    sai_npu_vlan_stp_get,
};

sai_npu_stp_api_t* sai_vm_stp_api_query (void)
{
    return &sai_vm_stp_api_table;
}


