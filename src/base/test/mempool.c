#include <stdlib.h>

#include <ok/ok.h>
#include "../include/wbuf.h"
#include "../include/mempool.h"

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

static int alloc(void)
{
	struct mempool pool;

	make_person_mempool(&pool, 1);
	if (mempool_alloc(&pool) == NULL) { ok = -1; }
	if (mempool_alloc(&pool) == NULL) { ok = -2; }
	if (mempool_alloc(&pool) == NULL) { ok = -3; }
	mempool_term(&pool);
	return ok;
}

static int dealloc(void)
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

static int reuse(void)
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

struct test const tests[] = {
	{ alloc, 	"allocate element" },
	{ dealloc, 	"free allocation" },
	{ reuse, 	"freed blocks are reused in FIFO order" },
	{ NULL, NULL }
};
