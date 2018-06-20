/************************************************************************
* LEGALESE:   "Copyright (c) 2017, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file sai_vm_fc_port.c
*
* @brief This file contains SAI-VM FC Port API functionality
*
*************************************************************************/

#include "saifcport.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_vm_event_log.h"

static sai_status_t sai_fc_port_set_attribute(sai_object_id_t port_id,
                                       const sai_attribute_t *attr)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    return ret;
}

static sai_status_t sai_fc_port_get_attribute(sai_object_id_t port_id,
                                       uint32_t attr_count,
                                       sai_attribute_t *attr_list)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    return ret;
}

static sai_status_t sai_fc_port_get_stats(sai_object_id_t port_obj_id,
                                   const sai_fc_port_counter_t *counter_ids,
                                   uint32_t number_of_counters,
                                   uint64_t* counters)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    return ret;
}

static sai_status_t sai_fc_port_clear_stats(sai_object_id_t port_obj_id,
                                     const sai_fc_port_counter_t *counter_ids,
                                     uint32_t number_of_counters)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    return ret;
}

static sai_status_t sai_fc_port_create(sai_object_id_t *port_id,
                                sai_object_id_t fc_switch_id,
                                uint32_t attr_count,
                                const sai_attribute_t *attr_list)

{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    return ret;

}

static sai_status_t sai_fc_port_remove(sai_object_id_t port_id)
{

    sai_status_t ret_code = SAI_STATUS_SUCCESS;


    return ret_code;
}

static sai_fc_port_api_t sai_fc_port_method_table =
{
    sai_fc_port_create,
    sai_fc_port_remove,
    sai_fc_port_set_attribute,
    sai_fc_port_get_attribute,
    sai_fc_port_get_stats,
    sai_fc_port_clear_stats,
};

sai_fc_port_api_t* sai_fc_port_api_query(void)
{
    SAI_VM_DB_LOG_INFO("Entered %s",__FUNCTION__);
    return (&sai_fc_port_method_table);
}
