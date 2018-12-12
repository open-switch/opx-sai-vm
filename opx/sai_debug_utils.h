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
* @file sai_debug_utils.h
*
* @brief This file contains utility APIs for SAI debug functions
*
*************************************************************************/
#ifndef __SAI_DEBUG_UTILS_H__
#define __SAI_DEBUG_UTILS_H__

#include <stdio.h>

/** \defgroup SAIDEBUGUTILAPIS SAI - Common Utility API
 *   Contains Debug Utility APIs to be used by other SAI components
 *  \{
 */


/** Utility to be used for debug/dump routines */
#define SAI_DEBUG(msg, ...) \
    do { \
        printf (msg"\n", ##__VA_ARGS__); \
    } while (0)


/**
 * \}
 */

#define SAI_BCM_PORT_DEBUG_DUMP_ALL 0

typedef enum sai_port_debug_function_t {
    SAI_PORT_BCM_INFO       = 1,
    SAI_PORT_BCM_HWINFO     = 2,
    SAI_PORT_BCM_PHYINFO    = 3,
    SAI_PORT_BCM_DEFAULT    = 4,
    SAI_PORT_BCM_DUMP_ALL   = 5,
    SAI_PORT_BCM_PORT_MAP   = 6,
}sai_port_debug_function_t;

void sai_port_attr_info_dump_port(sai_object_id_t port_id);
void sai_port_info_dump_port(sai_object_id_t port_id);
void sai_port_info_dump_all(void);


#endif
