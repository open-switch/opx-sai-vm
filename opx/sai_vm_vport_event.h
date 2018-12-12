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
 * @file sai_vm_vport_event.h
 *
 * @brief This file contains the structures and APIs to support virtual port
 *      event monitoring in a VM environment.
 *************************************************************************/

#ifndef __SAI_VM_VPORT_EVENT_H__
#define __SAI_VM_VPORT_EVENT_H__

#include "saiport.h"
#include "sai_port_common.h"

/* Type definition for callback function to notify port object of virtual port status changes */
typedef sai_status_t (*sai_vport_oper_status_cb_t)(const sai_npu_port_id_t npu_port_id,
                                           const sai_port_oper_status_t oper_status);

/*
 * This function initializes a thread that listens to netlink events relevant to the SAI VM implementation.
 */
void sai_vm_vport_event_init(void);


/* Set the callback function for reporting a virtual port status change */
void sai_vm_vport_event_oper_status_callback (sai_vport_oper_status_cb_t func);



#endif /* __SAI_VM_VPORT_EVENT_H__ */
