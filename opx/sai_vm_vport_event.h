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
