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
* @file sai_tunnel_obj.c
*
* @brief This file contains function definitions for the SAI tunnel object
*        API functions.
*
*************************************************************************/
#include "saistatus.h"
#include "saitypes.h"
#include "saitunnel.h"
#include "sai_modules_init.h"
#include "sai_l3_api_utils.h"
#include "sai_common_infra.h"
#include "sai_tunnel_api_utils.h"
#include "sai_gen_utils.h"
#include "sai_oid_utils.h"
#include "sai_l3_util.h"
#include "sai_tunnel.h"
#include "sai_tunnel_util.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include "std_rbtree.h"
#include "std_llist.h"
#include <inttypes.h>
#include <stdlib.h>

static sai_tunnel_api_t sai_tunnel_api_method_table;

static inline dn_sai_tunnel_t *dn_sai_tunnel_object_alloc (void)
{
    return ((dn_sai_tunnel_t *) calloc (1, sizeof (dn_sai_tunnel_t)));
}

static inline void dn_sai_tunnel_object_free (dn_sai_tunnel_t *tunnel_obj)
{
    free ((void *) tunnel_obj);
}

static sai_status_t sai_tunnel_object_id_bitmap_init (void)
{
    dn_sai_tunnel_global_t *p_global_param = dn_sai_tunnel_access_global_config();

    /* Create the tunnel object index bitmap */
    p_global_param->tunnel_obj_id_bitmap =
        std_bitmap_create_array (SAI_TUNNEL_OBJ_MAX_ID);
    if (NULL == p_global_param->tunnel_obj_id_bitmap) {

        SAI_TUNNEL_LOG_CRIT ("Failed to allocate tunnel obj index bitmap.");

        return SAI_STATUS_NO_MEMORY;
    }

    /* Create the tunnel termination entry index bitmap */
    p_global_param->tunnel_term_id_bitmap =
        std_bitmap_create_array (SAI_TUNNEL_TERM_OBJ_MAX_ID);
    if (NULL == p_global_param->tunnel_term_id_bitmap) {

        SAI_TUNNEL_LOG_CRIT ("Failed to allocate tunnel termination entry index bitmap.");

        return SAI_STATUS_NO_MEMORY;
    }

    return SAI_STATUS_SUCCESS;
}

static void sai_tunnel_object_id_bitmap_deinit (void)
{
    dn_sai_tunnel_global_t *p_global_param = dn_sai_tunnel_access_global_config();

    if (p_global_param->tunnel_obj_id_bitmap != NULL) {
        std_bitmaparray_free_data (p_global_param->tunnel_obj_id_bitmap);
    }

    if (p_global_param->tunnel_term_id_bitmap != NULL) {
        std_bitmaparray_free_data (p_global_param->tunnel_term_id_bitmap);
    }
}

sai_status_t sai_tunnel_init (void)

{
    sai_status_t status = SAI_STATUS_SUCCESS;

    SAI_TUNNEL_LOG_DEBUG ("Entering SAI Tunnel Module Init.");

    dn_sai_tunnel_global_t *p_global_param = dn_sai_tunnel_access_global_config();

    if (p_global_param->is_init_complete) {

        SAI_TUNNEL_LOG_INFO ("SAI Tunnel Init done already.");

        return status;
    }

    memset (p_global_param, 0, sizeof (dn_sai_tunnel_global_t));

    do {
        status = SAI_STATUS_UNINITIALIZED;

        p_global_param->tunnel_db =
                            std_rbtree_create_simple ("tunnel_id_tree",
                                  STD_STR_OFFSET_OF (dn_sai_tunnel_t, tunnel_id),
                                  STD_STR_SIZE_OF (dn_sai_tunnel_t, tunnel_id));

        if (NULL == p_global_param->tunnel_db) {
            SAI_TUNNEL_LOG_CRIT ("Failed to allocate memory for tunnel obj db.");

            break;
        }

        p_global_param->tunnel_term_table_db =
              std_rbtree_create_simple ("tunnel_term_tree",
                   STD_STR_OFFSET_OF (dn_sai_tunnel_term_entry_t, term_entry_id),
                   STD_STR_SIZE_OF (dn_sai_tunnel_term_entry_t, term_entry_id));

        if (NULL == p_global_param->tunnel_term_table_db) {
            SAI_TUNNEL_LOG_CRIT ("Failed to allocate memory for tunnel term db.");

            break;
        }

        p_global_param->tunnel_map_db =
               std_rbtree_create_simple ("tunnel_map_tree",
                    STD_STR_OFFSET_OF (dn_sai_tunnel_map_t, map_id),
                    STD_STR_SIZE_OF (dn_sai_tunnel_map_t, map_id));

        if (NULL == p_global_param->tunnel_map_db) {
            SAI_TUNNEL_LOG_CRIT ("Failed to allocate memory for tunnel map db.");

            break;
        }

        p_global_param->tunnel_map_entry_db =
            std_rbtree_create_simple ("tunnel_map_entry_tree",
                       STD_STR_OFFSET_OF (dn_sai_tunnel_map_entry_t, tunnel_map_entry_id),
                       STD_STR_SIZE_OF (dn_sai_tunnel_map_entry_t, tunnel_map_entry_id));

        if (NULL == p_global_param->tunnel_map_entry_db) {
            SAI_TUNNEL_LOG_CRIT ("Failed to allocate memory for tunnel map entry db.");

            break;
        }

        status = sai_tunnel_object_id_bitmap_init();

        if (status != SAI_STATUS_SUCCESS) {
            break;
        }

        status = sai_tunnel_npu_api_get()->tunnel_init();

        if (status != SAI_STATUS_SUCCESS) {
            SAI_TUNNEL_LOG_CRIT ("Tunnel module NPU initialization failure.");

            break;
        }

        dn_sai_tunnel_map_init();
        dn_sai_tunnel_map_entry_init();

        p_global_param->is_init_complete = true;

    } while (0);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_TUNNEL_LOG_DEBUG ("Cleaning up SAI Tunnel global parameters.");

        sai_tunnel_deinit ();

    } else {

        SAI_TUNNEL_LOG_INFO ("SAI Tunnel module initialization done.");
    }

    return status;
}

