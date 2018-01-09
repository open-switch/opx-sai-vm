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
