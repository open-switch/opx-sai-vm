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
 * @file sai_vm_netlink.h
 *
 * @brief This file contains the structures and APIs to support netlink
 *      monitoring in VM environment.
 *************************************************************************/

#ifndef __SAI_VM_NETLINK_H__
#define __SAI_VM_NETLINK_H__

sai_status_t sai_get_bridge_port_for_fdb_entry (const sai_fdb_entry_t *fdb_entry,
                                                sai_object_id_t *bridge_port_id);


#endif /* __SAI_VM_NETLINK_H__ */
