
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Return a `malloc`'d copy of the first `len` characters from `src`. If the
   length of `src` is less than `len`, `src` is simply copied (possibly wasting
   space. */
char *strdup_prefix(const char *src, size_t len)
{
	char *p;

	p = malloc(len + 1);
	strncpy(p, src, len);
	p[len] = '\0';
	return p;
}

char *strfmt(char *buffer, size_t buffer_size, const char *fmt, ...)
{
	va_list ap;
	int n;
	size_t size;
	char *p, *q;

	size = buffer_size ? buffer_size : 1;
	p = buffer_size && buffer ? buffer : malloc(size);
	if (!p) { return NULL; }

	/* This loop runs at most two times */
	while (1) {
		va_start(ap, fmt);
		n = vsnprintf(p, size, fmt, ap);
		va_end(ap);
		if (n < 0) { break; } /* Encoding error */
		if ((size_t)n < size) { return p; }
		size = n + 1;
		q = realloc(p == buffer ? NULL : p, size);
		if (!q) { break; }
		p = q;
	}
	if (p != buffer) { free(p); }
	return NULL;
}

