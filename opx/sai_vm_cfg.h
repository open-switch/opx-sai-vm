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
