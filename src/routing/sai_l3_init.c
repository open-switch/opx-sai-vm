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
* @file sai_l3_init.c
*
* @brief This file contains initialization functions for SAI L3 component.
*
*************************************************************************/

#include "sai_l3_util.h"
#include "sai_l3_api.h"
#include "sai_switch_utils.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_mutex_lock.h"
#include <string.h>

/**************************************************************************
 *                            GLOBALS
 **************************************************************************/
static sai_fib_global_t g_sai_fib_config = {
    is_init_complete: false,
};

/* Simple Mutex lock for accessing FIB resources */
static std_mutex_lock_create_static_init_fast (g_sai_fib_lock);

/***************************************************************************
 *                          Accessor Functions
 ***************************************************************************/
sai_fib_global_t *sai_fib_access_global_config (void)
{
    return (&g_sai_fib_config);
}

/***************************************************************************
 *                          Private Functions
 ***************************************************************************/
void sai_fib_lock (void)
{
    std_mutex_lock (&g_sai_fib_lock);
}

void sai_fib_unlock (void)
{
    std_mutex_unlock (&g_sai_fib_lock);
}

sai_status_t sai_fib_global_init (void)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;

    if (sai_fib_is_init_complete ()) {
        return sai_rc;
    }

    do {
        memset (&g_sai_fib_config, 0, sizeof (sai_fib_global_t));

        g_sai_fib_config.vrf_tree =
            std_rbtree_create_simple ("SAI FIB VRF tree",
                                      STD_STR_OFFSET_OF (sai_fib_vrf_t, vrf_id),
                                      STD_STR_SIZE_OF (sai_fib_vrf_t, vrf_id));

        if (g_sai_fib_config.vrf_tree == NULL) {
            SAI_ROUTER_LOG_CRIT ("SAI VRF Tree creation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        g_sai_fib_config.router_interface_tree =
            std_rbtree_create_simple ("SAI FIB Router Interface tree",
                                      STD_STR_OFFSET_OF (sai_fib_router_interface_t, rif_id),
                                      STD_STR_SIZE_OF (sai_fib_router_interface_t, rif_id));

        if (g_sai_fib_config.router_interface_tree == NULL) {
            SAI_ROUTER_LOG_CRIT ("SAI Router Intf Tree creation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        g_sai_fib_config.nh_group_tree =
            std_rbtree_create_simple ("SAI FIB Next Hop Group tree",
                                      STD_STR_OFFSET_OF (sai_fib_nh_group_t, key),
                                      STD_STR_SIZE_OF (sai_fib_nh_group_t, key));

        if (g_sai_fib_config.nh_group_tree == NULL) {
            SAI_ROUTER_LOG_CRIT ("SAI Next Hop Group Tree creation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        g_sai_fib_config.nh_id_tree =
            std_rbtree_create_simple ("SAI FIB Next Hop ID tree",
                                      STD_STR_OFFSET_OF (sai_fib_nh_t, next_hop_id),
                                      STD_STR_SIZE_OF (sai_fib_nh_t, next_hop_id));

        if (g_sai_fib_config.nh_id_tree == NULL) {
            SAI_ROUTER_LOG_CRIT ("SAI Next Hop ID Tree creation failed.");

            sai_rc = SAI_STATUS_NO_MEMORY;
            break;
        }

        sai_rc = sai_switch_max_virtual_routers_get
            (&g_sai_fib_config.max_virtual_routers);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ROUTER_LOG_CRIT ("Failed to get maximum virtual routers in the switch.");

            break;
        }

        sai_rc =
            sai_switch_max_ecmp_paths_get (&g_sai_fib_config.max_ecmp_paths);

        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_ROUTER_LOG_CRIT ("Failed to get maximum ECMP paths in the switch.");

            break;
        }

        g_sai_fib_config.neighbor_mac_tree =
                      std_radix_create ("Neighbor_MAC_Tree",
                                        SAI_FIB_NEIGHBOR_MAC_ENTRY_TREE_KEY_LEN,
                                        NULL, NULL, 0);

        if (g_sai_fib_config.neighbor_mac_tree == NULL) {
            SAI_ROUTER_LOG_CRIT ("Failed to create Neighbor MAC Radix Tree.");

            break;
        }
    } while (0);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        sai_fib_global_cleanup ();
    } else {
        g_sai_fib_config.is_init_complete = true;

        SAI_ROUTER_LOG_INFO ("SAI FIB Global Data structures Init complete.");
    }

    return sai_rc;
}

void sai_fib_global_cleanup (void)
{
    if (g_sai_fib_config.vrf_tree != NULL)
    {
        std_rbtree_destroy (g_sai_fib_config.vrf_tree);
    }
    if (g_sai_fib_config.router_interface_tree != NULL)
    {
        std_rbtree_destroy (g_sai_fib_config.router_interface_tree);
    }
    if (g_sai_fib_config.nh_group_tree != NULL)
    {
        std_rbtree_destroy (g_sai_fib_config.nh_group_tree);
    }
    if (g_sai_fib_config.nh_id_tree != NULL)
    {
        std_rbtree_destroy (g_sai_fib_config.nh_id_tree);
    }
    if (g_sai_fib_config.neighbor_mac_tree != NULL)
    {
        std_radix_destroy (g_sai_fib_config.neighbor_mac_tree);
    }

    memset (&g_sai_fib_config, 0, sizeof (sai_fib_global_t));

    g_sai_fib_config.is_init_complete = false;
}

