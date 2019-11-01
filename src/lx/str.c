#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>
#include <string.h>
#include <limits.h>

#include "common.h"
#include "lx.h"
#include "memory.h"
#include "ref.h"
#include "alloc.h"
#include "str.h"
#include "list.h"

size_t lx_strlen(struct lxstring string)
{
	return ((union lxcell const *)string.value.s)[-1].i;
}

/* create a string of exactly n bytes (which are not null) from src */
static struct lxstring strdup_exact(struct lxmem *mem, char const *src, size_t n)
{
	size_t free_bytes;
	char *dest;
	union lxcell *sz;

	/* check available space */
	free_bytes = alloc_free_count(&mem->alloc) * sizeof (union lxcell);
	if (n + sizeof (union lxcell) + 1 >= free_bytes) {
		longjmp(mem->escape, mem->oom);
	}

	/* allocate string */
	sz = mem->alloc.raw_free;
	dest = (char *)(mem->alloc.raw_free + 1);
	mem->alloc.raw_free += ceil_div(n + 1, sizeof (union lxcell)) + 1;

	/* copy and nul-terminate */
	memcpy(dest, src, n);
	dest[n] = 0;
	sz->i = n;

	return (struct lxstring) {
		.value.tag = lx_string_tag,
		.value.s = dest,
	};
}

struct lxstring lx_strdup(struct lxmem *mem, char const *src)
{
	return strdup_exact(mem, src, strlen(src));
}

struct lxstring lx_strndup(struct lxmem *mem, char const *src, size_t n)
{
	char const *p;

	/* look for early NUL-terminator */
	p = memchr(src, 0, n);
	if (p) {
		return strdup_exact(mem, src, p - src);
	} else {
		return strdup_exact(mem, src, n);
	}
}

struct lxstring lx_vsprintf(struct lxmem *mem, char const *fmt, va_list ap)
{
	size_t cells, free_bytes;
	union lxcell *sz = mem->alloc.raw_free;
	char *dest = NULL;
	int n;

	cells = alloc_free_count(&mem->alloc);
	if (cells > 0) {
		dest = (char *)(mem->alloc.raw_free + 1);
		/* reserve one cell for length */
		cells--;
	}
	free_bytes = cells * sizeof (union lxcell);
	n = vsnprintf(dest, free_bytes, fmt, ap);
	if (n < 0) {
		abort(); // FIXME
	} else if ((size_t)n >= free_bytes) {
		// FIXME: set resize hint based on "n"?
		longjmp(mem->escape, mem->oom);
	}
	sz->i = n;

	mem->alloc.raw_free += ceil_div(n + 1, sizeof (union lxcell)) + 1;

	return (struct lxstring) {
		.value.tag = lx_string_tag,
		.value.s = dest,
	};
}

struct lxstring lx_sprintf(struct lxmem *mem, char const *fmt, ...)
{
	va_list ap;
	struct lxstring result;

	va_start(ap, fmt);
	result = lx_vsprintf(mem, fmt, ap);
	va_end(ap);

	return result;
}
