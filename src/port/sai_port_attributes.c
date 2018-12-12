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
* @file sai_port_attributes.c
*
* @brief This file contains SAI Port attributes default value initialization
* and sai_port_attr_info_t cache set and get APIs and few port attributes
* get implementation.
*
*************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "std_assert.h"

#include "saiswitch.h"
#include "saitypes.h"
#include "saiport.h"
#include "saistatus.h"

#include "sai_port_common.h"
#include "sai_port_utils.h"

/* CPU ports attribute info cache */
static sai_port_attr_info_t cpu_port_attr_info;

/* Port attributes default values */
void sai_port_attr_info_defaults_init(sai_port_attr_info_t *port_attr_info)
{
    if(port_attr_info == NULL) {
        SAI_PORT_LOG_TRACE("port_attr_info is %p in attr defaults init",  port_attr_info);
        return;
    }

    if(port_attr_info->default_init) {
        return;
    }
    port_attr_info->oper_status = SAI_DFLT_OPER_STATUS;
    port_attr_info->speed = SAI_DFLT_SPEED;
    port_attr_info->duplex = SAI_DFLT_FULLDUPLEX;
    port_attr_info->admin_state = SAI_DFLT_ADMIN_STATE;
    port_attr_info->media_type = SAI_DFLT_MEDIA_TYPE;
    port_attr_info->default_vlan = SAI_DFLT_VLAN;
    port_attr_info->default_vlan_priority = SAI_DFLT_VLAN_PRIORITY;
    port_attr_info->drop_untagged = SAI_DFLT_DROP_UNTAGGED;
    port_attr_info->drop_tagged = SAI_DFLT_DROP_TAGGED;
    port_attr_info->internal_loopback = SAI_DFLT_LOOPBACK_MODE;
    port_attr_info->update_dscp = SAI_DFLT_UPDATE_DSCP;
    port_attr_info->mtu = SAI_DFLT_MTU;
    port_attr_info->flow_control_mode = SAI_DFLT_FLOW_CONTROL_MODE;
    port_attr_info->pfc_enabled_bitmap = SAI_DFLT_PFC_ENABLED_BITMAP;
    port_attr_info->fec_mode = SAI_DFLT_FEC_MODE;
    port_attr_info->oui_code = SAI_DFLT_OUI_CODE;
    port_attr_info->default_init = true;
}

/* Set the default port attribute values */
void sai_port_attr_defaults_init(void)
{
    sai_port_info_t *port_info = NULL;

    SAI_PORT_LOG_TRACE("Attributes default value init");

    for (port_info = sai_port_info_getfirst(); (port_info != NULL);
         port_info = sai_port_info_getnext(port_info)) {

        sai_port_attr_info_defaults_init (&port_info->port_attr_info);
    }

    /* Fill defaults for CPU port */
    memset(&cpu_port_attr_info, 0, sizeof(cpu_port_attr_info));
    sai_port_attr_info_defaults_init(&cpu_port_attr_info);
}

