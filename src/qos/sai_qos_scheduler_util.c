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
* @file  sai_qos_scheduler_util.c
*
* @brief This file contains utility function definitions for SAI QOS
*        scheduler functionality API implementation.
*
*************************************************************************/

#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "sai_qos_api_utils.h"
#include "sai_common_infra.h"

#include "saistatus.h"
#include "saitypes.h"

#include "std_assert.h"

#include <inttypes.h>

/* Input *p_count != 0 in case of revert == true */
static sai_status_t sai_qos_scheduler_queue_reapply (
                                      dn_sai_qos_scheduler_t *p_old_sched_node,
                                      dn_sai_qos_scheduler_t *p_new_sched_node,
                                      uint_t *p_count, bool revert)
{
    sai_status_t            sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_queue_t     *p_queue_node = NULL;
    uint_t                  count = 0;
    dn_sai_qos_scheduler_t *p_sched_node = NULL;

    STD_ASSERT (p_old_sched_node != NULL);
    STD_ASSERT (p_new_sched_node != NULL);
    STD_ASSERT (p_count != NULL);

    /* Only Original scheduler node will have glue pointers.
     * Apply case Original - p_old_sched_node
     * Revert case Original - p_new_sched_node */
    p_sched_node = (revert ? p_new_sched_node : p_old_sched_node);

    for (p_queue_node = sai_qos_scheduler_get_first_queue (p_sched_node);
        (p_queue_node != NULL); p_queue_node =
         sai_qos_scheduler_get_next_queue (p_sched_node, p_queue_node)) {

        if (revert && (count >= *p_count))
            break;

        SAI_SCHED_LOG_TRACE ("Reapplying Scheduler 0x%"PRIx64" on "
                             "Queue 0x%"PRIx64"", p_old_sched_node->key.scheduler_id,
                             p_queue_node->key.queue_id);
        sai_rc = sai_scheduler_npu_api_get()->scheduler_set (p_queue_node->key.queue_id,
                                                             p_old_sched_node,
                                                             p_new_sched_node);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR("Scheduler 0x%"PRIx64" reapply on queue 0x%"PRIx64" "
                              "failed with err %d", p_old_sched_node->key.scheduler_id,
                              p_queue_node->key.queue_id, sai_rc);
            break;
        }
        count++;
    }
    *p_count = count;

    return sai_rc;
}

/* Input *p_count != 0 in case of revert == true */
static sai_status_t sai_qos_scheduler_sched_group_reapply (
                                      dn_sai_qos_scheduler_t *p_old_sched_node,
                                      dn_sai_qos_scheduler_t *p_new_sched_node,
                                      uint_t *p_count, bool revert)
{
    sai_status_t                sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_sched_group_t   *p_sg_node = NULL;
    uint_t                      count = 0;
    dn_sai_qos_scheduler_t     *p_sched_node = NULL;

    STD_ASSERT (p_old_sched_node != NULL);
    STD_ASSERT (p_new_sched_node != NULL);
    STD_ASSERT (p_count != NULL);

    /* Only Original scheduler node will have glue pointers.
     * Apply case Original - p_old_sched_node
     * Revert case Original - p_new_sched_node */
    p_sched_node = (revert ? p_new_sched_node : p_old_sched_node);


    for (p_sg_node = sai_qos_scheduler_get_first_sched_group (p_sched_node);
        (p_sg_node != NULL) ; p_sg_node =
         sai_qos_scheduler_get_next_sched_group (p_sched_node, p_sg_node)) {

        if (revert && (count >= *p_count))
            break;

        SAI_SCHED_LOG_TRACE ("Reapplying Scheduler 0x%"PRIx64" on SG 0x%"PRIx64"",
                             p_old_sched_node->key.scheduler_id,
                             p_sg_node->key.sched_group_id);
        sai_rc = sai_scheduler_npu_api_get()->scheduler_set (p_sg_node->key.sched_group_id,
                                                             p_old_sched_node,
                                                             p_new_sched_node);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR("Scheduler 0x%"PRIx64" reapply on SG 0x%"PRIx64" "
                              "failed with err %d", p_old_sched_node->key.scheduler_id,
                              p_sg_node->key.sched_group_id, sai_rc);
            break;

        }
        count++;
    }
    *p_count = count;

    return sai_rc;
}

