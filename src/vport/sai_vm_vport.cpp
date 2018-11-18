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
 * @file sai_vm_vport.cpp
 *
 * @brief Implementation of the virtual port entity and functions
 ************************************************************************/
#include "sai_vm_vport.h"

#include "std_config_node.h"
#include "std_utils.h"
#include "std_system.h"
#include "std_socket_tools.h"
#include "event_log.h"
#include "sai_switch_utils.h"
#include "std_file_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <net/if_arp.h>



/* Configuration XML file describing the interface to port mapping */
#define PORT_PHYSICAL_MAPPING_TABLE_XML "/etc/opx/base_port_physical_mapping_table.xml"

/* String defining the front panel port node.  This is file format specific */
#define FRONT_PORT_NODE_STRING  "front-panel-port"

/* string defining the interface name attribute.  This is file format-specific */
#define INTERFACE_NAME_ATTRIBUTE_STRING "name"

/* String defining the front entry node.  This is file format specific */
#define ENTRY_NODE_STRING  "entry"

/* string defining the HW (NPU) port ID attribute.  This is file format-specific */
#define HW_PORT_ATTRIBUTE_STRING "hwport"

/* string defining the front port ID attribute.  This is file format-specific */
#define FP_ID_ATTRIBUTE_STRING "id"

#define MAC_OFFSET_ATTRIBUTE_STRING "mac_offset"

/* Allowable size of the interface name (from configuration file) */
#define INTERFACE_NAME_LEN  32

// See also: vnic.sh in platform-VM: config/scripts/common/bin/vnic.sh
#define VNIC_PORT_PREFIX "vport"


/** Virtual front panel port */
class sai_vport {

protected:

    vport_desc_t desc;
    int mac_addr_offset;
    std::string if_name;
    std::string vnic_name;

    sai_vport():
        mac_addr_offset(-1)
    {
        desc.fpp_id = 0;
        desc.if_index = 0;
        desc.data_sock = STD_INVALID_FD;
    }
    virtual ~sai_vport() {}
    bool read_cfg(std_config_node_t& fpp_node);

    t_std_error start_ctl_oper(int* sock, int* ns_handle);
    void finish_ctl_oper(int sock, int ns_handle);

    // List of (virtual) ports - addressed by if_index
    static std::unordered_map<int, sai_vport*> fp_ports_by_ifindex;
    static std::unordered_map<unsigned int, sai_vport*> fp_ports_by_hwport;
    static inline uint16_t add_offset(uint8_t* value, uint16_t offset) {
        uint16_t result = (uint16_t)*value + offset;
        *value = (uint8_t)(result & 0xFF);
        return (result >> 8) & 0xFF;
    }

public:
    static bool init(const char* cfg_file_name);
    static t_std_error init_packet_io();
    static void do_packet_rx_loop(vport_packet_rx_t);

    static sai_vport* find_interface_by_ifindex(int if_index);
    static sai_vport* find_interface_by_hwport(unsigned int port_id);
    sai_status_t static set_mac_address (const sai_mac_t *mac_address);

    bool set_admin_state(bool enable);
    bool set_mtu_size(unsigned int mtu_sz);
    sai_port_oper_status_t get_oper_status();
    bool update_mac_address(const sai_mac_t *mac_address);

    vport_desc_t* get_desc() { return &this->desc; }
};


std::unordered_map<int, sai_vport*> sai_vport::fp_ports_by_ifindex;
std::unordered_map<unsigned int, sai_vport*> sai_vport::fp_ports_by_hwport;

