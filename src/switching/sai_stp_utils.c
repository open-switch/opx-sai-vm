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
* @file sai_stp_utils.c
*
* @brief This file contains memory alloc and free functions for SAI stp
*        data structures.
*
*************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "std_mutex_lock.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_stp_defs.h"
#include "sai_stp_api.h"
#include "sai_stp_util.h"
#include "saistp.h"
#include "std_struct_utils.h"


static std_mutex_type_t stp_lock;

dn_sai_stp_info_t *sai_stp_info_node_alloc (void)
{
    return ((dn_sai_stp_info_t *)calloc (1,
                                        sizeof (dn_sai_stp_info_t)));
}

void sai_stp_info_node_free (dn_sai_stp_info_t *p_stp_info)
{
    free ((void *) p_stp_info);
}

sai_vlan_id_t *sai_stp_vlan_node_alloc (void)
{
    return ((sai_vlan_id_t *)calloc (1,
                                        sizeof (sai_vlan_id_t)));
}

void sai_stp_vlan_node_free (sai_vlan_id_t *p_vlan_node)
{
    free ((void *) p_vlan_node);
}

dn_sai_stp_port_info_t *sai_stp_port_node_alloc (void)
{
    return ((dn_sai_stp_port_info_t *)calloc (1,
                sizeof (dn_sai_stp_port_info_t)));
}

void sai_stp_port_node_free (dn_sai_stp_port_info_t *p_stp_port_node)
{
    free ((void *) p_stp_port_node);
}

void sai_stp_mutex_lock_init(void)
{
    std_mutex_lock_create_static_init_fast (fast_lock);

    stp_lock = fast_lock;
}

void sai_stp_lock(void)
{
    std_mutex_lock (&stp_lock);
}

void sai_stp_unlock(void)
{
    std_mutex_unlock (&stp_lock);
}

bool sai_stp_can_bridge_port_learn_mac (sai_vlan_id_t vlan_id, sai_object_id_t bridge_port_id)
{
    sai_status_t sai_rc;
    sai_object_id_t stp_inst_id;
    sai_stp_port_state_t port_stp_state;
    bool mac_learn_allowed = false;

    stp_inst_id =  sai_stp_get_instance_from_vlan_map(vlan_id);

    sai_rc = sai_npu_stp_port_state_get (stp_inst_id, bridge_port_id, &port_stp_state);

    if(sai_rc == SAI_STATUS_SUCCESS) {
        if(port_stp_state !=  SAI_STP_PORT_STATE_BLOCKING) {
            mac_learn_allowed = true;
        }
    }

    return mac_learn_allowed;
}

bool sai_stp_port_state_valid(sai_stp_port_state_t port_state)
{
    switch (port_state) {
        case SAI_STP_PORT_STATE_LEARNING:
        case SAI_STP_PORT_STATE_FORWARDING:
        case SAI_STP_PORT_STATE_BLOCKING:
            return true;
        default:
            return false;
    }
}

