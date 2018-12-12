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
 * @file sai_vm_netlink.c
 *
 * @brief This file contains function implementations for netlink events
 *        relevant to SAI VM
 *************************************************************************/
#include <asm/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>

#include "std_thread_tools.h"
#include "std_socket_tools.h"
#include "sai_switch_utils.h"
#include "std_file_utils.h"
#include "sai_vm_port.h"
#include "sai_vm_cfg.h"
#include "sai_port_utils.h"


/* Set buffer size to 64K */
#define BUFFER_SIZE (64*1024)

static int nl_socket = STD_INVALID_FD;

/* Open a netlink socket */
static inline int sock_open (void)
{
    int sock = STD_INVALID_FD;
    struct sockaddr_nl addr;

    sock = socket (AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
    if (sock < 0) {
        SAI_SWITCH_LOG_ERR ("Cannot open netlink socket %s(%d)", strerror (errno), errno);
        return STD_INVALID_FD;
    }

    memset (&addr, 0, sizeof (addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_LINK;

    if (bind (sock, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
        SAI_SWITCH_LOG_ERR ("Cannot bind netlink socket %s(%d)", strerror (errno), errno);
        std_close (sock);
        return STD_INVALID_FD;
    }

    if (std_sock_set_rcvbuf(sock, BUFFER_SIZE) != STD_ERR_OK) {
        SAI_SWITCH_LOG_ERR ("Cannot set rcvbuf size %s(%d)", strerror (errno), errno);
        /* Continue: we can still receive messages. */
    }
    return sock;
}


static void reset_socket (void)
{
    /*  reset socket: close and reopen */
    std_close (nl_socket);
    nl_socket = sock_open ();
}

static inline bool is_socket_ok (void)
{
    return (nl_socket != STD_INVALID_FD);
}

static inline void set_link_promisc (struct ifinfomsg *ifi, const char *name)
{
    int status;
    struct {
        struct nlmsghdr hdr;
        struct ifinfomsg ifi;
    } req = {
      /* Set the length of the message to the sizeof 'ifinfomsg' - the length of the netlink header is added by default */
      .hdr.nlmsg_len   = NLMSG_LENGTH (sizeof (struct ifinfomsg)),
      .hdr.nlmsg_flags = NLM_F_REQUEST,
      .hdr.nlmsg_type  = RTM_NEWLINK,
      .hdr.nlmsg_seq   = 0,
      .ifi.ifi_family = ifi->ifi_family,
      .ifi.ifi_index  = ifi->ifi_index,
      .ifi.ifi_flags  = IFF_PROMISC,
      .ifi.ifi_change = IFF_PROMISC,
    };
    struct sockaddr_nl nladdr = {.nl_family = AF_NETLINK };
    struct iovec iov = {
        .iov_base = &req.hdr,
        .iov_len  = req.hdr.nlmsg_len
    };
    struct msghdr msg = {
        .msg_name    = &nladdr,
        .msg_namelen = sizeof (nladdr),
        .msg_iov     = &iov,
        .msg_iovlen  = 1,
    };

    status = sendmsg (nl_socket, &msg, 0);
    if (status < 0) {
        SAI_SWITCH_LOG_ERR ("ifname: %s sendmsg error %s (%d)",
                            (name != NULL) ? name : "na", strerror (errno), errno);
    reset_socket();
    }
}

static void sai_vm_state_handler(const char *iface_name, struct ifinfomsg *ifi)
{
    sai_npu_port_id_t      npu_port_id;
    sai_port_oper_status_t port_status = SAI_PORT_OPER_STATUS_UNKNOWN;
    sai_status_t           status = SAI_STATUS_FAILURE;

    /* determine the nature of the change */
    if (ifi->ifi_flags & IFF_RUNNING) {
        port_status = SAI_PORT_OPER_STATUS_UP;
    } else {
        port_status = SAI_PORT_OPER_STATUS_DOWN;
}

    SAI_SWITCH_LOG_TRACE("interface %s is %s", iface_name,
            port_status == SAI_PORT_OPER_STATUS_UP ? "up" : "down");

    /* find NPU port ID for interface name */
    status = sai_vm_cfg_find_interface(iface_name, &npu_port_id);
    if ( status == SAI_STATUS_SUCCESS ) {
        /* notify SAI state change for port */
        sai_port_attr_oper_status_set(npu_port_id, port_status);
    }
}

static void event_handler (struct sockaddr_nl *nladdr, struct nlmsghdr *n)
{
    struct ifinfomsg *ifi = NLMSG_DATA (n);
    struct rtattr *rta = IFLA_RTA (ifi);
    const char *name = NULL;
    uint32_t master_ifindex = 0;
    int len = n->nlmsg_len;

    len -= NLMSG_LENGTH (sizeof (*ifi));

    while (RTA_OK (rta, len)) {
        if ((rta->rta_type & 0xffff) == IFLA_IFNAME) {
            name = (const char *) RTA_DATA (rta);
        }
        if ((rta->rta_type & 0xffff) == IFLA_MASTER) {
            master_ifindex = *(uint32_t *) RTA_DATA (rta);
        }

        rta = RTA_NEXT (rta, len);
    }

    /* Set the interfaces which have a 'master' (bridge or bond - 'master > 0') to promiscuous mode
     * This allows ARPs and other multicast messages (e.g. LLDP) to be received and transmitted
     * for ports part of VLANs (represented as bridges) and LAG interfaces.
     * Only perform this action if the interfaces is UP / RUNNING and is not yet set to promsicuous mode.
     */
    if ((master_ifindex != 0) &&
        ((IFF_PROMISC & ifi->ifi_flags) == 0) &&
        (((ifi->ifi_flags & IFF_RUNNING) != 0) || ((ifi->ifi_flags & IFF_UP) != 0))) {

        set_link_promisc (ifi, name);
    }

    /* determine and report any change to the port status */
    sai_vm_state_handler(name, ifi);
}


/*
 * Main thread for handling VM related netlink events.
 */
static void* vm_netlink_thread_func (void* param)
{
    /* The buffer can be large; we declare it static - there is a single thread
     * that can receive messages, thus re-entrance is not an issue.
     */
    static char buf[BUFFER_SIZE];
    int count;
    struct nlmsghdr *hdr;
    struct sockaddr_nl nladdr = {.nl_family = AF_NETLINK };
    struct iovec iov = {
        .iov_base = buf,
    };
    struct msghdr msg = {
        .msg_name    = &nladdr,
        .msg_namelen = sizeof (nladdr),
        .msg_iov     = &iov,
        .msg_iovlen  = 1,
    };

    while (is_socket_ok()) {

        iov.iov_len = sizeof (buf);
        count = recvmsg (nl_socket, &msg, 0);

        if (count < 0) {
            if (errno == EINTR || errno == EAGAIN) continue;
            SAI_SWITCH_LOG_ERR ("recvmsg error %s (%d)", strerror (errno), errno);
            reset_socket ();
            continue;
        }

        if (count == 0) {
            SAI_SWITCH_LOG_ERR ("NetLink EOF");
            reset_socket ();
            continue;
        }
        if (msg.msg_namelen != sizeof (nladdr)) {
            SAI_SWITCH_LOG_ERR ("Invalid addr len: %d", msg.msg_namelen);
            continue;
        }

        for (hdr = (struct nlmsghdr *) buf; count >= sizeof (*hdr); ) {
            int len = hdr->nlmsg_len;

            if ( ((len - sizeof (*hdr)) < 0) || (len > count)) {
                if (msg.msg_flags & MSG_TRUNC) {
                    SAI_SWITCH_LOG_ERR ("Received truncated message");
                }
                else {
                    SAI_SWITCH_LOG_ERR ("Received malformed message: len=%d\n", len);
                }
                reset_socket ();
                break;
            }

            event_handler (&nladdr, hdr);

            count -= NLMSG_ALIGN (len);
            hdr = (struct nlmsghdr *) ((char *) hdr + NLMSG_ALIGN (len));
        }
    }
    SAI_SWITCH_LOG_ERR ("NetLink Socket Error - exiting");
    return NULL;
}


void sai_vm_netlink_thread_start (void)
{
    std_thread_create_param_t thread_param;
    t_std_error rc = STD_ERR_OK;
    /* Initialize Port Mapping table */
    sai_vm_cfg_load_interface_cfg();

    /* Allocate socket for receiving NetLink Events */
    nl_socket = sock_open ();
    if (nl_socket == STD_INVALID_FD) {
        /* Error already logged; nothing else to do, return */
        return;
    }

    std_thread_init_struct (&thread_param);
    thread_param.name = "sai-vm-netlink",
    thread_param.thread_function = (std_thread_function_t)vm_netlink_thread_func;

    rc = std_thread_create (&thread_param);
    if (rc != STD_ERR_OK) {
        SAI_SWITCH_LOG_ERR ("Failed initializing netlink socket thread");
    }
}

