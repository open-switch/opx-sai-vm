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

/*
 * @file sai_vm_vport.h
 *
 * @brief This file contains the data structures and function prototypes for
 *        SAI VM virtual ports - front panel ports and NPU port mapping for VMs
 *        Virtual ports are a representation of Ethernet devices mapped to
 *        virtual network adaptors provided by the Hypervisor.
 *************************************************************************/

#ifndef __SAI_VM_VPORT_H__
#define __SAI_VM_VPORT_H__

#include "saitypes.h"
#include "sai_port_common.h"
#include "std_error_codes.h"

/* Virtual front panel name space
 * See also, in platform-VM: config/scripts/common/bin/vport.sh
 */
#define VPORT_NAME_SPACE "vportnetns"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _vport_desc_t {
    /* Front panel port identifier */
    unsigned int fpp_id;
    /* Interface index of Ethernet device associated */
    int if_index;
    /* Data Rx/Tx RAW socket for the Ethernet device */
    int data_sock;
    /* HW NPU port identifier */
    unsigned int npu_port_id;
} vport_desc_t;

typedef void (*vport_packet_rx_t)(vport_desc_t *desc);

/**************************************************************************
 *
 * Initialize Virtual Port Module.
 * Called at SAI-VM switch module initialization time.
 *
 **************************************************************************/
void sai_vport_init(void);

/***************************************************************************
 * Get HW NPU port Id based of interface index of virtual port
 **************************************************************************/
sai_status_t sai_vport_get_npu_port(int if_index, sai_npu_port_id_t *port);

/***************************************************************************
 *  Enable/Disable virtual port, given the associated HW NPU Port Id
 ****************************************************************************/
bool sai_vport_set_admin_state(sai_npu_port_id_t port_id, bool enable);

/***************************************************************************
 *  Set the MTU size for a virtual port, given the associated HW NPU Port Id
 ****************************************************************************/
bool sai_vport_set_mtu_size(sai_npu_port_id_t port_id, unsigned int mtu_sz);

/***************************************************************************
 *  Get operational state of a virtual port, given the associated HW NPU Port Id
 ****************************************************************************/
sai_port_oper_status_t sai_vport_get_oper_status(sai_npu_port_id_t port_id);


/***************************************************************************
 * Called when the switch MAC address is set.
 * Updates the virtual port MAC addresses as per standard device MAC address scheme rules.
 ****************************************************************************/
sai_status_t sai_vport_set_switch_mac_address (const sai_mac_t *mac_address);

/***************************************************************************
 *  Get te descriptor of  virtual port, given the associated HW NPU Port Id
 ****************************************************************************/
vport_desc_t* sai_vm_vport_get_desc(sai_npu_port_id_t port_id);

/***************************************************************************
 * Initialize packet I/O of the Virtual ports (opens sockets, etc.)
 ****************************************************************************/
t_std_error sai_vport_init_packet_io(void);

/***************************************************************************
 * Packet reception loop
 ****************************************************************************/
void sai_vport_do_packet_rx_loop(vport_packet_rx_t rx_func);


#ifdef __cplusplus
}
#endif

#endif /* __SAI_VM_VPORT_H__ */
