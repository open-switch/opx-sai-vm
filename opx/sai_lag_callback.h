/************************************************************************
* * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/***
 * \file    sai_lag_callback.h
 *
 * \brief Declaration of SAI LAG related CALLBACKs
*/

#if !defined (__SAILAGCALLBACK_H_)
#define __SAILAGCALLBACK_H_

#include "saitypes.h"
#include "saistatus.h"
#include "sai_common_utils.h"

/*LAG Operation: List of operations possible on a LAG*/
typedef enum _sai_lag_operation_t {
    /*Create a LAG*/
    SAI_LAG_OPER_CREATE,
    /*Delete a LAG*/
    SAI_LAG_OPER_DELETE,
    /*Add ports to a LAG*/
    SAI_LAG_OPER_ADD_PORTS,
    /*Delete ports from a a LAG*/
    SAI_LAG_OPER_DEL_PORTS,
} sai_lag_operation_t;

/** SAI LAG CALLBACK API - LAG RIF Callback
      \param[in] lag_id LAG Identifier
      \param[in] rif_id Router interface Identifier
      \param[in] port_list List of ports
      \param[in] lag_operation Operation performed on LAG
*/
typedef sai_status_t (*sai_lag_event_callback) (
    sai_object_id_t lag_id,
    sai_lag_operation_t lag_operation,
    const sai_object_list_t *port_list
);

/*Lag Callback: List of callbacks registered with LAG Module*/
typedef struct _sai_lag_event_t {
    /*lag_callback: LAG Event callback*/
    sai_lag_event_callback lag_callback;
} sai_lag_event_t;

/** SAI LAG CALLBACK API - Register Router interface callback
      \param[in]rif_callback Router interface callback
*/
sai_status_t sai_lag_event_callback_register(sai_module_t module_id, sai_lag_event_callback lag_callback);

#endif
