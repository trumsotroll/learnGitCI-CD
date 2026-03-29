#include "Unity/unity.h"
#include "../src/memory_check.h"
#include "../src/memory_check.c"

#define __attribute__(x)

void setUp(void)
{
    // chạy trước mỗi test, để trống nếu không cần
}

void tearDown(void)
{
    // chạy sau mỗi test
}

/* test vùng sram-l */
void test_below_sram_l(void)
{
    TEST_ASSERT_EQUAL_UINT32(0, is_in_ram(0x1FFF7FFF));
}

void test_sram_l_middle(void)
{
    TEST_ASSERT_EQUAL_UINT32(1, is_in_ram(0x1FFFC000));
}

void test_sram_l_start_boundary(void)
{
    TEST_ASSERT_EQUAL_UINT32(1, is_in_ram(0x1FFF8000));
}

void test_sram_l_end_boundary(void)
{
    TEST_ASSERT_EQUAL_UINT32(1, is_in_ram(0x1FFFFFFF));
}

/* test vùng sram-u */
void test_above_sram_u(void)
{
    TEST_ASSERT_EQUAL_UINT32(0, is_in_ram(0x20007001));
}

void test_sram_u_middle(void)
{
    TEST_ASSERT_EQUAL_UINT32(1, is_in_ram(0x20003000));
}

void test_sram_u_start_boundary(void)
{
    TEST_ASSERT_EQUAL_UINT32(1, is_in_ram(0x20000000));
}

void test_sram_u_end_boundary(void)
{
    TEST_ASSERT_EQUAL_UINT32(1, is_in_ram(0x20006FFF));
}

/* test vùng ngoài ram */
void test_flash_region(void)
{
	TEST_ASSERT_EQUAL_UINT32(0, is_in_ram(0x00000000));
}

void test_peripheral_region(void)
{
	TEST_ASSERT_EQUAL_UINT32(0, is_in_ram(0x40000000));
}


int main(void)
{
    UNITY_BEGIN();

    /* test sram-l */
    RUN_TEST(test_below_sram_l);
    RUN_TEST(test_sram_l_middle);
    RUN_TEST(test_sram_l_start_boundary);
    RUN_TEST(test_sram_l_end_boundary);

    /* test sram-u */
    RUN_TEST(test_above_sram_u);
    RUN_TEST(test_sram_u_middle);
    RUN_TEST(test_sram_u_start_boundary);
    RUN_TEST(test_sram_u_end_boundary);

    /* test ngoài vùng ram */
    RUN_TEST(test_flash_region);
    RUN_TEST(test_peripheral_region);

    return UNITY_END();
}
