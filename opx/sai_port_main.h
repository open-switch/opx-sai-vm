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

