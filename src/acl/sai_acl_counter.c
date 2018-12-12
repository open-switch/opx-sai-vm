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
 * * @file sai_acl_counter.c
 * *
 * * @brief This file contains functions for SAI ACL counter operations.
 * *
 * *************************************************************************/
#include "sai_acl_type_defs.h"
#include "sai_acl_npu_api.h"
#include "sai_acl_utils.h"
#include "sai_common_acl.h"

#include "saitypes.h"
#include "saiacl.h"
#include "saistatus.h"
#include "sai_oid_utils.h"

#include "std_type_defs.h"
#include "std_rbtree.h"
#include "sai_common_infra.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static dn_sai_id_gen_info_t acl_counter_obj_info;

static bool sai_is_acl_counter_id_in_use(uint64_t obj_id)
{
    acl_node_pt acl_node = sai_acl_get_acl_node();

    sai_object_id_t acl_counter_id =
        sai_uoid_create(SAI_OBJECT_TYPE_ACL_COUNTER,obj_id);

    if(sai_acl_cntr_find(acl_node->sai_acl_counter_tree,
                                acl_counter_id) != NULL) {
        return true;
    }
    return false;
}

static sai_object_id_t sai_acl_counter_id_create(void)
{
    if(SAI_STATUS_SUCCESS ==
       dn_sai_get_next_free_id(&acl_counter_obj_info)) {
        return (sai_uoid_create(SAI_OBJECT_TYPE_ACL_COUNTER,
                                acl_counter_obj_info.cur_id));
    }
    return SAI_NULL_OBJECT_ID;
}

void sai_acl_counter_init(void)
{
    acl_counter_obj_info.cur_id = 0;
    acl_counter_obj_info.is_wrappped = false;
    acl_counter_obj_info.mask = SAI_UOID_NPU_OBJ_ID_MASK;
    acl_counter_obj_info.is_id_in_use = sai_is_acl_counter_id_in_use;
}

static void sai_acl_cntr_free(sai_acl_counter_t *acl_cntr)
{
    STD_ASSERT(acl_cntr != NULL);
    free (acl_cntr);
}

static sai_status_t sai_acl_cntr_populate(sai_acl_counter_t *acl_cntr,
                                   uint_t attr_count,
                                   const sai_attribute_t *attr_list)
{
    uint_t attribute_count = 0, dup_index = 0;
    bool packet_count = false;
    bool byte_count = false, byte_attr = false;
    sai_attr_id_t attribute_id = 0;
    uint_t invalid_attr_cnt = 0;

    STD_ASSERT(acl_cntr != NULL);
    STD_ASSERT(attr_list != NULL);
    STD_ASSERT(attr_count > 0);

    for (attribute_count = 0; attribute_count < attr_count; attribute_count++) {

        attribute_id = attr_list[attribute_count].id;

        if (!sai_acl_cntr_valid_attr_range(attribute_id)) {

            SAI_ACL_LOG_ERR ("Attribute ID %d not a valid Counter "
                             "attribute", attribute_id);

            return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                           attribute_count);
        }

        if (attribute_id == SAI_ACL_COUNTER_ATTR_TABLE_ID) {

            acl_cntr->table_id = attr_list[attribute_count].value.oid;

        } else if (attribute_id == SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT) {
            if (attr_list[attribute_count].value.booldata == true) {
                packet_count = true;
            }
        } else if (attribute_id == SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT) {
            byte_attr = true;
            if (attr_list[attribute_count].value.booldata == true) {
                byte_count = true;
            } else {
                invalid_attr_cnt = attribute_count;
            }
        } else if ((attribute_id == SAI_ACL_COUNTER_ATTR_BYTES) ||
                   (attribute_id == SAI_ACL_COUNTER_ATTR_PACKETS)) {
            return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                           attribute_count);
        }

        for (dup_index = attribute_count + 1; dup_index < attr_count;
             dup_index++) {
            if (attribute_id == attr_list[dup_index].id) {
                SAI_ACL_LOG_ERR ("Duplicate attributes in "
                           "indices %d and %d", attribute_count, dup_index);
                return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                               dup_index);
            }
        }
    }

    if (packet_count && byte_count) {

        acl_cntr->counter_type = SAI_ACL_COUNTER_BYTES_PACKETS;

    } else if (packet_count) {

        acl_cntr->counter_type = SAI_ACL_COUNTER_PACKETS;

    } else {
        if (byte_attr && !byte_count) {
            return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0,
                                           invalid_attr_cnt);
        }

        /* Default Counter Type */
        acl_cntr->counter_type = SAI_ACL_COUNTER_BYTES;
    }

    return SAI_STATUS_SUCCESS;
}

