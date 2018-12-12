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
* @file sai_hostintf.c
*
* @brief This file contains implementations SAI Host Interface functions.
*
*************************************************************************/
#include "saitypes.h"
#include "saistatus.h"
#include "saihostif.h"
#include "sai_hostif_main.h"
#include "sai_hostif_api.h"
#include "sai_hostif_common.h"
#include "sai_npu_hostif.h"
#include "sai_gen_utils.h"
#include "sai_oid_utils.h"
#include "sai_common_infra.h"
#include "sai_switch_utils.h"

#include "std_type_defs.h"
#include "std_assert.h"
#include "std_struct_utils.h"
#include "std_llist.h"
#include "std_rbtree.h"
#include "std_error_codes.h"

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

static const dn_sai_hostif_pkt_attr_property_t packet_attrs[] = {
    { SAI_HOSTIF_PACKET_ATTR_HOSTIF_TRAP_ID, false, false},
    { SAI_HOSTIF_PACKET_ATTR_INGRESS_PORT, false, false},
    { SAI_HOSTIF_PACKET_ATTR_INGRESS_LAG, false, false},
    { SAI_HOSTIF_PACKET_ATTR_HOSTIF_TX_TYPE, true, true},
    { SAI_HOSTIF_PACKET_ATTR_EGRESS_PORT_OR_LAG, true, false}
};

static const dn_sai_hostif_trap_group_attr_property_t trap_group_attrs[] = {
    { SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE, false, true},
    { SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE, false, true},
    { SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER, false, true}
};

static const dn_sai_hostif_trap_attr_property_t trap_attrs[] = {
    {SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION, dn_sai_hostif_validate_action},
    {SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY, NULL},
    {SAI_HOSTIF_TRAP_ATTR_EXCLUDE_PORT_LIST, dn_sai_hostif_validate_portlist},
    {SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP, dn_sai_hostif_validate_trapgroup_oid}
};

#define DN_SAI_HOSTIF_SFLOW_QUEUE   (1)
static sai_status_t sai_create_hostif_trap_group(sai_object_id_t *trap_group_id,
                                                 _In_ sai_object_id_t switch_id,
                                                 uint_t attr_count,
                                                 const sai_attribute_t *attr_list);

static sai_status_t sai_hostif_create_sflow_trap();
static dn_sai_hostintf_info_t g_hostif_info = {0};

dn_sai_hostintf_info_t * dn_sai_hostintf_get_info()
{
    return &g_hostif_info;
}

sai_status_t sai_hostintf_init(void)
{
    uint_t index = 0;
    uint_t attr_count = 0;
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_object_id_t default_trap_group = SAI_NULL_OBJECT_ID;
    sai_attribute_t attr_list[DN_HOSTIF_DEFAULT_TRAP_GROUP_ATTRS];

    memset(&g_hostif_info, 0, sizeof(dn_sai_hostintf_info_t));
    memset(&attr_list, 0, sizeof(attr_list));

    SAI_HOSTIF_LOG_INFO("Initializing hostif");
    g_hostif_info.max_pkt_attrs = sizeof(packet_attrs)
                         /sizeof(packet_attrs[0]);
    g_hostif_info.max_trap_group_attrs = sizeof(trap_group_attrs)
                                /sizeof(trap_group_attrs[0]);
    g_hostif_info.max_trap_attrs = sizeof(trap_attrs)
                          /sizeof(trap_attrs[0]);

    for(index=0; index < g_hostif_info.max_pkt_attrs; index++) {
        if(packet_attrs[index].mandatory_on_send) {
             g_hostif_info.mandatory_send_pkt_attr_count++;
        }
    }

    for(index=0; index < g_hostif_info.max_trap_group_attrs; index++) {
        if(trap_group_attrs[index].mandatory_in_create) {
            g_hostif_info.mandatory_trap_group_attr_count++;
        }
    }

    rc = sai_hostif_npu_api_get()->npu_hostif_init();
    if (rc != SAI_STATUS_SUCCESS) {
        SAI_HOSTIF_LOG_CRIT("Hostif initialization failed at NPU");
        return SAI_STATUS_UNINITIALIZED;
    }

    do {
        g_hostif_info.trap_tree = std_rbtree_create_simple("trap_tree",
                                STD_STR_OFFSET_OF(dn_sai_trap_node_t, key),
                                STD_STR_SIZE_OF(dn_sai_trap_node_t,key));
        if (NULL == g_hostif_info.trap_tree) {
            SAI_HOSTIF_LOG_CRIT("Failed to allocate memory for trap database");
            rc = SAI_STATUS_UNINITIALIZED;
            break;
        }

        g_hostif_info.trap_group_tree = std_rbtree_create_simple("trap_group_tree",
                                      STD_STR_OFFSET_OF(dn_sai_trap_group_node_t, key),
                                      STD_STR_SIZE_OF(dn_sai_trap_group_node_t,key));
        if (NULL == g_hostif_info.trap_group_tree) {
            SAI_HOSTIF_LOG_CRIT("Failed to allocate memory for trap group database");
            rc = SAI_STATUS_UNINITIALIZED;
            break;
        }

        g_hostif_info.trap_group_bitmap = std_bitmap_create_array(
                                                      DN_HOSTIF_MAX_TRAP_GROUPS);
        if (NULL == g_hostif_info.trap_group_bitmap) {
            SAI_HOSTIF_LOG_CRIT("Failed to initialize trap group id generator");
            rc = SAI_STATUS_UNINITIALIZED;
            break;
        }
    } while(0);

    if (rc != SAI_STATUS_SUCCESS) {
        if (g_hostif_info.trap_tree != NULL) {
            std_rbtree_destroy(g_hostif_info.trap_tree);
        }

        if (g_hostif_info.trap_group_tree != NULL) {
            std_rbtree_destroy(g_hostif_info.trap_group_tree);
        }
    } else {

        /*Creating default read only trap group*/
        attr_list[attr_count].id = SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE;
        attr_list[attr_count].value.booldata = true;
        attr_count++;

        attr_list[attr_count].id = SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE;
        attr_list[attr_count].value.u32 = DN_SAI_HOSTIF_DEFAULT_QUEUE;
        attr_count++;

        rc = sai_create_hostif_trap_group(&default_trap_group, sai_switch_id_get(),
                                          attr_count, attr_list);
        if (rc != SAI_STATUS_SUCCESS) {
            SAI_HOSTIF_LOG_CRIT("Hostif default trap group cration failed");
            return SAI_STATUS_UNINITIALIZED;
        }

        g_hostif_info.default_trap_group_id =  default_trap_group;

        /**
         * Temporary code to take sflow packets in Q1 to CPU.
         * Will be removed once new hostintf implementation is done.
         */
        rc = sai_hostif_create_sflow_trap();

        if (rc != SAI_STATUS_SUCCESS) {
            SAI_HOSTIF_LOG_CRIT("Hostif default trap group creation for sflow failed");
            return SAI_STATUS_UNINITIALIZED;
        }

        SAI_HOSTIF_LOG_INFO("Successful hostif initialization");
    }

    return rc;
}

