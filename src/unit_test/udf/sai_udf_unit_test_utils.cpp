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
/**
* @file  sai_udf_unit_test_utils.cpp
*
* @brief This file contains utility and helper function definitions for
*        testing the SAI UDF functionalities.
*
*************************************************************************/

#include "gtest/gtest.h"

#include "sai_udf_unit_test.h"

extern "C" {
#include "sai.h"
#include "saitypes.h"
#include "saistatus.h"
#include "saiswitch.h"
#include "saiudf.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>
}

/* Definition for the data members */
sai_switch_api_t* saiUdfTest::p_sai_switch_api_tbl = NULL;
sai_udf_api_t* saiUdfTest::p_sai_udf_api_tbl = NULL;
sai_object_id_t saiUdfTest::dflt_udf_group_id = SAI_NULL_OBJECT_ID;
static sai_object_id_t switch_id = 0;

#define UDF_PRINT(msg, ...) \
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
void saiUdfTest::SetUpTestCase (void)
{
    sai_status_t              status = SAI_STATUS_FAILURE;

    /*
     * Query and populate the SAI Switch API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
               (SAI_API_SWITCH, (static_cast<void**>
                                 (static_cast<void*>(&p_sai_switch_api_tbl)))));

    ASSERT_TRUE (p_sai_switch_api_tbl != NULL);

    /*
     * Switch Initialization.
     */

    sai_attribute_t sai_attr_set[7];
    uint32_t attr_count = 7;

    memset(&sai_attr_set,0, sizeof(sai_attribute_t));

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

    /* Query the UDF API method tables */
    sai_test_udf_api_table_get ();

    /* Create a default UDF Group */
    status = sai_test_udf_group_create (&dflt_udf_group_id,
                                        dflt_udf_grp_attr_count,
                                        SAI_UDF_GROUP_ATTR_TYPE,
                                        SAI_UDF_GROUP_TYPE_GENERIC,
                                        SAI_UDF_GROUP_ATTR_LENGTH, default_udf_len);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    UDF_PRINT ("Created a default UDF Group Id: 0x%" PRIx64 " for UDF tests.",
               dflt_udf_group_id);
}

/* SAI UDF API Query */
void saiUdfTest::sai_test_udf_api_table_get (void)
{
    /*
     * Query and populate the SAI UDF API Table.
     */
    EXPECT_EQ (SAI_STATUS_SUCCESS, sai_api_query
               (SAI_API_UDF, (static_cast<void**>
                               (static_cast<void*>(&p_sai_udf_api_tbl)))));

    ASSERT_TRUE (p_sai_udf_api_tbl != NULL);
}

void saiUdfTest::sai_test_udf_group_attr_value_fill (sai_attribute_t *p_attr,
                                                     unsigned long attr_val)
{
    if (p_attr == NULL) {
        UDF_PRINT ("%s(): Attr pointer is NULL.", __FUNCTION__);
        return;
    }

    switch (p_attr->id) {
        case SAI_UDF_GROUP_ATTR_TYPE:
            p_attr->value.s32 = (int32_t) attr_val;

            UDF_PRINT ("Value s32: %d.", p_attr->value.s32);
            break;

        case SAI_UDF_GROUP_ATTR_LENGTH:
            p_attr->value.u16 = (uint16_t) attr_val;

            UDF_PRINT ("Value u16: %u.", p_attr->value.u16);
            break;

        default:
            p_attr->value.u64 = attr_val;

            UDF_PRINT ("Value: %d\r", (int)p_attr->value.u64);
            break;
    }
}

