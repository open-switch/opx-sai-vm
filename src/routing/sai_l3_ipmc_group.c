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
* @file sai_l3_ipmc_group.c
*
* @brief This file contains implementation of SAI IPMC GROUP APIs.
*************************************************************************/

#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "std_assert.h"
#include "saiipmc.h"
#include "saiipmcgroup.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_modules_init.h"
#include "sai_ipmc_common.h"
#include "sai_ipmc_api.h"
#include "sai_mcast_api.h"
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

/**
 * @brief Create IPMC group
 *
 * @param[out] ipmc_group_id IPMC group id
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_create_group(
        sai_object_id_t *ipmc_group_id,
        sai_object_id_t switch_id,
        uint32_t attr_count,
        const sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    *ipmc_group_id = SAI_NULL_OBJECT_ID;
    return (sai_rc);
}

/**
 * @brief Remove IPMC group
 *
 * @param[in] ipmc_group_id IPMC group id
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_remove_group(
        sai_object_id_t ipmc_group_id)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    return (sai_rc);
}

/**
 * @brief Set IPMC Group attribute
 *
 * @param[in] ipmc_group_id IPMC group id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_set_group_attribute(
        sai_object_id_t ipmc_group_id,
        const sai_attribute_t *attr)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    return (sai_rc);
}

/**
 * @brief Get IPMC Group attribute
 *
 * @param[in] ipmc_group_id IPMC group id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_get_group_attribute(
        sai_object_id_t ipmc_group_id,
        uint32_t attr_count,
        sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    return (sai_rc);
}

/**
 * @brief Create IPMC group member
 *
 * @param[out] ipmc_group_member_id IPMC group member id
 * @param[in] switch_id Switch ID
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_create_group_member(
        sai_object_id_t *ipmc_group_member_id,
        sai_object_id_t switch_id,
        uint32_t attr_count,
        const sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    *ipmc_group_member_id = SAI_NULL_OBJECT_ID;
    return (sai_rc);
}

/**
 * @brief Remove IPMC group member
 *
 * @param[in] ipmc_group_member_id IPMC group member id
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_remove_group_member(
        sai_object_id_t ipmc_group_member_id)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    return (sai_rc);
}

/**
 * @brief Set IPMC Group attribute
 *
 * @param[in] ipmc_group_member_id IPMC group member id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_set_group_member_attribute(
        sai_object_id_t ipmc_group_member_id,
        const sai_attribute_t *attr)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    return (sai_rc);
}

/**
 * @brief Get IPMC Group attribute
 *
 * @param[in] ipmc_group_member_id IPMC group member ID
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t sai_ipmc_get_group_member_attribute(
        sai_object_id_t ipmc_group_member_id,
        uint32_t attr_count,
        sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    return (sai_rc);
}

static sai_ipmc_group_api_t sai_ipmc_group_method_table =
{
    sai_ipmc_create_group,
    sai_ipmc_remove_group,
    sai_ipmc_set_group_attribute,
    sai_ipmc_get_group_attribute,
    sai_ipmc_create_group_member,
    sai_ipmc_remove_group_member,
    sai_ipmc_set_group_member_attribute,
    sai_ipmc_get_group_member_attribute,
};

sai_ipmc_group_api_t* sai_ipmc_group_api_query(void)
{
    return (&sai_ipmc_group_method_table);
}