static sai_status_t sai_create_hostif(sai_object_id_t * hif_id,
                                      _In_ sai_object_id_t switch_id,
                                      uint_t attr_count,
                                      const sai_attribute_t *attr_list)
{
    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t sai_remove_hostif(sai_object_id_t hif_id)
{
    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t sai_set_hostif(sai_object_id_t hif_id,
                                   const sai_attribute_t *attr)
{
    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t sai_get_hostif(sai_object_id_t hif_id,
                                   uint_t attr_count,
                                   sai_attribute_t *attr_list)
{
    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t dn_sai_hostif_validate_trapgroup_attrlist(
                                        uint_t attr_count,
                                        const sai_attribute_t *attr_list,
                                        dn_sai_hostif_op_t operation)
{
    uint_t attr_idx = 0, valid_idx = 0;
    uint_t dup_index = 0, mand_attr_count = 0;
    sai_status_t rc = SAI_STATUS_FAILURE;

    STD_ASSERT(attr_list != NULL);

    SAI_HOSTIF_LOG_TRACE("Validating trap group attributes");

    if (0 == attr_count) {
        SAI_HOSTIF_LOG_ERR("Invalid trap group attribute count");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (dn_sai_check_duplicate_attr(attr_count, attr_list, &dup_index)) {
        SAI_HOSTIF_LOG_ERR("Duplicate trapgroup attribute at index %u", dup_index);
        return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                       dup_index);
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        SAI_HOSTIF_LOG_TRACE("Trap group attribute id = %u",
                             attr_list[attr_idx].id);
        for(valid_idx = 0; valid_idx < g_hostif_info.max_trap_group_attrs;
            valid_idx++) {
            if(attr_list[attr_idx].id ==
               trap_group_attrs[valid_idx].attr_id) {

                if (DN_SAI_HOSTIF_CREATE == operation) {
                    trap_group_attrs[valid_idx].mandatory_in_create ?
                        mand_attr_count++ : mand_attr_count;
                } else if (DN_SAI_HOSTIF_SET == operation) {
                    if(!trap_group_attrs[valid_idx].valid_in_set) {
                        SAI_HOSTIF_LOG_ERR("Invalid trap group attribute %u for "
                                           "set operation", attr_list[attr_idx].id);
                        return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                                       attr_idx);
                    }
                }

                rc = sai_hostif_npu_api_get()->npu_validate_trapgroup(
                                                            &attr_list[attr_idx],
                                                            operation);
                if (SAI_STATUS_INVALID_ATTR_VALUE_0 == rc) {
                    SAI_HOSTIF_LOG_ERR("Failed npu validation for attribute %u",
                                       attr_list[attr_idx].id);
                    return sai_get_indexed_ret_val(rc, attr_idx);
                } else if (SAI_STATUS_ATTR_NOT_SUPPORTED_0 == rc) {
                    SAI_HOSTIF_LOG_ERR("Unsupported attribute %u",
                                       attr_list[attr_idx].id);
                    return sai_get_indexed_ret_val(rc, attr_idx);
                }else if (rc != SAI_STATUS_SUCCESS) {
                    SAI_HOSTIF_LOG_ERR("Failed npu validation for attribute %u",
                                       attr_list[attr_idx].id);
                    return rc;
                }
                SAI_HOSTIF_LOG_TRACE("Passed npu validation for attribute %u",
                                     attr_list[attr_idx].id);
                break;
            }
        }

        if (valid_idx == g_hostif_info.max_trap_group_attrs) {
            SAI_HOSTIF_LOG_ERR("Unknown trap group attribute %u",
                               attr_list[attr_idx].id);
            return sai_get_indexed_ret_val(SAI_STATUS_UNKNOWN_ATTRIBUTE_0,
                                           attr_idx);
        }
    }

    if (DN_SAI_HOSTIF_CREATE == operation) {
        if (mand_attr_count < g_hostif_info.mandatory_trap_group_attr_count) {
            SAI_HOSTIF_LOG_ERR("Missing mandatory trap group attributes");
            return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
        }
    }
    SAI_HOSTIF_LOG_TRACE("Successful validation of trap group attributes");
    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_hostif_alloc_trapgroup(
                                   dn_sai_trap_group_node_t **trap_group,
                                   uint_t attr_count,
                                   const sai_attribute_t *attr_list)
{
    uint_t attr_idx = 0;
    int id = 0;
    sai_attr_id_t attr_id = 0;
    sai_attribute_value_t attr_val = {0};

    STD_ASSERT(trap_group != NULL);
    STD_ASSERT(attr_list != NULL);

    id = std_find_first_bit(g_hostif_info.trap_group_bitmap,
                            DN_HOSTIF_MAX_TRAP_GROUPS, 1);
    if (id < 0) {
        SAI_HOSTIF_LOG_ERR("Max trap groups %u reached",
                           DN_HOSTIF_MAX_TRAP_GROUPS);
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    SAI_HOSTIF_LOG_TRACE("Allocating memory for trap group");

    *trap_group = (dn_sai_trap_group_node_t *) calloc(1,
                                         sizeof(dn_sai_trap_group_node_t));
    if (NULL == *trap_group) {
        SAI_HOSTIF_LOG_ERR("No memory for trap group allocation");
        return SAI_STATUS_NO_MEMORY;
    }

    std_dll_init(&((*trap_group)->trap_list));

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        attr_id = attr_list[attr_idx].id;
        attr_val = attr_list[attr_idx].value;

        if (SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE == attr_id) {
            (*trap_group)->admin_state = attr_val.booldata;
        } else if (SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE == attr_id) {
            (*trap_group)->cpu_queue = attr_val.u32;
        } else if (SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER == attr_id) {
            (*trap_group)->policer_id = attr_val.oid;
        }
    }

    (*trap_group)->key.trap_group_id = sai_uoid_create(
                                         SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP, id);

    STD_BIT_ARRAY_CLR (g_hostif_info.trap_group_bitmap, id);
    SAI_HOSTIF_LOG_TRACE("Successful memory allocation for trap group "
                         "id=%"PRIu64".",(*trap_group)->key.trap_group_id);
    return SAI_STATUS_SUCCESS;
}

static void dn_sai_hostif_dealloc_trapgroup(
                             dn_sai_trap_group_node_t * trap_group)
{
    uint_t id = 0;
    STD_ASSERT(trap_group != NULL);

    id = sai_uoid_npu_obj_id_get(trap_group->key.trap_group_id);
    STD_BIT_ARRAY_SET(g_hostif_info.trap_group_bitmap, id);

    SAI_HOSTIF_LOG_TRACE("Deallocating trap group %"PRIu64".",
                         trap_group->key.trap_group_id);
    free(trap_group);
}

static sai_status_t sai_create_hostif_trap_group(sai_object_id_t *trap_group_id,
                                                 _In_ sai_object_id_t switch_id,
                                                 uint_t attr_count,
                                                 const sai_attribute_t *attr_list)
{
    uint_t attr_idx = 0;
    bool group_allocated = false;
    sai_status_t rc = SAI_STATUS_FAILURE;
    const sai_attribute_value_t *attr_val = NULL;
    dn_sai_trap_group_node_t *trap_group = NULL;
    uint_t cpu_queue = 0;

    STD_ASSERT(trap_group_id != NULL);
    STD_ASSERT(attr_list != NULL);

    SAI_HOSTIF_LOG_INFO("Create a new trap group(attr_count=%u)", attr_count);

    rc = dn_sai_hostif_validate_trapgroup_attrlist(attr_count, attr_list,
                                                   DN_SAI_HOSTIF_CREATE);
    if (rc != SAI_STATUS_SUCCESS) {
        SAI_HOSTIF_LOG_ERR("Failed validation of create trap group attributes");
        return rc;
    }

    rc = sai_find_attr_in_attrlist(SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE,attr_count,
                                   attr_list,&attr_idx, &attr_val);

    if(rc != SAI_STATUS_SUCCESS) {
        cpu_queue = DN_SAI_HOSTIF_DEFAULT_QUEUE;
        SAI_HOSTIF_LOG_TRACE("Taking default cpu queue %u", cpu_queue);
    } else {
        cpu_queue = attr_val->u32;
        SAI_HOSTIF_LOG_TRACE("Trap group cpu queue %u", cpu_queue);
    }

    sai_hostif_lock();
    do {
        trap_group = dn_sai_hostif_find_trapgroup_by_queue(cpu_queue);
        if (trap_group != NULL) {
            SAI_HOSTIF_LOG_ERR("Trap group id %"PRIu64" with same cpu queue %u"
                               " already exists", trap_group->key.trap_group_id,
                               cpu_queue);
            rc = SAI_STATUS_ITEM_ALREADY_EXISTS;
            break;
        }

        rc = dn_sai_hostif_alloc_trapgroup(&trap_group, attr_count, attr_list);
        if (rc != SAI_STATUS_SUCCESS) {
            SAI_HOSTIF_LOG_ERR("Failed allocation for trap group");
            break;
        }

        group_allocated = true;
        rc = dn_sai_hostif_add_trapgroup(g_hostif_info.trap_group_tree,trap_group);
        if (rc != SAI_STATUS_SUCCESS) {
            SAI_HOSTIF_LOG_ERR("Failed to add trap group %"PRIu64" to "
                               "database",trap_group->key.trap_group_id);
            break;
        }
    } while(0);

    if (rc != SAI_STATUS_SUCCESS) {
        if (group_allocated) {
            dn_sai_hostif_dealloc_trapgroup(trap_group);
        }
    } else {
        *trap_group_id = trap_group->key.trap_group_id;
        SAI_HOSTIF_LOG_INFO("Successful creation of trap group %"PRIu64".",
                             *trap_group_id);
    }
    sai_hostif_unlock();

    return rc;
}

static sai_status_t sai_remove_hostif_trap_group(sai_object_id_t trap_group_id)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    dn_sai_trap_group_node_t *trap_group = NULL;

    SAI_HOSTIF_LOG_INFO("Remove trap group %"PRIu64".",trap_group_id);

    if(!sai_is_obj_id_hostif_trap_group(trap_group_id)) {
        SAI_HOSTIF_LOG_ERR("Invalid trap group object type id=%"PRIu64".",
                           trap_group_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_hostif_lock();
    do {
        trap_group = dn_sai_hostif_find_trapgroup(g_hostif_info.trap_group_tree,
                                                  trap_group_id);
        if (NULL == trap_group) {
            SAI_HOSTIF_LOG_ERR("Trap group %"PRIu64" not present",
                               trap_group_id);
            rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if (trap_group->trap_count > 0) {
            SAI_HOSTIF_LOG_ERR("Trap group %"PRIu64" is still referenced by "
                             "%u traps", trap_group_id, trap_group->trap_count);
            rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        if (g_hostif_info.default_trap_group_id == trap_group_id) {
            SAI_HOSTIF_LOG_ERR("Trap group %"PRIu64" is the default trap group",
                                trap_group_id);
            rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        if (NULL == dn_sai_hostif_remove_trapgroup(
                             g_hostif_info.trap_group_tree,trap_group)) {
            SAI_HOSTIF_LOG_ERR("Failed to remove trap group %"PRIu64" from "
                               "database",trap_group->key.trap_group_id);
            rc = SAI_STATUS_FAILURE;
            break;
        }

        rc = SAI_STATUS_SUCCESS;
        dn_sai_hostif_dealloc_trapgroup(trap_group);

        SAI_HOSTIF_LOG_INFO("Successful removal of trap group %"PRIu64".",
                             trap_group_id);

    } while(0);
    sai_hostif_unlock();

    return rc;
}

static sai_status_t dn_sai_hostif_update_trapgroup(
                                   dn_sai_trap_group_node_t * trap_group,
                                   const sai_attribute_t *attr)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    dn_sai_trap_node_t *trap_node = NULL;

    STD_ASSERT(trap_group != NULL);
    STD_ASSERT(attr != NULL);

    trap_node = (dn_sai_trap_node_t *)std_dll_getfirst(
                                        &trap_group->trap_list);
    while(trap_node != NULL) {
        rc = sai_hostif_npu_api_get()->npu_update_trapgroup(
                                             trap_node, trap_group,
                                             attr);
        if (rc != SAI_STATUS_SUCCESS) {
            SAI_HOSTIF_LOG_ERR("Failed to update trap %u in trap group "
                               "%"PRIu64" at NPU",trap_node->key.trap_id,
                               trap_group->key.trap_group_id);
            return rc;
        }
        trap_node = (dn_sai_trap_node_t *)std_dll_getnext(
                             &trap_group->trap_list, (std_dll *)trap_node);
    }

    if (SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE == attr->id) {
        SAI_HOSTIF_LOG_TRACE("Updating trap group 0x%"PRIx64" with new "
                             "admin state %u",trap_node->key.trap_id, attr->value.booldata);
        trap_group->admin_state = attr->value.booldata;
    } else if (SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE == attr->id) {
        SAI_HOSTIF_LOG_TRACE("Updating trap group 0x%"PRIx64" with new "
                             "cpu queue %u",trap_node->key.trap_id, attr->value.u32);
        trap_group->cpu_queue = attr->value.u32;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_set_hostif_trap_group(sai_object_id_t trap_group_id,
                                              const sai_attribute_t *attr)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    dn_sai_trap_group_node_t *trap_group = NULL;

    STD_ASSERT(attr != NULL);

    SAI_HOSTIF_LOG_INFO("Set trap group attribute %d",attr->id);

    if(!sai_is_obj_id_hostif_trap_group(trap_group_id)) {
        SAI_HOSTIF_LOG_ERR("Invalid trap group object type id=%"PRIu64".",
                           trap_group_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    rc = dn_sai_hostif_validate_trapgroup_attrlist(1, attr,
                                                   DN_SAI_HOSTIF_SET);
    if (rc != SAI_STATUS_SUCCESS) {
        SAI_HOSTIF_LOG_ERR("Failed to validate trap group for set operation");
        return rc;
    }

    sai_hostif_lock();
    do {
        trap_group = dn_sai_hostif_find_trapgroup(g_hostif_info.trap_group_tree,
                                                  trap_group_id);
        if (NULL == trap_group) {
            SAI_HOSTIF_LOG_ERR("Trap group %"PRIu64" not present",
                               trap_group_id);
            rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if (SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE == attr->id) {
            if (trap_group->admin_state == attr->value.booldata) {
                SAI_HOSTIF_LOG_TRACE("No change in admin trap group state");
                rc = SAI_STATUS_SUCCESS;
                break;
            }

        } else if (SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE == attr->id) {
            /*@TODO use oid once queue is represented as object id in SAI*/
            if (trap_group->cpu_queue == attr->value.u32) {
                SAI_HOSTIF_LOG_TRACE("No change in trap group cpu queue");
                rc = SAI_STATUS_SUCCESS;
                break;
            } else {
                if (dn_sai_hostif_find_trapgroup_by_queue(attr->value.u32)) {
                    SAI_HOSTIF_LOG_ERR("Trap group id %"PRIu64" with same cpu queue %u"
                                       " already exists", trap_group->key.trap_group_id,
                                       attr->value.u32);
                    rc = SAI_STATUS_ITEM_ALREADY_EXISTS;
                    break;
                }
            }
        } else {
            SAI_HOSTIF_LOG_ERR("Trap group attribute %u not supported",attr->id);
            rc = SAI_STATUS_ATTR_NOT_SUPPORTED_0;
            break;
        }
        rc = dn_sai_hostif_update_trapgroup(trap_group, attr);
    } while(0);
    sai_hostif_unlock();

    if (SAI_STATUS_SUCCESS == rc) {
        SAI_HOSTIF_LOG_INFO("Successful set trap group attribute %d operation",attr->id);
    }

    return rc;
}

static sai_status_t sai_get_hostif_trap_group(sai_object_id_t trap_group_id,
                                              uint_t attr_count,
                                              sai_attribute_t *attr_list)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    dn_sai_trap_group_node_t *trap_group = NULL;
    uint_t attr_idx = 0;

    STD_ASSERT(attr_list != NULL);

    SAI_HOSTIF_LOG_INFO("Get trap group attributes (attribute count = %u)", attr_count);

    if (0 == attr_count) {
        SAI_HOSTIF_LOG_ERR("Invalid attribute count %u for get trap group", attr_count);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    rc = dn_sai_hostif_validate_trapgroup_attrlist(attr_count, attr_list,
                                                   DN_SAI_HOSTIF_GET);
    if (rc != SAI_STATUS_SUCCESS) {
        SAI_HOSTIF_LOG_ERR("Failed to validate trap group for get operation");
        return rc;
    }

    sai_hostif_lock();
    do {
        trap_group = dn_sai_hostif_find_trapgroup(g_hostif_info.trap_group_tree,
                                                  trap_group_id);
        if (NULL == trap_group) {
            SAI_HOSTIF_LOG_ERR("Trap group %"PRIu64" not present",
                               trap_group_id);
            rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
            if (SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE == attr_list[attr_idx].id) {
                SAI_HOSTIF_LOG_TRACE("Admin state during get oper for "
                                     "trap group %"PRIu64" is %u", trap_group_id,
                                     trap_group->admin_state);
                attr_list[attr_idx].value.booldata = trap_group->admin_state;
            } else if (SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE == attr_list[attr_idx].id) {
                SAI_HOSTIF_LOG_TRACE("Cpu queue during get oper for "
                                     "trap group %"PRIu64" is %u", trap_group_id,
                                     trap_group->cpu_queue);
                attr_list[attr_idx].value.u32 = trap_group->cpu_queue;
            }
        }
    } while(0);
    sai_hostif_unlock();

    SAI_HOSTIF_LOG_INFO("Successful get trap group attributes operation");

    return rc;

}

static sai_status_t dn_sai_hostif_validate_trap(
                                 sai_hostif_trap_type_t trapid,
                                 const sai_attribute_t *attr,
                                 dn_sai_hostif_op_t operation)
{
    uint_t attr_idx = 0;
    bool check_attr_value = false;
    sai_status_t rc = SAI_STATUS_FAILURE;
    STD_ASSERT(attr != NULL);

    if (!dn_sai_hostif_is_valid_trap_id(trapid)) {
        SAI_HOSTIF_LOG_ERR("Invalid trapid %d", trapid);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    check_attr_value = (DN_SAI_HOSTIF_SET == operation);

    for(attr_idx = 0; attr_idx < g_hostif_info.max_trap_attrs; attr_idx++) {
       if (trap_attrs[attr_idx].attr_id == attr->id) {
           /*Validate the attribute values*/
           if ((trap_attrs[attr_idx].check_attr_func != NULL) &&
               check_attr_value) {
               rc = trap_attrs[attr_idx].check_attr_func(attr);
               if (rc != SAI_STATUS_SUCCESS) {
                   return rc;
               }
           }
           return sai_hostif_npu_api_get()->npu_validate_trap(trapid,
                                                              attr, operation);

       }
    }

    SAI_HOSTIF_LOG_ERR("Unknown trap attribute id %u", attr->id);
    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
}

static dn_sai_trap_node_t * dn_sai_hostif_add_trap_node(
                                        sai_hostif_trap_type_t trapid)
{
    dn_sai_trap_node_t *trap_node = NULL;
    dn_sai_trap_group_node_t *trap_group = NULL;
    t_std_error rc = STD_ERR_OK;

    trap_node = calloc(1, sizeof(dn_sai_trap_node_t));
    if (NULL == trap_node) {
        SAI_HOSTIF_LOG_ERR("No memory for trap node allocation trapid %d",
                           trapid);
        return NULL;
    }

    do {
        trap_node->trap_action = dn_sai_hostif_get_default_action(trapid);
        trap_node->trap_prio = DN_HOSTIF_DEFAULT_MIN_PRIO;
        trap_node->key.trap_id = trapid;
        rc = std_rbtree_insert(g_hostif_info.trap_tree, trap_node);
        if (rc != STD_ERR_OK) {
            SAI_HOSTIF_LOG_ERR("Unable to add trap %u to trap database", trapid);
            break;
        }

        if(SAI_NULL_OBJECT_ID != g_hostif_info.default_trap_group_id) {
            trap_group = dn_sai_hostif_find_trapgroup(g_hostif_info.trap_group_tree,
                                                      g_hostif_info.default_trap_group_id);
            STD_ASSERT(trap_group != NULL);
            dn_sai_hostif_add_trap_to_trapgroup(trap_node, trap_group);
            trap_node->trap_group = g_hostif_info.default_trap_group_id;
        } else {
            trap_node->trap_group = SAI_NULL_OBJECT_ID;
        }
    } while(0);

    if (rc != STD_ERR_OK) {
        free(trap_node);
        trap_node = NULL;
    }

    return trap_node;
}

static sai_status_t dn_sai_hostif_update_trap_node(
                              dn_sai_trap_node_t *trap_node,
                              const sai_attribute_t *attr)
{
    dn_sai_trap_group_node_t *old_group = NULL, *new_group = NULL;
    sai_object_id_t *list = NULL;

    STD_ASSERT(trap_node != NULL);
    STD_ASSERT(attr != NULL);

    SAI_HOSTIF_LOG_TRACE("Updating trap node %d with attribute %u",
                         trap_node->key.trap_id, attr->id);

    if (SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION == attr->id) {
        SAI_HOSTIF_LOG_TRACE("New trap packet action %d",attr->value.s32);
        trap_node->trap_action = attr->value.s32;
    } else if (SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY == attr->id) {
        SAI_HOSTIF_LOG_TRACE("New trap priority %u",attr->value.u32);
        trap_node->trap_prio = attr->value.u32;
    } else if (SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP == attr->id) {
        old_group = dn_sai_hostif_find_trapgroup(g_hostif_info.trap_group_tree,
                                                 trap_node->trap_group);
        if (old_group != NULL) {
            dn_sai_hostif_remove_trap_to_trapgroup(trap_node, old_group);
        }

        if(attr->value.oid != SAI_NULL_OBJECT_ID) {
            new_group = dn_sai_hostif_find_trapgroup(g_hostif_info.trap_group_tree,
                                                     attr->value.oid);
            if (NULL == new_group) {
                SAI_HOSTIF_LOG_CRIT("Retrieval of the new trap group failed");
            }
            /*new group must be present as we have already validated*/
            STD_ASSERT(new_group != NULL);
            dn_sai_hostif_add_trap_to_trapgroup(trap_node, new_group);
        }
        SAI_HOSTIF_LOG_TRACE("New trap group %"PRIu64".",attr->value.oid);
        trap_node->trap_group = attr->value.oid;

    } else if (SAI_HOSTIF_TRAP_ATTR_EXCLUDE_PORT_LIST == attr->id ) {
         list = calloc(attr->value.objlist.count, sizeof(sai_object_id_t));
         if (NULL == list) {
             SAI_HOSTIF_LOG_ERR("No memory for port object list count %u",
                                attr->value.objlist.count);
             return SAI_STATUS_NO_MEMORY;
         }

         memcpy(list, attr->value.objlist.list,
                attr->value.objlist.count * sizeof(sai_object_id_t));
         /*free old list*/
         free(trap_node->port_list.list);
         trap_node->port_list.list = list;
         trap_node->port_list.count = attr->value.objlist.count;
    }

    SAI_HOSTIF_LOG_TRACE("Updated trap node %d successfully", trap_node->key.trap_id);
    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_hostif_update_trap(
                              dn_sai_trap_node_t *trap_node,
                              const sai_attribute_t *attr,
                              bool is_new_trap)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    dn_sai_trap_group_node_t *trap_group = NULL;

    STD_ASSERT(trap_node != NULL);
    STD_ASSERT(attr != NULL);

    SAI_HOSTIF_LOG_TRACE("Update trap %d",trap_node->key.trap_id);

    if(SAI_NULL_OBJECT_ID != trap_node->trap_group) {
        /*Retrive existing trap group*/
        trap_group = dn_sai_hostif_find_trapgroup(g_hostif_info.trap_group_tree,
                                                  trap_node->trap_group);
        if (NULL == trap_group) {
            SAI_HOSTIF_LOG_CRIT("Retrieval of existing trap group failed");
        }
        STD_ASSERT(trap_group != NULL);
    }

    if (SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION == attr->id) {
        if((trap_node->trap_action == attr->value.s32) && (!is_new_trap)){
            SAI_HOSTIF_LOG_TRACE("No change in packet action %d", attr->value.s32);
            return SAI_STATUS_SUCCESS;
        }
    } else if (SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY == attr->id) {
        if((trap_node->trap_prio == attr->value.u32) && (!is_new_trap)) {
            SAI_HOSTIF_LOG_TRACE("No change in trap priority %d", attr->value.u32);
            return SAI_STATUS_SUCCESS;
        }
    } else if (SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP == attr->id) {
        if ((trap_node->trap_group == attr->value.oid) && (!is_new_trap)) {
            SAI_HOSTIF_LOG_TRACE("No change in trap group %"PRIu64"",
                                 attr->value.oid);
            return SAI_STATUS_SUCCESS;
        }
        /*New trap group id*/
        if (SAI_NULL_OBJECT_ID != attr->value.oid) {
            trap_group = dn_sai_hostif_find_trapgroup(g_hostif_info.trap_group_tree,
                                                      attr->value.oid);
        } else {
            trap_group = NULL;
        }
    }

    rc = sai_hostif_npu_api_get()->npu_set_trap(trap_node, trap_group, attr);
    if (rc != SAI_STATUS_SUCCESS) {
        SAI_HOSTIF_LOG_ERR("Failed to set trap %d in NPU", trap_node->key.trap_id);
        return rc;
    }

    return dn_sai_hostif_update_trap_node(trap_node, attr);
}

static sai_status_t sai_set_hostif_trap(sai_object_id_t trapid,
                                        const sai_attribute_t *attr)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    dn_sai_trap_node_t *trap_node = NULL;
    bool is_new_trap = false;

    STD_ASSERT(attr != NULL);

    SAI_HOSTIF_LOG_INFO("Set trap 0x%"PRIx64" attribute %d", trapid, attr->id);

    sai_hostif_lock();
    do {
        rc = dn_sai_hostif_validate_trap(trapid, attr, DN_SAI_HOSTIF_SET);
        if (rc != SAI_STATUS_SUCCESS) {
            SAI_HOSTIF_LOG_ERR("Failed to validate attribute id %u for trap 0x%"PRIx64"",
                               attr->id, trapid);
            break;
        }

        trap_node = dn_sai_hostif_find_trap_node(g_hostif_info.trap_tree,
                                                 trapid);
        if (NULL == trap_node) {
            /*Create and add the node in the global trap node tree*/
            trap_node = dn_sai_hostif_add_trap_node(trapid);
            if (NULL == trap_node ) {
                SAI_HOSTIF_LOG_ERR("Trap 0x%"PRIx64" could not be added", trapid);
                rc = SAI_STATUS_FAILURE;
                break;
            }
            is_new_trap = true;
        }
        rc = dn_sai_hostif_update_trap(trap_node, attr, is_new_trap);
    } while(0);
    sai_hostif_unlock();

    if (SAI_STATUS_SUCCESS == rc) {
        SAI_HOSTIF_LOG_INFO("Successful set trap 0x%"PRIx64" attribute %u", trapid, attr->id);
    }

    return rc;
}

static sai_status_t sai_get_hostif_trap(sai_object_id_t trapid,
                                        uint_t attr_count,
                                        sai_attribute_t *attr_list)
{
    uint_t attr_idx = 0;
    sai_status_t rc = SAI_STATUS_FAILURE;
    dn_sai_trap_node_t *trap_node = NULL;

    STD_ASSERT(attr_list != NULL);

    SAI_HOSTIF_LOG_INFO("Get trap 0x%"PRIx64" attributes count = %u", trapid, attr_count);

    if (0 == attr_count) {
        SAI_HOSTIF_LOG_ERR("Invalid trap attribute count %u", attr_count);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_hostif_lock();
    do {
        trap_node = dn_sai_hostif_find_trap_node(g_hostif_info.trap_tree, trapid);
        if (NULL == trap_node) {
            SAI_HOSTIF_LOG_ERR("Trap 0x%"PRIx64" could not be found for get operation", trapid);
            rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {
            rc = dn_sai_hostif_validate_trap(trapid, &attr_list[attr_idx],
                                             DN_SAI_HOSTIF_GET);
            if (rc != SAI_STATUS_SUCCESS) {
                SAI_HOSTIF_LOG_ERR("Failed to validate attribute id %u for trap 0x%"PRIx64"",
                                   attr_list[attr_idx].id, trapid);
                break;
            }
            if (SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION == attr_list[attr_idx].id) {
                attr_list[attr_idx].value.s32 = trap_node->trap_action;
            } else if (SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY == attr_list[attr_idx].id) {
                attr_list[attr_idx].value.u32 = trap_node->trap_prio;
            } else if (SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP == attr_list[attr_idx].id) {
                attr_list[attr_idx].value.oid = trap_node->trap_group;
            } else if (SAI_HOSTIF_TRAP_ATTR_EXCLUDE_PORT_LIST == attr_list[attr_idx].id) {
                if (attr_list[attr_idx].value.objlist.count <
                    trap_node->port_list.count) {
                    attr_list[attr_idx].value.objlist.count =
                        trap_node->port_list.count;
                    rc = SAI_STATUS_BUFFER_OVERFLOW;
                    break;
                }
                memcpy(attr_list[attr_idx].value.objlist.list,trap_node->port_list.list,
                       trap_node->port_list.count * sizeof(sai_object_id_t));
                attr_list[attr_idx].value.objlist.count =
                    trap_node->port_list.count;
            }
        }
    } while(0);
    sai_hostif_unlock();

    if (SAI_STATUS_SUCCESS == rc) {
        SAI_HOSTIF_LOG_INFO("Successful get attribute operation for trap 0x%"PRIx64" ", trapid);
    }

    return rc;
}

static sai_status_t sai_set_hostif_user_defined_trap(
                            sai_object_id_t hostif_user_defined_trap_id,
                            const sai_attribute_t *attr)
{
    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t sai_get_hostif_user_defined_trap(
                            sai_object_id_t hostif_user_defined_trap_id,
                            uint_t attr_count, sai_attribute_t *attr_list)
{
    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t sai_hostif_validate_pkt_attrlist(
                    uint_t attr_count, const sai_attribute_t *attr_list)
{
    uint_t attr_idx = 0, pkt_attr_idx = 0, dup_index = 0;
    uint_t cur_mand_attr_count = 0, mand_attr_count = 0;

    STD_ASSERT(attr_list != NULL);

    if (0 == attr_count) {
        SAI_HOSTIF_LOG_ERR("Invalid pkt attribute count %u", attr_count);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    mand_attr_count = g_hostif_info.mandatory_send_pkt_attr_count;

    if (dn_sai_check_duplicate_attr(attr_count, attr_list, &dup_index)) {
        SAI_HOSTIF_LOG_ERR("Duplicate pkt attrinute at index %u", dup_index);
        return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                       dup_index);
    }

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        SAI_HOSTIF_LOG_TRACE("Validate pkt attribute %u", attr_list[attr_idx].id);
        for (pkt_attr_idx = 0; pkt_attr_idx < g_hostif_info.max_pkt_attrs;
             pkt_attr_idx++) {
            if (attr_list[attr_idx].id == packet_attrs[pkt_attr_idx].attr_id) {

                packet_attrs[pkt_attr_idx].mandatory_on_send ?
                    cur_mand_attr_count++ : cur_mand_attr_count;

                if (!packet_attrs[pkt_attr_idx].valid_on_send) {
                    SAI_HOSTIF_LOG_ERR("Invalid attribute %u on pkt send",
                                       attr_list[attr_idx].id);
                    return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                                   attr_idx);
                }
                break;
            }
        }

        if (pkt_attr_idx == g_hostif_info.max_pkt_attrs) {
            SAI_HOSTIF_LOG_ERR("Unknown attribute %u on pkt send",
                               attr_list[attr_idx].id);
            return sai_get_indexed_ret_val(SAI_STATUS_UNKNOWN_ATTRIBUTE_0,
                                           attr_idx);
        }
    }

    if (cur_mand_attr_count < mand_attr_count) {
        SAI_HOSTIF_LOG_ERR("Missing mandatory attributes on packet send");
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_recv_hostif_packet(sai_object_id_t  hif_id,
                                    void *buffer, sai_size_t *buffer_size,
                                    uint_t *attr_count, sai_attribute_t *attr_list)
{
    /* This SAI API is meant for receiving  pkts from device fds which some
     * NPUs support */
    return SAI_STATUS_NOT_SUPPORTED;
}

static sai_status_t sai_send_hostif_packet(sai_object_id_t  hif_id,
                                    void *buffer, sai_size_t buffer_size,
                                    uint_t attr_count,
                                    sai_attribute_t *attr_list)
{
    sai_status_t rc = SAI_STATUS_FAILURE;

    STD_ASSERT(buffer != NULL);
    STD_ASSERT(attr_list != NULL);

    SAI_HOSTIF_LOG_TRACE("Transmitting packet of size %lu attribute count = %u",
                         buffer_size, attr_count);

    if (0 == buffer_size) {
        SAI_HOSTIF_LOG_ERR("Invalid buffer size %lu for pkt send", buffer_size);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    rc = sai_hostif_validate_pkt_attrlist(attr_count, attr_list);
    if(rc != SAI_STATUS_SUCCESS) {
        SAI_HOSTIF_LOG_ERR("failed validation of pkt attribute for pkt send");
        return rc;
    }

    rc = sai_hostif_npu_api_get()->npu_send_packet(buffer, buffer_size, attr_count,
                                                   attr_list);
    if(SAI_STATUS_SUCCESS == rc) {
        SAI_HOSTIF_LOG_TRACE("Successful transmission of packet size = %lu",
                             buffer_size);
    }

    return rc;
}

void sai_hostif_rx_register_callback(sai_packet_event_notification_fn rx_register_fn)
{
    sai_hostif_lock();
    sai_hostif_npu_api_get()->npu_register_packet_rx(rx_register_fn);
    sai_hostif_unlock();
}

static sai_status_t sai_hostif_create_sflow_trap()
{
    uint_t attr_count = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;
    sai_object_id_t sflow_trap_group = SAI_NULL_OBJECT_ID;
    sai_attribute_t attr_list[DN_HOSTIF_DEFAULT_TRAP_GROUP_ATTRS];
    sai_attribute_t set_attr;

    memset(&attr_list, 0, sizeof(attr_list));

    attr_list[attr_count].id = SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE;
    attr_list[attr_count].value.u32 = DN_SAI_HOSTIF_SFLOW_QUEUE;
    attr_count++;

    rc = sai_create_hostif_trap_group(&sflow_trap_group, sai_switch_id_get(),
                                      attr_count, attr_list);
    if (rc != SAI_STATUS_SUCCESS) {
        SAI_HOSTIF_LOG_CRIT("Hostif default trap group cration failed");
        return SAI_STATUS_UNINITIALIZED;
    }

    set_attr.id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
    set_attr.value.oid = sflow_trap_group;

    rc = sai_set_hostif_trap(SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET, &set_attr);

    if(rc != SAI_STATUS_SUCCESS) {
        SAI_HOSTIF_LOG_CRIT("Hostif samplepacket trap setting failed");
        return SAI_STATUS_UNINITIALIZED;
    }

    return rc;
}

static sai_hostif_api_t sai_hostintf_method_table =
{
    sai_create_hostif,
    sai_remove_hostif,
    sai_set_hostif,
    sai_get_hostif,
    NULL,
    NULL,
    NULL,
    NULL,
    sai_create_hostif_trap_group,
    sai_remove_hostif_trap_group,
    sai_set_hostif_trap_group,
    sai_get_hostif_trap_group,
    NULL,
    NULL,
    sai_set_hostif_trap,
    sai_get_hostif_trap,
    NULL,
    NULL,
    sai_set_hostif_user_defined_trap,
    sai_get_hostif_user_defined_trap,
    sai_recv_hostif_packet,
    sai_send_hostif_packet
};

sai_hostif_api_t *sai_hostif_api_query(void)
{
    return &sai_hostintf_method_table;
}
