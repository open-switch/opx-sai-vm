/************************************************************************
* * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/**
* @file sai_vlan.c
*
* @brief This file contains implementation of SAI VLAN APIs.
*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "std_assert.h"
#include "saivlan.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saiport.h"
#include "sai_modules_init.h"
#include "sai_vlan_common.h"
#include "sai_npu_port.h"
#include "sai_npu_vlan.h"
#include "sai_vlan_api.h"
#include "sai_switch_utils.h"
#include "sai_gen_utils.h"
#include "sai_oid_utils.h"
#include "sai_stp_api.h"
#include "sai_common_infra.h"
#include "sai_bridge_common.h"
#include "sai_bridge_api.h"
#include "sai_bridge_main.h"
#include "sai_port_utils.h"
#include "sai_lag_api.h"

#define SAI_L2_DEFAULT_VLAN_MAX_ATTR_COUNT 1

/*LAG lock needs to be taken since LAG can be modified independently of bridge port*/
static void sai_vlan_take_module_lock(sai_object_id_t bridge_port_id)
{
    sai_bridge_lock();

    if(sai_is_bridge_port_obj_lag(bridge_port_id)) {
        sai_lag_lock();
    }
}

static void sai_vlan_give_module_lock(sai_object_id_t bridge_port_id)
{
    if(sai_is_bridge_port_obj_lag(bridge_port_id)) {
        sai_lag_unlock();
    }

    sai_bridge_unlock();
}

