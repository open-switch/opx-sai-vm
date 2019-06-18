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
* @file sai_hostif_main.h
*
* @brief This private header file contains SAI hostif data structure
* definitions and static inline functions.
*
*******************************************************************************/

#ifndef _SAI_HOSTIF_MAIN_H_
#define _SAI_HOSTIF_MAIN_H_

#include "std_rbtree.h"
#include "std_llist.h"
#include "std_assert.h"
#include "std_error_codes.h"
#include "std_type_defs.h"
#include "sai_hostif_common.h"

#include "saitypes.h"
#include "saistatus.h"
#include "saiswitch.h"
#include <string.h>

#define DN_HOSTIF_MAX_TRAP_GROUPS          (128)
#define DN_HOSTIF_DEFAULT_MIN_PRIO         (0)
#define DN_HOSTIF_DEFAULT_TRAP_GROUP_ATTRS (3)
#define DN_HOSTIF_USER_DEF_TRAP_TYPE_SHIFT (16)
#define DN_HOSTIF_USER_DEF_TRAP_ID_MASK    (0xFFFF)

typedef struct _dn_sai_hostif_pkt_attr_property_t {
    sai_attr_id_t attr_id;
    bool valid_on_send;
    bool mandatory_on_send;
} dn_sai_hostif_pkt_attr_property_t;

typedef sai_status_t (*dn_sai_hostif_check_attribute_fn) (
                            const sai_attribute_t *attr);

typedef struct _dn_sai_hostif_trap_attr_property_t {
    sai_attr_id_t                    attr_id;
    bool                             mandatory_in_create;
    bool                             valid_in_set;
    dn_sai_hostif_check_attribute_fn check_attr_func;
} dn_sai_hostif_trap_attr_property_t;

typedef struct _dn_sai_hostif_trap_group_attr_property_t {
    sai_attr_id_t attr_id;
    bool          mandatory_in_create;
    bool          valid_in_set;
} dn_sai_hostif_trap_group_attr_property_t;

typedef struct _dn_sai_hostif_user_defined_trap_attr_property_t {
    sai_attr_id_t                    attr_id;
    bool                             mandatory_in_create;
    bool                             valid_in_set;
    dn_sai_hostif_check_attribute_fn check_attr_func;
} dn_sai_hostif_user_defined_trap_attr_property_t;

typedef struct _dn_sai_hostif_info_t {
    rbtree_handle             trap_tree;
    rbtree_handle             trap_group_tree;
    rbtree_handle             user_def_trap_tree;
    void                     *trap_group_bitmap;
    void                     *user_def_trap_bitmap;
    sai_object_id_t           default_trap_group_id;
    uint_t                    mandatory_send_pkt_attr_count;
    uint_t                    mandatory_trap_group_attr_count;
    uint_t                    mandatory_trap_attr_count;
    uint_t                    mandatory_user_def_trap_attr_count;
    uint_t                    max_pkt_attrs;
    uint_t                    max_trap_group_attrs;
    uint_t                    max_trap_attrs;
    uint_t                    max_user_def_trap_attrs;
    uint_t                    max_user_def_traps;
} dn_sai_hostintf_info_t;

typedef struct _dn_sai_hostif_valid_traps_t {
    sai_hostif_trap_type_t trap_type;
    sai_packet_action_t def_action;
} dn_sai_hostif_valid_traps_t;

/***************STATIC INLINE FUNCTIONS********************/
static inline void dn_sai_hostif_add_trap_to_trapgroup(
                            dn_sai_trap_node_t *trap_node,
                            dn_sai_trap_group_node_t *trap_group)
{
    STD_ASSERT(trap_group != NULL);
    STD_ASSERT(trap_node != NULL);

    std_dll_insert(&trap_group->trap_list, (std_dll *)trap_node);
    trap_group->trap_count++;
}

static inline void dn_sai_hostif_remove_trap_to_trapgroup(
                            dn_sai_trap_node_t *trap_node,
                            dn_sai_trap_group_node_t *trap_group)
{
    STD_ASSERT(trap_group != NULL);
    STD_ASSERT(trap_node != NULL);

    std_dll_remove(&trap_group->trap_list, (std_dll *)trap_node);
    trap_group->trap_count--;
}

static inline sai_status_t dn_sai_hostif_add_trapgroup(
                             rbtree_handle trap_group_tree,
                             dn_sai_trap_group_node_t * trap_group)
{
    STD_ASSERT(trap_group != NULL);
    STD_ASSERT(trap_group_tree != NULL);
    return (std_rbtree_insert(trap_group_tree, trap_group)
            == STD_ERR_OK ? SAI_STATUS_SUCCESS: SAI_STATUS_FAILURE);
}