bool sai_vport::read_cfg(std_config_node_t& fpp_node)
{
    char *fpp_id = std_config_attr_get(fpp_node, FP_ID_ATTRIBUTE_STRING);
    if (NULL == fpp_id) {
        EV_LOGGING(SAI_SWITCH, ERR, "SAI-VM-VFPP", "Cannot get attr: %s", FP_ID_ATTRIBUTE_STRING);
        return false;
    }
    char *interface_name = std_config_attr_get(fpp_node, INTERFACE_NAME_ATTRIBUTE_STRING);
    if(NULL == interface_name) {
        EV_LOGGING(SAI_SWITCH, ERR, "SAI-VM-VFPP", "Cannot get attr: %s", INTERFACE_NAME_ATTRIBUTE_STRING);
        return false;
    }
    char *mac_offset = std_config_attr_get(fpp_node, MAC_OFFSET_ATTRIBUTE_STRING);
    if(NULL == mac_offset) {
        EV_LOGGING(SAI_SWITCH, ERR, "SAI-VM-VFPP", "Cannot get attr: %s", MAC_OFFSET_ATTRIBUTE_STRING);
        return false;
    }

    std_config_node_t entry_node = std_config_get_child(fpp_node);

    /* check for entry node, skip others */
    if (strcmp(std_config_name_get(entry_node), ENTRY_NODE_STRING)) {
        EV_LOGGING(SAI_SWITCH, ERR, "SAI-VM-VFPP", "Cannot get attr: %s - ifname=%s", ENTRY_NODE_STRING, interface_name);
        return false;
    }

    char *hwport = std_config_attr_get(entry_node, HW_PORT_ATTRIBUTE_STRING);
    if (NULL == hwport) {
        EV_LOGGING(SAI_SWITCH, ERR, "SAI-VM-VFPP", "No HW ports for: %s", interface_name);
        return false;
    }
    desc.npu_port_id = atoi(hwport);
    desc.fpp_id      = atoi(fpp_id);
    mac_addr_offset = atoi(mac_offset);
    if_name = std::string(interface_name);
    vnic_name = std::string(VNIC_PORT_PREFIX) + std::to_string(desc.fpp_id);

    desc.if_index = if_nametoindex(vnic_name.c_str());
    if (0 == desc.if_index) {

        // LOG - info message - the case is valid, if the VM has less fewer adapter
        // than the VM flavor it represents
        EV_LOGGING(SAI_SWITCH, INFO, "SAI-VM-VFPP", "Cannot get if_index for: %s (%s) errno=%s(%d)",
                vnic_name.c_str(), if_name.c_str(), strerror(errno), errno);
        return false;
    }

    return true;
}


bool sai_vport::init(const char* cfg_file_name)
{
    bool rc = false;
    std_config_node_t root_node = NULL;
    std_config_node_t fpp_node = NULL;

    std_config_hdl_t  cfg_hdl = std_config_load(cfg_file_name);
    if (cfg_hdl == NULL) {
        EV_LOGGING(SAI_SWITCH, ERR, "SAI-VM-VFPP", "Cannot open configuration file (%s)",
                cfg_file_name);
        return rc;
    }

    root_node = std_config_get_root(cfg_hdl);
    int crt_ns_handle = STD_INVALID_FD;

    do {
        t_std_error rc = std_sys_set_netns(VPORT_NAME_SPACE, &crt_ns_handle);
        if (rc != STD_ERR_OK) {
            EV_LOGGING(SAI_SWITCH, ERR, "SAI-VM-VFPP", "Cannot set namespace (%s)",
                    VPORT_NAME_SPACE);
            crt_ns_handle = STD_INVALID_FD;
            break;
        }
        /*
         * Scan each child node and look for the "front-panel" nodes.
         * For each front panel node, extract the (interface) name and read
         * the FIRST hwport entry.  At this time, we are not interested in
         * the additional (breakout) ports available to that interface name
         */
        for (fpp_node = std_config_get_child(root_node); fpp_node != 0;
                fpp_node = std_config_next_node(fpp_node)) {

            /* check for front-panel-port node, skip others */
            if (strcmp(std_config_name_get(fpp_node), FRONT_PORT_NODE_STRING))
                continue;

            sai_vport *vfpp = new sai_vport();

            if (vfpp->read_cfg(fpp_node)) {
                fp_ports_by_ifindex.insert(std::pair<int, sai_vport*>(vfpp->desc.if_index, vfpp));
                /** @TODO: deal with break out ports */
                fp_ports_by_hwport.insert(std::pair<unsigned int, sai_vport*>(vfpp->desc.npu_port_id, vfpp));
            }
            else {
                delete vfpp;
            }
        }
        rc = true;
    } while (0);

    if(crt_ns_handle != STD_INVALID_FD) {
        std_sys_reset_netns(&crt_ns_handle);
    }
    /* close and cleanup allocated memory */
    std_config_unload(cfg_hdl);
    return rc;
}


