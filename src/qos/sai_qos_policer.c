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
/**
* @file  sai_qos_policer.c
*
* @brief This file contains function definitions for SAI QoS Policer
*        functionality API implementation.
*
*************************************************************************/
#include "sai.h"
#include "saipolicer.h"
#include "sai_npu_qos.h"
#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_api_utils.h"
#include "sai_gen_utils.h"
#include "sai_qos_mem.h"
#include "sai_common_infra.h"

#include "std_type_defs.h"
#include "std_utils.h"
#include "std_assert.h"

#include <stdlib.h>
#include <inttypes.h>

static uint_t sai_get_storm_control_type_from_port_attr(uint_t attr)
{
    if(attr == SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID){
        return SAI_QOS_POLICER_TYPE_STORM_FLOOD;
    }else if(attr == SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID){
        return SAI_QOS_POLICER_TYPE_STORM_BCAST;
    }else if(attr == SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID){
        return SAI_QOS_POLICER_TYPE_STORM_MCAST;
    }

    return SAI_QOS_POLICER_TYPE_INVALID;
}

sai_status_t sai_get_port_attr_from_storm_control_type(dn_sai_qos_policer_type_t type,
                                                       sai_port_attr_t *attr_id)
{
    STD_ASSERT (attr_id != NULL);

    switch(type)
    {
        case SAI_QOS_POLICER_TYPE_STORM_FLOOD:
            *attr_id = SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID;
            break;

        case SAI_QOS_POLICER_TYPE_STORM_BCAST:
            *attr_id = SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID;
            break;

        case SAI_QOS_POLICER_TYPE_STORM_MCAST:
            *attr_id = SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID;
            break;

        default:
            return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}
void sai_qos_policer_free_resources(dn_sai_qos_policer_t *p_policer_node)
{
    SAI_POLICER_LOG_TRACE("Freeing the policer node");
    sai_qos_policer_node_free(p_policer_node);
}

static sai_status_t sai_qos_policer_validate_action(uint_t action)
{
    if((action == SAI_PACKET_ACTION_DROP) || (action == SAI_PACKET_ACTION_FORWARD)){
        return SAI_STATUS_SUCCESS;
    }
    return SAI_STATUS_INVALID_ATTR_VALUE_0;
}

static sai_status_t sai_policer_exist_node_actions_update(dn_sai_qos_policer_t *p_policer_node,
                                                          const sai_attribute_t *p_attr)
{
    STD_ASSERT(p_attr != NULL);
    STD_ASSERT(p_policer_node != NULL);

    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    size_t list_idx = 0;
    bool   is_found = false;

    SAI_POLICER_LOG_TRACE("Update policer actions in old node");
    sai_rc = sai_qos_policer_validate_action(p_attr->value.s32);

    if(sai_rc == SAI_STATUS_SUCCESS){
        for(list_idx = 0; list_idx < p_policer_node->action_count; list_idx ++){

            if(p_policer_node->action_list[list_idx].action == p_attr->id){
                is_found = true;
                p_policer_node->action_list[list_idx].value = p_attr->value.s32;
                p_policer_node->action_list[list_idx].enable = true;
                break;
            }
        }
        if(!is_found){
            SAI_POLICER_LOG_TRACE("Action is not found adding it");
            p_policer_node->action_list[p_policer_node->action_count].action = p_attr->id;
            p_policer_node->action_list[p_policer_node->action_count].value = p_attr->value.s32;
            p_policer_node->action_list[p_policer_node->action_count].enable = true;
            p_policer_node->action_count ++;
        }
    }
    for(list_idx = 0; list_idx < p_policer_node->action_count; list_idx ++){
        SAI_POLICER_LOG_INFO("Action id: %d value: %d enable %d",
                              p_policer_node->action_list[list_idx].action,
                              p_policer_node->action_list[list_idx].value,
                              p_policer_node->action_list[list_idx].enable);

    }

    return sai_rc;
}

static sai_status_t sai_policer_action_populate(dn_sai_qos_policer_t *p_policer_node,
                                                const sai_attribute_t *p_attr)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    size_t list_idx = 0;

    STD_ASSERT(p_attr != NULL);
    STD_ASSERT(p_policer_node != NULL);

    SAI_POLICER_LOG_TRACE("Policer action populate in new node ");
    sai_rc = sai_qos_policer_validate_action(p_attr->value.s32);

    if(sai_rc == SAI_STATUS_SUCCESS){
        p_policer_node->action_list[p_policer_node->action_count].action = p_attr->id;
        p_policer_node->action_list[p_policer_node->action_count].value =
            p_attr->value.s32;

        p_policer_node->action_list[p_policer_node->action_count].enable = true;

        p_policer_node->action_count ++;
        for(list_idx = 0; list_idx < p_policer_node->action_count; list_idx ++){
            SAI_POLICER_LOG_INFO("Action id: %d value: %d enable %d",
                                  p_policer_node->action_list[list_idx].action,
                                  p_policer_node->action_list[list_idx].value,
                                  p_policer_node->action_list[list_idx].enable);

        }
    }
    return sai_rc;
}

