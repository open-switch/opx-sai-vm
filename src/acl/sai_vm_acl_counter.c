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
 * @file sai_vm_acl_counter.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI ACL counter object in VM environment.
 */

#include "sai_vm_defs.h"
#include "sai_acl_db_api.h"
#include "sai_oid_utils.h"
#include "sai_acl_npu_api.h"
#include "sai_acl_type_defs.h"
#include "sai_acl_utils.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_bit_masks.h"
#include "std_assert.h"
#include <inttypes.h>

static inline uint_t sai_vm_acl_bmp_idx_to_counter_id_get (uint_t table_id,
                                                           uint_t bmp_idx)
{
    return ((SAI_VM_ACL_TABLE_MAX_COUNTERS * table_id) + bmp_idx);
}

static inline uint_t sai_vm_acl_counter_id_to_bmp_idx_get (uint_t counter_id)
{
    return (counter_id % SAI_VM_ACL_TABLE_MAX_COUNTERS);
}

sai_status_t sai_npu_create_acl_cntr (sai_acl_table_t *acl_table,
                                      sai_acl_counter_t *acl_cntr)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    sai_npu_object_id_t table_id = 0;
    sai_object_id_t     cntr_obj_id = 0;
    uint_t              cntr_id = 0;
    int                 free_idx = 0;

    STD_ASSERT(acl_table != NULL);
    STD_ASSERT(acl_cntr != NULL);

    SAI_ACL_LOG_TRACE ("NPU ACL Counter Creation API.");

    table_id = sai_uoid_npu_obj_id_get (acl_cntr->table_id);

    free_idx = std_find_first_bit (sai_vm_access_acl_cntr_bitmap (table_id),
                                   SAI_VM_ACL_TABLE_MAX_COUNTERS, 0);

    if (free_idx < 0) {
        SAI_ACL_LOG_ERR ("No more free counters available on ACL TABLE %d "
                         "(object Id: 0x%"PRIx64").", table_id,
                         acl_cntr->table_id);

        return SAI_STATUS_TABLE_FULL;
    }

    cntr_id = sai_vm_acl_bmp_idx_to_counter_id_get (table_id, free_idx);

    cntr_obj_id = sai_uoid_create (SAI_OBJECT_TYPE_ACL_COUNTER,
                                   (sai_npu_object_id_t) cntr_id);

    acl_cntr->counter_key.counter_id = cntr_obj_id;

    /* Insert ACL Counter object record to DB. */
    sai_rc = sai_acl_counter_create_db_entry (acl_cntr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Error inserting entry to DB for ACL Counter: %d, "
                         "OBJ ID: %"PRIx64".", cntr_id, cntr_obj_id);

        return sai_rc;
    }

    STD_BIT_ARRAY_CLR (sai_vm_access_acl_cntr_bitmap (table_id), free_idx);

    SAI_ACL_LOG_TRACE ("ACL Counter Creation success, Cntr ID: %d, Obj Id: "
                       " 0x%"PRIx64" on Table Id: %d Obj Id: 0x%"PRIx64".",
                       cntr_id, cntr_obj_id, table_id, acl_cntr->table_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_delete_acl_cntr (sai_acl_counter_t *acl_cntr)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    sai_npu_object_id_t table_id = 0;
    sai_npu_object_id_t cntr_id = 0;
    uint_t              bitmap_idx = 0;

    SAI_ACL_LOG_TRACE ("NPU ACL Counter deletion API.");

    STD_ASSERT(acl_cntr != NULL);

    table_id = sai_uoid_npu_obj_id_get (acl_cntr->table_id);

    cntr_id = sai_uoid_npu_obj_id_get (acl_cntr->counter_key.counter_id);

    /* Remove ACL Counter object record from DB. */
    sai_rc = sai_acl_counter_delete_db_entry (acl_cntr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Error removing entry from DB for ACL Counter %d, "
                         "Object ID: 0x%" PRIx64 ".", cntr_id,
                         acl_cntr->counter_key.counter_id);

        return sai_rc;
    }

    bitmap_idx = sai_vm_acl_counter_id_to_bmp_idx_get (cntr_id);

    STD_BIT_ARRAY_SET (sai_vm_access_acl_cntr_bitmap (table_id), bitmap_idx);

    SAI_ACL_LOG_TRACE ("ACL Counter deletion success, Cntr ID: %d, Obj Id: "
                       " 0x%"PRIx64" from Tbl ID: %d, Obj Id: 0x%"PRIx64".",
                       cntr_id, acl_cntr->counter_key.counter_id, table_id,
                       acl_cntr->table_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_set_acl_cntr (sai_acl_counter_t *acl_cntr,
                                   uint64_t count_value, bool byte_set)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(acl_cntr != NULL);

    SAI_ACL_LOG_TRACE ("NPU ACL Counter set API.");

    /* Set ACL Counter value in the DB record. */
    sai_rc = sai_acl_counter_db_entry_set_cntrs (acl_cntr, count_value);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Error setting counter value %d in DB entry for "
                         "ACL Counter Obj ID: 0x%" PRIx64 ".", count_value,
                         acl_cntr->counter_key.counter_id);

        return sai_rc;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_get_acl_cntr (sai_acl_counter_t *acl_cntr,
                                   uint_t num_count, uint64_t *count_value)
{
    STD_ASSERT(acl_cntr != NULL);
    STD_ASSERT(count_value != NULL);

    SAI_ACL_LOG_TRACE ("NPU ACL Counter get API.");

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_attach_cntr_to_acl_rule (sai_acl_rule_t *acl_rule,
                                              sai_acl_counter_t *acl_cntr)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    uint_t       cntr_id = 0;

    STD_ASSERT(acl_rule != NULL);
    STD_ASSERT(acl_cntr != NULL);

    cntr_id = acl_cntr->counter_key.counter_id;
    acl_cntr->shared_count++;

    /* Update the ACL Counter reference count field in the DB record. */
    sai_rc = sai_acl_counter_db_entry_set_ref_count (acl_cntr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Error setting counter ref count %d in DB entry for"
                         " ACL Counter Obj ID: 0x%" PRIx64 ".",
                         acl_cntr->shared_count, cntr_id);

        return sai_rc;
    }

    /* Update the counter ID in the ACL Entry DB record. */
    sai_rc = sai_acl_rule_db_entry_update_counter_id (acl_rule->rule_key.acl_id,
                                                      cntr_id,
                                                      true /* attach cntr */);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Error setting counter ID 0x%"PRIx64" in DB entry "
                         "for ACL Rule ID: 0x%"PRIx64".", cntr_id,
                         acl_rule->rule_key.acl_id);

        return sai_rc;
    }

    SAI_ACL_LOG_TRACE ("Attaching counter obj: 0x%"PRIx64" to ACL rule obj: "
                       "0x%"PRIx64", counter shared_count after attaching: "
                       "%d.", cntr_id, acl_rule->rule_key.acl_id,
                       acl_cntr->shared_count);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_detach_cntr_from_acl_rule (sai_acl_rule_t *acl_rule,
                                                sai_acl_counter_t *acl_cntr)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    uint_t       cntr_id = 0;

    STD_ASSERT(acl_rule != NULL);
    STD_ASSERT(acl_cntr != NULL);

    cntr_id = acl_cntr->counter_key.counter_id;
    acl_cntr->shared_count--;

    /* Update the ACL Counter reference count field in the DB record. */
    sai_rc = sai_acl_counter_db_entry_set_ref_count (acl_cntr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Error setting counter ref count %d in DB entry for"
                         " ACL Counter Obj ID: 0x%" PRIx64 ".",
                         acl_cntr->shared_count, cntr_id);

        return sai_rc;
    }

    /* Update the counter ID in the ACL Entry DB record. */
    sai_rc = sai_acl_rule_db_entry_update_counter_id (acl_rule->rule_key.acl_id,
                                                      cntr_id,
                                                      false /* detach cntr */);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("Error setting counter ID 0x%"PRIx64" in DB entry "
                         "for ACL Rule ID: 0x%"PRIx64".", cntr_id,
                         acl_rule->rule_key.acl_id);

        return sai_rc;
    }


    SAI_ACL_LOG_TRACE ("Detaching counter obj: 0x%"PRIx64" from ACL rule "
                       "obj: 0x%"PRIx64", counter shared_count after "
                       "detaching: %d.", cntr_id, acl_rule->rule_key.acl_id,
                       acl_cntr->shared_count);

    return SAI_STATUS_SUCCESS;
}

void sai_npu_acl_dump_counter(const sai_acl_counter_t *acl_counter)
{
    STD_ASSERT(acl_counter != NULL);

    return;
}

