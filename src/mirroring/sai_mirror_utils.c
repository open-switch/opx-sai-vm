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
* @file sai_mirror_utils.c
*
* @brief This file contains memory alloc and free functions for SAI mirror
*        data structures.
*
*************************************************************************/

#include <stdlib.h>
#include "std_mutex_lock.h"

#include "sai_mirror_defs.h"
#include "sai_mirror_api.h"
#include "saimirror.h"

static std_mutex_type_t mirror_lock;

sai_mirror_session_info_t *sai_mirror_session_node_alloc (void)
{
    return ((sai_mirror_session_info_t *)calloc (1,
                                        sizeof (sai_mirror_session_info_t)));
}

void sai_mirror_session_node_free (sai_mirror_session_info_t *p_session_info)
{
    free ((void *) p_session_info);
}

sai_mirror_port_info_t *sai_source_port_node_alloc (void)
{
    return ((sai_mirror_port_info_t *)calloc (1,
                                        sizeof (sai_mirror_port_info_t)));
}

void sai_source_port_node_free (sai_mirror_port_info_t *p_source_info)
{
    free ((void *) p_source_info);
}

void sai_mirror_mutex_lock_init(void)
{
    std_mutex_lock_create_static_init_fast (fast_lock);

    mirror_lock = fast_lock;
}

void sai_mirror_lock(void)
{
    std_mutex_lock (&mirror_lock);
}

void sai_mirror_unlock(void)
{
    std_mutex_unlock (&mirror_lock);
}