sai_status_t sai_stp_port_info_create(sai_object_id_t stp_port_id, sai_object_id_t stp_inst_id,
                                      sai_object_id_t bridge_port_id,
                                      sai_stp_port_state_t port_state)
{
    dn_sai_stp_info_t      *p_stp_info = NULL;
    dn_sai_stp_port_info_t *p_stp_port_info = NULL;
    bool                    stp_tree_insert = false;
    sai_status_t            sai_rc = SAI_STATUS_FAILURE;
    rbtree_handle           global_stp_port_tree = sai_stp_port_tree_get();
    rbtree_handle           stp_info_tree = sai_stp_global_info_tree_get();

    p_stp_info = (dn_sai_stp_info_t *) std_rbtree_getexact(stp_info_tree,
                                                          (void *)&stp_inst_id);
    if(p_stp_info == NULL) {
        SAI_STP_LOG_ERR("Error in getting STP info for instance 0x%"PRIx64"", stp_inst_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    p_stp_port_info = sai_stp_port_node_alloc();
    if (p_stp_port_info == NULL) {
        SAI_STP_LOG_ERR ("STP port node memory allocation failed STP Inst 0x%"PRIx64""
                         " Bridge Port 0x%"PRIx64"", stp_inst_id, bridge_port_id);
        return SAI_STATUS_NO_MEMORY;
    }

    do {
        p_stp_port_info->bridge_port_id = bridge_port_id;
        if(std_rbtree_getexact(p_stp_info->stp_port_tree, p_stp_port_info) != NULL) {
            SAI_STP_LOG_ERR ("Bridge Port 0x%"PRIx64" already associated with STP"
                             " instance 0x%"PRIx64"",bridge_port_id,stp_inst_id);
            sai_rc = SAI_STATUS_ITEM_ALREADY_EXISTS;
            break;
        }

        p_stp_port_info->stp_inst_id = stp_inst_id;
        p_stp_port_info->stp_port_id = stp_port_id;
        p_stp_port_info->port_state = port_state;

        if(std_rbtree_insert(p_stp_info->stp_port_tree, p_stp_port_info) != STD_ERR_OK) {
            SAI_STP_LOG_ERR ("STP port tree insert failed STP Inst 0x%"PRIx64" Bridge Port "
                             "0x%"PRIx64"", stp_inst_id, bridge_port_id);
            sai_rc = SAI_STATUS_FAILURE;
            break;

        }
        stp_tree_insert = true;

        if(std_rbtree_insert(global_stp_port_tree, p_stp_port_info) != STD_ERR_OK) {
            SAI_STP_LOG_ERR ("STP port global tree insert failed STP Inst 0x%"PRIx64 ""
                    "Bridge Port 0x%"PRIx64"", stp_inst_id, bridge_port_id);
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }
        p_stp_info->num_ports++;
        sai_rc = SAI_STATUS_SUCCESS;
    } while(0);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        if(stp_tree_insert) {
            std_rbtree_remove(p_stp_info->stp_port_tree, p_stp_port_info);
            p_stp_info->num_ports--;
        }
        sai_stp_port_node_free(p_stp_port_info);
    }
    return sai_rc;
}

sai_status_t sai_stp_port_info_remove(sai_object_id_t stp_port_id)
{
    rbtree_handle           global_stp_port_tree = sai_stp_port_tree_get();
    rbtree_handle           stp_info_tree = sai_stp_global_info_tree_get();
    dn_sai_stp_info_t      *p_stp_info = NULL;
    dn_sai_stp_port_info_t  stp_port_info;
    dn_sai_stp_port_info_t *p_stp_port_info = NULL;

    memset(&stp_port_info,0,sizeof(stp_port_info));
    stp_port_info.stp_port_id = stp_port_id;
    if((p_stp_port_info = (dn_sai_stp_port_info_t *)std_rbtree_getexact(
                    global_stp_port_tree,&stp_port_info)) == NULL) {
        SAI_STP_LOG_ERR ("STP port obj 0x%"PRIx64" not found",stp_port_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    std_rbtree_remove(global_stp_port_tree, p_stp_port_info);

    p_stp_info = (dn_sai_stp_info_t *) std_rbtree_getexact(stp_info_tree,
                                                          (void *)&p_stp_port_info->stp_inst_id);
    if (p_stp_info == NULL) {
        SAI_STP_LOG_ERR ("STP instance UOID not found 0x%"PRIx64"", p_stp_port_info->stp_inst_id);
        return SAI_STATUS_FAILURE;
    } else {
        std_rbtree_remove(p_stp_info->stp_port_tree, p_stp_port_info);
        sai_stp_port_node_free(p_stp_port_info);
        p_stp_info->num_ports--;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_stp_info_create(sai_object_id_t stp_inst_id)
{
    rbtree_handle      stp_info_tree = sai_stp_global_info_tree_get();
    dn_sai_stp_info_t *p_stp_info = NULL;
    sai_status_t       sai_rc = SAI_STATUS_FAILURE;
    bool               stp_tree_init = false;
    bool               vlan_tree_init = false;


    p_stp_info = sai_stp_info_node_alloc ();
    if (p_stp_info == NULL) {
        SAI_STP_LOG_ERR ("Memory allocation failed");
        return SAI_STATUS_NO_MEMORY;
    }

    do {
        p_stp_info->stp_inst_id = stp_inst_id;

        if (std_rbtree_insert (stp_info_tree, (void *) p_stp_info) != STD_ERR_OK) {
            SAI_STP_LOG_ERR ("Node insertion failed for session 0x%"PRIx64"",
                                                                p_stp_info->stp_inst_id);
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

        stp_tree_init = true;
        p_stp_info->vlan_tree = std_rbtree_create_simple ("vlan_tree",
                                                          0, sizeof(sai_vlan_id_t));

        if (p_stp_info->vlan_tree == NULL) {
            SAI_STP_LOG_ERR ("Vlan tree creations failed for session 0x%"PRIx64"",
                             p_stp_info->stp_inst_id);
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

        vlan_tree_init = true;
        p_stp_info->stp_port_tree = std_rbtree_create_simple ("stp_port_tree",
                STD_STR_OFFSET_OF(dn_sai_stp_port_info_t, bridge_port_id),
                STD_STR_SIZE_OF(dn_sai_stp_port_info_t, bridge_port_id));

        if (p_stp_info->stp_port_tree == NULL) {
            SAI_STP_LOG_ERR ("STP port tree creation failed for STP session"
                    " 0x%"PRIx64"", p_stp_info->stp_inst_id);
            sai_rc = SAI_STATUS_FAILURE;
            break;
        }

        p_stp_info->num_ports = 0;

        sai_rc = SAI_STATUS_SUCCESS;

    } while(0);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        if(vlan_tree_init) {
            std_rbtree_destroy (p_stp_info->vlan_tree);
        }
        if(stp_tree_init) {
            std_rbtree_remove (stp_info_tree, (void *) p_stp_info);
        }
        sai_stp_info_node_free ((dn_sai_stp_info_t *)p_stp_info);
    }
    return sai_rc;
}

sai_status_t sai_stp_inst_valid_for_delete(sai_object_id_t stp_inst_id)
{
    dn_sai_stp_info_t *p_stp_info  = NULL;
    rbtree_handle      stp_info_tree = sai_stp_global_info_tree_get();

    p_stp_info = (dn_sai_stp_info_t *) std_rbtree_getexact (
            stp_info_tree, (void *)&stp_inst_id);

    if (p_stp_info == NULL) {
        SAI_STP_LOG_ERR ("STP instance not found 0x%"PRIx64"", stp_inst_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    /* Check if any vlan is still attached to the session */
    if (std_rbtree_getfirst(p_stp_info->vlan_tree) != NULL) {
        SAI_STP_LOG_ERR ("Vlans are still attached to STP instance 0x%"PRIx64"", stp_inst_id);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    /* Check if any stp port obj is still attached to the session */
    if (p_stp_info->num_ports) {
        SAI_STP_LOG_ERR ("Some stp ports object still exists in STP session"
                " 0x%"PRIx64"", stp_inst_id);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    return SAI_STATUS_SUCCESS;

}
sai_status_t sai_stp_info_remove(sai_object_id_t stp_inst_id)
{
    dn_sai_stp_info_t *p_stp_info  = NULL;
    rbtree_handle      stp_info_tree = sai_stp_global_info_tree_get();


    p_stp_info = (dn_sai_stp_info_t *) std_rbtree_getexact (
            stp_info_tree, (void *)&stp_inst_id);

    if (p_stp_info == NULL) {
        SAI_STP_LOG_ERR ("STP instance not found 0x%"PRIx64"", stp_inst_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }


    if (p_stp_info->vlan_tree) {
        std_rbtree_destroy (p_stp_info->vlan_tree);
    }

    if (p_stp_info->stp_port_tree) {
        std_rbtree_destroy (p_stp_info->stp_port_tree);
    }

    p_stp_info->num_ports = 0;

    if (std_rbtree_remove (stp_info_tree , (void *)p_stp_info) != p_stp_info) {
        SAI_STP_LOG_ERR ("STP instance node remove failed 0x%"PRIx64"", stp_inst_id);
        return SAI_STATUS_FAILURE;
    }

    sai_stp_info_node_free (p_stp_info);

    return SAI_STATUS_SUCCESS;
}
