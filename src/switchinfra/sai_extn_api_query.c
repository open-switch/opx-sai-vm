/************************************************************************
 * LEGALESE:   "Copyright (c) 2016, Dell Inc. All rights reserved."
 *
 * This source code is confidential, proprietary, and contains trade
 * secrets that are the sole property of Dell Inc.
 * Copy and/or distribution of this source code or disassembly or reverse
 * engineering of the resultant object code are strictly forbidden without
 * the written consent of Dell Inc.
 *
 ************************************************************************/
/*!
 * \file   sai_extn_api_query.c
 * \brief  SAI Extension module API query and infra functionality.
 *
*************************************************************************/
#include "sai.h"
#include "sai_common_infra.h"
#include "sai_switch_utils.h"
#include <dlfcn.h>
#include <string.h>

/* @TODO Move this table to a config file */
static struct sai_extn_api_table_t {
    sai_api_t api_id;
    const char *api_query;
    const char *extn_lib_name;
    void *extn_lib_handle;
} sai_extn_api_table[] = {
{SAI_API_FC_SWITCH, "sai_fc_switch_api_query", "libsai-fc.so", NULL},
{SAI_API_FC_PORT, "sai_fc_port_api_query", "libsai-fc.so", NULL},
};

#define SAI_EXTN_LIB_NAME_STR_LEN_MAX 24

static const uint_t max_extn_api_modules =
sizeof(sai_extn_api_table)/sizeof(sai_extn_api_table[0]);

static struct sai_extn_api_table_t* sai_extn_api_module_table_get (sai_api_t id)
{
    uint_t  index;

    for (index = 0; index < max_extn_api_modules; index++) {

        if (id == sai_extn_api_table[index].api_id) {

            return (&sai_extn_api_table[index]);
        }
    }

    return NULL;
}

static void *sai_extn_api_lib_handle_get (const char *lib_name)
{
    uint_t  index;

    for (index = 0; index < max_extn_api_modules; index++) {

        if ((!(strncmp (sai_extn_api_table[index].extn_lib_name,
               lib_name, SAI_EXTN_LIB_NAME_STR_LEN_MAX))) &&
             (sai_extn_api_table[index].extn_lib_handle != NULL)) {

            return (sai_extn_api_table[index].extn_lib_handle);
        }
    }

    return NULL;
}

sai_status_t sai_extn_module_load_api_table (sai_api_t sai_api_id, void** api_method_table)
{
    void* (*query_fn_ptr) (void);
    void *api_lib_handle = NULL;

    struct sai_extn_api_table_t *api_module =
                                    sai_extn_api_module_table_get (sai_api_id);
    if (api_module == NULL) {

        SAI_SWITCH_LOG_ERR ("Invalid SAI Extension API Id %d", sai_api_id);

        return SAI_STATUS_FAILURE;
    }

    api_lib_handle = sai_extn_api_lib_handle_get (api_module->extn_lib_name);

    if (api_lib_handle == NULL) {

        api_lib_handle = dlopen (api_module->extn_lib_name, RTLD_LAZY);

        if (api_lib_handle == NULL) {

            SAI_SWITCH_LOG_ERR ("dlopen() failed for %s with Err: %s.",
                                api_module->extn_lib_name, dlerror());

            return SAI_STATUS_FAILURE;
        }

        api_module->extn_lib_handle = api_lib_handle;
    }

    query_fn_ptr = dlsym (api_lib_handle, api_module->api_query);

    if (query_fn_ptr == NULL) {

        SAI_SWITCH_LOG_ERR ("dlsym() for %s failed with Err: %s.",
                            api_module->api_query, dlerror());

        return SAI_STATUS_FAILURE;

    } else {

        *api_method_table = (*query_fn_ptr)();
    }

    if ((*api_method_table) == NULL) {

        SAI_SWITCH_LOG_ERR ("SAI Extension API method table for API Id %d is null.",
                            sai_api_id);
        return SAI_STATUS_FAILURE;
    }

    SAI_SWITCH_LOG_INFO ("SAI Extension API method table query done for "
                         "API Id %d.", sai_api_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_extn_module_api_query (sai_api_t sai_api_id, void** api_method_table)
{
    sai_status_t  status = SAI_STATUS_FAILURE;

    switch (sai_api_id)
    {
        case SAI_API_FC_SWITCH:
        case SAI_API_FC_PORT:
            status = sai_extn_module_load_api_table (sai_api_id, api_method_table);
            break;

        default:
            SAI_SWITCH_LOG_ERR("Method table for api-id %d",sai_api_id);
            status = SAI_STATUS_NOT_SUPPORTED;
            break;
    }

    return status;
}