// Initialize packet I/O sockets -must be called in the thread where socjets are used
t_std_error sai_vport::init_packet_io()
{
    int crt_ns_handle = STD_INVALID_FD;
    t_std_error rc = std_sys_set_netns(VPORT_NAME_SPACE, &crt_ns_handle);
    if (rc != STD_ERR_OK) {
        EV_LOGGING(SAI_SWITCH, ERR, "SAI-VM-VFPP", "Cannot set namespace (%s)",
                VPORT_NAME_SPACE);
        return rc;
    }

    for (std::unordered_map<int, sai_vport*>::iterator it = fp_ports_by_ifindex.begin();
            it != fp_ports_by_ifindex.end(); ++it) {

        static int val = 1;
        struct sockaddr_ll sock_address;

        sai_vport* vfpp = it->second;
        rc = std_socket_create (e_std_sock_PACKET,
                e_std_sock_type_RAW,
                htons(ETH_P_ALL),
                (const std_socket_address_t*)NULL,
                &vfpp->desc.data_sock);

        if (rc != STD_ERR_OK) {
            EV_LOGGING(SAI_SWITCH, ERR, "SAI-VM-VFPP", "Cannot open socket for (%s)", vfpp->vnic_name.c_str());
            // Mark "not in use"
            vfpp->desc.data_sock = STD_INVALID_FD;
            continue;
        }

        if (setsockopt(vfpp->desc.data_sock, SOL_PACKET, PACKET_AUXDATA, &val, sizeof(val)) < 0) {
            EV_LOGGING(SAI_SWITCH,ERR,"SAI-VM-VFPP","setsockopt failed vnic=%s ifindex=%u errno=%s(%d)",
                    vfpp->vnic_name.c_str(), vfpp->desc.if_index,
                    strerror(errno), errno);
            std_close(vfpp->desc.data_sock);
            vfpp->desc.data_sock = STD_INVALID_FD;
            continue;
        }

        memset(&sock_address, 0, sizeof(sock_address));
        sock_address.sll_family = PF_PACKET;
        sock_address.sll_protocol = htons(ETH_P_ALL);
        sock_address.sll_ifindex = vfpp->desc.if_index;
        if (bind(vfpp->desc.data_sock, (struct sockaddr*) &sock_address, sizeof(sock_address)) < 0) {
            EV_LOGGING(SAI_SWITCH,ERR,"SAIHOSTIF","Bind failed name=%s ifindex=%u errno=%s(%d)",
                    vfpp->vnic_name.c_str(), vfpp->desc.if_index,
                    strerror(errno), errno);
            std_close(vfpp->desc.data_sock);
            vfpp->desc.data_sock = STD_INVALID_FD;
            continue;
        }
    }

    std_sys_reset_netns(&crt_ns_handle);
    return STD_ERR_OK;
}

// Packet I/O RX loop; must be called from its own thread
void sai_vport::do_packet_rx_loop(vport_packet_rx_t packet_rx)
{
    fd_set fds;
    int max_sock = 0;

    EV_LOGGING(SAI_SWITCH,INFO,"SAI-VM-VFPP","Starting packet I/O RX");

    memset(&fds, 0, sizeof(fd_set));
    std::unordered_map<int, sai_vport*>::iterator it;
    for (it = fp_ports_by_ifindex.begin();
            it != fp_ports_by_ifindex.end(); ++it) {

        sai_vport *vfpp = it->second;
        if (vfpp->desc.data_sock != STD_INVALID_FD) {
            FD_SET(vfpp->desc.data_sock, &fds);
            if (max_sock < vfpp->desc.data_sock) {
                max_sock = vfpp->desc.data_sock;
            }
        }
    }

    while (true) {
        fd_set readfds;

        memcpy(&readfds, &fds, sizeof(fd_set));
        int rc = select(max_sock+1, &readfds, NULL, NULL, NULL);
        if (rc > 0) {
            for (it = fp_ports_by_ifindex.begin();
                 it != fp_ports_by_ifindex.end(); ++it) {

                sai_vport *vfpp = it->second;

                if (vfpp->desc.data_sock != STD_INVALID_FD) {
                    if (FD_ISSET(vfpp->desc.data_sock, &readfds)) {
                        packet_rx(&vfpp->desc);
                    }
                }
            }
        }
        else {
            if (errno != EINTR) {
                sleep(3);
                /** @TODO: Reset all sockets ? Not much can be done here...*/
                EV_LOGGING(SAI_SWITCH,ERR,"SAI-VM-VFPP","Packet I/O RX select error rc=%d errno=%s(%d)",
                        rc, strerror(errno), errno);
            }
        }
    }
}


