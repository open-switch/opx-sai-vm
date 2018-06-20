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
* @file sai_l3_next_hop_group.c
*
* @brief This file contains function definitions for the SAI next hop
*        group functionality API.
*
*************************************************************************/

#include "sai_l3_common.h"
#include "sai_l3_util.h"
#include "sai_l3_api.h"
#include "saistatus.h"
#include "sainexthopgroup.h"
#include "saitypes.h"
#include "std_assert.h"
#include "std_llist.h"

#include "sai_l3_mem.h"
#include "sai_common_infra.h"
#include "sai_l3_api_utils.h"
#include "sai_l3_next_hop_group_utl.h"
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

static dn_sai_id_gen_info_t next_hop_grp_member_gen_info;

bool sai_fib_is_next_hop_grp_member_id_in_use(uint64_t obj_id)
{
    sai_object_id_t next_hop_grp_member_id =
        sai_uoid_create(SAI_OBJECT_TYPE_NEXT_HOP_GROUP_MEMBER,obj_id);
    sai_object_id_t nh_grp_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t nh_id = SAI_NULL_OBJECT_ID;

    if(sai_next_hop_map_get_ids_from_member_id(next_hop_grp_member_id,
                &nh_grp_id,&nh_id) == SAI_STATUS_SUCCESS) {
        return true;
    } else {
        return false;
    }
}

static sai_object_id_t sai_fib_generate_next_hop_grp_member_id(void)
{
    if(SAI_STATUS_SUCCESS ==
            dn_sai_get_next_free_id(&next_hop_grp_member_gen_info)) {
        return (sai_uoid_create(SAI_OBJECT_TYPE_NEXT_HOP_GROUP_MEMBER,
                    next_hop_grp_member_gen_info.cur_id));
    }
    return SAI_NULL_OBJECT_ID;
}

inline void sai_fib_next_hop_grp_member_gen_info_init(void)
{
    next_hop_grp_member_gen_info.cur_id = 0;
    next_hop_grp_member_gen_info.is_wrappped = false;
    next_hop_grp_member_gen_info.mask = SAI_UOID_NPU_OBJ_ID_MASK;
    next_hop_grp_member_gen_info.is_id_in_use =
        sai_fib_is_next_hop_grp_member_id_in_use;
}

static inline void sai_fib_nh_group_log_error (sai_fib_nh_group_t *p_group,
                                               char *p_error_str)
{
    SAI_NH_GROUP_LOG_ERR ("%s NH Group Id: 0x%"PRIx64", NH Group Type: %s, "
                          "NH Count: %d, Ref Count: %d.", p_error_str,
                          p_group->key.group_id, sai_fib_nh_group_type_str (
                          p_group->type), p_group->nh_count,
                          p_group->ref_count);
}

static inline void sai_fib_nh_group_log_trace (sai_fib_nh_group_t *p_group,
                                               char *p_trace_str)
{
    SAI_NH_GROUP_LOG_TRACE ("%s NH Group Id: 0x%"PRIx64", NH Group Type: %s, "
                            "NH Count: %d, Ref Count: %d.", p_trace_str,
                            p_group->key.group_id,
                            sai_fib_nh_group_type_str (p_group->type),
                            p_group->nh_count, p_group->ref_count);
}

static bool sai_fib_next_hop_group_type_validate (
                                              sai_next_hop_group_type_t type)
{
    switch (type) {

        case SAI_NEXT_HOP_GROUP_TYPE_ECMP:
            return true;

        default:
            return false;
    }
}

static inline bool sai_fib_is_nh_group_used (sai_fib_nh_group_t *p_group)
{
    return (p_group->ref_count ? true : false);
}

static inline bool sai_fib_is_group_type_attr_set (uint_t attr_flags)
{
    return ((attr_flags & SAI_FIB_NH_GROUP_TYPE_ATTR_FLAG) ? true : false);
}

static inline bool sai_fib_is_group_nh_list_attr_set (uint_t attr_flags)
{
    return ((attr_flags & SAI_FIB_NH_GROUP_NH_LIST_ATTR_FLAG) ? true : false);
}

static bool sai_fib_is_nh_group_size_valid (sai_fib_nh_group_t *p_nh_group,
                                            uint_t next_hop_count)

{
    STD_ASSERT (p_nh_group != NULL);

    if ((p_nh_group->nh_count + next_hop_count) >
         sai_fib_max_ecmp_paths_get ()) {

        SAI_NH_GROUP_LOG_ERR ("Next Hop Group size: %d exceeds max ecmp-paths"
                              ": %d.", (p_nh_group->nh_count + next_hop_count),
                              sai_fib_max_ecmp_paths_get ());

        return false;
    }

    return true;
}

static sai_fib_wt_link_node_t *sai_fib_nh_add_group_link_node (
                                                sai_fib_nh_t *p_next_hop,
                                                sai_fib_nh_group_t *p_nh_group)
{
    sai_fib_wt_link_node_t *p_group_link_node = NULL;

    STD_ASSERT (p_next_hop != NULL);
    STD_ASSERT (p_nh_group != NULL);

    /* Check if the group is already present in the list */
    p_group_link_node = sai_fib_nh_find_group_link_node (p_next_hop, p_nh_group);

    if (p_group_link_node) {

        p_group_link_node->weight++;

        SAI_NH_GROUP_LOG_TRACE ("Group is part of Next Hop's group list already"
                                ", Group Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64","
                                "Weight: %d.", p_nh_group->key.group_id,
                                p_next_hop->next_hop_id,
                                p_group_link_node->weight);

        return p_group_link_node;
    }

    /* Create a new group link node to insert into the NH Group list */
    p_group_link_node = sai_fib_weighted_link_node_alloc ();

    if ((!p_group_link_node)) {

        SAI_NH_GROUP_LOG_ERR ("Memory alloc for NH Group link node failed. "
                              "Group Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64".",
                              p_nh_group->key.group_id, p_next_hop->next_hop_id);

        return NULL;
    }

    p_group_link_node->link_node.self = (void *) p_nh_group;

    std_dll_insertatback (&p_next_hop->nh_group_list,
                          &p_group_link_node->link_node.dll_glue);

    p_group_link_node->weight++;

    SAI_NH_GROUP_LOG_TRACE ("Added Next Hop Group link node in NH. "
                            "Group Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64", "
                            "Weight: %d.", p_nh_group->key.group_id,
                            p_next_hop->next_hop_id, p_group_link_node->weight);

    return p_group_link_node;
}