void saiUdfTest::sai_test_udf_group_attr_value_print (sai_attribute_t *p_attr)
{
    unsigned int index;

    if (p_attr == NULL) {
        UDF_PRINT ("%s(): Attr pointer is NULL.", __FUNCTION__);
        return;
    }

    switch (p_attr->id) {
        case SAI_UDF_GROUP_ATTR_TYPE:
            UDF_PRINT ("UDF Group Type: %d.", p_attr->value.s32);
            break;

        case SAI_UDF_GROUP_ATTR_LENGTH:
            UDF_PRINT ("UDF Length: %u.", p_attr->value.u16);
            break;

        case SAI_UDF_GROUP_ATTR_UDF_LIST:
            UDF_PRINT ("UDF List count: %u.", p_attr->value.objlist.count);
            for (index = 0; index < p_attr->value.objlist.count; index++) {
                UDF_PRINT ("UDF Id[%u]: 0x%" PRIx64 ".", index,
                           p_attr->value.objlist.list[index]);
            }
            break;

        default:
            UDF_PRINT ("Unknown attr value: %d\r", (int)p_attr->value.u64);
            break;
    }
}


sai_status_t saiUdfTest::sai_test_udf_group_create (
                                               sai_object_id_t *udf_group_id,
                                               unsigned int attr_count, ...)
{
    sai_status_t     status = SAI_STATUS_FAILURE;
    va_list          ap;
    sai_attribute_t  attr_list [max_udf_grp_attr_count];
    unsigned int     attr_id = 0;
    unsigned long    val = 0;

    if (attr_count > max_udf_grp_attr_count) {

        UDF_PRINT ("%s(): Attr count %u is greater than UDF Group attr count "
                   "%u.", __FUNCTION__, attr_count, max_udf_grp_attr_count);

        return status;
    }

    memset (attr_list, 0, sizeof (attr_list));

    va_start (ap, attr_count);

    UDF_PRINT ("Testing UDF Group Creation with attribute count: %u.",
               attr_count);

    for (unsigned ap_idx = 0; ap_idx < attr_count; ap_idx++) {

        attr_id = va_arg (ap, unsigned int);
        attr_list [ap_idx].id = attr_id;

        UDF_PRINT ("Setting List index: %u with Attribute Id: %d.",
                   ap_idx, attr_id);

        val = va_arg (ap, unsigned long);

        sai_test_udf_group_attr_value_fill (&attr_list [ap_idx], val);
    }

    va_end (ap);

    status = p_sai_udf_api_tbl->create_udf_group (udf_group_id, switch_id, attr_count,
                                                  attr_list);

    if (status != SAI_STATUS_SUCCESS) {
        UDF_PRINT ("SAI UDF Group Creation API failed with error: %d.", status);
    } else {
        UDF_PRINT ("SAI UDF Group Creation API success, UDF Group Id: "
                   "0x%" PRIx64 ".", *udf_group_id);
    }

    return status;
}

sai_status_t saiUdfTest::sai_test_udf_group_remove (sai_object_id_t udf_group_id)
{
    sai_status_t       status;

    UDF_PRINT ("Testing UDF Group Id: 0x%" PRIx64 " remove.", udf_group_id);

    status = p_sai_udf_api_tbl->remove_udf_group (udf_group_id);

    if (status != SAI_STATUS_SUCCESS) {
        UDF_PRINT ("SAI UDF Group remove API failed with error: %d.", status);
    } else {
        UDF_PRINT ("SAI UDF Group remove API success.");
    }

    return status;
}

sai_status_t saiUdfTest::sai_test_udf_group_get (sai_object_id_t udf_group_id,
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

    status = p_sai_udf_api_tbl->get_udf_group_attribute (udf_group_id, attr_count,
                                                         p_attr_list);

    if (status != SAI_STATUS_SUCCESS) {

        UDF_PRINT ("SAI UDF Group Get attribute failed with error: %d.", status);

    } else {

        UDF_PRINT ("SAI UDF Group Get attribute success.");

        for (index = 0; index < attr_count; index++) {

            sai_test_udf_group_attr_value_print (&p_attr_list [index]);
        }
    }

    return status;
}

/*
 * Helper function to find if a UDF is present in UDF Group's list.
 */
