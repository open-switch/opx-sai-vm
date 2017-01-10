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
/*
 * @file sai_acl_db_api.h
 *
 * @brief This file contains the function prototypes for updating the
 *        SQL DB tables related to the SAI ACL objects - table, entry and
 *        counter objects in VM environment.
 *************************************************************************/

#ifndef __SAI_ACL_DB_API_H__
#define __SAI_ACL_DB_API_H__

#include "sai_acl_type_defs.h"
#include "saitypes.h"
#include "std_type_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief Create a new entry in the ACL TABLE database table
 * @param p_acl_table - New ACL table node to be inserted in DB.
 * @return sai status code
 */
sai_status_t sai_acl_table_create_db_entry (sai_acl_table_t *p_acl_table);

/*
 * @brief Delete an entry from the ACL TABLE database table
 * @param p_acl_table - ACL table node to be deleted from DB.
 * @return sai status code
 */
sai_status_t sai_acl_table_delete_db_entry (sai_acl_table_t *p_acl_table);

/*
 * @brief Create a new entry in the ACL ENTRY database table
 * @param p_acl_rule - New ACL entry node to be inserted in DB.
 * @return sai status code
 */
sai_status_t sai_acl_rule_create_db_entry (sai_acl_rule_t *p_acl_rule);

/*
 * @brief Delete an entry from the ACL ENTRY database table
 * @param p_acl_rule - ACL entry node to be deleted from DB.
 * @return sai status code
 */
sai_status_t sai_acl_rule_delete_db_entry (sai_acl_rule_t *p_acl_rule);

/*
 * @brief Set the fields in the existing ACL ENTRY database table
 * @param p_new_acl_rule - ACL entry node with new actions and filters.
 * @param p_existing_acl_rule - ACL entry node with existing actions and
 * filters.
 * @return sai status code
 */
sai_status_t sai_acl_rule_set_db_entry (sai_acl_rule_t *p_new_acl_rule,
                                        sai_acl_rule_t *p_existing_acl_rule);

/*
 * @brief Update the counter ID field in the ACL ENTRY database table.
 * @param rule_id - SAI object ID for ACL Entry.
 * @param cntr_id - SAI object ID for Counter Entry.
 * @param attach_cntr - Specifies whether counter has to be added or removed
 * from the ACL entry DB record.
 * @return sai status code
 */
sai_status_t sai_acl_rule_db_entry_update_counter_id (sai_object_id_t rule_id,
                                                      sai_object_id_t cntr_id,
                                                      bool attach_cntr);

/*
 * @brief Create a new entry in the ACL COUNTER database table
 * @param p_acl_cntr - New ACL Counter node to be inserted in DB.
 * @return sai status code
 */
sai_status_t sai_acl_counter_create_db_entry (sai_acl_counter_t *p_acl_cntr);

/*
 * @brief Delete an entry from the ACL COUNTER database table
 * @param p_acl_cntr - ACL Counter node to be deleted from DB.
 * @return sai status code
 */
sai_status_t sai_acl_counter_delete_db_entry (sai_acl_counter_t *p_acl_cntr);

/*
 * @brief Set counter fields in counter entry on ACL COUNTER database table
 * @param p_acl_cntr - ACL Counter node.
 * @param count_value - counter value to be updated on the counter field.
 * @return sai status code
 */
sai_status_t sai_acl_counter_db_entry_set_cntrs (sai_acl_counter_t *p_acl_cntr,
                                                 uint_t count_value);

/*
 * @brief Set reference count field in counter entry on ACL COUNTER database
 * table.
 * @param p_acl_cntr - ACL Counter node.
 * @return sai status code
 */
sai_status_t sai_acl_counter_db_entry_set_ref_count (
sai_acl_counter_t *p_acl_cntr);

#ifdef __cplusplus
}
#endif

#endif /* __SAI_ACL_DB_API_H__ */