static inline sai_status_t sai_acl_cntr_insert(
                                        rbtree_handle sai_acl_counter_tree,
                                        sai_acl_counter_t *acl_cntr)
{
    STD_ASSERT(sai_acl_counter_tree != NULL);
    STD_ASSERT(acl_cntr != NULL);

    return (std_rbtree_insert(sai_acl_counter_tree, acl_cntr)
            == STD_ERR_OK ? SAI_STATUS_SUCCESS: SAI_STATUS_FAILURE);
}

sai_status_t sai_create_acl_counter(sai_object_id_t *acl_counter_id,
                                    sai_object_id_t switch_id,
                                    uint32_t attr_count,
                                    const sai_attribute_t *attr_list)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_acl_table_t *acl_table = NULL;
    sai_acl_counter_t *acl_cntr = NULL;
    acl_node_pt acl_node = NULL;

    if (attr_count == 0) {
        SAI_ACL_LOG_ERR ("Parameter attr_count is 0");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(attr_list != NULL);
    STD_ASSERT(acl_counter_id != NULL);

    acl_cntr = (sai_acl_counter_t *)calloc(1, sizeof(sai_acl_counter_t));
    if (acl_cntr == NULL) {

        SAI_ACL_LOG_ERR ("Allocation of Memory failed for ACL Counter");
        return SAI_STATUS_NO_MEMORY;
    }

    do {
        sai_acl_lock();
        acl_node = sai_acl_get_acl_node();

        rc = sai_acl_cntr_populate(acl_cntr, attr_count, attr_list);
        if (rc != SAI_STATUS_SUCCESS) {

            SAI_ACL_LOG_ERR (" ACL Counter populate function failed");
            break;
        }

        if (acl_cntr->table_id == SAI_ACL_INVALID_TABLE_ID) {

            SAI_ACL_LOG_ERR ("Counter creation failed as "
                             "Table Id is not specified");

            rc = SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
            break;
        }

        acl_table = sai_acl_table_find(acl_node->sai_acl_table_tree,
                                       acl_cntr->table_id);
        if (acl_table == NULL) {

            SAI_ACL_LOG_ERR ("ACL Table not present, "
                             "Counter creation failed");

            rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

         /* If Acl table is not present, it needs to be created in the h/w*/
        if (acl_table->npu_table_info == NULL) {

            SAI_ACL_LOG_TRACE ("Table Id 0x%"PRIx64" not created in "
                               "h/w yet", acl_cntr->table_id);

            rc = sai_acl_npu_api_get()->create_acl_table(acl_table);
            if (rc != SAI_STATUS_SUCCESS) {

                SAI_ACL_LOG_ERR (" Table Creation failed "
                           "for ACL Counter creation Table Id 0x%"PRIx64"",
                           acl_cntr->table_id);
                break;
            }
        }
        acl_cntr->counter_key.counter_id = sai_acl_counter_id_create();

        if(acl_cntr->counter_key.counter_id == SAI_NULL_OBJECT_ID)
        {
            rc = SAI_STATUS_FAILURE;
            break;
        }

        /* Before creating SAI ACL Software database, program the hardware.*/
        rc = sai_acl_npu_api_get()->create_acl_cntr(acl_table, acl_cntr);
        if (rc != SAI_STATUS_SUCCESS) {

            SAI_ACL_LOG_ERR ("ACL Counter Creation failed "
                             "in hardware for Table Id 0x%"PRIx64"",
                             acl_cntr->table_id);
            break;
        }

        SAI_ACL_LOG_INFO ("ACL Counter Id 0x%"PRIx64" successfully created in hardware",
                          acl_cntr->counter_key.counter_id);

        /* Insert the ACL counter node in the RB Tree. */
        rc = sai_acl_cntr_insert(acl_node->sai_acl_counter_tree, acl_cntr);
        if (rc != SAI_STATUS_SUCCESS) {

            SAI_ACL_LOG_ERR ("Insertion of ACL Counter "
                             "Id 0x%"PRIx64" failed rc %d",
                             acl_cntr->counter_key.counter_id, rc);
            break;
        }

        *acl_counter_id = acl_cntr->counter_key.counter_id;
        acl_table->num_counters++;

    } while(0);

    if (rc != SAI_STATUS_SUCCESS) {
        sai_acl_cntr_free(acl_cntr);
    }

    sai_acl_unlock();
    return rc;
}

static sai_acl_counter_t *sai_acl_cntr_remove(rbtree_handle sai_acl_counter_tree,
                                 sai_acl_counter_t *acl_cntr)
{
    STD_ASSERT(acl_cntr != NULL);
    STD_ASSERT(sai_acl_counter_tree != NULL);

    return (sai_acl_counter_t *) std_rbtree_remove(sai_acl_counter_tree,
                                                   acl_cntr);
}

sai_status_t sai_delete_acl_counter(sai_object_id_t acl_counter_id)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    sai_acl_counter_t *acl_counter = NULL;
    sai_acl_table_t *acl_table = NULL;
    acl_node_pt acl_node = NULL;

    if (!sai_is_obj_id_acl_counter(acl_counter_id)) {
        SAI_ACL_LOG_ERR ("ACL Counter Id 0x%"PRIx64" is not "
                         "a ACL Counter Object", acl_counter_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    do {
        sai_acl_lock();

        acl_node = sai_acl_get_acl_node();

        acl_counter = sai_acl_cntr_find(acl_node->sai_acl_counter_tree,
                                        acl_counter_id);
        if (acl_counter == NULL) {

           /* ACL Counter deletion request is invalid as the counter id
            * does not exist */
            SAI_ACL_LOG_ERR ("ACL Counter Id 0x%"PRIx64" not present",
                             acl_counter_id);

            rc= SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        if (acl_counter->shared_count != 0) {
             SAI_ACL_LOG_ERR ("Failed to delete Counter 0x%"PRIx64" "
                              " as it is still in use",
                              acl_counter->counter_key.counter_id);
            rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        /* Delete the entry in the hardware */
        rc = sai_acl_npu_api_get()->delete_acl_cntr(acl_counter);

        if (rc != SAI_STATUS_SUCCESS) {

            SAI_ACL_LOG_ERR ("Failure deleting ACL Counter "
                             "Id 0x%"PRIx64" from hardware, counter deletion failed",
                             acl_counter_id);
            break;
        }

        /* Update all the SAI ACL data structures */
        acl_counter = sai_acl_cntr_remove(acl_node->sai_acl_counter_tree,
                                          acl_counter);
        if (acl_counter == NULL) {

            /*Removal from RB Tree failed */
            SAI_ACL_LOG_ERR ("Failure removing ACL Counter Id 0x%"PRIx64" "
                             "from Counter DB, counter deletion failed",
                             acl_counter_id);

            rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        acl_table = sai_acl_table_find(acl_node->sai_acl_table_tree,
                                       acl_counter->table_id);
        STD_ASSERT(acl_table != NULL);
        acl_table->num_counters--;

        /* Finally free the ACL counter memory */
        sai_acl_cntr_free(acl_counter);

        SAI_ACL_LOG_INFO ("ACL Counter Id 0x%"PRIx64" successfully removed "
                          "from hardware", acl_counter_id);
    } while(0);

    sai_acl_unlock();
    return rc;
}

static sai_status_t sai_acl_cntr_util_get_attr_count_value(
                                    sai_acl_counter_t *acl_counter,
                                    const sai_attribute_t *attr_list,
                                    uint64_t *counter_value, bool *byte_set)
{
    uint_t attribute_count = 0;
    sai_attr_id_t attribute_id = 0;

    STD_ASSERT(attr_list != NULL);
    STD_ASSERT(counter_value != NULL);

    attribute_id = attr_list->id;

    if (!sai_acl_cntr_valid_attr_range(attribute_id)) {

        SAI_ACL_LOG_ERR ("Attribute ID %d not in ACL "
                         "Counter attribute range", attribute_id);
        return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                       attribute_count);
    }

    if ((attribute_id == SAI_ACL_COUNTER_ATTR_PACKETS) &&
        (acl_counter->counter_type == SAI_ACL_COUNTER_BYTES)) {
        SAI_ACL_LOG_ERR ("Counter Attribute Packets does not "
                         "match with the counter type (byte)");
        return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                       attribute_count);
    }

    if ((attribute_id == SAI_ACL_COUNTER_ATTR_BYTES) &&
        (acl_counter->counter_type == SAI_ACL_COUNTER_PACKETS)) {
        SAI_ACL_LOG_ERR ("Counter Attribute Bytes does not "
                         "match with the counter type (packet) present");
        return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0,
                                       attribute_count);
    }

    if ((attribute_id == SAI_ACL_COUNTER_ATTR_PACKETS) ||
        (attribute_id == SAI_ACL_COUNTER_ATTR_BYTES)) {

        *counter_value = attr_list->value.u64;

    } else {

        SAI_ACL_LOG_ERR ("Invalid Attribute ID %d for "
                         "ACL Counter Set Attribute",
                         attribute_id);
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    if (attribute_id == SAI_ACL_COUNTER_ATTR_BYTES) {
        *byte_set = true;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_set_acl_cntr(sai_object_id_t acl_counter_id,
                              const sai_attribute_t *attr)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_acl_counter_t *acl_counter = NULL;
    uint64_t counter_value = 0;
    acl_node_pt acl_node = NULL;
    bool byte_set = false;

    STD_ASSERT(attr != NULL);

    if (!sai_is_obj_id_acl_counter(acl_counter_id)) {
        SAI_ACL_LOG_ERR ("ACL Counter Id 0x%"PRIx64" is not "
                         "a ACL Counter Object", acl_counter_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    do {
        sai_acl_lock();

        acl_node = sai_acl_get_acl_node();
        acl_counter = sai_acl_cntr_find(acl_node->sai_acl_counter_tree,
                                        acl_counter_id);
        if (acl_counter == NULL) {

            /* ACL Counter Set attribute is invalid as the counter id
             * does not exist in SAI ACL DB. */
             SAI_ACL_LOG_ERR ("ACL Counter not present for "
                              "Counter ID 0x%"PRIx64", counter set attribute failed",
                              acl_counter_id);
             rc = SAI_STATUS_INVALID_OBJECT_ID;
             break;
        }

        rc = sai_acl_cntr_util_get_attr_count_value(acl_counter,
                                                    attr, &counter_value, &byte_set);

        if (rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("ACL Counter get count from attribute failed "
                             "for Counter Id 0x%"PRIx64"", acl_counter_id);
            break;
        }

        rc = sai_acl_npu_api_get()->set_acl_cntr(acl_counter, counter_value, byte_set);

        if (rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("Failed in setting ACL Counter "
                             "set count in NPU");
            break;
        } else {
            SAI_ACL_LOG_INFO ("Attribute successfully set for ACL "
                              "Counter Id 0x%"PRIx64"", acl_counter_id);
        }
    } while(0);

    sai_acl_unlock();
    return rc;
}

static sai_status_t sai_acl_cntr_util_set_attr_count_value(
                                    sai_acl_counter_t *acl_counter,
                                    uint_t attr_count,
                                    sai_attribute_t *attr_list,
                                    uint_t num_counters,
                                    uint64_t *counter_value)
{
    uint_t attribute_count = 0;
    sai_attr_id_t attribute_id = 0;
    bool valid_attr = false;

    STD_ASSERT(attr_count > 0);
    STD_ASSERT(attr_list != NULL);

    for (attribute_count = 0; attribute_count < attr_count; attribute_count++) {

        attribute_id = attr_list[attribute_count].id;
        if (!sai_acl_cntr_valid_attr_range(attribute_id)) {
            SAI_ACL_LOG_ERR ("Attribute ID %d not in ACL "
                             "Counter attribute range", attribute_id);
            return sai_get_indexed_ret_val (SAI_STATUS_INVALID_ATTRIBUTE_0,
                                            attribute_count);
        }

        if ((attribute_id == SAI_ACL_COUNTER_ATTR_PACKETS) &&
            (acl_counter->counter_type == SAI_ACL_COUNTER_BYTES)) {
            SAI_ACL_LOG_ERR ("Counter Attribute Packets does not "
                             "match with the counter type (byte)");
            return sai_get_indexed_ret_val (SAI_STATUS_INVALID_ATTRIBUTE_0,
                                            attribute_count);
        }

        if ((attribute_id == SAI_ACL_COUNTER_ATTR_BYTES) &&
            (acl_counter->counter_type == SAI_ACL_COUNTER_PACKETS)) {
            SAI_ACL_LOG_ERR ("Counter Attribute Bytes does not "
                             "match with the counter type (packet) present");
            return sai_get_indexed_ret_val (SAI_STATUS_INVALID_ATTRIBUTE_0,
                                            attribute_count);
        }

        if (num_counters == SAI_ACL_COUNTER_NUM_PACKETS_AND_BYTES) {
            if (attribute_id == SAI_ACL_COUNTER_ATTR_BYTES) {
                valid_attr = true;
                attr_list[attribute_count].value.u64 = counter_value[0];
            } else if (attribute_id == SAI_ACL_COUNTER_ATTR_PACKETS) {
                valid_attr = true;
                attr_list[attribute_count].value.u64 = counter_value[1];
            }
        } else if (num_counters == SAI_ACL_COUNTER_NUM_PACKETS_OR_BYTES) {

            if ((attribute_id == SAI_ACL_COUNTER_ATTR_PACKETS) ||
                (attribute_id == SAI_ACL_COUNTER_ATTR_BYTES)) {
                valid_attr = true;
                attr_list[attribute_count].value.u64 = counter_value[0];
            }
        }

        if (attribute_id == SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT) {
            valid_attr = true;
            if (acl_counter->counter_type == SAI_ACL_COUNTER_BYTES_PACKETS) {
                attr_list[attribute_count].value.booldata = true;
            } else {
                attr_list[attribute_count].value.booldata =
                    (acl_counter->counter_type != SAI_ACL_COUNTER_BYTES);
            }
        } else if (attribute_id == SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT) {
            valid_attr = true;
            if (acl_counter->counter_type == SAI_ACL_COUNTER_BYTES_PACKETS) {
                attr_list[attribute_count].value.booldata = true;
            } else {
                attr_list[attribute_count].value.booldata =
                    (acl_counter->counter_type != SAI_ACL_COUNTER_PACKETS);
            }
        } else if (attribute_id == SAI_ACL_COUNTER_ATTR_TABLE_ID) {
            valid_attr = true;
            attr_list[attribute_count].value.oid = acl_counter->table_id;
        }
    }

    if (!valid_attr) {
        SAI_ACL_LOG_ERR ("Mandatory attribute missing");
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    return SAI_STATUS_SUCCESS;
}

static uint_t sai_acl_cntr_find_num_counters(uint_t attr_count,
                                             sai_attribute_t *attr_list)
{
    uint_t attribute_count = 0;
    sai_attr_id_t attribute_id = 0;
    bool byte_count = false, packet_count = false;

    for (attribute_count = 0; attribute_count < attr_count; attribute_count++) {

         attribute_id = attr_list[attribute_count].id;

         if (attribute_id == SAI_ACL_COUNTER_ATTR_PACKETS) {
             packet_count = true;
         } else if (attribute_id == SAI_ACL_COUNTER_ATTR_BYTES) {
             byte_count = true;
         }
    }

    if (packet_count && byte_count) {
        return SAI_ACL_COUNTER_NUM_PACKETS_AND_BYTES;
    } else if (packet_count || byte_count) {
        return SAI_ACL_COUNTER_NUM_PACKETS_OR_BYTES;
    }

    return 0;
}

static sai_status_t sai_acl_cntr_get_value(sai_acl_counter_t *acl_counter,
                                           uint_t attr_count,
                                           sai_attribute_t *attr_list)
{
    uint_t fetch_counter = 0;
    sai_status_t rc = SAI_STATUS_FAILURE;

    /*
     * Check if the attribute list contains counter values for
     * both bytes and packets and whether the counter type also
     * supports both bytes and packets.
     */
    fetch_counter = sai_acl_cntr_find_num_counters(attr_count, attr_list);
    uint64_t counter_value[fetch_counter];

    if (fetch_counter != 0) {
        memset(&counter_value, 0, sizeof(counter_value));

        rc = sai_acl_npu_api_get()->get_acl_cntr(acl_counter, fetch_counter, counter_value);

        if (rc != SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR ("Failed in getting ACL Counter get "
                             "count in NPU");
            return rc;
        }
    }

    rc = sai_acl_cntr_util_set_attr_count_value(acl_counter, attr_count,
                                                attr_list, fetch_counter, counter_value);
    return rc;
}

sai_status_t sai_get_acl_cntr(sai_object_id_t acl_counter_id,
                              uint32_t attr_count,
                              sai_attribute_t *attr_list)
{
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_acl_counter_t *acl_counter = NULL;
    acl_node_pt acl_node = NULL;

    if (attr_count == 0) {
        SAI_ACL_LOG_ERR ("ACL Counter Attribute count is 0");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(attr_list != NULL);

    if (!sai_is_obj_id_acl_counter(acl_counter_id)) {
        SAI_ACL_LOG_ERR ("ACL Counter Id 0x%"PRIx64" is not "
                         "a ACL Counter Object", acl_counter_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    do {
        sai_acl_lock();

        acl_node = sai_acl_get_acl_node();

        acl_counter = sai_acl_cntr_find(acl_node->sai_acl_counter_tree,
                                        acl_counter_id);

        if (acl_counter == NULL) {

        /* ACL Counter Set attribute is invalid as the counter id
         * does not exist in SAI ACL DB. */
            SAI_ACL_LOG_ERR ("ACL Counter not present for Counter "
                             "ID 0x%"PRIx64", counter set attribute failed",
                             acl_counter_id);

            rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        rc = sai_acl_cntr_get_value(acl_counter, attr_count, attr_list);

        if (rc != SAI_STATUS_SUCCESS) {

            SAI_ACL_LOG_ERR ("ACL Counter get count failed for "
                             "Counter Id 0x%"PRIx64"", acl_counter_id);
            break;
        } else {
            SAI_ACL_LOG_INFO ("Attribute successfully get for ACL "
                              "Counter Id 0x%"PRIx64"", acl_counter_id);
        }
    } while(0);

    sai_acl_unlock();
    return rc;
}

sai_status_t sai_attach_cntr_to_acl_rule(sai_acl_rule_t *acl_rule)
{
    sai_acl_counter_t *acl_counter = NULL;
    acl_node_pt acl_node = NULL;
    sai_status_t rc = SAI_STATUS_FAILURE;
    sai_object_id_t counter_id = 0;

    STD_ASSERT(acl_rule != NULL);

    SAI_ACL_LOG_TRACE ("Updating ACL Counter 0x%"PRIx64" "
                       "with ACL rule Id 0x%"PRIx64" in Table Id 0x%"PRIx64"",
                       acl_rule->counter_id, acl_rule->rule_key.acl_id,
                       acl_rule->table_id);

    acl_node = sai_acl_get_acl_node();

    counter_id = acl_rule->counter_id;
    acl_counter = sai_acl_cntr_find(acl_node->sai_acl_counter_tree,
                                         counter_id);

    if (acl_counter == NULL) {
        SAI_ACL_LOG_ERR ("ACL Counter not present for Counter "
                         "ID 0x%"PRIx64", counter attach to ACL Rule Id 0x%"PRIx64" "
                         "failed", counter_id, acl_rule->rule_key.acl_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    rc = sai_acl_npu_api_get()->attach_cntr_to_acl_rule(acl_rule, acl_counter);

    if (rc != SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_ERR ("ACL Counter 0x%"PRIx64" failed to attach "
                         "with ACL rule Id 0x%"PRIx64" for Table Id 0x%"PRIx64" in NPU",
                         counter_id, acl_rule->rule_key.acl_id,
                         acl_rule->table_id);
        return rc;
    }

    SAI_ACL_LOG_INFO ("ACL Counter 0x%"PRIx64" successfully attached to "
                      "ACL rule Id 0x%"PRIx64" for Table Id 0x%"PRIx64"",
                      counter_id, acl_rule->rule_key.acl_id, acl_rule->table_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_detach_cntr_from_acl_rule(sai_acl_rule_t *acl_rule)
{
    sai_acl_counter_t *acl_counter = NULL;
    acl_node_pt acl_node = NULL;
    sai_status_t rc = SAI_STATUS_FAILURE;

    STD_ASSERT(acl_rule != NULL);

    SAI_ACL_LOG_TRACE ("Updating ACL Counter 0x%"PRIx64" "
                       "with ACL rule Id 0x%"PRIx64" in Table Id 0x%"PRIx64"",
                       acl_rule->counter_id, acl_rule->rule_key.acl_id,
                       acl_rule->table_id);


    acl_node = sai_acl_get_acl_node();
    acl_counter = sai_acl_cntr_find(acl_node->sai_acl_counter_tree,
                                    acl_rule->counter_id);

    if (acl_counter == NULL) {
        /* ACL Counter cannot be detached from ACL rule as the counter id
         * does not exist in SAI ACL db.*/
        SAI_ACL_LOG_ERR ("ACL Counter not present for "
                         "Counter ID 0x%"PRIx64" counter detach failed",
                         acl_rule->counter_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    /* ACL counter present, validate that both Rule and Counter belongs
     * to the same table*/
    if (acl_counter->table_id != acl_rule->table_id) {
        SAI_ACL_LOG_ERR ("ACL Counter 0x%"PRIx64" failed to detach with "
                         "ACL rule Id 0x%"PRIx64" as Tables don't match, ACL Rule Table "
                         "Id 0x%"PRIx64" and ACL Counter Table Id 0x%"PRIx64"",
                         acl_rule->counter_id,
                         acl_rule->rule_key.acl_id,
                         acl_rule->table_id, acl_counter->table_id);
        return SAI_STATUS_FAILURE;
    }

    rc = sai_acl_npu_api_get()->detach_cntr_from_acl_rule(acl_rule, acl_counter);
    if (rc != SAI_STATUS_SUCCESS)
    {
        SAI_ACL_LOG_ERR ("ACL Counter 0x%"PRIx64" failed to detach "
                         "with ACL rule Id 0x%"PRIx64" for Table Id 0x%"PRIx64" in NPU",
                         acl_counter->counter_key.counter_id,
                         acl_rule->rule_key.acl_id,
                         acl_rule->table_id);

        return rc;
    }

    SAI_ACL_LOG_INFO ("ACL Counter 0x%"PRIx64" successfully detached from "
                      "ACL rule Id 0x%"PRIx64" for Table Id 0x%"PRIx64"",
                      acl_counter->counter_key.counter_id,
                      acl_rule->rule_key.acl_id, acl_rule->table_id);

    return SAI_STATUS_SUCCESS;
}