static sai_status_t sai_policer_attr_set(dn_sai_qos_policer_t *p_policer_node,
                                  const sai_attribute_t *p_attr,
                                  bool old_node_update)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_policer_node != NULL);

    STD_ASSERT(p_attr != NULL);

    SAI_POLICER_LOG_TRACE ("Setting Policer attribute id: %d.",
                        p_attr->id);

    switch (p_attr->id)
    {
        case SAI_POLICER_ATTR_METER_TYPE:
            p_policer_node->meter_type = p_attr->value.s32;
            break;

        case SAI_POLICER_ATTR_MODE:
            p_policer_node->policer_mode = p_attr->value.s32;
            break;

        case SAI_POLICER_ATTR_COLOR_SOURCE:
            p_policer_node->color_source = p_attr->value.s32;
            break;

        case SAI_POLICER_ATTR_CBS:
            p_policer_node->cbs = p_attr->value.u64;
            break;

        case SAI_POLICER_ATTR_CIR:
            p_policer_node->cir = p_attr->value.u64;
            break;

        case SAI_POLICER_ATTR_PBS:
            p_policer_node->pbs = p_attr->value.u64;
            break;

        case SAI_POLICER_ATTR_PIR:
            p_policer_node->pir = p_attr->value.u64;
            break;

        case SAI_POLICER_ATTR_GREEN_PACKET_ACTION:
        case SAI_POLICER_ATTR_YELLOW_PACKET_ACTION:
        case SAI_POLICER_ATTR_RED_PACKET_ACTION:
            if(!old_node_update){
                sai_rc = sai_policer_action_populate(p_policer_node, p_attr);
            }
            else {
                sai_rc = sai_policer_exist_node_actions_update(p_policer_node, p_attr);
            }
            break;

        case SAI_POLICER_ATTR_ENABLE_COUNTER_PACKET_ACTION_LIST:
            return SAI_STATUS_NOT_SUPPORTED;
            break;

        default:
            SAI_POLICER_LOG_ERR ("Attribute id: %d is not a known attribute "
                                 "for Policer.", p_attr->id);

            return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            break;
    }

    return sai_rc;
}
static uint_t sai_policer_storm_control_attr_bits()
{
    uint_t  storm_control_attr_bits = 0;

    storm_control_attr_bits |= sai_attribute_bit_set(SAI_POLICER_ATTR_METER_TYPE) |
     sai_attribute_bit_set(SAI_POLICER_ATTR_MODE) | sai_attribute_bit_set(SAI_POLICER_ATTR_PIR);

    SAI_POLICER_LOG_TRACE("storm control attr bits is %d",storm_control_attr_bits);
    return storm_control_attr_bits;
}