bool saiUdfTest::sai_test_find_udf_id_in_list (sai_object_id_t udf_id,
                                          const sai_object_list_t *p_udf_list)
{
    bool         is_found = false;
    unsigned int index;

    if (p_udf_list == NULL) {
        return is_found;
    }

    for (index = 0; index < p_udf_list->count; index++) {

        if (p_udf_list->list [index] == udf_id) {

            is_found = true;
            break;
        }
    }

    return is_found;
}

void saiUdfTest::sai_test_udf_group_verify (sai_object_id_t udf_group_id,
                                            sai_udf_group_type_t type,
                                            uint16_t udf_len,
                                            const sai_object_list_t *udf_list)
{
    sai_status_t    status;
    sai_attribute_t attr_list [max_udf_grp_attr_count];
    bool            is_found = false;
    uint32_t        max_udf_id_count = 16;
    sai_object_id_t list [max_udf_id_count];
    unsigned int    udf_count = ((udf_list != NULL) ? udf_list->count : 0);

    memset (attr_list, 0, sizeof (attr_list));

    attr_list[2].value.objlist.count = udf_count;
    attr_list[2].value.objlist.list  = list;

    status = sai_test_udf_group_get (udf_group_id, attr_list,
                                     max_udf_grp_attr_count,
                                     SAI_UDF_GROUP_ATTR_TYPE,
                                     SAI_UDF_GROUP_ATTR_LENGTH,
                                     SAI_UDF_GROUP_ATTR_UDF_LIST);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    EXPECT_EQ (type, attr_list[0].value.s32);
    EXPECT_EQ (udf_len, attr_list[1].value.u16);

    EXPECT_EQ (udf_count, attr_list[2].value.objlist.count);

    for (unsigned int index = 0; index < udf_count; index++)
    {
        is_found = sai_test_find_udf_id_in_list (udf_list->list[index],
                                                 &attr_list[2].value.objlist);

        EXPECT_EQ (is_found, true);
    }
}

sai_status_t saiUdfTest::sai_test_udf_create (sai_object_id_t *udf_id,
                                              sai_u8_list_t *hash_mask,
                                              unsigned int attr_count, ...)
{
    sai_status_t     status = SAI_STATUS_FAILURE;
    va_list          ap;
    unsigned int     ap_idx = 0;
    sai_attribute_t  attr_list [max_udf_attr_count];
    unsigned int     attr_id = 0;
    unsigned long    val = 0;

    if (attr_count > max_udf_attr_count) {

        UDF_PRINT ("%s(): Attr count %u is greater than UDF attr count "
                   "%u.", __FUNCTION__, attr_count, max_udf_attr_count);

        return status;
    }

    memset (attr_list, 0, sizeof (attr_list));

    va_start (ap, attr_count);

    UDF_PRINT ("Testing UDF Creation with attribute count: %u.",
               attr_count);

    for (ap_idx = 0; ap_idx < attr_count; ap_idx++) {

        attr_id = va_arg (ap, unsigned int);
        attr_list [ap_idx].id = attr_id;

        UDF_PRINT ("Setting List index: %u with Attribute Id: %d.",
                   ap_idx, attr_id);

        if (attr_id != SAI_UDF_ATTR_HASH_MASK) {
            val = va_arg (ap, unsigned long);
        }

        sai_test_udf_api_attr_value_fill (&attr_list [ap_idx], hash_mask, val);
    }

    va_end (ap);

    status = p_sai_udf_api_tbl->create_udf (udf_id, switch_id, attr_count, attr_list);

    if (status != SAI_STATUS_SUCCESS) {
        UDF_PRINT ("SAI UDF Creation API failed with error: %d.", status);
    } else {
        UDF_PRINT ("SAI UDF Creation API success, UDF Id: 0x%" PRIx64 ".", *udf_id);
    }

    return status;
}

