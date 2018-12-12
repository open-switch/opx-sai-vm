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

/***
 * \file    sai_qos_debug.h
 *
 * \brief Declaration of SAI QOS debug APIs
 */

#if !defined (__SAIQOSDEBUG_H_)
#define __SAIQOSDEBUG_H_

#include "saitypes.h"

/* QOS Debug routines */
sai_status_t sai_dump_queue_info(sai_object_id_t queue_id);

sai_status_t sai_dump_sg_info(sai_object_id_t sg_id);

sai_status_t sai_dump_qos_port_info(sai_object_id_t port_id);

sai_status_t sai_dump_scheduler_info(sai_object_id_t scheduler_id);

sai_status_t sai_dump_all_queue_nodes (void);

sai_status_t sai_dump_all_sg_nodes (void);

sai_status_t sai_dump_all_scheduler_nodes (void);

sai_status_t sai_dump_port_hierarchy(sai_object_id_t port_id);

sai_status_t sai_dump_all_parents(sai_object_id_t oid);

sai_status_t sai_dump_pg_info(sai_object_id_t pg_id);

sai_status_t sai_dump_all_pg_nodes (void);

sai_status_t sai_dump_buffer_profile_info (sai_object_id_t profile_id);

sai_status_t sai_dump_all_buffer_profile_nodes (void);

sai_status_t sai_dump_buffer_pool_info (sai_object_id_t pool_id);

sai_status_t sai_dump_all_buffer_pool_nodes (void);

void sai_qos_maps_dump(sai_object_id_t map_id);

void sai_qos_maps_dump_all(void);

void sai_qos_wred_dump(sai_object_id_t wred_id);

void sai_qos_wred_dump_all(void);

void sai_qos_policer_dump(sai_object_id_t policer_id);

void sai_qos_policer_dump_all(void);
#endif