/* Input *p_count != 0 in case of revert == true */
static sai_status_t sai_qos_scheduler_port_reapply (
                                      dn_sai_qos_scheduler_t *p_old_sched_node,
                                      dn_sai_qos_scheduler_t *p_new_sched_node,
                                      uint_t *p_count, bool revert)
{
    sai_status_t            sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_port_t      *p_qos_port_node = NULL;
    uint_t                  count = 0;
    dn_sai_qos_scheduler_t *p_sched_node = NULL;

    STD_ASSERT (p_old_sched_node != NULL);
    STD_ASSERT (p_new_sched_node != NULL);
    STD_ASSERT (p_count != NULL);

    /* Only Original scheduler node will have glue pointers.
     * Apply case Original - p_old_sched_node
     * Revert case Original - p_new_sched_node */
    p_sched_node = (revert ? p_new_sched_node : p_old_sched_node);

    for (p_qos_port_node = sai_qos_scheduler_get_first_port (p_sched_node);
         (p_qos_port_node != NULL) ; p_qos_port_node =
         sai_qos_scheduler_get_next_port (p_sched_node, p_qos_port_node)) {

        if (revert && (count >= *p_count))
            break;

        SAI_SCHED_LOG_TRACE ("Reapplying Scheduler 0x%"PRIx64" on port 0x%"PRIx64"",
                             p_old_sched_node->key.scheduler_id,
                             p_qos_port_node->port_id);

        sai_rc = sai_scheduler_npu_api_get()->scheduler_set (p_qos_port_node->port_id,
                                                             p_old_sched_node,
                                                             p_new_sched_node);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR("Scheduler 0x%"PRIx64" reapply on port 0x%"PRIx64" "
                              "failed with err %d", p_old_sched_node->key.scheduler_id,
                              p_qos_port_node->port_id, sai_rc);
            break;
        }
        count++;
    }

    *p_count = count;

    return sai_rc;
}

sai_status_t sai_qos_scheduler_reapply (dn_sai_qos_scheduler_t *p_old_sched_node,
                                        dn_sai_qos_scheduler_t *p_new_sched_node)
{
    sai_status_t                sai_rc = SAI_STATUS_SUCCESS;
    sai_status_t                sai_revrt_rc = SAI_STATUS_SUCCESS;
    uint_t                      update_queue_cnt = 0;
    uint_t                      update_sg_cnt = 0;
    uint_t                      update_port_cnt = 0;

    STD_ASSERT (p_old_sched_node != NULL);

    do {
        sai_rc = sai_qos_scheduler_queue_reapply (p_old_sched_node, p_new_sched_node,
                                                  &update_queue_cnt, false);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR("Failed to reapply Scheduler 0x%"PRIx64" on queue.",
                              p_new_sched_node->key.scheduler_id);
            break;
        }

        sai_rc = sai_qos_scheduler_sched_group_reapply (p_old_sched_node, p_new_sched_node,
                                                        &update_sg_cnt, false);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR("Failed to reapply Scheduler 0x%"PRIx64" on SG.",
                              p_new_sched_node->key.scheduler_id);
            break;
        }

        sai_rc = sai_qos_scheduler_port_reapply (p_old_sched_node, p_new_sched_node,
                                                 &update_port_cnt, false);
        if (sai_rc != SAI_STATUS_SUCCESS) {
            SAI_SCHED_LOG_ERR("Failed to reapply Scheduler 0x%"PRIx64" on port.",
                              p_new_sched_node->key.scheduler_id);
            break;
        }
    } while(0);

     if (sai_rc != SAI_STATUS_SUCCESS) {
        if (update_queue_cnt) {
            sai_revrt_rc = sai_qos_scheduler_queue_reapply (p_new_sched_node, p_old_sched_node,
                                                            &update_queue_cnt, true);
            if (sai_revrt_rc != SAI_STATUS_SUCCESS) {
                SAI_SCHED_LOG_ERR("Failed to revert Scheduler 0x%"PRIx64" on queue.",
                                  p_old_sched_node->key.scheduler_id);
                /* TODO - ADDing ASSERT is only possibel here */
            }
        }

        if (update_sg_cnt) {
            sai_revrt_rc = sai_qos_scheduler_sched_group_reapply (p_new_sched_node, p_old_sched_node,
                                                                  &update_sg_cnt, true);
            if (sai_revrt_rc != SAI_STATUS_SUCCESS) {
                SAI_SCHED_LOG_ERR("Failed to revert Scheduler 0x%"PRIx64" on SG.",
                                  p_old_sched_node->key.scheduler_id);
                /* TODO - ADDing ASSERT is only possibel here */
            }
        }

        if (update_port_cnt) {
            sai_revrt_rc = sai_qos_scheduler_port_reapply (p_new_sched_node, p_old_sched_node,
                                                           &update_port_cnt, true);
            if (sai_revrt_rc != SAI_STATUS_SUCCESS) {
                SAI_SCHED_LOG_ERR("Failed to revert Scheduler 0x%"PRIx64" on port.",
                                  p_old_sched_node->key.scheduler_id);
                /* TODO - ADDing ASSERT is only possibel here */
            }
        }
     }
     return sai_rc;
}
