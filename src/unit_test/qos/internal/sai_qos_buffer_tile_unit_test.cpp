/************************************************************************
* LEGALESE:   "Copyright (c) 2017, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file  sai_qos_buffer_tile_unit_test.cpp
*
* @brief This file contains tests for qos buffer having tile
*        based concept in NPU.
*************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"

extern "C" {
#include "sai_qos_common.h"
#include "sai_qos_buffer_util.h"
#include "sai_qos_unit_test_utils.h"
#include "sai.h"
#include "saibuffer.h"
#include "saiport.h"
#include "saiqueue.h"
#include <inttypes.h>
}

static sai_object_id_t switch_id =0;

#define LOG_PRINT(msg, ...) \
    printf(msg, ##__VA_ARGS__)
#define PG_ALL 255
#define QUEUE_ALL 255

unsigned int sai_total_buffer_size = 0;

typedef struct sai_buffer_pool_tiles_info_t
{
    /* shared pool is shadow register */
    unsigned int  tiles_shared_size;
    unsigned int  tiles_total_reserved_size;
    unsigned int  *tile_reserved_size;
} sai_buffer_pool_tiles_info_t;

/*
 * API query is done while running the first test case and
 * the method table is stored in sai_switch_api_table so
 * that its available for the rest of the test cases which
 * use the method table
 */
class qos_buffer : public ::testing::Test
{
    protected:
        static void SetUpTestCase()
        {
            sai_attribute_t get_attr;
            ASSERT_EQ(SAI_STATUS_SUCCESS,sai_api_query(SAI_API_SWITCH,
                        (static_cast<void**>(static_cast<void*>(&sai_switch_api_table)))));

            ASSERT_TRUE(sai_switch_api_table != NULL);

            EXPECT_TRUE(sai_switch_api_table->remove_switch != NULL);
            EXPECT_TRUE(sai_switch_api_table->set_switch_attribute != NULL);
            EXPECT_TRUE(sai_switch_api_table->get_switch_attribute != NULL);


            sai_attribute_t sai_attr_set[7];
            uint32_t attr_count = 7;

            memset(sai_attr_set,0, sizeof(sai_attr_set));

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

            ASSERT_TRUE(sai_switch_api_table->create_switch != NULL);

            EXPECT_EQ (SAI_STATUS_SUCCESS,
                    (sai_switch_api_table->create_switch (&switch_id , attr_count,
                                                          sai_attr_set)));


            ASSERT_EQ(SAI_STATUS_SUCCESS,sai_api_query(SAI_API_BUFFER,
                        (static_cast<void**>(static_cast<void*>(&sai_buffer_api_table)))));

            ASSERT_TRUE(sai_buffer_api_table != NULL);

            EXPECT_TRUE(sai_buffer_api_table->create_buffer_pool != NULL);
            EXPECT_TRUE(sai_buffer_api_table->remove_buffer_pool != NULL);
            EXPECT_TRUE(sai_buffer_api_table->set_buffer_pool_attribute != NULL);
            EXPECT_TRUE(sai_buffer_api_table->get_buffer_pool_attribute != NULL);
            EXPECT_TRUE(sai_buffer_api_table->set_ingress_priority_group_attribute != NULL);
            EXPECT_TRUE(sai_buffer_api_table->get_ingress_priority_group_attribute != NULL);
            EXPECT_TRUE(sai_buffer_api_table->create_buffer_profile != NULL);
            EXPECT_TRUE(sai_buffer_api_table->remove_buffer_profile != NULL);
            EXPECT_TRUE(sai_buffer_api_table->set_buffer_profile_attribute != NULL);
            EXPECT_TRUE(sai_buffer_api_table->get_buffer_profile_attribute != NULL);

            ASSERT_EQ(NULL,sai_api_query(SAI_API_PORT,
                        (static_cast<void**>(static_cast<void*>(&sai_port_api_table)))));

            ASSERT_TRUE(sai_port_api_table != NULL);

            EXPECT_TRUE(sai_port_api_table->set_port_attribute != NULL);
            EXPECT_TRUE(sai_port_api_table->get_port_attribute != NULL);
            EXPECT_TRUE(sai_port_api_table->get_port_stats != NULL);

            ASSERT_EQ(NULL,sai_api_query(SAI_API_QUEUE,
                        (static_cast<void**>(static_cast<void*>(&sai_queue_api_table)))));

            ASSERT_TRUE(sai_queue_api_table != NULL);
            p_sai_qos_queue_api_table = sai_queue_api_table;

            EXPECT_TRUE(sai_queue_api_table->set_queue_attribute != NULL);
            EXPECT_TRUE(sai_queue_api_table->get_queue_attribute != NULL);
            EXPECT_TRUE(sai_queue_api_table->get_queue_stats != NULL);
            EXPECT_TRUE(sai_queue_api_table->create_queue != NULL);
            EXPECT_TRUE(sai_queue_api_table->remove_queue != NULL);
            sai_attribute_t sai_port_attr;
            uint32_t * port_count = sai_qos_update_port_count();
            sai_status_t ret = SAI_STATUS_SUCCESS;

            memset (&sai_port_attr, 0, sizeof (sai_port_attr));

            sai_port_attr.id = SAI_SWITCH_ATTR_PORT_LIST;
            sai_port_attr.value.objlist.count = 256;
            sai_port_attr.value.objlist.list  = sai_qos_update_port_list();
            ret = sai_switch_api_table->get_switch_attribute(0,1,&sai_port_attr);
            *port_count = sai_port_attr.value.objlist.count;

            ASSERT_EQ(SAI_STATUS_SUCCESS,ret);

            memset(&get_attr, 0, sizeof(get_attr));
            get_attr.id = SAI_SWITCH_ATTR_TOTAL_BUFFER_SIZE;
            ASSERT_EQ(SAI_STATUS_SUCCESS,
                    sai_switch_api_table->get_switch_attribute(switch_id,1, &get_attr));
            sai_total_buffer_size = (get_attr.value.u32)*1024;

            printf("Total buffer size - %d bytes\r\n", sai_total_buffer_size);
            for (uint32_t i = 0; i < sai_qos_max_ports_get(); i++) {
                printf("port idx %d port id 0x%lx \r\n", i, sai_qos_port_id_get(i));
            }

        }