static void sai_fib_nh_remove_group_link_node (
                                    sai_fib_nh_t *p_nh_node,
                                    sai_fib_wt_link_node_t *p_group_link_node)
{
    sai_fib_nh_group_t     *p_nh_group = NULL;

    STD_ASSERT (p_nh_node != NULL);
    STD_ASSERT (p_group_link_node != NULL);

    STD_ASSERT (p_group_link_node->weight > 0);

    if (p_group_link_node->weight > 0) {

        p_group_link_node->weight--;

        p_nh_group = (sai_fib_nh_group_t *) p_group_link_node->link_node.self;

        if (!p_group_link_node->weight) {

            /* Remove the group link node from NH Group list and free the node */
            std_dll_remove (&p_nh_node->nh_group_list,
                            &p_group_link_node->link_node.dll_glue);

            sai_fib_weighted_link_node_free (p_group_link_node);

            SAI_NH_GROUP_LOG_TRACE ("Removed Next Hop Group link node from NH. "
                                    "Group Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64".",
                                    p_nh_group->key.group_id,
                                    p_nh_node->next_hop_id);
        } else {

            SAI_NH_GROUP_LOG_TRACE ("Decremented Next Hop Group link node weight. "
                                    "Group Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64""
                                    ", Weight: %d.", p_nh_group->key.group_id,
                                    p_nh_node->next_hop_id,
                                    p_group_link_node->weight);
        }
    }
}

static void sai_fib_nh_find_and_remove_group_node (sai_fib_nh_t *p_nh_node,
                                                   sai_fib_nh_group_t *p_nh_group)
{
    sai_fib_wt_link_node_t  *p_group_link_node = NULL;

    STD_ASSERT (p_nh_node != NULL);
    STD_ASSERT (p_nh_group != NULL);

    /* Check if the group node is present in next hop's group list */
    p_group_link_node = sai_fib_nh_find_group_link_node (p_nh_node, p_nh_group);

    if (p_group_link_node) {

        /* Remove the group node from next hop's group list */
        sai_fib_nh_remove_group_link_node (p_nh_node, p_group_link_node);

    } else {

        SAI_NH_GROUP_LOG_TRACE ("Group is not part of Next Hop's group list. "
                                "Group Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64".",
                                p_nh_group->key.group_id,
                                p_nh_node->next_hop_id);
    }
}

static sai_fib_wt_link_node_t *sai_fib_nh_group_add_nh_link_node (
                                                sai_fib_nh_group_t *p_nh_group,
                                                sai_fib_nh_t *p_next_hop)
{
    sai_fib_wt_link_node_t *p_nh_link_node = NULL;

    STD_ASSERT (p_next_hop != NULL);
    STD_ASSERT (p_nh_group != NULL);

    /* Check if the next hop is already present in the NH list */
    p_nh_link_node = sai_fib_nh_group_find_nh_link_node (p_nh_group, p_next_hop);

    if (p_nh_link_node) {

        /* Application can achieve WCMP by adding same next hop more than once
         * in NH Group. So, increment weight for the next hop link node. */

        p_nh_link_node->weight++;

        SAI_NH_GROUP_LOG_TRACE ("Next Hop is part of NH Group's NH list already. "
                                "Group Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64","
                                "Weight: %d.", p_nh_group->key.group_id,
                                p_next_hop->next_hop_id, p_nh_link_node->weight);
        return p_nh_link_node;
    }

    /* Create a new next hop link node to insert into the NH list */
    p_nh_link_node = sai_fib_weighted_link_node_alloc ();

    if ((!p_nh_link_node)) {

        SAI_NH_GROUP_LOG_ERR ("Memory alloc for Next Hop link node failed. "
                              "Group Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64".",
                              p_nh_group->key.group_id, p_next_hop->next_hop_id);

        return NULL;
    }

    p_nh_link_node->link_node.self = (void *) p_next_hop;

    std_dll_insertatback (&p_nh_group->nh_list,
                          &p_nh_link_node->link_node.dll_glue);

    p_nh_link_node->weight++;

    sai_fib_nh_group_dep_encap_nh_update (p_nh_group, p_next_hop, true);

    SAI_NH_GROUP_LOG_TRACE ("Added Next Hop link node in NH Group. "
                            "Group Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64", "
                            "Weight: %d.", p_nh_group->key.group_id,
                            p_next_hop->next_hop_id, p_nh_link_node->weight);

    return p_nh_link_node;
}

static void sai_fib_nh_group_remove_nh_link_node (
                                         sai_fib_nh_group_t *p_nh_group,
                                         sai_fib_wt_link_node_t *p_nh_link_node)
{
    sai_fib_nh_t   *p_nh_node = NULL;

    STD_ASSERT (p_nh_group != NULL);
    STD_ASSERT (p_nh_link_node != NULL);

    STD_ASSERT (p_nh_link_node->weight > 0);

    if (p_nh_link_node->weight > 0) {

        /* Application can remove next hop from a WCMP Group by removing it one
         * by one. So, deccrement weight for the next hop link node and remove
         * the node when weight becomes zero. */

        p_nh_link_node->weight--;

        p_nh_node = (sai_fib_nh_t *) p_nh_link_node->link_node.self;

        if (!p_nh_link_node->weight) {

            /* Remove the next hop link node from NH list and free the node */
            std_dll_remove (&p_nh_group->nh_list,
                            &p_nh_link_node->link_node.dll_glue);

            sai_fib_weighted_link_node_free (p_nh_link_node);

            sai_fib_nh_group_dep_encap_nh_update (p_nh_group, p_nh_node, false);

            SAI_NH_GROUP_LOG_TRACE ("Removed Next Hop link node from NH Group. "
                                    "Group Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64".",
                                    p_nh_group->key.group_id,
                                    p_nh_node->next_hop_id);
        } else {

            SAI_NH_GROUP_LOG_TRACE ("Decremented Next Hop link node weight. "
                                    "Group Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64""
                                    ", Weight: %d.", p_nh_group->key.group_id,
                                    p_nh_node->next_hop_id,
                                    p_nh_link_node->weight);
        }

    }
}

