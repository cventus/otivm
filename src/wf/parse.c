
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <base/wbuf.h>
#include <base/mem.h>
#include <text/token.h>

#include "private.h"

int wf_parse_keyword(char const *kw, struct keyword const *keywords, size_t n,
                     int def)
{
	size_t i;

	for (i = 0; i < n; i++) {
		if (streq(keywords[i].name, kw)) {
			return keywords[i].token;
		}
	}
	return def;
}

size_t wf_next_token(char *buffer, size_t size, FILE *fp)
{
	size_t n;
	int ch;

again:	n = read_token(buffer, size, " \t", "#\n\\", fp);
	if (n == 1 && buffer[0] == '\\') {
		ch = getc(fp);
		if (ch == '\n') {
			/* An escaped new-line counts as white space and does
			   not break a logical line. */
			goto again; 
		} else {
			ungetc(ch, fp);
		}
	}
	return n;
}

size_t wf_next_argument(char *buffer, size_t size, FILE *fp)
{
	size_t n;

	n = wf_next_token(buffer, size, fp);
	if (n > 0 && (buffer[0] == '\n' || buffer[0] == '#')) {
		ungetc(buffer[0], fp);
		return 0;
	}
	return n;
} 

/* Advance `fp` to the next new-line character or EOF */
void wf_skip_line(FILE *fp)
{
	char buffer[100];
	while (wf_next_argument(buffer, sizeof buffer, fp) > 0) { }
}

/* Consume the new-line character, or comment at the current position. Return
   non-zero if there wasn't a new-line or comment.  */
int wf_expect_eol(FILE *fp)
{
	char c;

	if (wf_next_token(&c, 1, fp) == 0 || c == '\n') { return 0; }
	if (c == '#') {
		wf_skip_line(fp);
		if (wf_next_token(&c, 1, fp) == 0 || c == '\n') { return 0; }
	}
	return -1;
}

int wf_parse_vector(double *vec, size_t dim, FILE *fp)
{
	char *end, token[100];
	size_t i;

	/* A vector is a sequence of numbers separated by space */
	for (i = 0; i < dim; i++) {
		if (wf_next_argument(token, sizeof token, fp) == 0) {
			return -1;
		}
		vec[i] = strtod(token, &end);
		if (end == token || *end != '\0') { return -1; }
	}

	/* Ignore comments or additional data at end of line (some OBJ variants
	   include vertex colors after the vertex). */
	wf_skip_line(fp);

	return 0;
}

int wf_parse_mtlname(char *buffer, size_t n, FILE *fp)
{
	return wf_next_argument(buffer, n, fp) == 0 ? -1 : 0;
}

int wf_parse_filename(struct wbuf *buffer, FILE *fp)
{
	int len;
	char filename[4096];
	
	len = wf_next_argument(filename, sizeof filename, fp);
	if (len == 0) { return 0; }
	if (!wbuf_write(buffer, filename, len + 1)) { return -1; }
	return len + 1;
}