static uint_t sai_policer_trtcm_mandatory_attr()
{
    uint_t trtcm_attr = 0;
    return (trtcm_attr | sai_attribute_bit_set(SAI_POLICER_ATTR_PIR));
}
static sai_status_t sai_policer_parse_update_attributes(
                                          dn_sai_qos_policer_t *p_policer_node,
                                          uint32_t attr_count,
                                          const sai_attribute_t *attr_list,
                                          dn_sai_operations_t op_type,
                                          bool old_node_update)
{
    sai_status_t               sai_rc = SAI_STATUS_SUCCESS;
    uint_t                     list_idx = 0;
    uint_t                     max_vendor_attr_count = 0;
    uint_t                     attr_bits = 0;
    const dn_sai_attribute_entry_t   *p_vendor_attr = NULL;

    sai_qos_policer_npu_api_get()->attribute_table_get
        (&p_vendor_attr, &max_vendor_attr_count);

    STD_ASSERT(p_vendor_attr != NULL);
    STD_ASSERT(max_vendor_attr_count != 0);

    sai_rc = sai_attribute_validate(attr_count,attr_list, p_vendor_attr,
                                    op_type, max_vendor_attr_count);

    if(sai_rc != SAI_STATUS_SUCCESS){
        SAI_POLICER_LOG_ERR("Attribute validation failed for %d operation",
                            op_type);
        return sai_rc;
    }


    for (list_idx = 0; list_idx < attr_count; list_idx++)
    {
        attr_bits |= sai_attribute_bit_set(attr_list[list_idx].id);
        if(op_type != SAI_OP_GET){
            sai_rc = sai_policer_attr_set(p_policer_node, &attr_list[list_idx], old_node_update);
            if(sai_rc != SAI_STATUS_SUCCESS) {
                sai_rc = sai_get_indexed_ret_val(sai_rc, list_idx);
                return sai_rc;
            }
        }
    }

    if(op_type == SAI_OP_CREATE){
        if((p_policer_node->policer_mode == SAI_POLICER_MODE_TR_TCM) &&
           ((sai_policer_trtcm_mandatory_attr() & attr_bits) ^ sai_policer_trtcm_mandatory_attr())){
            SAI_POLICER_LOG_ERR("Mandatory attribute missing");
            sai_rc = SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
        }
    }

    SAI_POLICER_LOG_TRACE("Attribute bits %d",attr_bits);
    if((p_policer_node->policer_mode == SAI_POLICER_MODE_STORM_CONTROL) &&
       ((attr_bits & sai_policer_storm_control_attr_bits()) ^ (attr_bits))){
        SAI_POLICER_LOG_ERR("Invalid attribute for storm control policer");
        sai_rc = SAI_STATUS_INVALID_PARAMETER;
    }

    return sai_rc;
}

