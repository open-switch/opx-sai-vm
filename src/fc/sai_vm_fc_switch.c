
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
