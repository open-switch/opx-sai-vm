/************************************************************************
 * * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
 * *
 * * This source code is confidential, proprietary, and contains trade
 * * secrets that are the sole property of Dell Inc.
 * * Copy and/or distribution of this source code or disassembly or reverse
 * * engineering of the resultant object code are strictly forbidden without
 * * the written consent of Dell Inc.
 * *
 * ************************************************************************/
/**
 * * @file sai_acl_init.c
 * *
 * * @brief This file contains functions to initialize SAI ACL
 * *        data structures.
 * *
 * *************************************************************************/
#include "sai_acl_utils.h"
#include "sai_common_acl.h"
#include "saistatus.h"
#include "sai_modules_init.h"
#include "sai_acl_type_defs.h"
#include "std_rbtree.h"
#include "std_struct_utils.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include "std_mutex_lock.h"
#include <stdlib.h>
#include <string.h>

static sai_acl_table_id_node_t sai_acl_table_id_generator[SAI_ACL_TABLE_ID_MAX];
static std_mutex_lock_create_static_init_fast(acl_lock);
static sai_acl_api_t sai_acl_method_table =
{
    sai_create_acl_table,
    sai_delete_acl_table,
    sai_set_acl_table,
    sai_get_acl_table,
    sai_create_acl_rule,
    sai_delete_acl_rule,
    sai_set_acl_rule,
    sai_get_acl_rule,
    sai_create_acl_counter,
    sai_delete_acl_counter,
    sai_set_acl_cntr,
    sai_get_acl_cntr,
    sai_create_acl_range,
    sai_delete_acl_range,
    sai_set_acl_range,
    sai_get_acl_range,
    sai_acl_table_group_create,
    sai_acl_table_group_delete,
    sai_acl_table_group_attribute_set,
    sai_acl_table_group_attribute_get,
    sai_acl_table_group_member_create,
    sai_acl_table_group_member_delete,
    sai_acl_table_group_member_attribute_set,
    sai_acl_table_group_member_attribute_get,

};

/**************************************************************************
 *                          ACCESSOR FUNCTION
 **************************************************************************/

sai_acl_api_t* sai_acl_api_query(void)
{
    return (&sai_acl_method_table);
}

sai_acl_table_id_node_t *sai_acl_get_table_id_gen(void)
{
    return sai_acl_table_id_generator;
}

void sai_acl_lock(void)
{
    std_mutex_lock (&acl_lock);
}

void sai_acl_unlock(void)
{
    std_mutex_unlock (&acl_lock);
}

static void sai_acl_table_id_generate(void)
{
    uint_t table_idx = 0;

    memset(&sai_acl_table_id_generator, 0,
           sizeof(sai_acl_table_id_node_t)*SAI_ACL_TABLE_ID_MAX);

    for (table_idx = 0; table_idx < SAI_ACL_TABLE_ID_MAX; table_idx++) {
        sai_acl_table_id_generator[table_idx].table_in_use = false;
        sai_acl_table_id_generator[table_idx].table_id = table_idx + 1;
    }
}

static void sai_acl_cleanup(void)
{
    acl_node_pt acl_node = sai_acl_get_acl_node();

    if (acl_node->sai_acl_table_tree) {
            std_rbtree_destroy(acl_node->sai_acl_table_tree);
    }

    if (acl_node->sai_acl_rule_tree) {
        std_rbtree_destroy(acl_node->sai_acl_rule_tree);
    }

    if (acl_node->sai_acl_counter_tree) {
        std_rbtree_destroy(acl_node->sai_acl_counter_tree);
    }

    if (acl_node->sai_acl_table_group_tree) {
        std_rbtree_destroy(acl_node->sai_acl_table_group_tree);
    }

    if (acl_node->sai_acl_table_group_member_tree) {
        std_rbtree_destroy(acl_node->sai_acl_table_group_member_tree);
    }


    if(acl_node->sai_acl_range_tree)
    {
        std_rbtree_destroy(acl_node->sai_acl_range_tree);
    }

    memset(acl_node, 0, sizeof(sai_acl_node_t));
}