static inline dn_sai_trap_group_node_t *dn_sai_hostif_remove_trapgroup(
                              rbtree_handle trap_group_tree,
                              dn_sai_trap_group_node_t * trap_group)
{
    STD_ASSERT(trap_group != NULL);
    STD_ASSERT(trap_group_tree != NULL);
    return ((dn_sai_trap_group_node_t *)std_rbtree_remove(
                                     trap_group_tree, trap_group));
}

static inline dn_sai_trap_group_node_t *dn_sai_hostif_find_trapgroup(
                                                rbtree_handle trap_group_tree,
                                                sai_object_id_t trap_group_id)
{
    STD_ASSERT(trap_group_tree != NULL);
    dn_sai_trap_group_node_t trap_group;

    memset(&trap_group, 0, sizeof(dn_sai_trap_group_node_t));
    trap_group.key.trap_group_id = trap_group_id;
    return ( dn_sai_trap_group_node_t *) std_rbtree_getexact(trap_group_tree,
                                                             &trap_group);
}

static inline dn_sai_trap_node_t * dn_sai_hostif_find_trap_node
                                    (rbtree_handle trap_tree,
                                     sai_object_id_t trapid)
{
    STD_ASSERT(trap_tree != NULL);
    dn_sai_trap_node_t temp_node;

    memset(&temp_node, 0, sizeof(dn_sai_trap_node_t));
    temp_node.key.trap_id = trapid;

    return (dn_sai_trap_node_t *) std_rbtree_getexact(trap_tree, &temp_node);
}

static inline void dn_sai_hostif_add_user_def_trap_to_trapgroup(
                            dn_sai_user_def_trap_node_t *trap_node,
                            dn_sai_trap_group_node_t *trap_group)
{
    STD_ASSERT(trap_group != NULL);
    STD_ASSERT(trap_node != NULL);

    std_dll_insert(&trap_group->user_def_trap_list, (std_dll *)trap_node);
    trap_group->user_def_trap_count++;
}

static inline void dn_sai_hostif_remove_user_def_trap_to_trapgroup(
                            dn_sai_user_def_trap_node_t *trap_node,
                            dn_sai_trap_group_node_t *trap_group)
{
    STD_ASSERT(trap_group != NULL);
    STD_ASSERT(trap_node != NULL);

    std_dll_remove(&trap_group->user_def_trap_list, (std_dll *)trap_node);

    if (trap_group->user_def_trap_count)
    {
        trap_group->user_def_trap_count--;
    }
}

static inline dn_sai_user_def_trap_node_t *
  dn_sai_hostif_find_user_def_trap_node(rbtree_handle user_def_trap_tree,
                                        sai_object_id_t user_def_trapid)
{
    STD_ASSERT(user_def_trap_tree != NULL);
    dn_sai_user_def_trap_node_t temp_node;

    memset(&temp_node, 0, sizeof(dn_sai_user_def_trap_node_t));
    temp_node.key.user_def_trap_id = user_def_trapid;

    return (dn_sai_user_def_trap_node_t *) std_rbtree_getexact(user_def_trap_tree,
                                                               &temp_node);
}

/***************Function prototypes*************************/
void sai_hostif_lock();
void sai_hostif_unlock();
dn_sai_hostintf_info_t * dn_sai_hostintf_get_info();
bool dn_sai_hostif_is_valid_trap_type(sai_hostif_trap_type_t trap_type);
sai_packet_action_t dn_sai_hostif_get_default_action(sai_hostif_trap_type_t trap_id);
sai_status_t dn_sai_hostif_validate_trapgroup_oid(const sai_attribute_t *attr);
sai_status_t dn_sai_hostif_validate_portlist(const sai_attribute_t *attr);
sai_status_t dn_sai_hostif_validate_action(const sai_attribute_t *attr);
dn_sai_trap_group_node_t *dn_sai_hostif_find_trapgroup_by_queue(uint_t cpu_queue);
void sai_hostif_debug_set(sai_hostif_debug_attr_t attr_id, int value);
void sai_hostif_dump_info(void);
void sai_hostif_dump_traps(void);
void sai_hostif_dump_user_def_traps(void);
void sai_hostif_dump_trapgroups(void);
void sai_hostif_dump_rx_errors_count(void);
#endif /*_SAI_HOSTIF_MAIN_H_*/
