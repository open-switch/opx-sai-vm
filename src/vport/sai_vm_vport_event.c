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
 * @file sai_vm_vport_event.c
 *
 * @brief Function implementations for virtual port events
 *        relevant to SAI VM. Uses a netlink implementation.
 *
 *************************************************************************/
#include "sai_vm_vport_event.h"
#include "std_thread_tools.h"
#include "std_socket_tools.h"
#include "sai_switch_utils.h"
#include "std_file_utils.h"
#include "sai_port_utils.h"
#include "sai_vm_vport.h"

#include <sys/socket.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/types.h>
#include <string.h>

/* Set buffer size to 64K */
#define BUFFER_SIZE (64*1024)

static int nl_socket = STD_INVALID_FD;
static sai_vport_oper_status_cb_t oper_status_cb_func = NULL;



/* Open a netlink socket */
static inline int sock_open (void)
{
    int sock = STD_INVALID_FD;
    struct sockaddr_nl addr;
    t_std_error rc = std_netns_socket_create (e_std_sock_NETLINK,
            e_std_sock_type_RAW,
            NETLINK_ROUTE,
            (const std_socket_address_t*)NULL,
            VPORT_NAME_SPACE,
            &sock);

    if (rc != STD_ERR_OK) {

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


static void sai_vm_state_handler(const char *iface_name, struct ifinfomsg *ifi)
{
    sai_npu_port_id_t      npu_port_id;
    sai_port_oper_status_t port_status = SAI_PORT_OPER_STATUS_UNKNOWN;
    sai_status_t           status = SAI_STATUS_FAILURE;

    /* find NPU port ID for interface name */
    status = sai_vport_get_npu_port(ifi->ifi_index, &npu_port_id);
    if ( status == SAI_STATUS_SUCCESS ) {

        /* determine the nature of the change */
        port_status = (ifi->ifi_flags & IFF_RUNNING) ?
                SAI_PORT_OPER_STATUS_UP :
                SAI_PORT_OPER_STATUS_DOWN;

        SAI_SWITCH_LOG_TRACE("interface %s is %s", iface_name,
                port_status == SAI_PORT_OPER_STATUS_UP ? "up" : "down");
        /* notify SAI state change for port */
        if (oper_status_cb_func != NULL) {
            oper_status_cb_func(npu_port_id, port_status);
        }
    }
}

static void event_handler (struct sockaddr_nl *nladdr, struct nlmsghdr *n)
{
    struct ifinfomsg *ifi = NLMSG_DATA (n);
    struct rtattr *rta = IFLA_RTA (ifi);
    const char *name = NULL;
    int len = n->nlmsg_len;

    len -= NLMSG_LENGTH (sizeof (*ifi));

    while (RTA_OK (rta, len)) {
        if ((rta->rta_type & 0xffff) == IFLA_IFNAME) {
            name = (const char *) RTA_DATA (rta);
        }

        rta = RTA_NEXT (rta, len);
    }

    /* determine and report any change to the port status */
    sai_vm_state_handler(name, ifi);
}


/*
 * Main thread for handling VM related netlink events.
 */
static void* vm_vport_event_thread_func (void* param)
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

    /* Allocate socket for receiving NetLink Events -
     * MUST be opened in the context of this thread for initialization of socket in correct namespace */
    nl_socket = sock_open ();
    if (nl_socket == STD_INVALID_FD) {
        /* Error already logged; nothing else to do, return */
        return NULL;
    }

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


void sai_vm_vport_event_init (void)
{
    std_thread_create_param_t thread_param;
    t_std_error rc = STD_ERR_OK;

    /* Initialize Port Mapping table */
    sai_vport_init();


    std_thread_init_struct (&thread_param);
    thread_param.name = "sai-vm-netlink",
            thread_param.thread_function = (std_thread_function_t)vm_vport_event_thread_func;

    rc = std_thread_create (&thread_param);
    if (rc != STD_ERR_OK) {
        SAI_SWITCH_LOG_ERR ("Failed initializing netlink socket thread");
    }
}

void sai_vm_vport_event_oper_status_callback (sai_vport_oper_status_cb_t func)
{
    oper_status_cb_func = func;
}

