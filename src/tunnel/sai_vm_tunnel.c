/************************************************************************
* LEGALESE:   "Copyright (c) 2016, Dell Inc. All rights reserved."

* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file  sai_vm_tunnel.c
*
* @brief This file contains the function definitions for Tunnel object
*        related functionality.
*************************************************************************/
#include "saistatus.h"
#include "saitypes.h"
#include "sai_tunnel.h"
#include "sai_tunnel_npu_api.h"
#include "sai_common_utils.h"
#include "std_type_defs.h"
#include "std_assert.h"

static sai_npu_tunnel_api_t sai_vm_tunnel_api_table;

static sai_status_t sai_vm_tunnel_init (void)
{
    return SAI_STATUS_SUCCESS;
}

static void sai_vm_tunnel_deinit (void)
{
}

/*
 * Attribute id properties table for SAI tunnel objects.
 */
static const dn_sai_attribute_entry_t dn_sai_tunnel_attr[] = {
    { SAI_TUNNEL_ATTR_TYPE,                      true, true, false, true, true, true },
    { SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE,        true, true, false, true, true, true },
    { SAI_TUNNEL_ATTR_OVERLAY_INTERFACE,         true, true, false, true, true, true },
    { SAI_TUNNEL_ATTR_ENCAP_SRC_IP,              false, true, false, true, true, true },
    { SAI_TUNNEL_ATTR_ENCAP_TTL_MODE,            false, true, false, true, true, true },
    { SAI_TUNNEL_ATTR_ENCAP_TTL_VAL,             false, true, true, true, true, true },
    { SAI_TUNNEL_ATTR_ENCAP_DSCP_MODE,           false, true, false, true, true, true },
    { SAI_TUNNEL_ATTR_ENCAP_DSCP_VAL,            false, true, true, true, true, true },
    { SAI_TUNNEL_ATTR_ENCAP_GRE_KEY_VALID,       false, true, false, true, false, false },
    { SAI_TUNNEL_ATTR_ENCAP_GRE_KEY,             false, true, false, true, false, false },
    { SAI_TUNNEL_ATTR_ENCAP_ECN_MODE,            false, true, false, true, false, false },
    { SAI_TUNNEL_ATTR_ENCAP_MAPPERS,             false, true, false, true, false, true },
    { SAI_TUNNEL_ATTR_DECAP_ECN_MODE,            false, true, false, true, false, false },
    { SAI_TUNNEL_ATTR_DECAP_MAPPERS,             false, true, false, true, false, true },
    { SAI_TUNNEL_ATTR_DECAP_TTL_MODE,            false, true, false, true, true, true },
    { SAI_TUNNEL_ATTR_DECAP_DSCP_MODE,           false, true, false, true, true, true },
};

static const dn_sai_attribute_entry_t dn_sai_tunnel_term_attr [] = {
    { SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_VR_ID,              false, true, false, true, true, true },
    { SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TYPE,               true, true, false, true, true, true },
    { SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_DST_IP,             true, true, false, true, true, true },
    { SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP,             true, true, false, true, true, true },
    { SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TUNNEL_TYPE,        true, true, false, true, true, true },
    { SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID,   true, true, false, true, true, true },
};

static void sai_vm_tunnel_obj_attr_table_get (
                                const dn_sai_attribute_entry_t **p_attr_table,
                                uint_t *p_attr_count)
{
    *p_attr_table = &dn_sai_tunnel_attr[0];

    *p_attr_count = (sizeof(dn_sai_tunnel_attr)) / (sizeof(dn_sai_tunnel_attr[0]));
}

static void sai_vm_tunnel_term_obj_attr_table_get (
                                const dn_sai_attribute_entry_t **p_attr_table,
                                uint_t *p_attr_count)
{
    *p_attr_table = &dn_sai_tunnel_term_attr[0];

    *p_attr_count = (sizeof(dn_sai_tunnel_term_attr)) /
                    (sizeof(dn_sai_tunnel_term_attr[0]));
}

static void sai_vm_tunnel_attr_id_table_get (
                          sai_object_type_t obj_type,
                          const dn_sai_attribute_entry_t **p_attr_table,
                          uint_t *p_attr_count)
{
    if (obj_type == SAI_OBJECT_TYPE_TUNNEL) {
        sai_vm_tunnel_obj_attr_table_get (p_attr_table, p_attr_count);

    } else if (obj_type == SAI_OBJECT_TYPE_TUNNEL_TERM_TABLE_ENTRY) {
        sai_vm_tunnel_term_obj_attr_table_get (p_attr_table, p_attr_count);
    }
}

static sai_status_t sai_vm_tunnel_obj_create (dn_sai_tunnel_t *tunnel_obj)
{
    STD_ASSERT (tunnel_obj != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_tunnel_obj_remove (dn_sai_tunnel_t *tunnel_obj)
{
    STD_ASSERT (tunnel_obj != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_tunnel_term_entry_create (
                                   dn_sai_tunnel_term_entry_t *tunnel_term_obj)
{
    STD_ASSERT (tunnel_term_obj != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_tunnel_term_entry_remove (
                                   dn_sai_tunnel_term_entry_t *tunnel_term_obj)
{
    STD_ASSERT (tunnel_term_obj != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_tunnel_obj_attr_validate (const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

/*
 * NPU plugin API method table
 */
static void sai_vm_tunnel_api_table_fill (sai_npu_tunnel_api_t *tunnel_api_table)
{
    tunnel_api_table->tunnel_init              = sai_vm_tunnel_init;
    tunnel_api_table->tunnel_deinit            = sai_vm_tunnel_deinit;
    tunnel_api_table->attr_id_table_get        = sai_vm_tunnel_attr_id_table_get;
    tunnel_api_table->tunnel_obj_create        = sai_vm_tunnel_obj_create;
    tunnel_api_table->tunnel_obj_remove        = sai_vm_tunnel_obj_remove;
    tunnel_api_table->tunnel_obj_attr_validate = sai_vm_tunnel_obj_attr_validate;
    tunnel_api_table->tunnel_term_entry_create = sai_vm_tunnel_term_entry_create;
    tunnel_api_table->tunnel_term_entry_remove = sai_vm_tunnel_term_entry_remove;
}

sai_npu_tunnel_api_t* sai_vm_tunnel_api_query (void)
{
    sai_vm_tunnel_api_table_fill (&sai_vm_tunnel_api_table);

    return &sai_vm_tunnel_api_table;
}
