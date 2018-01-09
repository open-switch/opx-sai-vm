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
* @file sai_qos_mem.h
*
* @brief This file contains the prototype declarations for memory alloc
*        and free util functions
*
*************************************************************************/
#ifndef __SAI_QOS_MEM_H__
#define __SAI_QOS_MEM_H__

#include "sai_qos_common.h"
#include <stdlib.h>

static inline dn_sai_qos_queue_t *sai_qos_queue_node_alloc (void)
{
    return ((dn_sai_qos_queue_t *) calloc (1, sizeof (dn_sai_qos_queue_t)));
}

static inline void sai_qos_queue_node_free (dn_sai_qos_queue_t *p_queue_node)
{
    free ((void *) p_queue_node);
}

static inline dn_sai_qos_sched_group_t *sai_qos_sched_grp_node_alloc (void)
{
    return ((dn_sai_qos_sched_group_t *) calloc (1,
                                     sizeof (dn_sai_qos_sched_group_t)));
}

static inline void sai_qos_sched_group_node_free (dn_sai_qos_sched_group_t *p_sched_grp_node)
{
    free ((void *) p_sched_grp_node);
}

static inline dn_sai_qos_port_t *sai_qos_port_node_alloc (void)
{
    return ((dn_sai_qos_port_t *) calloc (1, sizeof (dn_sai_qos_port_t)));
}

static inline void sai_qos_port_node_free (dn_sai_qos_port_t *p_qos_port_node)
{
    free ((void *) p_qos_port_node);
}


static inline dn_sai_qos_map_t *sai_qos_maps_node_alloc(void)
{
    return ((dn_sai_qos_map_t *)calloc (1, sizeof (dn_sai_qos_map_t)));
}

static inline void sai_qos_map_node_list_free (sai_qos_map_t *p_list)
{
    free ((void *) p_list);
}

static inline void sai_qos_map_node_free (dn_sai_qos_map_t *p_map_node)
{
    free ((void *) p_map_node);
}

static inline dn_sai_qos_scheduler_t *sai_qos_scheduler_node_alloc (void)
{
    return ((dn_sai_qos_scheduler_t *) calloc (1, sizeof (dn_sai_qos_scheduler_t)));
}

static inline void sai_qos_scheduler_node_free (dn_sai_qos_scheduler_t *p_sched_node)
{
    free ((void *) p_sched_node);
}

static inline void sai_qos_policer_node_free(dn_sai_qos_policer_t *p_policer_node)
{
    free ((void *) p_policer_node);
}

static inline dn_sai_qos_policer_t *sai_qos_policer_node_alloc()
{
    return ((dn_sai_qos_policer_t *) calloc(1,sizeof (dn_sai_qos_policer_t)));
}

static inline dn_sai_qos_wred_t *sai_qos_wred_node_alloc(void)
{
    return ((dn_sai_qos_wred_t *)calloc (1, sizeof (dn_sai_qos_wred_t)));
}

static inline void sai_qos_wred_node_free (dn_sai_qos_wred_t *p_wred_node)
{
    free ((void *) p_wred_node);
}

static inline dn_sai_qos_buffer_pool_t *sai_qos_buffer_pool_node_alloc (void)
{
    return ((dn_sai_qos_buffer_pool_t *) calloc (1, sizeof (dn_sai_qos_buffer_pool_t)));
}

static inline void sai_qos_buffer_pool_node_free (dn_sai_qos_buffer_pool_t *p_buffer_pool_node)
{
    free ((void *) p_buffer_pool_node);
}

static inline dn_sai_qos_buffer_profile_t *sai_qos_buffer_profile_node_alloc (void)
{
    return ((dn_sai_qos_buffer_profile_t *) calloc (1, sizeof (dn_sai_qos_buffer_profile_t)));
}

static inline void sai_qos_buffer_profile_node_free (dn_sai_qos_buffer_profile_t *p_buffer_profile_node)
{
    free ((void *) p_buffer_profile_node);
}

static inline dn_sai_qos_pg_t *sai_qos_pg_node_alloc (void)
{
    return ((dn_sai_qos_pg_t *) calloc (1, sizeof (dn_sai_qos_pg_t)));
}

static inline void sai_qos_pg_node_free (dn_sai_qos_pg_t *p_pg_node)
{
    free ((void *) p_pg_node);
}

#endif  /* __SAI_QOS_MEM_H__ */
