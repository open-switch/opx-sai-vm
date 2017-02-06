/*
 * Copyright (c) 2016 Dell Inc.
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
 * @file sai_vm_switch.c
 *
 * @brief This file contains function implementations of NPU handlers
 *        for SAI SWITCH object in VM environment.
 */

#include "sai_vm_defs.h"
#include "sai_vm_db_utils.h"
#include "sai_vm_npu_sim_info.h"
#include "sai_switch_db_api.h"
#include "sai_vm_port_util.h"
#include "saiswitch.h"
#include "sai_npu_switch.h"
#include "sai_switch_common.h"
#include "sai_switch_utils.h"
#include "sai_port_utils.h"
#include "sai_l3_util.h"
#include "sai_acl_type_defs.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_bit_masks.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include <stdlib.h>
#include <stddef.h>

static sai_vm_tcb_t g_sai_vm_tcb;

sai_vm_tcb_t *sai_vm_access_tcb (void)
{
    return (&g_sai_vm_tcb);
}

static void sai_vm_free_table_resources (void)
{
    uint_t table_id = 0;

    if (g_sai_vm_tcb.vrf_id_bitmap) {
        free (g_sai_vm_tcb.vrf_id_bitmap);
    }

    if (g_sai_vm_tcb.nh_id_bitmap) {
        free (g_sai_vm_tcb.nh_id_bitmap);
    }

    if (g_sai_vm_tcb.nh_grp_id_bitmap) {
        free (g_sai_vm_tcb.nh_grp_id_bitmap);
    }

    for (table_id = 0; table_id < SAI_ACL_TABLE_ID_MAX; table_id++) {
        if (g_sai_vm_tcb.acl_cntr_id_bitmap [table_id]) {
            free (g_sai_vm_tcb.acl_cntr_id_bitmap [table_id]);
        }
    }
}

static sai_status_t sai_vm_tables_init (void)
{
    sai_status_t ret_code = SAI_STATUS_SUCCESS;
    uint_t       table_id = 0;

    do {
        g_sai_vm_tcb.vrf_id_bitmap =
            std_bitmap_create_array (SAI_VM_MAX_VRF);

        if (!g_sai_vm_tcb.vrf_id_bitmap) {
            SAI_SWITCH_LOG_ERR ("Failed to create VRF bitmap array of "
                                "size %d.", SAI_VM_MAX_VRF);

            ret_code = SAI_STATUS_NO_MEMORY;
            break;
        }

        g_sai_vm_tcb.nh_id_bitmap =
            std_bitmap_create_array (SAI_VM_NH_TABLE_SIZE);

        if (!g_sai_vm_tcb.nh_id_bitmap) {
            SAI_SWITCH_LOG_ERR ("Failed to create Next-hop ID bitmap array "
                                "of size %d.", SAI_VM_NH_TABLE_SIZE);

            ret_code = SAI_STATUS_NO_MEMORY;
            break;
        }

        g_sai_vm_tcb.nh_grp_id_bitmap =
            std_bitmap_create_array (SAI_VM_NH_GRP_TABLE_SIZE);

        if (!g_sai_vm_tcb.nh_grp_id_bitmap) {
            SAI_SWITCH_LOG_ERR ("Failed to create Next-hop Group ID bitmap "
                                "array of size %d.",
                                SAI_VM_NH_GRP_TABLE_SIZE);

            ret_code = SAI_STATUS_NO_MEMORY;
            break;
        }

        for (table_id = 0; table_id < SAI_ACL_TABLE_ID_MAX; table_id++) {
            g_sai_vm_tcb.acl_cntr_id_bitmap [table_id] =
                std_bitmap_create_array (SAI_VM_ACL_TABLE_MAX_COUNTERS);

            if (!(g_sai_vm_tcb.acl_cntr_id_bitmap [table_id])) {
                SAI_SWITCH_LOG_ERR ("Failed to create ACL counter ID bitmap "
                                    "array of size %d for TABLE ID %d.",
                                    SAI_VM_ACL_TABLE_MAX_COUNTERS, table_id);

                ret_code = SAI_STATUS_NO_MEMORY;
                break;
            }
        }
    } while (0);

    if (ret_code != SAI_STATUS_SUCCESS) {
        sai_vm_free_table_resources ();
    }

    return ret_code;
}

