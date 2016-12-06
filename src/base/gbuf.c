
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <assert.h>

#include "include/gbuf.h"
#include "include/mem.h"
#define MINSIZE sizeof(double)

struct gbuf const empty_gbuf = { NULL, NULL, NULL, NULL };

static size_t newcapacity(size_t size, size_t hint)
{
	size_t nsize;

	if (size < MINSIZE) {
		nsize = (hint < MINSIZE) ? MINSIZE : hint;
	} else {
		/* Grow by 3/2 */
		nsize = size + (size >> 1);
		if (nsize < size) {
			/* Overflow - use zero to indicate error */
			nsize = (size < (size_t)-1) ? (size_t)-1 : 0;
		}
	}
	return nsize;
}

/* Get buffer offset one past the end of the gap */
static size_t gbuf_roffset(struct gbuf const *buf)
{
	assert(buf != NULL);
	return buf->lbegin ? (char *)buf->rbegin - (char *)buf->lbegin : 0;
}

/* Size of the left side; the content before the gap */
static size_t gbuf_lsize(struct gbuf const *buf)
{
	assert(buf != NULL);
	return buf->lbegin ? (char *)buf->lend - (char *)buf->lbegin : 0;
}

/* Size of the right side; the content after the gap. */
static size_t gbuf_rsize(struct gbuf const *buf)
{
	assert(buf != NULL);
	return buf->lbegin ? (char *)buf->rend - (char *)buf->rbegin : 0;
}

void init_gbuf(struct gbuf *buf)
{
	assert(buf != NULL);
	*buf = empty_gbuf;
}

int copy_gbuf(struct gbuf *dest, struct gbuf const *src)
{
	size_t cap;
	char *p;

	cap = gbuf_capacity(src);
	if (cap > 0) {
		p = malloc(cap);
		if (!p) { return -1; }
		dest->lbegin = p;
		dest->lend = p + gbuf_offset(src);
		dest->rbegin = p + gbuf_roffset(src);
		dest->rend = p + cap;
		(void)memcpy(dest->lbegin, src->lbegin, gbuf_lsize(src));
		(void)memcpy(dest->rbegin, src->rbegin, gbuf_rsize(src));
	} else {
		init_gbuf(dest);
	}
	return 0;
}

void term_gbuf(struct gbuf *buf)
{
	assert(buf != NULL);
	if (buf->lbegin) {
		free(buf->lbegin);
		init_gbuf(buf);
	}
}

static size_t min(size_t a, size_t b) { return a < b ? a : b; }

int gbuf_cmp(struct gbuf const *a, struct gbuf const *b)
{
	int res;
	size_t lsize, rest, a_lsize, a_rsize, b_lsize, b_rsize;
	char *pa, *pb;

	assert(a != NULL);
	assert(b != NULL);

	pa = a->lbegin;
	pb = b->lbegin;

	/* Compare shortest prefix/left side */
	a_lsize = gbuf_lsize(a);
	b_lsize = gbuf_lsize(b);
	lsize = min(a_lsize, b_lsize);
	if (lsize > 0) {
		res = memcmp(pa, pb, lsize);
		if (res) { return res; }
	}

	/* Compare rest of the longer prefix/left side */
	a_rsize = gbuf_rsize(a);
	b_rsize = gbuf_rsize(b);
	if (rest = a_lsize - lsize, rest > 0) {
		/* Compare remaining part of a's prefix */
		pb = b->rbegin;
		pa += lsize;
		res = memcmp(pa, pb, min(rest, b_rsize));
		if (res) { return res; } else if (b_rsize < rest) { return 1; }
		pb += rest;
		b_rsize -= rest;
		pa = a->rbegin;
	} else if (rest = b_lsize - lsize, rest > 0) {
		/* Compare remaining part of b's prefix */
		pa = a->rbegin;
		pb += lsize;
		res = memcmp(pa, pb, min(rest, a_rsize));
		if (res) { return res; } else if (a_rsize < rest) { return -1; }
		pa += rest;
		a_rsize -= rest;
		pb = b->rbegin;
	} else {
		pa = a->rbegin;
		pb = b->rbegin;
	}

	/* Compare suffixes */
	res = memcmp(pa, pb, min(a_rsize, b_rsize));
	if (res || a_rsize == b_rsize) {
		return res;
	} else if (a_rsize < b_rsize) {
		return -1;
	} else {
		return 1;
	}
}

int gbuf_reserve(struct gbuf *buf, size_t alloc_size)
{
	size_t newcap, lsize, rsize, roffset;
	char *p;

	assert(buf != NULL);
	if (gbuf_available(buf) < alloc_size) {
		newcap = gbuf_capacity(buf);
		lsize = gbuf_lsize(buf);
		rsize = gbuf_rsize(buf);
		roffset = gbuf_roffset(buf);
		do {
			newcap = newcapacity(newcap, alloc_size);
			if (newcap == 0) { return -1; }
		} while (newcap - (lsize + rsize) < alloc_size);
		p = realloc(buf->lbegin, newcap);
		if (!p) { return -2; }
		buf->lbegin = p;
		buf->lend = p + lsize;
		buf->rbegin = p + (newcap - rsize);
		buf->rend = p + newcap;
		(void)memmove(buf->rbegin, p + roffset, rsize);
	}
	return 0;
}

