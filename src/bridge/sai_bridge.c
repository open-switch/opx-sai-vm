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
* @file sai_bridge.c
*
* @brief This file contains implementation of SAI Bridge APIs.
*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "std_assert.h"
#include "saibridge.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_lag_callback.h"
#include "sai_bridge_common.h"
#include "sai_bridge_npu_api.h"
#include "sai_bridge_api.h"
#include "sai_port_utils.h"
#include "sai_port_common.h"
#include "sai_lag_api.h"
#include "sai_oid_utils.h"
#include "sai_vlan_api.h"
#include "sai_l3_util.h"
#include "sai_common_infra.h"
#include "sai_switch_utils.h"
#include "sai_tunnel_util.h"
#include "sai_bridge_main.h"
#include "sai_vlan_main.h"
#include "sai_gen_utils.h"
#include "sai_l2mc_api.h"

/* SAI default 1Q bridge */
static sai_object_id_t  sai_default_bridge_id = SAI_NULL_OBJECT_ID;

static bool sai_bridge_is_attr_l2mc_object_attribute(uint_t attr_count,
                                                     const sai_attribute_t *attr_list)
{
    uint_t attr_idx = 0;
    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        if((attr_list[attr_idx].id == SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP) ||
           (attr_list[attr_idx].id == SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP) ||
            (attr_list[attr_idx].id == SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP)) {
            return true;
        }
    }
    return false;

}

