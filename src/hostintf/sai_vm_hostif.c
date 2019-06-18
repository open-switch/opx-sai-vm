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

/**
 * @file sai_vm_hostif.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI Host interface in VM environment.
 *************************************************************************/

#include "sai_npu_hostif.h"

#include "saitypes.h"
#include "saistatus.h"
#include "sai_hostif_common.h"
#include "sai_switch_utils.h"
#include "sai_port_utils.h"
#include "std_file_utils.h"
#include "std_thread_tools.h"
#include "std_socket_tools.h"
#include "sai_vm_vport.h"
#include "std_system.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <errno.h>

#define VLAN_TAG_LEN (4)
#define VLAN_TAG_OFFSET (12)
#define VLAN_TPID (0x8100)
#define MSGSZ (16*1024)


static sai_packet_event_notification_fn vm_pkt_rx_fn = NULL;

static void packet_rx(vport_desc_t *pdesc)
{
    if (vm_pkt_rx_fn != NULL) {
        sai_attribute_t attr;
        struct cmsghdr      *cmsg;
        union {
            struct cmsghdr  cmsg;
            char        buf[CMSG_SPACE(sizeof(struct tpacket_auxdata))];
        } cmsg_buf;

        // Static buffers - function does not need to be re-entrant
        static uint8_t buf[16*1024+VLAN_TAG_LEN ];
        static uint16_t *vtag_offset     = (uint16_t *)&buf[VLAN_TAG_OFFSET];
        static uint16_t *vlan_tci_offset = (uint16_t *)&buf[VLAN_TAG_OFFSET+sizeof(uint16_t)];

        uint8_t *msgdata = &buf[VLAN_TAG_LEN];

        struct sockaddr_ll  from;
        struct tpacket_auxdata *aux = NULL;

        struct iovec        iov;
        iov.iov_base = msgdata;
        iov.iov_len  = MSGSZ;

        struct msghdr message;

        message.msg_name=&from;
        message.msg_namelen=sizeof(from);
        message.msg_iov=&iov;
        message.msg_iovlen=1;
        message.msg_control     = &cmsg_buf;
        message.msg_controllen  = sizeof(cmsg_buf);
        message.msg_flags       = 0;

        ssize_t num_bytes=recvmsg(pdesc->data_sock, &message, MSG_TRUNC);

        if ((num_bytes < 0) && (errno != EINTR) && (errno != ENETDOWN)) {

            // EINTR (signal) and ENETDOWN (interface down) are valid cases, so we do not log
            EV_LOGGING(SAI_HOSTIF,ERR,"SAIHOSTIF","recv failed fpp id=%u ifindex=%u errno=%s(%d)",
                    (unsigned int)pdesc->fpp_id, pdesc->if_index,
                    strerror(errno), errno);
            return;
        }

        if (from.sll_ifindex != pdesc->if_index) {
            EV_LOGGING(SAI_HOSTIF,ERR,"SAIHOSTIF","if_index mismatch fpp_id=%u ifindex=%u recv_if_index=%d",
                    (unsigned int)pdesc->fpp_id, pdesc->if_index, from.sll_ifindex);
            return;
        }

        if(message.msg_flags & MSG_CTRUNC) {
            EV_LOGGING(SAI_HOSTIF,ERR,"SAIHOSTIF","recv truncated fpp_id=%u ifindex=%u cnt=%lu",
                    (unsigned int)pdesc->fpp_id, pdesc->if_index, (unsigned long)num_bytes);

        }

        if (num_bytes > sizeof(buf)) {
            EV_LOGGING(SAI_HOSTIF,ERR,"SAIHOSTIF", "Recv Buf MSG_TRUNC:  from fpp_id (%d) cnt=%lu",
                    pdesc->fpp_id, pdesc->if_index, (unsigned long)num_bytes);
            num_bytes = sizeof(buf);
        }

        for (cmsg = CMSG_FIRSTHDR(&message); cmsg != NULL; cmsg = CMSG_NXTHDR(&message, cmsg)) {

            if (cmsg->cmsg_len < CMSG_LEN(sizeof(struct tpacket_auxdata)) ||
                    (cmsg->cmsg_level != SOL_PACKET) ||
                    (cmsg->cmsg_type != PACKET_AUXDATA)) {

                continue;
            }
            aux = (struct tpacket_auxdata *)CMSG_DATA(cmsg);
            if (aux == NULL) {
                EV_LOGGING(SAI_HOSTIF,ERR,"SAIHOSTIF", "Recv Buf AUX=NULL: npu port (%d) index=%u cnt=%lu CMSG len=%u level=%u type=%d",
                        pdesc->npu_port_id, pdesc->if_index, (unsigned long)num_bytes,
                        (unsigned int)cmsg->cmsg_len, (unsigned int)cmsg->cmsg_level, (unsigned int)cmsg->cmsg_type);
                continue;
            }

            if ((aux->tp_vlan_tci != 0) || ((aux->tp_status & TP_STATUS_VLAN_VALID) != 0)) {
                memmove(buf, msgdata, VLAN_TAG_OFFSET);

                *vtag_offset = htons(VLAN_TPID);
                *vlan_tci_offset = htons(aux->tp_vlan_tci);
                num_bytes += VLAN_TAG_LEN;
                msgdata = buf;
                break;
            }
        }

        sai_port_info_t  *port_info = sai_port_info_get_from_npu_phy_port((sai_npu_port_id_t)pdesc->npu_port_id);
        if(port_info == NULL) {
            EV_LOGGING(SAI_HOSTIF,ERR,"SAIHOSTIF", "Recv failed retrieving port information from npu port (%d) if_index=%u",
                    pdesc->npu_port_id, pdesc->if_index);
            return;
        }

        attr.id = SAI_HOSTIF_PACKET_ATTR_INGRESS_PORT;
        attr.value.oid = port_info->sai_port_id;

        vm_pkt_rx_fn(sai_switch_id_get(), (const void*)msgdata, (sai_size_t)num_bytes, 1, &attr);
    }
}


