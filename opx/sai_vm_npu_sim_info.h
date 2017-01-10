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
 * @file sai_vm_npu_sim_info.h
 *
 * @brief This file contains the function prototypes for the
 *        initialization of the basic switch and port info required for
 *        NPU simulation in VM environment.
 *
 *        @todo Remove this file once XML configuration based switch and
 *        port data retrieval infra is in place.
 *
 *************************************************************************/

#ifndef __SAI_VM_NPU_SIM_INFO_H__
#define __SAI_VM_NPU_SIM_INFO_H__

#include "saitypes.h"
#include "sai_switch_common.h" /* TODO To be removed */

/* Update common Switch level Info for VM */
void sai_vm_switch_info_update (sai_switch_id_t switch_id);

/* Update common port level info for VM */
sai_status_t sai_vm_port_info_update (void);

/* Default port's event default notification for all
 * valid logical Ports */
void sai_vm_port_event_default_notification (void);

#endif /* __SAI_VM_NPU_SIM_INFO_H__ */
