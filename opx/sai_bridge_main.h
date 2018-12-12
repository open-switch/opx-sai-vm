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
* @file sai_bridge_main.h
*
* @brief Private header file contains the declarations for APIs used in
*        sai_bridge.c
*
*************************************************************************/


#ifndef __SAI_BRIDGE_MAIN_H__
#define __SAI_BRIDGE_MAIN_H__
#include "saitypes.h"
#include "sai_bridge_common.h"
#include "sai_common_utils.h"
#include "sai_lag_callback.h"

#define SAI_DEBUG_MAX_UT_PORTS 2
#define SAI_DEBUG_TEST_VLAN 10

typedef enum _sai_bridge_debug_dump_type {
    SAI_BRIDGE_DEBUG_DUMP_TYPE_VLAN,
    SAI_BRIDGE_DEBUG_DUMP_TYPE_STP,
    SAI_BRIDGE_DEBUG_DUMP_TYPE_MCAST,
} sai_bridge_debug_dump_type;

#define SAI_BRIDGE_DEF_BRIDGE_PORT_ATTR_COUNT 4
sai_status_t sai_bridge_init (void);

sai_status_t sai_bridge_default_id_get (sai_object_id_t *default_bridge_id);

sai_status_t sai_bridge_vlan_lag_callback_npu_dump(void);

sai_status_t sai_bridge_stp_lag_callback_npu_dump(void);

sai_status_t sai_bridge_mcast_lag_callback_npu_dump(void);

sai_status_t sai_bridge_port_admin_state_change_npu_dump(void);
/**
 * @brief Register callback for bridge port event
 *
 * @param[in] module_id Module Identifier
 * @return Pointer to attached port info structure
 */
sai_status_t sai_bridge_port_event_cb_register(sai_module_t module_id,
                                               uint_t bridge_port_type_bmp,
                                               sai_bridge_port_event_cb_fn bridge_port_cb);


void sai_bridge_dump_bridge_info(sai_object_id_t bridge_id);

void sai_brige_dump_default_bridge(void);

void sai_bridge_dump_bridge_port_info(sai_object_id_t bridge_port_id);

void sai_bridge_dump_all_bridge_ids(void);

void sai_bridge_dump_all_bridge_info(void);

void sai_bridge_dump_all_bridge_port_ids(void);

void sai_bridge_dump_all_bridge_port_info(void);

sai_status_t sai_bridge_port_get_stats(sai_object_id_t bridge_port_id,
                                       uint32_t number_of_counters,
                                       const sai_bridge_port_stat_t *counter_ids,
                                       uint64_t *counters);

sai_status_t sai_bridge_port_clear_stats(sai_object_id_t bridge_port_id,
                                         uint32_t number_of_counters,
                                         const sai_bridge_port_stat_t *counter_ids);

sai_status_t sai_bridge_lag_notification_handler (sai_object_id_t lag_id,
                                                  sai_lag_operation_t lag_opcode,
                                                  const sai_object_list_t *port_list);

sai_status_t sai_bridge_add_default_bridge_port(sai_object_id_t sai_port_id);

sai_status_t sai_bridge_port_get_attribute (sai_object_id_t bridge_port_id, uint32_t attr_count,
                                            sai_attribute_t *attr_list);

sai_status_t sai_api_bridge_port_set_attribute (sai_object_id_t bridge_port_id,
                                                const sai_attribute_t *attr);

sai_status_t sai_api_bridge_port_remove (sai_object_id_t bridge_port_id);

sai_status_t sai_bridge_port_remove (sai_object_id_t bridge_port_id);

sai_status_t sai_api_bridge_port_create(sai_object_id_t *bridge_port_id,
                                        sai_object_id_t switch_id, uint32_t attr_count,
                                        const sai_attribute_t *attr_list);

void sai_bridge_dump_multicast_info(sai_object_id_t bridge_id);
#endif
