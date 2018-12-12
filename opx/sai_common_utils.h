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
* @file sai_common_utils.h
*
* @brief This file contains common utility APIs for SAI component
*
*************************************************************************/
#ifndef __SAI_COMMON_UTILS_H__
#define __SAI_COMMON_UTILS_H__

#include "saitypes.h"
#include "std_type_defs.h"
#include <stddef.h>

/** \defgroup SAICOMMONUTILAPIS SAI - Common Utility API
 *   Contains Common Utility APIs to be used by other SAI components
 *  \{
 */

/**
 * @brief SAI module types
 */
typedef enum _sai_module_t
{
    SAI_MODULE_SWITCH,                /**< SAI Switch module */
    SAI_MODULE_PORT,                  /**< SAI Port module */
    SAI_MODULE_FDB,                   /**< SAI FDB module */
    SAI_MODULE_VLAN,                  /**< SAI VLAN module */
    SAI_MODULE_VIRTUAL_ROUTER,        /**< SAI Virtual router module */
    SAI_MODULE_ROUTE,                 /**< SAI Route module */
    SAI_MODULE_NEXT_HOP,              /**< SAI Next hop module */
    SAI_MODULE_NEXT_HOP_GROUP,        /**< SAI Next hop group module*/
    SAI_MODULE_ROUTER_INTERFACE,      /**< SAI Router interface module */
    SAI_MODULE_NEIGHBOR,              /**< SAI Neighbor module */
    SAI_MODULE_ACL,                   /**< SAI ACL module */
    SAI_MODULE_HOST_INTERFACE,        /**< SAI Host interface module */
    SAI_MODULE_MIRROR,                /**< SAI Mirror module */
    SAI_MODULE_SAMPLEPACKET,          /**< SAI Sample packet module */
    SAI_MODULE_STP,                   /**< SAI STP module */
    SAI_MODULE_LAG,                   /**< SAI LAG module */
    SAI_MODULE_POLICER,               /**< SAI Policer module */
    SAI_MODULE_WRED,                  /**< SAI WRED module */
    SAI_MODULE_QOS_MAPS,              /**< SAI QOS Maps module */
    SAI_MODULE_QOS_QUEUE,             /**< SAI QOS Queue module */
    SAI_MODULE_SCHEDULER,             /**< SAI Scheduler module */
    SAI_MODULE_SCHEDULER_GROUP,       /**< SAI Scheduler group module */
    SAI_MODULE_QOS_PORT,              /**< SAI QOS Port module */
    SAI_MODULE_BRIDGE,                /**< SAI Bridge module */
    SAI_MODULE_L2MC,                  /**< SAI L2MC module */
    SAI_MODULE_MAX,                   /**< SAI Max module count */
} sai_module_t;

/**
 * @brief SAI data structure to maintain the attribute properties.
 */
typedef struct _dn_sai_attribute_entry_t
{
    /** Attribute id */
    sai_attr_id_t id;

    /** Attribute is Mandatory for create  */
    bool          mandatory_on_create;

    /** Attribute is valid for create  */
    bool          valid_for_create;

    /** Attribute is valid for create  */
    bool          valid_for_set;

    /** Attribute is valid for create  */
    bool          valid_for_get;

    /** Attribute is Implemented in SAI */
    bool          is_implemented;

    /** Attribute is supported */
    bool          is_supported;
} dn_sai_attribute_entry_t;

/**
 * @brief SAI Enum for attribute Operations.
 */
typedef enum _dn_sai_operations_t
{
    /** SAI API operation as create */
    SAI_OP_CREATE,

    /** SAI API operation as get */
    SAI_OP_GET,

    /** SAI API operation as set */
    SAI_OP_SET,

    /** SAI API operation as remove */
    SAI_OP_REMOVE,

    /** SAI MAX  API operations */
    SAI_OP_MAX,
} dn_sai_operations_t;

/**
 * @brief Utility function to validate the passed attributes based
 * on operation type.
 *
 * @param[in] attr_count Number of attributes in input list.
 * @param[in] p_attr_list Incoming attribute list to be validated.
 * @param[in] p_v_attr predefined list of attributes
 * @param[in] op_type Type of sai operation
 * @param[in] max_vendor_attr_count Number of mandatory vendor attributes
 *
 * @return SAI_STATUS_SUCCESS on success or a appropriate SAI
 *         error code.
 */

sai_status_t sai_attribute_validate (uint_t attr_count,
                                     const sai_attribute_t *p_attr_list,
                                     const dn_sai_attribute_entry_t *p_v_attr,
                                     dn_sai_operations_t op_type,
                                     uint_t max_vendor_attr_count);

/**
 * @brief This api takes a source array and maps it to the destination array based on position
 *  of the source element and returns a default value on failure
 *
 * @todo Move this API to the common-util repository as it is not SAI constrained.
 * @param[in] src Pointer to the source array
 * @param[in] dst Pointer to the destination array
 * @param[in] len Length of the array
 * @param[in] src_id source element to be mapped to the destination array
 * @param[in] default_id Default value when the source id is not present in source
 * @warning  Both source and destination array should be of the same length.
 *
 * @return On success return the mapped destination element
 *         On failure return the default_id
 */

static inline uint32_t translate(uint32_t *src, uint32_t *dst, size_t len,
                         uint32_t src_id, uint32_t default_id) {
    size_t ix = 0;
    for ( ix = 0; ix < len ; ++ix ) {
        if (src[ix]==src_id) return dst[ix];
    }
    return default_id;
}

/**
 * @brief Set the bitmap for the given attribute Id
 *
 * @param[in] attr_id Attribute Id
 * @return Bitmap to be set for the given attribute
 */
static inline uint32_t sai_attribute_bit_set (sai_attr_id_t attr_id)
{
    return (0x1 << (attr_id + 1));
}

/**
 * \}
 */

#endif
