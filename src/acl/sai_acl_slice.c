/************************************************************************
 * * LEGALESE:   "Copyright (c) 2017, Dell Inc. All rights reserved."
 * *
 * * This source code is confidential, proprietary, and contains trade
 * * secrets that are the sole property of Dell Inc.
 * * Copy and/or distribution of this source code or disassembly or reverse
 * * engineering of the resultant object code are strictly forbidden without
 * * the written consent of Dell Inc.
 * *
 * ************************************************************************/
 /**
 * * @file sai_acl_slice.c
 * *
 * * @brief This file contains functions for SAI ACL slice operations.
 * *
 * *
 * *************************************************************************/
#include "sai_acl_type_defs.h"
#include "sai_acl_npu_api.h"
#include "sai_common_acl.h"
#include "sai_common_utils.h"
#include "sai_acl_utils.h"
#include "sai_switch_utils.h"
#include "saitypes.h"
#include "saiacl.h"
#include "saiextensions.h"
#include "saistatus.h"

#include "std_type_defs.h"
#include "std_rbtree.h"
#include "std_llist.h"
#include "std_assert.h"
#include "sai_oid_utils.h"
#include "sai_common_infra.h"
#include <stdlib.h>
#include <inttypes.h>
static dn_sai_id_gen_info_t acl_slice_info;

bool sai_is_acl_slice_id_in_use(uint64_t obj_id)
{
    acl_node_pt acl_node = NULL;

    acl_node = sai_acl_get_acl_node();

    sai_object_id_t acl_slice_id = sai_uoid_create(
                SAI_OBJECT_TYPE_EXTENSIONS_ACL_SLICE,obj_id);

    if(sai_acl_slice_find(acl_node->sai_acl_slice_tree,
                                acl_slice_id) != NULL) {
        return true;
    } else {
        return false;
    }
}

static sai_object_id_t sai_acl_slice_id_create(void)
{
    if(SAI_STATUS_SUCCESS ==
       dn_sai_get_next_free_id(&acl_slice_info)) {
        return (sai_uoid_create(SAI_OBJECT_TYPE_EXTENSIONS_ACL_SLICE,
                                acl_slice_info.cur_id));
    }
    return SAI_NULL_OBJECT_ID;
}

static sai_status_t sai_acl_slice_node_create(  sai_acl_slice_t *acl_slice_node,
                                                uint_t slice_id,
                                                uint_t pipe_id,
                                                sai_acl_stage_t stage)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    acl_node_pt     acl_node = NULL;

    STD_ASSERT(acl_slice_node != NULL);
    sai_acl_lock();

    acl_node = sai_acl_get_acl_node();
    if(acl_node == NULL) {
        SAI_ACL_LOG_ERR ("Acl node get failed");
        sai_acl_unlock();
        return SAI_STATUS_FAILURE;
    }

    /** Slice id starts with '1' */
    acl_slice_node->slice_id = slice_id;
    acl_slice_node->pipe_id = pipe_id;
    acl_slice_node->acl_stage = stage;
    acl_slice_node->slice_depth = sai_acl_fp_slice_depth_get(stage,slice_id);
    acl_slice_node->acl_slice_id = sai_acl_slice_id_create();
    if(acl_slice_node->acl_slice_id == SAI_NULL_OBJECT_ID)
    {
        sai_rc = SAI_STATUS_FAILURE;
        sai_acl_unlock();
        return sai_rc;
    }

    if (sai_acl_slice_insert(acl_node->sai_acl_slice_tree, acl_slice_node) != STD_ERR_OK){
        SAI_ACL_LOG_ERR("Slice id insertion failed in RB tree");
        sai_rc = SAI_STATUS_FAILURE;
        sai_acl_unlock();
        return sai_rc;
    }

    sai_acl_unlock();
    return sai_rc;
}
void sai_acl_slice_init(void)
{
    acl_slice_info.cur_id = 0;
    acl_slice_info.is_wrappped = false;
    acl_slice_info.mask = SAI_UOID_NPU_OBJ_ID_MASK;
    acl_slice_info.is_id_in_use = sai_is_acl_slice_id_in_use;
}

