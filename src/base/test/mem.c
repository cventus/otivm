#include <stdio.h>
#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>

#include "ok/ok.h"
#include "base/mem.h"

static void check_po2(size_t val, bool expect)
{
	bool res = is_power_of_2(val);
	if (res != expect) {
		if (expect) {
			printf("%zu not recognized as a power of two\n", val);
		} else {
			printf("%zu is not a power of two\n", val);
		}
		ok = -1;
	}
}

static void check_align(size_t o, size_t a, size_t e)
{
	assert(is_power_of_2(a));
	size_t r = align_to(o, a);
	if (r != e) {
		printf("align_to(%zu, %zu) == %zu, expected %zu\n", o, a, r, e);
		ok = -1;
	}
}

int test_power_of_two(void)
{
	check_po2( 0, false);
	check_po2( 1, true);
	check_po2( 2, true);
	check_po2( 3, false);
	check_po2( 4, true);
	check_po2( 5, false);
	check_po2( 6, false);
	check_po2( 7, false);
	check_po2( 8, true);
	check_po2( 9, false);
	check_po2(10, false);
	check_po2(11, false);
	check_po2(12, false);
	check_po2(13, false);
	check_po2(14, false);
	check_po2(15, false);
	check_po2(16, true);
	check_po2(~(size_t)0 ^ (~(size_t)0 >> 1), true);
	check_po2((~(size_t)0 ^ (~(size_t)0 >> 1)) + 1, false);
	return ok;
}

int test_align(void)
{
	check_align(0, 4, 0);
	check_align(1, 4, 4);
	check_align(2, 4, 4);
	check_align(3, 4, 4);
	check_align(4, 4, 4);

	check_align(0, 16, 0);
	check_align(1, 16, 16);
	check_align(16, 16, 16);

	return ok;
}