static sai_status_t sai_bridge_validate_and_fill_attributes (dn_sai_bridge_info_t *bridge_info,
                                                             uint32_t attr_count,
                                                             const sai_attribute_t *attr_list)
{
    uint_t                     attr_idx = 0;
    bool                       type_present = false;
    dn_sai_bridge_flood_type_t flood_type = SAI_BRIDGE_FLOOD_TYPE_MAX;


    if (attr_count > 0) {
        STD_ASSERT ((attr_list != NULL));
    } else {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }
    if(bridge_info == NULL) {
        SAI_BRIDGE_LOG_TRACE("NULL bridge info passed in fill mandatory attr");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        switch(attr_list[attr_idx].id) {
            case SAI_BRIDGE_ATTR_TYPE:
                bridge_info->bridge_type = attr_list[attr_idx].value.s32;
                if((bridge_info->bridge_type != SAI_BRIDGE_TYPE_1Q) &&
                   (bridge_info->bridge_type != SAI_BRIDGE_TYPE_1D)) {
                    return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTR_VALUE_0, attr_idx);
                }
                if((bridge_info->bridge_type == SAI_BRIDGE_TYPE_1Q) &&
                   (sai_default_bridge_id != SAI_NULL_OBJECT_ID)) {
                    return SAI_STATUS_ITEM_ALREADY_EXISTS;
                }
                type_present = true;
                break;

            case SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES:
                bridge_info->max_learned_address = attr_list[attr_idx].value.u32;
                break;

            case SAI_BRIDGE_ATTR_LEARN_DISABLE:
                bridge_info->learn_disable = attr_list[attr_idx].value.s32;
                break;

            case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
            case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
            case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
                sai_bridge_get_flood_type_from_attr(attr_list[attr_idx].id, &flood_type);
                bridge_info->flood_control[flood_type] = attr_list[attr_idx].value.s32;
                break;

            case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
            case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
            case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP:
                sai_bridge_get_flood_type_from_attr(attr_list[attr_idx].id, &flood_type);
                bridge_info->l2mc_flood_group[flood_type] = attr_list[attr_idx].value.oid;


            case SAI_BRIDGE_ATTR_PORT_LIST:
                return sai_get_indexed_ret_val(SAI_STATUS_INVALID_ATTRIBUTE_0, attr_idx);
        }
    }
    if(!type_present) {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_bridge_attributes_validate (sai_bridge_type_t bridge_type,
                                                    uint_t attr_count,
                                                    const sai_attribute_t *attr_list,
                                                        dn_sai_operations_t op_type)
{
    sai_status_t                    sai_rc = SAI_STATUS_SUCCESS;
    uint_t                          max_vendor_attr_count = 0;
    const dn_sai_attribute_entry_t *p_vendor_attr = NULL;

    SAI_BRIDGE_LOG_TRACE ("Parsing attributes for bridge attribute count %d op_type %d.",
                          attr_count,  op_type);

    if (attr_count == 0) {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    STD_ASSERT(attr_list);

    sai_rc = sai_bridge_npu_api_get()->bridge_attr_table_get(bridge_type, &p_vendor_attr,
                                                             &max_vendor_attr_count);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    STD_ASSERT(p_vendor_attr != NULL);
    STD_ASSERT(max_vendor_attr_count > 0);

    sai_rc = sai_attribute_validate (attr_count, attr_list, p_vendor_attr,
                                     op_type, max_vendor_attr_count);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BRIDGE_LOG_ERR ("Attribute validation failed for %d operation", op_type);
    }

    return sai_rc;
}

static sai_status_t sai_bridge_set_attribute (sai_object_id_t bridge_id,
                                              const sai_attribute_t *attr)
{
    sai_status_t         sai_rc = SAI_STATUS_FAILURE;
    dn_sai_bridge_info_t *p_bridge_info = NULL;

    STD_ASSERT(attr != NULL);

    sai_rc = sai_bridge_cache_read (bridge_id, &p_bridge_info);
    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BRIDGE_LOG_ERR("Invalid bridge object id 0x%"PRIx64" in set attribute", bridge_id);
        return sai_rc;
    }
    if(p_bridge_info == NULL) {
        SAI_BRIDGE_LOG_ERR("Unable to retrieve valid bridge info for bridge 0x%"PRIx64"",
                bridge_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_rc = sai_bridge_attributes_validate(p_bridge_info->bridge_type,
                                            1, attr, SAI_OP_SET);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        return sai_rc;
    }

    if(sai_bridge_is_duplicate_attr_val(p_bridge_info, attr)) {
        SAI_BRIDGE_LOG_TRACE("Duplicate attr %d set on bridge 0x%"PRIx64"", attr->id, bridge_id);
        return SAI_STATUS_SUCCESS;
    }
    sai_rc = sai_bridge_npu_api_get()->bridge_set_attribute(p_bridge_info, attr);

    if(sai_rc != SAI_STATUS_SUCCESS) {

        SAI_BRIDGE_LOG_ERR("Bridge object id 0x%"PRIx64" set attribute id %d returns error %d",
                            bridge_id, attr->id, sai_rc);
        return sai_rc;
    }
    sai_bridge_update_attr_value_in_cache (p_bridge_info, attr);

    return sai_rc;
}

static sai_status_t sai_api_bridge_set_attribute (sai_object_id_t bridge_id,
                                                  const sai_attribute_t *attr)
{
    sai_status_t         sai_rc = SAI_STATUS_FAILURE;
    bool is_l2mc_attr = false;
    STD_ASSERT(attr != NULL);

    is_l2mc_attr = sai_bridge_is_attr_l2mc_object_attribute(1, attr);
    if(is_l2mc_attr) {
        sai_l2mc_lock();
    }

    sai_bridge_lock();
    sai_rc = sai_bridge_set_attribute(bridge_id, attr);
    sai_bridge_unlock();

    if(is_l2mc_attr) {
        sai_l2mc_unlock();
    }
    return sai_rc;
}
static sai_status_t sai_bridge_create (sai_object_id_t *bridge_id, sai_object_id_t switch_id,
                                       uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t         sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_bridge_info_t bridge_info;
    bool                 npu_update = false;

    STD_ASSERT(bridge_id != NULL);
    *bridge_id = SAI_NULL_OBJECT_ID;
    memset(&bridge_info,0,sizeof(bridge_info));
    sai_bridge_init_default_bridge_info(&bridge_info);

    do {
        sai_rc = sai_bridge_validate_and_fill_attributes(&bridge_info, attr_count, attr_list);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }

        sai_rc = sai_bridge_attributes_validate(bridge_info.bridge_type, attr_count,
                                                attr_list, SAI_OP_CREATE);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }


        bridge_info.switch_obj_id = switch_id;
        sai_rc = sai_bridge_npu_api_get()->bridge_create(bridge_id, &bridge_info);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }
        npu_update = true;

        bridge_info.bridge_id = *bridge_id;

        sai_rc = sai_bridge_cache_write (*bridge_id, &bridge_info);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }
    } while(0);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        if(npu_update) {
            sai_bridge_npu_api_get()->bridge_remove(&bridge_info);
        }
    }

    return sai_rc;
}

