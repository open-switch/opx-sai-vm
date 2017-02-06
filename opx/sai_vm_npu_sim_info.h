/*
 * Copyright (c) 2016 Dell Inc.
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
 * @file sai_vm_npu_sim_info.h
 *
 * @brief This file contains the function prototypes for the
 *        initialization of the basic switch and port info required for
 *        NPU simulation in VM environment.
 *
 *        @todo Remove this file once XML configuration based switch and
 *        port data retrieval infra is in place.
 */

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