static sai_status_t sai_policer_update_port_list(dn_sai_qos_policer_t *p_policer_new)
{
    STD_ASSERT(p_policer_new != NULL);
    uint_t type = 0;
    dn_sai_qos_port_t *p_port_node = NULL;
    dn_sai_qos_policer_t *p_policer_node_old = NULL;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    p_policer_node_old = sai_qos_policer_node_get(p_policer_new->key.policer_id);

    if(p_policer_node_old == NULL){
        SAI_POLICER_LOG_ERR("Passed policer node is not found in tree");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    SAI_POLICER_LOG_TRACE("Updating ports with policerid 0x%"PRIx64"",p_policer_new->key.policer_id);

    for(type = 0; type < SAI_QOS_POLICER_TYPE_MAX; type ++){
        SAI_POLICER_LOG_TRACE("updating type %d", type);
        p_port_node = sai_qos_port_node_from_policer_get(p_policer_node_old, type);

        while(p_port_node != NULL){
            SAI_POLICER_LOG_TRACE("Updating port 0x%"PRIx64" for type %d",
                                  p_port_node->port_id, type);
            sai_rc = sai_qos_policer_npu_api_get()->policer_port_set
                (p_port_node->port_id, p_policer_new, type, true);

            if(sai_rc != SAI_STATUS_SUCCESS){
                SAI_POLICER_LOG_ERR("Npu update failed ");
                return sai_rc;
            }
            p_port_node = sai_qos_next_port_node_from_policer_get(p_policer_node_old, p_port_node, type);
        }
    }

    return sai_rc;
}

static sai_status_t sai_qos_policer_create(sai_object_id_t *policer_id,
                                    _In_ sai_object_id_t switch_id,
                                    uint32_t attr_count,
                                    const sai_attribute_t *attr_list)
{
    sai_status_t           sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_policer_t   *p_policer_node = NULL;
    sai_npu_object_id_t    hw_policer_id = 0;
    size_t                 idx = 0;

    STD_ASSERT(attr_list != NULL);
    STD_ASSERT(policer_id != NULL);

    sai_qos_lock();

    do
    {
        if((p_policer_node = sai_qos_policer_node_alloc()) == NULL){
            SAI_POLICER_LOG_ERR("Failed to allocate memory for policer node.");
            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        sai_rc = sai_policer_parse_update_attributes(p_policer_node, attr_count,
                                                 attr_list, SAI_OP_CREATE,
                                                 false);
        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_POLICER_LOG_ERR("Failed to parse attributes for policer");
            break;
        }

        SAI_POLICER_LOG_TRACE("Policer Mode is %d",p_policer_node->policer_mode);

        sai_rc = sai_qos_policer_npu_api_get()->policer_create(p_policer_node,
                                                           &hw_policer_id);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_POLICER_LOG_ERR("Npu policer create failed for policer");
            break;
        }

        SAI_POLICER_LOG_TRACE("Policer created in NPU 0x%"PRIx64"",hw_policer_id);

        hw_policer_id = sai_add_type_to_object(hw_policer_id,
                                               p_policer_node->policer_mode);

        *policer_id = sai_uoid_create(SAI_OBJECT_TYPE_POLICER,
                                  hw_policer_id);

        SAI_POLICER_LOG_TRACE("Policer object id is 0x%"PRIx64"",*policer_id);

        p_policer_node->key.policer_id = *policer_id;

        for(idx = 0; idx < SAI_QOS_POLICER_TYPE_MAX; idx ++){
            std_dll_init(&p_policer_node->port_dll_head[idx]);
        }

        std_dll_init(&p_policer_node->acl_dll_head);

        if (sai_qos_policer_node_insert(p_policer_node) != STD_ERR_OK){
            SAI_POLICER_LOG_ERR("Policer id insertion failed in RB tree");
            sai_qos_policer_npu_api_get()->policer_remove(*policer_id);
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

    }while(0);

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_POLICER_LOG_INFO("Policer created successfully");
    }
    else{
        SAI_POLICER_LOG_ERR("Policer create failed");
        sai_qos_policer_free_resources(p_policer_node);
    }

    sai_qos_unlock();
    return sai_rc;
}

static sai_status_t sai_qos_policer_is_object_in_use(dn_sai_qos_policer_t *p_policer_node)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    uint_t type = 0;
    uint_t policer_type = 0;
    STD_ASSERT(p_policer_node != NULL);

    policer_type = sai_get_type_from_npu_object(p_policer_node->key.policer_id);

    if(policer_type == SAI_POLICER_MODE_STORM_CONTROL){
        for(type = 0; type < SAI_QOS_POLICER_TYPE_MAX; type ++){
            if(sai_qos_port_node_from_policer_get(p_policer_node, type)!= NULL){
                SAI_POLICER_LOG_WARN("policer node is in use");
                sai_rc = SAI_STATUS_OBJECT_IN_USE;
                break;
            }
        }
    }
    else if((policer_type == SAI_POLICER_MODE_TR_TCM) ||
            (policer_type == SAI_POLICER_MODE_SR_TCM)){
        if(sai_qos_first_acl_rule_node_from_policer_get(p_policer_node) != NULL){
            SAI_POLICER_LOG_WARN("policer node is in use");
            sai_rc = SAI_STATUS_OBJECT_IN_USE;
        }
    }

    return sai_rc;
}