void sai_tunnel_deinit (void)
{
    dn_sai_tunnel_global_t *p_global_param = dn_sai_tunnel_access_global_config();

    if (p_global_param->is_init_complete) {

        sai_tunnel_npu_api_get()->tunnel_deinit();
    }

    if (p_global_param->tunnel_db != NULL) {
        std_rbtree_destroy (p_global_param->tunnel_db);
    }

    if (p_global_param->tunnel_term_table_db != NULL) {
        std_rbtree_destroy (p_global_param->tunnel_term_table_db);
    }

    if (p_global_param->tunnel_map_db != NULL) {
        std_rbtree_destroy (p_global_param->tunnel_map_db);
    }

    sai_tunnel_object_id_bitmap_deinit ();

    p_global_param->is_init_complete = false;

    memset (p_global_param, 0, sizeof (dn_sai_tunnel_global_t));

    SAI_TUNNEL_LOG_INFO ("SAI Tunnel module deinitialization done.");
}

sai_status_t dn_sai_tunnel_attr_list_validate (sai_object_type_t obj_type,
                                               uint32_t attr_count,
                                               const sai_attribute_t *attr_list,
                                               dn_sai_operations_t op_type)
{
    sai_status_t  status = SAI_STATUS_FAILURE;
    const dn_sai_attribute_entry_t  *p_attr_table = NULL;
    uint_t max_attr_count = 0;
    uint_t dup_index = 0;

    if (dn_sai_check_duplicate_attr(attr_count, attr_list, &dup_index)) {

        SAI_TUNNEL_LOG_INFO ("Duplicate tunnel attribute at index %u", dup_index);

        return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0, dup_index);
    }

    sai_tunnel_npu_api_get()->attr_id_table_get (obj_type, &p_attr_table,
                                                 &max_attr_count);

    if ((p_attr_table == NULL) || (max_attr_count == 0)) {
        /* Not expected */
        SAI_TUNNEL_LOG_ERR ("Unable to get attribute id table.");

        return status;
    }

    status = sai_attribute_validate (attr_count, attr_list, p_attr_table,
                                     op_type, max_attr_count);

    if (status != SAI_STATUS_SUCCESS) {

        SAI_TUNNEL_LOG_INFO ("Attr list validation failed. op-type: %d, Error: %d.",
                             op_type, status);
    } else {

        SAI_TUNNEL_LOG_INFO ("Attr list validation passed. op-type: %d.",
                             op_type);
    }

    return status;
}