        static sai_switch_api_t* sai_switch_api_table;
        static sai_buffer_api_t* sai_buffer_api_table;
        static sai_port_api_t* sai_port_api_table;
        static sai_queue_api_t* sai_queue_api_table;

        /* MX TH+ tile0- 0,1  tile1- 8 tile2-22  tile3-30 */
        static const unsigned int test_tile0_port_index = 0;
        static const unsigned int test_tile0_port_index_1 = 1;
        static const unsigned int test_tile1_port_index = 8;
        static const unsigned int test_tile2_port_index = 22;
        static const unsigned int test_tile3_port_index = 30;
};

sai_switch_api_t* qos_buffer ::sai_switch_api_table = NULL;
sai_buffer_api_t* qos_buffer ::sai_buffer_api_table = NULL;
sai_port_api_t* qos_buffer ::sai_port_api_table = NULL;
sai_queue_api_t* qos_buffer ::sai_queue_api_table = NULL;

void sai_qos_tile_buffer_verify (sai_object_id_t pool_id,
               unsigned int tile_shared_size, int tile_total_reserved_size,
               unsigned int tile_0_reserved_size, unsigned int tile_1_reserved_size,
               unsigned int tile_2_reserved_size, unsigned int tile_3_reserved_size)
{
    sai_buffer_pool_tiles_info_t *p_tile_info = NULL;
    dn_sai_qos_buffer_pool_t     *p_pool_node = NULL;

    p_pool_node = sai_qos_buffer_pool_node_get(pool_id);

    p_tile_info = (sai_buffer_pool_tiles_info_t *)p_pool_node->hw_info;

    printf("tiles_shared_size       : %d \r\n", p_tile_info->tiles_shared_size);
    printf("tiles_total_reserved_size : %d \r\n", p_tile_info->tiles_total_reserved_size);
    printf("Reserved Size Tile0 - %d Tile1 - %d Tile2 - %d Tile3 - %d \r\n",
            p_tile_info->tile_reserved_size[0], p_tile_info->tile_reserved_size[1],
            p_tile_info->tile_reserved_size[2], p_tile_info->tile_reserved_size[3]);

//    ASSERT_EQ (tile_shared_size, p_tile_info->tiles_shared_size);
    ASSERT_EQ (tile_total_reserved_size, p_tile_info->tiles_total_reserved_size);
    ASSERT_EQ (tile_0_reserved_size, p_tile_info->tile_reserved_size[0]/4);
    ASSERT_EQ (tile_1_reserved_size, p_tile_info->tile_reserved_size[1]/4);
    ASSERT_EQ (tile_2_reserved_size, p_tile_info->tile_reserved_size[2]/4);
    ASSERT_EQ (tile_3_reserved_size, p_tile_info->tile_reserved_size[3]/4);
}

