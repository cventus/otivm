#include <stdio.h>
#include <string.h>

#include "base/wbuf.h"
#include "fs/file.h"

char *relpath(char const *base, char const *path)
{
	char const *p;
	struct wbuf buf;
	size_t pathsz, prefixsz;

	wbuf_init(&buf);
	pathsz = strlen(path) + 1;
	p = base ? strrchr(base, '/') : NULL;
	if (p) {
		prefixsz = (p - base) + 1;
		if (wbuf_reserve(&buf, prefixsz + pathsz)) { return NULL; }
		(void)wbuf_write(&buf, base, prefixsz);
		(void)wbuf_write(&buf, path, pathsz);
	} else {
		if (!wbuf_write(&buf, path, pathsz)) { return NULL; }
	}
	return buf.begin;
}

FILE *open_relative(char const *rel, char const *filename, char const *mode)
{
	FILE *fp;
	struct wbuf buf;
	char const *p, *path;
	size_t prefix, pathsize;

	wbuf_init(&buf);
	p = rel ? strrchr(rel, '/') : NULL;
	if (p) {
		prefix = (p - rel) + 1;
		pathsize = strlen(filename) + 1;
		if (wbuf_reserve(&buf, prefix + pathsize)) { return NULL; }
		(void)wbuf_write(&buf, rel, prefix);
		(void)wbuf_write(&buf, filename, pathsize);
		path = buf.begin;
	} else {
		path = filename;
	}
	fp = fopen(path, mode);
	wbuf_term(&buf);
	return fp;
}

char *read_all(FILE *fp)
{
	char buffer[BUFSIZ];
	struct wbuf buf;
	size_t n;

	wbuf_init(&buf);
	while (n = fread(buffer, 1, sizeof buffer, fp), n > 0) {
		if (!wbuf_write(&buf, buffer, n)) { goto error; }
	}
	if (ferror(fp) || !wbuf_write(&buf, "", 1)) { goto error; }
	return buf.begin;

error:	wbuf_term(&buf);
	return NULL;
}

