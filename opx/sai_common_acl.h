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
* @file sai_common_acl.h
*
* @brief This file contains SAI ACL common definitions and prototypes
*
*************************************************************************/

#ifndef _SAI_COMMON_ACL_H
#define _SAI_COMMON_ACL_H

#include "sai_acl_type_defs.h"
#include "saistatus.h"
#include "std_type_defs.h"
#include "std_rbtree.h"
#include "std_struct_utils.h"
#include "std_assert.h"
#include "string.h"
#include <stdlib.h>
#include "saiacl.h"

#define SAI_ACL_RANGE_MAX_ATTR_COUNT   (SAI_ACL_RANGE_ATTR_END - SAI_ACL_RANGE_ATTR_START)
#define SAI_ACL_DEFAULT_SET_ATTR_COUNT (1)

#define SAI_ACL_RULE_DLL_GLUE_OFFSET \
         STD_STR_OFFSET_OF(sai_acl_rule_t, acl_rule_priority)

#define SAI_ACL_RULE_DLL_GLUE_SIZE \
         STD_STR_SIZE_OF(sai_acl_rule_t, acl_rule_priority)

#define SAI_ACL_COUNTER_NUM_PACKETS_AND_BYTES (2)
#define SAI_ACL_COUNTER_NUM_PACKETS_OR_BYTES (1)

/**************************************************************************
 *                     Static Inline Functions
 **************************************************************************/

static inline bool sai_acl_valid_stage(sai_acl_stage_t stage)
{
    if((stage == SAI_ACL_STAGE_INGRESS) ||
       (stage == SAI_ACL_STAGE_EGRESS)) {
        return true;
    }
    return false;
}

static inline bool sai_acl_table_valid_attr_range(sai_attr_id_t id)
{
    if ((id >= SAI_ACL_TABLE_ATTR_START) &&
        (id <= SAI_ACL_TABLE_ATTR_CUSTOM_RANGE_END)) {
        return true;
    }
    /* Included extension attribute under valid range */
    if ((id >= SAI_ACL_TABLE_ATTR_EXTENSIONS_RANGE_START) &&
        (id <= SAI_ACL_TABLE_ATTR_EXTENSIONS_RANGE_END)) {
        return true;
    }
    return false;
}

static inline bool sai_acl_table_field_attr_range(sai_attr_id_t id)
{
    if ((id >= SAI_ACL_TABLE_ATTR_FIELD_START) &&
        (id <= SAI_ACL_TABLE_ATTR_CUSTOM_RANGE_END)) {
        return true;
    }
    /* Included extension attribute under valid range */
    if ((id >= SAI_ACL_TABLE_ATTR_EXTENSIONS_RANGE_START) &&
        (id <= SAI_ACL_TABLE_ATTR_EXTENSIONS_RANGE_END)) {
        return true;
    }
    return false;
}

static inline bool sai_acl_table_action_attr_range(sai_acl_action_type_t id)
{
    if ((id >= SAI_ACL_ACTION_TYPE_REDIRECT) &&
        (id <= SAI_ACL_ACTION_TYPE_SET_DO_NOT_LEARN)) {
        return true;
    }
    return false;
}

static inline bool sai_acl_rule_valid_attr_range(sai_attr_id_t id)
{
    if ((id >= SAI_ACL_ENTRY_ATTR_START) &&
        (id <= SAI_ACL_ENTRY_ATTR_ACTION_END)) {
        return true;
    }
    return false;
}

static inline bool sai_acl_rule_field_attr_range(sai_attr_id_t id)
{
    if ((id >= SAI_ACL_ENTRY_ATTR_FIELD_START) &&
        (id <= SAI_ACL_ENTRY_ATTR_FIELD_END)) {
        return true;
    }
    return false;
}

static inline bool sai_acl_rule_action_attr_range(sai_attr_id_t id)
{
    if ((id >= SAI_ACL_ENTRY_ATTR_ACTION_START) &&
        (id <= SAI_ACL_ENTRY_ATTR_ACTION_END)) {
        return true;
    }
    return false;
}

static inline bool sai_acl_cntr_valid_attr_range(sai_attr_id_t id)
{
    if ((id >= SAI_ACL_COUNTER_ATTR_START) &&
        (id < SAI_ACL_COUNTER_ATTR_END)) {
        return true;
    }
    return false;
}

static inline sai_acl_range_t *sai_acl_range_node_alloc()
{
    return ((sai_acl_range_t *)
            calloc(1,sizeof (sai_acl_range_t)));
}

static inline sai_acl_table_group_member_t *sai_acl_table_group_member_node_alloc()
{
    return ((sai_acl_table_group_member_t*)
            calloc(1,sizeof (sai_acl_table_group_member_t)));
}

static inline sai_acl_table_group_t *sai_acl_table_group_node_alloc()
{
    return ((sai_acl_table_group_t*) calloc(1,sizeof (sai_acl_table_group_t)));
}

static inline sai_acl_slice_t *sai_acl_slice_node_alloc()
{
        return ((sai_acl_slice_t *)
                    calloc(1,sizeof (sai_acl_slice_t)));
}

static inline void sai_acl_range_free(sai_acl_range_t *p_acl_range_node)
{
    free ((void *)p_acl_range_node);
}

static inline void sai_acl_table_group_free(sai_acl_table_group_t *p_acl_table_group_node)
{
    free ((void *)p_acl_table_group_node);
}

