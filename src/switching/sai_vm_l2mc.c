/************************************************************************
* * LEGALESE:   "Copyright (c) 2017, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/**
* @file sai_vm_l2mc.c
*
* @brief This file contains L2MC implementation APIs for sai-vm.
*************************************************************************/

#include "std_assert.h"
#include "saistatus.h"
#include "sai_npu_l2mc.h"
#include "sai_l2mc_common.h"
#include "sai_l2mc_api.h"
#include "sai_port_utils.h"
#include <inttypes.h>

static dn_sai_id_gen_info_t l2mc_obj_gen_info;
static dn_sai_id_gen_info_t l2mc_group_obj_gen_info;

bool sai_vm_is_l2mc_member_id_in_use(uint64_t obj_id)
{
    sai_object_id_t l2mc_member_id =
        sai_uoid_create(SAI_OBJECT_TYPE_L2MC_GROUP_MEMBER,obj_id);

    if(sai_find_l2mc_member_node(l2mc_member_id) != NULL) {
        return true;
    } else {
        return false;
    }
}

static sai_object_id_t sai_vm_l2mc_member_id_create(void)
{
    if(SAI_STATUS_SUCCESS ==
            dn_sai_get_next_free_id(&l2mc_obj_gen_info)) {
        return (sai_uoid_create(SAI_OBJECT_TYPE_L2MC_GROUP_MEMBER,
                    l2mc_obj_gen_info.cur_id));
    }
    return SAI_NULL_OBJECT_ID;
}

bool sai_vm_is_l2mc_group_id_in_use(uint64_t obj_id)
{
    sai_object_id_t l2mc_group_id =
        sai_uoid_create(SAI_OBJECT_TYPE_L2MC_GROUP,obj_id);

    if(sai_find_l2mc_group_node(l2mc_group_id) != NULL) {
        return true;
    } else {
        return false;
    }
}

static sai_object_id_t sai_vm_l2mc_group_id_create(void)
{
    if(SAI_STATUS_SUCCESS ==
            dn_sai_get_next_free_id(&l2mc_group_obj_gen_info)) {
        return (sai_uoid_create(SAI_OBJECT_TYPE_L2MC_GROUP,
                    l2mc_group_obj_gen_info.cur_id));
    }
    return SAI_NULL_OBJECT_ID;
}

static sai_status_t sai_vm_l2mc_init(void)
{
    l2mc_obj_gen_info.cur_id = 0;
    l2mc_obj_gen_info.is_wrappped = false;
    l2mc_obj_gen_info.mask = SAI_UOID_NPU_OBJ_ID_MASK;
    l2mc_obj_gen_info.is_id_in_use = sai_vm_is_l2mc_member_id_in_use;

    l2mc_group_obj_gen_info.cur_id = 0;
    l2mc_group_obj_gen_info.is_wrappped = false;
    l2mc_group_obj_gen_info.mask = SAI_UOID_NPU_OBJ_ID_MASK;
    l2mc_group_obj_gen_info.is_id_in_use = sai_vm_is_l2mc_group_id_in_use;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l2mc_group_create(dn_sai_l2mc_group_node_t *l2mc_group_node)
{

    STD_ASSERT(l2mc_group_node != NULL);

    l2mc_group_node->l2mc_group_id = sai_vm_l2mc_group_id_create();
    l2mc_group_node->port_count = 0;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l2mc_group_delete(dn_sai_l2mc_group_node_t *l2mc_group_node)
{

    STD_ASSERT(l2mc_group_node != NULL);
    if(!sai_is_obj_id_l2mc_group(l2mc_group_node->l2mc_group_id)) {
        SAI_L2MC_LOG_ERR("Wrong l2mc_group_id 0x%"PRIx64"", l2mc_group_node->l2mc_group_id);
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l2mc_member_remove(
        const dn_sai_l2mc_member_node_t *l2mc_member_node)
{

    STD_ASSERT(l2mc_member_node != NULL);
    if(!sai_is_obj_id_l2mc_member(l2mc_member_node->l2mc_member_id)) {
        SAI_L2MC_LOG_ERR("Wrong l2mc_member_id 0x%"PRIx64"", l2mc_member_node->l2mc_member_id);
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l2mc_member_create(dn_sai_l2mc_member_node_t *l2mc_member_node)
{

    STD_ASSERT(l2mc_member_node != NULL);

    l2mc_member_node->l2mc_member_id = sai_vm_l2mc_member_id_create();
    if(SAI_NULL_OBJECT_ID ==
            l2mc_member_node->l2mc_member_id) {
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_l2mc_member_lag_notif_handler(const dn_sai_l2mc_member_node_t
                                                         *l2mc_member_node,
                                                         sai_object_id_t lag_id,
                                                         uint_t port_cnt,
                                                         const sai_object_id_t *port_list,
                                                         bool is_add)
{
    return SAI_STATUS_SUCCESS;
}

void sai_vm_l2mc_group_dump_hw_info(const void *hw_info)
{
    STD_ASSERT(hw_info != NULL);
}

static sai_npu_l2mc_api_t sai_vm_l2mc_api_table = {
    sai_vm_l2mc_init,
    sai_vm_l2mc_group_create,
    sai_vm_l2mc_group_delete,
    sai_vm_l2mc_member_create,
    sai_vm_l2mc_member_remove,
    sai_vm_l2mc_member_lag_notif_handler,
    sai_vm_l2mc_group_dump_hw_info
};

sai_npu_l2mc_api_t* sai_vm_l2mc_api_query (void)
{
    return &sai_vm_l2mc_api_table;
}

