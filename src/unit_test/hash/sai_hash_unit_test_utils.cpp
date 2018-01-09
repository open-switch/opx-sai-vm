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
/**
* @file  sai_hash_unit_test_utils.cpp
*
* @brief This file contains utility and helper function definitions for
*        testing the SAI Hash functionality.
*
*************************************************************************/

#include "gtest/gtest.h"

#include "sai_hash_unit_test.h"
#include "sai_udf_unit_test.h"

extern "C" {
#include "sai.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saiswitch.h"
#include "saihash.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>
}


/* Definition for the data members */
sai_switch_api_t* saiHashTest::p_sai_switch_api_tbl = NULL;
sai_hash_api_t* saiHashTest::p_sai_hash_api_tbl = NULL;
sai_object_id_t saiHashTest::default_lag_hash_obj = SAI_NULL_OBJECT_ID;
sai_object_id_t saiHashTest::default_ecmp_hash_obj = SAI_NULL_OBJECT_ID;

static sai_object_id_t switch_id = 0;

int32_t saiHashTest::default_native_fields[default_native_fld_count] = {
    SAI_NATIVE_HASH_FIELD_SRC_MAC, SAI_NATIVE_HASH_FIELD_DST_MAC,
    SAI_NATIVE_HASH_FIELD_IN_PORT, SAI_NATIVE_HASH_FIELD_ETHERTYPE
};

const char *saiHashTest::native_fields_str[max_native_fld_count] = {
    "SAI_NATIVE_HASH_FIELD_SRC_IP",
    "SAI_NATIVE_HASH_FIELD_DST_IP",
    "SAI_NATIVE_HASH_FIELD_INNER_SRC_IP",
    "SAI_NATIVE_HASH_FIELD_INNER_DST_IP",
    "SAI_NATIVE_HASH_FIELD_VLAN_ID",
    "SAI_NATIVE_HASH_FIELD_IP_PROTOCOL",
    "SAI_NATIVE_HASH_FIELD_ETHERTYPE",
    "SAI_NATIVE_HASH_FIELD_L4_SRC_PORT",
    "SAI_NATIVE_HASH_FIELD_L4_DST_PORT",
    "SAI_NATIVE_HASH_FIELD_SRC_MAC",
    "SAI_NATIVE_HASH_FIELD_DST_MAC",
    "SAI_NATIVE_HASH_FIELD_IN_PORT",
};

