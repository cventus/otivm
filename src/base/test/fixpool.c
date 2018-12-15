#include <stdlib.h>

#include "ok/ok.h"
#include "base/fixpool.h"

struct person
{
	char const *name;
	int age;
};

static void make_person_fixpool(struct fixpool *pool, size_t n)
{
	size_t size = sizeof (struct person);
	fixpool_init(pool, calloc(n, size), n, size);
}

static void free_person_fixpool(struct fixpool *pool)
{
	free(pool->buffer);
}

int test_allocate_until_exhaustion(void)
{
	struct fixpool pool;

	make_person_fixpool(&pool, 2);
	if (fixpool_alloc(&pool) == NULL) { ok = -1; }
	if (fixpool_alloc(&pool) == NULL) { ok = -2; }
	if (fixpool_alloc(&pool) != NULL) { ok = -3; }
	free_person_fixpool(&pool);
	return ok;
}

int test_deallocated_items_can_be_allocated_again(void)
{
	struct fixpool pool;
	struct person *p, *q;

	make_person_fixpool(&pool, 2);
	p = fixpool_alloc(&pool);
	q = fixpool_alloc(&pool);
	fixpool_free(&pool, p);
	fixpool_free(&pool, q);
	if (fixpool_alloc(&pool) == NULL) { ok = -1; }
	if (fixpool_alloc(&pool) == NULL) { ok = -2; }
	if (fixpool_alloc(&pool) != NULL) { ok = -3; }
	free_person_fixpool(&pool);
	return ok;
}

int test_fixpool_is_empty_initially_and_after_deallocation(void)
{
	struct fixpool pool;
	struct person *p;

	make_person_fixpool(&pool, 1);
	if (fixpool_is_empty(&pool)) { ok = -1; }
	p = fixpool_alloc(&pool);
	if (!fixpool_is_empty(&pool)) { ok = -2; }
	fixpool_free(&pool, p);
	if (fixpool_is_empty(&pool)) { ok = -3; }
	free_person_fixpool(&pool);
	return ok;

	return ok;
}