sai_vport* sai_vport::find_interface_by_ifindex(int if_index)
{
    std::unordered_map<int, sai_vport*>::iterator it = fp_ports_by_ifindex.find(if_index);
    if (it != fp_ports_by_ifindex.end()) {

        return it->second;
    }
    return NULL;

}

sai_vport* sai_vport::find_interface_by_hwport(unsigned int hw_port)
{
    std::unordered_map<unsigned int, sai_vport*>::iterator it = fp_ports_by_hwport.find(hw_port);
    if (it != fp_ports_by_hwport.end()) {
        return it->second;
    }
    return NULL;
}

t_std_error sai_vport::start_ctl_oper(int* sock, int* ns_handle)
{
    *sock = STD_INVALID_FD;
    *ns_handle = STD_INVALID_FD;
    t_std_error rc = std_sys_set_netns(VPORT_NAME_SPACE, ns_handle);
    if (rc != STD_ERR_OK) {
        return rc;
    }
    rc = std_socket_create (e_std_sock_INET4,
            e_std_sock_type_DGRAM,
            0,
            (const std_socket_address_t*)NULL,
            sock);
    if (rc != STD_ERR_OK) {
        EV_LOGGING(SAI_SWITCH, ERR, "SAI-VM-VFPP", "Cannot open IOCTL socket");
        *sock = STD_INVALID_FD;
    }

    return rc;
}

void sai_vport::finish_ctl_oper(int sock, int ns_handle)
{
    if (sock != STD_INVALID_FD) {
        std_close(sock);
    }

    (void)std_sys_reset_netns((int*)&ns_handle);
}

bool sai_vport::set_admin_state(bool enable)
{
    bool res = false;
    /** @TODO: Optimize this operation: e.g. keep the socket open at all times.
     * However, it is not clear that this operation function is always called from the same socket,
     * thus we cannot rely on the network namespace being the same. */
    int sock = STD_INVALID_FD;
    int crt_ns_handle = STD_INVALID_FD;
    t_std_error rc = start_ctl_oper(&sock, &crt_ns_handle);
    if (STD_ERR_OK == rc) {

        do {
            struct ifreq  ifr;
            safestrncpy(ifr.ifr_ifrn.ifrn_name,vnic_name.c_str(),sizeof(ifr.ifr_ifrn.ifrn_name));

            if (ioctl(sock, SIOCGIFFLAGS, &ifr) != 0) {
                EV_LOGGING(SAI_SWITCH,ERR,"SAI-VM-VFPP","ifname=%s ioctl GET IFFFLAGS errno=%s(%d)",
                        if_name.c_str(), strerror(errno), errno);
                break;
            }
            if (enable) {
                ifr.ifr_flags |= IFF_UP;
            } else {
                ifr.ifr_flags &= ~IFF_UP;
            }
            if (ioctl(sock, SIOCSIFFLAGS, &ifr) != 0) {
                EV_LOGGING(SAI_SWITCH,ERR,"SAI-VM-VFPP","ifname=%s ioctl SET IFFFLAGS errno=%s(%d)",
                        if_name.c_str(), strerror(errno), errno);
                break;
            }
            res = true;
        }
        while (0);
    }

    finish_ctl_oper(sock, crt_ns_handle);
    return res;
}

