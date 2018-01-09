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
 * \file    sai_stp_debug.h
 *
 * \brief Declaration of SAI STP debug APIs
 */

#if !defined (__SAISTPDEBUG_H_)
#define __SAISTPDEBUG_H_

#include "saitypes.h"

void sai_stp_dump_global_info(sai_object_id_t stp_inst_id, bool all);

#endif