sai_status_t sai_acl_init(void)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    acl_node_pt acl_node = sai_acl_get_acl_node();

    do {
        acl_node->sai_acl_table_tree = std_rbtree_create_simple(
                        "acl_table_tree",
                        STD_STR_OFFSET_OF(sai_acl_table_t, table_key),
                        STD_STR_SIZE_OF(sai_acl_table_t, table_key));

        if (acl_node->sai_acl_table_tree == NULL) {
            SAI_ACL_LOG_CRIT ("Creation of ACL Table tree node failed");
            rc = SAI_STATUS_UNINITIALIZED;
            break;
        }

        acl_node->sai_acl_rule_tree = std_rbtree_create_simple(
                                     "acl_rule_tree",
                                     STD_STR_OFFSET_OF(sai_acl_rule_t, rule_key),
                                     STD_STR_SIZE_OF(sai_acl_rule_t, rule_key));

        if (acl_node->sai_acl_rule_tree == NULL) {
            SAI_ACL_LOG_CRIT ("Creation of ACL Rule tree node failed");
            rc = SAI_STATUS_UNINITIALIZED;
            break;
        }

        acl_node->sai_acl_counter_tree = std_rbtree_create_simple(
                             "acl_counter_tree",
                              STD_STR_OFFSET_OF(sai_acl_counter_t, counter_key),
                              STD_STR_SIZE_OF(sai_acl_counter_t, counter_key));

        if (acl_node->sai_acl_counter_tree == NULL) {
            SAI_ACL_LOG_CRIT ("Creation of ACL Counter tree node failed");
            rc = SAI_STATUS_UNINITIALIZED;
            break;
        }
        acl_node->sai_acl_table_group_tree = std_rbtree_create_simple(
                             "acl_table_group_tree",
                              STD_STR_OFFSET_OF(sai_acl_table_group_t, acl_table_group_id),
                              STD_STR_SIZE_OF(sai_acl_table_group_t, acl_table_group_id));

        if (acl_node->sai_acl_table_group_tree == NULL) {
            SAI_ACL_LOG_CRIT ("Creation of ACL table group tree node failed");
            rc = SAI_STATUS_UNINITIALIZED;
            break;
        }
        acl_node->sai_acl_table_group_member_tree = std_rbtree_create_simple(
                             "acl_table_group_member_tree",
                              STD_STR_OFFSET_OF(sai_acl_table_group_member_t,
                                                acl_table_group_member_id),
                              STD_STR_SIZE_OF(sai_acl_table_group_member_t,
                                              acl_table_group_member_id));

        if (acl_node->sai_acl_table_group_member_tree == NULL) {
            SAI_ACL_LOG_CRIT ("Creation of ACL table group member tree node failed");
            rc = SAI_STATUS_UNINITIALIZED;
            break;
        }

        acl_node->sai_acl_range_tree = std_rbtree_create_simple(
                             "acl_table_range_tree",
                              STD_STR_OFFSET_OF(sai_acl_range_t,
                                               acl_range_id),
                              STD_STR_SIZE_OF(sai_acl_range_t,
                                              acl_range_id));

        if (acl_node->sai_acl_range_tree == NULL) {
            SAI_ACL_LOG_CRIT ("Creation of ACL range tree failed");
            rc = SAI_STATUS_UNINITIALIZED;
            break;
        }

        sai_acl_table_id_generate();

        sai_acl_counter_init();

        sai_acl_table_group_init();

        sai_acl_table_group_member_init();

        sai_acl_range_init();
    } while(0);

    if (rc != SAI_STATUS_SUCCESS) {
        sai_acl_cleanup();
    }

    SAI_ACL_LOG_INFO ("SAI ACL Data structures Init complete");

    return rc;
}
