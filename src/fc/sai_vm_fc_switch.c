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
* @file sai_vm_fc_switch.c
*
* @brief This file contains SAI-VM FC Switch API functionality
*
*************************************************************************/

#include "saifcport.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saifcswitch.h"
#include "sai_vm_event_log.h"

sai_status_t sai_set_fc_switch_attribute(
        sai_object_id_t fc_switch_id,
        const sai_attribute_t *attr)
{
    int ret = SAI_STATUS_SUCCESS;

    return ret;
}

static sai_status_t sai_get_fc_switch_attribute(sai_object_id_t fc_switch_id,
                                                sai_uint32_t attr_count,
                                                sai_attribute_t *attr_list)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    /** @TODO handle port event notification */
    return ret;
}

static sai_status_t sai_create_fc_switch (sai_object_id_t* fc_switch_id,
                                   sai_object_id_t switch_id,
                                   uint32_t attr_count,
                                   const sai_attribute_t *attr_list)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    return ret;

}


static sai_status_t sai_remove_fc_switch(sai_object_id_t fc_switch_id)

{
    return SAI_STATUS_SUCCESS;
}


static sai_fc_switch_api_t  sai_fc_switch_method_table = {
    sai_create_fc_switch,
    sai_remove_fc_switch,
    sai_set_fc_switch_attribute,
    sai_get_fc_switch_attribute,
};

sai_fc_switch_api_t *sai_fc_switch_api_query()
{
    SAI_VM_DB_LOG_INFO("Entered %s",__FUNCTION__);

    return (&sai_fc_switch_method_table);
}

