/************************************************************************
* * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/***
 * @file  sai_npu_hostif.h
 *
 * @brief Declaration of SAI NPU HOSTIF APIs
*/
#ifndef _SAI_NPU_HOST_IF_H_
#define _SAI_NPU_HOST_IF_H_

#include "saitypes.h"
#include "saihostintf.h"
#include "sai_hostif_common.h"

/** @defgroup SAINPUHOSTIFAPI SAI - NPU Host Interface Functionality
 *   Host Interface functions for SAI NPU component
 *
 * @ingroup SAIHOSTIFAPI
 * @{
 */

/**
 * @brief Initialize NPU host interface
 *
 * @return SAI_STATUS_SUCCESS if initialization is successful
 * otherwise a different error code is returned.
 */
typedef sai_status_t (*sai_npu_hostif_init)(void);

/**
 * @brief Send a packet out via the NPU
 *
 * @param[in] buffer The packet buffer
 * @param[in] buff_size Size of the packet buffer
 * @param[in] attr_count Attribute Count in the Attribute List
 * @param[in] attr_list Attribute List
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 * error code is returned.
 */
typedef sai_status_t (*sai_npu_hostif_send_packet)(
                                       const void* buffer,
                                       size_t buff_size,
                                       uint_t attr_count,
                                       const sai_attribute_t *attr_list);
/**
 * @brief Check if the trapgroup attribute is supported by the NPU
 *
 * @param[in] attr The attribute
 * @param[in] operation The operation being performed eg CREATE,SET etc
 * @return SAI_STATUS_SUCCESS if validation is successful otherwise a different
 * error code is returned.
 */
typedef sai_status_t (*sai_npu_hostif_validate_trapgroup)(
                                         const sai_attribute_t *attr,
                                         dn_sai_hostif_op_t operation);

/**
 * @brief Update the trapgroup related programming in the NPU
 *
 * @param[in] trap_node The trap node associated with trap group
 * @param[in] trap_group The trap group node
 * @param[in] attr The attribute based on which the updation needs to be done
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 * error code is returned.
 */
typedef sai_status_t (*sai_npu_hostif_update_trapgroup)(
                               const dn_sai_trap_node_t * trap_node,
                               const dn_sai_trap_group_node_t *trap_group,
                               const sai_attribute_t *attr);

/**
 * @brief Check if the trap attribute is supported by the NPU
 *
 * @param[in] trapid The trap identifier
 * @param[in] attr The trap attribute
 * @param[in] operation The operation being performed eg CREATE,SET etc
 * @return SAI_STATUS_SUCCESS if validation is successful otherwise a
 * different error code is returned.
 */
typedef sai_status_t (*sai_npu_hostif_validate_trap)(
                               sai_hostif_trap_type_t trapid,
                               const sai_attribute_t *attr,
                               dn_sai_hostif_op_t operation);

/**
 * @brief Set the trap properties in the NPU
 *
 * @param[in] trap_node The trap node associated with the trap node
 * @param[in] trap_group The trap group
 * @param[in] attr The attribute which needs to be set
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 * error code is returned.
 */
typedef sai_status_t (*sai_npu_hostif_set_trap)(
                               dn_sai_trap_node_t *trap_node,
                               const dn_sai_trap_group_node_t *trap_group,
                               const sai_attribute_t *attr);

/**
 * @brief Register with call back function for switch operational state
 * change notifications
 *
 * @param[in] sai_rcv_packet_fn callback to handle the recieved packets
 *            use NULL as input to unregister from the callback notifications
 *  @warning Calling this API for the second time will overwrite the existing
 *           registered function
 */

typedef void (*sai_npu_hostif_reg_packet_rx_fn)(
                          const sai_packet_event_notification_fn pkt_rx_fn);

/**
 * @brief Dump NPU specific information contained in the trap node
 *
 * @param[in] trap_node The trap node associated with the hostif trap
 */
typedef void (*sai_npu_hostif_dump_trap)(const dn_sai_trap_node_t *trap_node);

/**
 * @brief HOSTIF NPU API table.
 */
typedef struct _sai_npu_hostif_api_t {
    sai_npu_hostif_init                npu_hostif_init;
    sai_npu_hostif_validate_trapgroup  npu_validate_trapgroup;
    sai_npu_hostif_update_trapgroup    npu_update_trapgroup;
    sai_npu_hostif_validate_trap       npu_validate_trap;
    sai_npu_hostif_set_trap            npu_set_trap;
    sai_npu_hostif_send_packet         npu_send_packet;
    sai_npu_hostif_reg_packet_rx_fn    npu_register_packet_rx;
    sai_npu_hostif_dump_trap           npu_dump_trap;
}sai_npu_hostif_api_t;
/**
 * @}
 */
#endif /* _SAI_NPU_HOST_IF_H_ */
