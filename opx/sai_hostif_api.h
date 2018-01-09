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
* @file sai_hostif_api.h
*
* @brief This private header file contains SAI hostif API signatures
* to be used by other SAI common components
*
*******************************************************************************/
#ifndef __SAI_HOSTIF_API_H__
#define __SAI_HOSTIF_API_H__

#include "saihostintf.h"
#include "saitypes.h"
#include "saistatus.h"

void sai_hostif_rx_register_callback(sai_packet_event_notification_fn rx_register_fn);
sai_status_t sai_hostif_get_default_trap_group(sai_attribute_t *attr);
#endif

