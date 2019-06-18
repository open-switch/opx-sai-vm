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
* @file sai_port_utils.c
*
* @brief This file contains SAI Common Port Utility API's.
*        Common Port Utility API's can be used by other SAI components.
*
*************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "saiswitch.h"
#include "saitypes.h"
#include "saiport.h"
#include "saistatus.h"

#include "sai_port_common.h"
#include "sai_port_utils.h"
#include "sai_switch_utils.h"

#include "std_rbtree.h"
#include "std_assert.h"
#include "std_mutex_lock.h"
#include "std_struct_utils.h"

static std_mutex_lock_create_static_init_fast(port_lock);

void sai_port_lock(void)
{
    std_mutex_lock(&port_lock);
}

void sai_port_unlock(void)
{
    std_mutex_unlock(&port_lock);
}

/* Allocate memory for switch info table */
sai_status_t sai_port_info_init(void)
{
    sai_switch_info_t *switch_info = sai_switch_info_get();

    SAI_PORT_LOG_TRACE("Port info table initialization");

    switch_info->port_info_table =
        std_rbtree_create_simple("SAI Port info tree",
                                 STD_STR_OFFSET_OF(sai_port_info_t, sai_port_id),
                                 STD_STR_SIZE_OF(sai_port_info_t, sai_port_id));

    /* @todo replace STD_ASSERT with RELEASE_ASSERT */
    STD_ASSERT(!(switch_info->port_info_table == NULL));

    switch_info->port_applications_tree =
        std_rbtree_create_simple ("applications_tree_per_port",
                                  STD_STR_OFFSET_OF(sai_port_application_info_t, port_id),
                                  STD_STR_SIZE_OF(sai_port_application_info_t, port_id));

    /* @todo replace STD_ASSERT with RELEASE_ASSERT */
    STD_ASSERT(!(switch_info->port_applications_tree == NULL));

    return SAI_STATUS_SUCCESS;
}

/* Assumption is port info table will be allocated during Init,
 * and it can be accessed without failure checks
 */
sai_port_info_table_t sai_port_info_table_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return sai_switch_info_ptr->port_info_table;
}

sai_port_info_t *sai_port_info_get(sai_object_id_t port)
{
    sai_port_info_table_t port_info_table = sai_port_info_table_get();
    sai_port_info_t port_info_t;

    memset(&port_info_t, 0, sizeof(sai_port_info_t));
    port_info_t.sai_port_id = port;

    /* Port validation check is expected to be done before calling this */
    return ((sai_port_info_t *) std_rbtree_getexact(port_info_table, &port_info_t));
}

static inline bool sai_is_logical_port_valid(sai_object_id_t port)
{
    sai_port_info_t *port_info_table = sai_port_info_get(port);

    if(port_info_table == NULL) {
        SAI_PORT_LOG_TRACE("Port info is %p in logical port valid for port 0x%"PRIx64"",
                           port_info_table,port);
        return false;
    }

    return (port_info_table->port_valid);
}

bool sai_is_port_valid(sai_object_id_t port)
{
    if(!(sai_is_obj_id_cpu_port(port) || sai_is_obj_id_logical_port(port))) {
        return false;
    }

    /* CPU port is a valid sai port as well.
     * All public common sai port api's can pass cpu port as
     * a input for applicable set/get api's */

    if(port == sai_switch_cpu_port_obj_id_get()) {
        return true;
    }

    return sai_is_logical_port_valid(port);
}

