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
* @file sai_acl_utils.h
*
* @brief This file contains utility functions for  SAI ACL component.
*
*************************************************************************/

/* OPENSOURCELICENSE */

#ifndef __SAI_ACL_UTILS_H__
#define __SAI_ACL_UTILS_H__
#include "sai_acl_type_defs.h"
#include "std_rbtree.h"
#include "saitypes.h"
#include "sai_event_log.h"
#include "std_assert.h"
#include <string.h>
#include <inttypes.h>
#include <stdio.h>


/** \defgroup SAIACLUTILS SAI - ACL Utility functions
 *  Util functions in the SAI ACL component
 *
 *  \{
 */

/**
 * @brief Enumeration to specifiy ACL Atribute data types
 */
typedef enum _sai_acl_rule_attr_type {
    SAI_ACL_ENTRY_ATTR_BOOL,
    SAI_ACL_ENTRY_ATTR_ONE_BYTE,
    SAI_ACL_ENTRY_ATTR_TWO_BYTES,
    SAI_ACL_ENTRY_ATTR_FOUR_BYTES,
    SAI_ACL_ENTRY_ATTR_ENUM,
    SAI_ACL_ENTRY_ATTR_MAC,
    SAI_ACL_ENTRY_ATTR_IPv4,
    SAI_ACL_ENTRY_ATTR_IPv6,
    SAI_ACL_ENTRY_ATTR_OBJECT_ID,
    SAI_ACL_ENTRY_ATTR_OBJECT_LIST,
    SAI_ACL_ENTRY_ATTR_ONE_BYTE_LIST,
    SAI_ACL_ENTRY_ATTR_INVALID
} sai_acl_rule_attr_type;