static inline void sai_acl_table_group_member_free
    (sai_acl_table_group_member_t *p_acl_table_group_member_node)
{
    free ((void *)p_acl_table_group_member_node);
}

static inline void sai_acl_slice_free(sai_acl_slice_t *p_acl_slice_node)
{
    free ((void *)p_acl_slice_node);
}
/**************************************************************************
 *                     Function Prototypes
 **************************************************************************/

sai_acl_table_id_node_t *sai_acl_get_table_id_gen(void);
sai_status_t sai_attach_cntr_to_acl_rule(sai_acl_rule_t *acl_rule);
sai_status_t sai_detach_cntr_from_acl_rule(sai_acl_rule_t *acl_rule);
sai_status_t sai_create_acl_table(sai_object_id_t *acl_table_id,
                                  sai_object_id_t switch_id,
                                  uint32_t attr_count,
                                  const sai_attribute_t *attr_list);
sai_status_t sai_delete_acl_table(sai_object_id_t table_id);
sai_status_t sai_get_acl_table(sai_object_id_t table_id,
                               uint32_t attr_count,
                               sai_attribute_t *attr_list);
sai_status_t sai_set_acl_table(sai_object_id_t table_id,
                               const sai_attribute_t *attr);
sai_status_t sai_create_acl_rule(sai_object_id_t *acl_rule_id,
                                  sai_object_id_t switch_id,
                                 uint32_t attr_count,
                                 const sai_attribute_t *attr_list);
sai_status_t sai_delete_acl_rule(sai_object_id_t acl_id);
sai_status_t sai_set_acl_rule(sai_object_id_t acl_id,
                              const sai_attribute_t *attr);
sai_status_t sai_get_acl_rule(sai_object_id_t acl_id,
                              uint32_t attr_count,
                              sai_attribute_t *attr_list);
sai_status_t sai_create_acl_counter(sai_object_id_t *acl_counter_id,
                                    sai_object_id_t switch_id,
                                    uint32_t attr_count,
                                    const sai_attribute_t *attr_list);
sai_status_t sai_delete_acl_counter(sai_object_id_t acl_counter_id);
sai_status_t sai_set_acl_cntr(sai_object_id_t acl_counter_id,
                              const sai_attribute_t *attr);
sai_status_t sai_get_acl_cntr(sai_object_id_t acl_counter_id,
                              uint32_t attr_count,
                              sai_attribute_t *attr_list);
sai_status_t  sai_acl_rule_policer_update(sai_acl_rule_t *acl_rule_modify,
                                          sai_acl_rule_t *acl_rule_present);
sai_status_t sai_attach_policer_to_acl_rule(sai_acl_rule_t *acl_rule);
sai_status_t sai_detach_policer_from_acl_rule(sai_acl_rule_t *acl_rule);

void sai_acl_counter_init(void);
sai_status_t sai_acl_table_group_member_create(sai_object_id_t *acl_table_group_mem_id,
        sai_object_id_t switch_id,
        uint32_t attr_count,
        const sai_attribute_t *attr_list);
sai_status_t sai_acl_table_group_member_delete(sai_object_id_t acl_table_group_mem_id);
sai_status_t sai_acl_table_group_member_attribute_set(sai_object_id_t acl_table_group_mem_id,
                                                      const sai_attribute_t *attr);
sai_status_t sai_acl_table_group_member_attribute_get(sai_object_id_t acl_table_group_mem_id,
                                                      uint32_t attr_count,
                                                      sai_attribute_t *attr_list);
sai_status_t sai_acl_table_group_create(sai_object_id_t *acl_table_group_id,
        sai_object_id_t switch_id,
        uint32_t attr_count,
        const sai_attribute_t *attr_list);
sai_status_t sai_acl_table_group_delete(sai_object_id_t acl_table_group_id);
sai_status_t sai_acl_table_group_attribute_set(sai_object_id_t acl_table_group_id,
                                               const sai_attribute_t *attr);
sai_status_t sai_acl_table_group_attribute_get(sai_object_id_t acl_table_group_id,
                                               uint32_t attr_count,
                                               sai_attribute_t *attr_list);
sai_status_t sai_create_acl_range(sai_object_id_t *acl_range_id,
                                  sai_object_id_t switch_id,
                                  uint32_t attr_count,
                                  const sai_attribute_t *attr_list);
sai_status_t sai_delete_acl_range(sai_object_id_t acl_range_id);
sai_status_t sai_set_acl_range(sai_object_id_t acl_range_id,
                               const sai_attribute_t *attr);
sai_status_t sai_get_acl_range(sai_object_id_t acl_range_id,
                               uint32_t attr_count,
                               sai_attribute_t *attr_list);
void sai_acl_table_group_init(void);
void sai_acl_table_group_member_init(void);
void sai_acl_range_init(void);
void sai_acl_lock(void);
void sai_acl_unlock(void);
void sai_acl_dump_all_tables(void);
void sai_acl_dump_table(sai_object_id_t table_id);
void sai_acl_dump_rule(sai_object_id_t rule_id);
void sai_acl_dump_counters();
void sai_acl_dump_counter_per_entry(int eid);
sai_status_t sai_acl_slice_attribute_get(sai_object_id_t acl_slice_id,
                                            uint32_t attr_count,
                                            sai_attribute_t *attr_list);
void sai_acl_slice_init(void);
sai_status_t sai_acl_slice_create_objects(void);
sai_status_t sai_get_acl_slice_objects(sai_attribute_t *attr);
#endif  /* _SAI_COMMON_ACL_H */
