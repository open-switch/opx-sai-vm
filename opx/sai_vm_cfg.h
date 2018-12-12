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
 * @file sai_vm_cfg.h
 *
 * @brief This file contains the data structures and function prototypes for
 *        SAI Configuration functions in VM environment.
 *************************************************************************/

#ifndef __SAI_VM_CFG_H__
#define __SAI_VM_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "saitypes.h"
#include "sai_port_common.h"

/***************************************************************************
 * Function:    sai_vm_cfg_find_interface
 *
 * Description: Scans the interface linked list searching for the match.
 *              Returns SAI_STATUS_FAILURE if interface is not found
 *                      SAI_STATUS_SUCCESS if interface is found
 **************************************************************************/
sai_status_t sai_vm_cfg_find_interface(const char *name, sai_npu_port_id_t *port);

/***************************************************************************
 * Function:    sai_vm_cfg_load_interface_cfg()
 *
 * Description: Loads XML configuration information for port mapping.
 *
 **************************************************************************/
void sai_vm_cfg_load_interface_cfg(void);

#ifdef __cplusplus
}
#endif

#endif /* __SAI_VM_CFG_H__ */