sai_status_t saiUdfTest::sai_test_udf_remove (sai_object_id_t udf_id)
{
    sai_status_t  status;

    UDF_PRINT ("Testing UDF Id: 0x%" PRIx64 " remove.", udf_id);

    status = p_sai_udf_api_tbl->remove_udf (udf_id);

    if (status != SAI_STATUS_SUCCESS) {
        UDF_PRINT ("SAI UDF remove API failed with error: %d.", status);
    } else {
        UDF_PRINT ("SAI UDF remove API success.");
    }

    return status;
}

void saiUdfTest::sai_test_udf_verify (sai_object_id_t udf_id,
                                      sai_object_id_t match_id,
                                      sai_object_id_t group_id,
                                      sai_udf_base_t base, uint16_t offset)
{
    sai_status_t    status;
    const unsigned int attr_count = 4;
    sai_attribute_t attr_list [attr_count];

    memset (attr_list, 0, sizeof (attr_list));

    status = sai_test_udf_get (udf_id, attr_list, attr_count,
                               SAI_UDF_ATTR_MATCH_ID, SAI_UDF_ATTR_GROUP_ID,
                               SAI_UDF_ATTR_BASE, SAI_UDF_ATTR_OFFSET);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    EXPECT_EQ (match_id, attr_list[0].value.oid);
    EXPECT_EQ (group_id, attr_list[1].value.oid);
    EXPECT_EQ (base, attr_list[2].value.s32);
    EXPECT_EQ (offset, attr_list[3].value.u16);
}

void saiUdfTest::sai_test_udf_hash_mask_verify (sai_object_id_t udf_id,
                                                sai_u8_list_t *p_hash_mask)
{
    sai_status_t    status;
    sai_attribute_t attr_list;
    uint32_t        max_hash_mask_bytes = 64;

    if (p_hash_mask == NULL){
        UDF_PRINT ("%s(): Invalid hash mask pointer: %p passed.",
                   __FUNCTION__, p_hash_mask);
        return;
    }

    memset (&attr_list, 0, sizeof (attr_list));

    if (p_hash_mask->count > max_hash_mask_bytes) {
        max_hash_mask_bytes = p_hash_mask->count;
    }

    uint8_t  list [max_hash_mask_bytes];
    attr_list.value.u8list.count = p_hash_mask->count;
    attr_list.value.u8list.list  = list;

    status = sai_test_udf_get (udf_id, &attr_list, 1, SAI_UDF_ATTR_HASH_MASK);

    EXPECT_EQ (SAI_STATUS_SUCCESS, status);

    EXPECT_EQ (p_hash_mask->count, attr_list.value.u8list.count);

    for (unsigned int byte = 0; byte < p_hash_mask->count; byte++) {

        EXPECT_EQ (p_hash_mask->list[byte], attr_list.value.u8list.list[byte]);
    }
}

sai_status_t saiUdfTest::sai_test_udf_get (sai_object_id_t udf_id,
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

    status = p_sai_udf_api_tbl->get_udf_attribute (udf_id, attr_count,
                                                   p_attr_list);

    if (status != SAI_STATUS_SUCCESS) {

        UDF_PRINT ("SAI UDF Get attribute failed with error: %d.", status);

    } else {

        UDF_PRINT ("SAI UDF Get attribute success.");

        for (index = 0; index < attr_count; index++) {

            sai_test_udf_api_attr_value_print (&p_attr_list [index]);
        }
    }

    return status;
}

