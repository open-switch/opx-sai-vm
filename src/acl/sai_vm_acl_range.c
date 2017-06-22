/************************************************************************
 * LEGALESE:   "Copyright (c) 2017, Dell Inc. All rights reserved."
 *
 * This source code is confidential, proprietary, and contains trade
 * secrets that are the sole property of Dell Inc.
 * Copy and/or distribution of this source code or disassembly or reverse
 * engineering of the resultant object code are strictly forbidden without
 * the written consent of Dell Inc.
 *
 ************************************************************************/
/*
 * @file sai_vm_acl_range.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI ACL range object in VM environment.
 *************************************************************************/
#include "sai_vm_defs.h"
#include "sai_acl_db_api.h"
#include "sai_oid_utils.h"
#include "sai_acl_npu_api.h"
#include "sai_acl_type_defs.h"
#include "sai_acl_utils.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_bit_masks.h"
#include "std_assert.h"
#include <inttypes.h>


/**
 * Vendor attribute array for acl range containing the attribute
 * values and properties for create,set and get functionality.
 */
static const dn_sai_attribute_entry_t sai_range_attr[] = {
    {SAI_ACL_RANGE_ATTR_TYPE, true, true, false, true, true, true},
    {SAI_ACL_RANGE_ATTR_LIMIT, true, true, false, true, true, true},
};


void sai_npu_range_attribute_table_get(const dn_sai_attribute_entry_t
                                       **vendor,
                                       uint_t *max_attr_count)
{
    *vendor = &sai_range_attr[0];

    *max_attr_count = sizeof(sai_range_attr)/sizeof(dn_sai_attribute_entry_t);

    return;
}

sai_status_t sai_npu_create_acl_range(sai_acl_range_t *acl_range)
{
    STD_ASSERT(acl_range != NULL);

    return SAI_STATUS_SUCCESS;
}


sai_status_t sai_npu_delete_acl_range(sai_acl_range_t *acl_range)
{
    STD_ASSERT(acl_range != NULL);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_set_acl_range(sai_acl_range_t *acl_range, uint_t attr_count,
                                   const sai_attribute_t *p_attr)
{
    STD_ASSERT(acl_range != NULL);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_get_acl_range(sai_acl_range_t *acl_range, uint_t attr_count,
                                   sai_attribute_t *p_attr_list)
{
    STD_ASSERT(acl_range != NULL);
    STD_ASSERT(p_attr_list != NULL);

    return SAI_STATUS_SUCCESS;
}

