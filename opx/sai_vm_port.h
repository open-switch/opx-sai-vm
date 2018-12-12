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
