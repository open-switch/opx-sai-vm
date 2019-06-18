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

/*!
 * \file   sai_func_query.c
 * \brief  SAI Infra and Query functionality
 * \date   12-2014
 * */
#include "sai.h"
#include "sai_common_infra.h"
#include "sai_infra_api.h"
#include "sai_switch_utils.h"
#include "sai_npu_hostif.h"
#include <stdio.h>
#include "sai_oid_utils.h"
#include "sai_npu_api_plugin.h"
#include "std_assert.h"
#include <dlfcn.h>

static sai_service_method_table_t sai_service_method_table;
static sai_npu_api_t *sai_npu_api_method_table = NULL;
static void *npu_api_lib_handle = NULL;

sai_status_t sai_api_initialize(uint64_t flags, const sai_service_method_table_t* services)
{

    SAI_SWITCH_LOG_TRACE("SAI Initialization with service method table");

    if(services != NULL)
    {
        sai_switch_lock();

        sai_service_method_table.profile_get_value = services->profile_get_value;

        sai_service_method_table.profile_get_next_value = services->profile_get_next_value;

        sai_switch_unlock();
    }

    return SAI_STATUS_SUCCESS;

}

const sai_service_method_table_t *sai_service_method_table_get(void)
{
    return &sai_service_method_table;
}

sai_status_t sai_api_query(sai_api_t sai_api_id, void** api_method_table)
{

    SAI_SWITCH_LOG_TRACE("Method table for api-id %d",sai_api_id);

    switch (sai_api_id)
    {
        case SAI_API_SWITCH:
            *api_method_table = sai_switch_api_query();
            break;

        case SAI_API_PORT:
            *api_method_table = sai_port_api_query();
            break;

        case SAI_API_FDB:
            *api_method_table = sai_fdb_api_query();
            break;

        case SAI_API_VLAN:
            *api_method_table = sai_vlan_api_query();
            break;

        case SAI_API_VIRTUAL_ROUTER:
            *api_method_table = sai_vr_api_query();
            break;

        case SAI_API_ROUTE:
            *api_method_table = sai_route_api_query();
            break;

        case SAI_API_NEXT_HOP:
            *api_method_table = sai_nexthop_api_query();
            break;

        case SAI_API_NEXT_HOP_GROUP:
            *api_method_table = sai_nexthop_group_api_query();
            break;

        case SAI_API_ROUTER_INTERFACE:
            *api_method_table = sai_router_intf_api_query();
            break;

        case SAI_API_NEIGHBOR:
            *api_method_table = sai_neighbor_api_query();
            break;

        case SAI_API_ACL:
            *api_method_table = sai_acl_api_query();
            break;

        case SAI_API_MIRROR:
            *api_method_table = sai_mirror_api_query();
            break;

        case SAI_API_HOSTIF:
            *api_method_table = sai_hostif_api_query();
            break;

        case SAI_API_STP:
            *api_method_table = sai_stp_api_query();
            break;

        case SAI_API_LAG:
            *api_method_table = sai_lag_api_query();
            break;

        case SAI_API_QOS_MAP:
            *api_method_table = sai_qosmaps_api_query();
            break;

        case SAI_API_SAMPLEPACKET:
            *api_method_table = sai_samplepacket_api_query();
            break;

        case SAI_API_POLICER:
            *api_method_table = sai_policer_api_query();
            break;

        case SAI_API_QUEUE:
            *api_method_table = sai_qos_queue_api_query();
            break;

        case SAI_API_SCHEDULER_GROUP:
            *api_method_table = sai_qos_sched_group_api_query();
            break;

        case SAI_API_SCHEDULER:
            *api_method_table = sai_qos_scheduler_api_query();
            break;

        case SAI_API_WRED:
            *api_method_table = sai_wred_api_query();
            break;

        case SAI_API_BUFFER:
            *api_method_table = sai_qos_buffer_api_query();
             break;

        case SAI_API_UDF:
            *api_method_table = sai_udf_api_query();
            break;

        case SAI_API_HASH:
            *api_method_table = sai_hash_api_query();
            break;

        case SAI_API_TUNNEL:
            *api_method_table = sai_tunnel_api_query();
            break;

        case SAI_API_BRIDGE:
            *api_method_table = sai_bridge_api_query();
            break;

        case SAI_API_L2MC:
            *api_method_table = sai_l2mc_entry_api_query();
            break;

        case SAI_API_L2MC_GROUP:
            *api_method_table = sai_l2mc_group_api_query();
            break;

        case SAI_API_IPMC:
            *api_method_table = sai_ipmc_entry_api_query();
            break;

        case SAI_API_IPMC_GROUP:
            *api_method_table = sai_ipmc_group_api_query();
            break;

        case SAI_API_RPF_GROUP:
            *api_method_table = sai_ipmc_rpf_group_api_query();
            break;

        case SAI_API_UNSPECIFIED:
        default:
            if (sai_api_id >= SAI_API_CUSTOM_RANGE_START) {
                return (sai_extn_module_api_query (sai_api_id, api_method_table));
            }
            SAI_SWITCH_LOG_TRACE("Method table for api-id %d",sai_api_id);
            return SAI_STATUS_NOT_SUPPORTED;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_api_uninitialize(void)
{
    SAI_SWITCH_LOG_TRACE("Uninitialize the SAI functionality");
    return SAI_STATUS_SUCCESS;
}

sai_object_type_t sai_object_type_query(sai_object_id_t object_id)
{
    return (sai_uoid_obj_type_get (object_id));
}

sai_status_t sai_npu_api_initialize (const char *lib_name)
{
    sai_npu_api_t* (*query_fn_ptr) (void);

    //STD_ASSERT(lib_name != NULL);
    SAI_SWITCH_LOG_TRACE ("SAI NPU API method table Initialization.");

    npu_api_lib_handle = dlopen (lib_name, RTLD_LAZY);

    if (npu_api_lib_handle == NULL) {

        SAI_SWITCH_LOG_ERR ("dlopen() failed with Err: %s.", dlerror());

        return SAI_STATUS_FAILURE;
    }

    query_fn_ptr = dlsym (npu_api_lib_handle, "sai_npu_api_query");

    if (query_fn_ptr == NULL) {

        SAI_SWITCH_LOG_ERR ("dlsym() for sai_npu_api_query failed with Err: %s.",
                            dlerror());

    } else {

        sai_npu_api_method_table = (*query_fn_ptr)();
    }

    if (sai_npu_api_method_table == NULL) {

        SAI_SWITCH_LOG_ERR ("SAI NPU API method table is null.");

        return SAI_STATUS_FAILURE;
    }

    SAI_SWITCH_LOG_INFO ("SAI NPU API method table query done.");

    return SAI_STATUS_SUCCESS;
}

sai_npu_api_t* sai_npu_api_table_get (void)
{
    return sai_npu_api_method_table;
}

void sai_npu_api_uninitialize (void)
{
    int rc;

    sai_npu_api_method_table = NULL;

    rc = dlclose (npu_api_lib_handle);

    if (rc != 0) {

        SAI_SWITCH_LOG_ERR ("dlclose() failed with Err: %d.", rc);

    } else {

        SAI_SWITCH_LOG_INFO ("SAI NPU lib dlclose() done.");
    }

    npu_api_lib_handle = NULL;
}

/******************************Logging API Implementations ************************************/

sai_status_t sai_log_set(sai_api_t sai_api_id, sai_log_level_t log_level)
{
    sai_log_level_set (sai_api_id, log_level);

    return SAI_STATUS_SUCCESS;
}
