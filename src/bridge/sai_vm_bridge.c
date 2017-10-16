/************************************************************************
* * LEGALESE:   "Copyright (c) 2017, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/**
* @file sai_vm_bridge.c
*
* @brief This file contains Bridge implementation APIs for for BCM.
*************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "std_assert.h"
#include "saibridge.h"
#include "sai_bridge_api.h"
#include "sai_bridge_npu_api.h"
#include "sai_bridge_common.h"
#include "sai_port_utils.h"
#include "sai_lag_api.h"

static dn_sai_id_gen_info_t bridge_obj_gen_info;
static dn_sai_id_gen_info_t bridge_port_obj_gen_info;

static bool sai_vm_is_bridge_port_id_in_use(uint64_t obj_id)
{
    sai_object_id_t bridge_port_id =  sai_uoid_create(SAI_OBJECT_TYPE_BRIDGE_PORT, obj_id);
    return sai_is_bridge_port_created(bridge_port_id);
}

static bool sai_vm_is_bridge_id_in_use(uint64_t obj_id)
{
    sai_object_id_t bridge_id = sai_uoid_create(SAI_OBJECT_TYPE_BRIDGE, obj_id);
    return sai_is_bridge_created(bridge_id);
}

static sai_object_id_t sai_vm_bridge_port_id_create(void)
{
    if(SAI_STATUS_SUCCESS == dn_sai_get_next_free_id(&bridge_port_obj_gen_info)) {
        return (sai_uoid_create(SAI_OBJECT_TYPE_BRIDGE_PORT, bridge_port_obj_gen_info.cur_id));
    }
    return SAI_NULL_OBJECT_ID;
}

static sai_object_id_t sai_vm_bridge_id_create(void)
{
    if(SAI_STATUS_SUCCESS == dn_sai_get_next_free_id(&bridge_obj_gen_info)) {
        return (sai_uoid_create(SAI_OBJECT_TYPE_BRIDGE, bridge_obj_gen_info.cur_id));
    }
    return SAI_NULL_OBJECT_ID;
}

static sai_status_t sai_vm_bridge_init(bool init)
{
    if(init) {
        bridge_port_obj_gen_info.cur_id = 0;
        bridge_port_obj_gen_info.is_wrappped = false;
        bridge_port_obj_gen_info.mask = SAI_UOID_NPU_OBJ_ID_MASK;
        bridge_port_obj_gen_info.is_id_in_use = sai_vm_is_bridge_port_id_in_use;

        bridge_obj_gen_info.cur_id = 0;
        bridge_obj_gen_info.is_wrappped = false;
        bridge_obj_gen_info.mask = SAI_UOID_NPU_OBJ_ID_MASK;
        bridge_obj_gen_info.is_id_in_use = sai_vm_is_bridge_id_in_use;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_create(sai_object_id_t      *bridge_id,
                                          dn_sai_bridge_info_t *bridge_info)
{

    if ((bridge_id == NULL) || (bridge_info == NULL)) {
        SAI_BRIDGE_LOG_TRACE("bridge id is %p bridge info is %p in bcm bridge create",
                             bridge_id, bridge_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *bridge_id = sai_vm_bridge_id_create();
    bridge_info->hw_info = NULL;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_remove(dn_sai_bridge_info_t *bridge_info)
{
    if(bridge_info == NULL) {
        SAI_BRIDGE_LOG_TRACE("bridge info is NULL in bcm bridge remove");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(bridge_info->bridge_type == SAI_BRIDGE_TYPE_1Q){
        return SAI_STATUS_NOT_SUPPORTED;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_set_attribute (const dn_sai_bridge_info_t *bridge_info,
                                                 const sai_attribute_t *attr)
{
    if((bridge_info == NULL) || (attr == NULL)){
        SAI_BRIDGE_LOG_TRACE("bridge info is %p attr is %p in bcm bridge set attr",
                             bridge_info, attr);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_port_create(sai_object_id_t *bridge_port_id,
                                              dn_sai_bridge_port_info_t *bridge_port_info)
{
    if((bridge_port_id == NULL) || (bridge_port_info == NULL)) {
        SAI_BRIDGE_LOG_TRACE("bridge port id is %p bridge port info is %p in bcm bridge port create",
                             bridge_port_id, bridge_port_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *bridge_port_id =  sai_vm_bridge_port_id_create();

    return SAI_STATUS_SUCCESS;
}


static sai_status_t sai_vm_bridge_port_remove(dn_sai_bridge_port_info_t *bridge_port_info)
{
    if(bridge_port_info == NULL) {
        SAI_BRIDGE_LOG_TRACE("bridge port info is NULL in bridge port remove");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Nothing to be done*/
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_port_set_attribute (const dn_sai_bridge_port_info_t
                                                      *bridge_port_info,
                                                      const sai_attribute_t *attr)
{
    if((bridge_port_info == NULL) || (attr == NULL)){
        SAI_BRIDGE_LOG_TRACE("bridge port info is %p and attr is %p in bridge port set attr",
                             bridge_port_info, attr);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_bridge_port_lag_handler(dn_sai_bridge_port_info_t
                                                   *bridge_port_info,
                                                   sai_object_id_t lag_id, bool add_ports,
                                                   const sai_object_list_t *port_list)
{
    if((bridge_port_info == NULL) || (port_list == NULL)) {
        SAI_BRIDGE_LOG_TRACE("bridge port info is %p port_list is %p in sub port attribute set",
                             bridge_port_info, port_list);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

static void sai_vm_bridge_dump_bridge_hw_info(const dn_sai_bridge_info_t *p_bridge_info)
{
}

static void sai_vm_bridge_dump_bridge_port_info(const dn_sai_bridge_port_info_t *bridge_port_info)
{
}

static sai_npu_bridge_api_t sai_vm_bridge_api_table = {
    sai_vm_bridge_init,
    sai_vm_bridge_create,
    sai_vm_bridge_remove,
    sai_vm_bridge_set_attribute,
    sai_vm_bridge_port_create,
    sai_vm_bridge_port_remove,
    sai_vm_bridge_port_set_attribute,
    sai_vm_bridge_port_lag_handler,
    sai_vm_bridge_dump_bridge_hw_info,
    sai_vm_bridge_dump_bridge_port_info
};

sai_npu_bridge_api_t* sai_vm_bridge_api_query (void)
{
    return &sai_vm_bridge_api_table;
}