bool sai_vport::set_mtu_size(unsigned int mtu_sz)
{
    bool res = false;
    int sock = STD_INVALID_FD;
    int crt_ns_handle = STD_INVALID_FD;
    t_std_error rc = start_ctl_oper(&sock, &crt_ns_handle);
    if (STD_ERR_OK == rc) {

        struct ifreq  ifr;
        safestrncpy(ifr.ifr_ifrn.ifrn_name,vnic_name.c_str(),sizeof(ifr.ifr_ifrn.ifrn_name));

        ifr.ifr_mtu = mtu_sz;
        if (ioctl(sock, SIOCSIFMTU, &ifr) != 0) {
            EV_LOGGING(SAI_SWITCH,ERR,"SAI-VM-VFPP","ifname=%s ioctl SET MTU errno=%s(%d)",
                       if_name.c_str(), strerror(errno), errno);
        }
        else {
            res = true;
        }
    }

    finish_ctl_oper(sock, crt_ns_handle);
    return res;
}

sai_port_oper_status_t sai_vport::get_oper_status()
{

    int sock = STD_INVALID_FD;
    int crt_ns_handle = STD_INVALID_FD;
    sai_port_oper_status_t port_status = SAI_PORT_OPER_STATUS_UNKNOWN;
    t_std_error rc = start_ctl_oper(&sock, &crt_ns_handle);
    if (STD_ERR_OK == rc) {

        struct ifreq  ifr;
        safestrncpy(ifr.ifr_ifrn.ifrn_name,vnic_name.c_str(),sizeof(ifr.ifr_ifrn.ifrn_name));

        if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
            EV_LOGGING(SAI_SWITCH,ERR,"SAI-VM-VFPP","ifname=%s ioctl errno=%s(%d)",
                    ifr.ifr_ifrn.ifrn_name, strerror(errno), errno);
        }
        else {
            port_status = ((ifr.ifr_flags & IFF_RUNNING) != 0) ?
                    SAI_PORT_OPER_STATUS_UP :
                    SAI_PORT_OPER_STATUS_DOWN;
        }
    }

    finish_ctl_oper(sock, crt_ns_handle);
    return port_status;
}

bool sai_vport::update_mac_address (const sai_mac_t *mac_address)
{

    struct ifreq  ifr;

    if (sizeof (sai_mac_t) > sizeof(ifr.ifr_hwaddr.sa_data)) {
        return false;
    }

    bool res = false;
    /** @TODO: Optimize this operation: e.g. keep the socket open at all times.
     * It is not clear that this operation function is always called from the same socket,
     * thus we cannot rely on the network namespace being the same, if we */
    int sock = STD_INVALID_FD;
    int crt_ns_handle = STD_INVALID_FD;
    t_std_error rc = start_ctl_oper(&sock, &crt_ns_handle);
    if (STD_ERR_OK == rc) {

        safestrncpy(ifr.ifr_ifrn.ifrn_name,vnic_name.c_str(),sizeof(ifr.ifr_ifrn.ifrn_name));
        memcpy(ifr.ifr_hwaddr.sa_data, mac_address, sizeof (sai_mac_t));

        // Add offset to last 3 bytes
        uint16_t carry = add_offset((uint8_t*)&ifr.ifr_hwaddr.sa_data[ETH_ALEN-1], (uint16_t)mac_addr_offset);
        carry = add_offset((uint8_t*)&ifr.ifr_hwaddr.sa_data[ETH_ALEN-2], carry);
        carry = add_offset((uint8_t*)&ifr.ifr_hwaddr.sa_data[ETH_ALEN-3], carry);
        if (carry != 0) {
            // This is an unlikely event, but we cannot modify the OUI portion of the MAC address
            EV_LOGGING(SAI_SWITCH,ERR,"SAI-VM-VFPP","name=%s %s offset=%u Set MAC address OUI cannot be modified",
                    if_name.c_str(), vnic_name.c_str(), mac_addr_offset);
        }

        ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;

        if (ioctl(sock, SIOCSIFHWADDR, &ifr) != 0) {
            EV_LOGGING(SAI_SWITCH,ERR,"SAI-VM-VFPP","ifname=%s ioctl Set MAC address errno=%s(%d)",
                    if_name.c_str(), strerror(errno), errno);
        }
        else {
            res = true;
        }
    }

    finish_ctl_oper(sock, crt_ns_handle);
    return res;
}