static sai_status_t sai_qos_policer_remove(sai_object_id_t policer_id)
{
    dn_sai_qos_policer_t  *p_policer_node = NULL;
    sai_status_t      sai_rc = SAI_STATUS_SUCCESS;

    if(!sai_is_obj_id_policer(policer_id)) {
        SAI_POLICER_LOG_ERR("Passed object is not policer object");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    SAI_POLICER_LOG_TRACE("Removing policer id 0x%"PRIx64"",policer_id);
    sai_qos_lock();
    do
    {
        p_policer_node = sai_qos_policer_node_get(policer_id);

        if(!p_policer_node){
            SAI_POLICER_LOG_ERR("Policer_id 0x%"PRIx64" not present in tree",
                             policer_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_rc = sai_qos_policer_is_object_in_use(p_policer_node);

        if(sai_rc != SAI_STATUS_SUCCESS){
            break;
        }
        sai_rc = sai_qos_policer_npu_api_get()->policer_remove(policer_id);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_POLICER_LOG_ERR("Npu policer remove failed for policerid 0x%"PRIx64"",
                             policer_id);
            break;
        }

        sai_qos_policer_node_remove(policer_id);
        sai_qos_policer_free_resources(p_policer_node);
    }while(0);

    sai_qos_unlock();
    return sai_rc;
}

static sai_status_t sai_qos_policer_attribute_set(sai_object_id_t policer_id,
                                              const sai_attribute_t *p_attr)
{
    dn_sai_qos_policer_t   policer_new_node;
    dn_sai_qos_policer_t  *p_policer_exist_node = NULL;
    sai_status_t      sai_rc = SAI_STATUS_SUCCESS;
    uint_t            attr_count = 1;

    STD_ASSERT (p_attr != NULL);

    SAI_POLICER_LOG_TRACE("Setting attribute Id: %d on Policer Id 0x%"PRIx64"",
                          p_attr->id, policer_id);

    if(!sai_is_obj_id_policer(policer_id)) {
        SAI_POLICER_LOG_ERR("Passed object is not policer object");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }
    sai_qos_lock();

    do
    {
        p_policer_exist_node = sai_qos_policer_node_get(policer_id);

        if(p_policer_exist_node == NULL){
            SAI_POLICER_LOG_ERR("Policer node not found");
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        memset(&policer_new_node, 0, sizeof(dn_sai_qos_policer_t));
        policer_new_node.key.policer_id = policer_id;

        sai_rc = sai_policer_parse_update_attributes(&policer_new_node,
                                                     attr_count,
                                                     p_attr, SAI_OP_SET,
                                                     false);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_POLICER_LOG_ERR("Validation failed for attribute Id %d",
                                p_attr->id);
            break;
        }

        sai_rc = sai_qos_policer_npu_api_get()->policer_set(&policer_new_node, p_attr);

        if(sai_rc != SAI_STATUS_SUCCESS){
            SAI_POLICER_LOG_ERR("NPU set failed for attribute Id %d"
                                "on policerid 0x%"PRIx64"",p_attr->id, policer_id);
            break;
        }
        SAI_POLICER_LOG_TRACE("NPU attribute set success for id %d"
                              "for policer 0x%"PRIx64"", p_attr->id, policer_id);

        if((sai_get_type_from_npu_object(policer_id) == SAI_POLICER_MODE_TR_TCM) ||
           (sai_get_type_from_npu_object(policer_id) == SAI_POLICER_MODE_SR_TCM)){
            if(sai_qos_policer_npu_api_get()->is_acl_reinstall_needed()){
                SAI_POLICER_LOG_TRACE("Updating associated ACL for policer 0x%"PRIx64"",
                                      policer_id);
                sai_rc = sai_policer_acl_entries_update(p_policer_exist_node,
                                                        &policer_new_node);
                if(sai_rc != SAI_STATUS_SUCCESS){
                    SAI_POLICER_LOG_ERR("Updating policer acl entries failed");
                    break;
                }
            }
        }
        else if(sai_get_type_from_npu_object(policer_id) == SAI_POLICER_MODE_STORM_CONTROL){
            if(!sai_qos_policer_npu_api_get()->is_storm_control_hw_port_list_supported()){
                SAI_POLICER_LOG_TRACE("Updating associated ports ");
                sai_rc = sai_policer_update_port_list(&policer_new_node);
                if(sai_rc != SAI_STATUS_SUCCESS){
                    SAI_POLICER_LOG_ERR("Updating policer on port failed");
                    break;
                }
            }
        }
        sai_rc = sai_policer_attr_set(p_policer_exist_node, p_attr, true);

    }while(0);

    sai_qos_unlock();

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_POLICER_LOG_INFO("Set attribute success for policer 0x%"PRIx64" for attr %d",
                              p_policer_exist_node->key.policer_id, p_attr->id);
    }

    return sai_rc;
}

static sai_status_t sai_qos_policer_attribute_get(sai_object_id_t policer_id,
                                       uint32_t attr_count,
                                       sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_qos_policer_t  *p_policer_node = NULL;

    STD_ASSERT(attr_list != NULL);

    SAI_POLICER_LOG_TRACE("Get policer attributes for id 0x%"PRIx64"",policer_id);

    if ((attr_count == 0) || (attr_count > SAI_QOS_POLICER_MAX_ATTR_COUNT)){
        SAI_POLICER_LOG_ERR("Invalid number of attributes for qos policer");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(!sai_is_obj_id_policer(policer_id)) {
        SAI_POLICER_LOG_ERR("Passed object is not policer object");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }
    sai_qos_lock();

    do
    {
        p_policer_node = sai_qos_policer_node_get(policer_id);

        if(p_policer_node == NULL){
            SAI_POLICER_LOG_ERR("Passed policer node is not found in tree");
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_rc = sai_policer_parse_update_attributes(p_policer_node, attr_count,
                                                 attr_list, SAI_OP_GET,
                                                 false);

        if (sai_rc != SAI_STATUS_SUCCESS){
            SAI_POLICER_LOG_ERR("Validation failed for attribute");
            break;
        }

        sai_rc = sai_qos_policer_npu_api_get()->policer_get(p_policer_node,
                                                      attr_count, attr_list);

        if (sai_rc != SAI_STATUS_SUCCESS){
            SAI_POLICER_LOG_ERR("Npu get failed for policerid 0x%"PRIx64"",
                                policer_id);
            break;
        }
    } while (0);

    sai_qos_unlock();

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_POLICER_LOG_INFO("Get attribute success for policer id 0x%"PRIx64"",
                           policer_id);
    }
    else
    {
        SAI_POLICER_LOG_ERR("Get attribute failed for policer id 0x%"PRIx64"",
                           policer_id);
    }
    return sai_rc;
}

static sai_status_t sai_qos_policer_stats_get(sai_object_id_t policer_id,
                                              uint32_t number_of_counters,
                                              const sai_policer_stat_t *counter_ids,
                                              uint64_t* counters)
{
    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t sai_qos_policer_stats_clear(sai_object_id_t policer_id,
                                                uint32_t number_of_counters,
                                                const sai_policer_stat_t *counter_ids)
{
    return SAI_STATUS_NOT_SUPPORTED;
}


static sai_status_t sai_policer_port_storm_control_node_update(dn_sai_qos_port_t *p_port_node,
                                                dn_sai_qos_policer_t *p_policer_node,
                                                uint_t type, bool is_add)
{
    dn_sai_qos_policer_t *p_policer_node_old = NULL;
    STD_ASSERT(p_port_node != NULL);
    STD_ASSERT(p_policer_node != NULL);

    if(is_add){
        SAI_POLICER_LOG_TRACE("Add policer 0x%"PRIx64" on port 0x%"PRIx64"",
                              p_policer_node->key.policer_id,p_port_node->port_id);
        if(p_port_node->policer_id[type] != SAI_NULL_OBJECT_ID){
            SAI_POLICER_LOG_TRACE("Removing existing policerid of type %d",type);
            p_policer_node_old = sai_qos_policer_node_get(p_port_node->policer_id[type]);

            if(p_policer_node_old != NULL){
                std_dll_remove(&p_policer_node_old->port_dll_head[type],
                               &p_port_node->policer_dll_glue[type]);
            }

        }
        std_dll_insertatback(&p_policer_node->port_dll_head[type],
                             &p_port_node->policer_dll_glue[type]);
        p_port_node->policer_id[type] = p_policer_node->key.policer_id;
    }else {
        SAI_POLICER_LOG_TRACE("Remove policer 0x%"PRIx64" on port 0x%"PRIx64"",
                              p_policer_node->key.policer_id,p_port_node->port_id);
        std_dll_remove(&p_policer_node->port_dll_head[type],
                       &p_port_node->policer_dll_glue[type]);

        p_port_node->policer_id[type] = SAI_NULL_OBJECT_ID;
    }

    return SAI_STATUS_SUCCESS;

}

sai_status_t sai_port_attr_storm_control_policer_set(sai_object_id_t port_id,
                                                    const sai_attribute_t *attr)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    sai_qos_lock();

    sai_rc = sai_port_attr_storm_control_policer_set_internal (port_id, attr);

    sai_qos_unlock();

    return sai_rc;

}

sai_status_t sai_port_attr_storm_control_policer_set_internal
                                                   (sai_object_id_t port_id,
                                                    const sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);
    uint_t type = 0;
    bool is_add = false;
    dn_sai_qos_policer_t *p_policer_node = NULL;
    dn_sai_qos_port_t *p_port_node = NULL;
    sai_status_t sai_rc =  SAI_STATUS_SUCCESS;

    if(attr->id == SAI_PORT_ATTR_POLICER_ID)
    {
        if(!sai_qos_policer_npu_api_get()->port_policer_supported()){
            SAI_POLICER_LOG_ERR("Port level policer is not supported");
            return SAI_STATUS_NOT_SUPPORTED;
        }
    }

    type = sai_get_storm_control_type_from_port_attr(attr->id);

    if(type == SAI_QOS_POLICER_TYPE_INVALID){
        return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }
    p_port_node = sai_qos_port_node_get(port_id);

    if(p_port_node == NULL){
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if((attr->value.oid != SAI_NULL_OBJECT_ID) && (!sai_is_obj_id_policer(attr->value.oid))) {
        SAI_POLICER_LOG_ERR("Object passed is not policer");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if (attr->value.oid == p_port_node->policer_id[type])
      return SAI_STATUS_SUCCESS;

    if(attr->value.oid != SAI_NULL_OBJECT_ID){
        is_add = true;
        if(sai_get_type_from_npu_object(attr->value.oid) != SAI_POLICER_MODE_STORM_CONTROL) {
            SAI_POLICER_LOG_ERR("Policer is not storm control policer");
            return SAI_STATUS_INVALID_OBJECT_TYPE;
        }

        p_policer_node = sai_qos_policer_node_get(attr->value.oid);

        if(p_policer_node == NULL) {
            SAI_POLICER_LOG_ERR("Policer node is NULL");
            return SAI_STATUS_INVALID_OBJECT_ID;
        }

    }else {
        p_policer_node = sai_qos_policer_node_get(p_port_node->policer_id[type]);
        if(p_policer_node == NULL) {
            SAI_POLICER_LOG_ERR("Policer node is NULL");
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        SAI_POLICER_LOG_TRACE(" Setting null object id for policerid 0x%"PRIx64" of type %d",
                              p_port_node->policer_id[type], type);
        is_add = false;
    }

    sai_rc = sai_qos_policer_npu_api_get()->policer_port_set(p_port_node->port_id,
                                                             p_policer_node,
                                                             type, is_add);

    if(sai_rc == SAI_STATUS_SUCCESS){
        sai_rc = sai_policer_port_storm_control_node_update(p_port_node,
                                                            p_policer_node,
                                                            type, is_add);
    }

    return sai_rc;
}
/* API method table for Qos policer to be returned during query.
 **/
static sai_policer_api_t sai_qos_policer_method_table = {
    sai_qos_policer_create,
    sai_qos_policer_remove,
    sai_qos_policer_attribute_set,
    sai_qos_policer_attribute_get,
    sai_qos_policer_stats_get,
    sai_qos_policer_stats_clear
};

sai_policer_api_t *sai_policer_api_query (void)
{
    return &sai_qos_policer_method_table;
}
