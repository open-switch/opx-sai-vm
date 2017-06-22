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

/**
 * This function initializes a thread that listens to netlink events relevant to the SAI VM implementation.
 */
void sai_vm_netlink_thread_start(void);


#endif /* __SAI_VM_NETLINK_H__ */
