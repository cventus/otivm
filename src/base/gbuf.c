
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>
#include <limits.h>
#include <string.h>
#include <assert.h>

#include "include/gbuf.h"
#include "include/mem.h"
#define MINSIZE sizeof(double)
#define MINALIGN alignof(max_align_t)

struct gbuf const empty_gbuf = { { NULL, NULL} , { NULL, NULL } };

static size_t newcapacity(size_t size, size_t hint)
{
	size_t nsize, aligned;

	if (size < MINSIZE) {
		nsize = (hint < MINSIZE) ? MINSIZE : hint;
	} else {
		/* Grow by 3/2 */
		nsize = size + (size >> 1);
		if (nsize < size) { nsize = SIZE_MAX - MINALIGN; }
	}
	aligned = align_to(nsize, MINALIGN);
	/* check for overflow - use zero to indicate error */
	return aligned < nsize ? 0 : aligned;
}

static size_t range_size(void const *begin, void const *end)
{
	return begin ? (char *)end - (char *)begin : 0;
}

/* Get buffer offset one past the end of the gap */
static size_t gbuf_roffset(struct gbuf const *buf)
{
	assert(buf != NULL);
	return range_size(buf->begin[0], buf->begin[1]);
}

/* Size of the left side; the content before the gap */
static size_t gbuf_lsize(struct gbuf const *buf)
{
	assert(buf != NULL);
	return range_size(buf->begin[0], buf->end[0]);
}

/* Size of the right side; the content after the gap. */
static size_t gbuf_rsize(struct gbuf const *buf)
{
	assert(buf != NULL);
	return range_size(buf->begin[1], buf->end[1]);
}

/* Check if `offset+size <= gbuf_size(buf)` */
static int validate_range(struct gbuf const *buf, size_t offset, size_t size)
{
	size_t bufsz;

	assert(buf != NULL);
	bufsz = gbuf_size(buf);
	if (bufsz < offset) { return -1; }
	if (bufsz - offset < size) { return -2; }
	return 0;
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
		dest->begin[0] = p;
		dest->end[0] = p + gbuf_offset(src);
		dest->begin[1] = p + gbuf_roffset(src);
		dest->end[1] = p + cap;
		(void)memcpy(dest->begin[0], src->begin[0], gbuf_lsize(src));
		(void)memcpy(dest->begin[1], src->begin[1], gbuf_rsize(src));
	} else {
		init_gbuf(dest);
	}
	return 0;
}

