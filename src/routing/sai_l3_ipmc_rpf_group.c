/************************************************************************
* * LEGALESE:   "Copyright (c) 2019, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/**
* @file sai_l3_ipmc_rpf_group.c
*
* @brief This file contains implementation of SAI IPMC RPF GROUP APIs.
*************************************************************************/
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "std_assert.h"
#include "saiipmc.h"
#include "sairpfgroup.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_modules_init.h"
#include "sai_ipmc_common.h"
#include "sai_ipmc_api.h"
#include "sai_mcast_api.h"
#include "sai_lag_api.h"
#include "sai_npu_port.h"
#include "sai_npu_l2mc.h"
#include "sai_npu_ipmc.h"
#include "sai_gen_utils.h"
#include "sai_oid_utils.h"
#include "sai_npu_l3_mcast.h"
#include "sai_common_infra.h"
#include "sai_bridge_api.h"
#include "sai_bridge_main.h"
#include "sai_l3_util.h"
#include "sai_lag_callback.h"
#include "sai_l3_api_utils.h"
#include "sai_port_utils.h"

/**
 * @brief Create RPF interface group
 *
 * @param[out] rpf_group_id RPF interface group id
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_rpf_create_group(
        sai_object_id_t *rpf_group_id,
        sai_object_id_t switch_id,
        uint32_t attr_count,
        const sai_attribute_t *attr_list)
{
    return SAI_STATUS_SUCCESS;
}

/**
 * @brief Remove RPF interface group
 *
 * @param[in] rpf_group_id RPF interface group id
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_rpf_remove_group( sai_object_id_t rpf_group_id)
{
    return SAI_STATUS_SUCCESS;
}

/**
 * @brief Get RPF interface Group attribute
 *
 * @param[in] rpf_group_id RPF interface group id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_rpf_get_group_attribute(
        sai_object_id_t rpf_group_id,
        uint32_t attr_count,
        sai_attribute_t *attr_list)
{
    return SAI_STATUS_SUCCESS;
}

/**
 * @brief Set RPF interface Group attribute
 *
 * @param[in] rpf_group_id RPF interface group id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_rpf_set_group_attribute(
        sai_object_id_t rpf_group_id,
        const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

/**
 * @brief Create RPF interface group member
 *
 * @param[out] rpf_group_member_id RPF interface group member id
 * @param[in] switch_id Switch ID
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_rpf_create_group_member(
    sai_object_id_t *rpf_group_member_id,
    sai_object_id_t switch_id,
    uint32_t attr_count,
    const sai_attribute_t *attr_list)
{
    return SAI_STATUS_SUCCESS;
}

/**
 * @brief Remove RPF interface group member
 *
 * @param[in] rpf_group_member_id RPF interface group member id
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_rpf_remove_group_member(
        sai_object_id_t rpf_group_member_id)
{
    return SAI_STATUS_SUCCESS;
}

/**
 * @brief Set RPF interface Group attribute
 *
 * @param[in] rpf_group_member_id RPF interface group member id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_rpf_set_group_member_attribute(
    sai_object_id_t rpf_group_member_id,
    const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

/**
 * @brief Get RPF interface Group attribute
 *
 * @param[in] rpf_group_member_id RPF group member ID
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_rpf_get_group_member_attribute(
    sai_object_id_t rpf_group_member_id,
    uint32_t attr_count,
    sai_attribute_t *attr_list)
{
    return SAI_STATUS_SUCCESS;
}

static sai_rpf_group_api_t sai_ipmc_rpf_group_method_table =
{
    sai_ipmc_rpf_create_group,
    sai_ipmc_rpf_remove_group,
    sai_ipmc_rpf_set_group_attribute,
    sai_ipmc_rpf_get_group_attribute,
    sai_ipmc_rpf_create_group_member,
    sai_ipmc_rpf_remove_group_member,
    sai_ipmc_rpf_set_group_member_attribute,
    sai_ipmc_rpf_get_group_member_attribute,
};

sai_rpf_group_api_t* sai_ipmc_rpf_group_api_query(void)
{
    return (&sai_ipmc_rpf_group_method_table);
}