static void* vm_packet_rx_thread_func(void* param)
{
    t_std_error rc = sai_vport_init_packet_io();
    if (rc != STD_ERR_OK) {
        return NULL;
    }
    sai_vport_do_packet_rx_loop(packet_rx);

    return NULL;
}

static sai_status_t sai_vm_hostif_init()
{
    std_thread_create_param_t thread_param;
    t_std_error rc = STD_ERR_OK;

    EV_LOGGING(SAI_HOSTIF,INFO,"SAIHOSTIF", "VMHOSTIF (%s)", __FUNCTION__);

    std_thread_init_struct (&thread_param);
    thread_param.name = "sai-vm-packet-rx",
            thread_param.thread_function = (std_thread_function_t)vm_packet_rx_thread_func;

    rc = std_thread_create (&thread_param);
    if (rc != STD_ERR_OK) {
        EV_LOGGING(SAI_HOSTIF,ERR,"SAIHOSTIF", "Failed initializing SAI packet I/O socket thread");
    }


    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_hostintf_send_packet(
        const void* buffer,
        size_t buff_size,
        uint_t attr_count,
        const sai_attribute_t *attr_list)
{

    sai_attribute_t *pattr = (sai_attribute_t *)attr_list;
    sai_npu_port_id_t egress_port = (sai_npu_port_id_t)(-1);
    sai_status_t rc = SAI_STATUS_SUCCESS;
    sai_port_info_t *port_info = NULL;

    while (pattr < &attr_list[attr_count]) {

        switch (pattr->id) {
        case SAI_HOSTIF_PACKET_ATTR_EGRESS_PORT_OR_LAG:
            port_info = sai_port_info_get(pattr->value.oid);
            if (NULL == port_info) {
                EV_LOGGING(SAI_HOSTIF,ERR,"SAIHOSTIF","Cannot get NPU port");
                return SAI_STATUS_INVALID_OBJECT_ID;
            }
            egress_port = port_info->phy_port_id;
            break;
        case SAI_HOSTIF_PACKET_ATTR_HOSTIF_TX_TYPE:
            if (pattr->value.s32 != SAI_HOSTIF_TX_TYPE_PIPELINE_BYPASS) {
                EV_LOGGING(SAI_HOSTIF,DEBUG,"SAIHOSTIF","Non Bypass");
                return rc;
            }
            break;
        default:
            break;
        }
        ++pattr;
    }
    if ((sai_npu_port_id_t)(-1) == egress_port) {
        EV_LOGGING(SAI_HOSTIF,ERR,"SAIHOSTIF","NPU port not found");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    vport_desc_t* pdesc = sai_vm_vport_get_desc(egress_port);
    if (NULL == pdesc) {
        EV_LOGGING(SAI_HOSTIF,ERR,"SAIHOSTIF","Invalid npu port=%u", (unsigned int)egress_port);
        return SAI_STATUS_INVALID_PORT_NUMBER;
    }

    if (0 == pdesc->if_index || pdesc->data_sock == STD_INVALID_FD) {
        // discard invalid interfaces
        return rc;
    }

    struct sockaddr_ll socket_address;

    memset(&socket_address, 0, sizeof(socket_address));
    /* Index of the network device */
    socket_address.sll_ifindex = pdesc->if_index;
    socket_address.sll_halen = ETH_ALEN;
    memcpy(socket_address.sll_addr, buffer, ETH_ALEN);
    /* Send packet */

    if (sendto(pdesc->data_sock, buffer, buff_size, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0) {
        EV_LOGGING(SAI_HOSTIF,ERR,"SAIHOSTIF","Send Error npu port=%u ifindex=%u num_bytes=%lu errno=%s(%d)",
                (unsigned int)egress_port, pdesc->if_index, (unsigned long)buff_size,
                strerror(errno), errno);
        return SAI_STATUS_FAILURE;
    }

    return rc;
}

static sai_status_t sai_vm_hostif_validate_trapgroup(
        const sai_attribute_t *attr,
        dn_sai_hostif_op_t operation)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_hostif_update_trapgroup(
        const dn_sai_trap_node_t * trap_node,
        const dn_sai_trap_group_node_t *trap_group,
        const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}


static sai_status_t sai_vm_hostif_validate_trap(sai_hostif_trap_type_t trapid,
        const sai_attribute_t *attr,
        dn_sai_hostif_op_t operation)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_vm_hostif_set_trap(dn_sai_trap_node_t *trap_node,
        const dn_sai_trap_group_node_t *trap_group,
        const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

static void sai_vm_hostintf_reg_packet_rx_fn(
        const sai_packet_event_notification_fn pkt_rx_fn)
{
    vm_pkt_rx_fn = pkt_rx_fn;
}

static void sai_vm_hostintf_dump_trap(const dn_sai_trap_node_t *trap_node)
{
}

static void sai_vm_hostif_debug_set(sai_hostif_debug_attr_t attr_id, int value)
{
}

static uint64_t sai_vm_hosif_rx_errors_get(void)
{
    return 0;
}

static uint32_t sai_vm_hostif_get_max_user_def_traps(void)
{
    return 256;
}

static sai_npu_hostif_api_t sai_vm_hostif_api_table = {

        sai_vm_hostif_init,
        sai_vm_hostif_validate_trapgroup,
        sai_vm_hostif_update_trapgroup,
        sai_vm_hostif_validate_trap,
        sai_vm_hostif_set_trap,
        sai_vm_hostintf_send_packet,
        sai_vm_hostintf_reg_packet_rx_fn,
        sai_vm_hostintf_dump_trap,
        sai_vm_hostif_debug_set,
        sai_vm_hosif_rx_errors_get,
        sai_vm_hostif_get_max_user_def_traps
};

sai_npu_hostif_api_t* sai_vm_hostif_api_query (void)
{
    return &sai_vm_hostif_api_table;
}
