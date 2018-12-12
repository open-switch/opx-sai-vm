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
 * \file   sai_db_init.cpp
 * \brief  SAI DB init functionality
 * \date   03-2015
 */

#include "sai_vm_db_utils.h"
#include "db_sql_ops.h"
#include "sai_vm_event_log.h"
#include "saitypes.h"
#include "saistatus.h"
#include "event_log.h"
#include "std_config_file.h"
#include "std_type_defs.h"

#include <stdlib.h>
#include <stddef.h>

#include <string>
#include <sstream>

#define SAI_DB_CONFIG_FILE     "/etc/opx/sai_vm_db.cfg"
#define SAI_DB_PATH_INFO_GRP   "sai_db_path_info"
#define SAI_DB_PATH            "db_path"
#define SAI_DB_SQL_SCRIPT_PATH "sql_script_path"
#define SAI_DB_CREATE_SCRIPT   "create_script"
#define SAI_DB_DELETE_SCRIPT   "delete_script"

db_sql_handle_t db = NULL;

sai_status_t sai_vm_db_init (void)
{
    std_cfg_file_handle_t  cfg_file_handle;
    size_t                 grp_idx = 0;
    static const char     *obj_grp_name [] = {
        "sai_db_switch_cfg", "sai_db_route_cfg", "sai_db_switching_cfg",
        "sai_db_acl_cfg"};
    uint_t                 num_obj_grp =
        sizeof (obj_grp_name)/ sizeof (*obj_grp_name);

    if ((std_config_file_open (&cfg_file_handle, SAI_DB_CONFIG_FILE)) !=
        STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Missing SAI VM DB config file: %s.",
                           SAI_DB_CONFIG_FILE);

        return SAI_STATUS_FAILURE;
    }

    const char *db_path =
        std_config_file_get (cfg_file_handle, SAI_DB_PATH_INFO_GRP,
                             SAI_DB_PATH);

    const char *sql_script_path =
        std_config_file_get (cfg_file_handle, SAI_DB_PATH_INFO_GRP,
                             SAI_DB_SQL_SCRIPT_PATH);

    if ((db_path == NULL) || (sql_script_path == NULL)) {
        SAI_VM_DB_LOG_ERR ("Error Parsing SAI VM DB config file: %s, group: %s",
                           SAI_DB_CONFIG_FILE, SAI_DB_SQL_SCRIPT_PATH);

        return SAI_STATUS_FAILURE;
    }

    if (db_sql_open ((void **)&db, std::string(db_path).c_str())
        != STD_ERR_OK) {
        SAI_VM_DB_LOG_ERR ("Error getting database handle.");

        return SAI_STATUS_FAILURE;
    }

    for (grp_idx = 0; grp_idx < num_obj_grp; grp_idx++) {
        const char *delete_script =
            std_config_file_get (cfg_file_handle, obj_grp_name [grp_idx],
                                 SAI_DB_DELETE_SCRIPT);

        const char *create_script =
            std_config_file_get (cfg_file_handle, obj_grp_name [grp_idx],
                                 SAI_DB_CREATE_SCRIPT);

        if ((create_script == NULL) || (delete_script == NULL)) {
            SAI_VM_DB_LOG_ERR ("Error Parsing SAI VM DB config file: %s, "
                               "group: %s.", SAI_DB_CONFIG_FILE,
                               obj_grp_name [grp_idx]);

            return SAI_STATUS_FAILURE;
        }

        const std::string create_table_str =
            "cat " + std::string(sql_script_path) + std::string(create_script) +
            " | sqlite3 " + std::string(db_path);
        const std::string drop_table_str =
            "cat " + std::string(sql_script_path) + std::string(delete_script) +
            " | sqlite3 " + std::string(db_path);

        if (system (drop_table_str.c_str ()) == -1) {
            SAI_VM_DB_LOG_ERR ("Error dropping tables from Database.");

            return SAI_STATUS_FAILURE;
        }

        if (system (create_table_str.c_str ()) == -1) {
            SAI_VM_DB_LOG_ERR ("Error creating tables in Database.");

            return SAI_STATUS_FAILURE;
        }
    }

    std_config_file_close (cfg_file_handle);

    return SAI_STATUS_SUCCESS;
}

db_sql_handle_t sai_vm_get_db_handle (void)
{
    return db;
}
