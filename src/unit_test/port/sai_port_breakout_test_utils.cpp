#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sai_port_breakout_test_utils.h"

extern "C" {
#include "saiswitch.h"
#include "saitypes.h"
#include "saiport.h"
#include "saistatus.h"
#include "std_assert.h"
#include <inttypes.h>
}


#define LOG_PRINT(msg, ...) \
    printf(msg, ##__VA_ARGS__)
/*
 *Checks whether the given port is a logical port
 */

bool sai_port_type_logical(sai_object_id_t port_id, sai_port_api_t
                                                    *sai_port_api_table)
{
    sai_attribute_t sai_attr_get;

    memset(&sai_attr_get, 0, sizeof(sai_attribute_t));
    sai_attr_get.id = SAI_PORT_ATTR_TYPE;

    if(sai_port_api_table->get_port_attribute(port_id, 1, &sai_attr_get) != SAI_STATUS_SUCCESS) {
        return false;
    }

    if(SAI_PORT_TYPE_LOGICAL != sai_attr_get.value.s32) {
        return false;
    }

    return true;
}

/*
 * Checks whether the given breakout mode is supported in the given port
 */

sai_status_t sai_check_supported_breakout_mode(sai_port_api_t *sai_port_api_table,
                                                      sai_object_id_t port,
                                                      sai_port_breakout_mode_type_t
                                                      breakout_mode,
                                                      bool *is_supported)
{
    unsigned int mode_index = 0;
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t sai_attr_get;
    *is_supported = false;

    if(sai_port_type_logical(port, sai_port_api_table) != true) {
        LOG_PRINT("Port 0x%" PRIx64 " is not a logical port", port);
        return ret;
    }

    memset(&sai_attr_get, 0, sizeof(sai_attr_get));
    sai_attr_get.id = SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE_TYPE;
    sai_attr_get.value.s32list.count = SAI_PORT_BREAKOUT_MODE_TYPE_MAX;
    sai_attr_get.value.s32list.list = (int32_t *)calloc (SAI_PORT_BREAKOUT_MODE_TYPE_MAX,
                                                         sizeof(int32_t));
    if(sai_attr_get.value.s32list.list == NULL) {
        LOG_PRINT("Could not allocate memory for breakout modes list");
        return SAI_STATUS_NO_MEMORY;
    }

    ret = sai_port_api_table->get_port_attribute(port, 1, &sai_attr_get);
    if(ret == SAI_STATUS_BUFFER_OVERFLOW ) {
        free(sai_attr_get.value.u32list.list);
        sai_attr_get.value.s32list.list = (int32_t *)calloc (sai_attr_get.value.s32list.count,
                                                             sizeof(int32_t));
        ret = sai_port_api_table->get_port_attribute(port, 1, &sai_attr_get);
    }
    if(ret != SAI_STATUS_SUCCESS) {
        free(sai_attr_get.value.s32list.list);
        return ret;
    }

    for(mode_index = 0; mode_index < sai_attr_get.value.s32list.count;
    mode_index++) {
        if(sai_attr_get.value.s32list.list[mode_index] == breakout_mode) {
            LOG_PRINT("port 0x%" PRIx64 " supports mode %d", port, breakout_mode);
            *is_supported = true;
            break;
        }
    }

    free(sai_attr_get.value.u32list.list);
    return ret;
}

/*
 * Gets the current breakout mode of the given port
 */

sai_status_t sai_port_break_out_mode_get(sai_port_api_t *sai_port_api_table,
                                         sai_object_id_t port,
                                         sai_port_breakout_mode_type_t breakout_mode)
{
    sai_status_t ret = SAI_STATUS_FAILURE;
    sai_attribute_t get_attr;
    get_attr.id = SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE;

    ret = sai_port_api_table->get_port_attribute(port, 1, &get_attr);
    if(ret != SAI_STATUS_SUCCESS) {
        LOG_PRINT("Port 0x%" PRIx64 " get current breakout mode %d failed\n",
                   port, breakout_mode);
        return ret;
    }

    if (get_attr.value.s32 != breakout_mode) {
        LOG_PRINT("Port 0x%" PRIx64 " breakout mode %d configuration failed \n",
                   port, breakout_mode);
        return SAI_STATUS_FAILURE;
    }

    LOG_PRINT("Port 0x%" PRIx64 " breakout configuration %d success\n", port,
               breakout_mode);
    return ret;
}
