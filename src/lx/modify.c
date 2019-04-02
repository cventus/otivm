#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>
#include <string.h>

#include "common.h"
#include "lx.h"
#include "memory.h"
#include "ref.h"
#include "list.h"
#include "heap.h"

static struct lxresult err_result(int err)
{
	return (struct lxresult) {
		err,
		lx_list(lx_empty_list())
	};
}

struct lxresult lx_modify(
	struct lxheap *heap,
	union lxvalue modify(struct lxmem *, union lxvalue, void *),
	void *param)
{
	struct lxmem mem;
	union lxvalue newval;
	int err;

	mem.oom = OOM_COMPACT;
	switch (setjmp(mem.escape)) {
	default: abort();
	case 0: break;

	case OOM_GROW:
		/* Memory ran out even though garbage was collected */
		err = lx_resize_heap(heap, lx_heap_size(heap)*2);
		if (err) { return err_result(err); }

	case OOM_COMPACT:
		err = lx_gc(heap);
		if (err) { return err_result(err); }
		mem.oom = OOM_GROW;
	}

	mem.alloc = heap->alloc;
	newval = modify(&mem, heap->root, param);

	heap->root = newval;
	heap->alloc = mem.alloc;

	return (struct lxresult) { 0, newval };
}