static sai_status_t sai_api_bridge_create (sai_object_id_t *bridge_id, sai_object_id_t switch_id,
                                           uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    bool is_l2mc_attr = false;
    STD_ASSERT(bridge_id != NULL);

    is_l2mc_attr = sai_bridge_is_attr_l2mc_object_attribute(attr_count, attr_list);
    if(is_l2mc_attr) {
        sai_l2mc_lock();
    }
    sai_bridge_lock();
    sai_rc = sai_bridge_create(bridge_id, switch_id, attr_count, attr_list);
    sai_bridge_unlock();
    if(is_l2mc_attr) {
        sai_l2mc_unlock();
    }
    return sai_rc;
}

static sai_status_t sai_bridge_remove (sai_object_id_t bridge_id)
{
    sai_status_t          sai_rc = SAI_STATUS_FAILURE;
    dn_sai_bridge_info_t *p_bridge_info = NULL;

    sai_rc = sai_bridge_cache_read (bridge_id, &p_bridge_info);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BRIDGE_LOG_ERR("Invalid bridge object id 0x%"PRIx64" in set attribute", bridge_id);
        return sai_rc;
    }

    if(p_bridge_info == NULL) {
        SAI_BRIDGE_LOG_ERR("Unable to retrieve valid bridge info for bridge 0x%"PRIx64"",
                           bridge_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if(p_bridge_info->ref_count > 0) {
        return SAI_STATUS_OBJECT_IN_USE;
    }
    sai_rc = sai_bridge_npu_api_get()->bridge_remove(p_bridge_info);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BRIDGE_LOG_ERR("Error %d in removing bridge 0x%"PRIx64" at npu", sai_rc, bridge_id);
        return sai_rc;
    }

    sai_bridge_cleanup_l2mc_group_references(p_bridge_info);
    sai_bridge_cache_delete(bridge_id);

    return sai_rc;
}

static sai_status_t sai_api_bridge_remove (sai_object_id_t bridge_id)
{
    sai_status_t          sai_rc = SAI_STATUS_FAILURE;

    sai_bridge_lock();
    do {
        if(bridge_id == sai_default_bridge_id) {
            SAI_BRIDGE_LOG_ERR("Error - cannot remove default bridge 0x%"PRIx64"", bridge_id);
            sai_rc = SAI_STATUS_OBJECT_IN_USE;
            break;
        }
        sai_rc = sai_bridge_remove(bridge_id);
    } while (0);

    sai_bridge_unlock();
    return sai_rc;
}
static sai_status_t sai_bridge_get_attribute (sai_object_id_t bridge_id, uint32_t attr_count,
                                              sai_attribute_t *attr_list)
{
    sai_status_t          sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_bridge_info_t *p_bridge_info = NULL;

    STD_ASSERT(attr_list != NULL);


    sai_bridge_lock();
    do {
        sai_rc = sai_bridge_cache_read (bridge_id, &p_bridge_info);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BRIDGE_LOG_ERR("Invalid bridge object id 0x%"PRIx64" in set attribute", bridge_id);
            break;
        }

        if(p_bridge_info == NULL) {
            SAI_BRIDGE_LOG_ERR("Unable to retrieve valid bridge info for bridge 0x%"PRIx64"",
                               bridge_id);
            sai_rc = SAI_STATUS_INVALID_PARAMETER;
            break;
        }

        sai_rc = sai_bridge_attributes_validate(p_bridge_info->bridge_type, attr_count,
                                                attr_list, SAI_OP_GET);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }

        sai_rc = sai_bridge_get_attr_value_from_bridge_info (p_bridge_info, attr_count, attr_list);

        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BRIDGE_LOG_ERR("Bridge object id 0x%"PRIx64" get attribute returns error %d",
                               bridge_id, sai_rc);
            break;
        }

    } while(0);
    sai_bridge_unlock();

    return sai_rc;
}

