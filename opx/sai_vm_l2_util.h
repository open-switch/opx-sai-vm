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
 * @file sai_vm_l2_util.h
 *
 * @brief This file contains the function prototypes for SAI L2 functions
 *        in VM environment.
 */

#ifndef __SAI_VM_L2_UTIL_H__
#define __SAI_VM_L2_UTIL_H__

#include "saitypes.h"
#include "saifdb.h"

sai_status_t sai_get_port_for_fdb_entry (const sai_fdb_entry_t *fdb_entry,
                                         sai_object_id_t *port_id);


#endif /* __SAI_VM_L2_UTIL_H__ */
