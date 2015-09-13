
struct keyword
{
	char const *name;
	int token;
};

/* Check string equality (allows `p` and/or `q` to be NULL) */
static inline int streq(char const *p, char const *q)
{
        return p == q || (p && q && strcmp(p, q) == 0);
}

/* Parse a token in the format of .obj and .mtl files. */
size_t wf_next_token(char *buffer, size_t size, FILE *fp);

/* Parse a token, except a new-line or comment */
size_t wf_next_argument(char *buffer, size_t size, FILE *fp);

/* Advance `fp` to the next new-line character or EOF */
void wf_skip_line(FILE *fp);

/* Parse `dim` floating point values from the current position until the end
   of line, and the advance `fp` to the end of line. */
int wf_parse_vector(double *vec, size_t dim, FILE *fp);

/* Return token associated with `kw`, or `def` if it cannot be found. */
int wf_parse_keyword(char const *kw, struct keyword const *keywords, size_t n,
                     int def);

/* Read a single material name from the current position of the stream */
int wf_parse_mtlname(char *buffer, size_t n, FILE *fp);

int wf_parse_filename(struct wbuf *buffer, FILE *fp);

int wf_expect_eol(FILE *fp);
