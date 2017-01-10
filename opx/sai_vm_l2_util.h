
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
 * @file sai_vm_l2_util.h
 *
 * @brief This file contains the function prototypes for SAI L2 functions
 *        in VM environment.
 *************************************************************************/

#ifndef __SAI_VM_L2_UTIL_H__
#define __SAI_VM_L2_UTIL_H__

#include "saitypes.h"
#include "saifdb.h"

sai_status_t sai_get_port_for_fdb_entry (const sai_fdb_entry_t *fdb_entry,
                                         sai_object_id_t *port_id);


#endif /* __SAI_VM_L2_UTIL_H__ */