sai_status_t sai_vport::set_mac_address (const sai_mac_t *mac_address)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    for (std::unordered_map<int, sai_vport*>::iterator it = fp_ports_by_ifindex.begin();
         it != fp_ports_by_ifindex.end(); ++it) {

        sai_vport *vfpp = it->second;
        if (!vfpp->update_mac_address(mac_address)) {
            // already logged, nothing else to do
            rc = SAI_STATUS_FAILURE;
        }
    }
    return rc;
}


extern "C" sai_status_t sai_vport_get_npu_port(int if_index, sai_npu_port_id_t *port)
{
    sai_vport *vfpp = sai_vport::find_interface_by_ifindex(if_index);
    if (vfpp != NULL) {

        *port = (sai_npu_port_id_t)vfpp->get_desc()->npu_port_id;
        return SAI_STATUS_SUCCESS;
    }
    return SAI_STATUS_FAILURE;
}

extern "C" vport_desc_t* sai_vm_vport_get_desc(sai_npu_port_id_t port_id)
{
    sai_vport *vfpp = sai_vport::find_interface_by_hwport((unsigned int)port_id);

    if (NULL == vfpp) {
        return NULL;
    }

    return vfpp->get_desc();
}


extern "C" t_std_error sai_vport_init_packet_io(void)
{
    return sai_vport::init_packet_io();
}

extern "C" void sai_vport_do_packet_rx_loop(vport_packet_rx_t packet_rx)
{
    sai_vport::do_packet_rx_loop(packet_rx);
}


extern "C" bool sai_vport_set_admin_state(sai_npu_port_id_t port_id, bool enable)
{
    sai_vport *vfpp = sai_vport::find_interface_by_hwport((unsigned int)port_id);
    if (NULL == vfpp) {
        // This is not an error - we do not have such an interface in the VM
        EV_LOGGING(SAI_SWITCH,INFO,"SAI-VM-VFPP","Set Admin State - No such HW port = %u", (unsigned int)port_id);
        return true;
    }
    return vfpp->set_admin_state(enable);
}

extern "C" bool sai_vport_set_mtu_size(sai_npu_port_id_t port_id, unsigned int mtu_sz)
{
    sai_vport *vfpp = sai_vport::find_interface_by_hwport((unsigned int)port_id);
    if (NULL == vfpp) {
        // This is not an error - we do not have such an interface in the VM
        EV_LOGGING(SAI_SWITCH,INFO,"SAI-VM-VFPP","Set MTU Size - No such HW port = %u SZ=%u", (unsigned int)port_id, mtu_sz);
        return true;
    }
    return vfpp->set_mtu_size(mtu_sz);
}

extern "C" sai_status_t sai_vport_set_switch_mac_address (const sai_mac_t *mac_address)
{
    return sai_vport::set_mac_address (mac_address);
}


extern "C" sai_port_oper_status_t sai_vport_get_oper_status(sai_npu_port_id_t port_id)
{
    sai_vport *vfpp = sai_vport::find_interface_by_hwport((unsigned int)port_id);
    if (NULL == vfpp) {
        // This is not an error - we do not have such an interface in the VM
        EV_LOGGING(SAI_SWITCH,INFO,"SAI-VM-VFPP","Get oper status - No such HW port = %u", (unsigned int)port_id);
        return SAI_PORT_OPER_STATUS_UNKNOWN;
    }
    return vfpp->get_oper_status();
}


/***************************************************************************
 *
 * Description: Loads XML configuration information for port mapping.
 *
 **************************************************************************/
extern "C" void sai_vport_init(void)
{
    sai_vport::init(PORT_PHYSICAL_MAPPING_TABLE_XML);
}
