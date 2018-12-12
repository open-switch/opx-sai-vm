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
* @file sai_samplepacket_utils.c
*
* @brief This file contains memory alloc and free functions for SAI samplepacket
*        data structures.
*
*************************************************************************/

#include "sai_samplepacket_defs.h"
#include "sai_samplepacket_api.h"
#include "saisamplepacket.h"

#include <stdlib.h>
#include "std_mutex_lock.h"

static std_mutex_type_t samplepacket_lock;

dn_sai_samplepacket_session_info_t *sai_samplepacket_session_node_alloc (void)
{
    return ((dn_sai_samplepacket_session_info_t *)calloc (1,
                                        sizeof (dn_sai_samplepacket_session_info_t)));
}

void sai_samplepacket_session_node_free (dn_sai_samplepacket_session_info_t *p_session_info)
{
    free ((void *) p_session_info);
}

dn_sai_samplepacket_port_info_t *sai_samplepacket_port_node_alloc (void)
{
    return ((dn_sai_samplepacket_port_info_t *)calloc (1,
                                        sizeof (dn_sai_samplepacket_port_info_t)));
}

void sai_samplepacket_port_node_free (dn_sai_samplepacket_port_info_t *p_source_info)
{
    free ((void *) p_source_info);
}

void sai_samplepacket_mutex_lock_init(void)
{
    std_mutex_lock_create_static_init_fast (fast_lock);

    samplepacket_lock = fast_lock;
}

void sai_samplepacket_lock(void)
{
    std_mutex_lock (&samplepacket_lock);
}

void sai_samplepacket_unlock(void)
{
    std_mutex_unlock (&samplepacket_lock);
}
