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
 * @file sai_vm_acl_table.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI ACL Table object in VM environment.
 */

#include "sai_vm_acl_util.h"
#include "sai_vm_defs.h"
#include "sai_acl_db_api.h"
#include "sai_acl_npu_api.h"
#include "sai_acl_type_defs.h"
#include "sai_acl_utils.h"
#include "sai_qos_common.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_assert.h"
#include "inttypes.h"

static sai_status_t sai_npu_create_acl_table (sai_acl_table_t *acl_table)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT (acl_table != NULL);

    SAI_ACL_LOG_TRACE ("NPU ACL Table Creation API, Table ID: 0x%"PRIx64".",
                       acl_table->table_key.acl_table_id);

    /* Insert ACL Table object record to DB. */
    sai_rc = sai_acl_table_create_db_entry (acl_table);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Error inserting entry to DB for ACL Table Object "
                         "ID: 0x%" PRIx64 ".",
                         acl_table->table_key.acl_table_id);
    }

    return sai_rc;
}

static sai_status_t sai_npu_delete_acl_table (sai_acl_table_t *acl_table)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT (acl_table != NULL);

    SAI_ACL_LOG_TRACE ("NPU ACL Table Deletion API, Table ID: 0x%"PRIx64".",
                       acl_table->table_key.acl_table_id);

     /* Remove ACL Table object record from DB. */
    sai_rc = sai_acl_table_delete_db_entry (acl_table);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Error removing entry from DB for ACL Table "
                         "Object ID: 0x%" PRIx64 ".",
                         acl_table->table_key.acl_table_id);
    }

     return sai_rc;
}

static sai_status_t sai_npu_validate_acl_table_field (
sai_acl_table_t *acl_table)
{
    STD_ASSERT (acl_table != NULL);

    SAI_ACL_LOG_TRACE ("NPU ACL Table Qualifier validation, Table ID: "
                       "0x%"PRIx64".", acl_table->table_key.acl_table_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_update_policer_acl_rule(dn_sai_qos_policer_t *p_policer,
                                             dn_sai_qos_policer_t *p_policer_new)
{
    STD_ASSERT(p_policer != NULL);
    STD_ASSERT(p_policer_new != NULL);
    return SAI_STATUS_SUCCESS;
}

void sai_npu_acl_dump_table(const sai_acl_table_t *acl_table)
{
    STD_ASSERT (acl_table != NULL);

    return;
}

sai_status_t sai_npu_get_acl_attribute(sai_attribute_t *attr)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(attr != NULL);

    switch(attr->id) {
        case SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY:
            attr->value.u32 = SAI_VM_ACL_GROUP_PRIO_LOWEST;
            break;
        case SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY:
            attr->value.u32 = SAI_VM_ACL_GROUP_PRIO_HIGHEST;
            break;
        case SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY:
            attr->value.u32 = SAI_VM_ACL_ENTRY_PRIO_LOWEST;
            break;
        case SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY:
            attr->value.u32 = SAI_VM_ACL_ENTRY_PRIO_HIGHEST;
            break;
        case SAI_SWITCH_ATTR_FDB_DST_USER_META_DATA_RANGE:
            attr->value.u32range.min = SAI_VM_ACL_FDB_DST_USER_META_DATA_MIN;
            attr->value.u32range.max = SAI_VM_ACL_FDB_DST_USER_META_DATA_MAX;
            break;
        case SAI_SWITCH_ATTR_ROUTE_DST_USER_META_DATA_RANGE:
            attr->value.u32range.min = SAI_VM_ACL_ROUTE_DST_USER_META_DATA_MIN;
            attr->value.u32range.max = SAI_VM_ACL_ROUTE_DST_USER_META_DATA_MAX;
            break;
        case SAI_SWITCH_ATTR_NEIGHBOR_DST_USER_META_DATA_RANGE:
            attr->value.u32range.min = SAI_VM_ACL_NEIGHBOR_DST_USER_META_DATA_MIN;
            attr->value.u32range.max = SAI_VM_ACL_NEIGHBOR_DST_USER_META_DATA_MAX;
            break;
        case SAI_SWITCH_ATTR_PORT_USER_META_DATA_RANGE:
            attr->value.u32range.min = SAI_VM_ACL_PORT_USER_META_DATA_MIN;
            attr->value.u32range.max = SAI_VM_ACL_PORT_USER_META_DATA_MAX;
            break;
        case SAI_SWITCH_ATTR_VLAN_USER_META_DATA_RANGE:
            attr->value.u32range.min = SAI_VM_ACL_VLAN_USER_META_DATA_MIN;
            attr->value.u32range.max = SAI_VM_ACL_VLAN_USER_META_DATA_MAX;
            break;
        case SAI_SWITCH_ATTR_ACL_USER_META_DATA_RANGE:
            attr->value.u32range.min = SAI_VM_ACL_USER_META_DATA_MIN;
            attr->value.u32range.max = SAI_VM_ACL_USER_META_DATA_MAX;
            break;
       default:
            SAI_ACL_LOG_ERR("Invalid acl switch attribute %d", attr->id);
            sai_rc = SAI_STATUS_INVALID_ATTRIBUTE_0;
            break;
    }

    return sai_rc;
}

static sai_npu_acl_api_t sai_vm_acl_api_table = {
    sai_npu_create_acl_table,
    sai_npu_delete_acl_table,
    sai_npu_validate_acl_table_field,
    sai_npu_create_acl_rule,
    sai_npu_delete_acl_rule,
    sai_npu_copy_acl_rule,
    sai_npu_set_acl_rule,
    sai_npu_free_acl_rule,
    sai_npu_create_acl_cntr,
    sai_npu_delete_acl_cntr,
    sai_npu_set_acl_cntr,
    sai_npu_get_acl_cntr,
    sai_npu_attach_cntr_to_acl_rule,
    sai_npu_detach_cntr_from_acl_rule,
    sai_npu_update_policer_acl_rule,
    sai_npu_get_acl_attribute,
    sai_npu_acl_dump_table,
    sai_npu_acl_dump_rule,
    sai_npu_acl_dump_counter
};

sai_npu_acl_api_t* sai_vm_acl_api_query (void)
{
    return &sai_vm_acl_api_table;
}
