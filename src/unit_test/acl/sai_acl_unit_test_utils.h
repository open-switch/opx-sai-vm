/************************************************************************
 * * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
 * *
 * * This source code is confidential, proprietary, and contains trade
 * * secrets that are the sole property of Dell Inc.
 * * Copy and/or distribution of this source code or disassembly or reverse
 * * engineering of the resultant object code are strictly forbidden without
 * * the written consent of Dell Inc.
 * *
 * ************************************************************************/
 /**
 * * @file sai_acl_unit_test_utils.h
 * *
 * * @brief This file contains class definition, utility and helper
 * *        function prototypes for testing the SAI ACL functionalities.
 * *
  *************************************************************************/

#ifndef __SAI_ACL_UNIT_TEST_H__
#define __SAI_ACL_UNIT_TEST_H__

#include "gtest/gtest.h"

extern "C" {
#include "saitypes.h"
#include "saistatus.h"
#include "saiswitch.h"
#include "saiacl.h"
#include "sailag.h"
#include "saimirror.h"
#include "saivlan.h"
#include "saiport.h"
#include "saibridge.h"
}

class saiACLTest : public ::testing::Test
{
    public:
        static void SetUpTestCase();
        static sai_object_id_t sai_acl_get_global_switch_id(void);

        /* Get SAI port id for acl test cases */
        static sai_object_id_t sai_acl_port_id_get (uint32_t port_index);
        static sai_object_id_t sai_acl_bridge_port_id_get (uint32_t port_index);
        static sai_object_id_t sai_acl_invalid_port_id_get ();
        static sai_object_id_t sai_test_acl_get_cpu_port ();

        static sai_status_t sai_test_acl_table_switch_get (
                                        sai_attribute_t *p_attr_list,
                                        unsigned int attr_count, ...);

        static sai_status_t sai_test_acl_table_create_attr_list(
                                            sai_attribute_t **p_attr_list,
                                            unsigned int attr_count);
        static void         sai_test_acl_table_free_attr_list(
                                            sai_attribute_t *p_attr_list);

        /* SAI API - ACL Table functionality testing. */
        static sai_status_t sai_test_acl_table_create (
                            sai_object_id_t *acl_table_id,
                            unsigned int attr_count, ...);
        static sai_status_t sai_test_acl_table_remove (
                            sai_object_id_t acl_table_id);
        static sai_status_t sai_test_acl_table_set (
                            sai_object_id_t acl_table_id,
                            unsigned int attr_count, ...);
        static sai_status_t sai_test_acl_table_get (
                            sai_object_id_t acl_table_id,
                            sai_attribute_t *p_attr_list_get,
                            unsigned int attr_count, ...);

        /* SAI API - ACL Rule functionality testing. */
        static sai_status_t sai_test_acl_rule_create (
                            sai_object_id_t *acl_rule_id,
                            unsigned int attr_count, ...);
        static sai_status_t sai_test_acl_rule_remove (
                            sai_object_id_t acl_rule_id);
        static sai_status_t sai_test_acl_rule_set (
                            sai_object_id_t acl_rule_id,
                            unsigned int attr_count, ...);
        static sai_status_t sai_test_acl_rule_get (
                            sai_object_id_t acl_rule_id,
                            sai_attribute_t *p_attr_list,
                            unsigned int attr_count, ...);

        /* SAI API - ACL Counter functionality testing. */
        static sai_status_t sai_test_acl_counter_create (
                            sai_object_id_t *acl_counter_id,
                            unsigned int attr_count, ...);
        static sai_status_t sai_test_acl_counter_remove (
                            sai_object_id_t acl_counter_id);
        static sai_status_t sai_test_acl_counter_set (
                            sai_object_id_t acl_counter_id,
                            unsigned int attr_count, ...);
        static sai_status_t sai_test_acl_counter_get (
                            sai_object_id_t acl_counter_id,
                            sai_attribute_t *p_attr_list,
                            unsigned int attr_count, ...);

        /* Util for converting to attribute index based status code */
        static inline sai_status_t sai_test_invalid_attr_status_code (
                                                       sai_status_t status,
                                                       unsigned int attr_index)
        {
            return (status + SAI_STATUS_CODE (attr_index));
        }

