/************************************************************************
* * LEGALESE:   "Copyright (c) 2017, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/***
 * \file    sai_lag_debug.h
 *
 * \brief Declaration of SAI LAG debug APIs
 */

#if !defined (__SAILAGDEBUG_H_)
#define __SAILAGDEBUG_H_

#include "saitypes.h"

void sai_dump_lag_info(sai_object_id_t lag_id, bool all);

sai_status_t sai_debug_validate_lag_attribute_set_on_members(sai_object_id_t lag_id);

#endif
