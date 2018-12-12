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
 *  * filename:sai_infra_api.h
 *  */

/** OPENSOURCELICENSE */

#ifndef __SAI_INFRA_API_H
#define __SAI_INFRA_API_H
#include "std_type_defs.h"
#include "saiswitch.h"

/** \defgroup SAISWITCHAPIS SAI - Switch and infra implementation
 *   Switch and Infra APIs for SAI. APIS are to be used by SAI
 *   only and not the upper layers.
 *
 *  \{
 */

/**
 * @brief  Retrieve switch functionality method table.
 *
 * @return Address of the structure containing the switch functionality
 *         method table.
 **/

sai_switch_api_t* sai_switch_api_query(void);

/**
 * @brief Create switch
 *
 *   SDK initialization/connect to SDK. After the call the capability attributes should be
 *   ready for retrieval via sai_get_switch_attribute(). Same Switch Object id should be
 *   given for create/connect for each NPU.
 *
 * @param[out] switch_id The Switch Object ID
 * @param[in] attr_count number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t sai_create_switch(sai_object_id_t* switch_id, uint32_t attr_count,
                                                 const sai_attribute_t *attr_list);

/**
 * @brief Remove/disconnect Switch
 *   Release all resources associated with currently opened switch
 *
 * @param[in] switch_id The Switch id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t sai_remove_switch(sai_object_id_t switch_id);

/**
 *  @brief Set the switch attribute value.
 *
 *  @param[in] attr - switch attribute structure with the attribute
 *                    and value.
 *
 *  @return SAI_STATUS_SUCCESS - on success or a failure status code on
 *          error
 */

sai_status_t sai_switch_set_attribute(sai_object_id_t switch_id,const sai_attribute_t *attr);


/**
 * @brief Get the switch attribute value.
 *
 * @param[in] attr_count - number of switch attributes to get
 * @param[out] attr_list - Array which holds the attributes and values
 *
 * @return SAI_STATUS_SUCCESS - on success or a failure status code on
 *         error
 */

sai_status_t sai_switch_get_attribute(sai_object_id_t switch_id,unsigned int attr_count,
                                                              sai_attribute_t *attr_list);



/**
 * @brief This API disconnects library from the initialized SDK.
 *
 */

void  sai_disconnect_switch(void);


/**
 * \}
 */

#endif