TEST_F(qos_buffer, sai_qos_buffer_profile_pg_tile_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t profile_id_1 = 0;
    sai_object_id_t profile_id_2 = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t port_id = 0;
    sai_object_id_t pg_obj[10] = {0} /*SAI_NULL_OBJECT_ID */;

    unsigned int sai_buffer_pool_test_size_1 = 10485760; // 10MB
    unsigned int sai_buffer_profile_test_size_1 = 262144;// 0.25MB
    unsigned int sai_buffer_profile_test_size_2 = 1048576;// 1MB

    printf("Total buffer size - %d bytes\r\n", sai_total_buffer_size);
    printf("sai_buffer_pool_test_size_1 - %d bytes\r\n", sai_buffer_pool_test_size_1);
    printf("sai_buffer_profile_test_size_1 - %d bytes\r\n", sai_buffer_profile_test_size_1);
    printf("sai_buffer_profile_test_size_2 - %d bytes\r\n", sai_buffer_profile_test_size_2);

    // pool 10MB in dynamic mode
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_TYPE_INGRESS,
                                  SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    // profile_id = reserved size 1MB, static - 0 hdrm - 0.25MB
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0x6b ,pool_id_1, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));

    // profile_id_1 = reserved size 1MB, static - 0 hdrm - 0.25MB
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_1,
                                  0x6b ,pool_id_1, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    // profile_id_2 = reserved size 0MB, static - 0 hdrm - 0.25MB
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_2,
                                  0x6b ,pool_id_1, 0, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               0         0MB         0      0
     * shared size            2.5MB     2.5MB     2.5MB   2.5MB
     *
     */
    sai_qos_tile_buffer_verify(pool_id_1, 10485760, 0,0,0,0,0);

    // tile 0
    port_id = sai_qos_port_id_get (test_tile0_port_index);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj[0]));
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                                               (pg_obj[0], (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_ingress_priority_group_attribute (pg_obj[0], 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id);

    ASSERT_EQ (SAI_STATUS_OBJECT_IN_USE,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = (Pool Size - (Max resevred size in pool)/sum tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1.25MB    0          0      0
     * shared size            1.25MB    1.25MB    1.25MB  1.25MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 5242880, 5242880, 1310720,0,0,0);
    printf("Apply profile on tile 1 \r\n");
    // tile 1
    port_id = sai_qos_port_id_get (test_tile1_port_index);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj[1]));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                                               (pg_obj[1], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1.25MB    1.25MB     0       0
     * shared size            0MB       0MB        0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 10485760, 1310720, 1310720,0,0);

    // tile 2
    port_id = sai_qos_port_id_get (test_tile2_port_index);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj[2]));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                                               (pg_obj[2], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1.25MB    1.25MB    1.25MB   0
     * shared size            0MB        0MB      0MB      0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 15728640, 1310720, 1310720, 1310720,0);

    // tile 3
    port_id = sai_qos_port_id_get (test_tile3_port_index);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj[3]));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                                               (pg_obj[3], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1.25MB    1.25MB    1.25MB  1.25MB
     * shared size            0MB       0MB       0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 20971520, 1310720, 1310720, 1310720, 1310720);

    // update tile 0 with 1.25MB reserved size
    port_id = sai_qos_port_id_get (test_tile0_port_index_1);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj[4]));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                                               (pg_obj[4], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2.5MB    1.25MB    1.25MB  1.25MB
     * shared size            0MB      0MB       0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 26214400, 2621440, 1310720, 1310720, 1310720);
    printf("After updating tile 0 with 2.5MB\r\n");
    printf("Modify the buffer profile \r\n");
    printf("Reduce Reserved size from 1MB to 0.25MB\r\n");
    // profile size update - reduce reserved size 1MB to 0.25MB
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    set_attr.value.u32 = sai_buffer_profile_test_size_1;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attribute (profile_id,
                                              (const sai_attribute_t *)&set_attr));
    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1MB       0.5MB     0.5MB  0.5MB
     * shared size            0MB       0MB       0MB    0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 10485760, 1048576, 524288, 524288, 524288);
    printf("Increase Reserved size to 0.75MB  xoff 0.25MB\r\n");
    // profile update - increase reserved size to 0.75MB
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    set_attr.value.u32 = sai_buffer_profile_test_size_1 * 3; // 0.75 MB
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attribute (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2MB       1MB       1MB      1MB
     * shared size            0MB       0MB     0MB    0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 20971520, 2097152, 1048576, 1048576,1048576);
    printf("Xoff size set 0.25MB to 0.5MB, reserved size 0.75MB\r\n");

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_XOFF_TH;
    set_attr.value.u32 = sai_buffer_profile_test_size_1 * 2;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attribute (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2.5MB       1.25MB  1.25MB  1.25MB
     * shared size            0MB         0MB     0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 26214400, 2621440, 1310720, 1310720, 1310720);
    printf("Xoff size set 0.5MB to 0.75MB, reserved size 0.75MB\r\n");

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_XOFF_TH;
    set_attr.value.u32 = sai_buffer_profile_test_size_1 * 3;
    ASSERT_EQ (SAI_STATUS_INSUFFICIENT_RESOURCES,
               sai_buffer_api_table->set_buffer_profile_attribute (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2.5MB       1.25MB  1.25MB  1.25MB
     * shared size            0MB         0MB     0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 26214400, 2621440, 1310720, 1310720, 1310720);
    printf("Apply profile id 1 to pg_obj on tile 0 \r\n");
    // update tile 0 port 2 with new profile id of size 1.25MB reserved size
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id_1;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                                               (pg_obj[4], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2.5MB       1.25MB  1.25MB  1.25MB
     * shared size            0MB         0MB     0MB     0MB
     *
     */
    // remove profiles

    sai_qos_tile_buffer_verify(pool_id_1, 0, 26214400, 2621440, 1310720, 1310720, 1310720);

    printf("Removeing profile from PG\r\n");
    for (int idx = 0; idx < 10 ; idx++)
    {
        set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
        set_attr.value.oid = SAI_NULL_OBJECT_ID;
        if (pg_obj[idx]) {
            ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                       (pg_obj[idx], (const sai_attribute_t *)&set_attr));
        }
    }

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               0MB       0MB       0MB     0MB
     * shared size            2.5MB     2.5MB     2.5MB   2.5MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 10485760, 0, 0, 0, 0, 0);

   // tile 0
    port_id = sai_qos_port_id_get (test_tile0_port_index);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj[0]));
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id_2;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                                               (pg_obj[0], (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_ingress_priority_group_attribute (pg_obj[0], 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id_2);

    ASSERT_EQ (SAI_STATUS_OBJECT_IN_USE,
               sai_buffer_api_table->remove_buffer_profile (profile_id_2));
    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = (Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               0.25MB    0          0      0
     * shared size            2.25MB    2.25MB    2.25MB  2.25MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 9437184, 1048576, 262144, 0, 0, 0);
    printf("Apply profile on tile 1 \r\n");
    // tile 1
    port_id = sai_qos_port_id_get (test_tile1_port_index);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj[1]));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id_2;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                                               (pg_obj[1], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               0.25MB    0.25MB     0       0
     * shared size            2MB       2MB        2MB     2MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 8388608, 2097152, 262144, 262144, 0, 0);

    printf("Removeing profile from PG\r\n");
    for (int idx = 0; idx < 10 ; idx++)
    {
        set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
        set_attr.value.oid = SAI_NULL_OBJECT_ID;
        if (pg_obj[idx]) {
            ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                       (pg_obj[idx], (const sai_attribute_t *)&set_attr));
        }
    }

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               0MB       0MB       0MB     0MB
     * shared size            2.5MB     2.5MB     2.5MB   2.5MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 10485760, 0, 0, 0, 0, 0);


    printf("After Removeing profiles from PG\r\n");

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_1));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_2));

    sai_buffer_api_table->remove_buffer_pool (pool_id_1);
}
#if 0
TEST_F(qos_buffer, sai_qos_buffer_profile_xoff_size_tile_test)
{
    sai_attribute_t set_attr;
    sai_status_t     sai_rc;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t profile_id_1 = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t port_id =  0;
    sai_object_id_t pg_obj[10] = {0} /*SAI_NULL_OBJECT_ID */;

    unsigned int sai_buffer_pool_test_size_1 = 10485760; // 10MB

    unsigned int sai_buffer_profile_test_size_1 = 262144;// 0.25MB
    unsigned int sai_buffer_profile_test_size_2 = 1048576;// 1MB
    unsigned int sai_buffer_pool_xoff_size = 2097152;// 2MB

    printf("Total buffer size - %d bytes\r\n", sai_total_buffer_size);
    printf("sai_buffer_pool_test_size_1 - %d bytes\r\n", sai_buffer_pool_test_size_1);
    printf("sai_buffer_pool_xoff_size   - %d bytes\r\n", sai_buffer_pool_xoff_size);
    printf("sai_buffer_profile_test_size_1 - %d bytes\r\n", sai_buffer_profile_test_size_1);
    printf("sai_buffer_profile_test_size_2 - %d bytes\r\n", sai_buffer_profile_test_size_2);

    // pool 10MB in dynamic mode
    sai_attribute_t attr[4];

    attr[0].id = SAI_BUFFER_POOL_ATTR_TYPE;
    attr[0].value.s32 = SAI_BUFFER_POOL_TYPE_INGRESS;

    attr[1].id = SAI_BUFFER_POOL_ATTR_SIZE;
    attr[1].value.u32 = sai_buffer_pool_test_size_1;

    attr[2].id = SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE;
    attr[2].value.s32 = SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC;

    attr[3].id = SAI_BUFFER_POOL_ATTR_XOFF_SIZE;
    attr[3].value.s32 = 0;

    sai_rc = sai_buffer_api_table->create_buffer_pool(&pool_id_1, switch_id, 4, &attr[0]);

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_rc);

    set_attr.id = SAI_BUFFER_POOL_ATTR_XOFF_SIZE;
    set_attr.value.u32 = sai_buffer_pool_xoff_size;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attribute(pool_id_1,
                                                  (const sai_attribute_t *)&set_attr));

    // profile_id = reserved size 1MB, static - 0 hdrm - 0.25MB
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0x6b ,pool_id_1,
                                  sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));

    // profile_id_1 = reserved size 1MB, static - 0 hdrm - 0.25MB
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_1,
                                  0x6b ,pool_id_1, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  sai_buffer_profile_test_size_1, sai_buffer_profile_test_size_1));
    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               0.5       0.5MB     0.5     0.5
     * shared size            0.5MB     0.5MB     0.5MB   0.5MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 2097152, 8388608, 524288, 524288, 524288, 524288);

    port_id = sai_qos_port_id_get (test_tile0_port_index);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj[0]));
    // tile 0
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                                               (pg_obj[0], (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->
               get_ingress_priority_group_attribute (pg_obj[0], 1, get_attr));

    EXPECT_EQ (get_attr[0].value.oid, profile_id);

    ASSERT_EQ (SAI_STATUS_OBJECT_IN_USE,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1.5MB     0.5       0.5     0.5
     * shared size            0MB       0MB       0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 12582912, 1572864, 524288, 524288, 524288);

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = SAI_NULL_OBJECT_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
               (pg_obj[0], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               0.5MB     0.5       0.5     0.5
     * shared size            0.5MB     0.5MB     0.5MB   0.5MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 2097152, 8388608, 524288, 524288, 524288, 524288);
    // tile 0
    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
               (pg_obj[0], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1.5MB     0.5       0.5     0.5
     * shared size            0MB       0MB       0MB     0MB
     *
     */
    sai_qos_tile_buffer_verify(pool_id_1, 0, 12582912, 1572864, 524288, 524288, 524288);

    printf("Apply profile on tile 1\r\n");

    // tile 1
    port_id = sai_qos_port_id_get (test_tile1_port_index);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj[1]));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                                               (pg_obj[1], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1.5MB    1.5MB       0.5    0.5
     * shared size            0MB      0MB         0MB    0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 16777216, 1572864, 1572864, 524288, 524288);

    // tile 2
    port_id = sai_qos_port_id_get (test_tile2_port_index);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj[2]));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                                               (pg_obj[2], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1.5MB    1.5MB       1.5    0.5
     * shared size            0MB      0MB         0MB    0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 20971520, 1572864, 1572864, 1572864, 524288);

    // tile 3
    port_id = sai_qos_port_id_get (test_tile3_port_index);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj[3]));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                                               (pg_obj[3], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1.5MB    1.5MB       1.5    1.5
     * shared size            0MB      0MB         0MB    0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 25165824, 1572864, 1572864, 1572864, 1572864);

    // update tile 0 with 1MB reserved size
    port_id = sai_qos_port_id_get (test_tile0_port_index_1);

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_pg(sai_port_api_table, port_id, &pg_obj[4]));

    set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                                               (pg_obj[4], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2.5MB     1.5MB     1.5MB     1.5MB
     * shared size            0MB      0MB       0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 29360128, 2621440, 1572864, 1572864, 1572864);

    printf("Reduce XOFF_SIZE 2MB to 1MB\r\n");

    set_attr.id = SAI_BUFFER_POOL_ATTR_XOFF_SIZE;
    set_attr.value.u32 = sai_buffer_pool_xoff_size/2;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attribute(pool_id_1,
                                                  (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2.25MB    1.25MB    1.25MB  1.25MB
     * shared size            0MB       0MB       0MB     0MB
     *
     */
    sai_qos_tile_buffer_verify(pool_id_1, 0, 25165824, 2359296, 1310720, 1310720, 1310720);

    printf("Increase XOFF_SIZE 1MB to 4MB(i.e 0.75MB increases for each tile)\r\n");

    set_attr.id = SAI_BUFFER_POOL_ATTR_XOFF_SIZE;
    set_attr.value.u32 = (sai_buffer_pool_xoff_size * 2);

    ASSERT_EQ(SAI_STATUS_INSUFFICIENT_RESOURCES, sai_buffer_api_table->set_buffer_pool_attribute(pool_id_1,
                                                  (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2.25MB    1.25MB    1.25MB  1.25MB
     * shared size            0MB       0MB      0MB     0MB
     * global shared size - 4MB
     */
    sai_qos_tile_buffer_verify(pool_id_1, 0, 25165824, 2359296, 1310720, 1310720, 1310720);

    printf("Increase XOFF_SIZE 1MB to 2MB(i.e 0.25MB increases for each tile)\r\n");

    set_attr.id = SAI_BUFFER_POOL_ATTR_XOFF_SIZE;
    set_attr.value.u32 = sai_buffer_pool_xoff_size;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attribute(pool_id_1,
                                                  (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2.5MB     1.5MB      1.5MB    1.5MB
     * shared size            0MB       0MB       0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 29360128, 2621440, 1572864, 1572864, 1572864);

    printf("Increase XOFF_SIZE 2MB to 0(i.e 0.5MB release to each tile and hdrm 1.25MB )\r\n");

    set_attr.id = SAI_BUFFER_POOL_ATTR_XOFF_SIZE;
    set_attr.value.u32 = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attribute(pool_id_1,
                                                  (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2.5MB     1.25MB    1.25MB  1.25MB
     * shared size            0MB       0MB       0MB     0MB
     *
     */
    sai_qos_tile_buffer_verify(pool_id_1, 0, 26214400, 2621440, 1310720, 1310720, 1310720);

    printf("Increase XOFF_SIZE 0MB to 2MB(i.e 1.25MB release to pool and reserved  hdrm 0.5MB to tile 0)\r\n");

    set_attr.id = SAI_BUFFER_POOL_ATTR_XOFF_SIZE;
    set_attr.value.u32 = sai_buffer_pool_xoff_size;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attribute(pool_id_1,
                                                  (const sai_attribute_t *)&set_attr));


    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2.5MB     1.5MB     1.5MB   1.5MB
     * shared size            0MB       0MB       0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 29360128, 2621440, 1572864, 1572864, 1572864);
    printf("Increase xoff_th to 1MB\r\n");

    // profile size update - increase xooff_th size to 1MB
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_XOFF_TH;
    set_attr.value.u32 = sai_buffer_profile_test_size_2;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attribute (profile_id,
                                              (const sai_attribute_t *)&set_attr));
    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2.5MB     1.5MB     1.5MB   1.5MB
     * shared size            0MB       0MB       0MB     0MB
     *
     */
    sai_qos_tile_buffer_verify(pool_id_1, 0, 29360128, 2621440, 1572864, 1572864, 1572864);

    printf("Decrease XOFF_SIZE 2MB to 0\r\n");

    set_attr.id = SAI_BUFFER_POOL_ATTR_XOFF_SIZE;
    set_attr.value.u32 = 0;

    ASSERT_EQ(SAI_STATUS_INSUFFICIENT_RESOURCES, sai_buffer_api_table->set_buffer_pool_attribute(pool_id_1,
                                                  (const sai_attribute_t *)&set_attr));


    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2.5MB     1.5MB      1.5MB  1.5MB
     * shared size            0MB       0MB       0MB     0MB
     * global shared size 3MB
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 29360128, 2621440, 1572864, 1572864, 1572864);

    printf("Decrease reserverd size for buffer profile to 0.25MB\r\n");

    set_attr.id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    set_attr.value.u32 = sai_buffer_profile_test_size_1;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attribute (profile_id,
                                              (const sai_attribute_t *)&set_attr));
    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1MB       0.75MB    0.75MB  0.75MB
     * shared size            0MB       0MB       0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 13631488, 1048576, 786432, 786432, 786432);

    printf("Decrease XOFF_SIZE 2MB to 0\r\n");

    set_attr.id = SAI_BUFFER_POOL_ATTR_XOFF_SIZE;
    set_attr.value.u32 = 0;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attribute(pool_id_1,
                                                  (const sai_attribute_t *)&set_attr));


    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2.5MB     1.25MB    1.25MB  1.25MB
     * shared size            0MB       0MB       0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 26214400, 2621440, 1310720, 1310720, 1310720);

    printf("Increase XOFF_SIZE 0 to 2MB\r\n");

    set_attr.id = SAI_BUFFER_POOL_ATTR_XOFF_SIZE;
    set_attr.value.u32 = sai_buffer_pool_xoff_size;

    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attribute(pool_id_1,
                                                  (const sai_attribute_t *)&set_attr));
    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1MB       0.75MB    0.75MB  0.75MB
     * shared size            0MB       0MB       0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 13631488, 1048576, 786432, 786432, 786432);

    printf("Decrease Buffer pool size by 2MB\r\n");
    set_attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    set_attr.value.u32 = (sai_buffer_pool_test_size_1 - ((2 * sai_buffer_profile_test_size_2))); //size - 2MB
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_buffer_api_table->set_buffer_pool_attribute(pool_id_1,
                                                                             (const sai_attribute_t *)&set_attr));
    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1MB       0.75MB    0.75MB  0.75MB
     * shared size            0MB       0MB       0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 13631488, 1048576, 786432, 786432, 786432);

    printf("Removeing profile from PG\r\n");
    for (int idx = 0; idx < 10 ; idx++)
    {
        set_attr.id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE;
        set_attr.value.oid = SAI_NULL_OBJECT_ID;
        if (pg_obj[idx]) {
            ASSERT_EQ (SAI_STATUS_SUCCESS, sai_buffer_api_table->set_ingress_priority_group_attribute
                       (pg_obj[idx], (const sai_attribute_t *)&set_attr));
        }
    }

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               0.5MB     0.5MB     0.5MB    0.5MB
     * shared size            0MB       0MB       0MB    0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 8388608, 524288, 524288, 524288, 524288);
    printf("After Removeing profiles from PG\r\n");

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_1));

    printf("After Removeing profiles\r\n");

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_pool (pool_id_1));
}

