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
* @file  sai_qos_map_utils.c
*
* @brief This file contains function definitions for SAI QoS Maps
*        functionality API implementation.
*
*************************************************************************/
#include "sai_npu_qos.h"
#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_api_utils.h"
#include "sai_qos_mem.h"
#include "sai_common_infra.h"
#include "sai_npu_switch.h"

#include "sai.h"
#include "saiqosmap.h"

#include "std_type_defs.h"
#include "std_utils.h"
#include "std_assert.h"
#include <stdlib.h>
#include <inttypes.h>

sai_status_t sai_qos_map_port_list_update(dn_sai_qos_map_t *p_map)
{
    dn_sai_qos_port_t *p_qos_port_node = NULL;
    sai_status_t      sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_map != NULL);

    p_qos_port_node  = sai_qos_maps_get_port_node_from_map(p_map);

    while(p_qos_port_node != NULL)
    {

        sai_rc = sai_qos_map_npu_api_get()->port_map_set(p_qos_port_node->port_id,
                                                         p_map->key.map_id,
                                                         p_map->map_type,
                                                         true);

        if(sai_rc != SAI_STATUS_SUCCESS){
            break;
        }
        p_qos_port_node  = sai_qos_maps_next_port_node_from_map_get(p_map, p_qos_port_node);

    }

    return sai_rc;

}
void sai_qos_map_free_resources(dn_sai_qos_map_t *p_map_node)
{

    if(p_map_node == NULL){
        return;
    }

    SAI_MAPS_LOG_TRACE("Freeing the mapnode");
    /* Maps can be created with maptype alone. So the valuelist
     * can be NULL.Adding NULL check instead of Assert.
     */
    if(p_map_node->map_to_value.list != NULL){
        SAI_MAPS_LOG_TRACE("Freeing the mapnode list");
        sai_qos_map_node_list_free(p_map_node->map_to_value.list);
        p_map_node->map_to_value.list = NULL;
    }
    sai_qos_map_node_free(p_map_node);
}

sai_status_t sai_qos_map_type_attr_set(
dn_sai_qos_map_t *p_map_node, uint_t map_type)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    STD_ASSERT(p_map_node != NULL);

    switch(map_type)
    {
        case SAI_QOS_MAP_TYPE_DOT1P_TO_TC:
        case SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR:
        case SAI_QOS_MAP_TYPE_DSCP_TO_TC:
        case SAI_QOS_MAP_TYPE_DSCP_TO_COLOR:
        case SAI_QOS_MAP_TYPE_TC_TO_QUEUE:
        case SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP:
        case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE:
        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P:
        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP:
            p_map_node->map_type = map_type;

            SAI_MAPS_LOG_TRACE("Maptype updated in map node %d",
                   p_map_node->map_type);
            break;

        default:
            sai_rc = SAI_STATUS_NOT_SUPPORTED;
            break;
    }
    return sai_rc;
}

