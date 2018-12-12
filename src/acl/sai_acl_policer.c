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
 *
 *
 *  @file sai_acl_policer.c
 *
 *  @brief This file contains functions for SAI ACL policer operations.
 *
 *************************************************************************/
#include "sai_acl_utils.h"
#include "sai_qos_util.h"
#include "sai_acl_type_defs.h"
#include "sai_acl_npu_api.h"
#include "sai_common_acl.h"

#include "saitypes.h"
#include "saistatus.h"
#include "sai_oid_utils.h"

#include "std_type_defs.h"
#include "sai_common_infra.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

sai_status_t sai_attach_policer_to_acl_rule(sai_acl_rule_t *acl_rule)
{
    dn_sai_qos_policer_t  *p_policer_node = NULL;

    STD_ASSERT(acl_rule != NULL);

    SAI_ACL_LOG_TRACE ("Updating ACL Policer 0x%"PRIx64" "
                       "with ACL rule Id 0x%"PRIx64" in Table Id 0x%"PRIx64"", acl_rule->policer_id,
                       acl_rule->rule_key.acl_id, acl_rule->table_id);

    p_policer_node = sai_qos_policer_node_get(acl_rule->policer_id);

    if(p_policer_node == NULL) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    std_dll_insertatback(&p_policer_node->acl_dll_head, &acl_rule->policer_glue);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_detach_policer_from_acl_rule(sai_acl_rule_t *acl_rule)
{
    dn_sai_qos_policer_t  *p_policer_node = NULL;

    STD_ASSERT(acl_rule != NULL);

    p_policer_node = sai_qos_policer_node_get(acl_rule->policer_id);

    if(p_policer_node == NULL) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    SAI_ACL_LOG_TRACE ("Updating Policer 0x%"PRIx64" "
                       "with ACL rule Id 0x%"PRIx64" in Table Id 0x%"PRIx64"",
                       acl_rule->policer_id, acl_rule->rule_key.acl_id,
                       acl_rule->table_id);

    std_dll_remove(&p_policer_node->acl_dll_head, &acl_rule->policer_glue);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_policer_acl_entries_update(dn_sai_qos_policer_t *p_policer,
                                            dn_sai_qos_policer_t *p_policer_new)
{
    STD_ASSERT(p_policer != NULL);
    sai_status_t rc = SAI_STATUS_SUCCESS;

    rc = sai_acl_npu_api_get()->update_policer_acl_rule(p_policer, p_policer_new);

    return rc;
}

sai_status_t  sai_acl_rule_policer_update(sai_acl_rule_t *acl_rule_modify,
                                          sai_acl_rule_t *acl_rule_present)
{
    sai_status_t sai_rc = SAI_STATUS_SUCCESS;
    SAI_ACL_LOG_TRACE("Policer id in hardware is 0x%"PRIx64"",acl_rule_modify->policer_id);

    if(acl_rule_modify->policer_id != 0) {
        if(acl_rule_modify->policer_id == acl_rule_present->policer_id){
            SAI_ACL_LOG_TRACE("Policer id is already present on rule");
            return sai_rc;
        }
        if(acl_rule_present->policer_id != 0){
            if ((sai_rc = sai_detach_policer_from_acl_rule(acl_rule_present)) !=
                SAI_STATUS_SUCCESS) {
                SAI_ACL_LOG_ERR (" ACL Policer 0x%"PRIx64" failed to detach from  ACL"
                                 "Rule Id 0x%"PRIx64" in Table Id 0x%"PRIx64"",
                                 acl_rule_present->policer_id,
                                 acl_rule_present->rule_key.acl_id,
                                 acl_rule_present->table_id);
                return sai_rc;
            }
        }

        acl_rule_present->policer_id = acl_rule_modify->policer_id;
        SAI_ACL_LOG_TRACE("Policer id in hardware is 0x%"PRIx64"",acl_rule_modify->policer_id);

        if ((sai_rc = sai_attach_policer_to_acl_rule(acl_rule_present)) !=
            SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR (" ACL Policer 0x%"PRIx64" failed to attach with ACL"
                             "Rule Id 0x%"PRIx64" in Table Id 0x%"PRIx64"",
                             acl_rule_present->policer_id,
                             acl_rule_present->rule_key.acl_id,
                             acl_rule_present->table_id);
            return sai_rc;
        }
    }

    if((acl_rule_modify->policer_id == 0) && (acl_rule_present->policer_id != 0)){
        if ((sai_rc = sai_detach_policer_from_acl_rule(acl_rule_present)) !=
            SAI_STATUS_SUCCESS) {
            SAI_ACL_LOG_ERR (" ACL Policer 0x%"PRIx64" failed to detach from  ACL"
                             "Rule Id 0x%"PRIx64" in Table Id 0x%"PRIx64"",
                             acl_rule_present->policer_id,
                             acl_rule_present->rule_key.acl_id,
                             acl_rule_present->table_id);
            return sai_rc;
        }
        acl_rule_present->policer_id = acl_rule_modify->policer_id;
    }
    return sai_rc;
}
