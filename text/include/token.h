
/* Read a single token from stream
 * 
 * A token is a sequence of characters that are delimited by any character in
 * `delim`, or is a single character punctuation defined in `punct`.
 * Punctuation and delimiters separate tokens. Return length of token read. A
 * value of zero indicates end of stream.
 */
size_t read_token(
	char *buffer,
	size_t n,
	char const *delm,
	char const *punct,
	FILE *fp);

/* Parse tokens in `src` and store them sequentially in `dest`, separated by
   `sep` and return the number of tokens read. `dest` and `src` should either
   be equal or point to non overlapping regions of space, in which case `dest`
   must be at least as big as `src`. */
int tokenize(char *dest, char const *src, int sep);

int tokenize_line(char *buffer, size_t n, int sep, FILE *fp);

