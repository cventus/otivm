
/* Like `fopen(3)`, open relative path `filename` starting from the directory
   of `rel`. */
FILE *open_relative(char const *rel, char const *filename, char const *mode);

/* Read the whole contents of `fp` into a buffer allocated with `malloc(3)`.
   The caller needs to free it when it has been used. */
char *read_all(FILE *fp);

/* Append `path` to the directory of `base`. Return a newly allocated
   string with the contents. */
char *relpath(char const *base, char const *path);

