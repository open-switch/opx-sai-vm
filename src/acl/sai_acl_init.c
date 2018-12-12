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
#include "sai_npu_switch.h"
#include "sai_switch_init_config.h"
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
    sai_acl_slice_attribute_get,
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

    if(acl_node->sai_acl_slice_tree)
    {
        std_rbtree_destroy(acl_node->sai_acl_slice_tree);
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


        acl_node->sai_acl_slice_tree = std_rbtree_create_simple(
                            "acl_slice_tree",
                            STD_STR_OFFSET_OF(sai_acl_slice_t,
                                              acl_slice_id),
                            STD_STR_SIZE_OF(sai_acl_slice_t,
                                            acl_slice_id));

        if (acl_node->sai_acl_slice_tree == NULL) {
            SAI_ACL_LOG_CRIT ("Creation of ACL slice tree failed");
            rc = SAI_STATUS_UNINITIALIZED;
            break;
        }

        sai_acl_table_id_generate();

        sai_acl_counter_init();

        sai_acl_table_group_init();

        sai_acl_table_group_member_init();

        sai_acl_range_init();

        sai_acl_slice_init();

        sai_acl_slice_create_objects();
    } while(0);

    if (rc != SAI_STATUS_SUCCESS) {
        sai_acl_cleanup();
    }

    SAI_ACL_LOG_INFO ("SAI ACL Data structures Init complete");

    return rc;
}

static sai_status_t sai_acl_config_slice_depth_handler(
                                                std_config_node_t acl_xml_node,
                                                uint_t *depth_list)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    std_config_node_t sai_node = NULL;
    sai_uint32_t slice_id = 0, slice_depth = 0;

    STD_ASSERT(depth_list != NULL);

    SAI_ACL_LOG_TRACE("ACL_USAGE: Slice depth handling");
    for (sai_node = std_config_get_child(acl_xml_node);
            sai_node != NULL;
            sai_node = std_config_next_node(sai_node)) {

        if(strncmp(std_config_name_get(sai_node),
                    SAI_NODE_NAME_ACL_FP_SLICE, SAI_MAX_NAME_LEN) == 0)
        {
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_FP_SLICE_ID,
                    &slice_id,0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_FP_SLICE_DEPTH,
                    &slice_depth,0);

            if (&depth_list[slice_id] != NULL)
                depth_list[slice_id] = slice_depth;
        }
    }
    return sai_rc;
}

sai_status_t sai_acl_config_handler(std_config_node_t acl_xml_node,
                                    acl_node_pt acl_node)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    std_config_node_t sai_node = NULL;
    sai_acl_table_static_config_t *acl_config;

    if (acl_node == NULL) {
        SAI_ACL_LOG_ERR ("ACL node data is NULL");
        return SAI_STATUS_FAILURE;
    }

    acl_config = &acl_node->sai_acl_table_config;
    memset(acl_config, 0, sizeof(sai_acl_table_static_config_t));

    for (sai_node = std_config_get_child(acl_xml_node);
            sai_node != NULL;
            sai_node = std_config_next_node(sai_node)) {

        /**Handle ACL priority config*/
        if(strncmp(std_config_name_get(sai_node), SAI_NODE_NAME_ACL_PRIO,
                    SAI_MAX_NAME_LEN) == 0)
        {
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_SYS_FLOW,
                    &acl_config->priority[SAI_ACL_INGRESS_SYSTEM_FLOW],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_OPENFLOW,
                    &acl_config->priority[SAI_ACL_INGRESS_OPENFLOW],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_VLT,
                    &acl_config->priority[SAI_ACL_INGRESS_VLT],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_ISCSI,
                    &acl_config->priority[SAI_ACL_INGRESS_ISCSI],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_FCOE,
                    &acl_config->priority[SAI_ACL_INGRESS_FCOE],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_FCOE_FPORT,
                    &acl_config->priority[SAI_ACL_INGRESS_FCOE_FPORT],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_FEDGOV,
                    &acl_config->priority[SAI_ACL_INGRESS_FEDGOV],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_L2_ACL,
                    &acl_config->priority[SAI_ACL_INGRESS_L2_ACL],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_V4_ACL,
                    &acl_config->priority[SAI_ACL_INGRESS_V4_ACL],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_V6_ACL,
                    &acl_config->priority[SAI_ACL_INGRESS_V6_ACL],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_V4_PBR,
                    &acl_config->priority[SAI_ACL_INGRESS_V4_PBR],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_V6_PBR,
                    &acl_config->priority[SAI_ACL_INGRESS_V6_PBR],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_L2_QOS,
                    &acl_config->priority[SAI_ACL_INGRESS_L2_QOS],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_V4_QOS,
                    &acl_config->priority[SAI_ACL_INGRESS_V4_QOS],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_INGRESS_V6_QOS,
                    &acl_config->priority[SAI_ACL_INGRESS_V6_QOS],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_EGRESS_L2_ACL,
                    &acl_config->priority[SAI_ACL_EGRESS_L2_ACL],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_EGRESS_V4_ACL,
                    &acl_config->priority[SAI_ACL_EGRESS_V4_ACL],0);
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_EGRESS_V6_ACL,
                    &acl_config->priority[SAI_ACL_EGRESS_V6_ACL],0);
        }

        /**Handle ACL ingress FP config*/
        if(strncmp(std_config_name_get(sai_node),
                    SAI_NODE_NAME_ACL_INGRESS_FP, SAI_MAX_NAME_LEN) == 0)
        {
            SAI_ACL_LOG_TRACE("ACL_USAGE: Processing ACL ingress fp");
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_MAX_IFP_SLICE,
                    &acl_config->max_ifp_slice,0);

            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_DEPTH_PER_ENTRY,
                    &acl_config->depth_per_entry,0);

            acl_config->ifp_slice_depth_list = calloc(acl_config->max_ifp_slice,
                                                        sizeof(uint_t));
            if (acl_config->ifp_slice_depth_list == NULL)
                return SAI_STATUS_NO_MEMORY;

            sai_rc = sai_acl_config_slice_depth_handler(sai_node,
                        acl_config->ifp_slice_depth_list);
            if (sai_rc != SAI_STATUS_SUCCESS) {
                //LOG
            }
        }

        if(strncmp(std_config_name_get(sai_node),
                    SAI_NODE_NAME_ACL_EGRESS_FP, SAI_MAX_NAME_LEN) == 0)
        {
            sai_std_config_attr_update(sai_node, SAI_ATTR_ACL_MAX_EFP_SLICE,
                    &acl_config->max_efp_slice,0);
            acl_config->efp_slice_depth_list = calloc(acl_config->max_efp_slice,
                                                        sizeof(uint_t));
            if (acl_config->efp_slice_depth_list == NULL)
                return SAI_STATUS_NO_MEMORY;

            sai_rc = sai_acl_config_slice_depth_handler(sai_node,
                    acl_config->efp_slice_depth_list);
            if (sai_rc != SAI_STATUS_SUCCESS) {
                //LOG
            }
        }
    }
    return sai_rc;
}