#define HASH_PRINT(msg, ...) \
    printf(msg"\n", ##__VA_ARGS__)

/*
 * Stubs for Callback functions to be passed from adapter host/application.
 */
#ifdef UNREFERENCED_PARAMETER
#elif defined(__GNUC__)
#define UNREFERENCED_PARAMETER(P)   (void)(P)
#else
#define UNREFERENCED_PARAMETER(P)   (P)
#endif

static inline void sai_port_state_evt_callback (
                                     uint32_t count,
                                     sai_port_oper_status_notification_t *data)
{
    UNREFERENCED_PARAMETER(count);
    UNREFERENCED_PARAMETER(data);
}

static inline void sai_fdb_evt_callback (uint32_t count,
                                         sai_fdb_event_notification_data_t *data)
{
    UNREFERENCED_PARAMETER(count);
    UNREFERENCED_PARAMETER(data);
}

static inline void sai_switch_operstate_callback (sai_switch_oper_status_t
                                                  switchstate)
{
    UNREFERENCED_PARAMETER(switchstate);
}

static inline void sai_packet_event_callback (const void *buffer,
                                              sai_size_t buffer_size,
                                              uint32_t attr_count,
                                              const sai_attribute_t *attr_list)
{
    UNREFERENCED_PARAMETER(buffer);
    UNREFERENCED_PARAMETER(buffer_size);
    UNREFERENCED_PARAMETER(attr_count);
    UNREFERENCED_PARAMETER(attr_list);
}

static inline void sai_switch_shutdown_callback (void)
{
}

/* SAI switch initialization */
void saiHashTest::SetUpTestCase (void)
{
    sai_attribute_t           get_attr;

    sai_attribute_t sai_attr_set[7];
    uint32_t attr_count = 7;

    memset(sai_attr_set,0, sizeof(sai_attr_set));

    /*
     * Query and populate the SAI Switch API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
               (SAI_API_SWITCH, (static_cast<void**>
                                 (static_cast<void*>(&p_sai_switch_api_tbl)))));

    ASSERT_TRUE (p_sai_switch_api_tbl != NULL);

    sai_attr_set[0].id = SAI_SWITCH_ATTR_INIT_SWITCH;
    sai_attr_set[0].value.booldata = 1;

    sai_attr_set[1].id = SAI_SWITCH_ATTR_SWITCH_PROFILE_ID;
    sai_attr_set[1].value.u32 = 0;

    sai_attr_set[2].id = SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY;
    sai_attr_set[2].value.ptr = (void *)sai_fdb_evt_callback;

    sai_attr_set[3].id = SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY;
    sai_attr_set[3].value.ptr = (void *)sai_port_state_evt_callback;

    sai_attr_set[4].id = SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY;
    sai_attr_set[4].value.ptr = (void *)sai_packet_event_callback;

    sai_attr_set[5].id = SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY;
    sai_attr_set[5].value.ptr = (void *)sai_switch_operstate_callback;

    sai_attr_set[6].id = SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY;
    sai_attr_set[6].value.ptr = (void *)sai_switch_shutdown_callback;

    ASSERT_TRUE(p_sai_switch_api_tbl->create_switch != NULL);

    EXPECT_EQ (SAI_STATUS_SUCCESS,
               (p_sai_switch_api_tbl->create_switch (&switch_id , attr_count,
                                                         sai_attr_set)));

    /* Query the Hash API method tables */
    sai_test_hash_api_table_query ();

    /* Query the UDF API method table */
    saiUdfTest::sai_test_udf_api_table_get();

    /* Get the default Hash object id values */
    get_attr.id = SAI_SWITCH_ATTR_ECMP_HASH;

    EXPECT_EQ (SAI_STATUS_SUCCESS,
               p_sai_switch_api_tbl->get_switch_attribute(switch_id,1, &get_attr));

    default_ecmp_hash_obj = get_attr.value.oid;

    get_attr.value.oid = SAI_NULL_OBJECT_ID;

    get_attr.id = SAI_SWITCH_ATTR_LAG_HASH;

    EXPECT_EQ (SAI_STATUS_SUCCESS,
               p_sai_switch_api_tbl->get_switch_attribute(switch_id,1, &get_attr));

    default_lag_hash_obj = get_attr.value.oid;
}

/* SAI Hash API Query */
void saiHashTest::sai_test_hash_api_table_query (void)
{
    /*
     * Query and populate the SAI Hash API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
               (SAI_API_HASH, (static_cast<void**>
                               (static_cast<void*>(&p_sai_hash_api_tbl)))));

    ASSERT_TRUE (p_sai_hash_api_tbl != NULL);
}

void saiHashTest::sai_test_hash_attr_value_fill (
                                         sai_attribute_t *p_attr,
                                         sai_s32_list_t *p_native_fld_list,
                                         sai_object_list_t *p_udf_group_list)
{
    if (p_attr == NULL) {
        HASH_PRINT ("%s(): Attr pointer is NULL.", __FUNCTION__);
        return;
    }

    switch (p_attr->id)
    {
        case SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST:
            if (p_native_fld_list == NULL) {

                HASH_PRINT ("%s(): Invalid native field list pointer: %p, "
                            "passed.", __FUNCTION__, p_native_fld_list);
                break;
            }

            memcpy (&p_attr->value.s32list, p_native_fld_list,
                    sizeof (sai_s32_list_t));
            break;

        case SAI_HASH_ATTR_UDF_GROUP_LIST:
            if (p_udf_group_list == NULL) {

                HASH_PRINT ("%s(): Invalid Hash group list pointer: %p, "
                            "passed.", __FUNCTION__, p_udf_group_list);
                break;
            }

            memcpy (&p_attr->value.objlist, p_udf_group_list,
                    sizeof (sai_object_list_t));
            break;

        default:
            HASH_PRINT ("Unknown Hash attribute id: %d.", p_attr->id);
            break;
    }
}

sai_status_t saiHashTest::sai_test_hash_create (sai_object_id_t *hash_id,
                                                sai_s32_list_t *native_fld_list,
                                                sai_object_list_t *udf_group_list,
                                                unsigned int attr_count, ...)
{
    sai_status_t     status = SAI_STATUS_FAILURE;
    va_list          ap;
    sai_attribute_t  attr_list [max_hash_attr_count];
    unsigned int     attr_id = 0;

    if (attr_count > max_hash_attr_count) {

        HASH_PRINT ("%s(): Attr count %u is greater than Max Hash attr count "
                    "%u.", __FUNCTION__, attr_count, max_hash_attr_count);

        return status;
    }

    memset (attr_list, 0, sizeof (attr_list));

    va_start (ap, attr_count);

    HASH_PRINT ("Testing Hash object Creation with attribute count: %u.",
                attr_count);

    for (unsigned ap_idx = 0; ap_idx < attr_count; ap_idx++) {

        attr_id = va_arg (ap, unsigned int);
        attr_list [ap_idx].id = attr_id;

        HASH_PRINT ("Setting List index: %u with Attribute Id: %d.",
                    ap_idx, attr_id);

        sai_test_hash_attr_value_fill (&attr_list [ap_idx], native_fld_list,
                                       udf_group_list);

    }

    va_end (ap);

    status = p_sai_hash_api_tbl->create_hash (hash_id, switch_id, attr_count, attr_list);

    if (status != SAI_STATUS_SUCCESS) {
        HASH_PRINT ("SAI Hash Creation API failed with error: %d.", status);
    } else {
        HASH_PRINT ("SAI Hash Creation API success, Hash Obj Id: "
                    "0x%" PRIx64 ".", *hash_id);
    }

    return status;
}

sai_status_t saiHashTest::sai_test_hash_remove (sai_object_id_t hash_id)
{
    sai_status_t  status;

    HASH_PRINT ("Testing Hash Id: 0x%" PRIx64 " remove.", hash_id);

    status = p_sai_hash_api_tbl->remove_hash (hash_id);

    if (status != SAI_STATUS_SUCCESS) {
        HASH_PRINT ("SAI Hash remove API failed with error: %d.", status);
    } else {
        HASH_PRINT ("SAI Hash remove API success.");
    }

    return status;
}

sai_status_t saiHashTest::sai_test_hash_attr_set (sai_object_id_t hash_id,
                                                sai_s32_list_t *native_fld_list,
                                                sai_object_list_t *udf_group_list,
                                                sai_attr_id_t attr_id)
{
    sai_status_t     status = SAI_STATUS_FAILURE;
    sai_attribute_t  attr_list;

    memset (&attr_list, 0, sizeof (attr_list));

    attr_list.id = attr_id;

    sai_test_hash_attr_value_fill (&attr_list, native_fld_list, udf_group_list);

    status = p_sai_hash_api_tbl->set_hash_attribute (hash_id, &attr_list);

    if (status != SAI_STATUS_SUCCESS) {
        HASH_PRINT ("SAI Hash Set Attr API failed with error: %d.", status);
    } else {
        HASH_PRINT ("SAI Hash Id: 0x%" PRIx64 " Set Attr API success.", hash_id);
    }

    return status;
}

void saiHashTest::sai_test_hash_attr_value_print (sai_attribute_t *p_attr)
{
    unsigned int index;

    if (p_attr == NULL) {
        HASH_PRINT ("%s(): Attr pointer is NULL.", __FUNCTION__);
        return;
    }

    switch (p_attr->id)
    {
        case SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST:
            HASH_PRINT ("Native Field List count: %u.",
                        p_attr->value.s32list.count);
            for (index = 0; index < p_attr->value.s32list.count; index++) {
                HASH_PRINT ("Field Id[%u]: %d[%s].", index,
                            p_attr->value.s32list.list[index],
                            (p_attr->value.s32list.list[index] < max_native_fld_count) ?
                            native_fields_str [p_attr->value.s32list.list[index]]
                            : "Unknown");
            }
            break;

        case SAI_HASH_ATTR_UDF_GROUP_LIST:
            HASH_PRINT ("UDF Group List count: %u.",
                        p_attr->value.objlist.count);
            for (index = 0; index < p_attr->value.objlist.count; index++) {
                HASH_PRINT ("UDF Group Id[%u]: 0x%" PRIx64 ".", index,
                            p_attr->value.objlist.list[index]);
            }
            break;

        default:
            HASH_PRINT ("Unknown Hash attribute id: %d.", p_attr->id);
            break;
    }

}

sai_status_t saiHashTest::sai_test_hash_attr_get (sai_object_id_t hash_id,
                                                  sai_attribute_t *p_attr_list,
                                                  unsigned int attr_count, ...)
{
    sai_status_t    status;
    va_list         varg_list;
    unsigned int    index;

    va_start (varg_list, attr_count);

    for (index = 0; index < attr_count; index++) {

        p_attr_list [index].id = va_arg (varg_list, unsigned int);
    }

    va_end (varg_list);

    status = p_sai_hash_api_tbl->get_hash_attribute (hash_id, attr_count,
                                                     p_attr_list);

    if (status != SAI_STATUS_SUCCESS) {

        HASH_PRINT ("SAI Hash Get attribute failed with error: %d.", status);

    } else {

        HASH_PRINT ("SAI Hash Group Get attribute success.");

        for (index = 0; index < attr_count; index++) {

            sai_test_hash_attr_value_print (&p_attr_list [index]);
        }
    }

    return status;
}


bool saiHashTest::sai_test_find_native_field_in_list (
                                         sai_int32_t field_id,
                                         const sai_s32_list_t *p_field_list)
{
    bool         is_found = false;
    unsigned int index;

    if (p_field_list == NULL) {
        return is_found;
    }

    for (index = 0; index < p_field_list->count; index++) {

        if (p_field_list->list [index] == field_id) {

            is_found = true;
            break;
        }
    }

    return is_found;
}

bool saiHashTest::sai_test_find_udf_grp_id_in_list (
                                       sai_object_id_t udf_grp_id,
                                       const sai_object_list_t *p_udf_grp_list)
{
    bool         is_found = false;
    unsigned int index;

    if (p_udf_grp_list == NULL) {
        return is_found;
    }

    for (index = 0; index < p_udf_grp_list->count; index++) {

        if (p_udf_grp_list->list [index] == udf_grp_id) {

            is_found = true;
            break;
        }
    }

    return is_found;
}

void saiHashTest::sai_test_hash_verify (sai_object_id_t hash_id,
                                        const sai_s32_list_t *native_fld_list,
                                        const sai_object_list_t *udf_grp_list)
{
    sai_status_t    status;
    sai_attribute_t attr_list [max_hash_attr_count];
    bool            is_found = false;
    uint32_t        max_field_id_count = 16;
    sai_object_id_t obj_list [max_field_id_count];
    sai_int32_t     field_list [max_field_id_count];
    unsigned int    udf_count =
                    ((udf_grp_list != NULL) ? udf_grp_list->count : 0);
    unsigned int    native_fld_count =
                    ((native_fld_list != NULL) ? native_fld_list->count : 0);

    memset (attr_list, 0, sizeof (attr_list));

    attr_list[0].value.s32list.count = native_fld_count;
    attr_list[0].value.s32list.list  = field_list;

    attr_list[1].value.objlist.count = udf_count;
    attr_list[1].value.objlist.list  = obj_list;

    status = sai_test_hash_attr_get (hash_id, attr_list, max_hash_attr_count,
                                     SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST,
                                     SAI_HASH_ATTR_UDF_GROUP_LIST);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    EXPECT_EQ (native_fld_count, attr_list[0].value.s32list.count);

    for (unsigned int index = 0; index < native_fld_count; index++)
    {
        is_found =  sai_test_find_native_field_in_list (
                                                 native_fld_list->list[index],
                                                 &attr_list[0].value.s32list);

        EXPECT_EQ (is_found, true);
    }

    EXPECT_EQ (udf_count, attr_list[1].value.objlist.count);

    for (unsigned int index = 0; index < udf_count; index++)
    {
        is_found =  sai_test_find_udf_grp_id_in_list (udf_grp_list->list[index],
                                                      &attr_list[1].value.objlist);

        EXPECT_EQ (is_found, true);
    }
}
