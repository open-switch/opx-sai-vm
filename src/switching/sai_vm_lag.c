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
 * @file sai_vm_lag.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI LAG object in VM environment.
 *************************************************************************/
#include "sailag.h"
#include "sai_npu_lag.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_assert.h"
#include "sai_lag_common.h"
#include <stddef.h>
#include <inttypes.h>
#include "sai_oid_utils.h"
#include "sai_vm_defs.h"

static bool lag_id_in_use[SAI_VM_SWITCH_MAX_LAG_NUMBER];

static void sai_npu_lag_init (void)
{
}

static sai_status_t sai_npu_lag_create (sai_object_id_t *lag_id)
{
    uint_t lag_idx;
    SAI_LAG_LOG_TRACE ("LAG Creation.");

    STD_ASSERT (lag_id != NULL);
    for(lag_idx = 0; lag_idx < SAI_VM_SWITCH_MAX_LAG_NUMBER; lag_idx++) {
        if(!lag_id_in_use[lag_idx]) {
            lag_id_in_use[lag_idx] = true;
            *lag_id = sai_uoid_create(SAI_OBJECT_TYPE_LAG,lag_idx);
            return SAI_STATUS_SUCCESS;
        }
    }

    SAI_LAG_LOG_ERR("LAG Index unavailable");
    return SAI_STATUS_INSUFFICIENT_RESOURCES;
}

static sai_status_t sai_npu_lag_remove (sai_object_id_t lag_id)
{
    uint_t lag_idx = (uint_t)sai_uoid_npu_obj_id_get(lag_id);
    SAI_LAG_LOG_TRACE ("LAG Remove, ID: 0x%"PRIx64".", lag_id);

    if(lag_idx < SAI_VM_SWITCH_MAX_LAG_NUMBER) {
        if(lag_id_in_use[lag_idx]) {
            lag_id_in_use[lag_idx] = false;
            return SAI_STATUS_SUCCESS;
        } else {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
    }
    return SAI_STATUS_INVALID_OBJECT_ID;
}

static sai_status_t sai_npu_add_ports_to_lag (sai_object_id_t lag_id,
                                              const sai_object_list_t *lag_port_list,
                                              sai_object_list_t *lag_member_id_list)
{
    uint_t index;
    SAI_LAG_LOG_TRACE ("LAG Add ports, ID: 0x%"PRIx64".", lag_id);

    STD_ASSERT (lag_port_list != NULL);
    STD_ASSERT (lag_member_id_list != NULL);
    STD_ASSERT (lag_port_list->count <= lag_member_id_list->count);

    for (index = 0; index < lag_port_list->count; index++) {
        lag_member_id_list->list [index]
            = sai_uoid_create (SAI_OBJECT_TYPE_LAG_MEMBER,
                               sai_uoid_npu_obj_id_get (lag_port_list->list[index]));
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_remove_ports_from_lag (
sai_object_id_t lag_id, const sai_object_list_t *lag_port_list)
{
    SAI_LAG_LOG_TRACE ("LAG Remove ports, ID: 0x%"PRIx64".", lag_id);

    STD_ASSERT (lag_port_list != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_lag_port_flag_set (sai_object_id_t lag_id,
                                               sai_object_id_t port_id,
                                               bool            is_ingress,
                                               bool            value)
{
    SAI_LAG_LOG_TRACE ("LAG port flag set, LAG ID: 0x%"PRIx64" "
                       "Port ID: 0x%"PRIx64" Direction: %s Value: %s.",
                       lag_id, port_id, (is_ingress) ? "Ingress" : "Egress",
                       (value) ? "true" : "false");

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_lag_port_flag_get (sai_object_id_t  lag_id,
                                               sai_object_id_t  port_id,
                                               bool             is_ingress,
                                               bool            *value)
{
    SAI_LAG_LOG_TRACE ("LAG port flag get, LAG ID: 0x%"PRIx64" "
                       "Port ID: 0x%"PRIx64" Direction: %s.",
                       lag_id, port_id, (is_ingress) ? "Ingress" : "Egress");

    return SAI_STATUS_SUCCESS;
}

static sai_npu_lag_api_t sai_vm_lag_api_table = {
    sai_npu_lag_init,
    sai_npu_lag_create,
    sai_npu_lag_remove,
    sai_npu_add_ports_to_lag,
    sai_npu_remove_ports_from_lag,
    sai_npu_lag_port_flag_set,
    sai_npu_lag_port_flag_get,
};

sai_npu_lag_api_t* sai_vm_lag_api_query (void)
{
    return &sai_vm_lag_api_table;
}