static void sai_fib_nh_group_find_and_remove_nh_node (
                                                sai_fib_nh_group_t *p_nh_group,
                                                sai_fib_nh_t *p_nh_node)
{
    sai_fib_wt_link_node_t *p_nh_link_node = NULL;

    STD_ASSERT (p_nh_group != NULL);
    STD_ASSERT (p_nh_node != NULL);

    /* Check if the next hop is present in the group's NH list */
    p_nh_link_node = sai_fib_nh_group_find_nh_link_node (p_nh_group, p_nh_node);

    if (p_nh_link_node) {

        /* Remove the next hop node from group's NH list */
        sai_fib_nh_group_remove_nh_link_node (p_nh_group, p_nh_link_node);

        STD_ASSERT (p_nh_group->nh_count > 0);

        /* Decrement the nh list count */
        p_nh_group->nh_count--;

    } else {

        SAI_NH_GROUP_LOG_TRACE ("Next Hop is not part of NH Group's list. "
                                "Group Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64".",
                                p_nh_group->key.group_id, p_nh_node->next_hop_id);
    }
}

static void sai_fib_nh_group_remove_from_lists (
                                           sai_fib_nh_group_t *p_group_node,
                                           uint_t next_hop_count,
                                           sai_fib_nh_t *ap_next_hop[])
{
    uint_t      nh_index;

    SAI_NH_GROUP_LOG_TRACE ("NH Group Id: 0x%"PRIx64", Next Hop Count: %d, Removing "
                            "from list.", p_group_node->key.group_id,
                            next_hop_count);

    for (nh_index = 0; nh_index < next_hop_count; nh_index++)
    {
        /* Remove the group node in next hop's group list */
        sai_fib_nh_find_and_remove_group_node (ap_next_hop [nh_index],
                                               p_group_node);

        /* Remove the next hop node in group's nh list */
        sai_fib_nh_group_find_and_remove_nh_node (p_group_node,
                                                  ap_next_hop [nh_index]);

        sai_fib_decr_nh_group_ref_count (p_group_node);
        sai_fib_decr_nh_ref_count (ap_next_hop [nh_index]);
    }
}

static sai_status_t sai_fib_nh_group_add_in_lists (
                                           sai_fib_nh_group_t *p_group_node,
                                           uint_t next_hop_count,
                                           sai_fib_nh_t *ap_next_hop[])
{
    uint_t                  nh_index;
    sai_fib_wt_link_node_t *p_nh_link_node = NULL;
    sai_fib_wt_link_node_t *p_group_link_node = NULL;

    STD_ASSERT (p_group_node != NULL);
    STD_ASSERT (ap_next_hop != NULL);

    SAI_NH_GROUP_LOG_TRACE ("NH Group Id: 0x%"PRIx64", Next Hop Count: %d, Adding in "
                            "list.", p_group_node->key.group_id, next_hop_count);

    for (nh_index = 0; nh_index < next_hop_count; nh_index++)
    {
        /* Insert the group node in next hop's group list */
        p_group_link_node = sai_fib_nh_add_group_link_node (ap_next_hop [nh_index],
                                                            p_group_node);
        if ((!p_group_link_node)) {

            break;
        }

        /* Insert the next hop node in group's nh list */
        p_nh_link_node = sai_fib_nh_group_add_nh_link_node (p_group_node,
                                                    ap_next_hop [nh_index]);

        if ((!p_nh_link_node)) {

            /* Remove the group node from next hop's group list */
            sai_fib_nh_remove_group_link_node (ap_next_hop [nh_index],
                                               p_group_link_node);
            break;
        }

        /* Increment the nh list count */
        p_group_node->nh_count++;
        sai_fib_incr_nh_group_ref_count (p_group_node);
        sai_fib_incr_nh_ref_count (ap_next_hop [nh_index]);
    }

    if (nh_index && (nh_index != next_hop_count)) {

        SAI_NH_GROUP_LOG_ERR ("Failure to add Next Hop Id: 0x%"PRIx64" in "
                              "NH Group list.", ap_next_hop [nh_index]->next_hop_id);

        /* Cleanup the nodes added */
        sai_fib_nh_group_remove_from_lists (p_group_node, nh_index, ap_next_hop);

        return SAI_STATUS_NO_MEMORY;

    } else {

        return SAI_STATUS_SUCCESS;
    }
}

