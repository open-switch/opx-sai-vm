/************************************************************************
* LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file  sai_qos_unit_test_utils.h
*
* @brief This file contains  utility and helper function prototypes for
* testing the SAI QOS functionalities.
*
*************************************************************************/

#ifndef __SAI_QOS_UNIT_TEST_H__
#define __SAI_QOS_UNIT_TEST_H__

extern "C" {
#include "sai.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saiswitch.h"
#include "saiport.h"
#include "saischedulergroup.h"
#include "saiqueue.h"
#include <inttypes.h>
}

void SetUpTestCase();

/* Methods for retrieving SAI port id for routing test cases */
sai_object_id_t sai_qos_port_id_get (uint32_t port_index);
sai_object_id_t* sai_qos_update_port_list();
uint32_t * sai_qos_update_port_count();

sai_object_id_t sai_qos_invalid_port_id_get ();
sai_object_id_t sai_qos_max_ports_get ();

sai_status_t sai_test_switch_max_number_hierarchy_levels_get (unsigned int *hierarchy_levels);
sai_status_t sai_test_port_max_number_queues_get (sai_object_id_t  port_id,
                                                  unsigned int *queue_count);
sai_status_t sai_test_port_queue_id_list_get (sai_object_id_t  port_id,
                                              unsigned int queue_count,
                                              sai_object_id_t *p_queue_id_list);

sai_status_t sai_test_port_sched_group_id_count_get (sai_object_id_t port_id,
                                                     unsigned int *sg_count);

sai_status_t sai_test_port_sched_group_id_list_get (sai_object_id_t port_id,
                                                    unsigned int sg_count,
                                                    sai_object_id_t *p_sg_id_list);

sai_status_t sai_test_port_queue_create(sai_object_id_t port_id,
                                       sai_queue_type_t queue_type,
                                       uint8_t queue_index,
                                       sai_object_id_t parent_sg_id,
                                       sai_object_id_t *queue_id);

static inline sai_status_t sai_test_invalid_attr_status_code (
                                          sai_status_t status,
                                          unsigned int attr_index)
{
    return (status + SAI_STATUS_CODE (attr_index));
}
sai_status_t sai_test_scheduler_create (sai_object_id_t *p_sched_id,
                                        unsigned int attr_count, ...);
sai_status_t sai_test_scheduler_remove (sai_object_id_t sched_id);

sai_status_t sai_test_cpu_port_id_get (sai_object_id_t *port_id);

sai_status_t sai_test_queue_create(sai_object_id_t *p_queue_id,
                                   unsigned int attr_count, ...);

sai_status_t sai_test_queue_remove(sai_object_id_t queue_id);

extern sai_switch_api_t            *p_sai_switch_api_table;
extern sai_scheduler_group_api_t   *p_sai_qos_sg_api_table;
extern sai_queue_api_t             *p_sai_qos_queue_api_table;
extern sai_port_api_t              *p_sai_port_api_table;
extern sai_scheduler_api_t         *p_sai_scheduler_api_table;

extern void sai_port_state_evt_callback (uint32_t count, sai_port_oper_status_notification_t *data);
extern void sai_fdb_evt_callback(uint32_t count, sai_fdb_event_notification_data_t *data);
extern void sai_switch_operstate_callback (sai_switch_oper_status_t switchstate);
extern sai_status_t sai_packet_rx_callback (void * buffer, uint32_t buffer_size,
                                            uint32_t attr_count, sai_attribute_t *attr_list);
extern void  sai_switch_shutdown_callback (void);
extern void sai_packet_event_callback (const void *buffer,
                                sai_size_t buffer_size,
                                uint32_t attr_count,
                                const sai_attribute_t *attr_list);

extern sai_status_t sai_test_port_queue_count_and_id_list_get(sai_object_id_t port_id,
                                                    unsigned int *max_queues,
                                                    sai_object_id_t *queue_id_list);

sai_status_t sai_create_buffer_pool(sai_buffer_api_t* buffer_api_table,
                                    sai_object_id_t *pool_id, unsigned int size,
                                    sai_buffer_pool_type_t type,
                                    sai_buffer_pool_threshold_mode_t th_mode);
sai_status_t sai_create_buffer_profile(sai_buffer_api_t* buffer_api_table,
                                       sai_object_id_t *profile_id, unsigned int attr_bmp,
                                       sai_object_id_t pool_id, unsigned int size, int th_mode,
                                       int dynamic_th, unsigned int static_th,
                                       unsigned int xoff_th, unsigned int xon_th);
sai_status_t sai_qos_buffer_get_first_pg (sai_port_api_t* sai_port_api_table,
                                          sai_object_id_t port_id, sai_object_id_t *pg_obj);
sai_status_t sai_qos_buffer_get_first_queue (sai_port_api_t* sai_port_api_table,
                                             sai_object_id_t port_id,
                                             sai_object_id_t *queue_obj);

#endif /* __SAI_QOS_UNIT_TEST_H__ */
