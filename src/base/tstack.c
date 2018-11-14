
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>

#include "base/wbuf.h"
#include "base/tstack.h"

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

static struct entry *get_entry(
	struct tstack *ts,
	void const *p,
	void (*dtor)(void const *, void const *))
{
	for (struct entry *q = ts->buf.begin, *e = ts->buf.end; q < e; q++) {
		if (q->p == p && q->dtor == dtor) { return q; }
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
	assert(get_entry(ts, p, dtor) == NULL);
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

int tstack_retain_mem(struct tstack *ts, void const *res)
{
	return tstack_retain(ts, &free_mem, res);
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

int tstack_retain_file(struct tstack *ts, FILE *fp)
{
	return tstack_retain(ts, &close_stream, fp);
}

static void free_wbuf(void const *p, void const *ctx)
{
	(void)ctx;
	wbuf_term((struct wbuf *)p);
}

void tstack_push_wbuf(struct tstack *ts, struct wbuf *buf)
{
	tstack_push(ts, &free_wbuf, buf, NULL);
}

int tstack_retain_wbuf(struct tstack *ts, struct wbuf *buf)
{
	return tstack_retain(ts, &free_wbuf, buf);
}

void tstack_retain_all(struct tstack *ts)
{
	assert(ts);
	wbuf_term(&ts->buf);
	wbuf_init(&ts->buf);
}

int tstack_retain(
	struct tstack *ts,
	void (*dtor)(void const *, void const *),
	void const *p)
{
	struct entry *e;
	assert(ts);
	if (p && (e = get_entry(ts, p, dtor))) {
		e->p = NULL;
		return 0;
	} else {
		return -1;
	}
}

void tstack_term(struct tstack *ts)
{
	assert(ts);
	for (struct entry *b = ts->buf.begin, *e = ts->buf.end; e-- > b; ) {
		if (e->p) { e->dtor(e->p, e->context); }
	}
	wbuf_term(&ts->buf);
}

void tstack_fail(struct tstack *ts)
{
	assert(ts);
	tstack_term(ts);
	longjmp(*ts->failjmp, -1);
}

