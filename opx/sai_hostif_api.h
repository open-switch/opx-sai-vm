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
* @file sai_hostif_api.h
*
* @brief This private header file contains SAI hostif API signatures
* to be used by other SAI common components
*
*******************************************************************************/
#ifndef __SAI_HOSTIF_API_H__
#define __SAI_HOSTIF_API_H__

#include "saihostif.h"
#include "saitypes.h"
#include "saistatus.h"

void sai_hostif_rx_register_callback(sai_packet_event_notification_fn rx_register_fn);
sai_status_t sai_hostif_get_default_trap_group(sai_attribute_t *attr);
#endif

