/************************************************************************
* * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/**
* @file sai_lag.c
*
* @brief This file contains implementation of SAI LAG APIs.
*************************************************************************/

#include "saitypes.h"
#include "sailag.h"
#include "saistatus.h"
#include "sai_modules_init.h"
#include "sai_lag_common.h"
#include "sai_npu_lag.h"
#include "sai_lag_api.h"
#include "sai_lag_callback.h"
#include "sai_port_utils.h"
#include "sai_gen_utils.h"
#include "sai_common_infra.h"
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "std_assert.h"

static sai_lag_event_t lag_event[SAI_MODULE_MAX];

sai_status_t sai_lag_init(void)
{
    sai_lag_cache_init();
    sai_lag_npu_api_get()->lag_init();
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_lag_event_callback_register(sai_module_t module_id, sai_lag_event_callback lag_callback)
{
    if(module_id >= SAI_MODULE_MAX) {
        SAI_LAG_LOG_ERR("Error! Unknown module id %d",module_id);
        return SAI_STATUS_FAILURE;
    }
    lag_event[module_id].lag_callback = lag_callback;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_lag_notify_modules (sai_object_id_t lag_id,
                                            sai_lag_operation_t lag_operation,
                                            const sai_object_list_t *lag_port_list)
{
    sai_module_t mod_idx = 0;


    for(mod_idx = 0; mod_idx < SAI_MODULE_MAX; mod_idx++) {

        if(lag_event[mod_idx].lag_callback != NULL) {
            lag_event[mod_idx].lag_callback(lag_id, lag_operation, lag_port_list);
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_l2_add_lag_port (sai_object_id_t  lag_id,
                                         sai_object_id_t  port_id,
                                         sai_object_id_t *member_id)
{
    sai_object_list_t port_list;
    sai_object_list_t member_id_list;
    sai_status_t      ret_val;

    SAI_LAG_LOG_TRACE ("Adding port 0x%"PRIx64" on lag:0x%"PRIx64"",
                       port_id, lag_id);

    memset (&port_list, 0, sizeof (port_list));
    memset (&member_id_list, 0, sizeof (member_id_list));

    port_list.count      = 1;
    port_list.list       = &port_id;
    member_id_list.count = 1;
    member_id_list.list  = member_id;

    ret_val = sai_lag_npu_api_get()->add_ports_to_lag (lag_id,
                                                       &port_list,
                                                       &member_id_list);
    if (ret_val != SAI_STATUS_SUCCESS) {
        return ret_val;
    }

    ret_val = sai_lag_port_node_add (lag_id, port_id, *member_id);

    if (ret_val != SAI_STATUS_SUCCESS) {
        sai_lag_npu_api_get()->remove_ports_from_lag (lag_id, &port_list);
    }

    return ret_val;
}

static sai_status_t sai_l2_remove_lag_port (sai_object_id_t lag_id,
                                            sai_object_id_t port_id)
{
    sai_object_list_t port_list;
    sai_status_t      ret_val = SAI_STATUS_SUCCESS;

    SAI_LAG_LOG_TRACE ("Removing port 0x%"PRIx64" from lag:0x%"PRIx64"",
                       port_id, lag_id);

    port_list.count = 1;
    port_list.list  = &port_id;

    ret_val = sai_lag_npu_api_get()->remove_ports_from_lag (lag_id, &port_list);

    sai_lag_port_node_remove (lag_id, port_id);

    return ret_val;
}

static sai_status_t sai_lag_apply_attribute(sai_object_id_t lag_id,
                                            const sai_attribute_t *attr)
{
    sai_status_t     ret_val = SAI_STATUS_FAILURE;
    sai_lag_node_t  *lag_info = NULL;
    sai_attribute_t  old_attr;

    STD_ASSERT(attr != NULL);
    lag_info = sai_lag_node_get(lag_id);
    if(lag_info == NULL) {
        SAI_LAG_LOG_WARN("lag id not found 0x%"PRIx64"",lag_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    old_attr.id = attr->id;
    ret_val = sai_lag_node_get_attr(lag_info, &old_attr);
    if(ret_val != SAI_STATUS_SUCCESS){
        SAI_LAG_LOG_ERR("Error %d in getting current attr %d value for lag 0x%"PRIx64"",
                        ret_val, attr->id, lag_id);
        return ret_val;
    }

    ret_val = sai_lag_npu_api_get()->lag_attribute_set(lag_info, attr);
    if(ret_val != SAI_STATUS_SUCCESS) {
        return ret_val;
    }

    ret_val = sai_lag_node_set_attr(lag_info, attr);
    if(ret_val != SAI_STATUS_SUCCESS) {
        sai_lag_npu_api_get()->lag_attribute_set(lag_info, &old_attr);
        return ret_val;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_l2_create_lag(sai_object_id_t *lag_id, sai_object_id_t switch_id,
                                      uint32_t attr_count,
                                      const sai_attribute_t *attr_list)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;
    unsigned int attr_idx = 0;

    if(attr_count > 0) {
        STD_ASSERT((attr_list != NULL));
    }
    sai_lag_lock();
    ret_val = sai_lag_npu_api_get()->lag_create(lag_id);
    if(ret_val != SAI_STATUS_SUCCESS) {
        SAI_LAG_LOG_ERR("Unable to create new LAG");
        sai_lag_unlock();
        return ret_val;
    }
    ret_val = sai_lag_node_add(*lag_id);
    if(ret_val != SAI_STATUS_SUCCESS) {
        SAI_LAG_LOG_WARN("Unable to add LAG id:0x%"PRIx64" in cache", *lag_id);
        sai_lag_npu_api_get()->lag_remove(*lag_id);
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        ret_val = sai_lag_apply_attribute(*lag_id, &attr_list[attr_idx]);
        if(ret_val != SAI_STATUS_SUCCESS) {
            ret_val = sai_get_indexed_ret_val(ret_val, attr_idx);
            break;
        }
    }

    SAI_LAG_LOG_TRACE("Return value is %d",ret_val);
    if(ret_val != SAI_STATUS_SUCCESS) {
        sai_lag_npu_api_get()->lag_remove(*lag_id);
        sai_lag_node_remove(*lag_id);
    }
    sai_lag_unlock();

    if(ret_val == SAI_STATUS_SUCCESS) {
        sai_lag_notify_modules (*lag_id, SAI_LAG_OPER_CREATE, NULL);
    }

    return ret_val;
}

static sai_status_t sai_l2_remove_lag(sai_object_id_t lag_id)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;
    unsigned int port_count = 0;

    sai_lag_lock();

    if(!sai_is_lag_created(lag_id)) {
        SAI_LAG_LOG_WARN("LAG id 0x%"PRIx64" not found",lag_id);
        sai_lag_unlock();
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    sai_lag_port_count_get(lag_id, &port_count);
    if((port_count != 0 ) || (sai_is_lag_in_use(lag_id))) {
        SAI_LAG_LOG_ERR("Error LAG id 0x%"PRIx64" in use", lag_id);
        sai_lag_unlock();
        return SAI_STATUS_OBJECT_IN_USE;
    }

    ret_val = sai_lag_node_remove(lag_id);
    if(ret_val == SAI_STATUS_SUCCESS) {
        ret_val = sai_lag_npu_api_get()->lag_remove(lag_id);
        if(ret_val != SAI_STATUS_SUCCESS) {
            SAI_LAG_LOG_ERR("Unable to remove LAG id:0x%"PRIx64"", lag_id);
            sai_lag_node_add(lag_id);
        }
    }
    sai_lag_unlock();

    if(ret_val == SAI_STATUS_SUCCESS) {
        sai_lag_notify_modules (lag_id, SAI_LAG_OPER_DELETE, NULL);
    }
    return ret_val;
}

static sai_status_t sai_l2_set_lag_attribute(sai_object_id_t lag_id,
                                             const sai_attribute_t *attr)
{
    sai_status_t ret_val;

    sai_lag_lock();
    ret_val = sai_lag_apply_attribute(lag_id, attr);
    sai_lag_unlock();

    return ret_val;
}

static sai_status_t sai_l2_get_lag_attribute(sai_object_id_t lag_id, uint32_t attr_count,
                                             sai_attribute_t *attr_list)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;
    unsigned int attr_idx = 0;
    sai_lag_node_t *lag_info = NULL;

    STD_ASSERT(attr_list != NULL);

    sai_lag_lock ();
    lag_info = sai_lag_node_get(lag_id);
    if(lag_info == NULL) {
        SAI_LAG_LOG_WARN("lag id not found 0x%"PRIx64"",lag_id);
        sai_lag_unlock();
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++) {
        ret_val = sai_lag_node_get_attr(lag_info, &(attr_list[attr_idx]));
        if(ret_val != SAI_STATUS_SUCCESS){
            ret_val = sai_get_indexed_ret_val(ret_val, attr_idx);
            SAI_LAG_LOG_ERR("Error in setting the lag 0x%"PRIx64" attributes RC = %d",
                            lag_id, ret_val);
            break;
        }
    }

    sai_lag_unlock();
    return ret_val;
}

static sai_status_t sai_l2_lag_port_flag_set (sai_object_id_t lag,
                                              sai_object_id_t port_id,
                                              bool            is_ingress,
                                              bool            value)
{
    sai_status_t rc;
    bool         old_value;

    rc = sai_lag_member_get_disable_status (lag, port_id,
                                            is_ingress, &old_value);

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }

    if (old_value == value) {
        return SAI_STATUS_SUCCESS;
    }

    rc = sai_lag_npu_api_get()->lag_port_flag_set (lag, port_id,
                                                   is_ingress, value);

    if (rc == SAI_STATUS_SUCCESS) {
        sai_lag_member_set_disable_status (lag, port_id, is_ingress, value);
    }
    else {
        sai_lag_npu_api_get()->lag_port_flag_set(lag, port_id,
                                                 is_ingress, old_value);
    }

    return rc;
}

static sai_status_t sai_l2_validate_lag_port_add (sai_object_id_t lag_id,
                                                  sai_object_id_t port_id)
{
    sai_port_fwd_mode_t fwd_mode = SAI_PORT_FWD_MODE_UNKNOWN;
    sai_object_id_t     bridge_port_id = SAI_NULL_OBJECT_ID;
    sai_status_t        sai_rc = SAI_STATUS_FAILURE;

    if (!sai_is_port_valid (port_id)) {
        SAI_LAG_LOG_ERR("Invalid port");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!sai_is_lag_created (lag_id)) {
        SAI_LAG_LOG_WARN ("lag id not found 0x%"PRIx64"", lag_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (sai_is_port_lag_member (lag_id, port_id)) {
        SAI_LAG_LOG_WARN ("Member Port 0x%"PRIx64" already a member "
                          "of lag 0x%"PRIx64"", port_id, lag_id);
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    if (sai_is_port_part_of_different_lag (lag_id, port_id)) {
        SAI_LAG_LOG_WARN ("Invalid port 0x%"PRIx64" to lag 0x%"PRIx64"",
                          port_id, lag_id);
        return SAI_STATUS_INVALID_PORT_MEMBER;
    }

    sai_rc = sai_port_forwarding_mode_get (port_id, &fwd_mode);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_LAG_LOG_ERR("Error %d in obtaining forwarding mode for port 0x%"PRIx64"",
                       sai_rc, port_id);
        return sai_rc;
    }

    if (fwd_mode == SAI_PORT_FWD_MODE_ROUTING) {
        SAI_LAG_LOG_ERR("Port 0x%"PRIx64" is in routing mode.", port_id);
        return sai_rc;
    }

    sai_rc = sai_port_def_bridge_port_get(port_id, &bridge_port_id);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_LAG_LOG_ERR("Error %d in getting bridge port for port 0x%"PRIx64"", sai_rc, port_id);
        return sai_rc;
    }

    if(bridge_port_id != SAI_NULL_OBJECT_ID) {
        SAI_LAG_LOG_ERR("Cannot add port to lag. Port 0x%"PRIx64" has bridge port 0x%"PRIx64"",
                        port_id, bridge_port_id);
        return SAI_STATUS_OBJECT_IN_USE;
    }
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_l2_create_lag_member (sai_object_id_t *out_member_id,
                                              sai_object_id_t  switch_id,
                                              uint32_t         attr_count,
                                              const sai_attribute_t *attr_list)
{
    sai_object_id_t   lag = SAI_NULL_OBJECT_ID;
    sai_object_id_t   port_id = SAI_NULL_OBJECT_ID;
    sai_object_list_t port_list;
    bool              lag_attr_present = false;
    bool              port_attr_present = false;
    bool              ing_disable_attr_present = false;
    bool              egr_disable_attr_present = false;
    bool              ing_disable = false;
    bool              egr_disable = false;
    bool              lag_port_added = false;
    sai_status_t      ret_val = SAI_STATUS_SUCCESS;
    uint_t            attr_idx;

    STD_ASSERT (out_member_id != NULL);

    if (attr_count > 0) {
        STD_ASSERT ((attr_list != NULL));
    }

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {

        switch (attr_list [attr_idx].id) {

            case SAI_LAG_MEMBER_ATTR_LAG_ID:
                if (lag_attr_present) {

                    if (lag != attr_list [attr_idx].value.oid) {

                        ret_val = SAI_STATUS_INVALID_PARAMETER;
                        break;
                    }
                }
                else {
                    lag = attr_list [attr_idx].value.oid;
                    lag_attr_present = true;
                }
                break;

            case SAI_LAG_MEMBER_ATTR_PORT_ID:
                if (port_attr_present) {

                    if (port_id != attr_list [attr_idx].value.oid) {

                        ret_val = SAI_STATUS_INVALID_PARAMETER;
                        break;
                    }
                }
                else {
                    port_id = attr_list [attr_idx].value.oid;
                    port_attr_present = true;
                }
                break;

            case SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE:
                egr_disable_attr_present = true;
                egr_disable = attr_list [attr_idx].value.booldata;
                break;

            case SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE:
                ing_disable_attr_present = true;
                ing_disable = attr_list [attr_idx].value.booldata;
                break;

            default:
                return sai_get_indexed_ret_val(SAI_STATUS_UNKNOWN_ATTRIBUTE_0, attr_idx);;
        }
    }

    if ((!lag_attr_present) || (!port_attr_present)) {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    sai_lag_lock();
    sai_port_lock();

    do {
        ret_val = sai_l2_validate_lag_port_add (lag, port_id);

        if (ret_val == SAI_STATUS_SUCCESS) {

            ret_val = sai_l2_add_lag_port (lag, port_id, out_member_id);

            if (ret_val != SAI_STATUS_SUCCESS) {
                SAI_LAG_LOG_ERR ("Add port 0x%"PRIx64" to LAG 0x%"PRIx64" failed with err :%d",
                                  port_id, lag, ret_val);
                break;
            }
        }
        else {
            break;
        }

        lag_port_added = true;

        if (ing_disable_attr_present) {
            ret_val
                = sai_l2_lag_port_flag_set (lag, port_id, true, ing_disable);

            if (ret_val != SAI_STATUS_SUCCESS) {
                SAI_LAG_LOG_ERR ("Ingress disable set in LAG 0x%"PRIx64" port 0x%"PRIx64" failed with err :%d",
                                 lag, port_id, ret_val);
                break;
            }
        }

        if (egr_disable_attr_present) {
            ret_val
                = sai_l2_lag_port_flag_set (lag, port_id, false, egr_disable);

            if (ret_val != SAI_STATUS_SUCCESS) {
                SAI_LAG_LOG_ERR ("Egress disable set in LAG 0x%"PRIx64" port 0x%"PRIx64" failed with err :%d",
                                 lag, port_id, ret_val);
                break;
            }
        }

        port_list.count = 1;
        port_list.list  = &port_id;

        sai_port_lag_set(port_id, lag);
        sai_port_increment_ref_count(port_id);

    } while (0);

    if ((ret_val != SAI_STATUS_SUCCESS) && (lag_port_added)) {
        sai_l2_remove_lag_port (lag, port_id);
    }

    sai_port_unlock();
    sai_lag_unlock ();

    if (ret_val == SAI_STATUS_SUCCESS) {
        sai_lag_notify_modules (lag, SAI_LAG_OPER_ADD_PORTS, &port_list);
    }
    return ret_val;
}

static sai_status_t sai_l2_remove_lag_member (sai_object_id_t member_id)
{
    sai_object_list_t port_list;
    sai_object_id_t   lag_id;
    sai_object_id_t   port_id;
    sai_status_t      rc;

    sai_lag_lock();
    sai_port_lock();

    do {

        rc = sai_lag_get_info_from_member_id (member_id, &lag_id, &port_id);

        if (rc != SAI_STATUS_SUCCESS) {
            break;
        }

        if (!sai_is_port_valid (port_id)) {
            SAI_LAG_LOG_ERR("Invalid port");
            rc = SAI_STATUS_INVALID_PARAMETER;
            break;
        }

        if (!sai_is_lag_created (lag_id)) {
            SAI_LAG_LOG_WARN ("lag id not found 0x%"PRIx64"", lag_id);
            rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        if (!sai_is_port_lag_member (lag_id, port_id)) {
            SAI_LAG_LOG_WARN ("Port 0x%"PRIx64" not a member "
                              "of lag 0x%"PRIx64"", port_id, lag_id);
            rc = SAI_STATUS_ITEM_NOT_FOUND;
            break;
        }

        sai_l2_remove_lag_port (lag_id, port_id);

        sai_port_lag_set(port_id, SAI_NULL_OBJECT_ID);
        sai_port_decrement_ref_count(port_id);

        port_list.count = 1;
        port_list.list  = &port_id;


    } while (0);

    sai_port_unlock();
    sai_lag_unlock ();

    if(rc == SAI_STATUS_SUCCESS) {
        sai_lag_notify_modules (lag_id,
                                SAI_LAG_OPER_DEL_PORTS, &port_list);
    }

    return rc;
}

static sai_status_t sai_l2_set_lag_member_attribute(sai_object_id_t  member_id,
                                                    const sai_attribute_t *attr)
{
    sai_object_id_t lag_id;
    sai_object_id_t port_id;
    sai_status_t    rc;

    STD_ASSERT ((attr != NULL));

    sai_lag_lock();
    rc = sai_lag_get_info_from_member_id (member_id, &lag_id, &port_id);
    sai_lag_unlock ();

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }

    sai_lag_lock();

    switch (attr->id) {

        case SAI_LAG_MEMBER_ATTR_LAG_ID:
        case SAI_LAG_MEMBER_ATTR_PORT_ID:
            rc = SAI_STATUS_INVALID_ATTRIBUTE_0;
            break;

        case SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE:
            rc = sai_l2_lag_port_flag_set (lag_id, port_id,
                                           false, attr->value.booldata);
            break;

        case SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE:
            rc = sai_l2_lag_port_flag_set (lag_id, port_id,
                                           true, attr->value.booldata);
            break;

        default:
            SAI_LAG_LOG_TRACE ("Unknown attr :%d for lag member id 0x%"PRIx64"",
                                attr->id, member_id);
            rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            break;
    }

    sai_lag_unlock ();

    return rc;
}

static sai_status_t sai_l2_get_lag_member_attribute(sai_object_id_t  member_id,
                                                    uint32_t         attr_count,
                                                    sai_attribute_t *attr_list)
{
    sai_object_id_t lag_id;
    sai_object_id_t port_id;
    sai_status_t    rc = SAI_STATUS_SUCCESS;
    uint_t          attr_idx;

    if (attr_count > 0) {
        STD_ASSERT ((attr_list != NULL));
    }

    sai_lag_lock ();
    rc = sai_lag_get_info_from_member_id (member_id, &lag_id, &port_id);
    sai_lag_unlock ();

    if (rc != SAI_STATUS_SUCCESS) {
        return rc;
    }

    sai_lag_lock ();

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++) {

        rc = SAI_STATUS_SUCCESS;

        switch (attr_list [attr_idx].id) {

            case SAI_LAG_MEMBER_ATTR_LAG_ID:
                attr_list [attr_idx].value.oid = lag_id;
                break;

            case SAI_LAG_MEMBER_ATTR_PORT_ID:
                attr_list [attr_idx].value.oid = port_id;
                break;

            case SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE:
                rc = sai_lag_member_get_disable_status (lag_id, port_id,
                                                        false,
                                                        &attr_list [attr_idx]
                                                        .value.booldata);
                break;

            case SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE:
                rc = sai_lag_member_get_disable_status (lag_id, port_id,
                                                        true,
                                                        &attr_list [attr_idx]
                                                        .value.booldata);
                break;

            default:
                rc = SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                break;
        }

        if (rc != SAI_STATUS_SUCCESS) {
            rc = sai_get_indexed_ret_val(rc, attr_idx);
            break;
        }
    }

    sai_lag_unlock ();

    return rc;
}
static sai_status_t sai_l2_bulk_lag_member_create(sai_object_id_t switch_id,
                                                  uint32_t object_count,
                                                  const uint32_t *attr_count,
                                                  const sai_attribute_t **attrs,
                                                  sai_bulk_op_error_mode_t type,
                                                  sai_object_id_t *object_id,
                                                  sai_status_t *object_statuses)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

static sai_status_t sai_l2_bulk_lag_member_remove(uint32_t object_count,
                                                  const sai_object_id_t *object_id,
                                                  sai_bulk_op_error_mode_t type,
                                                  sai_status_t *object_statuses)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

static sai_lag_api_t sai_lag_method_table =
{
    sai_l2_create_lag,
    sai_l2_remove_lag,
    sai_l2_set_lag_attribute,
    sai_l2_get_lag_attribute,
    sai_l2_create_lag_member,
    sai_l2_remove_lag_member,
    sai_l2_set_lag_member_attribute,
    sai_l2_get_lag_member_attribute,
    sai_l2_bulk_lag_member_create,
    sai_l2_bulk_lag_member_remove
};

sai_lag_api_t  *sai_lag_api_query (void)
{
    return (&sai_lag_method_table);
}
