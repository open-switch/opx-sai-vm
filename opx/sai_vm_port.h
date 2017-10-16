/************************************************************************
 * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
 *
 * This source code is confidential, proprietary, and contains trade
 * secrets that are the sole property of Dell Inc.
 * Copy and/or distribution of this source code or disassembly or reverse
 * engineering of the resultant object code are strictly forbidden without
 * the written consent of Dell Inc.
 *
 ************************************************************************/
/*
 * @file sai_vm_port.h
 *
 * @brief This file contains the data structures and function prototypes for
 *        SAI Port functions in VM environment.
 *************************************************************************/

#ifndef __SAI_VM_PORT_H__
#define __SAI_VM_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "saitypes.h"
#include "saiport.h"
#include "sai_port_common.h"

// TODO use the prototype below, and look for port_info inside the implementation on sai_vm_port.c
/*
 * sai_port_attr_oper_status_set()
 *
 * Description: sets the operational state for a given NPU port ID.  
 *
 * Returns:     SAI_STATUS_SUCCESS on success
 *              SAI_STATUS_FAILURE on failure
 */
sai_status_t sai_port_attr_oper_status_set(const sai_npu_port_id_t npu_port_id,
                                           const sai_port_oper_status_t oper_status);

#ifdef __cplusplus
}
#endif


#endif /* __SAI_VM_PORT_H__ */
