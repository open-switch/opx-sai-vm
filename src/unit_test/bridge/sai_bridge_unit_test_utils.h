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
 * filename: sai_bridge_unit_test_utils.h
 */

/*
 * sai_bridge_unit_test_utils.h
 *
 */

#include "saitypes.h"
#include "saiswitch.h"
#include "saibridge.h"

#define SAI_BRIDGE_GTEST_VLAN     10
#define SAI_BRIDGE_GTEST_VLAN_OBJ 0x2600000000000a
#define SAI_MAX_BRIDGE_PORTS      256

sai_status_t sai_bridge_ut_get_bridge_port_from_port(sai_switch_api_t *p_sai_switch_api_tbl,
                                                     sai_bridge_api_t *p_sai_bridge_api_tbl,
                                                     sai_object_id_t switch_id,
                                                     sai_object_id_t port_id,
                                                     sai_object_id_t *bridge_port_id);


sai_status_t sai_bridge_ut_create_bridge_port(sai_bridge_api_t * p_sai_bridge_api_tbl,
                                              sai_object_id_t switch_id,
                                              sai_object_id_t port_id,
                                              bool def_vlan_add,
                                              sai_object_id_t *bridge_port_id);

sai_status_t sai_bridge_ut_remove_bridge_port(sai_bridge_api_t * p_sai_bridge_api_tbl,
                                              sai_object_id_t switch_id,
                                              sai_object_id_t bridge_port_id,
                                              bool def_vlan_remove);

sai_status_t sai_bridge_ut_get_port_from_bridge_port(sai_bridge_api_t *p_sai_bridge_api_tbl,
                                                     sai_object_id_t bridge_port_id,
                                                     sai_object_id_t *port_id);

sai_status_t sai_bridge_ut_clean_bridge_port_configs(sai_object_id_t switch_id,
                                                     sai_object_id_t port_id);

sai_status_t sai_bridge_ut_init_bridge_port_configs(sai_object_id_t switch_id,
                                                    sai_object_id_t port_id);