static sai_status_t sai_qos_map_update_ingress_maps(dn_sai_qos_port_t *p_qos_port_node,
                                       sai_qos_map_type_t map_type)
{
    sai_object_id_t map_id = 0;
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;

    STD_ASSERT(p_qos_port_node != NULL);

    SAI_MAPS_LOG_TRACE("Ingress map update for maptype %d",map_type);

    /**
     * NPU removes all ingress maps on setting NULL_OBJECT. So reapplying the maps
     * other than the removed one.
     */
    if((map_type == SAI_QOS_MAP_TYPE_DOT1P_TO_TC) ||
       (map_type == SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR)) {
        if(p_qos_port_node->maps_id[SAI_QOS_MAP_TYPE_DSCP_TO_TC] != SAI_NULL_OBJECT_ID){
            map_id = p_qos_port_node->maps_id[SAI_QOS_MAP_TYPE_DSCP_TO_TC];

            SAI_MAPS_LOG_TRACE("Ingress map update for maptype %d mapid 0x%"PRIx64"",
                               SAI_QOS_MAP_TYPE_DSCP_TO_TC, map_id);

            sai_rc = sai_qos_map_npu_api_get()->port_map_set
                (p_qos_port_node->port_id, map_id,
                 SAI_QOS_MAP_TYPE_DSCP_TO_TC, true);

            return sai_rc;
        }

        if(p_qos_port_node->maps_id[SAI_QOS_MAP_TYPE_DSCP_TO_COLOR] != SAI_NULL_OBJECT_ID){
            map_id = p_qos_port_node->maps_id[SAI_QOS_MAP_TYPE_DSCP_TO_COLOR];

            SAI_MAPS_LOG_TRACE("Ingress map update for maptype %d mapid 0x%"PRIx64"",
                               SAI_QOS_MAP_TYPE_DSCP_TO_COLOR, map_id);

            sai_rc = sai_qos_map_npu_api_get()->port_map_set
                (p_qos_port_node->port_id, map_id,
                 SAI_QOS_MAP_TYPE_DSCP_TO_COLOR, true);

            return sai_rc;
        }
    }
    else if((map_type == SAI_QOS_MAP_TYPE_DSCP_TO_TC) ||
            (map_type == SAI_QOS_MAP_TYPE_DSCP_TO_COLOR)) {

        if ((p_qos_port_node->maps_id[SAI_QOS_MAP_TYPE_DOT1P_TO_TC] == SAI_NULL_OBJECT_ID) &&
            (p_qos_port_node->maps_id[SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR] == SAI_NULL_OBJECT_ID)) {

            sai_rc = sai_qos_map_npu_api_get()->port_map_set
                     (p_qos_port_node->port_id, SAI_NULL_OBJECT_ID,
                      SAI_QOS_MAP_TYPE_DOT1P_TO_TC, true);
            return sai_rc;

        } else {

            if (p_qos_port_node->maps_id[SAI_QOS_MAP_TYPE_DOT1P_TO_TC] != SAI_NULL_OBJECT_ID) {
                map_id = p_qos_port_node->maps_id[SAI_QOS_MAP_TYPE_DOT1P_TO_TC];

                SAI_MAPS_LOG_TRACE("Ingress map update for maptype %d mapid 0x%"PRIx64"",
                                   SAI_QOS_MAP_TYPE_DOT1P_TO_TC, map_id);

                sai_rc = sai_qos_map_npu_api_get()->port_map_set
                         (p_qos_port_node->port_id, map_id,
                         SAI_QOS_MAP_TYPE_DOT1P_TO_TC, true);
                return sai_rc;
            }

            if (p_qos_port_node->maps_id[SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR] != SAI_NULL_OBJECT_ID) {
                map_id = p_qos_port_node->maps_id[SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR];

                SAI_MAPS_LOG_TRACE("Ingress map update for maptype %d mapid 0x%"PRIx64"",
                                   SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR, map_id);

                sai_rc = sai_qos_map_npu_api_get()->port_map_set
                         (p_qos_port_node->port_id, map_id,
                         SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR, true);
                return sai_rc;
            }

        }
    }

    return sai_rc;
}

