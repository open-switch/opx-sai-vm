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

/*
 * @file sai_vm_cfg.c
 *
 * @brief This file contains function implementations for configuration
 *        loading relevant to SAI VM
 *************************************************************************/
#include "std_config_node.h"
#include "std_llist.h"
#include "std_utils.h"
#include "sai_switch_utils.h"
#include <stdlib.h>

/* Configuration XML file describing the interface to port mapping */
#define PORT_PHYSICAL_MAPPING_TABLE_XML "/etc/opx/base_port_physical_mapping_table.xml"

/* String defining the front panel port node.  This is file format specific */
#define FRONT_PORT_NODE_STRING  "front-panel-port"

/* string defining the interface name attribute.  This is file format-specific */
#define INTERFACE_NAME_ATTRIBUTE_STRING "name"

/* String defining the front entry node.  This is file format specific */
#define ENTRY_NODE_STRING  "entry"

/* string defining the port ID attribute.  This is file format-specific */
#define HW_PORT_ATTRIBUTE_STRING "hwport"

/* Allowable size of the interface name (from configuration file) */
#define INTERFACE_NAME_LEN  32

/* Linked node for mapping entries */
typedef struct _interface_map_list_entry_t {
    std_dll node;
    char    name[ INTERFACE_NAME_LEN ];
    int     port;
} interface_map_entry_t;

/* master linked list holding interface : port mapping */
static std_dll_head head;


/***************************************************************************
 * Function:    sai_vm_cfg_find_interface()
 *
 * Description: Scans the interface linked list searching for the match.
 *              Returns the port number, or (-1) if not found.
 **************************************************************************/
sai_status_t sai_vm_cfg_find_interface(const char *name, sai_npu_port_id_t *port)
{
    sai_status_t status = SAI_STATUS_FAILURE;
    std_dll      *walk = NULL;

    for(walk = std_dll_getfirst(&head);
            walk != NULL;
            walk = std_dll_getnext(&head, walk)) {
        interface_map_entry_t *entry = (interface_map_entry_t*)walk;

        if (strncasecmp((char*)entry->name, name, INTERFACE_NAME_LEN) == 0) {
            *port = (sai_npu_port_id_t)entry->port;
            status = SAI_STATUS_SUCCESS;
            break;
        }
    }
    return status;
}


/***************************************************************************
 * Function:    sai_vm_cfg_add_interface_entry()
 *
 * Description: Adds an interface : port pair to the mapping.
 *
 **************************************************************************/
static void sai_vm_cfg_add_interface_entry(const char *name, int port)
{
    interface_map_entry_t *new = NULL;

    new = (interface_map_entry_t*)calloc(1, sizeof(interface_map_entry_t));
    if (new == NULL) {
        SAI_SWITCH_LOG_ERR("Failed allocating memory for interface entry");
    } else {
        safestrncpy(new->name, name, sizeof(new->name));
        new->port = port;
        std_dll_insertatback(&head, (std_dll*)new);
    }
}

/***************************************************************************
 * Function:    sai_vm_cfg_load_interface_cfg()
 *
 * Description: Loads XML configuration information for port mapping.
 *
 **************************************************************************/
void sai_vm_cfg_load_interface_cfg(void) {
    std_config_hdl_t  cfg_hdl = NULL;
    std_config_node_t root_node = NULL;
    std_config_node_t panel_node = NULL;

    std_dll_init(&head);

    /* open configuration file */
    cfg_hdl = std_config_load(PORT_PHYSICAL_MAPPING_TABLE_XML);
    if (cfg_hdl == NULL) {
        SAI_SWITCH_LOG_ERR("Cannot open configuration file (%s)",
                PORT_PHYSICAL_MAPPING_TABLE_XML);
        return;
    }

    root_node = std_config_get_root(cfg_hdl);

    /*
     * Scan each child node and look for the "front-panel" nodes.
     * For each front panel node, extract the (interface) name and read
     * the FIRST hwport entry.  At this time, we are not interested in
     * the additional (breakout) ports available to that interface name
     */
    for (panel_node = std_config_get_child(root_node); panel_node != 0;
            panel_node = std_config_next_node(panel_node)) {

        std_config_node_t entry_node = NULL;
        int port_id = 0;

        char *interface_name = NULL;
        char *hwport = NULL;

        /* check for front-panel-port node, skip others */
        if (strcmp(std_config_name_get(panel_node), FRONT_PORT_NODE_STRING))
            continue;

        /* check for correct interface name syntax, skip if questionable */
        if((interface_name = std_config_attr_get(panel_node, INTERFACE_NAME_ATTRIBUTE_STRING)) == NULL) {
            continue;
        }

        entry_node = std_config_get_child(panel_node);

        /* check for entry node, skip others */
        if (strcmp(std_config_name_get(entry_node), ENTRY_NODE_STRING))
            continue;

        /* extract the hwport value */
        if((hwport = std_config_attr_get(entry_node, HW_PORT_ATTRIBUTE_STRING)) == NULL) {
            continue;
        }

        port_id = atoi(hwport);

        /* add the name/id pair to the mapping database */
        sai_vm_cfg_add_interface_entry(interface_name, port_id);
    }

    /* close and cleanup allocated memory */
    std_config_unload(cfg_hdl);
}