TEST_F(qos_buffer, sai_qos_buffer_profile_queue_tile_test)
{
    sai_attribute_t set_attr;
    sai_attribute_t get_attr[6];
    sai_object_id_t profile_id = 0;
    sai_object_id_t profile_id_1 = 0;
    sai_object_id_t pool_id_1 = 0;
    sai_object_id_t port_id = 0;
    sai_object_id_t q_obj[10] = {0} /*SAI_NULL_OBJECT_ID */;

    unsigned int sai_buffer_pool_test_size_1 = 10485760; // 10MB

    unsigned int sai_buffer_profile_test_size_1 = 262144;// 0.25MB
    unsigned int sai_buffer_profile_test_size_2 = 1048576;// 1MB

    printf("Total buffer size - %d bytes\r\n", sai_total_buffer_size);
    printf("sai_buffer_pool_test_size_1 - %d bytes\r\n", sai_buffer_pool_test_size_1);
    printf("sai_buffer_profile_test_size_1 - %d bytes\r\n", sai_buffer_profile_test_size_1);
    printf("sai_buffer_profile_test_size_2 - %d bytes\r\n", sai_buffer_profile_test_size_2);

    // pool 10MB in dynamic mode
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_pool(sai_buffer_api_table, &pool_id_1,
                                  sai_buffer_pool_test_size_1, SAI_BUFFER_POOL_TYPE_EGRESS,
                                  SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC));

    // profile_id = reserved size 1MB, static - 0
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id,
                                  0xb ,pool_id_1, sai_buffer_profile_test_size_2, 0, 1, 0, 0, 0));

    // profile_id_1 = reserved size 1MB, static - 0
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_create_buffer_profile(sai_buffer_api_table, &profile_id_1,
                                  0xb ,pool_id_1, sai_buffer_profile_test_size_2, 0, 1, 0,
                                  0, 0));
    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               0         0MB         0      0
     * shared size            2.5MB     2.5MB     2.5MB   2.5MB
     *
     */
    sai_qos_tile_buffer_verify(pool_id_1, 10485760, 0,0,0,0,0);
    // tile 0
    port_id = sai_qos_port_id_get (test_tile0_port_index);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id, &q_obj[0]));
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
               (q_obj[0], (const sai_attribute_t *)&set_attr));

    memset(get_attr, 0, sizeof(get_attr));
    get_attr[0].id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->get_queue_attribute (
                                                     q_obj[0], 1,  get_attr));
    EXPECT_EQ (get_attr[0].value.oid, profile_id);

    ASSERT_EQ (SAI_STATUS_OBJECT_IN_USE,
               sai_buffer_api_table->remove_buffer_profile (profile_id));
    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1MB       0          0      0
     * shared size            1.5MB     1.5MB     1.5MB   1.5MB
     */

    sai_qos_tile_buffer_verify(pool_id_1, 6291456, 4194304, 1048576,0,0,0);
    printf("Apply profile on tile 1 \r\n");
    // tile 1
    port_id = sai_qos_port_id_get (test_tile1_port_index);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id, &q_obj[1]));
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
               (q_obj[1], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1MB       1MB       0       0
     * shared size            0.5MB     0.5MB     0.5MB   0.5MB
     */

    sai_qos_tile_buffer_verify(pool_id_1, 2097152, 8388608, 1048576, 1048576,0,0);
    // tile 2

    port_id = sai_qos_port_id_get (test_tile2_port_index);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id, &q_obj[2]));
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
               (q_obj[2], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1MB       1MB       1MB     0
     * shared size            0MB       0MB       0MB     0MB
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 12582912, 1048576, 1048576, 1048576,0);
    // tile 3
    port_id = sai_qos_port_id_get (test_tile3_port_index);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id, &q_obj[3]));
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id;
    ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
               (q_obj[3], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1MB       1MB       1MB     1MB
     * shared size            0MB       0MB       0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 16777216, 1048576, 1048576, 1048576, 1048576);
    // update tile 0 with 1MB reserved size
    port_id = sai_qos_port_id_get (test_tile0_port_index_1);
    ASSERT_EQ(SAI_STATUS_SUCCESS, sai_qos_buffer_get_first_queue(sai_port_api_table, port_id,
                                                                 &q_obj[4]));
    set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    set_attr.value.oid = profile_id;
     ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                (q_obj[4], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               2MB       1MB       1MB     1MB
     * shared size            0MB       0MB       0MB     0MB
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 20971520, 2097152, 1048576, 1048576, 1048576);
    printf("After updating tile 0 with 2MB\r\n");

    printf("Reduce Reserved size from 1MB to 0.25MB\r\n");
    // profile size update - reduce reserved size 1MB to 0.25MB
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    set_attr.value.u32 = sai_buffer_profile_test_size_1;
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attribute (profile_id,
                                              (const sai_attribute_t *)&set_attr));
    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               0.5MB     0.25MB    0.25MB  0.25MB
     * shared size            1.25MB    1.25MB    1.25MB  1.25MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 5242880, 5242880, 524288, 262144, 262144, 262144);
    printf("Increase Reserved size to 0.75MB \r\n");
    // profile update - increase reserved size to 0.75MB
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    set_attr.value.u32 = sai_buffer_profile_test_size_1 * 3; // 0.75 MB
    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->set_buffer_profile_attribute (profile_id,
                                              (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1.5MB     0.75MB    0.75MB  0.75MB
     * shared size            0MB       0MB       0MB     0MB
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 15728640, 1572864, 786432, 786432, 786432);
    printf("Xoff size set 0.25MB to 0.5MB, reserved size 0.75MB\r\n");
    set_attr.id = SAI_BUFFER_PROFILE_ATTR_XOFF_TH;
    set_attr.value.u32 = sai_buffer_profile_test_size_1 * 2;
    ASSERT_EQ (SAI_STATUS_INVALID_ATTRIBUTE_0,
               sai_buffer_api_table->set_buffer_profile_attribute (profile_id,
                                     (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1.5MB     0.75MB    0.75MB  0.75MB
     * shared size            0MB       0MB       0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 15728640, 1572864, 786432, 786432, 786432);
    printf("Apply profile id 1 to queue on tile 0 \r\n");
    // update tile 0 with new profile id of size 1MB reserved size
     set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
     set_attr.value.oid = profile_id_1;
     ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                (q_obj[4], (const sai_attribute_t *)&set_attr));

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               1.75MB    0.75MB    0.75MB  0.75MB
     * shared size            0MB       0MB       0MB     0MB
     *
     */

    sai_qos_tile_buffer_verify(pool_id_1, 0, 16777216, 1835008, 786432, 786432, 786432);
    // remove profiles
    printf("Removeing profile from Q\r\n");
    for (int idx = 0; idx < 10 ; idx++)
    {
        set_attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
        set_attr.value.oid = SAI_NULL_OBJECT_ID;
        if (q_obj[idx]) {
            ASSERT_EQ (SAI_STATUS_SUCCESS, sai_queue_api_table->set_queue_attribute
                       (q_obj[idx], (const sai_attribute_t *)&set_attr));
        }
    }

    /*  Max resevred size in pool = 4 * sum(tiles reserved size )
     *  Shared Size per tile = ( Pool Size - (Max resevred size in pool)/max tiles.
     *                        Tile 0    Tile 1    Tile 2  Tile 3
     * Reserved               0MB       0MB       0MB     0MB
     * shared size            2.5MB     2.5MB     2.5MB   2.5MB
     *
     */

    sai_qos_tile_buffer_verify (pool_id_1, 10485760, 0, 0, 0, 0, 0);
    printf("After Removeing profiles from Q\r\n");

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id));

    ASSERT_EQ (SAI_STATUS_SUCCESS,
               sai_buffer_api_table->remove_buffer_profile (profile_id_1));

    printf("After Removeing profiles\r\n");

    sai_buffer_api_table->remove_buffer_pool (pool_id_1);
}
#endif