static sai_status_t sai_bridge_deinit()
{
    uint_t           bridge_port_count = 0;
    uint_t           port_idx = 0;
    sai_status_t     sai_rc = SAI_STATUS_FAILURE;
    sai_object_id_t  bridge_port_id = SAI_NULL_OBJECT_ID;

    sai_rc = sai_bridge_npu_api_get()->bridge_init(false);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BRIDGE_LOG_ERR("Error %d in bridge de init", sai_rc);
        return sai_rc;
    }

    sai_rc = sai_bridge_map_get_port_count(sai_default_bridge_id, &bridge_port_count);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BRIDGE_LOG_ERR("Error %d in getting bridge port count for bridge id 0x%"PRIx64"",
                           sai_rc, sai_default_bridge_id);
        return sai_rc;
    }

    for(port_idx = 0; port_idx < bridge_port_count; port_idx++) {
        sai_rc = sai_bridge_map_get_bridge_port_at_index(sai_default_bridge_id, port_idx,
                                                         &bridge_port_id);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BRIDGE_LOG_ERR("Error %d in getting bridge port[%d] for bridge id 0x%"PRIx64"",
                               sai_rc,  port_idx, sai_default_bridge_id);
            return sai_rc;
        }
        sai_rc = sai_bridge_port_remove(bridge_port_id);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BRIDGE_LOG_ERR("Error %d in removing bridge port 0x%"PRIx64" in deinit",
                               sai_rc, bridge_port_id);
            return sai_rc;
        }
    }

    sai_rc = sai_bridge_remove(sai_default_bridge_id);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BRIDGE_LOG_ERR("Error %d in removing bridge 0x%"PRIx64"",
                           sai_rc, sai_default_bridge_id);
        return sai_rc;
    }

    return sai_rc;
}

sai_status_t sai_bridge_init (void)
{
    sai_attribute_t  bridge_attr;
    sai_status_t     sai_rc = SAI_STATUS_FAILURE;
    sai_port_info_t *sai_port_info = NULL;

    sai_rc = sai_bridge_npu_api_get()->bridge_init(true);
    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_BRIDGE_LOG_ERR("Error %d in bridge init", sai_rc);
        return sai_rc;
    }

    /* Create a default 1Q Bridge */
    memset(&bridge_attr, 0, sizeof(bridge_attr));
    bridge_attr.id = SAI_BRIDGE_ATTR_TYPE;
    bridge_attr.value.s32 = SAI_BRIDGE_TYPE_1Q;

    sai_rc = sai_bridge_create (&sai_default_bridge_id, SAI_DEFAULT_SWITCH_ID,
                                1, &bridge_attr);

    do {
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BRIDGE_LOG_ERR("Error %d in creating default 1Q bridge", sai_rc);
            break;
        }

        /* Add all valid ports to 1Q Bridge */
        for (sai_port_info = sai_port_info_getfirst(); (sai_port_info != NULL);
                sai_port_info = sai_port_info_getnext(sai_port_info)) {
            if(!sai_is_port_valid(sai_port_info->sai_port_id)) {
                continue;
            }

            sai_rc  = sai_bridge_add_default_bridge_port(sai_port_info->sai_port_id);

            if(sai_rc != SAI_STATUS_SUCCESS) {
                SAI_BRIDGE_LOG_ERR("Error %d in adding port 0x%"PRIx64" to default 1Q bridge",
                        sai_rc, sai_port_info->sai_port_id);
                break;
            }
        }
        if(sai_rc != SAI_STATUS_SUCCESS) {
            break;
        }
        sai_rc = sai_lag_event_callback_register (SAI_MODULE_BRIDGE,
                                                  sai_bridge_lag_notification_handler);
        if(sai_rc != SAI_STATUS_SUCCESS) {
            SAI_BRIDGE_LOG_ERR("Error %d in registering bridge to lag module", sai_rc);
        }
    } while(0);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        sai_bridge_deinit();
    }

    return sai_rc;
}