sai_status_t sai_port_port_group_get(sai_object_id_t port, uint_t *port_group)
{
    if(port_group == NULL) {
        SAI_PORT_LOG_TRACE("Port group is %p in port group get",port_group);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_PORT_NUMBER;
    }

    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    *port_group = port_info_table->port_group;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_to_npu_local_port(sai_object_id_t port,
                                        sai_npu_port_id_t *local_port_id)
{
    if(local_port_id == NULL) {
        SAI_PORT_LOG_TRACE("sai port to npu local port local_port_id is %p for port 0x%"PRIx64"",
                           local_port_id, port);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(!(sai_is_obj_id_cpu_port(port) || sai_is_obj_id_logical_port(port))) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if(port == sai_switch_cpu_port_obj_id_get()) {
        *local_port_id = sai_switch_get_cpu_port();
        return SAI_STATUS_SUCCESS;
    }

    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    *local_port_id = port_info_table->local_port_id;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_local_port_to_sai_port(sai_npu_port_id_t local_port_id,
                                            sai_object_id_t *port)
{
    if(port == NULL) {
        SAI_PORT_LOG_TRACE("Npu local port to sai port Port is NULL local port id is %d",
                           local_port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(local_port_id == sai_switch_get_cpu_port()) {
        *port = sai_switch_cpu_port_obj_id_get();
        return SAI_STATUS_SUCCESS;
    }

    *port = sai_port_id_create(SAI_PORT_TYPE_LOGICAL,
                               sai_switch_id_get(),
                               local_port_id);
    return SAI_STATUS_SUCCESS;

}

sai_port_info_t *sai_port_info_get_from_npu_phy_port(sai_npu_port_id_t phy_port_id)
{
    sai_port_info_t *port_info_table = NULL;

    for (port_info_table = sai_port_info_getfirst(); (port_info_table != NULL);
         port_info_table = sai_port_info_getnext(port_info_table)) {

        if(port_info_table->phy_port_id == phy_port_id) {
            return  port_info_table;
        }
    }
    return NULL;
}

sai_status_t sai_port_attr_supported_speed_update(sai_port_info_t * const sai_port_info,
                                                  uint_t speed_capb)
{
    uint_t lane = 0;
    uint_t max_lanes = 0;
    sai_port_info_t *tmp_port_info = NULL;

    if(sai_port_info == NULL) {
        SAI_PORT_LOG_TRACE("Port info is %p in port attr supported speed update ",sai_port_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    max_lanes = sai_port_info->max_lanes_per_port;
    tmp_port_info = sai_port_info;
    for (lane = 0; lane < max_lanes; lane++) {
        if(sai_port_info != NULL) {
            tmp_port_info->port_speed_capb = speed_capb;
        }
        tmp_port_info = sai_port_info_getnext(tmp_port_info);
    }
    return SAI_STATUS_SUCCESS;
}


static inline void sai_port_capablility_enable_update(sai_port_info_t *sai_port_info,
                                                      bool enable, uint64_t capb_val)
{
    if(sai_port_info == NULL) {
        SAI_PORT_LOG_TRACE("Port info is %p in port capb enable updadte",sai_port_info);
        return;
    }

    /* Enable only the supported capabilities */
    capb_val = capb_val & (sai_port_info->port_supported_capb);

    if(enable) {
        sai_port_info->port_enabled_capb |= capb_val;

    } else {
        sai_port_info->port_enabled_capb &= ~capb_val;
    }
}

sai_status_t sai_port_forwarding_mode_set(sai_object_id_t port,
                                          sai_port_fwd_mode_t fwd_mode)
{
    sai_port_info_t *sai_port_info_ptr = sai_port_info_get(port);
    if(sai_port_info_ptr == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    sai_port_info_ptr->fwd_mode = fwd_mode;

    SAI_PORT_LOG_TRACE("Port forwarding mode set to %d\r\n",fwd_mode);
    return SAI_STATUS_SUCCESS;
}


sai_status_t sai_port_forwarding_mode_get(sai_object_id_t port,
                                          sai_port_fwd_mode_t *fwd_mode)
{
    if(fwd_mode == NULL) {
        SAI_PORT_LOG_TRACE("fwd_mode is %p in forwarding mode get",fwd_mode);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    sai_port_info_t *sai_port_info_ptr = sai_port_info_get(port);
    if(sai_port_info_ptr == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    *fwd_mode = sai_port_info_ptr->fwd_mode;

    SAI_PORT_LOG_TRACE("Port forwarding mode is %d\r\n",*fwd_mode);
    return SAI_STATUS_SUCCESS;
}

const char *sai_port_forwarding_mode_to_str (sai_port_fwd_mode_t fwd_mode)
{
    if (fwd_mode == SAI_PORT_FWD_MODE_UNKNOWN) {
        return "Unknown";

    } else if (fwd_mode == SAI_PORT_FWD_MODE_SWITCHING) {
        return "Switching";

    } else if (fwd_mode == SAI_PORT_FWD_MODE_ROUTING) {
        return "Routing";

    } else {
        return "Invalid";
    }
}

sai_port_application_info_t* sai_port_application_info_create_and_get (sai_object_id_t port_id)
{
    sai_port_application_info_t *p_port_node = NULL;
    sai_port_application_info_t tmp_port_node;
    rbtree_handle               ports_applications_tree = NULL;

    memset (&tmp_port_node, 0, sizeof(sai_port_application_info_t));

    ports_applications_tree = sai_switch_info_get()->port_applications_tree;

    if (ports_applications_tree == NULL) {
        SAI_PORT_LOG_ERR ("Port applications tree is not created, Could be because of"
                          "switch initialization is not been completed");
        return NULL;
    }

    tmp_port_node.port_id = port_id;

    p_port_node = std_rbtree_getexact (ports_applications_tree, (void *) &tmp_port_node);

    if (p_port_node == NULL) {
        p_port_node = (sai_port_application_info_t *)
                        calloc (1, sizeof(sai_port_application_info_t));

        if (p_port_node == NULL) {
            SAI_PORT_LOG_ERR ("Could not allocate memory of size %lu for application specific"
                              "info on port 0x%"PRIx64"", sizeof(sai_port_application_info_t),
                              port_id);
            return NULL;
        }

        p_port_node->port_id = port_id;

        if (std_rbtree_insert(ports_applications_tree, (void *) p_port_node) != STD_ERR_OK) {
            SAI_PORT_LOG_ERR ("Port Node insertion failed for port 0x%"PRIx64"",p_port_node->port_id);
            free ((void *)p_port_node);
            return NULL;
        }

    }

    return p_port_node;
}

sai_status_t sai_port_application_info_remove (sai_port_application_info_t *p_port_node)
{
    rbtree_handle               ports_applications_tree = NULL;

    if(p_port_node == NULL) {
        SAI_PORT_LOG_TRACE("p_port_node is %p in port app info remove",p_port_node);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    ports_applications_tree = sai_switch_info_get()->port_applications_tree;

    if (ports_applications_tree == NULL) {
        SAI_PORT_LOG_ERR ("Port applications tree is not created, Could be because of"
                          "switch initialization is not been completed");
        return SAI_STATUS_FAILURE;
    }

    /* All the applications running on the port should add a check here */

    if (p_port_node->mirror_sessions_tree != NULL) {
        SAI_PORT_LOG_TRACE ("Mirror Applications still running on the port 0x%"PRIx64"",
                            p_port_node->port_id);
        return SAI_STATUS_SUCCESS;
    }

    if (p_port_node->qos_port_db != NULL) {
        SAI_PORT_LOG_TRACE ("Qos Applications still running on the port 0x%"PRIx64"",
                            p_port_node->port_id);
        return SAI_STATUS_SUCCESS;
    }

    if (std_rbtree_remove (ports_applications_tree, (void *)p_port_node) != p_port_node) {
        SAI_PORT_LOG_ERR ("Port Node remove failed for port 0x%"PRIx64"",p_port_node->port_id);
        return SAI_STATUS_FAILURE;
    }

    free ((void *)p_port_node);

    return SAI_STATUS_SUCCESS;
}

sai_port_application_info_t* sai_port_application_info_get (sai_object_id_t port_id)
{
    sai_port_application_info_t *p_port_node = NULL;
    sai_port_application_info_t tmp_port_node;
    rbtree_handle               ports_applications_tree = NULL;

    memset (&tmp_port_node, 0, sizeof(sai_port_application_info_t));

    ports_applications_tree = sai_switch_info_get()->port_applications_tree;

    if ( NULL == ports_applications_tree) {
        SAI_PORT_LOG_ERR ("Port applications tree is not created, Could be because of"
                          "switch initialization is not been completed");
        return NULL;
    }

    tmp_port_node.port_id = port_id;

    p_port_node = std_rbtree_getexact (ports_applications_tree, (void *) &tmp_port_node);

    return p_port_node;
}

sai_port_application_info_t *sai_port_first_application_node_get (void)
{
    rbtree_handle port_applications_tree = sai_switch_info_get()->port_applications_tree;

    if (NULL == port_applications_tree)
        return NULL;

    return ((sai_port_application_info_t *)std_rbtree_getfirst(port_applications_tree));
}

sai_port_application_info_t *sai_port_next_application_node_get (
                                              sai_port_application_info_t *p_port_node)
{
    rbtree_handle port_applications_tree = sai_switch_info_get()->port_applications_tree;

    if(p_port_node == NULL) {
        return NULL;
    }

    if (NULL == port_applications_tree)
        return NULL;

    return ((sai_port_application_info_t *)std_rbtree_getnext(port_applications_tree, p_port_node));
}

/* Get the active breakout mode for a given port */
sai_port_breakout_mode_type_t sai_port_current_breakout_mode_get(const sai_port_info_t *port_info)
{
    if(port_info == NULL) {
        SAI_PORT_LOG_TRACE("port_info is %p in curr breakout mode get",port_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(port_info->port_enabled_capb & SAI_PORT_CAP_BREAKOUT_MODE_2X) {
        return sai_port_break_mode_from_capb(SAI_PORT_CAP_BREAKOUT_MODE_2X);
    } else if(port_info->port_enabled_capb & SAI_PORT_CAP_BREAKOUT_MODE_4X) {
        return sai_port_break_mode_from_capb(SAI_PORT_CAP_BREAKOUT_MODE_4X);
    }
    /* A valid breakout port should be part of one of the possible breakout modes;
     * Default mode is SAI_PORT_CAP_BREAKOUT_MODE_1X */
    return sai_port_break_mode_from_capb(SAI_PORT_CAP_BREAKOUT_MODE_1X);
}

/* Note: port validation is expected to be done before invoking this call */
static sai_status_t sai_port_break_mode_list_get(const sai_port_info_t *sai_port_info,
                                                 sai_s32_list_t *mode_list)
{
    uint32_t count = 0;
    sai_port_capability_t avail_capb[SAI_PORT_MAX_BREAKOUT_MODE_CAP] =
                                                 {SAI_PORT_CAP_BREAKOUT_MODE_1X,
                                                  SAI_PORT_CAP_BREAKOUT_MODE_2X,
                                                  SAI_PORT_CAP_BREAKOUT_MODE_4X };
    uint_t mode_idx = 0;

    if((sai_port_info == NULL) || (mode_list == NULL)) {
        SAI_PORT_LOG_TRACE("sai_port_info is %p mode_list is %p in breakout mode list get",
                           sai_port_info, mode_list);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for(mode_idx = 0; mode_idx < SAI_PORT_MAX_BREAKOUT_MODE_CAP; mode_idx++) {
        if(sai_is_port_capb_supported (sai_port_info, avail_capb[mode_idx])) {
            if(count >= mode_list->count) {
                return SAI_STATUS_BUFFER_OVERFLOW;
            }
            mode_list->list[count] = sai_port_break_mode_from_capb(avail_capb[mode_idx]);
            count++;
        }
    }
    mode_list->count = count;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_attr_supported_breakout_mode_get(sai_object_id_t port_id,
                                                       const sai_port_info_t *sai_port_info,
                                                       sai_attribute_value_t *value)
{
    sai_status_t sai_rc = SAI_STATUS_FAILURE;
    sai_s32_list_t mode_list;
    int32_t breakout_list[SAI_PORT_MAX_BREAKOUT_MODE_CAP] = {0};

    if((sai_port_info == NULL) || (value == NULL)) {
        SAI_PORT_LOG_TRACE("sai_port_info is %p value is %p in supported "
                           "breakout mode get", sai_port_info, value);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    mode_list.list = breakout_list;

    mode_list.count = SAI_PORT_MAX_BREAKOUT_MODE_CAP;

    sai_rc = sai_port_break_mode_list_get (sai_port_info, &mode_list);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("Error in fetching the current number of breakout modes");
        return  sai_rc;
    }

    if(value->s32list.count < mode_list.count) {
        SAI_PORT_LOG_ERR("Get supported breakout mode list count %d is less than "
                "actual mode supported %d in for port 0x%"PRIx64"",
                value->s32list.count, mode_list.count, port_id);

        value->s32list.count = mode_list.count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    memcpy(value->s32list.list, mode_list.list,
            (mode_list.count * sizeof(sai_port_breakout_mode_type_t)));

    /* Update the attr value count with actual breakout modes count */
    value->s32list.count = mode_list.count;

    SAI_PORT_LOG_TRACE("Breakout mode get successful for port 0x%"PRIx64" count %d",
                       port_id, value->s32list.count);

    return SAI_STATUS_SUCCESS;
}

bool sai_port_is_breakout_mode_supported (const sai_port_info_t *sai_port_info,
                                          sai_port_breakout_mode_type_t breakout_mode)
{
    sai_s32_list_t mode_list;
    int32_t breakout_list[SAI_PORT_MAX_BREAKOUT_MODE_CAP] = {0};
    uint_t mode_idx = 0;
    sai_status_t sai_rc = SAI_STATUS_FAILURE;

    if(sai_port_info == NULL) {
        SAI_PORT_LOG_TRACE("sai_port_info is %p in is breakout mode supported", sai_port_info);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    mode_list.count = SAI_PORT_MAX_BREAKOUT_MODE_CAP;
    mode_list.list = breakout_list;

    sai_rc = sai_port_break_mode_list_get (sai_port_info, &mode_list);

    if(sai_rc != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("Error in fetching the current number of breakout modes");
        return false;
    }

    for (mode_idx = 0; mode_idx < mode_list.count; mode_idx++) {
        if(mode_list.list[mode_idx] == breakout_mode) {
            return true;
        }
    }

    return false;
}
static sai_status_t sai_port_hw_lane_list_get(const sai_port_info_t *sai_port_info,
                                              uint32_t lane_count, uint32_t *lane_list)
{
    uint_t           cur_lane = 0;
    uint_t           count = 0;
    uint_t           serdes_port = 0;

    if((sai_port_info == NULL) || (lane_list == NULL)) {
        SAI_PORT_LOG_TRACE("sai_port_info is %p lane_list is %p hw lane list get",
                           sai_port_info, lane_list);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    serdes_port = sai_port_info->phy_port_id;

    lane_list[count] = serdes_port;
    count++;

    for (cur_lane = 1; cur_lane < lane_count; cur_lane++) {
        lane_list[count] = ++serdes_port;
        count++;
    }
    return SAI_STATUS_SUCCESS;
}

/* Get the HW lane list for a given SAI valid Logical port and CPU port is not supported */
sai_status_t sai_port_attr_hw_lane_list_get(sai_object_id_t port_id,
                                            const sai_port_info_t *sai_port_info,
                                            sai_attribute_value_t *value)
{
    uint32_t                      *lane_list = NULL;
    uint_t                         lane_count = 0;
    sai_status_t                   ret_code = SAI_STATUS_FAILURE;
    sai_port_breakout_mode_type_t  brkout_mode;

    if((sai_port_info == NULL) || (value == NULL)) {
        SAI_PORT_LOG_TRACE("sai_port_info is %p value is %p in port attr hw lane list get for "
                           "port 0x%"PRIx64"", sai_port_info, value, port_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    lane_count = sai_port_info->max_lanes_per_port;

    if(lane_count == 0) { /* Not likely */
        return SAI_STATUS_FAILURE;
    }

    if(sai_port_is_breakout_enabled(sai_port_info)) {
       brkout_mode = sai_port_current_breakout_mode_get (sai_port_info);
       lane_count = sai_port_breakout_lane_count_get (brkout_mode);
    }

    if(value->u32list.count < lane_count) {
        SAI_PORT_LOG_ERR("Get hw lane list count %d is less than "
                         "actual lanes count %d in for port 0x%"PRIx64"",
                         value->u32list.count, lane_count, port_id);

        value->u32list.count = lane_count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    lane_list = (uint32_t *) calloc(lane_count, sizeof(uint32_t));
    if(lane_list == NULL) {
        SAI_PORT_LOG_ERR ("Allocation of Memory failed "
                          "for hw port lane list ");
        return SAI_STATUS_NO_MEMORY;
    }

    do {
        ret_code = sai_port_hw_lane_list_get(sai_port_info, lane_count, lane_list);
        if(ret_code != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_ERR("Port hw lane list get port 0x%"PRIx64" ret %d",
                             port_id, ret_code);
            break;
        }

        memcpy(value->u32list.list, lane_list, (lane_count * sizeof(uint32_t)));

        /* Update the attr value count with actual HW lane count */
        value->u32list.count = lane_count;
    } while(0);

    free(lane_list);
    lane_list = NULL;

    SAI_PORT_LOG_TRACE("HW lane list get successful for port 0x%"PRIx64" count %d",
                       port_id, lane_count);

    return ret_code;
}

sai_status_t sai_port_breakout_mode_update (sai_port_info_t * const sai_port_info,
                                            sai_port_breakout_config_t  *breakout_cfg)
{

    uint_t                lane = 0;
    uint_t                max_lanes = 0;
    uint_t                cap_val =0;
    sai_port_media_type_t media_type = SAI_PORT_MEDIA_TYPE_NOT_PRESENT;
    uint_t                prev_cap_val = 0;
    sai_object_id_t       sai_port_id = SAI_NULL_OBJECT_ID;
    sai_port_info_t       *tmp_port_info = sai_port_info;

    if((sai_port_info == NULL) || (breakout_cfg == NULL)) {
        SAI_PORT_LOG_TRACE("sai_port_info is %p breakout_cfg is %p in breakout mode update",
                           sai_port_info, breakout_cfg);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    sai_port_id = tmp_port_info->sai_port_id;

    prev_cap_val = sai_port_capb_from_break_mode(breakout_cfg->curr_mode);

    SAI_PORT_LOG_TRACE("Updating sai_port_info_t for control port 0x%"PRIx64" "
                       "with breakout mode %d", sai_port_id, breakout_cfg->new_mode);

    max_lanes = tmp_port_info->max_lanes_per_port;
    /* @todo handle SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE */
    if(breakout_cfg->new_mode == SAI_PORT_BREAKOUT_MODE_TYPE_1_LANE) { /* Single lane mode */

        /* Update control port's speed, lane bitmap and port capabilities */
        tmp_port_info->port_speed = breakout_cfg->new_speed;
        tmp_port_info->port_attr_info.speed = breakout_cfg->new_speed;

        /* @todo lane bitmap should be based on max_lanes */
        tmp_port_info->port_lane_bmap = SAI_FOUR_LANE_BITMAP;

        cap_val = (SAI_PORT_CAP_BREAKOUT_MODE | prev_cap_val);
        sai_port_capablility_enable_update(tmp_port_info, false, cap_val);
        sai_port_capablility_enable_update(tmp_port_info, true, SAI_PORT_CAP_BREAKOUT_MODE_1X);

        /* Disable the subsidiary ports and set appropriate capability flags */
        for (lane = 1; lane < max_lanes; lane++) {
            tmp_port_info = sai_port_info_getnext(tmp_port_info);
            sai_port_capablility_enable_update(tmp_port_info, false, cap_val);
            sai_port_capablility_enable_update(tmp_port_info, true, SAI_PORT_CAP_BREAKOUT_MODE_1X);
            tmp_port_info->port_valid = false;
            tmp_port_info->media_type = media_type;
        }
    } else if(breakout_cfg->new_mode == SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE) { /* 4 lane Mode */

        /* Update control port's speed, lane bitmap and port capabilities */
        tmp_port_info->port_lane_bmap = SAI_ONE_LANE_BITMAP;
        tmp_port_info->port_speed = breakout_cfg->new_speed;
        tmp_port_info->port_attr_info.speed = breakout_cfg->new_speed;

        sai_port_capablility_enable_update(tmp_port_info, false, prev_cap_val);
        cap_val = (SAI_PORT_CAP_BREAKOUT_MODE | SAI_PORT_CAP_BREAKOUT_MODE_4X);
        sai_port_capablility_enable_update(tmp_port_info, true, cap_val);
        media_type = tmp_port_info->media_type;

        /* Update the subsidiary ports with speed, media type, capabilities */
        for (lane = 1; lane < max_lanes; lane++) {
            tmp_port_info = sai_port_info_getnext(tmp_port_info);

            /* Set the port_valid bit and appropriate port speed */
            tmp_port_info->port_valid = true;
            tmp_port_info->port_speed = breakout_cfg->new_speed;
            tmp_port_info->port_attr_info.speed = breakout_cfg->new_speed;

            /* update with control port's media type
             * @todo remove this as it should be set from adapter host */
            tmp_port_info->media_type = media_type;

            /* Clear the previous capability and enable the new port capability */
            sai_port_capablility_enable_update(tmp_port_info, false, prev_cap_val);
            sai_port_capablility_enable_update(tmp_port_info, true, cap_val);
        }
    } else if(breakout_cfg->new_mode == SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE) { /* 2 lane mode */

        /* Update control port's speed, lane bitmap and port capabilities */
        tmp_port_info->port_lane_bmap = SAI_TWO_LANE_BITMAP;
        tmp_port_info->port_speed = breakout_cfg->new_speed;
        tmp_port_info->port_attr_info.speed = breakout_cfg->new_speed;

        sai_port_capablility_enable_update(tmp_port_info, false, prev_cap_val);
        cap_val = (SAI_PORT_CAP_BREAKOUT_MODE | SAI_PORT_CAP_BREAKOUT_MODE_2X);
        sai_port_capablility_enable_update(tmp_port_info, true, cap_val);
        media_type = tmp_port_info->media_type;

        /* Update the subsidiary ports with speed, media type, capabilities */
        for (lane = 1; lane < max_lanes; lane++) {
            tmp_port_info = sai_port_info_getnext(tmp_port_info);
            if(lane == 2) {
                /* Set the port_valid bit and appropriate port speed */
                tmp_port_info->port_valid = true;
                tmp_port_info->port_speed = breakout_cfg->new_speed;
                tmp_port_info->port_attr_info.speed = breakout_cfg->new_speed;
            } else {
                tmp_port_info->port_valid = false;
            }
            /* update with control port's media type*/
            tmp_port_info->media_type = media_type;
            /* Clear the previous capability and enable the new port capability */
            sai_port_capablility_enable_update(tmp_port_info, false, prev_cap_val);
            sai_port_capablility_enable_update(tmp_port_info, true, cap_val);

        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_update_valdity_on_create (sai_object_id_t create_port,
                                                sai_port_info_t * const sai_control_port_info,
                                                sai_port_breakout_mode_type_t new_mode)
{
    uint_t           lane = 0;
    uint_t           max_lanes = 0;
    sai_status_t     ret_code = SAI_STATUS_FAILURE;
    sai_port_info_t *sai_port_info = sai_control_port_info;

    if(sai_control_port_info == NULL) {
        SAI_PORT_LOG_TRACE("sai_control_port_info is %p for create port 0x%"PRIx64""
                           "in update validity on create",sai_control_port_info, create_port);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    max_lanes = sai_control_port_info->max_lanes_per_port;
    /* @todo handle SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE */
    if(new_mode == SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE) { /* 4 lane Mode */

        for (lane = 0; lane < max_lanes; lane++) {
            if(sai_port_info == NULL) {
                return SAI_STATUS_FAILURE;
            }
            if (create_port == sai_port_info->sai_port_id) {
                sai_port_info->port_valid = true;
            } else {
                sai_port_info->port_valid = false;
            }
            sai_port_info = sai_port_info_getnext(sai_port_info);
        }
    } else if (new_mode == SAI_PORT_BREAKOUT_MODE_TYPE_2_LANE) {
        for (lane = 0; lane < max_lanes; lane++) {
            if(sai_port_info == NULL) {
                return SAI_STATUS_FAILURE;
            }
            if(lane ==0 || lane == 2){
                if (create_port == sai_port_info->sai_port_id) {
                    sai_port_info->port_valid = true;
                } else {
                    sai_port_info->port_valid = false;
                }
            }
            sai_port_info = sai_port_info_getnext(sai_port_info);
        }
    }

    return ret_code;
}
void sai_port_logical_list_get(sai_object_list_t *port_list)
{
    uint_t count = 0;
    sai_port_info_t *port_info = NULL;

    if((port_list == NULL) || (port_list->list == NULL)) {
        SAI_PORT_LOG_TRACE("port_list is %p port_list->list is %p in port list get",
                           port_list, port_list->list);
        return;
    }

    for (port_info = sai_port_info_getfirst(); (port_info != NULL);
         port_info = sai_port_info_getnext(port_info)) {

        if(!sai_is_port_valid(port_info->sai_port_id)) {
            continue;
        }

        port_list->list[count] = port_info->sai_port_id;
        count++;
    }

    port_list->count = count;
}

bool sai_port_is_oper_up(sai_object_id_t port_obj)
{
    bool port_oper_up = false;

    sai_port_lock ();

    sai_port_info_t            *sai_port_info = sai_port_info_get(port_obj);
    const sai_port_attr_info_t *port_attr_info = sai_port_attr_info_read_only_get(port_obj,
                                                                                  sai_port_info);
    if (port_attr_info == NULL) {
        SAI_PORT_LOG_ERR ("Port attr info not found for port 0x%"PRIx64"",port_obj);
    } else if (port_attr_info->oper_status == SAI_PORT_OPER_STATUS_UP) {
        port_oper_up = true;
    }

    sai_port_unlock ();
    return port_oper_up;
}

sai_status_t sai_port_def_bridge_port_set (sai_object_id_t port, sai_object_id_t bridge_port_id)
{
    sai_port_info_t *port_info_table = sai_port_info_get(port);

    if(port_info_table == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    port_info_table->def_bridge_port_id = bridge_port_id;
    return SAI_STATUS_SUCCESS;

}

sai_status_t sai_port_def_bridge_port_get (sai_object_id_t port, sai_object_id_t *bridge_port_id)
{
    sai_port_info_t *port_info_table = sai_port_info_get(port);

    if(port_info_table == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    *bridge_port_id = port_info_table->def_bridge_port_id;
    return SAI_STATUS_SUCCESS;

}

sai_status_t sai_port_lag_set (sai_object_id_t port, sai_object_id_t lag_id)
{
    sai_port_info_t *port_info_table = sai_port_info_get(port);

    if(port_info_table == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    port_info_table->lag_id = lag_id;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_lag_get (sai_object_id_t port, sai_object_id_t *lag_id)
{
    sai_port_info_t *port_info_table = sai_port_info_get(port);

    if(port_info_table == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    *lag_id = port_info_table->lag_id;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_increment_ref_count (sai_object_id_t port)
{
    sai_port_info_t *port_info_table = sai_port_info_get(port);

    if(port_info_table == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    port_info_table->ref_count++;
    return SAI_STATUS_SUCCESS;

}

sai_status_t sai_port_decrement_ref_count (sai_object_id_t port)
{
    sai_port_info_t *port_info_table = sai_port_info_get(port);

    if(port_info_table == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    if(port_info_table->ref_count == 0) {
        return SAI_STATUS_FAILURE;
    }
    port_info_table->ref_count--;
    return SAI_STATUS_SUCCESS;

}

bool sai_is_port_in_use (sai_object_id_t port)
{
    sai_port_info_t *port_info_table = sai_port_info_get(port);

    if(port_info_table == NULL) {
        return false;
    }
    if(port_info_table->ref_count > 0) {
        return true;
    }
    return false;
}

sai_status_t sai_port_increment_vlan_count (sai_object_id_t port)
{
    sai_port_info_t *port_info_table = sai_port_info_get(port);

    if(port_info_table == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    port_info_table->num_vlans++;
    return SAI_STATUS_SUCCESS;

}

sai_status_t sai_port_decrement_vlan_count (sai_object_id_t port)
{
    sai_port_info_t *port_info_table = sai_port_info_get(port);

    if(port_info_table == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    if(port_info_table->num_vlans == 0) {
        return SAI_STATUS_FAILURE;
    }
    port_info_table->num_vlans--;
    return SAI_STATUS_SUCCESS;

}

uint_t sai_port_get_vlan_count (sai_object_id_t port)
{
    sai_port_info_t *port_info_table = sai_port_info_get(port);

    if(port_info_table == NULL) {
        return 0;
    }
    return port_info_table->num_vlans;
}

sai_status_t sai_port_pvid_get (sai_object_id_t port, uint16_t *pvid)
{
    sai_port_info_t *port_info_table = sai_port_info_get(port);

    if(pvid == NULL) {
        SAI_PORT_LOG_TRACE("pvid is NULL for port 0x%"PRIx64" in pvid get", port);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if(port_info_table == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    *pvid = port_info_table->port_attr_info.default_vlan;
    return SAI_STATUS_SUCCESS;

}

