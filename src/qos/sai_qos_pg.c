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
* @file  sai_qos_pg.c
*
* @brief This file contains function definitions for SAI QoS PG
*        functionality API implementation.
*
*************************************************************************/
#include "sai_npu_qos.h"
#include "sai_npu_switch.h"
#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_buffer_util.h"
#include "sai_qos_api_utils.h"
#include "sai_gen_utils.h"
#include "sai_qos_mem.h"
#include "sai_common_infra.h"
#include "sai_switch_utils.h"
#include "sai_common_infra.h"

#include "sai.h"
#include "saibuffer.h"

#include "std_type_defs.h"
#include "std_utils.h"
#include "std_assert.h"
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>


static sai_status_t sai_qos_pg_create_internal(sai_object_id_t port_id,
                                      uint_t pg_idx)
{
    dn_sai_qos_pg_t *p_pg_node = NULL;
    sai_object_id_t pg_id = 0;
    dn_sai_qos_port_t *p_port_node = NULL;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    p_pg_node = sai_qos_pg_node_alloc ();

    if(p_pg_node == NULL) {
        SAI_BUFFER_LOG_ERR ("Error unable to allocate memory for pg init");
        return SAI_STATUS_NO_MEMORY;
    }

    do {

        p_port_node = sai_qos_port_node_get(port_id);

        if(p_port_node == NULL) {
            SAI_BUFFER_LOG_ERR ("Error Invalid port Id 0x%"PRIx64" pg:%u.", port_id, pg_idx);
            sai_rc =  SAI_STATUS_INVALID_PARAMETER;
            break;
        }
        p_pg_node->port_id = port_id;

        sai_rc = sai_buffer_npu_api_get()->pg_create (p_pg_node, pg_idx, &pg_id);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Error unable to create pg node for port"
                    " 0x%"PRIx64" pg:%u.", port_id, pg_idx);
            break;
        }
        p_pg_node->key.pg_id = pg_id;

        sai_rc = sai_qos_pg_node_insert_to_tree (p_pg_node);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Error unable to insert pg node for port"
                    " 0x%"PRIx64" pg:%u.", port_id, pg_idx);
            break;
        }
        std_dll_insertatback (&p_port_node->pg_dll_head,
                              &p_pg_node->port_dll_glue);
        p_port_node->num_pg++;
    } while (0);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        sai_qos_pg_node_free(p_pg_node);
    }
    return sai_rc;
}

static sai_status_t sai_qos_pg_remove_configs(dn_sai_qos_pg_t *p_pg_node)
{
    sai_status_t sai_rc;
    sai_status_t rev_sai_rc;
    sai_object_id_t buffer_profile_id = SAI_NULL_OBJECT_ID;


    STD_ASSERT (p_pg_node != NULL);

    buffer_profile_id = p_pg_node->buffer_profile_id;

    if(buffer_profile_id != SAI_NULL_OBJECT_ID) {
        sai_rc = sai_qos_obj_update_buffer_profile(p_pg_node->key.pg_id, SAI_NULL_OBJECT_ID);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Error: Unable to remove buffer profile from pg:0x%"PRIx64"",
                    p_pg_node->key.pg_id);

            rev_sai_rc =  sai_qos_obj_update_buffer_profile(p_pg_node->key.pg_id, buffer_profile_id);

            if (rev_sai_rc != SAI_STATUS_SUCCESS) {
                SAI_BUFFER_LOG_ERR ("Error: Unable to revert buffer profile for pg:0x%"PRIx64"",
                        p_pg_node->key.pg_id);
            }
            return sai_rc;
        }
        p_pg_node->buffer_profile_id = SAI_NULL_OBJECT_ID;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_qos_pg_destroy_internal (sai_object_id_t pg_id)
{
    dn_sai_qos_pg_t *p_pg_node = NULL;
    dn_sai_qos_port_t *p_port_node = NULL;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    do {
        p_pg_node = sai_qos_pg_node_get(pg_id);
        if(p_pg_node == NULL) {
            SAI_BUFFER_LOG_ERR ("Error Unable to find PG 0x%"PRIx64"", pg_id);
            sai_rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        sai_rc = sai_qos_pg_remove_configs (p_pg_node);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Error unable to remove configs from pg:0x%"PRIx64".", pg_id);
            break;

        }
        sai_rc = sai_buffer_npu_api_get()->pg_destroy (p_pg_node);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BUFFER_LOG_ERR ("Error unable to destroy pg:0x%"PRIx64".", pg_id);
            break;
        }
        p_pg_node->key.pg_id = pg_id;

        p_port_node = sai_qos_port_node_get(p_pg_node->port_id);

        if(p_port_node == NULL) {
            sai_rc =  SAI_STATUS_INVALID_PARAMETER;
            break;
        }
        std_dll_remove (&p_port_node->pg_dll_head,
                &p_pg_node->port_dll_glue);


        sai_qos_pg_node_remove_from_tree (p_pg_node);
        sai_qos_pg_node_free(p_pg_node);
        p_port_node->num_pg--;
    } while (0);

    return sai_rc;
}