static sai_status_t dn_sai_tunnel_type_set (dn_sai_tunnel_t *p_tunnel_obj,
                                            const sai_attribute_t  *p_attr)
{
    if ((p_attr->value.s32 != SAI_TUNNEL_TYPE_IPINIP) &&
        (p_attr->value.s32 != SAI_TUNNEL_TYPE_IPINIP_GRE) &&
        (p_attr->value.s32 != SAI_TUNNEL_TYPE_VXLAN) &&
        (p_attr->value.s32 != SAI_TUNNEL_TYPE_MPLS)) {

        SAI_TUNNEL_LOG_ERR ("Invalid tunnel type value: %d.", p_attr->value.s32);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    p_tunnel_obj->tunnel_type = p_attr->value.s32;

    SAI_TUNNEL_LOG_DEBUG ("Tunnel object type set to %d.",
                          p_tunnel_obj->tunnel_type);
    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_tunnel_intf_set (dn_sai_tunnel_t *p_tunnel_obj,
                                            const sai_attribute_t  *p_attr)
{
    sai_fib_router_interface_t *p_rif_node = NULL;

    if (!sai_is_obj_id_rif (p_attr->value.oid)) {

        SAI_TUNNEL_LOG_ERR ("0x%"PRIx64" is not a valid RIF obj Id.",
                            p_attr->value.oid);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    p_rif_node = sai_fib_router_interface_node_get (p_attr->value.oid);
    if (NULL ==  p_rif_node) {

        SAI_TUNNEL_LOG_ERR ("RIF Id 0x%"PRIx64" not found.",
                            p_attr->value.oid);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (p_attr->id == SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE) {
        p_tunnel_obj->underlay_vrf = p_rif_node->vrf_id;
        p_tunnel_obj->underlay_rif = p_attr->value.oid;

        SAI_TUNNEL_LOG_DEBUG ("Tunnel Underlay RIF/VR set to 0x%"PRIx64"/0x%"PRIx64".",
                              p_tunnel_obj->underlay_rif, p_tunnel_obj->underlay_vrf);
    } else {
        p_tunnel_obj->overlay_vrf = p_rif_node->vrf_id;
        p_tunnel_obj->overlay_rif = p_attr->value.oid;

        SAI_TUNNEL_LOG_DEBUG ("Tunnel Overlay RIF/VR set to 0x%"PRIx64"/0x%"PRIx64".",
                              p_tunnel_obj->overlay_rif, p_tunnel_obj->overlay_vrf);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_tunnel_encap_sip_set (dn_sai_tunnel_t *p_tunnel_obj,
                                                 const sai_attribute_t  *p_attr)
{
    char   ip_addr_str [SAI_FIB_MAX_BUFSZ];

    if ((p_attr->value.ipaddr.addr_family != SAI_IP_ADDR_FAMILY_IPV4) &&
        (p_attr->value.ipaddr.addr_family != SAI_IP_ADDR_FAMILY_IPV6)) {

        SAI_TUNNEL_LOG_ERR ("Invalid IP addr family: %d.",
                             p_attr->value.ipaddr.addr_family);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    sai_fib_ip_addr_copy (&p_tunnel_obj->src_ip, &p_attr->value.ipaddr);

    SAI_TUNNEL_LOG_DEBUG ("Tunnel Encap Source IP set to %s.",
                          sai_ip_addr_to_str (&p_attr->value.ipaddr, ip_addr_str,
                                              SAI_FIB_MAX_BUFSZ));
    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_tunnel_ttl_mode_set (dn_sai_tunnel_t *p_tunnel_obj,
                                                const sai_attribute_t  *p_attr)
{
    if ((p_attr->value.s32 != SAI_TUNNEL_TTL_MODE_UNIFORM_MODEL) &&
        (p_attr->value.s32 != SAI_TUNNEL_TTL_MODE_PIPE_MODEL)) {

        SAI_TUNNEL_LOG_ERR ("Invalid ttl mode value: %d.", p_attr->value.s32);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    if (p_attr->id == SAI_TUNNEL_ATTR_ENCAP_TTL_MODE) {
        p_tunnel_obj->encap.ttl_mode = p_attr->value.s32;
    } else {
        p_tunnel_obj->decap.ttl_mode = p_attr->value.s32;
    }

    SAI_TUNNEL_LOG_DEBUG ("Tunnel object ttl mode set to %d.",
                          p_attr->value.s32);
    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_tunnel_dscp_mode_set (dn_sai_tunnel_t *p_tunnel_obj,
                                                 const sai_attribute_t  *p_attr)
{
    if ((p_attr->value.s32 != SAI_TUNNEL_DSCP_MODE_UNIFORM_MODEL) &&
        (p_attr->value.s32 != SAI_TUNNEL_DSCP_MODE_PIPE_MODEL)) {

        SAI_TUNNEL_LOG_ERR ("Invalid dscp mode value: %d.", p_attr->value.s32);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    if (p_attr->id == SAI_TUNNEL_ATTR_ENCAP_DSCP_MODE) {
        p_tunnel_obj->encap.dscp_mode = p_attr->value.s32;
    } else {
        p_tunnel_obj->decap.dscp_mode = p_attr->value.s32;
    }

    SAI_TUNNEL_LOG_DEBUG ("Tunnel object dscp mode set to %d.",
                          p_attr->value.s32);
    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_tunnel_dscp_set (dn_sai_tunnel_t *p_tunnel_obj,
                                            const sai_attribute_t  *p_attr)
{
    if ((p_attr->value.u8 > SAI_TUNNEL_MAX_DSCP_VAL)) {

        SAI_TUNNEL_LOG_ERR ("Invalid dscp value: %u.", p_attr->value.u8);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    if (p_attr->id == SAI_TUNNEL_ATTR_ENCAP_DSCP_VAL) {
        p_tunnel_obj->encap.dscp = p_attr->value.u8;
    } else {
        p_tunnel_obj->decap.dscp = p_attr->value.u8;
    }

    SAI_TUNNEL_LOG_DEBUG ("Tunnel object dscp val set to %u.",
                          p_attr->value.u8);
    return SAI_STATUS_SUCCESS;
}

static sai_status_t dn_sai_tunnel_ttl_set (dn_sai_tunnel_t *p_tunnel_obj,
                                           const sai_attribute_t  *p_attr)
{
    if ((p_attr->value.u8 == 0)) {

        SAI_TUNNEL_LOG_ERR ("Invalid ttl value: %u.", p_attr->value.u8);

        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    if (p_attr->id == SAI_TUNNEL_ATTR_ENCAP_TTL_VAL) {
        p_tunnel_obj->encap.ttl = p_attr->value.u8;
    } else {
        p_tunnel_obj->decap.ttl = p_attr->value.u8;
    }

    SAI_TUNNEL_LOG_DEBUG ("Tunnel object TTL val set to %u.",
                          p_attr->value.u8);
    return SAI_STATUS_SUCCESS;
}

static void dn_sai_tunnel_mapper_clean(sai_object_id_t tunnel_id,
                                       sai_object_list_t *p_objlist)
{
    uint_t idx = 0;
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_tunnel_map_t *p_tunnel_map = NULL;

    for(idx = 0; idx < p_objlist->count; idx++) {
        p_tunnel_map = dn_sai_tunnel_map_get(p_objlist->list[idx]);
        if(p_tunnel_map != NULL) {

            sai_rc = dn_sai_tunnel_map_dep_tunnel_remove(p_tunnel_map->map_id,
                                                         tunnel_id);
            if (sai_rc != SAI_STATUS_SUCCESS) {

                SAI_TUNNEL_LOG_ERR ("Failed to clean up tunnel_id 0x%"PRIx64
                                    "from tunnel map 0x%"PRIx64"",
                                    tunnel_id, p_tunnel_map->map_id);
            }

            if ( p_tunnel_map->ref_count > 0) {
                p_tunnel_map->ref_count--;
            }
        }
    }

    p_objlist->count = 0;
    free(p_objlist->list);
    p_objlist->list = NULL;
}

static sai_status_t dn_sai_tunnel_mapper_set (dn_sai_tunnel_t *p_tunnel,
                                              const sai_attribute_t  *p_attr)
{
    uint_t idx = 0;
    const sai_object_list_t *p_objlist = NULL;
    sai_object_list_t *p_new_objlist = NULL;
    dn_sai_tunnel_map_t *p_tunnel_map = NULL;
    bool bridge_to_vnid_map_found = false;
    bool vnid_to_bridge_map_found = false;
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    p_objlist = &p_attr->value.objlist;

    if(NULL == p_objlist->list) {
        SAI_TUNNEL_LOG_ERR("Invalid tunnel mapper list");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    p_new_objlist = (p_attr->id == SAI_TUNNEL_ATTR_ENCAP_MAPPERS) ?
                    &p_tunnel->tunnel_encap_mapper_list :
                    &p_tunnel->tunnel_decap_mapper_list;

    p_new_objlist->list = calloc(p_objlist->count, sizeof(sai_object_id_t));
    if(NULL == p_new_objlist->list) {
        SAI_TUNNEL_LOG_ERR("Failed to allocate memory for tunnel mapper list");
        return SAI_STATUS_NO_MEMORY;
    }

    p_new_objlist->count = 0;

    for(idx = 0; idx < p_objlist->count; idx++) {

        p_tunnel_map = dn_sai_tunnel_map_get(p_objlist->list[idx]);

        if(NULL == p_tunnel_map) {

            SAI_TUNNEL_LOG_ERR("Invalid tunnel map id %"PRIx64" in tunnel "
                               "mapper list",p_objlist->list[idx]);

            sai_rc = SAI_STATUS_INVALID_PARAMETER;
            break;
        }

        if(p_attr->id == SAI_TUNNEL_ATTR_ENCAP_MAPPERS) {

            if(!dn_tunnel_map_is_encap_map_type(p_tunnel_map->type)) {

                SAI_TUNNEL_LOG_ERR("Decap tunnel map given in encap mapper list");
                sai_rc = SAI_STATUS_INVALID_PARAMETER;
                break;
            }

            if(p_tunnel_map->type == SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI) {

                if(bridge_to_vnid_map_found) {
                    SAI_TUNNEL_LOG_ERR("Encap tunnel map of same type already exists");
                    sai_rc = SAI_STATUS_INVALID_PARAMETER;
                    break;
                }
                bridge_to_vnid_map_found = true;
            }

        } else {

            if(dn_tunnel_map_is_encap_map_type(p_tunnel_map->type)) {

                SAI_TUNNEL_LOG_ERR("Encap tunnel map given in decap mapper list");
                sai_rc = SAI_STATUS_INVALID_PARAMETER;
                break;
            }

            if(p_tunnel_map->type == SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF) {

                if(vnid_to_bridge_map_found) {
                    sai_rc = SAI_STATUS_INVALID_PARAMETER;
                    SAI_TUNNEL_LOG_ERR("Decap tunnel map of same type already exists");
                    break;
                }

                vnid_to_bridge_map_found = true;
            }
        }

        sai_rc = dn_sai_tunnel_map_dep_tunnel_add(p_tunnel_map->map_id,
                                                  p_tunnel->tunnel_id);
        if(sai_rc != SAI_STATUS_SUCCESS) {

            SAI_TUNNEL_LOG_ERR("Failed to add dep tunnel 0x%"PRIx64" to tunnel map  0x%"
                               PRIx64"",p_tunnel->tunnel_id,p_tunnel_map->map_id);
            break;
        }

        p_tunnel_map->ref_count++;
        p_new_objlist->list[idx] = p_objlist->list[idx];
        p_new_objlist->count++;
    }

    if(sai_rc != SAI_STATUS_SUCCESS) {

        dn_sai_tunnel_mapper_clean(p_tunnel->tunnel_id, p_new_objlist);
    }

    return sai_rc;
}

static sai_status_t dn_sai_tunnel_attr_set (dn_sai_tunnel_t *p_tunnel_obj,
                                            uint_t attr_count,
                                            const sai_attribute_t *attr_list)
{
    uint_t                  attr_idx;
    sai_status_t            status = SAI_STATUS_FAILURE;
    const sai_attribute_t  *p_attr;
    bool encap_mapper_present = false;
    bool decap_mapper_present = false;

    for (attr_idx = 0, p_attr = attr_list; (attr_idx < attr_count);
         ++attr_idx, ++p_attr) {

        SAI_TUNNEL_LOG_DEBUG ("Setting Tunnel attr id: %d at list index: %u.",
                              p_attr->id, attr_idx);

        status = SAI_STATUS_SUCCESS;

        switch (p_attr->id) {

            case SAI_TUNNEL_ATTR_TYPE:
                status = dn_sai_tunnel_type_set (p_tunnel_obj, p_attr);
                break;

            case SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE:
            case SAI_TUNNEL_ATTR_OVERLAY_INTERFACE:
                status = dn_sai_tunnel_intf_set (p_tunnel_obj, p_attr);
                break;

            case SAI_TUNNEL_ATTR_ENCAP_SRC_IP:
                status = dn_sai_tunnel_encap_sip_set (p_tunnel_obj, p_attr);
                break;

            case SAI_TUNNEL_ATTR_ENCAP_TTL_MODE:
            case SAI_TUNNEL_ATTR_DECAP_TTL_MODE:
                status = dn_sai_tunnel_ttl_mode_set (p_tunnel_obj, p_attr);
                break;

            case SAI_TUNNEL_ATTR_ENCAP_DSCP_MODE:
            case SAI_TUNNEL_ATTR_DECAP_DSCP_MODE:
                status = dn_sai_tunnel_dscp_mode_set (p_tunnel_obj, p_attr);
                break;

            case SAI_TUNNEL_ATTR_ENCAP_TTL_VAL:
                status = dn_sai_tunnel_ttl_set (p_tunnel_obj, p_attr);
                break;

            case SAI_TUNNEL_ATTR_ENCAP_DSCP_VAL:
                status = dn_sai_tunnel_dscp_set (p_tunnel_obj, p_attr);
                break;

            case SAI_TUNNEL_ATTR_ENCAP_MAPPERS:
                encap_mapper_present = true;
                status = dn_sai_tunnel_mapper_set (p_tunnel_obj, p_attr);
                break;

            case SAI_TUNNEL_ATTR_DECAP_MAPPERS:
                decap_mapper_present = true;
                status = dn_sai_tunnel_mapper_set (p_tunnel_obj, p_attr);
                break;

            default:
                status = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }

        if (status != SAI_STATUS_SUCCESS) {

            SAI_TUNNEL_LOG_ERR ("Failure in Tunnel attr list. Index: %d, "
                                "Attribute Id: %d, Error: %d.", attr_idx,
                                p_attr->id, status);

            status = sai_get_indexed_ret_val (status, attr_idx);
            break;
        }

        status = sai_tunnel_npu_api_get()->tunnel_obj_attr_validate (p_attr);

        if (status != SAI_STATUS_SUCCESS) {

            SAI_TUNNEL_LOG_ERR ("NPU failure in Tunnel attr list. Index: %d, "
                                "Attribute Id: %d, Error: %d.", attr_idx,
                                p_attr->id, status);

            status = sai_get_indexed_ret_val (status, attr_idx);
            break;
        }
    }

    if((status == SAI_STATUS_SUCCESS) &&
       (dn_sai_is_vxlan_tunnel(p_tunnel_obj))) {

        if(!encap_mapper_present || !decap_mapper_present) {

            SAI_TUNNEL_LOG_ERR ("Encap and decap mappers are mandatory for "
                                "VXLAN tunnel");
            status = SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
        }
    }

    if (status != SAI_STATUS_SUCCESS) {
            dn_sai_tunnel_mapper_clean(p_tunnel_obj->tunnel_id,
                                       &p_tunnel_obj->tunnel_encap_mapper_list);
            dn_sai_tunnel_mapper_clean(p_tunnel_obj->tunnel_id,
                                       &p_tunnel_obj->tunnel_decap_mapper_list);
    }

    return status;
}

static sai_status_t dn_sai_tunnel_create (sai_object_id_t *tunnel_id,
                                          sai_object_id_t  switch_id,
                                          uint32_t attr_count,
                                          const sai_attribute_t *attr_list)
{
    dn_sai_tunnel_t  *p_tunnel_obj = NULL;
    sai_status_t     status;
    int              tunnel_index;

    STD_ASSERT (tunnel_id != NULL);
    STD_ASSERT (attr_list != NULL);

    SAI_TUNNEL_LOG_DEBUG ("Entering SAI Tunnel object creation.");

    status = dn_sai_tunnel_attr_list_validate (SAI_OBJECT_TYPE_TUNNEL,
                                               attr_count, attr_list,
                                               SAI_OP_CREATE);
    if (status != SAI_STATUS_SUCCESS) {

        return status;
    }

    p_tunnel_obj = dn_sai_tunnel_object_alloc();

    if (p_tunnel_obj == NULL) {

        SAI_TUNNEL_LOG_CRIT ("SAI Tunnel object node memory alloc failed");

        return SAI_STATUS_NO_MEMORY;
    }

    std_dll_init (&p_tunnel_obj->tunnel_encap_nh_list);
    std_dll_init (&p_tunnel_obj->tunnel_term_entry_list);

    dn_sai_tunnel_lock();
    sai_fib_lock ();

    do {

        tunnel_index = std_find_first_bit (dn_sai_tunnel_obj_id_bitmap(),
                                           SAI_TUNNEL_OBJ_MAX_ID, 1);

        if (tunnel_index < 0) {

            SAI_TUNNEL_LOG_INFO ("No free index in tunnel object index bitmap of "
                                 "size %d.", SAI_TUNNEL_OBJ_MAX_ID);

            status = SAI_STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        p_tunnel_obj->tunnel_id = sai_uoid_create (SAI_OBJECT_TYPE_TUNNEL,
                                                   tunnel_index);

        /* Validate and fill the attribute values passed in the list */
        status = dn_sai_tunnel_attr_set (p_tunnel_obj, attr_count, attr_list);

        if (status != SAI_STATUS_SUCCESS) {
            SAI_TUNNEL_LOG_ERR ("SAI Tunnel object attribute error.");

            break;
        }

        /* Create the tunnel object in NPU */
        status = sai_tunnel_npu_api_get()->tunnel_obj_create (p_tunnel_obj);

        if (status != SAI_STATUS_SUCCESS) {
            SAI_TUNNEL_LOG_ERR ("SAI Tunnel object create failed in NPU.");

            break;
        }

        t_std_error rc = std_rbtree_insert (dn_sai_tunnel_tree_handle (),
                                            p_tunnel_obj);
        if (rc != STD_ERR_OK) {

            SAI_TUNNEL_LOG_ERR ("Error in inserting Tunnel obj Id: 0x%"PRIx64" "
                                "to database", p_tunnel_obj->tunnel_id);

            status = SAI_STATUS_FAILURE;

            break;
        }

        sai_rif_increment_ref_count(p_tunnel_obj->underlay_rif);
        sai_rif_increment_ref_count(p_tunnel_obj->overlay_rif);

        STD_BIT_ARRAY_CLR (dn_sai_tunnel_obj_id_bitmap(), tunnel_index);

        *tunnel_id = p_tunnel_obj->tunnel_id;

    } while (0);

    sai_fib_unlock ();
    dn_sai_tunnel_unlock();

    if (status != SAI_STATUS_SUCCESS) {

        dn_sai_tunnel_mapper_clean(p_tunnel_obj->tunnel_id,
                                   &p_tunnel_obj->tunnel_encap_mapper_list);
        dn_sai_tunnel_mapper_clean(p_tunnel_obj->tunnel_id,
                                   &p_tunnel_obj->tunnel_decap_mapper_list);

        dn_sai_tunnel_object_free (p_tunnel_obj);

        SAI_TUNNEL_LOG_INFO ("SAI Tunnel object create error: %d.", status);

    } else {

        SAI_TUNNEL_LOG_INFO ("Created SAI Tunnel Id: 0x%"PRIx64".", *tunnel_id);
    }

    return status;
}

static sai_status_t dn_sai_tunnel_remove (sai_object_id_t  tunnel_id)
{
    sai_status_t     status = SAI_STATUS_FAILURE;
    dn_sai_tunnel_t  *p_tunnel_obj = NULL;

    SAI_TUNNEL_LOG_DEBUG ("Entering SAI Tunnel object remove.");

    if (!sai_is_obj_id_tunnel (tunnel_id)) {

        SAI_TUNNEL_LOG_ERR ("OID: 0x%"PRIx64" is not of tunnel object.", tunnel_id);

        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    dn_sai_tunnel_lock();
    sai_fib_lock ();

    do {

        p_tunnel_obj = dn_sai_tunnel_obj_get (tunnel_id);

        if (p_tunnel_obj == NULL) {

            SAI_TUNNEL_LOG_ERR ("Tunnel object not found for OID: 0x%"PRIx64".",
                                 tunnel_id);
            status = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        /* Check if the tunnel is in use */
        if ((std_dll_getfirst (&p_tunnel_obj->tunnel_encap_nh_list) != NULL) ||
            (std_dll_getfirst (&p_tunnel_obj->tunnel_term_entry_list) != NULL)) {

            SAI_TUNNEL_LOG_ERR ("Tunnel object 0x%"PRIx64" is being used.",
                                tunnel_id);
            status = SAI_STATUS_OBJECT_IN_USE;
            break;
        }

        dn_sai_tunnel_t *p_tmp_node = (dn_sai_tunnel_t *)
            std_rbtree_remove (dn_sai_tunnel_tree_handle(), p_tunnel_obj);

        if (p_tmp_node != p_tunnel_obj) {

            SAI_TUNNEL_LOG_ERR ("Error in removing Tunnel Obj from database.");

            break;
        }

        /* Remove the tunnel object in NPU */
        status = sai_tunnel_npu_api_get()->tunnel_obj_remove (p_tunnel_obj);

        if (status != SAI_STATUS_SUCCESS) {
            SAI_TUNNEL_LOG_ERR ("SAI Tunnel object remove failed in NPU.");

            std_rbtree_insert (dn_sai_tunnel_tree_handle (), p_tunnel_obj);
            break;
        }

        STD_BIT_ARRAY_SET (dn_sai_tunnel_obj_id_bitmap(),
                           sai_uoid_npu_obj_id_get (tunnel_id));

        sai_rif_decrement_ref_count(p_tunnel_obj->underlay_rif);
        sai_rif_decrement_ref_count(p_tunnel_obj->overlay_rif);

        dn_sai_tunnel_mapper_clean(tunnel_id,
                                   &p_tunnel_obj->tunnel_encap_mapper_list);
        dn_sai_tunnel_mapper_clean(tunnel_id,
                                   &p_tunnel_obj->tunnel_decap_mapper_list);

        dn_sai_tunnel_object_free (p_tunnel_obj);

    } while (0);

    sai_fib_unlock ();
    dn_sai_tunnel_unlock();

    SAI_TUNNEL_LOG_INFO ("SAI Tunnel object Id 0x%"PRIx64" remove status: %d.",
                         tunnel_id, status);

    return status;
}

static sai_status_t dn_sai_tunnel_set_attr (sai_object_id_t tunnel_id,
                                            const sai_attribute_t *attr)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

static sai_status_t dn_sai_tunnel_get_attr (sai_object_id_t tunnel_id,
                                            uint32_t attr_count,
                                            sai_attribute_t *attr_list)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

static sai_status_t dn_sai_tunnel_stats_get (sai_object_id_t tunnel_id,
                                             uint32_t num_counters,
                                             const sai_tunnel_stat_t *counter_ids,
                                             uint64_t *counters)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_tunnel_t *p_tunnel = NULL;

    if((num_counters == 0) || (counter_ids == NULL) ||
       (counters == NULL)) {

        SAI_TUNNEL_LOG_ERR ("Invalid paramter supplied for getting tunnel statistics");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    dn_sai_tunnel_lock();

    do {

        p_tunnel = dn_sai_tunnel_obj_get (tunnel_id);

        if (p_tunnel == NULL) {

            SAI_TUNNEL_LOG_ERR ("Tunnel object not found for OID: 0x%"PRIx64".",
                                tunnel_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_rc = sai_tunnel_npu_api_get()->tunnel_stats_get(tunnel_id, num_counters,
                                                            counter_ids, counters);
        if(sai_rc != SAI_STATUS_SUCCESS) {

            SAI_TUNNEL_LOG_ERR ("Failed to get tunnel stats for tunnel 0x%"
                                PRIx64" from NPU",tunnel_id);
            break;
        }

    } while(0);

    dn_sai_tunnel_unlock();

    return sai_rc;
}

static sai_status_t dn_sai_tunnel_stats_clear (sai_object_id_t tunnel_id,
                                               uint32_t num_counters,
                                               const sai_tunnel_stat_t *counter_ids)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    dn_sai_tunnel_t *p_tunnel = NULL;

    if((num_counters == 0) || (counter_ids == NULL)) {

        SAI_TUNNEL_LOG_ERR ("Invalid paramter supplied for clearing tunnel statistics");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    dn_sai_tunnel_lock();

    do {

        p_tunnel = dn_sai_tunnel_obj_get (tunnel_id);

        if (p_tunnel == NULL) {

            SAI_TUNNEL_LOG_ERR ("Tunnel object not found for OID: 0x%"PRIx64".",
                                tunnel_id);
            sai_rc = SAI_STATUS_INVALID_OBJECT_ID;
            break;
        }

        sai_rc = sai_tunnel_npu_api_get()->tunnel_stats_clear(tunnel_id, num_counters,
                                                              counter_ids);
        if(sai_rc != SAI_STATUS_SUCCESS) {

            SAI_TUNNEL_LOG_ERR ("Failed to clear tunnel stats for tunnel 0x%"
                                PRIx64" from NPU",tunnel_id);
            break;
        }

    } while(0);

    dn_sai_tunnel_unlock();

    return sai_rc;
}

void dn_sai_tunnel_obj_api_fill (sai_tunnel_api_t *api_table)
{
    api_table->create_tunnel = dn_sai_tunnel_create;
    api_table->remove_tunnel = dn_sai_tunnel_remove;
    api_table->set_tunnel_attribute = dn_sai_tunnel_set_attr;
    api_table->get_tunnel_attribute = dn_sai_tunnel_get_attr;
    api_table->get_tunnel_stats = dn_sai_tunnel_stats_get;
    api_table->clear_tunnel_stats = dn_sai_tunnel_stats_clear;
}

/*
 * Tunnel API method table query function
 */
sai_tunnel_api_t *sai_tunnel_api_query (void)
{
    memset (&sai_tunnel_api_method_table, 0, sizeof (sai_tunnel_api_t));

    dn_sai_tunnel_obj_api_fill (&sai_tunnel_api_method_table);
    dn_sai_tunnel_term_obj_api_fill (&sai_tunnel_api_method_table);
    dn_sai_tunnel_map_obj_api_fill (&sai_tunnel_api_method_table);
    dn_sai_tunnel_map_entry_obj_api_fill (&sai_tunnel_api_method_table);

    return (&sai_tunnel_api_method_table);
}