sai_status_t sai_bridge_default_id_get (sai_object_id_t *default_bridge_id)
{
    if(sai_default_bridge_id == SAI_NULL_OBJECT_ID) {
        return SAI_STATUS_UNINITIALIZED;
    }
    if(default_bridge_id == NULL) {
        SAI_BRIDGE_LOG_TRACE("NULL bridge port id passed in default bridge id get");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *default_bridge_id = sai_default_bridge_id;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_bridge_get_stats(sai_object_id_t bridge_id, uint32_t number_of_counters,
                                         const sai_bridge_stat_t *counter_ids, uint64_t *counters)
{
    sai_status_t          sai_rc = SAI_STATUS_FAILURE;
    dn_sai_bridge_info_t *bridge_info = NULL;

    if((number_of_counters == 0) || (counter_ids == NULL) || (counters == NULL)) {
        SAI_BRIDGE_LOG_TRACE("counter_ids is %p num counters is %d for bridge id is 0x%"PRIx64"",
                             counter_ids, number_of_counters, bridge_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    sai_rc = sai_bridge_cache_read (bridge_id, &bridge_info);
    if ((sai_rc != SAI_STATUS_SUCCESS) || (bridge_info == NULL)) {
        SAI_BRIDGE_LOG_ERR("Invalid bridge object id 0x%"PRIx64" in set attribute", bridge_id);
        return sai_rc;
    }

    return sai_bridge_npu_api_get()->bridge_get_stats(bridge_info, number_of_counters,
                                                      counter_ids, counters);
}

static sai_status_t sai_bridge_clear_stats(sai_object_id_t bridge_id, uint32_t number_of_counters,
                                           const sai_bridge_stat_t *counter_ids)
{
    sai_status_t          sai_rc = SAI_STATUS_FAILURE;
    dn_sai_bridge_info_t *bridge_info = NULL;

    if((number_of_counters == 0) || (counter_ids == NULL)) {
        SAI_BRIDGE_LOG_TRACE("counter_ids is %p num counters is %d for bridge id is 0x%"PRIx64"",
                             counter_ids, number_of_counters, bridge_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    sai_rc = sai_bridge_cache_read (bridge_id, &bridge_info);
    if ((sai_rc != SAI_STATUS_SUCCESS) || (bridge_info == NULL)) {
        SAI_BRIDGE_LOG_ERR("Invalid bridge object id 0x%"PRIx64" in set attribute", bridge_id);
        return sai_rc;
    }

    return sai_bridge_npu_api_get()->bridge_clear_stats(bridge_info, number_of_counters,
                                                        counter_ids);
}

static sai_bridge_api_t sai_bridge_method_table =
{
    sai_api_bridge_create,
    sai_api_bridge_remove,
    sai_api_bridge_set_attribute,
    sai_bridge_get_attribute,
    sai_bridge_get_stats,
    sai_bridge_clear_stats,
    sai_api_bridge_port_create,
    sai_api_bridge_port_remove,
    sai_api_bridge_port_set_attribute,
    sai_bridge_port_get_attribute,
    sai_bridge_port_get_stats,
    sai_bridge_port_clear_stats
};

sai_bridge_api_t  *sai_bridge_api_query (void)
{
    return (&sai_bridge_method_table);
}