int gbuf_trim(struct gbuf *buf)
{
	size_t trimsize, offset;
	char *p;

	assert(buf != NULL);
	if (gbuf_available(buf) == 0) { return 0; }
	trimsize = gbuf_size(buf);
	offset = gbuf_offset(buf);
	gbuf_append(buf);
	p = realloc(buf->lbegin, trimsize);
	if (!p) {
		gbuf_move_to(buf, offset);
		return -1;
	}
	buf->lbegin = p;
	buf->lend = buf->rbegin = p + offset;
	buf->rend = p + trimsize;
	return 0;
}

void gbuf_append(struct gbuf *buf)
{
	ptrdiff_t rsize;

	assert(buf != NULL);

	rsize = gbuf_rsize(buf);
	if (rsize == 0) { return; }

	(void)memmove(buf->lend, buf->rbegin, rsize);
	buf->lend = (char *)buf->lend + rsize;
	buf->rbegin = buf->rend;
}

void gbuf_prepend(struct gbuf *buf)
{
	ptrdiff_t lsize;

	assert(buf != NULL);

	lsize = gbuf_lsize(buf);
	if (lsize == 0) { return; }

	buf->rbegin = (char *)buf->rbegin - lsize;
	buf->lend = buf->lbegin;
	(void)memmove(buf->rbegin, buf->lend, lsize);
}

size_t gbuf_size(struct gbuf const *buf)
{
	assert(buf != NULL);
	return gbuf_lsize(buf) + gbuf_rsize(buf);
}

size_t gbuf_available(struct gbuf const *buf)
{
	assert(buf != NULL);
	return buf->lbegin ? (char *)buf->rbegin - (char *)buf->lend : 0;
}

size_t gbuf_capacity(struct gbuf const *buf)
{
	assert(buf != NULL);
	return buf->lbegin ? (char *)buf->rend - (char *)buf->lbegin : 0;
}

size_t gbuf_nmemb(struct gbuf const *buf, size_t elem_size)
{
	assert(buf != NULL);
	return gbuf_size(buf) / elem_size;
}

void *gbuf_get(struct gbuf *buf, size_t offset)
{
	assert(buf != NULL);
	size_t gap = gbuf_offset(buf);
	if (offset < gap) {
		return (char *)buf->lbegin + offset;
	} else if (offset < gbuf_size(buf)){
		return (char *)buf->rbegin + (offset - gap);
	} else {
		return NULL;
	}
}

size_t gbuf_offset(struct gbuf const *buf)
{
	assert(buf != NULL);
	return gbuf_lsize(buf);
}

int gbuf_move_to(struct gbuf *buf, size_t offset)
{
	size_t gap, diff;

	assert(buf != NULL);
	gap = gbuf_offset(buf);
	if (gap > offset) {
		diff = gap - offset;
		buf->lend = (char *)buf->lend - diff;
		buf->rbegin = (char *)buf->rbegin - diff;
		(void)memmove(buf->rbegin, buf->lend, diff);
	} else if (gap < offset) {
		if (offset > gbuf_size(buf)) { return -1; } 
		diff = offset - gap;
		(void)memmove(buf->lend, buf->rbegin, diff);
		buf->rbegin = (char *)buf->rbegin + diff;
		buf->lend = (char *)buf->lend + diff;
	}
	return 0;
}

int gbuf_move_by(struct gbuf *buf, ptrdiff_t rel)
{
	assert(buf != NULL);
	return gbuf_move_to(buf, gbuf_offset(buf) + rel);
}

int gbuf_align(struct gbuf *buf, size_t align)
{
	size_t off, pad;
	assert(buf != NULL);
	assert(is_power_of_2(align));
	if (align == 1 || buf->lbegin == NULL) return 0;
	off = gbuf_offset(buf);
	pad = align_to(off, align) - off;
	return gbuf_alloc(buf, pad) ? 0 : -1;
}

void *gbuf_alloc(struct gbuf *buf, size_t size)
{
	void *result;
	assert(buf != NULL);
	if (gbuf_reserve(buf, size)) {
		result = NULL;
	} else {
		result = buf->lend;
		buf->lend = (char*)buf->lend + size;
	}
	return result;
}

void *gbuf_write(struct gbuf *buf, void const *data, size_t size)
{
	assert(buf != NULL);
	void *result = gbuf_alloc(buf, size);
	return result ? memcpy(result, data, size) : NULL;
}

int gbuf_retract(struct gbuf *buf, size_t size)
{
	assert(buf != NULL);
	if (size > gbuf_lsize(buf)) { return -1; }
	buf->lend = (char *)buf->lend - size;
	return 0;
}

int gbuf_delete(struct gbuf *buf, size_t size)
{
	assert(buf != NULL);
	if (size > gbuf_rsize(buf)) { return -1; }
	buf->rbegin = (char *)buf->rbegin + size;
	return 0;
}

void *gbuf_copy(void *dest, struct gbuf const *src)
{
	size_t lsize, rsize;
	if (src->lbegin) {
		lsize = gbuf_lsize(src);
		rsize = gbuf_rsize(src);
		(void)memcpy(dest, src->lbegin, lsize);
		(void)memcpy((char *)dest + lsize, src->rbegin, rsize);
	}
	return dest;
}

