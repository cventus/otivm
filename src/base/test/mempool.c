#include <stdlib.h>

#include "ok/ok.h"
#include "base/wbuf.h"
#include "base/mempool.h"

struct person
{
	char const *name;
	int age;
};

static void make_person_mempool(struct mempool *pool, size_t n)
{
	size_t size = sizeof (struct person);
	mempool_init(pool, n, size);
}

int test_allocate_element(void)
{
	struct mempool pool;

	make_person_mempool(&pool, 1);
	if (mempool_alloc(&pool) == NULL) { ok = -1; }
	if (mempool_alloc(&pool) == NULL) { ok = -2; }
	if (mempool_alloc(&pool) == NULL) { ok = -3; }
	mempool_term(&pool);
	return ok;
}

int test_deallocate_elements(void)
{
	struct mempool pool;
	struct person *p, *q;

	make_person_mempool(&pool, 1);
	p = mempool_alloc(&pool);
	q = mempool_alloc(&pool);
	mempool_free(&pool, p);
	mempool_free(&pool, q);
	if (mempool_alloc(&pool) == NULL) { ok = -1; }
	if (mempool_alloc(&pool) == NULL) { ok = -2; }
	if (mempool_alloc(&pool) == NULL) { ok = -3; }
	mempool_term(&pool);
	return ok;
}

int test_freed_blocks_should_be_reused_in_fifo_order(void)
{
	struct mempool pool;
	struct person *p, *q;

	make_person_mempool(&pool, 1);
	p = mempool_alloc(&pool);
	q = mempool_alloc(&pool);

	mempool_free(&pool, p);
	if (mempool_alloc(&pool) != p) { ok = -1; }

	mempool_free(&pool, q);
	if (mempool_alloc(&pool) != q) { ok = -1; }

	mempool_term(&pool);
	return ok;
}
