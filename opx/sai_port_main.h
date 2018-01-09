/************************************************************************
* LEGALESE:   "Copyright (c); 2015, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* filename: sai_port_main.h
*
* description: This private header file contains common SAI port API signatures
* to be used by other SAI common components
*
*******************************************************************************/

/* OPENSOURCELICENSE */

#ifndef __SAI_PORT_MAIN_H__
#define __SAI_PORT_MAIN_H__

#include "saiport.h"
#include "sai_common_utils.h"

/*
 * SAI port link state change callback registration API
 */
void sai_port_state_register_callback(sai_port_state_change_notification_fn port_state_notf_fn);

/*
 * Get the list of valid Logical Port in the switch.
 */
sai_status_t sai_port_list_get(sai_object_list_t *objlist);

/*
 * Port Attribute set
 **/
sai_status_t sai_port_set_attribute(sai_object_id_t port_id,
                                    const sai_attribute_t *attr);

/*
 * SAI modules internal port event notification registration API.
 * port_event should be set to NULL, to unregister the notification.
 */
sai_status_t sai_port_event_internal_notif_register(sai_module_t module_id,
                                                    sai_port_event_notification_fn port_event);

#endif /* __SAI_PORT_MAIN_H__ */

