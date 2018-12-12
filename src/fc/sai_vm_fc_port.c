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