sai_status_t sai_acl_slice_create_objects(void)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    sai_acl_slice_t *acl_slice_node = NULL;
    uint_t          max_ing_acl_slice, max_egr_acl_slice, max_pipes;
    int             ing_slice_id, egr_slice_id, pipe_id;

    max_ing_acl_slice = sai_acl_max_ifp_slice_get();
    max_egr_acl_slice = sai_acl_max_efp_slice_get();
    max_pipes = sai_switch_max_tiles_get();

    /**Create Ingress ACL TCAM slice objects */
    for (pipe_id=0; pipe_id<max_pipes; pipe_id++)
    {
        for(ing_slice_id=0; ing_slice_id<max_ing_acl_slice; ing_slice_id++)
        {
            acl_slice_node = sai_acl_slice_node_alloc();
            if (acl_slice_node == NULL)
            {
                SAI_ACL_LOG_ERR("Slice id %d node allocation failed", ing_slice_id);
                break;
            }

            sai_rc = sai_acl_slice_node_create(acl_slice_node, ing_slice_id, pipe_id,
                                                SAI_ACL_STAGE_INGRESS);

            if (sai_rc != SAI_STATUS_SUCCESS) {
                sai_acl_slice_free(acl_slice_node);
                break;
            }
        }
    }

    /**Create Egress ACL TCAM slice objects */
    for (pipe_id=0; pipe_id<max_pipes; pipe_id++)
    {
        for (egr_slice_id = 0; egr_slice_id<max_egr_acl_slice; egr_slice_id++)
        {
            acl_slice_node = sai_acl_slice_node_alloc();
            if (acl_slice_node == NULL)
            {
                SAI_ACL_LOG_ERR("Slice id %d node allocation failed", egr_slice_id);
                break;
            }

            sai_rc = sai_acl_slice_node_create(acl_slice_node, egr_slice_id, pipe_id,
                                                SAI_ACL_STAGE_EGRESS);

            if (sai_rc != SAI_STATUS_SUCCESS) {
                sai_acl_slice_free(acl_slice_node);
                break;
            }
        }
    }

    return sai_rc;
}

sai_status_t sai_acl_slice_attribute_get(sai_object_id_t acl_slice_id,
                                            uint32_t attr_count,
                                            sai_attribute_t *attr_list)
{
    STD_ASSERT(attr_list != NULL);
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;

    sai_rc = sai_acl_npu_api_get()->get_acl_slice_attribute(acl_slice_id , attr_count, attr_list);
    if(sai_rc == SAI_STATUS_SUCCESS) {
        SAI_ACL_LOG_TRACE("Acl slice attr get success for 0x%"PRIx64"",
            acl_slice_id);
    }

    return sai_rc;
}

sai_status_t sai_get_acl_slice_objects(sai_attribute_t *attr)
{
    STD_ASSERT(attr != NULL);
    uint_t acl_slice_total_count = 0, index = 0;
    acl_node_pt  acl_node = NULL;
    sai_acl_slice_t *acl_slice_node = NULL;
    sai_object_list_t *objlist = NULL;

    acl_slice_total_count = (sai_acl_max_ifp_slice_get()
                                * sai_switch_max_tiles_get())
                                + (sai_acl_max_efp_slice_get()
                                * sai_switch_max_tiles_get());

    objlist = (sai_object_list_t *)&attr->value.objlist;
    if (objlist->count < acl_slice_total_count)
    {
        SAI_ACL_LOG_TRACE ("No enough buffer to get ACL slice objects. "
                            "in slice-count %u out slice-count %u",
                            objlist->count, acl_slice_total_count);
        objlist->count = acl_slice_total_count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    objlist->count = acl_slice_total_count;
    sai_acl_lock();

    acl_node = sai_acl_get_acl_node();
    if(acl_node == NULL) {
        SAI_ACL_LOG_ERR ("Acl node get failed");
        sai_acl_unlock();
        return SAI_STATUS_FAILURE;
    }

    acl_slice_node = (sai_acl_slice_t *)
        std_rbtree_getfirst(acl_node->sai_acl_slice_tree);
    if (acl_slice_node == NULL) {
        SAI_ACL_LOG_ERR ("No ACL slice node present "
                "in the ACL Slice RB tree");
        sai_acl_unlock();
        return SAI_STATUS_FAILURE;
    }

    while(acl_slice_node && (index<acl_slice_total_count)) {

        objlist->list[index++] = acl_slice_node->acl_slice_id;
        acl_slice_node =(sai_acl_slice_t *)
            std_rbtree_getnext(acl_node->sai_acl_slice_tree, acl_slice_node);
    }

    sai_acl_unlock();
    SAI_ACL_LOG_TRACE ("Successfully returned all ACL slice objects");
    return SAI_STATUS_SUCCESS;
}