        /* SAI API - LAG functionality testing. */
        static sai_status_t sai_test_acl_rule_lag_create (
                            sai_object_id_t *lag_id, sai_attribute_t *attr);
        static sai_status_t sai_test_acl_rule_lag_delete (
                            sai_object_id_t lag_id);

        /* SAI API - Mirror functionality testing. */
        static sai_status_t sai_test_acl_rule_mirror_session_create (
                            sai_object_id_t *p_mirror_session_id,
                            unsigned int attr_count, sai_attribute_t *p_attr_list);
        static sai_status_t sai_test_acl_rule_mirror_session_destroy (
                            sai_object_id_t mirror_session_id);

        /* SAI API - Queue functionality testing. */
        static sai_status_t sai_test_acl_rule_get_max_queues (
                            sai_object_id_t port_id,
                            unsigned int *queue_count);
        static sai_status_t sai_test_acl_rule_get_queue_id_list (
                            sai_object_id_t port_id,
                            unsigned int queue_count,
                            sai_object_id_t *p_queue_id_list);

        /* SAI API - FDB functionality testing. */
        static sai_status_t sai_test_acl_fdb_entry_create (
                            sai_fdb_entry_t *fdb_entry,
                            sai_attribute_t *attr);
        static sai_status_t sai_test_acl_fdb_entry_remove (
                            sai_fdb_entry_t *fdb_entry);
        static sai_status_t sai_test_acl_fdb_entry_set (
                            sai_fdb_entry_t *fdb_entry,
                            uint32_t fdb_metadata);
        static sai_status_t sai_test_acl_fdb_entry_get (
                            sai_fdb_entry_t *fdb_entry,
                            sai_attribute_t *fdb_get_attr);

        /* SAI API - VLAN functionality testing. */
        static sai_status_t sai_test_acl_rule_vlan_port_add(
                sai_object_id_t *vlan_member_id, sai_object_id_t vlan_obj_id,
                sai_object_id_t bridge_port_id, sai_vlan_tagging_mode_t tagging_mode);
        static sai_status_t sai_test_acl_rule_vlan_port_remove(
                sai_object_id_t vlan_member_id);
        static sai_status_t sai_test_acl_rule_vlan_set(
                sai_object_id_t *vlan_obj_id, sai_vlan_id_t vlan_id,
                sai_attribute_t *p_attr_list, bool isCreate);
        static sai_status_t sai_test_acl_rule_vlan_remove(
                sai_object_id_t vlan_obj_id);
        static sai_status_t sai_test_acl_rule_vlan_get(
                sai_object_id_t vlan_obj_id, sai_attribute_t *p_attr_list);

        /* SAI API - Port functionality testing. */
        static sai_status_t sai_test_acl_rule_port_set(
                            sai_object_id_t port_id,
                            sai_attribute_t *p_attr_list);
        static sai_status_t sai_test_acl_rule_port_get(
                            sai_object_id_t port_id,
                            sai_attribute_t *p_attr_list);

        static const unsigned int SAI_TEST_MAX_MIRROR_SESSIONS      = 4;
        static const unsigned int SAI_ACL_MAX_FDB_ATTRIBUTES        = 4;
        static const unsigned int SAI_ACL_TEST_VLAN                 = 10;
        static const unsigned int SAI_ACL_TABLE_MAX_SIZE            = 4096;
        static const unsigned int SAI_ACL_TEST_TABLE_SIZE           = 10;
        static const unsigned int SAI_ACL_TEST_MAX_UDF_BYTES        = 32;

        static const sai_object_id_t SAI_ACL_TEST_VLAN_OBJ          = 0x2600000000000a;

    protected:
        static sai_switch_api_t             *p_sai_switch_api_tbl;
        static sai_acl_api_t                *p_sai_acl_api_tbl;
        static sai_lag_api_t                *p_sai_lag_api_tbl;
        static sai_mirror_api_t             *p_sai_mirror_api_tbl;
        static sai_port_api_t               *p_sai_port_api_tbl;
        static sai_fdb_api_t                *p_sai_fdb_api_tbl;
        static sai_vlan_api_t               *p_sai_vlan_api_tbl;
        static sai_bridge_api_t             *p_sai_bridge_api_tbl;
};

#endif  /* __SAI_ACL_UNIT_TEST_H__ */