sai_status_t sai_port_attr_type_get(sai_object_id_t port_id,
                                    const sai_port_info_t *sai_port_info,
                                    sai_attribute_value_t *value)
{
    sai_port_type_t port_type = sai_port_type_get(port_id);

    /*NULL check is not required for sai_port_info as cpu port will not have port info structure*/
    if(value == NULL) {
        SAI_PORT_LOG_TRACE("value is %p for port 0x%"PRIx64" in attr type get", value, port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch(port_type)
    {
        case SAI_PORT_TYPE_CPU:
        case SAI_PORT_TYPE_LOGICAL:
            value->s32 = port_type;
            break;
        default:
            return SAI_STATUS_INVALID_OBJECT_ID;
    }

    return SAI_STATUS_SUCCESS;
}

sai_port_attr_info_t *sai_port_attr_info_get_for_update(sai_object_id_t port,
                                                       sai_port_info_t *port_info_table)
{
    if(sai_is_obj_id_cpu_port(port)) {
        return &cpu_port_attr_info;

    } else if(sai_is_obj_id_logical_port(port)) {

        if (port_info_table == NULL) {
            return NULL;
        }
        return &port_info_table->port_attr_info;
    }

    return NULL;
}

const sai_port_attr_info_t *sai_port_attr_info_read_only_get(sai_object_id_t port,
                                                             const sai_port_info_t *port_info_table)
{
    if(sai_is_obj_id_cpu_port(port)) {
        return &cpu_port_attr_info;

    } else if(sai_is_obj_id_logical_port(port)) {

        if (port_info_table == NULL) {
            return NULL;
        }
        return &port_info_table->port_attr_info;
    }

    return NULL;
}
/* Cache the port attributes for VM and Dump */
sai_status_t sai_port_attr_info_cache_set(sai_object_id_t port_id,
                                          sai_port_info_t *port_info,
                                          const sai_attribute_t *attr)
{

    sai_port_attr_info_t *port_attr_info = sai_port_attr_info_get_for_update(port_id, port_info);

    /*NULL check is not required for sai_port_info as cpu port will not have port info structure*/
    if((port_attr_info == NULL) || (attr == NULL)) {
        SAI_PORT_LOG_TRACE("port_attr_info is %p attr is %p for port 0x%"PRIx64" in attr cache set",
                           port_attr_info, attr, port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    SAI_PORT_LOG_TRACE("Attribute %d cache update for port 0x%"PRIx64"", attr->id, port_id);

    switch(attr->id) {
        case SAI_PORT_ATTR_OPER_STATUS:
            port_attr_info->oper_status = attr->value.s32;
            break;

        case SAI_PORT_ATTR_SPEED:
            port_attr_info->speed = attr->value.u32;
            break;

        case SAI_PORT_ATTR_FULL_DUPLEX_MODE:
            port_attr_info->duplex = attr->value.booldata;
            break;

        case SAI_PORT_ATTR_AUTO_NEG_MODE:
            port_attr_info->autoneg = attr->value.booldata;
            break;

        case SAI_PORT_ATTR_ADMIN_STATE:
            port_attr_info->admin_state = attr->value.booldata;
            break;

        case SAI_PORT_ATTR_MEDIA_TYPE:
            port_attr_info->media_type = attr->value.s32;
            break;

        case SAI_PORT_ATTR_PORT_VLAN_ID:
            port_attr_info->default_vlan = attr->value.u16;
            break;

        case SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY:
            port_attr_info->default_vlan_priority = attr->value.u8;
            break;

        case SAI_PORT_ATTR_DROP_UNTAGGED:
            port_attr_info->drop_untagged = attr->value.booldata;
            break;

        case SAI_PORT_ATTR_DROP_TAGGED:
            port_attr_info->drop_tagged = attr->value.booldata;
            break;

        case SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE:
            port_attr_info->internal_loopback = attr->value.s32;
            break;

        case SAI_PORT_ATTR_UPDATE_DSCP:
            port_attr_info->update_dscp = attr->value.booldata;
            break;

        case SAI_PORT_ATTR_MTU:
            port_attr_info->mtu = attr->value.u32;
            break;

        case SAI_PORT_ATTR_META_DATA:
            port_attr_info->meta_data = attr->value.u32;
            break;

        case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE:
            port_attr_info->flow_control_mode = attr->value.s32;
            break;

        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL:
            port_attr_info->pfc_enabled_bitmap = attr->value.u8;
            break;

        case SAI_PORT_ATTR_FEC_MODE:
            port_attr_info->fec_mode = attr->value.s32;
            break;

        case SAI_PORT_ATTR_ADVERTISED_OUI_CODE:
            port_attr_info->oui_code = attr->value.u32;
            break;

        default:
            SAI_PORT_LOG_TRACE("Attribute %d not in cache list for port 0x%"PRIx64"",
                             attr->id, port_id);
    }

    return SAI_STATUS_SUCCESS;
}

bool sai_port_is_duplicate_attribute_val (sai_object_id_t port_id,
                                          const sai_port_info_t *port_info,
                                          const sai_attribute_t *attr)
{

    const sai_port_attr_info_t *port_attr_info = sai_port_attr_info_read_only_get(port_id,
                                                                                  port_info);

    /*NULL check is not required for sai_port_info as cpu port will not have port info structure*/
    if((port_attr_info == NULL) || (attr == NULL)) {
        SAI_PORT_LOG_TRACE("port_attr_info is %p attr is %p for port 0x%"PRIx64" in attr dup check",
                           port_attr_info, attr, port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch(attr->id) {
        case SAI_PORT_ATTR_OPER_STATUS:
            return (attr->value.s32 == port_attr_info->oper_status);

        case SAI_PORT_ATTR_SPEED:
            return (attr->value.u32 == port_attr_info->speed);

        case SAI_PORT_ATTR_FULL_DUPLEX_MODE:
            return (attr->value.booldata == port_attr_info->duplex);

        case SAI_PORT_ATTR_AUTO_NEG_MODE:
            return (attr->value.booldata == port_attr_info->autoneg);

        case SAI_PORT_ATTR_ADMIN_STATE:
            return (attr->value.booldata == port_attr_info->admin_state);

        case SAI_PORT_ATTR_MEDIA_TYPE:
            return (attr->value.s32 == port_attr_info->media_type);

        case SAI_PORT_ATTR_PORT_VLAN_ID:
            return (attr->value.u16 == port_attr_info->default_vlan);

        case SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY:
            return (attr->value.u8 == port_attr_info->default_vlan_priority);

        case SAI_PORT_ATTR_DROP_UNTAGGED:
            return (attr->value.booldata == port_attr_info->drop_untagged);

        case SAI_PORT_ATTR_DROP_TAGGED:
            return (attr->value.booldata == port_attr_info->drop_tagged);

        case SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE:
            return (attr->value.s32 == port_attr_info->internal_loopback);

        case SAI_PORT_ATTR_UPDATE_DSCP:
            return (attr->value.booldata == port_attr_info->update_dscp);

        case SAI_PORT_ATTR_MTU:
            return (attr->value.u32 == port_attr_info->mtu);

        case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE:
            return (attr->value.s32 == port_attr_info->flow_control_mode);

        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL:
            return (attr->value.u8 == port_attr_info->pfc_enabled_bitmap);

        case SAI_PORT_ATTR_FEC_MODE:
            return (attr->value.s32 == port_attr_info->fec_mode);

        case SAI_PORT_ATTR_ADVERTISED_OUI_CODE:
            return (attr->value.u32 == port_attr_info->oui_code);

        default:
            return false;
    }
    return false;
}
sai_status_t sai_port_attr_info_cache_get(sai_object_id_t port_id,
                                          const sai_port_info_t *port_info,
                                          sai_attribute_t *attr)
{

    const sai_port_attr_info_t *port_attr_info = sai_port_attr_info_read_only_get(port_id,
                                                                                  port_info);


    /*NULL check is not required for sai_port_info as cpu port will not have port info structure*/
    if((port_attr_info == NULL) || (attr == NULL)) {
        SAI_PORT_LOG_TRACE("port_attr_info is %p attr is %p for port 0x%"PRIx64" in attr cache get",
                           port_attr_info, attr, port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch(attr->id) {
        case SAI_PORT_ATTR_OPER_STATUS:
            attr->value.s32 = port_attr_info->oper_status;
            break;

        case SAI_PORT_ATTR_SPEED:
            attr->value.u32 = port_attr_info->speed;
            break;

        case SAI_PORT_ATTR_FULL_DUPLEX_MODE:
            attr->value.booldata = port_attr_info->duplex;
            break;

        case SAI_PORT_ATTR_AUTO_NEG_MODE:
            attr->value.booldata = port_attr_info->autoneg;
            break;

        case SAI_PORT_ATTR_ADMIN_STATE:
            attr->value.booldata = port_attr_info->admin_state;
            break;

        case SAI_PORT_ATTR_MEDIA_TYPE:
            attr->value.s32 = port_attr_info->media_type;
            break;

        case SAI_PORT_ATTR_PORT_VLAN_ID:
            attr->value.u16 = port_attr_info->default_vlan;
            break;

        case SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY:
            attr->value.u8 = port_attr_info->default_vlan_priority;
            break;

        case SAI_PORT_ATTR_DROP_UNTAGGED:
            attr->value.booldata = port_attr_info->drop_untagged;
            break;

        case SAI_PORT_ATTR_DROP_TAGGED:
            attr->value.booldata = port_attr_info->drop_tagged;
            break;

        case SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE:
            attr->value.s32 = port_attr_info->internal_loopback;
            break;

        case SAI_PORT_ATTR_UPDATE_DSCP:
            attr->value.booldata = port_attr_info->update_dscp;
            break;

        case SAI_PORT_ATTR_MTU:
            attr->value.u32 = port_attr_info->mtu;
            break;

        case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE:
            attr->value.s32 = port_attr_info->flow_control_mode;
            break;

        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL:
            attr->value.u8 = port_attr_info->pfc_enabled_bitmap;
            break;

        case SAI_PORT_ATTR_FEC_MODE:
            attr->value.s32 = port_attr_info->fec_mode;
            break;

        case SAI_PORT_ATTR_ADVERTISED_OUI_CODE:
            attr->value.u32 = port_attr_info->oui_code;
            break;

        default:
            SAI_PORT_LOG_TRACE("Attribute %d not in cache list for port 0x%"PRIx64"",
                             attr->id, port_id);
            return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    return SAI_STATUS_SUCCESS;
}

