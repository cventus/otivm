
/* Read a single token from stream
 * 
 * A token is a sequence of characters that are delimited by any character in
 * `delim`, or is a single character punctuation defined in `punct`.
 * Punctuation and delimiters separate tokens. Return length of token read. A
 * value of zero indicates end of stream.
 */
size_t read_token(char *buffer, size_t n, char const *delm, char const *punct, FILE *fp);

/* Tokenize a single string
 *
 * Space separates tokens, except when within a quote or escaped with a
 * backslash. White space is removed and tokens are moved to the beginning of
 * the line. Tokens are separated with a '\0' character and the number of
 * tokens is returned.
 */
int tokenize(char *dest, char const *src, char const *delim);

int tokenize_line(char *buffer, size_t n, int sep, FILE *fp);