static sai_status_t sai_qos_map_port_remove(dn_sai_qos_port_t *p_qos_port_node,
                                            sai_qos_map_type_t map_type)
{
    STD_ASSERT(p_qos_port_node != NULL);
    dn_sai_qos_map_t    *p_map_node = NULL;
    sai_object_id_t     map_id = SAI_NULL_OBJECT_ID;
    sai_status_t       sai_rc = SAI_STATUS_SUCCESS;

    map_id = p_qos_port_node->maps_id[map_type];

    p_map_node = sai_qos_map_node_get(map_id);

    if(p_map_node == NULL){
        SAI_MAPS_LOG_ERR ("Qos Map 0x%"PRIx64" does not exist in tree.",
                          map_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    sai_rc = sai_qos_map_npu_api_get()->port_map_set(p_qos_port_node->port_id,
                                                     map_id, map_type, false);

    if(sai_rc != SAI_STATUS_SUCCESS){
        SAI_MAPS_LOG_ERR("Npu set failed to remove for mapid 0x%"PRIx64" on portid 0x%"PRIx64"",
                         map_id, p_qos_port_node->port_id);
        return sai_rc;
    }

    p_qos_port_node->maps_id[map_type] = SAI_NULL_OBJECT_ID;

    std_dll_remove(&p_map_node->port_dll_head,
                   &p_qos_port_node->maps_dll_glue[map_type]);

    SAI_MAPS_LOG_TRACE("Ingress map update for maptype %d port_id 0x%"PRIx64"",
                       map_type, p_qos_port_node->port_id);

    sai_rc = sai_qos_map_update_ingress_maps(p_qos_port_node, map_type);

    return sai_rc;
}

static sai_status_t sai_qos_map_port_add(dn_sai_qos_port_t *p_qos_port_node,
                                         sai_object_id_t map_id,
                                         sai_qos_map_type_t map_type)
{
    STD_ASSERT(p_qos_port_node != NULL);
    dn_sai_qos_map_t   *p_map_node = NULL;
    dn_sai_qos_map_t   *p_map_node_old = NULL;
    sai_status_t       sai_rc = SAI_STATUS_SUCCESS;

    p_map_node = sai_qos_map_node_get(map_id);

    if(p_map_node == NULL){
        SAI_MAPS_LOG_ERR ("Qos Map 0x%"PRIx64" does not exist in tree.",
                          map_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }


    sai_rc = sai_qos_map_npu_api_get()->port_map_set(p_qos_port_node->port_id,
                                                     map_id, map_type, true);

    if(sai_rc != SAI_STATUS_SUCCESS){
        SAI_MAPS_LOG_ERR("Npu set failed to add for mapid 0x%"PRIx64" on portid 0x%"PRIx64"",
                         map_id, p_qos_port_node->port_id);
        return sai_rc;
    }

    if(p_qos_port_node->maps_id[map_type] != SAI_NULL_OBJECT_ID)
    {
        p_map_node_old = sai_qos_map_node_get(p_qos_port_node->maps_id[map_type]);

        SAI_MAPS_LOG_TRACE("Old map id on the port is 0x%"PRIx64"",
                           p_qos_port_node->maps_id[map_type]);
        if(p_map_node_old != NULL)
        {
            SAI_MAPS_LOG_TRACE("Removing port dll glue from existing map");
            std_dll_remove(&p_map_node_old->port_dll_head,
                           &p_qos_port_node->maps_dll_glue[map_type]);
        }
    }

    p_qos_port_node->maps_id[map_type] = map_id;

    std_dll_insertatback(&p_map_node->port_dll_head,
                         &p_qos_port_node->maps_dll_glue[map_type]);

    SAI_MAPS_LOG_TRACE("Map id set success on portid 0x%"PRIx64"",
                       p_qos_port_node->port_id);
    return sai_rc;
}

sai_status_t sai_qos_map_on_port_set (sai_object_id_t port_id,
                                      const sai_attribute_t *attr,
                                      sai_qos_map_type_t map_type)
{
    sai_status_t  sai_rc = SAI_STATUS_SUCCESS;

    sai_qos_lock();

    sai_rc = sai_qos_map_on_port_set_internal (port_id, attr,
                                               map_type);
    sai_qos_unlock();

    return sai_rc;
}

sai_status_t sai_qos_map_on_port_set_internal
(sai_object_id_t port_id, const sai_attribute_t *attr,
 sai_qos_map_type_t map_type)
{
    sai_status_t        sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_port_t   *p_qos_port_node = NULL;
    sai_object_id_t     map_id = SAI_NULL_OBJECT_ID;

    STD_ASSERT(attr != NULL);

    if(map_type == SAI_QOS_MAP_INVALID_TYPE){
        return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }
    map_id = attr->value.oid;

    SAI_MAPS_LOG_TRACE("Map id 0x%"PRIx64" set on port id 0x%"PRIx64"",
                       map_id, port_id);

    p_qos_port_node = sai_qos_port_node_get(port_id);
    if(p_qos_port_node == NULL){
        SAI_MAPS_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                          port_id);

        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if(p_qos_port_node->maps_id[map_type] == map_id){
        SAI_MAPS_LOG_TRACE("Map id already on the port");
        return SAI_STATUS_SUCCESS;
    }

    if(map_id == SAI_NULL_OBJECT_ID){
        SAI_MAPS_LOG_TRACE("Object is Null.Remove map for port");
        sai_rc = sai_qos_map_port_remove(p_qos_port_node, map_type);
    }
    else{
        SAI_MAPS_LOG_TRACE("Apply Map object on port");
        sai_rc = sai_qos_map_port_add(p_qos_port_node, map_id, map_type);
    }

    if(sai_rc == SAI_STATUS_SUCCESS){
        SAI_MAPS_LOG_INFO("Qos Map id 0x%"PRIx64" set success on port 0x%"PRIx64"",
                           map_id, port_id);
    }
    else{
        SAI_MAPS_LOG_ERR("Qos Map id 0x%"PRIx64" set failed on port 0x%"PRIx64"",
                           map_id, port_id);
    }

    return sai_rc;
}