sai_status_t sai_qos_port_create_all_pg (sai_object_id_t port_id)
{
    uint_t pg_idx = 0;
    sai_status_t sai_rc;


    for(pg_idx = 0; pg_idx < sai_switch_num_pg_get(); pg_idx++) {

        sai_rc = sai_qos_pg_create_internal (port_id, pg_idx);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_QOS_LOG_CRIT ("SAI QOS Port 0x%"PRIx64" pg :%u init failed.",
                              port_id, pg_idx);
            return sai_rc;
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_port_destroy_all_pg (sai_object_id_t port_id)
{
    sai_status_t sai_rc;
    dn_sai_qos_pg_t *p_pg_node = NULL;
    dn_sai_qos_pg_t *p_next_pg_node = NULL;
    dn_sai_qos_port_t  *p_qos_port_node = sai_qos_port_node_get (port_id);
    STD_ASSERT (p_qos_port_node != NULL);

    for(p_pg_node = sai_qos_port_get_first_pg(p_qos_port_node);
        p_pg_node != NULL; p_pg_node = p_next_pg_node) {
        p_next_pg_node = sai_qos_port_get_next_pg(p_qos_port_node,p_pg_node);
        sai_rc = sai_qos_pg_destroy_internal(p_pg_node->key.pg_id);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            return sai_rc;
        }
    }

    return SAI_STATUS_SUCCESS;
}
sai_status_t sai_qos_pg_attr_set (sai_object_id_t ingress_pg_id,
                                  const sai_attribute_t *attr)
{
    dn_sai_qos_pg_t *p_pg_node = NULL;
    sai_status_t sai_rc;

    sai_qos_lock();
    p_pg_node = sai_qos_pg_node_get(ingress_pg_id);

    if (p_pg_node == NULL) {
        sai_qos_unlock ();
        SAI_BUFFER_LOG_ERR ("Object id 0x%"PRIx64" not found", ingress_pg_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    switch (attr->id) {
        case SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE:
            sai_rc = sai_qos_obj_update_buffer_profile(ingress_pg_id,
                                                        attr->value.oid);
            break;

        default:
            sai_rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            break;
    }
    sai_qos_unlock();
    return sai_rc;
}

sai_status_t sai_qos_pg_attr_get (sai_object_id_t ingress_pg_id,
                                  uint32_t attr_count,
                                  sai_attribute_t *attr_list)
{
    dn_sai_qos_pg_t *p_pg_node = NULL;
    uint_t attr_idx = 0;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    sai_qos_lock();
    p_pg_node = sai_qos_pg_node_get(ingress_pg_id);

    if (p_pg_node == NULL) {
        sai_qos_unlock ();
        SAI_BUFFER_LOG_ERR ("Object id 0x%"PRIx64" not found", ingress_pg_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        switch (attr_list[attr_idx].id) {
            case SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE:
                attr_list->value.oid = p_pg_node->buffer_profile_id;
                sai_rc = SAI_STATUS_SUCCESS;
                break;

            default:
                sai_rc = sai_get_indexed_ret_val(SAI_STATUS_UNKNOWN_ATTRIBUTE_0, attr_idx);
                break;
        }
        if(sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }
    }
    sai_qos_unlock();
    return sai_rc;
}

sai_status_t sai_qos_pg_stats_get (sai_object_id_t pg_id, uint32_t number_of_counters,
                                   const sai_ingress_priority_group_stat_t *counter_ids,
                                   uint64_t* counters)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    sai_qos_lock();

    sai_rc =  sai_buffer_npu_api_get()->pg_stats_get (pg_id, counter_ids, number_of_counters,
                                                   counters);
    sai_qos_unlock();

    return sai_rc;
}

sai_status_t sai_qos_pg_stats_clear (sai_object_id_t pg_id, uint32_t number_of_counters,
                                     const sai_ingress_priority_group_stat_t *counter_ids)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    sai_qos_lock();

    sai_rc = sai_buffer_npu_api_get()->pg_stats_clear (pg_id, counter_ids, number_of_counters);

    sai_qos_unlock();

    return sai_rc;
}
