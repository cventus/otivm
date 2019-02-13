#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>

#include <ok/ok.h>
#include "../common.h"
#include "../lx.h"
#include "../memory.h"
#include "../mark.h"

unsigned char bitset[2];
unsigned b;

void before_each_test(void)
{
	memset(bitset, 0, sizeof bitset);
	b = 0;
}

int test_mark_member_zero_marks_one_bit(void)
{
	b = mark_bits(bitset, 0);
	assert(b == 0x00);
	assert(bitset[0] == 0x01);
	assert(bitset[1] == 0x00);
	return 0;
}

int test_mark_member_zero_twice_marks_both_bits(void)
{
	mark_bits(bitset, 0);
	b = mark_bits(bitset, 0);
	assert(b == 0x01);
	assert(bitset[0] == 0x03);
	assert(bitset[1] == 0x00);
	return 0;
}

int test_mark_member_zero_three_times_has_no_further_effect(void)
{
	mark_bits(bitset, 0);
	mark_bits(bitset, 0);
	b = mark_bits(bitset, 0);
	assert(b == 0x03);
	assert(bitset[0] == 0x03);
	assert(bitset[1] == 0x00);
	return 0;
}

int test_mark_member_one_marks_one_bit(void)
{
	b = mark_bits(bitset, 1);
	assert(b == 0x00);
	assert(bitset[0] == 0x04);
	assert(bitset[1] == 0x00);
	return 0;
}

int test_mark_member_one_twice_marks_both_bits(void)
{
	mark_bits(bitset, 1);
	b = mark_bits(bitset, 1);
	assert(b == 0x01);
	assert(bitset[0] == 0x0c);
	assert(bitset[1] == 0x00);
	return 0;
}

int test_mark_member_three_marks_one_bit(void)
{
	b = mark_bits(bitset, 3);
	assert(b == 0x00);
	assert(bitset[0] == 0x40);
	assert(bitset[1] == 0x00);
	return 0;
}

int test_member_four_is_marked_in_the_second_byte(void)
{
	b = mark_bits(bitset, 4);
	assert(b == 0x00);
	assert(bitset[0] == 0x00);
	assert(bitset[1] == 0x01);
	return 0;
}

int test_member_five_is_marked_in_the_second_byte(void)
{
	b = mark_bits(bitset, 5);
	assert(b == 0x00);
	assert(bitset[0] == 0x00);
	assert(bitset[1] == 0x04);
	return 0;
}

int test_clearing_unsets_both_bits(void)
{
	bitset[0] = 0xff;
	bitset[1] = 0xff;
	clear_bits(bitset, 2);
	assert(bitset[0] == 0xcf);
	assert(bitset[1] == 0xff);
	return 0;
}

int test_get_bits_retrieves_both_bits(void)
{
	bitset[0] = 0x00;
	bitset[1] = 0x70;
	assert(get_bits(bitset, 5) == 0x0);
	assert(get_bits(bitset, 6) == 0x3);
	assert(get_bits(bitset, 7) == 0x1);
	return 0;
}