static sai_status_t sai_npu_switch_init (sai_switch_id_t switch_id)
{
    sai_status_t ret_code = SAI_STATUS_UNINITIALIZED;
    sai_attribute_t attr;

    SAI_SWITCH_LOG_TRACE ("VM specific Switch and Port initializations.");

    do {
        if ((ret_code = sai_vm_db_init ()) != SAI_STATUS_SUCCESS) {
            SAI_SWITCH_LOG_CRIT ("SAI VM DB init failed with err: %d",
                                 ret_code);
            break;
        }

        if ((ret_code = sai_vm_tables_init ()) != SAI_STATUS_SUCCESS) {
            SAI_SWITCH_LOG_CRIT ("SAI VM Table init failed with err: %d",
                                 ret_code);

            break;
        }

        if ((ret_code = sai_vm_switch_db_init (switch_id)) !=
            SAI_STATUS_SUCCESS) {
            SAI_SWITCH_LOG_CRIT ("SAI VM Switch DB init failed with err: %d "
                                 "for switch ID: %d.", ret_code, switch_id);

            break;
        }
    } while (0);

    /* Update the Switch DB entry with this attribute info. */
    attr.id = SAI_SWITCH_ATTR_OPER_STATUS;

    if (ret_code != SAI_STATUS_SUCCESS) {
        attr.value.s32 = SAI_SWITCH_OPER_STATUS_FAILED;

        SAI_SWITCH_LOG_CRIT ("SAI VM switch Init failed with err %d", ret_code);
    } else {
        attr.value.s32 = SAI_SWITCH_OPER_STATUS_UP;

        SAI_SWITCH_LOG_NTC ("SAI VM Switch is UP and Init Successful");

        sai_vm_switch_id_set (switch_id);
    }

    ret_code = sai_switch_attribute_set_db_entry (switch_id, &attr);

    if (ret_code != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_ERR ("Failed to update the Switch attribute %d in "
                            "DB table.", attr.id);
    }

    return ret_code;
}

static sai_status_t sai_npu_switching_mode_set (const sai_switch_switching_mode_t mode)
{
    sai_status_t ret = SAI_STATUS_ATTR_NOT_SUPPORTED_0;
    sai_attribute_t attr;

    SAI_SWITCH_LOG_TRACE ("Set: Packet switching Mode is %d", mode);

    sai_switch_lock ();

    do {
        if (mode == SAI_SWITCHING_MODE_CUT_THROUGH) {
            if (sai_switch_cut_through_supported ()) {
                sai_switch_capablility_enable
                    (false, SAI_SWITCH_CAP_STORE_AND_FORWARD_MODE);

                sai_switch_capablility_enable
                    (true, SAI_SWITCH_CAP_CUT_THROUGH_MODE);

                ret = SAI_STATUS_SUCCESS;
            } else {
                SAI_SWITCH_LOG_ERR ("Cut through mode not supported");

                ret = SAI_STATUS_NOT_SUPPORTED;
                break;
            }
        } else if (mode == SAI_SWITCHING_MODE_STORE_AND_FORWARD) {
            if (sai_switch_store_and_forward_supported()) {
                sai_switch_capablility_enable
                    (false, SAI_SWITCH_CAP_CUT_THROUGH_MODE);

                sai_switch_capablility_enable
                    (true, SAI_SWITCH_CAP_STORE_AND_FORWARD_MODE);

                ret = SAI_STATUS_SUCCESS;
            } else {
                SAI_SWITCH_LOG_ERR ("Store and forward mode not supported");

                ret = SAI_STATUS_NOT_SUPPORTED;
                break;
            }
        } else {
            SAI_SWITCH_LOG_ERR ("Mode %d is not a valid mode");

            ret = SAI_STATUS_INVALID_ATTR_VALUE_0;
            break;
        }
    } while (0);

    sai_switch_unlock ();

    if (ret == SAI_STATUS_SUCCESS) {
        /* Update the Switch DB entry with this attribute info. */
        attr.id = SAI_SWITCH_ATTR_SWITCHING_MODE;
        attr.value.s32 = mode;

        ret = sai_switch_attribute_set_db_entry (sai_vm_switch_id_get(), &attr);

        if (ret != SAI_STATUS_SUCCESS) {
            SAI_SWITCH_LOG_ERR ("Failed to update the Switch attribute %d in "
                                "DB table.", attr.id);
        }
    }

    return ret;
}

static sai_status_t sai_npu_ttl1_violation_action_set (sai_packet_action_t action)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_ttl1_violation_action_get (sai_packet_action_t *action)
{
    STD_ASSERT(action != NULL);

    /* TODO - Cache and get. */

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_switch_temp_get (sai_attribute_value_t *value)
{
    STD_ASSERT(value != NULL);

    /* TODO - Temp should be configurable through a database */
    value->s32 = SAI_VM_SWITCH_TEMP;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_switch_counter_refresh_interval_set(uint_t cntr_interval)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    sai_attribute_t attr;

    SAI_SWITCH_LOG_TRACE ("Switch counter refresh interval set to %d.",
                          cntr_interval);

    /* Update the Switch DB entry with this attribute info. */
    attr.id = SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL;
    attr.value.u32 = cntr_interval;

    sai_rc = sai_switch_attribute_set_db_entry (sai_vm_switch_id_get(), &attr);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_ERR ("Failed to update the switch attribute %d in "
                            "DB table.", attr.id);
    }

    return sai_rc;
}

