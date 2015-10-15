
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>

#include "include/wbuf.h"
#include "include/tstack.h"

struct entry
{
	void (*dtor)(void const *, void const *);
	void const *p, *context;
};

void tstack_init(struct tstack *ts, jmp_buf *failjmp)
{
	assert(ts);
	assert(failjmp != NULL);
	wbuf_init(&ts->buf);
	ts->failjmp = failjmp;
}

void tstack_initn(struct tstack *ts, jmp_buf *failjmp, size_t n)
{
	tstack_init(ts, failjmp);
	if (!wbuf_reserve(&ts->buf, n * sizeof(struct entry))) {
		longjmp(*failjmp, -1);
	}
}

static struct entry *get_entry(struct tstack *ts, void const *p)
{
	for (struct entry *q = ts->buf.begin, *e = ts->buf.end; q < e; q++) {
		if (q->p == p) { return q; }
	}
	return NULL;
}

void tstack_push(
	struct tstack *ts,
	void (*dtor)(void const *, void const *),
	void const *p,
	void const *context)
{
	struct entry *e;
	assert(ts);
	assert(get_entry(ts, p) == NULL);
	if (p && dtor && (e = wbuf_alloc(&ts->buf, sizeof *e))) {
		e->dtor = dtor;
		e->p = p;
		e->context = context;
	} else {
		tstack_fail(ts);
	}
}

static void free_mem(void const *p, void const *ctx)
{
	(void)ctx;
	free((void *)p);
}

void tstack_push_mem(struct tstack *ts, void const *res)
{
	tstack_push(ts, &free_mem, res, NULL);
}

static void close_stream(void const *p, void const *ctx)
{
	(void)ctx;
	fclose((FILE *)p);
}

void tstack_push_file(struct tstack *ts, FILE *fp)
{
	tstack_push(ts, &close_stream, fp, NULL);
}

static void free_wbuf(void const *p, void const *ctx)
{
	(void)ctx;
	wbuf_free((struct wbuf *)p);
}

void tstack_push_wbuf(struct tstack *ts, struct wbuf *buf)
{
	tstack_push(ts, &free_wbuf, buf, NULL);
}

int tstack_retain(struct tstack *ts, void const *p)
{
	struct entry *e;
	assert(ts);
	if (p && (e = get_entry(ts, p))) {
		e->p = NULL;
		return 0;
	} else {
		return -1;
	}
}

void tstack_free(struct tstack *ts)
{
	assert(ts);
	for (struct entry *b = ts->buf.begin, *e = ts->buf.end; e-- > b; ) {
		if (e->p) { e->dtor(e->p, e->context); }
	}
	wbuf_free(&ts->buf);
}

void tstack_fail(struct tstack *ts)
{
	assert(ts);
	tstack_free(ts);
	longjmp(*ts->failjmp, -1);
}

