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
 * @file sai_vm_vlan.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI VLAN object in VM environment.
 */

#include "sai_vm_defs.h"
#include "sai_vm_l2_util.h"
#include "sai_switching_db_api.h"
#include "sai_switch_db_api.h"
#include "sai_oid_utils.h"
#include "sai_npu_vlan.h"
#include "sai_vlan_api.h"
#include "sai_vlan_common.h"
#include "sai_switch_utils.h"
#include "sai_port_common.h"
#include "sai_port_utils.h"
#include "saivlan.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include <string.h>
#include <inttypes.h>
#include "sai_gen_utils.h"

static dn_sai_id_gen_info_t vlan_obj_gen_info;

bool sai_vm_is_vlan_member_id_in_use(uint64_t obj_id)
{
    sai_object_id_t vlan_member_id =
        sai_uoid_create(SAI_OBJECT_TYPE_VLAN_MEMBER,obj_id);

    if(sai_find_vlan_member_node(vlan_member_id) != NULL) {
        return true;
    } else {
        return false;
    }
}

static sai_object_id_t sai_vm_vlan_member_id_create(void)
{
    if(SAI_STATUS_SUCCESS ==
            dn_sai_get_next_free_id(&vlan_obj_gen_info)) {
        return (sai_uoid_create(SAI_OBJECT_TYPE_VLAN_MEMBER,
                    vlan_obj_gen_info.cur_id));
    }
    return SAI_NULL_OBJECT_ID;
}