void saiUdfTest::sai_test_udf_api_attr_value_fill (sai_attribute_t *p_attr,
                                                   sai_u8_list_t *p_hash_mask,
                                                   unsigned long attr_val)
{
    if (p_attr == NULL) {
        UDF_PRINT ("%s(): Attr pointer is NULL.", __FUNCTION__);
        return;
    }

    switch (p_attr->id) {
        case SAI_UDF_ATTR_BASE:
            p_attr->value.s32 = (int32_t) attr_val;

            UDF_PRINT ("Value s32: %d.", p_attr->value.s32);
            break;

        case SAI_UDF_ATTR_MATCH_ID:
        case SAI_UDF_ATTR_GROUP_ID:
            p_attr->value.oid = (sai_object_id_t) attr_val;

            UDF_PRINT ("Value OID: 0x%" PRIx64 ".", p_attr->value.oid);
            break;

        case SAI_UDF_ATTR_OFFSET:
            p_attr->value.u16 = (uint16_t) attr_val;

            UDF_PRINT ("Value u16: %u.", p_attr->value.u16);
            break;

        case SAI_UDF_ATTR_HASH_MASK:
            if (p_hash_mask == NULL) {

                UDF_PRINT ("%s(): Invalid hash mask pointer: %p passed, "
                           "skipping attribute.", __FUNCTION__, p_hash_mask);
                break;
            }
            memcpy (&p_attr->value.u8list, p_hash_mask, sizeof (sai_u8_list_t));

            UDF_PRINT ("Value u8list.count: %u.", p_attr->value.u8list.count);
            for (unsigned int itr = 0; itr < p_attr->value.u8list.count; itr++) {
                UDF_PRINT ("Value u8list.list[%d]: 0x%x.", itr,
                           p_attr->value.u8list.list[itr]);
            }
            break;

        default:
            p_attr->value.u64 = attr_val;

            UDF_PRINT ("Value: %d\r", (int)p_attr->value.u64);
            break;
    }
}

void saiUdfTest::sai_test_udf_api_attr_value_print (sai_attribute_t *p_attr)
{
    if (p_attr == NULL) {
        UDF_PRINT ("%s(): Attr pointer is NULL.", __FUNCTION__);
        return;
    }

    switch (p_attr->id) {
        case SAI_UDF_ATTR_BASE:
            UDF_PRINT ("UDF Base value: %d.", p_attr->value.s32);
            break;

        case SAI_UDF_ATTR_MATCH_ID:
            UDF_PRINT ("UDF Match ID: 0x%" PRIx64 ".", p_attr->value.oid);
            break;

        case SAI_UDF_ATTR_GROUP_ID:
            UDF_PRINT ("UDF Group ID: 0x%" PRIx64 ".", p_attr->value.oid);
            break;

        case SAI_UDF_ATTR_OFFSET:
            UDF_PRINT ("UDF Offset: %u.", p_attr->value.u16);
            break;

        case SAI_UDF_ATTR_HASH_MASK:
            UDF_PRINT ("UDF Hash Mask count: %u.", p_attr->value.u8list.count);

            for (unsigned int itr = 0; itr < p_attr->value.u8list.count; itr++) {
                UDF_PRINT ("Hash Mask Value [%d]: 0x%x.", itr,
                           p_attr->value.u8list.list[itr]);
            }
            break;

        default:
            UDF_PRINT ("Unknown attr value: %d\r", (int)p_attr->value.u64);
            break;
    }
}

sai_status_t saiUdfTest::sai_test_udf_match_create (sai_object_id_t *match_id,
                                                    unsigned int attr_count, ...)
{
    sai_status_t     status = SAI_STATUS_FAILURE;
    va_list          ap;
    unsigned int     ap_idx = 0;
    sai_attribute_t  attr_list [max_udf_match_attr_count];
    unsigned int     attr_id = 0;
    unsigned int     field = 0;
    unsigned int     mask = 0;

    if (attr_count > max_udf_match_attr_count) {

        UDF_PRINT ("%s(): Attr count %u is greater than UDF Match attr count "
                   "%u.", __FUNCTION__, attr_count, max_udf_match_attr_count);

        return status;
    }

    memset (attr_list, 0, sizeof (attr_list));

    va_start (ap, attr_count);

    UDF_PRINT ("Testing UDF Creation with attribute count: %u.",
               attr_count);

    for (ap_idx = 0; ap_idx < attr_count; ap_idx++) {

        attr_id = va_arg (ap, unsigned int);
        attr_list [ap_idx].id = attr_id;

        UDF_PRINT ("Setting List index: %u with Attribute Id: %d.",
                   ap_idx, attr_id);

        field = va_arg (ap, unsigned int);

        if (attr_id != SAI_UDF_MATCH_ATTR_PRIORITY) {
            mask  = va_arg (ap, unsigned int);
        }

        sai_test_udf_match_attr_value_fill (&attr_list [ap_idx], field, mask);
    }

    va_end (ap);

    status = p_sai_udf_api_tbl->create_udf_match (match_id, switch_id, attr_count, attr_list);

    if (status != SAI_STATUS_SUCCESS) {
        UDF_PRINT ("SAI UDF Match Creation API failed with error: %d.", status);
    } else {
        UDF_PRINT ("SAI UDF Match Creation API success, Match Id: 0x%" PRIx64 ".",
                   *match_id);
    }

    return status;
}