/** Logging utility for SAI ACL API */
#define SAI_ACL_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_ACL, level)) { \
            SAI_LOG_UTIL(ev_log_t_ACL, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI ACL API */
#define SAI_ACL_LOG_TRACE(msg, ...) \
        SAI_ACL_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_ACL_LOG_CRIT(msg, ...) \
        SAI_ACL_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_ACL_LOG_ERR(msg, ...) \
        SAI_ACL_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_ACL_LOG_INFO(msg, ...) \
        SAI_ACL_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_ACL_LOG_WARN(msg, ...) \
        SAI_ACL_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_ACL_LOG_NTC(msg, ...) \
        SAI_ACL_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/** Custom ACL Field macro
 *  @TODO to be removed once its added in OCP.
 * */
#define SAI_ACL_ENTRY_ATTR_FIELD_DST_PORT (SAI_ACL_TABLE_ATTR_CUSTOM_RANGE_START + 3)

/**
 * @brief insert range node into tree.
 *
 * @param[in] sai_acl_range_tree pointer to range tree
 * @param[in] acl_range_node pointer to range node
 *
 * @return SAI_STATUS_SUCCESS on successful insert.
 */
static inline sai_status_t sai_acl_range_insert(rbtree_handle sai_acl_range_tree,
                                 sai_acl_range_t *acl_range_node)
{
    STD_ASSERT(sai_acl_range_tree != NULL);
    STD_ASSERT(acl_range_node != NULL);

    return (std_rbtree_insert(sai_acl_range_tree, acl_range_node)
            == STD_ERR_OK ? SAI_STATUS_SUCCESS: SAI_STATUS_FAILURE);
}
/**
 * @brief remove range node from tree.
 *
 * @param[in] sai_acl_range_tree pointer to range tree
 * @param[in] acl_range_node pointer to range node
 *
 * @return pointer to removed node.
 */
static inline sai_acl_range_t *sai_acl_range_remove(rbtree_handle sai_acl_range_tree,
                                    sai_acl_range_t * acl_range_node)
{
    STD_ASSERT(sai_acl_range_tree != NULL);
    STD_ASSERT(acl_range_node != NULL);

    return (sai_acl_range_t *)std_rbtree_remove(sai_acl_range_tree,acl_range_node);
}

/**
 * @brief return the range node for given range tree.
 *
 * @param[in] sai_acl_range_tree Pointer to the range tree.
 * @param[in] acl_range_id range object id.
 *
 * @return Pointer to the range node if found in tree.
 */
static inline sai_acl_range_t *sai_acl_range_find(rbtree_handle sai_acl_range_tree,
                                           sai_object_id_t acl_range_id)
{
    sai_acl_range_t acl_range;
    acl_range.acl_range_id = acl_range_id;

    STD_ASSERT(sai_acl_range_tree != NULL);
    return ((sai_acl_range_t *)std_rbtree_getexact(sai_acl_range_tree, &acl_range));
}

/**
 * @brief insert group member node into tree.
 *
 * @param[in] sai_acl_table_group_member_tree pointer to tree
 * @param[in] acl_table_group_member pointer to acl table group member node
 *
 * @return SAI_STATUS_SUCCESS on successful insert.
 */
static inline sai_status_t sai_acl_table_group_member_insert(
                                 rbtree_handle sai_acl_table_group_member_tree,
                                 sai_acl_table_group_member_t* acl_table_group_member)
{
    STD_ASSERT(sai_acl_table_group_member_tree != NULL);
    STD_ASSERT(acl_table_group_member != NULL);

    return (std_rbtree_insert(sai_acl_table_group_member_tree, acl_table_group_member)
            == STD_ERR_OK ? SAI_STATUS_SUCCESS: SAI_STATUS_FAILURE);
}
/**
 * @brief remove acl table group member node from tree.
 *
 * @param[in] sai_acl_table_group_member_tree pointer to range tree
 * @param[in] acl_table_group_member pointer to range node
 *
 * @return pointer to removed node.
 */
static inline sai_acl_table_group_member_t *sai_acl_table_group_member_remove(rbtree_handle
                                                     sai_acl_table_group_member_tree,
                                    sai_acl_table_group_member_t * acl_table_group_member)
{
    STD_ASSERT(sai_acl_table_group_member_tree != NULL);
    STD_ASSERT(acl_table_group_member != NULL);

    return (sai_acl_table_group_member_t *)std_rbtree_remove(sai_acl_table_group_member_tree,
                                                      acl_table_group_member);
}

/**
 * @brief return the acl table group member node for given table tree.
 *
 * @param[in] table_tree Pointer to the acl table group member tree.
 * @param[in] table_id table group member object id.
 *
 * @return Pointer to the table group member node if found in tree.
 */
static inline sai_acl_table_group_member_t *sai_acl_table_group_member_find(
                                           rbtree_handle sai_acl_table_group_member_tree,
                                           sai_object_id_t acl_table_group_member_id)
{
    sai_acl_table_group_member_t acl_table_group_member;

    acl_table_group_member.acl_table_group_member_id = acl_table_group_member_id;

    STD_ASSERT(sai_acl_table_group_member_tree != NULL);
    return ((sai_acl_table_group_member_t*)std_rbtree_getexact(sai_acl_table_group_member_tree,
                                                               &acl_table_group_member));
}
/**
 * @brief insert group node into tree.
 *
 * @param[in] sai_acl_table_group_tree pointer to tree
 * @param[in] acl_table_group pointer to acl table group node
 *
 * @return SAI_STATUS_SUCCESS on successful insert.
 */
static inline sai_status_t sai_acl_table_group_insert(rbtree_handle sai_acl_table_group_tree,
                                 sai_acl_table_group_t* acl_table_group)
{
    STD_ASSERT(sai_acl_table_group_tree != NULL);
    STD_ASSERT(acl_table_group != NULL);

    return (std_rbtree_insert(sai_acl_table_group_tree, acl_table_group)
            == STD_ERR_OK ? SAI_STATUS_SUCCESS: SAI_STATUS_FAILURE);
}

/**
 * @brief remove acl table group node from tree.
 *
 * @param[in] sai_acl_table_group_pointer to range tree
 * @param[in] acl_table_group pointer to range node
 *
 * @return pointer to removed node.
 */
static inline sai_acl_table_group_t *sai_acl_table_group_remove(
                                    rbtree_handle sai_acl_table_group_tree,
                                    sai_acl_table_group_t* acl_table_group)
{
    STD_ASSERT(sai_acl_table_group_tree != NULL);
    STD_ASSERT(acl_table_group != NULL);

    return (sai_acl_table_group_t *)std_rbtree_remove(sai_acl_table_group_tree, acl_table_group);
}

/**
 * @brief return the acl table group node for given table tree.
 *
 * @param[in] table_tree Pointer to the acl table group tree.
 * @param[in] table_id table group object id.
 *
 * @return Pointer to the table group node if found in tree.
 */
static inline sai_acl_table_group_t *sai_acl_table_group_find(
                                  rbtree_handle sai_acl_table_group_tree,
                                  sai_object_id_t acl_table_group_id)
{
    sai_acl_table_group_t acl_table_group;
    acl_table_group.acl_table_group_id = acl_table_group_id;

    STD_ASSERT(sai_acl_table_group_tree != NULL);
    return ((sai_acl_table_group_t*)std_rbtree_getexact(sai_acl_table_group_tree,
                                                  &acl_table_group));
}

/**
 * @brief return the table node for given table tree.
 *
 * @param[in] table_tree Pointer to the table tree.
 * @param[in] table_id table object id.
 *
 * @return Pointer to the table node if found in tree.
 */
static inline sai_acl_table_t *sai_acl_table_find(rbtree_handle table_tree,
                                                  sai_object_id_t table_id)
{
    sai_acl_table_t tmp_acl_table;

    memset (&tmp_acl_table, 0, sizeof(sai_acl_table_t));
    tmp_acl_table.table_key.acl_table_id = table_id;

    STD_ASSERT(table_tree != NULL);
    return ((sai_acl_table_t *)std_rbtree_getexact(table_tree,
                                                   &tmp_acl_table));
}

/**
 * @brief return the counter node for given counter tree.
 *
 * @param[in] counter_tree Pointer to the counter tree.
 * @param[in] counter_id counter object id.
 *
 * @return Pointer to the counter node if found in tree.
 */
static inline sai_acl_counter_t *sai_acl_cntr_find(rbtree_handle counter_tree,
                                     sai_object_id_t counter_id)
{
    sai_acl_counter_t tmp_acl_cntr;

    memset (&tmp_acl_cntr, 0, sizeof(sai_acl_counter_t));
    tmp_acl_cntr.counter_key.counter_id = counter_id;

    STD_ASSERT(counter_tree != NULL);
    return ((sai_acl_counter_t *)std_rbtree_getexact(counter_tree,
                                                     &tmp_acl_cntr));
}
/**
 * Return the acl node global structure.
 */
acl_node_pt sai_acl_get_acl_node(void);
/**
 * @brief Accessor function for fetching the data type of attribute passed
 *
 * @param[in] attribute_id  Attribute Id
 * @return Enum value of the attribute data type
 */
sai_acl_rule_attr_type sai_acl_rule_get_attr_type (
                                        sai_attr_id_t attribute_id);

/**
 * @brief To determine the ACL Table field belongs to the UDF range
 *
 * @param[in] attribute_id  Attribute Id
 * @return Bool value: True if ACL table field belongs to UDF range, else false.
 */
static inline bool sai_acl_table_udf_field_attr_range(
                                        sai_attr_id_t attribute_id)
{
    if ((attribute_id >= SAI_ACL_TABLE_ATTR_USER_DEFINED_FIELD_GROUP_MIN) &&
        (attribute_id <= SAI_ACL_TABLE_ATTR_USER_DEFINED_FIELD_GROUP_MAX)) {
        return true;
    }

    return false;
}

/**
 * @brief To determine the ACL Rule field belongs to the UDF range
 *
 * @param[in] attribute_id  Attribute Id
 * @return Bool value: True if ACL rule field belongs to UDF range, else false.
 */
static inline bool sai_acl_rule_udf_field_attr_range(
                                        sai_attr_id_t attribute_id)
{
    if ((attribute_id >= SAI_ACL_ENTRY_ATTR_USER_DEFINED_FIELD_GROUP_MIN) &&
        (attribute_id <= SAI_ACL_ENTRY_ATTR_USER_DEFINED_FIELD_GROUP_MAX)) {
        return true;
    }

    return false;
}

/**
 * \}
 */

#endif /* __SAI_ACL_UTILS_H__ */

