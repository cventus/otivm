#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>

#include "ok/ok.h"
#include "lx32x4.h"

int test_heap_size(void)
{
	size_t size = 4096;
	struct lxheap *heap = lx_make_heap(size, NULL);
	assert_int_eq(lx_heap_size(heap), size);
	lx_free_heap(heap);
	return 0;
}