static sai_status_t sai_is_vlan_id_available(sai_vlan_id_t vlan_id)
{
    if(!sai_is_valid_vlan_id(vlan_id)) {
        SAI_VLAN_LOG_ERR("Invalid vlan id %d",vlan_id);
        return SAI_STATUS_INVALID_VLAN_ID;
    }
    if(sai_is_internal_vlan_id(vlan_id)) {
        SAI_VLAN_LOG_WARN("Not modifying internal vlan id %d", vlan_id);
        return SAI_STATUS_NOT_SUPPORTED;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_is_vlan_configurable(sai_vlan_id_t vlan_id)
{
    sai_status_t ret_val;
    ret_val = sai_is_vlan_id_available(vlan_id);

    if(ret_val != SAI_STATUS_SUCCESS) {
        return ret_val;
    }
    if(!sai_is_vlan_created(vlan_id)) {
        SAI_VLAN_LOG_ERR("vlan id not found %d",vlan_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_l2_set_vlan_attribute(sai_object_id_t vlan_obj_id,
                                           const sai_attribute_t *attr)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;
    sai_vlan_id_t vlan_id = VLAN_UNDEF;
    sai_attribute_t old_attr;

    if(!sai_is_obj_id_vlan(vlan_obj_id)) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    vlan_id = sai_vlan_obj_id_to_vlan_id(vlan_obj_id);

    STD_ASSERT(attr != NULL);
    if(!sai_is_valid_vlan_id(vlan_id)) {
        SAI_VLAN_LOG_ERR("Invalid vlan id %d", vlan_id);
        return SAI_STATUS_INVALID_VLAN_ID;
    }
    memset(&old_attr, 0, sizeof(old_attr));
    sai_vlan_lock();
    if(!sai_is_vlan_created(vlan_id)) {
        SAI_VLAN_LOG_ERR("vlan id not found %d",vlan_id);
        sai_vlan_unlock();
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    switch(attr->id)
    {
        case SAI_VLAN_ATTR_STP_INSTANCE:
            SAI_VLAN_LOG_TRACE("Setting stp instance for vlan:%d value=0x%"PRIx64"",
                                 vlan_id, attr->value.oid);
            ret_val = sai_stp_vlan_handle(vlan_id, attr->value.oid);
            break;

        case SAI_VLAN_ATTR_VLAN_ID:
        case SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES:
        case SAI_VLAN_ATTR_MEMBER_LIST:
        case SAI_VLAN_ATTR_IPV4_MCAST_LOOKUP_KEY_TYPE:
        case SAI_VLAN_ATTR_IPV6_MCAST_LOOKUP_KEY_TYPE:
        case SAI_VLAN_ATTR_UNKNOWN_NON_IP_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_UNKNOWN_IPV4_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_UNKNOWN_IPV6_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_UNKNOWN_LINKLOCAL_MCAST_OUTPUT_GROUP_ID:
        case SAI_VLAN_ATTR_INGRESS_ACL:
        case SAI_VLAN_ATTR_EGRESS_ACL:
        case SAI_VLAN_ATTR_LEARN_DISABLE:
        case SAI_VLAN_ATTR_META_DATA:
        case SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
        case SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
        case SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
        case SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
        case SAI_VLAN_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
        case SAI_VLAN_ATTR_BROADCAST_FLOOD_GROUP:

            SAI_VLAN_LOG_TRACE("Setting Vlan Meta Data for vlan:%d value=%d",
                    vlan_id, attr->value.u32);
            ret_val = sai_vlan_npu_api_get()->vlan_set_attribute(vlan_obj_id, attr);
            break;

        default:
            SAI_VLAN_LOG_TRACE("Invalid attr :%d for vlan id %d",
                               attr->id, vlan_id);
            ret_val = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            break;

    }
    if(ret_val == SAI_STATUS_SUCCESS) {
        ret_val = sai_vlan_set_attribute_in_cache(vlan_obj_id, attr);
        if(ret_val != SAI_STATUS_SUCCESS) {
            SAI_VLAN_LOG_ERR("Error %d in setting vlan attr %d on vlan obj 0x%"PRIx64" in cache",
                             ret_val, attr->id, vlan_obj_id);
            old_attr.id = attr->id;
            sai_vlan_get_attribute_from_cache(vlan_obj_id, &old_attr);
            sai_vlan_npu_api_get()->vlan_set_attribute(vlan_obj_id, &old_attr);
        }
    } else {
        SAI_VLAN_LOG_ERR("Error %d in setting vlan attr %d on vlan obj 0x%"PRIx64"",
                         ret_val, attr->id, vlan_obj_id);
    }
    sai_vlan_unlock();
    return ret_val;
}

static sai_status_t sai_l2_get_vlan_attribute(sai_object_id_t vlan_obj_id,
        uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;
    unsigned int attr_idx = 0;
    sai_vlan_id_t vlan_id = VLAN_UNDEF;

    if(!sai_is_obj_id_vlan(vlan_obj_id)) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    vlan_id = sai_vlan_obj_id_to_vlan_id(vlan_obj_id);

    STD_ASSERT(attr_list != NULL);
    if(!sai_is_valid_vlan_id(vlan_id)) {
        SAI_VLAN_LOG_ERR("Invalid vlan id %d",vlan_id);
        return SAI_STATUS_INVALID_VLAN_ID;
    }
    if(attr_count == 0) {
        SAI_VLAN_LOG_ERR("Invalid attribute count 0 for vlan:%d",vlan_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    sai_vlan_lock();
    if(!sai_is_vlan_created(vlan_id)) {
        SAI_VLAN_LOG_ERR("vlan id not found %d",vlan_id);
        sai_vlan_unlock();
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        switch(attr_list[attr_idx].id)
        {
            case SAI_VLAN_ATTR_STP_INSTANCE:
                SAI_VLAN_LOG_TRACE("Getting STP instance id for vlan:%d", vlan_id);
                ret_val = sai_stp_vlan_stp_get (vlan_id, &attr_list[attr_idx].value.oid);
                break;

            case SAI_VLAN_ATTR_VLAN_ID:
            case SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES:
            case SAI_VLAN_ATTR_MEMBER_LIST:
            case SAI_VLAN_ATTR_LEARN_DISABLE:
            case SAI_VLAN_ATTR_IPV4_MCAST_LOOKUP_KEY_TYPE:
            case SAI_VLAN_ATTR_IPV6_MCAST_LOOKUP_KEY_TYPE:
            case SAI_VLAN_ATTR_UNKNOWN_NON_IP_MCAST_OUTPUT_GROUP_ID:
            case SAI_VLAN_ATTR_UNKNOWN_IPV4_MCAST_OUTPUT_GROUP_ID:
            case SAI_VLAN_ATTR_UNKNOWN_IPV6_MCAST_OUTPUT_GROUP_ID:
            case SAI_VLAN_ATTR_UNKNOWN_LINKLOCAL_MCAST_OUTPUT_GROUP_ID:
            case SAI_VLAN_ATTR_INGRESS_ACL:
            case SAI_VLAN_ATTR_EGRESS_ACL:
            case SAI_VLAN_ATTR_META_DATA:
            case SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
            case SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
            case SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
            case SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
            case SAI_VLAN_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
            case SAI_VLAN_ATTR_BROADCAST_FLOOD_GROUP:

                ret_val = sai_vlan_get_attribute_from_cache(vlan_obj_id, &attr_list[attr_idx]);
                break;

            default:
                SAI_VLAN_LOG_TRACE("Invalid attr :%d for vlan id %d",
                        attr_list[attr_idx].id, vlan_id);
                ret_val = SAI_STATUS_INVALID_ATTRIBUTE_0;
                break;

        }
        if(ret_val != SAI_STATUS_SUCCESS){
            ret_val = sai_get_indexed_ret_val(ret_val, attr_idx);
            break;
        }
    }
    sai_vlan_unlock();
    return ret_val;
}

static sai_status_t sai_l2_delete_vlan(sai_vlan_id_t vlan_id)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;

    ret_val = sai_stp_vlan_destroy(vlan_id);
    if(ret_val != SAI_STATUS_SUCCESS){
        SAI_VLAN_LOG_ERR("vlan removal from stp instance failed for vlan %d",vlan_id);
        return ret_val;
    }

    ret_val = sai_vlan_npu_api_get()->vlan_delete(vlan_id);
    if(ret_val != SAI_STATUS_SUCCESS){
        SAI_VLAN_LOG_ERR("Not removing vlan id %d",vlan_id);
        return ret_val;
    }
    sai_remove_vlan_from_list(vlan_id);
    return ret_val;
}

static sai_status_t sai_l2_remove_vlan(sai_object_id_t vlan_obj_id)
{
    sai_status_t ret_val = SAI_STATUS_SUCCESS;
    sai_vlan_id_t vlan_id = VLAN_UNDEF;

    if(!sai_is_obj_id_vlan(vlan_obj_id)) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    SAI_VLAN_LOG_TRACE("Deleting VLAN %d ", vlan_id);
    vlan_id = sai_vlan_obj_id_to_vlan_id(vlan_obj_id);

    sai_vlan_lock();

    do {
        ret_val = sai_is_vlan_configurable(vlan_id);
        if(ret_val == SAI_STATUS_SUCCESS) {
            if(sai_is_vlan_obj_in_use(vlan_id)) {
                ret_val =  SAI_STATUS_OBJECT_IN_USE;
                break;
            }
            ret_val = sai_l2_delete_vlan(vlan_id);
        }
    }while (0);

    sai_vlan_unlock();
    return ret_val;
}

static sai_status_t sai_l2_create_vlan(
        sai_object_id_t *vlan_obj_id,
        sai_object_id_t switch_id,
        uint32_t attr_count,
        const sai_attribute_t *attr_list)
{
    uint32_t attr_idx = 0;
    sai_status_t ret_val = SAI_STATUS_SUCCESS;
    sai_vlan_id_t vlan_id = VLAN_UNDEF;
    bool vlan_id_attr_present = false;

    if (attr_count > 0) {
        STD_ASSERT ((attr_list != NULL));
    } else {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    for (attr_idx = 0;
            (attr_idx < attr_count) && !(vlan_id_attr_present);
            attr_idx++) {
        switch (attr_list [attr_idx].id) {
            case SAI_VLAN_ATTR_VLAN_ID:
                vlan_id_attr_present = true;
                vlan_id = attr_list[attr_idx].value.u16;
                break;
        }
    }

    if(!vlan_id_attr_present) {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    SAI_VLAN_LOG_TRACE("Creating VLAN %d ", vlan_id);
    sai_vlan_lock();
    do {
        if((ret_val = sai_is_vlan_id_available(vlan_id))
                != SAI_STATUS_SUCCESS) {
            break;
        }

        if((ret_val = sai_insert_vlan_in_list(vlan_id))
                != SAI_STATUS_SUCCESS) {
            SAI_VLAN_LOG_ERR("Not creating vlan id %d", vlan_id);
            break;
        }

        ret_val = sai_vlan_npu_api_get()->vlan_create(vlan_id);
        if((ret_val != SAI_STATUS_SUCCESS) &&
            (ret_val != SAI_STATUS_ITEM_ALREADY_EXISTS)) {
            sai_remove_vlan_from_list(vlan_id);
            break;
        }

        if(vlan_id != SAI_INIT_DEFAULT_VLAN) {
            if((ret_val = sai_stp_default_stp_vlan_update(vlan_id))
                    != SAI_STATUS_SUCCESS) {
                SAI_VLAN_LOG_ERR(
                        "Association of vlan %d to default STP instance failed",
                        vlan_id);
            }
        }
    }while(0);

    sai_vlan_unlock();

    if (attr_count > 1) {
        for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {
            if((ret_val = sai_l2_set_vlan_attribute(
                            sai_vlan_id_to_vlan_obj_id(vlan_id),
                            &attr_list[attr_idx])) != SAI_STATUS_SUCCESS) {
                sai_l2_remove_vlan(sai_vlan_id_to_vlan_obj_id(vlan_id));
            }
        }
    }

    if(ret_val == SAI_STATUS_SUCCESS) {
        *vlan_obj_id = sai_vlan_id_to_vlan_obj_id(vlan_id);
    } else {
        *vlan_obj_id = SAI_NULL_OBJECT_ID;
    }

    return ret_val;
}

static bool sai_vlan_member_is_valid_bridge_port(sai_object_id_t bridge_port_id)
{
    if(!sai_is_bridge_port_created(bridge_port_id)){
        SAI_VLAN_LOG_ERR("Invalid bridge port id 0x%"PRIx64"", bridge_port_id);
         return false;
    }
    if(!sai_is_bridge_port_type_port(bridge_port_id)){
       SAI_VLAN_LOG_ERR("Invalid bridge port type for add to vlan member"
                        " for bridge port id 0x%"PRIx64"", bridge_port_id);
        return false;
    }
    return true;
}

static sai_status_t sai_vlan_get_bridge_port_from_attr(uint32_t attr_count,
                                                       const sai_attribute_t *attr_list,
                                                       sai_object_id_t *bridge_port_id)
{
    uint_t attr_idx;

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        if(attr_list[attr_idx].id == SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID) {
            *bridge_port_id = attr_list[attr_idx].value.oid;
            return SAI_STATUS_SUCCESS;
        }
    }
    *bridge_port_id = SAI_NULL_OBJECT_ID;
    return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
}

static sai_status_t sai_l2_create_vlan_member(sai_object_id_t *vlan_member_id,
                                              sai_object_id_t switch_id, uint32_t attr_count,
                                              const sai_attribute_t *attr_list)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;
    sai_port_fwd_mode_t fwd_mode = SAI_PORT_FWD_MODE_UNKNOWN;
    bool vlan_id_attr_present = false;
    bool bridge_port_present = false;
    sai_vlan_member_node_t vlan_node;
    sai_vlan_id_t vlan_id = VLAN_UNDEF;
    uint32_t attr_idx = 0;
    sai_object_id_t port_obj_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;

    STD_ASSERT (vlan_member_id != NULL);
    STD_ASSERT (attr_list != NULL);

    *vlan_member_id = SAI_INVALID_VLAN_MEMBER_ID;

    if (attr_count > 0) {
        STD_ASSERT ((attr_list != NULL));
    } else {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }
    ret_val = sai_vlan_get_bridge_port_from_attr(attr_count, attr_list, &bridge_port_id);

    if(ret_val != SAI_STATUS_SUCCESS) {
        return ret_val;
    }

    vlan_node.switch_id = switch_id;
    vlan_node.tagging_mode = SAI_VLAN_TAGGING_MODE_UNTAGGED;

    sai_vlan_lock();
    sai_vlan_take_module_lock(bridge_port_id);

    do {
        for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {
            switch (attr_list [attr_idx].id) {
                case SAI_VLAN_MEMBER_ATTR_VLAN_ID:
                    if(!sai_is_obj_id_vlan(attr_list[attr_idx].value.oid)) {
                        ret_val = SAI_STATUS_INVALID_OBJECT_ID;
                        break;
                    }
                    vlan_node.vlan_id = attr_list[attr_idx].value.oid;
                    vlan_id = sai_vlan_obj_id_to_vlan_id(
                            attr_list[attr_idx].value.oid);
                    vlan_id_attr_present = true;
                    break;
                case SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID:
                    vlan_node.bridge_port_id = attr_list[attr_idx].value.oid;
                    if(!sai_vlan_member_is_valid_bridge_port(vlan_node.bridge_port_id)) {
                        SAI_VLAN_LOG_ERR("Invalid bridge port id 0x%"PRIx64" in create",
                                         vlan_node.bridge_port_id);
                        ret_val = sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                          attr_idx);
                        break;
                    }
                    bridge_port_present = true;
                    break;
                case SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE:
                    vlan_node.tagging_mode = attr_list[attr_idx].value.u32;
                    if(!sai_is_valid_vlan_tagging_mode(vlan_node.tagging_mode)) {
                        SAI_VLAN_LOG_ERR("Invalid tagging mode %d for creating vlan member",
                                         vlan_node.tagging_mode);
                        ret_val = sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                                          attr_idx);
                        break;
                    }
                    break;
                default:
                    ret_val = sai_get_indexed_ret_val(SAI_STATUS_UNKNOWN_ATTRIBUTE_0, attr_idx);
            }
            if(ret_val != SAI_STATUS_SUCCESS) {
                break;
            }
        }

        if(ret_val != SAI_STATUS_SUCCESS) {
            break;
        }
        if(!(vlan_id_attr_present) || !(bridge_port_present)) {
            ret_val =  SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
            break;
        }

        if((ret_val = sai_is_vlan_configurable(vlan_id))
                != SAI_STATUS_SUCCESS) {
            break;
        }

        ret_val = sai_bridge_port_get_port_id(vlan_node.bridge_port_id, &port_obj_id);
        if(ret_val != SAI_STATUS_SUCCESS) {
            SAI_VLAN_LOG_ERR("Error in getting port obj id from bridge port 0x%"PRIx64"",
                             vlan_node.bridge_port_id);
            break;
        }
        if(sai_bridge_is_sub_port_created(port_obj_id, vlan_id)) {
            ret_val = SAI_STATUS_FAILURE;
            SAI_VLAN_LOG_ERR("Error - A bridge subport is already created with port 0x%"PRIx64""
                             "and vlan %d", port_obj_id, vlan_id);
            break;
        }
        if(sai_is_obj_id_lag(port_obj_id)) {
            ret_val = sai_lag_forwarding_mode_get(port_obj_id, &fwd_mode);
        } else {
            ret_val = sai_port_forwarding_mode_get(port_obj_id, &fwd_mode);
        }
        if(ret_val != SAI_STATUS_SUCCESS) {
            SAI_VLAN_LOG_ERR("Error %d in getting forwarding mode for 0x%"PRIx64"",
                             ret_val, port_obj_id);
            break;
        }

        if(fwd_mode == SAI_PORT_FWD_MODE_ROUTING) {
            ret_val = SAI_STATUS_FAILURE;
            SAI_VLAN_LOG_ERR("port 0x%"PRIx64" attached to bridge port 0x%"PRIx64""
                             " is in routing mode.", port_obj_id, vlan_node.bridge_port_id);
            break;
        }

        if(sai_is_bridge_port_vlan_member(vlan_id, vlan_node.bridge_port_id)) {
            ret_val = SAI_STATUS_ITEM_ALREADY_EXISTS;
            break;
        }

        if((ret_val = sai_vlan_npu_api_get()->vlan_member_create(&vlan_node))
                != SAI_STATUS_SUCCESS) {
            break;
        }

        *vlan_member_id = vlan_node.vlan_member_id;
        if((ret_val = sai_add_vlan_member_node(vlan_node))
                != SAI_STATUS_SUCCESS) {
            SAI_VLAN_LOG_ERR("Unable to add VLAN member 0x%"PRIx64" to vlan:%d cache",
                             vlan_node.vlan_member_id,vlan_id);
            sai_vlan_npu_api_get()->vlan_member_remove(&vlan_node);
        }
        sai_bridge_port_to_vlan_member_map_insert(vlan_node.bridge_port_id, *vlan_member_id);

        SAI_VLAN_LOG_TRACE("Added bridge port 0x%"PRIx64" on vlan:%d",
                           vlan_node.bridge_port_id, vlan_id);
    } while(0);

    sai_vlan_give_module_lock(bridge_port_id);
    sai_vlan_unlock();
    return ret_val;
}

static sai_status_t sai_l2_set_vlan_member_attribute(sai_object_id_t vlan_member_id,
                                                     const sai_attribute_t *attr)
{
    sai_status_t ret_val = SAI_STATUS_SUCCESS;
    sai_vlan_member_node_t *vlan_member_node = NULL;
    sai_vlan_tagging_mode_t old_tagging_mode = SAI_VLAN_TAGGING_MODE_UNTAGGED;

    STD_ASSERT(attr != NULL);

    if(!sai_is_obj_id_vlan_member(vlan_member_id)) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_vlan_lock();

    vlan_member_node = sai_find_vlan_member_node(vlan_member_id);
    if(vlan_member_node == NULL) {
        ret_val = SAI_STATUS_ITEM_NOT_FOUND;
    } else {
        sai_vlan_take_module_lock(vlan_member_node->bridge_port_id);
        switch(attr->id)
        {
            case SAI_VLAN_MEMBER_ATTR_VLAN_ID:
                ret_val = SAI_STATUS_INVALID_ATTRIBUTE_0;
                break;
            case SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID:
                ret_val = SAI_STATUS_INVALID_ATTRIBUTE_0;
                break;
            case SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE:
                if(vlan_member_node->tagging_mode != attr->value.u32) {
                    old_tagging_mode = vlan_member_node->tagging_mode;
                    vlan_member_node->tagging_mode = attr->value.u32;
                    ret_val = sai_vlan_npu_api_get()->set_vlan_member_tagging_mode(
                                                            vlan_member_node);
                    if(ret_val != SAI_STATUS_SUCCESS) {
                        vlan_member_node->tagging_mode = old_tagging_mode;
                    }
                }
                break;
            default:
                ret_val = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }
        sai_vlan_give_module_lock(vlan_member_node->bridge_port_id);
    }

    sai_vlan_unlock();
    return ret_val;
}

static sai_status_t sai_l2_get_vlan_member_attribute(sai_object_id_t vlan_member_id,
        const uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t ret_val = SAI_STATUS_SUCCESS;
    sai_vlan_member_node_t *vlan_member_node = NULL;
    uint32_t attr_idx = 0;

    STD_ASSERT(attr_list != NULL);
    if (attr_count > 0) {
        STD_ASSERT ((attr_list != NULL));
    }

    if(!sai_is_obj_id_vlan_member(vlan_member_id)) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_vlan_lock();
    if((vlan_member_node = sai_find_vlan_member_node(vlan_member_id))
            == NULL) {
        ret_val = SAI_STATUS_ITEM_NOT_FOUND;
    } else {
        for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {
            switch (attr_list [attr_idx].id) {
                case SAI_VLAN_MEMBER_ATTR_VLAN_ID:
                    attr_list[attr_idx].value.oid = vlan_member_node->vlan_id;
                    break;
                case SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID:
                    attr_list[attr_idx].value.oid = vlan_member_node->bridge_port_id;
                    break;
                case SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE:
                    attr_list[attr_idx].value.u32 = vlan_member_node->tagging_mode;
                    break;
                default:
                    ret_val = sai_get_indexed_ret_val(SAI_STATUS_UNKNOWN_ATTRIBUTE_0,attr_idx);
                    break;
            }
            if(SAI_STATUS_SUCCESS != ret_val) {
                break;
            }
        }
    }

    sai_vlan_unlock();
    return ret_val;
}

static sai_status_t sai_l2_remove_vlan_member(sai_object_id_t vlan_member_id)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;
    sai_vlan_member_node_t *vlan_node = NULL;
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;

    if(!sai_is_obj_id_vlan_member(vlan_member_id)) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_vlan_lock();
    if((vlan_node = sai_find_vlan_member_node(vlan_member_id)) == NULL) {
        sai_vlan_unlock();
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    bridge_port_id = vlan_node->bridge_port_id;
    sai_vlan_take_module_lock(vlan_node->bridge_port_id);
    do {

        if((ret_val = sai_vlan_npu_api_get()->vlan_member_remove(vlan_node))
                != SAI_STATUS_SUCCESS) {
            break;
        }
        sai_bridge_port_to_vlan_member_map_remove(vlan_node->bridge_port_id, vlan_member_id);

        ret_val = sai_remove_vlan_member_node(*vlan_node);
    } while(0);

    sai_vlan_give_module_lock(bridge_port_id);

    sai_vlan_unlock();
    return ret_val;
}

static sai_status_t sai_l2_get_vlan_stats(sai_object_id_t vlan_obj_id,
        uint32_t number_of_counters,
        const sai_vlan_stat_t *counter_ids,
        uint64_t *counters)
{
    sai_vlan_id_t vlan_id = VLAN_UNDEF;

    STD_ASSERT(counter_ids != NULL);
    STD_ASSERT(counters != NULL);

    if(!sai_is_obj_id_vlan(vlan_obj_id)) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    vlan_id = sai_vlan_obj_id_to_vlan_id(vlan_obj_id);
    if(!sai_is_valid_vlan_id(vlan_id)) {
        SAI_VLAN_LOG_ERR("Invalid vlan id %d",vlan_id);
        return SAI_STATUS_INVALID_VLAN_ID;
    }
    SAI_VLAN_LOG_TRACE("Get stats collection for vlan id %d",
            vlan_id);

    return sai_vlan_npu_api_get()->get_vlan_stats(vlan_id, counter_ids,
            number_of_counters, counters);
}

static sai_status_t sai_l2_clear_vlan_stats (sai_object_id_t vlan_obj_id,
                                             uint32_t number_of_counters,
                                             const sai_vlan_stat_t *counter_ids)
{
    sai_vlan_id_t vlan_id = VLAN_UNDEF;

    STD_ASSERT(counter_ids != NULL);
    if(!sai_is_obj_id_vlan(vlan_obj_id)) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    vlan_id = sai_vlan_obj_id_to_vlan_id(vlan_obj_id);
    if(!sai_is_valid_vlan_id(vlan_id)) {
        SAI_VLAN_LOG_ERR("Invalid vlan id %d",vlan_id);
        return SAI_STATUS_INVALID_VLAN_ID;
    }
    SAI_VLAN_LOG_TRACE("Clear stats for vlan id %d",
            vlan_id);

    return sai_vlan_npu_api_get()->clear_vlan_stats (vlan_id, counter_ids,
            number_of_counters);
}

sai_status_t sai_vlan_add_bridge_port_to_default_vlan(sai_object_id_t bridge_port_id)
{
    sai_vlan_id_t   vlan_id = SAI_INIT_DEFAULT_VLAN;
    sai_status_t    ret_val = SAI_STATUS_FAILURE;;
    sai_attribute_t attr[SAI_VLAN_MEMBER_ATTR_END];
    uint32_t        attr_count = 0;
    sai_object_id_t vlan_member_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t sai_port_id = SAI_NULL_OBJECT_ID;


    ret_val = sai_bridge_port_get_port_id(bridge_port_id, &sai_port_id);

    if(ret_val != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR("Error in getting port object id from bridge port 0x%"PRIx64"",
                         bridge_port_id);
        return ret_val;
    }
    /*Add newly created port to default VLAN ID*/

    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr[attr_count].value.oid = sai_vlan_id_to_vlan_obj_id(vlan_id);
    attr_count++;

    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr[attr_count].value.oid = bridge_port_id;
    attr_count++;

    attr[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
    attr[attr_count].value.u32 = SAI_DEF_VLAN_TAGGING_MODE;
    attr_count++;

    ret_val = sai_l2_create_vlan_member(&vlan_member_id, 0, attr_count, attr);
    if((ret_val != SAI_STATUS_SUCCESS) &&
            (ret_val != SAI_STATUS_ITEM_ALREADY_EXISTS)) {
        SAI_VLAN_LOG_ERR("Unable to add bridge port 0x%"PRIx64" to def vlan:%d rc:%d",
                         bridge_port_id, vlan_id,ret_val);
        return ret_val;
    }
    memset(&attr, 0, sizeof(attr));
    attr->id = SAI_PORT_ATTR_PORT_VLAN_ID;
    attr->value.u16 = vlan_id;
    ret_val = sai_port_api_query()->set_port_attribute(sai_port_id,
                                                      (const sai_attribute_t*)&attr);
    if(ret_val != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR("Unable to set port 0x%"PRIx64" pvid to def vlan:%d",
                         sai_port_id, vlan_id);
        sai_l2_remove_vlan_member(vlan_member_id);
        return ret_val;
    }

    memset(&attr, 0, sizeof(attr));
    attr->id = SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY;
    attr->value.u8 = SAI_DFLT_VLAN_PRIORITY;
    ret_val = sai_port_api_query()->set_port_attribute(sai_port_id,
                                                      (const sai_attribute_t*)&attr);
    if(ret_val != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR("Unable to set port 0x%"PRIx64" def vlan priority:%d",
                         sai_port_id, SAI_DFLT_VLAN_PRIORITY);
        return ret_val;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_vlan_handle_lag_change_notification(sai_object_id_t bridge_port_id,
        const sai_bridge_port_notif_t *data)
{
    uint_t                  vlan_member_cnt = 0;
    uint_t                  member_idx = 0;
    sai_object_id_t         vlan_member_id = SAI_NULL_OBJECT_ID;
    sai_status_t            sai_rc = SAI_STATUS_FAILURE;
    sai_vlan_member_node_t *vlan_member_node = NULL;


    sai_vlan_lock();
    sai_vlan_take_module_lock(bridge_port_id);
    do {
        sai_rc = sai_bridge_port_to_vlan_member_count_get(bridge_port_id, &vlan_member_cnt);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_VLAN_LOG_ERR("Error %d in obtaining the vlan member count for bridge port 0x%"PRIx64"",
                             sai_rc, bridge_port_id);
            break;
        }

        if(vlan_member_cnt == 0) {
            SAI_VLAN_LOG_TRACE("No VLAN members associated to bridge port 0x%"PRIx64"",
                               bridge_port_id);
            sai_rc =  SAI_STATUS_SUCCESS;
            break;
        }


        for(member_idx = 0; member_idx < vlan_member_cnt; member_idx++) {
            sai_rc = sai_bridge_port_get_vlan_member_at_index(bridge_port_id, member_idx,
                                                              &vlan_member_id);
            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_VLAN_LOG_ERR("Error %d in obtaining the vlan member at idx %d for bridge port "
                                 "0x%"PRIx64"", sai_rc, member_idx, bridge_port_id);
                break;
            }
            vlan_member_node = sai_find_vlan_member_node(vlan_member_id);
            if(vlan_member_node == NULL) {
                SAI_VLAN_LOG_ERR("Error - Unable to find info for vlan member 0x%"PRIx64"",
                                 vlan_member_id);
                sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
                break;
            }
            sai_rc = sai_vlan_npu_api_get()->
                vlan_member_lag_notif_handler(vlan_member_node, data->lag_id,
                                              data->lag_port_mod_list->count,
                                              data->lag_port_mod_list->list,
                                              data->lag_add_port);
            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_VLAN_LOG_ERR("Error %d in npu update of lag handler for vlan member "
                                 "0x%"PRIx64"", sai_rc, vlan_member_id);
                break;
            }
        }

    } while(0);

    sai_vlan_give_module_lock(bridge_port_id);
    sai_vlan_unlock();
    return sai_rc;
}

static sai_status_t sai_vlan_bridge_port_notif_handler(sai_object_id_t bridge_port_id,
                                                       const sai_bridge_port_notif_t *data)
{
    if(data == NULL) {
        SAI_VLAN_LOG_ERR("NULL data in bridge port 0x%"PRIx64" callback",bridge_port_id);
    }
    if(data->event == SAI_BRIDGE_PORT_EVENT_INIT_CREATE) {
        return sai_vlan_add_bridge_port_to_default_vlan(bridge_port_id);
    } else if(data->event == SAI_BRIDGE_PORT_EVENT_LAG_MODIFY) {
        return sai_vlan_handle_lag_change_notification(bridge_port_id, data);
    }
    return SAI_STATUS_SUCCESS;
}


static sai_status_t sai_l2_default_vlan_init(sai_vlan_id_t vlan_id)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;
    sai_attribute_t attr[SAI_L2_DEFAULT_VLAN_MAX_ATTR_COUNT];
    uint32_t attr_count = 0;
    sai_object_id_t vlan_obj_id = SAI_NULL_OBJECT_ID;

    attr[attr_count].id = SAI_VLAN_ATTR_VLAN_ID;
    attr[attr_count].value.u16 = vlan_id;
    attr_count++;

    ret_val = sai_l2_create_vlan(&vlan_obj_id, 0, attr_count, attr);
    if((ret_val != SAI_STATUS_SUCCESS) &&
            (ret_val != SAI_STATUS_ITEM_ALREADY_EXISTS)){
        SAI_VLAN_LOG_ERR("Not creating vlan id %d", vlan_id);
        return ret_val;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_vlan_init(void)
{
    sai_status_t ret_val;
    uint_t       type_bmp = SAI_BRIDGE_PORT_TYPE_TO_BMP(SAI_BRIDGE_PORT_TYPE_PORT);
    ret_val = sai_vlan_cache_init();
    if(ret_val != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR("Vlan cache init failed");
        return ret_val;
    }
    ret_val = sai_vlan_npu_api_get()->vlan_init();
    if(ret_val != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR("Vlan NPU init failed");
        return ret_val;
    }
    ret_val = sai_l2_default_vlan_init(SAI_INIT_DEFAULT_VLAN);
    if(ret_val != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR("Default Vlan init failed");
        return ret_val;
    }

    ret_val = sai_bridge_port_event_cb_register(SAI_MODULE_VLAN, type_bmp,
                                                sai_vlan_bridge_port_notif_handler);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_l2_bulk_create_vlan_member(
        sai_object_id_t switch_id,
        uint32_t object_count,
        const uint32_t *attr_count,
        const sai_attribute_t **attrs,
        sai_bulk_op_error_mode_t type,
        sai_object_id_t *object_id,
        sai_status_t *object_statuses)
{
    return SAI_STATUS_NOT_SUPPORTED;
}

sai_status_t sai_l2_bulk_remove_vlan_member(
        uint32_t object_count,
        const sai_object_id_t *object_id,
        sai_bulk_op_error_mode_t type,
        sai_status_t *object_statuses)
{
    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_vlan_api_t sai_vlan_method_table =
{
    sai_l2_create_vlan,
    sai_l2_remove_vlan,
    sai_l2_set_vlan_attribute,
    sai_l2_get_vlan_attribute,
    sai_l2_create_vlan_member,
    sai_l2_remove_vlan_member,
    sai_l2_set_vlan_member_attribute,
    sai_l2_get_vlan_member_attribute,
    sai_l2_bulk_create_vlan_member,
    sai_l2_bulk_remove_vlan_member,
    sai_l2_get_vlan_stats,
    sai_l2_clear_vlan_stats,
};

sai_vlan_api_t  *sai_vlan_api_query (void)
{
    return (&sai_vlan_method_table);
}