void term_gbuf(struct gbuf *buf)
{
	assert(buf != NULL);
	if (buf->begin[0]) {
		free(buf->begin[0]);
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

	pa = a->begin[0];
	pb = b->begin[0];

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
		pb = b->begin[1];
		pa += lsize;
		res = memcmp(pa, pb, min(rest, b_rsize));
		if (res) { return res; } else if (b_rsize < rest) { return 1; }
		pb += rest;
		b_rsize -= rest;
		pa = a->begin[1];
	} else if (rest = b_lsize - lsize, rest > 0) {
		/* Compare remaining part of b's prefix */
		pa = a->begin[1];
		pb += lsize;
		res = memcmp(pa, pb, min(rest, a_rsize));
		if (res) { return res; } else if (a_rsize < rest) { return -1; }
		pa += rest;
		a_rsize -= rest;
		pb = b->begin[1];
	} else {
		pa = a->begin[1];
		pb = b->begin[1];
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
		p = realloc(buf->begin[0], newcap);
		if (!p) { return -2; }
		buf->begin[0] = p;
		buf->end[0] = p + lsize;
		buf->begin[1] = p + (newcap - rsize);
		buf->end[1] = p + newcap;
		(void)memmove(buf->begin[1], p + roffset, rsize);
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
	p = realloc(buf->begin[0], trimsize);
	if (!p) {
		gbuf_move_to(buf, offset);
		return -1;
	}
	buf->begin[0] = p;
	buf->end[0] = buf->begin[1] = p + offset;
	buf->end[1] = p + trimsize;
	return 0;
}

void gbuf_append(struct gbuf *buf)
{
	ptrdiff_t rsize;

	assert(buf != NULL);

	rsize = gbuf_rsize(buf);
	if (rsize == 0) { return; }

	(void)memmove(buf->end[0], buf->begin[1], rsize);
	buf->end[0] = (char *)buf->end[0] + rsize;
	buf->begin[1] = buf->end[1];
}

void gbuf_prepend(struct gbuf *buf)
{
	ptrdiff_t lsize;

	assert(buf != NULL);

	lsize = gbuf_lsize(buf);
	if (lsize == 0) { return; }

	buf->begin[1] = (char *)buf->begin[1] - lsize;
	buf->end[0] = buf->begin[0];
	(void)memmove(buf->begin[1], buf->end[0], lsize);
}

size_t gbuf_size(struct gbuf const *buf)
{
	assert(buf != NULL);
	return gbuf_lsize(buf) + gbuf_rsize(buf);
}

size_t gbuf_available(struct gbuf const *buf)
{
	assert(buf != NULL);
	return buf->begin[0] ? (char *)buf->begin[1] - (char *)buf->end[0] : 0;
}

size_t gbuf_capacity(struct gbuf const *buf)
{
	assert(buf != NULL);
	return buf->begin[0] ? (char *)buf->end[1] - (char *)buf->begin[0] : 0;
}

int gbuf_isuniform(struct gbuf const *buf, size_t size, size_t align)
{
	assert(buf != NULL);
	assert(size != 0);
	assert(is_power_of_2(align));
	return (gbuf_size(buf) % size == 0) &&
		(gbuf_offset(buf) % size == 0) &&
		(gbuf_roffset(buf) % align == 0);
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
		return (char *)buf->begin[0] + offset;
	} else if (offset < gbuf_size(buf)){
		return (char *)buf->begin[1] + (offset - gap);
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
		buf->end[0] = (char *)buf->end[0] - diff;
		buf->begin[1] = (char *)buf->begin[1] - diff;
		(void)memmove(buf->begin[1], buf->end[0], diff);
	} else if (gap < offset) {
		if (offset > gbuf_size(buf)) { return -1; } 
		diff = offset - gap;
		(void)memmove(buf->end[0], buf->begin[1], diff);
		buf->begin[1] = (char *)buf->begin[1] + diff;
		buf->end[0] = (char *)buf->end[0] + diff;
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
	if (align == 1 || buf->begin[0] == NULL) return 0;
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
		result = buf->end[0];
		buf->end[0] = (char*)buf->end[0] + size;
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
	buf->end[0] = (char *)buf->end[0] - size;
	return 0;
}

int gbuf_delete(struct gbuf *buf, size_t size)
{
	assert(buf != NULL);
	if (size > gbuf_rsize(buf)) { return -1; }
	buf->begin[1] = (char *)buf->begin[1] + size;
	return 0;
}

/* Remove content from buffer range [offset, offset + size) by moving the gap,
   if necessary, and expanding the gap byte size. Return non-zero if the range
   is outside of the buffer. */
int gbuf_erase(struct gbuf *buf, size_t offset, size_t size)
{
	size_t lsize, rsize;

	if (validate_range(buf, offset, size)) { return -1; }
	if (size == 0) { return 0; } /* early exit */

	/* no risk of overflow at this point */
	if (offset + size < gbuf_offset(buf)) {
		/* move gap to the left */
		(void)gbuf_move_to(buf, offset + size);
		rsize = 0;
		lsize = size;
	} else if (offset > gbuf_offset(buf)) {
		/* move gap to the right */
		(void)gbuf_move_to(buf, offset);
		rsize = size;
		lsize = 0;
	} else {
		/* no need to move gap */
		lsize = gbuf_offset(buf) - offset;
		rsize = size - lsize;
	}
	if (lsize > 0) { (void)gbuf_retract(buf, lsize); }
	if (rsize > 0) { (void)gbuf_delete(buf, rsize); }
	return 0;
}

void *gbuf_copy(void *dest, struct gbuf const *src)
{
	size_t lsize, rsize;
	if (src->begin[0]) {
		lsize = gbuf_lsize(src);
		rsize = gbuf_rsize(src);
		(void)memcpy(dest, src->begin[0], lsize);
		(void)memcpy((char *)dest + lsize, src->begin[1], rsize);
	}
	return dest;
}

int gbuf_read(void *dest, struct gbuf *src, size_t offset, size_t size)
{
	size_t srcoff, lsize, rsize;

	if (validate_range(src, offset, size)) { return -1; }

	srcoff = gbuf_offset(src);

	lsize = (offset < srcoff) ? min(size, srcoff - offset) : 0;
	rsize = size - lsize;

	if (lsize > 0) {
		(void)memcpy(dest, (char *)src->begin[0] + offset, lsize);
	}
	if (rsize > 0) {
		size_t off = offset > srcoff ? offset - srcoff : 0;
		void *rbegin = (char *)src->begin[1] + off;
		(void)memcpy((char *)dest + lsize, rbegin, rsize);
	}
	return 0;
}
