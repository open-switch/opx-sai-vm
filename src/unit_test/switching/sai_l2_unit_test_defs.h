/*
 * filename: sai_l2_unit_test_defs.h
 * (c) Copyright 2015 Dell Inc. All Rights Reserved.
 */

/*
 * sai_l2_unit_test_defs.h
 *
 */

#include "saitypes.h"
#include "saifdb.h"
#include "sai_switch_utils.h"

#define SAI_GTEST_VLAN_MEMBER_ATTR_COUNT 3
#define SAI_GTEST_VLAN_MEMBER_PORT_COUNT 2
#define SAI_GTEST_VLAN 10
#define SAI_GTEST_VLAN_OBJ 0x2600000000000a
#define SAI_GTEST_INVALID_VLAN_ID 4096

/*
 * Stubs for Callback functions to be passed from adaptor host/application
 * upon switch initialization API call.
 */
inline void sai_port_state_evt_callback (uint32_t count,
                                         sai_port_oper_status_notification_t *data)
{
}

inline void sai_fdb_evt_callback(uint32_t count, sai_fdb_event_notification_data_t *data)
{
}

inline void sai_switch_operstate_callback (sai_switch_oper_status_t
                                                  switchstate)
{
}

/* Packet event callback
 */
static inline void sai_packet_event_callback (const void *buffer,
                                              sai_size_t buffer_size,
                                              uint32_t attr_count,
                                              const sai_attribute_t *attr_list)
{
}

inline void  sai_switch_shutdown_callback (void)
{
}