sai_status_t saiUdfTest::sai_test_udf_match_remove (sai_object_id_t match_id)
{
    sai_status_t  status;

    UDF_PRINT ("Testing UDF Match Id: 0x%" PRIx64 " remove.", match_id);

    status = p_sai_udf_api_tbl->remove_udf_match (match_id);

    if (status != SAI_STATUS_SUCCESS) {
        UDF_PRINT ("SAI UDF Match remove API failed with error: %d.", status);
    } else {
        UDF_PRINT ("SAI UDF Match remove API success.");
    }

    return status;
}

void saiUdfTest::sai_test_udf_match_verify (sai_object_id_t udf_match_id,
                                            unsigned int attr_count, ...)
{
    sai_status_t    status;
    va_list         varg_list;
    sai_attribute_t attr_list [max_udf_match_attr_count];
    unsigned int    attr_id = 0;
    unsigned int    data = 0;
    unsigned int    mask = 0;

    memset (attr_list, 0, sizeof (attr_list));

    status = sai_test_udf_match_get (udf_match_id, attr_list,
                                     max_udf_match_attr_count,
                                     SAI_UDF_MATCH_ATTR_L2_TYPE,
                                     SAI_UDF_MATCH_ATTR_L3_TYPE,
                                     SAI_UDF_MATCH_ATTR_GRE_TYPE,
                                     SAI_UDF_MATCH_ATTR_PRIORITY);

    ASSERT_EQ (SAI_STATUS_SUCCESS, status);

    va_start (varg_list, attr_count);

    for (unsigned int index = 0; index < attr_count; index++) {

        attr_id = va_arg (varg_list, unsigned int);
        data    = va_arg (varg_list, unsigned int);

        switch (attr_id) {
            case SAI_UDF_MATCH_ATTR_L2_TYPE:
                mask = va_arg (varg_list, unsigned int);

                EXPECT_EQ (data, attr_list[0].value.aclfield.data.u16);
                EXPECT_EQ (mask, attr_list[0].value.aclfield.mask.u16);
                break;

            case SAI_UDF_MATCH_ATTR_L3_TYPE:
                mask = va_arg (varg_list, unsigned int);

                EXPECT_EQ (data, attr_list[1].value.aclfield.data.u16);
                EXPECT_EQ (mask, attr_list[1].value.aclfield.mask.u16);
                break;

            case SAI_UDF_MATCH_ATTR_GRE_TYPE:
                mask = va_arg (varg_list, unsigned int);

                EXPECT_EQ (data, attr_list[2].value.aclfield.data.u16);
                EXPECT_EQ (mask, attr_list[2].value.aclfield.mask.u16);
                break;

            case SAI_UDF_MATCH_ATTR_PRIORITY:
                EXPECT_EQ (data, attr_list[3].value.u8);
                break;

            default:
                break;
        }
    }

    va_end (varg_list);
}

