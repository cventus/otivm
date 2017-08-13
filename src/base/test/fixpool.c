#include <stdlib.h>

#include <ok/ok.h>
#include "../include/fixpool.h"

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

static int alloc(void)
{
	struct fixpool pool;

	make_person_fixpool(&pool, 2);
	if (fixpool_alloc(&pool) == NULL) { ok = -1; }
	if (fixpool_alloc(&pool) == NULL) { ok = -2; }
	if (fixpool_alloc(&pool) != NULL) { ok = -3; }
	free_person_fixpool(&pool);
	return ok;
}

static int dealloc(void)
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

static int is_empty(void)
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

struct test const tests[] = {
	{ alloc, 	"allocate element" },
	{ dealloc, 	"free allocation" },
	{ is_empty, 	"is empty" },
	{ NULL, NULL }
};