static sai_status_t sai_npu_vlan_member_remove(sai_vlan_member_node_t vlan_member_node)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    SAI_VLAN_LOG_TRACE("Removing port 0x%"PRIx64" to vlan %d.",
            vlan_member_node.port_id,
            sai_vlan_obj_id_to_vlan_id(vlan_member_node.vlan_id));

    /* Update the port removed from the VLAN in the DB. */
    sai_rc = sai_vlan_delete_port_list_from_db_entry(vlan_member_node);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR ("Error removing port list from DB entry for "
                "VLAN ID: %d.",
                sai_vlan_obj_id_to_vlan_id(vlan_member_node.vlan_id));

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_vlan_member_create(sai_vlan_member_node_t *vlan_member_node)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(vlan_member_node != NULL);

    SAI_VLAN_LOG_TRACE("Adding port 0x%"PRIx64" to vlan %d.",
            vlan_member_node->port_id,
            sai_vlan_obj_id_to_vlan_id(vlan_member_node->vlan_id));

    /* Update list of ports added to the VLAN in the DB. */
    sai_rc =
        sai_vlan_add_port_list_to_db_entry(*vlan_member_node);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR("Error adding port list to DB entry for "
                "VLAN ID: %d.",
                sai_vlan_obj_id_to_vlan_id(vlan_member_node->vlan_id));

        return SAI_STATUS_FAILURE;
    }

    vlan_member_node->vlan_member_id = sai_vm_vlan_member_id_create();
    if(SAI_NULL_OBJECT_ID ==
            vlan_member_node->vlan_member_id) {
        sai_npu_vlan_member_remove(*vlan_member_node);
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_vlan_create (sai_vlan_id_t vlan_id)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    SAI_VLAN_LOG_TRACE ("Creating VLAN %d.", vlan_id);

    /* Insert VLAN record to DB. */
    sai_rc = sai_vlan_create_db_entry (vlan_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR ("Error inserting VLAN entry to DB for vlan ID: "
                             "%d.", vlan_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}


static sai_status_t sai_npu_internal_vlan_init (sai_vlan_id_t vlan_id)
{
    sai_status_t ret = SAI_STATUS_FAILURE;

    SAI_VLAN_LOG_TRACE ("Initializing Internal VLAN %d.", vlan_id);

    if (!sai_is_valid_vlan_id (vlan_id)) {
        SAI_VLAN_LOG_ERR ("Invalid vlan ID %d", vlan_id);

        return SAI_STATUS_INVALID_VLAN_ID;
    }

    sai_vlan_lock ();

    /* Create the internal L3 vlan ID */
    ret = sai_npu_vlan_create (vlan_id);

    if ((ret != SAI_STATUS_SUCCESS) && (ret != SAI_STATUS_ITEM_ALREADY_EXISTS))
    {
        sai_vlan_unlock ();

        SAI_VLAN_LOG_ERR ("Unable to create vlan %d with err: %d",
                             vlan_id, ret);

        return ret;
    }

    sai_init_internal_vlan_id (vlan_id);

    sai_vlan_unlock ();

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_vlan_init (void)
{
    sai_status_t ret = SAI_STATUS_FAILURE;


    /* Internal vlan id related programming */
    ret = sai_npu_internal_vlan_init (SAI_VM_DFLT_L3_VLAN_ID);

    vlan_obj_gen_info.cur_id = 0;
    vlan_obj_gen_info.is_wrappped = false;
    vlan_obj_gen_info.mask = SAI_UOID_NPU_OBJ_ID_MASK;
    vlan_obj_gen_info.is_id_in_use = sai_vm_is_vlan_member_id_in_use;

    if (ret != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_WARN ("Internal Vlan id init failed with err %d", ret);
    }
    return ret;
}

static sai_status_t sai_npu_vlan_delete (sai_vlan_id_t vlan_id)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    SAI_VLAN_LOG_TRACE ("Deleting VLAN %d.", vlan_id);

    /* Remove VLAN record from DB. */
    sai_rc = sai_vlan_delete_db_entry (vlan_id);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR ("Error removing VLAN entry from DB for vlan ID: "
                             "%d.", vlan_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_set_vlan_max_learned_address (sai_vlan_id_t vlan_id,
                                                          uint32_t value)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    if (!sai_is_valid_vlan_id (vlan_id)) {
        SAI_VLAN_LOG_ERR ("Invalid vlan ID %d", vlan_id);

        return SAI_STATUS_INVALID_VLAN_ID;
    }

    /* Update VLAN record in DB with max learn limit info. */
    attr.id = SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES;
    attr.value.u32 = value;

    sai_rc = sai_vlan_set_db_entry (vlan_id, &attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR ("Error updating VLAN entry in DB with max learn "
                              "limit attribute for vlan ID: %d.", vlan_id);

        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_get_vlan_max_learned_address (sai_vlan_id_t vlan_id,
                                                          uint32_t *value)
{
    STD_ASSERT(value != NULL);

    if (!sai_is_valid_vlan_id (vlan_id)) {
        SAI_VLAN_LOG_ERR ("Invalid vlan ID %d", vlan_id);

        return SAI_STATUS_INVALID_VLAN_ID;
    }

    *value = sai_vlan_max_learn_adddress_cache_read(vlan_id);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_get_vlan_stats (sai_vlan_id_t vlan_id,
                                            const sai_vlan_stat_t *cntr_ids,
                                            unsigned int number_of_cntrs,
                                            uint64_t* cntrs)
{
    STD_ASSERT(cntrs != NULL);
    STD_ASSERT(cntr_ids != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_clear_vlan_stats (sai_vlan_id_t vlan_id,
                                              const sai_vlan_stat_t *cntr_ids,
                                              unsigned int number_of_cntrs)
{
    STD_ASSERT(cntr_ids != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_vlan_learn_disable_set(sai_vlan_id_t vlan_id, bool disable)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    /* Update VLAN record in DB with disable learn info */
    attr.id = SAI_VLAN_ATTR_LEARN_DISABLE;
    attr.value.booldata = disable;

    sai_rc = sai_vlan_set_db_entry (vlan_id, &attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR ("Error updating VLAN entry in DB with learn disable"
                              "for vlan ID: %d.", vlan_id);

        return SAI_STATUS_FAILURE;
    }

    SAI_VLAN_LOG_TRACE("Vlan ID:%d learn disable:%d", vlan_id, disable);
    return SAI_STATUS_SUCCESS;

}

static sai_status_t sai_npu_vlan_learn_disable_get(sai_vlan_id_t vlan_id, bool *disable)
{
    if(disable == NULL) {
        SAI_VLAN_LOG_ERR("Error invalid paramter disable is passed");
        STD_ASSERT(0);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *disable = sai_vlan_learn_disable_cache_read(vlan_id);
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_vlan_meta_data_set(sai_vlan_id_t vlan_id, uint_t value)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    /* Update VLAN record in DB with vlan meta data*/
    attr.id = SAI_VLAN_ATTR_META_DATA;
    attr.value.u32 = value;

    sai_rc = sai_vlan_set_db_entry (vlan_id, &attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_VLAN_LOG_ERR ("Error updating VLAN entry in DB with metadata"
                              "for vlan ID: %d.", vlan_id);

        return SAI_STATUS_FAILURE;
    }

    SAI_VLAN_LOG_TRACE("Vlan ID:%d metadata:%d", vlan_id, value);
    return SAI_STATUS_SUCCESS;

}

static sai_status_t sai_npu_vlan_meta_data_get(sai_vlan_id_t vlan_id, uint_t *value)
{
    if(value == NULL) {
        SAI_VLAN_LOG_ERR("Error invalid paramter is passed");
        STD_ASSERT(0);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *value = sai_vlan_meta_data_cache_read(vlan_id);
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_set_vlan_member_tagging_mode(
        sai_vlan_member_node_t vlan_member_node)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    if((rc = sai_npu_vlan_member_remove(vlan_member_node)) ==
        SAI_STATUS_SUCCESS) {
        rc = sai_npu_vlan_member_create(&vlan_member_node);
    }
    return rc;
}

static sai_npu_vlan_api_t sai_vm_vlan_api_table = {
    sai_npu_vlan_init,
    sai_npu_vlan_create,
    sai_npu_vlan_delete,
    sai_npu_vlan_member_create,
    sai_npu_vlan_member_remove,
    sai_npu_set_vlan_max_learned_address,
    sai_npu_get_vlan_max_learned_address,
    sai_npu_get_vlan_stats,
    sai_npu_clear_vlan_stats,
    sai_npu_vlan_learn_disable_set,
    sai_npu_vlan_learn_disable_get,
    sai_npu_vlan_meta_data_set,
    sai_npu_vlan_meta_data_get,
    sai_npu_set_vlan_member_tagging_mode,
};

sai_npu_vlan_api_t* sai_vm_vlan_api_query (void)
{
    return &sai_vm_vlan_api_table;
}