sai_status_t saiUdfTest::sai_test_udf_match_get (sai_object_id_t udf_match_id,
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

    status = p_sai_udf_api_tbl->get_udf_match_attribute (udf_match_id, attr_count,
                                                         p_attr_list);

    if (status != SAI_STATUS_SUCCESS) {

        UDF_PRINT ("SAI UDF Match Get attribute failed with error: %d.", status);

    } else {

        UDF_PRINT ("SAI UDF Match Get attribute success.");

        for (index = 0; index < attr_count; index++) {

            sai_test_udf_match_attr_value_print (&p_attr_list [index]);
        }
    }

    return status;
}

void saiUdfTest::sai_test_udf_match_attr_value_fill (sai_attribute_t *p_attr,
                                                     unsigned int data,
                                                     unsigned int mask)
{
    if (p_attr == NULL) {
        UDF_PRINT ("%s(): Attr pointer is NULL.", __FUNCTION__);
        return;
    }

    switch (p_attr->id) {
        case SAI_UDF_MATCH_ATTR_L2_TYPE:
        case SAI_UDF_MATCH_ATTR_GRE_TYPE:
            p_attr->value.aclfield.enable   = true;
            p_attr->value.aclfield.data.u16 = (uint16_t) data;
            p_attr->value.aclfield.mask.u16 = (uint16_t) mask;

            UDF_PRINT ("ACL field data u16: 0x%x.", p_attr->value.aclfield.data.u16);
            UDF_PRINT ("ACL field mask u16: 0x%x.", p_attr->value.aclfield.mask.u16);
            break;

        case SAI_UDF_MATCH_ATTR_L3_TYPE:
            p_attr->value.aclfield.enable   = true;
            p_attr->value.aclfield.data.u8 = (uint8_t) data;
            p_attr->value.aclfield.mask.u8 = (uint8_t) mask;

            UDF_PRINT ("ACL field data u8: 0x%x.", p_attr->value.aclfield.data.u8);
            UDF_PRINT ("ACL field mask u8: 0x%x.", p_attr->value.aclfield.mask.u8);
            break;

        case SAI_UDF_MATCH_ATTR_PRIORITY:
            p_attr->value.u8 = (uint8_t) data;

            UDF_PRINT ("Value u8: %u.", p_attr->value.u8);
            break;

        default:
            p_attr->value.u64 = (uint64_t) data;

            UDF_PRINT ("Value: %d\r", (int)p_attr->value.u64);
            break;
    }
}

void saiUdfTest::sai_test_udf_match_attr_value_print (sai_attribute_t *p_attr)
{
    if (p_attr == NULL) {
        UDF_PRINT ("%s(): Attr pointer is NULL.", __FUNCTION__);
        return;
    }

    switch (p_attr->id) {
        case SAI_UDF_MATCH_ATTR_L2_TYPE:
            UDF_PRINT ("UDF Match L2 Type data: 0x%x.",
                       p_attr->value.aclfield.data.u16);
            UDF_PRINT ("UDF Match L2 Type mask: 0x%x.",
                       p_attr->value.aclfield.mask.u16);
            break;

        case SAI_UDF_MATCH_ATTR_L3_TYPE:
            UDF_PRINT ("UDF Match L3 Type data: 0x%x.",
                       p_attr->value.aclfield.data.u16);
            UDF_PRINT ("UDF Match L3 Type mask: 0x%x.",
                       p_attr->value.aclfield.mask.u16);
            break;

        case SAI_UDF_MATCH_ATTR_GRE_TYPE:
            UDF_PRINT ("UDF Match GRE Type data: 0x%x.",
                       p_attr->value.aclfield.data.u16);
            UDF_PRINT ("UDF Match GRE Type mask: 0x%x.",
                       p_attr->value.aclfield.mask.u16);
            break;

        case SAI_UDF_MATCH_ATTR_PRIORITY:
            UDF_PRINT ("UDF Match priority: %u.", p_attr->value.u8);
            break;

        default:
            UDF_PRINT ("Unknown attr value: %d\r", (int)p_attr->value.u64);
            break;
    }
}