static sai_status_t sai_fib_next_hop_fill_from_nh_id_list (
                                        sai_fib_nh_group_t *p_group_node,
                                        uint_t next_hop_count,
                                        const sai_object_id_t *p_next_hop_id,
                                        sai_fib_nh_t *ap_next_hop_node [],
                                        bool is_remove)
{
    uint_t                  nh_index;
    sai_fib_nh_t           *p_nh_node;
    sai_fib_wt_link_node_t *p_nh_link_node = NULL;

    STD_ASSERT (p_group_node != NULL);
    STD_ASSERT (p_next_hop_id != NULL);
    STD_ASSERT (ap_next_hop_node != NULL);

    if ((!next_hop_count)) {

        SAI_NH_GROUP_LOG_ERR ("Invalid Next Hop list count: %d, Group Id: 0x%"PRIx64".",
                              next_hop_count, p_group_node->key.group_id);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    SAI_NH_GROUP_LOG_TRACE ("SAI scanning input Next Hop Id list of count: %d.",
                            next_hop_count);

    /* Scan through the input next hop id array */
    for (nh_index = 0; nh_index < next_hop_count; nh_index++)
    {
        if (!sai_is_obj_id_next_hop (p_next_hop_id [nh_index])) {

            SAI_NH_GROUP_LOG_ERR ("Invalid SAI object id passed.");

            return SAI_STATUS_INVALID_OBJECT_TYPE;
        }

        /* Get the next hop node from id */
        p_nh_node = sai_fib_next_hop_node_get_from_id (
                                                 p_next_hop_id [nh_index]);
        if ((!p_nh_node)) {

            SAI_NH_GROUP_LOG_ERR ("Next Hop node not found for id: 0x%"PRIx64".",
                                  p_next_hop_id [nh_index]);

            return SAI_STATUS_INVALID_OBJECT_ID;
        }

        if (is_remove) {

            /* Check if the next hop is added in the NH Group */
            p_nh_link_node = sai_fib_nh_group_find_nh_link_node (p_group_node,
                                                                 p_nh_node);

            if ((!p_nh_link_node)) {

                SAI_NH_GROUP_LOG_ERR ("Next Hop not found in NH Group's list"
                                      ", Group Id: 0x%"PRIx64", Next Hop Id: 0x%"PRIx64".",
                                      p_group_node->key.group_id,
                                      p_nh_node->next_hop_id);

                return SAI_STATUS_INVALID_OBJECT_ID;
            }
        }

        ap_next_hop_node [nh_index] = p_nh_node;

        SAI_NH_GROUP_LOG_TRACE ("SAI filled Next Hop Id: 0x%"PRIx64" from Next Hop Id "
                                "list.", p_next_hop_id [nh_index]);
    }

    return SAI_STATUS_SUCCESS;
}

/*
 * This is a helper routine for parsing attribute list input. Caller must
 * recalculate the error code with the attribute's index in the list.
 */
static sai_status_t sai_fib_next_hop_group_type_set (
                                          sai_fib_nh_group_t *p_nh_group,
                                          const sai_attribute_value_t *p_value,
                                          bool is_create_req)
{
    sai_next_hop_group_type_t  group_type;

    STD_ASSERT (p_nh_group != NULL);
    STD_ASSERT (p_value != NULL);

    if (!is_create_req) {

        SAI_NH_GROUP_LOG_ERR ("Next Hop Group type create-only attr is set.");

        return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    group_type = (sai_next_hop_group_type_t) p_value->s32;

    if (!(sai_fib_next_hop_group_type_validate (group_type))) {

        SAI_NH_GROUP_LOG_ERR ("Invalid Next Hop Group attribute value: %d.",
                              group_type);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_nh_group->type = group_type;

    SAI_NH_GROUP_LOG_TRACE ("SAI set Next Hop Group type: %s.",
                            sai_fib_nh_group_type_str (group_type));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_next_hop_group_info_fill (
                              sai_fib_nh_group_t *p_nh_group,
                              uint_t attr_count, const sai_attribute_t *attr_list,
                              bool is_create, sai_fib_nh_t *ap_next_hop [])
{
    sai_status_t           status = SAI_STATUS_FAILURE;
    uint_t                 attr_index;
    uint_t                 attr_flag = 0;
    const sai_attribute_t *p_attr = NULL;

    STD_ASSERT (p_nh_group != NULL);
    STD_ASSERT (attr_list != NULL);

    if ((!attr_count)) {

        SAI_NH_GROUP_LOG_ERR ("SAI Next Hop Group fill from attr list. "
                              "Invalid input. attr_count: %d.", attr_count);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (attr_index = 0; attr_index < attr_count; attr_index++)
    {
        p_attr = &attr_list [attr_index];
        status = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;

        switch (p_attr->id) {

            case SAI_NEXT_HOP_GROUP_ATTR_TYPE:
                status = sai_fib_next_hop_group_type_set (p_nh_group,
                                     &p_attr->value, is_create);

                attr_flag |= SAI_FIB_NH_GROUP_TYPE_ATTR_FLAG;
                break;

            case SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST:
                SAI_NH_GROUP_LOG_ERR ("Next Hop Member List read-only attr is set.");

                status = SAI_STATUS_INVALID_ATTRIBUTE_0;
                break;

            case SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT:
                SAI_NH_GROUP_LOG_ERR ("Next Hop Count read-only attr is set.");

                status = SAI_STATUS_INVALID_ATTRIBUTE_0;
                break;

            default:
                SAI_NH_GROUP_LOG_ERR ("Invalid atttribute id: %d.", p_attr->id);
                break;
        }

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NH_GROUP_LOG_ERR ("Failure in Next Hop Group attr list. "
                                  "Index: %d, Attribute Id: %d, Error: %d.",
                                  attr_index, p_attr->id, status);

            return (sai_fib_attr_status_code_get (status, attr_index));
        }
    }

    if ((is_create) && (!sai_fib_is_group_type_attr_set (attr_flag))) {

        /* Mandatory for create */
        SAI_NH_GROUP_LOG_ERR ("Mandatory Next Hop Group %s attribute missing.",
                              (!sai_fib_is_group_type_attr_set (attr_flag)) ?
                              "Type" : "Next hop List");

        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    sai_fib_nh_group_log_trace (p_nh_group, "SAI Next Hop Group fill "
                                "attributes success.");

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fib_next_hop_group_member_get_info (
                                    uint_t                 attr_count,
                                    const sai_attribute_t *attr_list,
                                    sai_object_id_t       *p_out_nh_grp_id,
                                    uint_t                *p_in_out_nh_id_count,
                                    sai_object_id_t       *pa_out_nh_id)
{
    sai_status_t           status = SAI_STATUS_FAILURE;
    uint_t                 attr_index;
    const sai_attribute_t *p_attr = NULL;
    bool                   nh_grp_id_present = false;
    bool                   nh_id_present = false;
    uint_t                 index = 0;

    STD_ASSERT (attr_list != NULL);

    if ((!attr_count)) {

        SAI_NH_GROUP_LOG_ERR ("SAI NH Group Member get info from attr list. "
                              "Invalid input. attr_count: %d.", attr_count);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (attr_index = 0; attr_index < attr_count; attr_index++)
    {
        p_attr = &attr_list [attr_index];
        status = SAI_STATUS_SUCCESS;

        switch (p_attr->id) {

            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_GROUP_ID:
                if ((nh_grp_id_present) &&
                    (*p_out_nh_grp_id != p_attr->value.oid)) {
                    status = SAI_STATUS_INVALID_PARAMETER;
                    break;
                }

                nh_grp_id_present = true;
                *p_out_nh_grp_id = p_attr->value.oid;
                break;

            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID:
                if (index >= *p_in_out_nh_id_count) {
                    status = SAI_STATUS_BUFFER_OVERFLOW;
                    break;
                }

                nh_id_present = true;
                pa_out_nh_id [index] = p_attr->value.oid;
                index++;
                break;

            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_WEIGHT:
                status = SAI_STATUS_NOT_SUPPORTED;
                break;

            default:
                SAI_NH_GROUP_LOG_ERR ("Invalid atttribute id: %d.", p_attr->id);
                status = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NH_GROUP_LOG_ERR ("Failure in Next Hop Group Member attr list. "
                                  "Index: %d, Attribute Id: %d, Error: %d.",
                                  attr_index, p_attr->id, status);

            return (sai_fib_attr_status_code_get (status, attr_index));
        }
    }

    if ((!nh_grp_id_present) || (!nh_id_present)){
        SAI_NH_GROUP_LOG_ERR ("Mandatory Next Hop Group Member attribute "
                              "missing. NH Grp Id %spresent, NH Id %spresent",
                              (nh_grp_id_present) ? "" : "NOT ",
                              (nh_id_present) ? "" : "NOT ");

        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    *p_in_out_nh_id_count = index;

    SAI_NH_GROUP_LOG_TRACE ("SAI Next Hop Group Member get "
                            "attributes success.");

    return SAI_STATUS_SUCCESS;
}

static void sai_fib_nh_group_remove_all_nh (sai_fib_nh_group_t *p_nh_group)
{
    STD_ASSERT (p_nh_group != NULL);

    sai_fib_wt_link_node_t *p_nh_link_node = NULL;
    sai_fib_wt_link_node_t *p_next_nh_link_node = NULL;
    sai_fib_wt_link_node_t *p_group_link_node = NULL;
    sai_fib_nh_t           *p_nh_node = NULL;

    sai_fib_nh_group_log_trace (p_nh_group, "Removing all next hops from "
                                "NH Group.");

    for (p_nh_link_node = sai_fib_get_first_nh_from_nh_group (p_nh_group);
         p_nh_link_node != NULL; p_nh_link_node = p_next_nh_link_node)
    {
        /* Copy the next nh link node */
        p_next_nh_link_node =
            sai_fib_get_next_nh_from_nh_group (p_nh_group, p_nh_link_node);

        p_nh_node = sai_fib_get_nh_from_dll_link_node (&p_nh_link_node->link_node);

        /* Set weight to 1 to remove the weighted link node */
        p_nh_link_node->weight = 1;

        sai_fib_nh_group_remove_nh_link_node (p_nh_group, p_nh_link_node);

        /* Remove the group node from next hop's group list */
        p_group_link_node = sai_fib_nh_find_group_link_node (p_nh_node, p_nh_group);

        if (p_group_link_node) {

            /* Set weight to 1 to remove the weighted link node */
            p_group_link_node->weight = 1;

            sai_fib_nh_remove_group_link_node (p_nh_node, p_group_link_node);
        }
    }

    /* Reset the nh list count to 0 */
    p_nh_group->nh_count = 0;
}

static inline sai_fib_nh_t** sai_fib_nh_list_alloc (uint_t nh_count)
{
    return ((sai_fib_nh_t **) calloc (nh_count, sizeof (sai_fib_nh_t *)));
}

static inline void sai_fib_nh_list_free (sai_fib_nh_t **p_nh_list)
{
    free ((void *) p_nh_list);
}

static sai_status_t sai_fib_next_hop_group_create (
                                  sai_object_id_t *p_next_hop_group_id,
                                  sai_object_id_t switch_id,
                                  uint32_t attr_count,
                                  const sai_attribute_t *attr_list)
{
    sai_status_t         status = SAI_STATUS_FAILURE;
    sai_fib_nh_group_t  *p_nh_group_node = NULL;
    t_std_error          rc;
    sai_npu_object_id_t  nh_group_hw_id;

    SAI_NH_GROUP_LOG_TRACE ("SAI Next Hop Group creation, attr_count: %d.",
                            attr_count);

    STD_ASSERT (p_next_hop_group_id != NULL);

    p_nh_group_node = sai_fib_nh_group_node_alloc();

    if ((!p_nh_group_node)) {

        SAI_NH_GROUP_LOG_ERR ("Failed to allocate memory for Next Hop Group "
                              "node.");
        return SAI_STATUS_NO_MEMORY;
    }

    std_dll_init (&p_nh_group_node->nh_list);

    std_dll_init (&p_nh_group_node->dep_encap_nh_list);

    sai_fib_lock ();

    do {
        /* Fill group attributes from the input list */
        status = sai_fib_next_hop_group_info_fill (p_nh_group_node, attr_count,
                                            attr_list, true, NULL);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NH_GROUP_LOG_ERR ("SAI Next Hop Group creation. "
                                  "Error in attribute list.");
            break;
        }

        /* Create the Next Hop Group in NPU */
        status = sai_nh_group_npu_api_get()->nh_group_create (p_nh_group_node,
                                        &nh_group_hw_id);

        if (status != SAI_STATUS_SUCCESS) {

            sai_fib_nh_group_log_error (p_nh_group_node, "Failed to create "
                                        "Next Hop Group in NPU.");
            break;
        }

        (*p_next_hop_group_id) = sai_uoid_create (SAI_OBJECT_TYPE_NEXT_HOP_GROUP,
                                                  nh_group_hw_id);

        p_nh_group_node->key.group_id = (*p_next_hop_group_id);

        /* Insert into the Next Hop Group Id global view */
        rc = std_rbtree_insert (sai_fib_access_global_config()->nh_group_tree,
                                p_nh_group_node);
        if (STD_IS_ERR (rc)) {

            SAI_NH_GROUP_LOG_ERR ("Failed to insert Next hop group Id: 0x%"PRIx64" "
                                  "in rbtree. Error: %d", p_nh_group_node->key.group_id, rc);

            sai_nh_group_npu_api_get()->nh_group_remove (p_nh_group_node);

            break;
        }
    } while (0);

    if (status != SAI_STATUS_SUCCESS) {

        if (p_nh_group_node->nh_count) {

            sai_fib_nh_group_remove_all_nh (p_nh_group_node);
        }

        sai_fib_nh_group_node_free (p_nh_group_node);

    } else {

        sai_fib_nh_group_log_trace (p_nh_group_node, "SAI Next Hop Group created.");

        SAI_NH_GROUP_LOG_INFO ("Created Next Hop Group Id: 0x%"PRIx64".",
                               (*p_next_hop_group_id));
    }

    sai_fib_unlock ();

    return status;
}

static sai_status_t sai_fib_next_hop_group_remove (
                                          sai_object_id_t nh_group_id)
{
    sai_status_t        status = SAI_STATUS_FAILURE;
    sai_fib_nh_group_t *p_nh_group_node = NULL;

    SAI_NH_GROUP_LOG_TRACE ("SAI Next Hop Group deletion, nh_group_id: 0x%"PRIx64".",
                            nh_group_id);

    if (!sai_is_obj_id_next_hop_group (nh_group_id)) {
        SAI_NH_GROUP_LOG_ERR ("0x%"PRIx64" is not a valid Next Hop Group obj id.",
                              nh_group_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    sai_fib_lock ();

    do {
        /* Get the next hop group node */
        p_nh_group_node = sai_fib_next_hop_group_get (nh_group_id);

        if ((!p_nh_group_node)) {

            SAI_NH_GROUP_LOG_ERR ("Next Hop Group Id not found: 0x%"PRIx64".",
                                  nh_group_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }

        sai_fib_nh_group_log_trace (p_nh_group_node,
                                    "Before Next Hop Group remove.");

        /* Check if the nh group is pointed by another object */
        if (sai_fib_is_nh_group_used (p_nh_group_node)) {

            sai_fib_nh_group_log_error (p_nh_group_node, "Next Hop Group node "
                                        "is being used.");

            status = SAI_STATUS_OBJECT_IN_USE;

            break;
        }

        /* Remove it in hardware */
        status = sai_nh_group_npu_api_get()->nh_group_remove (p_nh_group_node);

        if (status != SAI_STATUS_SUCCESS) {

            sai_fib_nh_group_log_error (p_nh_group_node, "Failed to remove"
                                        " Next Hop Group in NPU.");
            break;
        }
    } while (0);

    if (status == SAI_STATUS_SUCCESS) {

        if (p_nh_group_node->nh_count) {

            sai_fib_nh_group_log_trace (p_nh_group_node, "Next Hop Group NH "
                                        "list is not empty.");

            sai_fib_nh_group_remove_all_nh (p_nh_group_node);
        }

        /* Remove the next hop group id from SAI database */
        std_rbtree_remove (sai_fib_access_global_config()->nh_group_tree,
                           p_nh_group_node);

        sai_fib_nh_group_log_trace (p_nh_group_node,
                                    "SAI Next Hop Group deleted.");

        /* Free the next hop group node */
        sai_fib_nh_group_node_free (p_nh_group_node);

        SAI_NH_GROUP_LOG_INFO ("Removed Next Hop Group Id: 0x%"PRIx64".",
                               nh_group_id);
    }

    sai_fib_unlock ();

    return status;
}

static sai_status_t sai_fib_next_hop_add_to_group (
                                        sai_object_id_t nh_group_id,
                                        uint32_t next_hop_count,
                                        const sai_object_id_t *p_next_hop_id)
{
    sai_status_t        status = SAI_STATUS_FAILURE;
    sai_fib_nh_group_t  *p_nh_group_node = NULL;
    sai_fib_nh_t        **ap_next_hop_node = NULL;
    bool                 is_added_in_list = false;

    SAI_NH_GROUP_LOG_TRACE ("SAI Add Next Hop to Group, nh_group_id: 0x%"PRIx64", "
                            "next_hop_count: %d.", nh_group_id, next_hop_count);

    if (!sai_is_obj_id_next_hop_group (nh_group_id)) {
        SAI_NH_GROUP_LOG_ERR ("0x%"PRIx64" is not a valid Next Hop Group obj id.",
                              nh_group_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    ap_next_hop_node = sai_fib_nh_list_alloc (sai_fib_max_ecmp_paths_get());
    if (ap_next_hop_node == NULL) {
        SAI_NH_GROUP_LOG_ERR ("Failed to allocate memory for Next Hop node list");

        return SAI_STATUS_NO_MEMORY;
    }

    do {
        /* Get the next hop group node */
        p_nh_group_node = sai_fib_next_hop_group_get (nh_group_id);

        if ((!p_nh_group_node)) {
            SAI_NH_GROUP_LOG_ERR ("Next Hop Group Id not found: 0x%"PRIx64".",
                                  nh_group_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }
        sai_fib_nh_group_log_trace (p_nh_group_node,
                                    "Before Add Next Hop to Group.");
        /* Validate the next hop count with max ecmp-paths attr */
        if (!sai_fib_is_nh_group_size_valid (p_nh_group_node, next_hop_count)) {

            status = SAI_STATUS_INSUFFICIENT_RESOURCES;

            break;
        }

        /* Scan through the input next hop id list */
        status = sai_fib_next_hop_fill_from_nh_id_list (p_nh_group_node,
                                              next_hop_count, p_next_hop_id,
                                              ap_next_hop_node, false);
        if (status != SAI_STATUS_SUCCESS) {

            SAI_NH_GROUP_LOG_ERR ("Error in filling Next Hop list. "
                                  "nh_group_id: 0x%"PRIx64".", nh_group_id);

            break;
        }

        status = sai_fib_nh_group_add_in_lists (p_nh_group_node, next_hop_count,
                                                ap_next_hop_node);
        if (status != SAI_STATUS_SUCCESS) {

            SAI_NH_GROUP_LOG_ERR ("Failure to add NH Group node in lists.");

            break;
        }

        is_added_in_list = true;

        /* Update the next hop group in NPU */
        status = sai_nh_group_npu_api_get()->add_nh_to_group (p_nh_group_node,
                                      next_hop_count, ap_next_hop_node);
        if (status != SAI_STATUS_SUCCESS) {

            sai_fib_nh_group_log_error (p_nh_group_node, "SAI Add Next Hop to "
                                        "Group failed in NPU.");
            break;

        }
    } while (0);

    if (status != SAI_STATUS_SUCCESS) {

        if (is_added_in_list) {

            sai_fib_nh_group_remove_from_lists (p_nh_group_node, next_hop_count,
                                                ap_next_hop_node);
        }
    } else {
        sai_fib_nh_group_log_trace (p_nh_group_node, "SAI Add Next Hop to Group done.");
        SAI_NH_GROUP_LOG_INFO ("Added Next Hop to Next Hop Group Id: 0x%"PRIx64".",
                               nh_group_id);
    }

    sai_fib_nh_list_free (ap_next_hop_node);

    return status;
}

static sai_status_t sai_fib_next_hop_remove_from_group (
                                        sai_object_id_t nh_group_id,
                                        uint32_t next_hop_count,
                                        const sai_object_id_t *p_next_hop_id)
{
    sai_status_t        status = SAI_STATUS_FAILURE;
    sai_fib_nh_group_t  *p_nh_group_node = NULL;
    sai_fib_nh_t        **ap_next_hop_node = NULL;

    SAI_NH_GROUP_LOG_TRACE ("SAI Remove Next Hop from Group, nh_group_id: 0x%"PRIx64", "
                            "next_hop_count: %d.", nh_group_id, next_hop_count);

    if (!sai_is_obj_id_next_hop_group (nh_group_id)) {
        SAI_NH_GROUP_LOG_ERR ("0x%"PRIx64" is not a valid Next Hop Group obj id.",
                              nh_group_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    ap_next_hop_node = sai_fib_nh_list_alloc (sai_fib_max_ecmp_paths_get());
    if (ap_next_hop_node == NULL) {

        SAI_NH_GROUP_LOG_ERR ("Failed to allocate memory for Next Hop node list");

        return SAI_STATUS_NO_MEMORY;
    }

    do {
        /* Get the next hop group node */
        p_nh_group_node = sai_fib_next_hop_group_get (nh_group_id);

        if ((!p_nh_group_node)) {

            SAI_NH_GROUP_LOG_ERR ("Next Hop Group Id not found: 0x%"PRIx64".",
                                  nh_group_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;

            break;
        }

        sai_fib_nh_group_log_trace (p_nh_group_node,
                                    "Before Remove Next Hop from Group.");

        /* Scan through the input next hop id list */
        status = sai_fib_next_hop_fill_from_nh_id_list (p_nh_group_node,
                                           next_hop_count, p_next_hop_id,
                                           ap_next_hop_node, true);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NH_GROUP_LOG_ERR ("Error in filling Next Hop list. "
                                  "nh_group_id: 0x%"PRIx64".", nh_group_id);

            break;
        }

        /* Update the next hop group in NPU */
        status = sai_nh_group_npu_api_get()->remove_nh_from_group (p_nh_group_node,
                                             next_hop_count, ap_next_hop_node);

        if (status != SAI_STATUS_SUCCESS) {

            sai_fib_nh_group_log_error (p_nh_group_node, "SAI Remove Next Hop "
                                        "from Group failed in NPU.");

            break;
        }
    } while (0);

    if (status == SAI_STATUS_SUCCESS) {

        sai_fib_nh_group_remove_from_lists (p_nh_group_node, next_hop_count,
                                            ap_next_hop_node);

        sai_fib_nh_group_log_trace (p_nh_group_node,
                                    "SAI Remove Next Hop from Group done.");
        SAI_NH_GROUP_LOG_INFO ("Removed Next Hop from Next Hop Group Id: "
                               "0x%"PRIx64".", nh_group_id);
    }

    sai_fib_nh_list_free (ap_next_hop_node);

    return status;
}

static sai_status_t sai_fib_next_hop_group_attribute_set (
                                           sai_object_id_t nh_group_id,
                                           const sai_attribute_t *p_attr)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

static sai_status_t sai_next_hop_get_member_list (sai_object_list_t *p_obj_list,
                                                  sai_object_id_t    nh_grp_id)
{
    sai_status_t rc;
    uint32_t     nh_count = 0;

    rc = sai_next_hop_map_get_count (nh_grp_id, &nh_count);

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }

    if (p_obj_list->count < nh_count) {
        p_obj_list->count = nh_count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    rc = sai_next_hop_map_get_member_list (nh_grp_id,
                                           &nh_count, p_obj_list->list);

    p_obj_list->count = nh_count;
    return rc;
}

static sai_status_t sai_fib_next_hop_group_attribute_get (
                                                 sai_object_id_t  nh_group_id,
                                                 uint32_t         attr_count,
                                                 sai_attribute_t *p_attr_list)
{
    sai_status_t        status = SAI_STATUS_FAILURE;
    sai_fib_nh_group_t  *p_nh_group_node = NULL;
    sai_attribute_t *p_attr;
    uint_t           attr_index;

    SAI_NH_GROUP_LOG_TRACE ("SAI Next Hop Group Get Attribute, "
                            "nh_group_id: 0x%"PRIx64".", nh_group_id);

    if (!sai_is_obj_id_next_hop_group (nh_group_id)) {
        SAI_NH_GROUP_LOG_ERR ("0x%"PRIx64" is not a valid Next Hop Group obj id.",
                              nh_group_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if ((!attr_count)) {

        SAI_NH_GROUP_LOG_ERR ("SAI Next Hop Group Get attribute. Invalid input."
                              " attr_count: %d.", attr_count);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT (p_attr_list != NULL);

    sai_fib_lock ();

    do {
        p_nh_group_node = sai_fib_next_hop_group_get (nh_group_id);

        if ((!p_nh_group_node)) {
            SAI_NH_GROUP_LOG_ERR ("Next Hop Group Id not found: 0x%"PRIx64".",
                                  nh_group_id);

            status = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        for (attr_index = 0; attr_index < attr_count; attr_index++) {
            p_attr = &p_attr_list [attr_index];
            status = SAI_STATUS_SUCCESS;

            switch (p_attr->id) {
                case SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT:
                    p_attr->value.s32 = p_nh_group_node->nh_count;
                    break;

                case SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST:
                    status = sai_next_hop_get_member_list (&p_attr->value.objlist,
                                                           nh_group_id);
                    break;

                case SAI_NEXT_HOP_GROUP_ATTR_TYPE:
                    p_attr->value.s32 = p_nh_group_node->type;
                    break;

                default:
                    status = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                    break;
            }

            if (status != SAI_STATUS_SUCCESS) {
                status = sai_fib_attr_status_code_get (status, attr_index);
                break;
            }
        }
    } while (0);

    if (status == SAI_STATUS_SUCCESS) {
        SAI_NH_GROUP_LOG_TRACE ("SAI Next Hop Group Get Attribute success.");
    }
    else {
        SAI_NH_GROUP_LOG_ERR ("SAI Next Hop Group Get Attribute failed.");
    }

    sai_fib_unlock ();

    return status;
}

static sai_status_t sai_fib_next_hop_group_member_create (
        sai_object_id_t       *out_member_id,
        sai_object_id_t switch_id,
        uint32_t               attr_count,
        const sai_attribute_t *attr_list)
{
    sai_status_t        rc = SAI_STATUS_SUCCESS;
    sai_object_id_t     nh_grp_id = 0;
    sai_object_id_t     nh_id = 0;
    uint_t              nh_id_count = 1;

    sai_fib_lock ();

    do
    {
        rc = sai_fib_next_hop_group_member_get_info (attr_count, attr_list,
                                                     &nh_grp_id, &nh_id_count,
                                                     &nh_id);

        if (rc != SAI_STATUS_SUCCESS) {
            break;
        }

        rc = sai_fib_next_hop_add_to_group (nh_grp_id, nh_id_count, &nh_id);

        if (rc != SAI_STATUS_SUCCESS) {
            break;
        }

        *out_member_id = sai_fib_generate_next_hop_grp_member_id();
        if(SAI_NULL_OBJECT_ID == *out_member_id) {
            rc = SAI_STATUS_FAILURE;
            break;
        }

        rc = sai_next_hop_map_insert (nh_grp_id, nh_id, *out_member_id);
    } while (0);

    sai_fib_unlock ();
    return rc;
}

static sai_status_t sai_fib_next_hop_group_member_remove (sai_object_id_t member_id)
{
    sai_status_t    rc = SAI_STATUS_SUCCESS;
    sai_object_id_t nh_grp_id;
    sai_object_id_t nh_id;

    sai_fib_lock ();

    do {
        if (!sai_is_obj_id_next_hop_group_member (member_id)) {
            SAI_NH_GROUP_LOG_ERR ("0x%"PRIx64" is not a valid Next Hop "
                                  "Group Member obj id.", member_id);

            rc = SAI_STATUS_INVALID_OBJECT_TYPE;
            break;
        }

        rc = sai_next_hop_map_get_ids_from_member_id (member_id,
                                                      &nh_grp_id, &nh_id);

        if (rc != SAI_STATUS_SUCCESS) {
            break;
        }

        rc = sai_fib_next_hop_remove_from_group (nh_grp_id, 1, &nh_id);

        if (rc != SAI_STATUS_SUCCESS) {
            break;
        }

        sai_next_hop_map_remove (nh_grp_id, nh_id, member_id);
    } while (0);

    sai_fib_unlock ();

    return rc;
}

static sai_status_t sai_fib_next_hop_group_member_attribute_set (
                                              sai_object_id_t        member_id,
                                              const sai_attribute_t *p_attr)
{
    sai_status_t status = SAI_STATUS_SUCCESS;

    STD_ASSERT (p_attr != NULL);

    sai_fib_lock ();

    do {
        status = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;

        switch (p_attr->id) {

            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_GROUP_ID:
            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID:
                status = SAI_STATUS_INVALID_PARAMETER;
                break;

            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_WEIGHT:
                status = SAI_STATUS_NOT_SUPPORTED;
                break;

            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_CONFIGURED_ROLE:
                status = SAI_STATUS_NOT_SUPPORTED;
                break;

            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_OBSERVED_ROLE:
                status = SAI_STATUS_NOT_SUPPORTED;
                break;

            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_MONITORED_OBJECT:
                status = SAI_STATUS_NOT_SUPPORTED;
                break;

            default:
                SAI_NH_GROUP_LOG_ERR ("Invalid atttribute id: %d.",
                                      p_attr->id);
                break;
        }

        if (status != SAI_STATUS_SUCCESS) {

            SAI_NH_GROUP_LOG_ERR ("Failure in NH Group Member attr list. "
                                  "Attribute Id: %d, Error: %d.",
                                  p_attr->id, status);
        }
    } while (0);

    SAI_NH_GROUP_LOG_TRACE("SAI Next Hop Group Member set attributes success.");

    sai_fib_unlock ();
    return status;
}

static sai_status_t sai_fib_next_hop_group_member_attribute_get (
                                                   sai_object_id_t  member_id,
                                                   uint32_t         attr_count,
                                                   sai_attribute_t *p_attr_list)
{
    sai_status_t     status = SAI_STATUS_SUCCESS;
    sai_object_id_t  nh_grp_id;
    sai_object_id_t  nh_id;
    sai_attribute_t *p_attr;
    uint_t           attr_index;

    SAI_NH_GROUP_LOG_TRACE ("SAI NH Group Member Get Attribute");

    if ((!attr_count)) {

        SAI_NH_GROUP_LOG_ERR("SAI NH Group Member Get attribute. Invalid input."
                             " attr_count: %d.", attr_count);

        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT (p_attr_list != NULL);

    sai_fib_lock ();

    do {
        status = sai_next_hop_map_get_ids_from_member_id (member_id,
                                                          &nh_grp_id, &nh_id);

        if (status != SAI_STATUS_SUCCESS) {
            break;
        }

        for (attr_index = 0; attr_index < attr_count; attr_index++) {
            p_attr = &p_attr_list [attr_index];

            switch (p_attr->id) {
                case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_GROUP_ID:
                    p_attr->value.oid = nh_grp_id;
                    break;

                case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID:
                    p_attr->value.oid = nh_id;
                    break;

                case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_WEIGHT:
                    status = SAI_STATUS_NOT_SUPPORTED;
                    break;

                case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_CONFIGURED_ROLE:
                    status = SAI_STATUS_NOT_SUPPORTED;
                    break;

                case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_OBSERVED_ROLE:
                    status = SAI_STATUS_NOT_SUPPORTED;
                    break;

                case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_MONITORED_OBJECT:
                    status = SAI_STATUS_NOT_SUPPORTED;
                    break;

                default:
                    break;
            }

            if (status != SAI_STATUS_SUCCESS) {
                break;
            }
        }
    } while (0);

    if (status == SAI_STATUS_SUCCESS) {
        SAI_NH_GROUP_LOG_TRACE ("SAI NH Group Member Get Attribute success.");

    } else {
        SAI_NH_GROUP_LOG_ERR ("SAI NH Group Member Get Attribute failed.");
    }

    sai_fib_unlock ();

    return status;
}

static sai_status_t sai_fib_next_hop_group_member_create_bulk (
                                          sai_object_id_t         switch_id,
                                          uint32_t                object_count,
                                          const uint32_t         *attr_count,
                                          const sai_attribute_t **attrs,
                                          sai_bulk_op_error_mode_t type,
                                          sai_object_id_t        *object_id,
                                          sai_status_t           *object_statuses)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

static sai_status_t sai_fib_next_hop_group_member_remove_bulk (
                                          uint32_t               object_count,
                                          const sai_object_id_t *object_id,
                                          sai_bulk_op_error_mode_t type,
                                          sai_status_t          *object_statuses)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

static sai_next_hop_group_api_t sai_next_hop_group_method_table = {
    sai_fib_next_hop_group_create,
    sai_fib_next_hop_group_remove,
    sai_fib_next_hop_group_attribute_set,
    sai_fib_next_hop_group_attribute_get,
    sai_fib_next_hop_group_member_create,
    sai_fib_next_hop_group_member_remove,
    sai_fib_next_hop_group_member_attribute_set,
    sai_fib_next_hop_group_member_attribute_get,
    sai_fib_next_hop_group_member_create_bulk,
    sai_fib_next_hop_group_member_remove_bulk,
};

sai_next_hop_group_api_t *sai_nexthop_group_api_query (void)
{
    return &sai_next_hop_group_method_table;
}

