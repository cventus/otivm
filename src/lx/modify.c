#include <stdarg.h>
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
	return (struct lxresult){ err, lx_empty_list().value };
}

static struct lxresult ok_result(struct lxvalue result)
{
	return (struct lxresult){ 0, result };
}

/* modifyl - Modify the root value of the heap through the callback `modify`
 * which receives an arbitrariy number of arguments in the form of a va_list.
 * The l-suffix in the name is modeled after e.g. execl(3) indicating the
 * variable arguments.
 *
 * WARNING: the implementation is an abomination of a function and it's
 * debatable whether its complexity is justified:
 *  - setjmp/longjmp (essential)
 *  - va_list, va_copy/va_end
 *  - goto
 *
 * The use of the va_list is sometimes more conventient than the void context
 * pointer since there's no need to define an ad-hoc struct to hold the
 * additional parameters. It does, however, incur some nasty constraints. Since
 * the modification function will potentially be called multiple times (due to
 * garbage colletion) a copy of the original va_list must be passed in every
 * time. A strict reading of the C language standard and my manual pages
 * suggest that each use of va_copy(3) should not only be paired with va_end(3)
 * but there can only be one syntactic pairing and it must be in the same block
 * since long ago an exotic implementation included curly braces in these
 * macros.
 */
struct lxresult lx_modifyl(
	struct lxheap *heap,
	struct lxvalue vmodify(struct lxmem *, struct lxvalue, va_list),
	...)
{
	va_list ap, ap_copy;
	struct lxmem mem;
	struct lxvalue newval;
	struct lxresult result;
	bool retry;
	int err;

	va_start(ap, vmodify);
	mem.oom = OOM_COMPACT;
	switch (setjmp(mem.escape)) {
	default: abort();
	case OOM_GROW:
		err = lx_resize_heap(heap, lx_heap_size(heap)*2);
		if (err) {
			result = err_result(err);
			retry = false; /* modified after setjmp */
			goto finish;
		}
		/* fallthrough */
	case OOM_COMPACT:
		mem.oom = OOM_GROW;
		err = lx_gc(heap);
		if (err) { result = err_result(err); }
		/* Whether retrying or not, we need to call va_end(3) on
		   ap_copy in the rare condition va_copy(3) allocates something
		   and va_end(3) is not just a no-op. At the same time, there
		   should *syntatically* only be one va_copy/va_end pair for a
		   list, so we cannot call it here but have to `goto` it :( */
		retry = err == 0;
		goto finish;
	case 0:
		do {
			retry = false;
			va_copy(ap_copy, ap);
			mem.alloc = heap->alloc;
			newval = vmodify(&mem, heap->root, ap_copy);
			result = ok_result(newval);
			heap->root = newval;
			heap->alloc = mem.alloc; /* commit new allocations */
finish:
			va_end(ap_copy);
		} while (retry);
		break;
	}
	va_end(ap);
	return result;
}

static struct lxvalue vpmodify(
	struct lxmem *mem,
	struct lxvalue root,
	va_list ap)
{
	typedef struct lxvalue f(struct lxmem *, struct lxvalue, void *);

	f *modify;
	void *param;

	modify = va_arg(ap, f *);
	param = va_arg(ap, void *);
	return modify(mem, root, param);
}

/* traditional callback+context pointer interface */
struct lxresult lx_modify(
	struct lxheap *heap,
	struct lxvalue modify(struct lxmem *, struct lxvalue, void *),
	void *param)
{
	return lx_modifyl(heap, vpmodify, modify, param);
}
