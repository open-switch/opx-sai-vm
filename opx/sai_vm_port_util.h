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
 * @file sai_vm_port_util.h
 *
 * @brief This file contains the data structures and function prototypes for
 *        SAI Port functions in VM environment.
 */

#ifndef __SAI_VM_PORT_UTIL_H__
#define __SAI_VM_PORT_UTIL_H__

#include "sai_switch_init_config.h"
#include "sai_port_common.h"
#include "std_config_node.h"
#include "std_type_defs.h"

#include "saitypes.h"

#define SAI_VM_MAX_LANE_PER_PORT 4
/**
 * @brief SAI vm port initialization information.
 */
typedef struct _sai_vm_port_init_info_t {

    /** Port creation active or disabled */
    bool                         port_active;

    /** Port default state valid or invalid  */
    bool                         port_valid;

    /** Port instance id */
    uint_t                       port_inst_id;

    /** Logical/Local port id to index NPU API's */
    sai_npu_port_id_t            local_port_id;

    /** Physical Port Number */
    sai_npu_port_id_t            phy_port_id;

    /** Group number it is part of in a multi-lane port */
    uint_t                       port_group;

    /** PHY used Internal or External */
    sai_port_phy_t               phy_type;

    /** External PHY address */
    sai_npu_port_id_t            ext_phy_addr;

    /* Breakout modes supported- [sai_port_breakout_mode_type_t] */
    uint_t                       breakout_modes_support;

    /** Port speed in Gbps */
    sai_port_speed_t             port_speed;

    /** Port supported speed [sai_port_speed_capability_t] */
    uint_t                       port_speed_capb;
    /** Maximum  Lanes support per port */
    uint_t                       max_lanes_per_port;

    /** Bitmap of the active lanes in the port */
    uint64_t                     port_lane_bmap;

    bool                         eee_support;

    uint_t                       default_breakout_mode;
} sai_vm_port_init_info_t;

/**
 * @brief Type for representing supported speed information
 */
typedef struct _speed_desc_t {

    /** Representing SAI port supported speed capability */
    uint_t                       speed_cap_bit;

    /** Representing SAI port supported speed value in Mbps */
    sai_uint32_t                 speed_value;

    /** Representing SAI port supported speed attribute value */
    char                         *speed;

} speed_desc_t;

/**
 * Map supported speed information [supported speed capability, supported speed value in Mbps, attribute value]
 */
const speed_desc_t* get_speed_map(size_t *sz);

/**
 * For a given logical switch port, get the max lane count of its controlling port
 */
sai_status_t sai_vm_control_port_max_lanes_get(sai_object_id_t sai_port_id, uint_t *max_lanes);
/**
 * VM vendor specific switch init config handler
 */
sai_status_t sai_npu_switch_init_config (sai_switch_info_t *sai_switch_info,
                                         std_config_node_t vm_node);

#endif /* __SAI_VM_PORT_UTIL_H__ */