static sai_status_t sai_npu_ecmp_hash_algorithm_set (sai_hash_algorithm_t ecmp_algo)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_lag_hash_algorithm_set (sai_hash_algorithm_t lag_algo)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_ecmp_hash_seed_value_set (uint32_t ecmp_hash_seed_value)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_lag_hash_seed_value_set (uint32_t lag_hash_seed_value)
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_switch_hash_field_set (sai_attr_id_t attr_id,
                                        const sai_s32_list_t *native_field_list,
                                        const sai_object_list_t *udf_group_list)
{
    STD_ASSERT(native_field_list != NULL);
    STD_ASSERT(udf_group_list != NULL);

    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_ecmp_hash_algorithm_get (sai_hash_algorithm_t *ecmp_algo)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get ();

    STD_ASSERT(sai_switch_info_ptr != NULL);
    STD_ASSERT(ecmp_algo != NULL);

    *ecmp_algo = sai_switch_info_ptr->ecmp_hash_algo;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_lag_hash_algorithm_get (sai_hash_algorithm_t *lag_algo)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();

    STD_ASSERT(sai_switch_info_ptr != NULL);
    STD_ASSERT(lag_algo != NULL);

    *lag_algo = sai_switch_info_ptr->lag_hash_algo;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_ecmp_hash_seed_value_get (sai_switch_hash_seed_t *ecmp_hash_seed_value)
{
    return (sai_switch_ecmp_hash_seed_value_get (ecmp_hash_seed_value));
}

static sai_status_t sai_npu_lag_hash_seed_value_get (sai_switch_hash_seed_t *lag_hash_seed_value)
{
    return (sai_switch_lag_hash_seed_value_get (lag_hash_seed_value));
}

static sai_status_t sai_npu_lag_max_number_get(uint32_t *max_number)
{
    if(max_number == NULL) {
        SAI_SWITCH_LOG_ERR("Error invalid paramter passed");
        STD_ASSERT(0);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *max_number = SAI_VM_SWITCH_MAX_LAG_NUMBER;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_npu_lag_max_members_get(uint32_t *max_members)
{
    if(max_members == NULL) {
        SAI_SWITCH_LOG_ERR("Error invalid paramter passed");
        STD_ASSERT(0);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *max_members = SAI_VM_SWITCH_MAX_LAG_MEMBERS;
    return SAI_STATUS_SUCCESS;
}
static sai_npu_switch_api_t sai_vm_switch_api_table = {
    sai_npu_switch_init,
    sai_npu_switch_init_config,
    sai_npu_switching_mode_set,
    sai_npu_switching_mode_get,
    sai_npu_switch_temp_get,
    sai_npu_ttl1_violation_action_set,
    sai_npu_ttl1_violation_action_get,
    sai_npu_switch_counter_refresh_interval_set,
    sai_npu_ecmp_hash_algorithm_set,
    sai_npu_ecmp_hash_seed_value_set,
    sai_npu_ecmp_hash_algorithm_get,
    sai_npu_ecmp_hash_seed_value_get,
    sai_npu_lag_hash_algorithm_set,
    sai_npu_lag_hash_seed_value_set,
    sai_npu_lag_hash_algorithm_get,
    sai_npu_lag_hash_seed_value_get,
    sai_npu_lag_max_members_get,
    sai_npu_lag_max_number_get,
    sai_npu_switch_hash_field_set,
};

sai_status_t sai_npu_switching_mode_get (sai_switch_switching_mode_t *mode)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    STD_ASSERT (mode != NULL);

    sai_switch_lock ();

    if (sai_switch_cut_through_enabled ())
    {
        *mode = SAI_SWITCHING_MODE_CUT_THROUGH;
    }
    else if (sai_switch_store_and_forward_enabled ())
    {
        *mode = SAI_SWITCHING_MODE_STORE_AND_FORWARD;
    }

    sai_switch_unlock ();

    SAI_SWITCH_LOG_TRACE ("Get: Packet switching Mode is %d", *mode);

    return ret;
}

sai_npu_switch_api_t* sai_vm_switch_api_query (void)
{
    return &sai_vm_switch_api_table;
}
