
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>

/* Try to read a single token, return zero if no token was found. */
static int next_token(char **to, char const **scan, int sep)
{
	char *p;
	char const *q;
	int quote;

	assert(scan);
	assert(*scan);
	assert(to);
	assert(*to); 

	p = *to;
	q = *scan;
	/* Skip leading white space */
	while (isspace(*q)) { q++; }

	/* Check whether there is a token to parse at all*/
	switch (q[0]) {
	case '\"': case '\\':
		/* If the string is consists of only (white space and) a
		   backslash or quote, then do not count it as a token */
		if (q[1] != '\0') break;
		q++;
		/* Fall through */

	case '\0':
		/* There's no token to parse */
		*p = '\0';
		*to = p + 1;
		*scan = q;
		return 0;

	default: break;
	}

	/* Parse the next token */
	quote = (*q == '\"') ? q++, 1 : 0;
	while (*q) {
		if (!quote && isspace(*q)) {
			/* end of token */
			*p++ = sep;
			q++;
			break;
		} else if (*q == '\\') {
			/* Copy next character without special interpretation */
			*p++ = *++q;

			/* Was it the string terminator? */
			if (*q == '\0') { break; }
		} else if (*q == '\"') {
			quote = !quote;
		} else {
			/* Default case: copy character */
			*p++ = *q;
		}

		if (*++q == '\0') { *p++ = '\0'; }
	}
	*to = p;
	*scan = q;
	return 1;
}

size_t read_token(char *buffer, size_t n, char const *delim, char const *punct, FILE *fp)
{
	int ch;
	size_t i;

	if (n == 0 || buffer == 0) { return 0; }

	do {
		ch = getc(fp);
		if (ch == EOF) { return 0; }
	} while (strchr(delim, ch));
	buffer[0] = ch;

	/* punctuation token */
	if (strchr(punct, ch)) {
		if (n > 1) { buffer[1] = '\0'; }
		return 1;
	}
	/* multi-character token */
	for (i = 0; ++i < n - 1; ) {
		ch = getc(fp);
		if (ch == EOF) { break; }
		if (strchr(punct, ch) || strchr(delim, ch)) {
			(void)ungetc(ch, fp);
			break;
		}
		buffer[i] = ch;
	}
	buffer[i] = '\0';
	return i;
}

int tokenize(char *dest, const char *src, int sep)
{
	int n_token;
	char *p;
	char const *q;

	if (src == NULL) { return 0; }
	if (dest == NULL) { return -1; }

	n_token = 0;
	p = dest;
	q = src;

	while (next_token(&p, &q, sep)) { n_token++; } 

	return n_token;
}

int tokenize_line(char *buffer, size_t n, int sep, FILE *fp)
{
	char *p;
	int whole_line, ntok;

	p = fgets(buffer, n, fp);
	if (!p) { return -1; }
	if (!*p) { return 0; }

	whole_line = (buffer[strlen(buffer) - 1] == '\n');
	ntok = tokenize(buffer, buffer, sep);
	if (!whole_line) {
		while (fgetc(fp) != '\n') { }
	}
	return ntok;
}

