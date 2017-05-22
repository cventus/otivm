
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <assert.h>

#include "include/wbuf.h"
#include "include/mem.h"
#define MINSIZE sizeof(double)

static size_t newsize(size_t size, size_t hint)
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

void wbuf_init(struct wbuf buf[static 1])
{
	assert(buf != NULL);
	buf->begin = buf->end = buf->bound = NULL;
}

int wbuf_reserve(struct wbuf buf[static 1], size_t alloc_size)
{
	size_t size, use;
	char *p;

	assert(buf != NULL);
	if (wbuf_available(buf) < alloc_size) {
		size = wbuf_capacity(buf);
		use = wbuf_size(buf);
		do {
			size = newsize(size, alloc_size);
			if (size == 0) { return -1; }
		} while (size - use < alloc_size);
		p = realloc(buf->begin, size);
		if (!p) { return -2; }
		buf->begin = p;
		buf->end = p + use;
		buf->bound = p + size;
	}
	return 0;
}

int wbuf_trim(struct wbuf buf[static 1])
{
	assert(buf != NULL);
	size_t use = wbuf_size(buf);
	char *p = realloc(buf->begin, use);
	if (!p) { return -1; }
	buf->begin = p;
	buf->end = buf->bound = p + use;
	return 0;
}

void wbuf_term(struct wbuf buf[static 1])
{
	assert(buf != NULL);
	if (buf->begin) {
		free(buf->begin);
		wbuf_init(buf);
	}
}

size_t wbuf_nmemb(struct wbuf const buf[static 1], size_t elem_size)
{
	assert(buf != NULL);
	return wbuf_size(buf) / elem_size;
}

size_t wbuf_size(struct wbuf const buf[static 1])
{
	assert(buf != NULL);
	return buf->begin ? (char *)buf->end - (char *)buf->begin : 0;
}

size_t wbuf_available(struct wbuf const buf[static 1])
{
	assert(buf != NULL);
	return buf->begin ? (char *)buf->bound - (char *)buf->end : 0;
}

size_t wbuf_capacity(struct wbuf const buf[static 1])
{
	assert(buf != NULL);
	return buf->begin ? (char *)buf->bound - (char *)buf->begin : 0;
}

void *wbuf_get(struct wbuf buf[static 1], size_t offset)
{
	assert(buf != NULL);
	return (char *)buf->begin + offset;
}

static int do_align(
	struct wbuf *buf,
	size_t align,
	void *alloc(struct wbuf *, size_t))
{
	size_t off, pad;
	assert(buf != NULL);
	assert(is_power_of_2(align));
	if (align == 1 || buf->begin == NULL) return 0;
	off = wbuf_size(buf);
	pad = align_to(off, align) - off;
	return alloc(buf, pad) ? 0 : -1;
}

int wbuf_align(struct wbuf buf[static 1], size_t align)
{
	return do_align(buf, align, wbuf_alloc);
}

void *wbuf_alloc(struct wbuf buf[static 1], size_t size)
{
	void *result;

	assert(buf != NULL);
	if (wbuf_reserve(buf, size)) {
		result = NULL;
	} else {
		result = buf->end;
		buf->end = (char*)buf->end + size;
	}
	return result;
}

void *wbuf_write(struct wbuf buf[static 1], void const *data, size_t size)
{
	assert(buf != NULL);
	void *result = wbuf_alloc(buf, size);
	if (result) { (void)memcpy(result, data, size); }
	return result;
}

int wbuf_salign(struct wbuf buf[static 1], size_t align)
{
	assert(buf != NULL);
	return do_align(buf, align, wbuf_salloc);
}

void *wbuf_salloc(struct wbuf buf[static 1], size_t size)
{
	void *result;

	assert(buf != NULL);
	if (wbuf_available(buf) < size) { return NULL; }
	result = buf->end;
	buf->end = (char*)buf->end + size;
	return result;
}

void *wbuf_swrite(struct wbuf buf[static 1], void const *data, size_t size)
{
	assert(buf != NULL);
	void *result = wbuf_salloc(buf, size);
	if (result) { (void)memcpy(result, data, size); }
	return result;
}

int wbuf_retract(struct wbuf buf[static 1], size_t size)
{
	assert(buf != NULL);
	if (size > wbuf_size(buf)) { return -1; }
	buf->end = (char *)buf->end - size;
	return 0;
}

int wbuf_pop(struct wbuf buf[static 1], void *data, size_t size)
{
	assert(buf != NULL);
	if (size > wbuf_size(buf)) { return -1; }
	(void)memmove(data, (char *)buf->end - size, size);
	return wbuf_retract(buf, size);
}

void *wbuf_concat(struct wbuf dest[static 1], struct wbuf const src[static 1])
{
	assert(src != NULL);
	assert(dest != NULL);
	return wbuf_write(dest, src->begin, wbuf_size(src));
}

void *wbuf_copy(void *dest, struct wbuf const src[static 1])
{
	return src->begin ? memcpy(dest, src->begin, wbuf_size(src)) : dest;
}
