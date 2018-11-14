
/* Make a copy a of `src` that is at most `len` characters long (excluding nul
   terminator), or equally long if the length if `src` is shorter. */
char *strdup_prefix(const char *src, size_t len);

/* Format a string (as snprintf) and store it into `buffer` which is an array
   of size `buffer_size` and return `buffer`. If `buffer` is to small to fit
   the formatted str, leave buffer alone and allocate and return a new string
   large enough to store the formatted string.

   If `buffer_size` is zero, then a freshly allocated string is. Otherwise, if
   `buffer` is NULL then `buffer_size` specifies the minimum size of the
   returned buffer.

   Example:
     char buffer[80], *p, *name = ...;
     p = strfmt(buffer, sizeof buffer, "hello %s", name);
     f(p);
     if (p != buffer) { free(p); }

     p = strfmt(0, 0, "(%d, %d)", 13, 42);
     g(p);
     free(p);
*/
char *strfmt(char *buffer, size_t buffer_size, const char *fmt, ...);

