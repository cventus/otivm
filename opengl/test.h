
struct glstate;
struct gltest;

/* Print all `glGetError` flags to stderr, prefixed with a formatted string. */
void gl_print_errors(char const *fmt, ...);

/* Inintialize a test context with first parameter, call function, free
   context. Return value of callback function. */
int gl_run_test(char const *, int (*)(struct glstate *, struct gltest *));

struct gltest *gl_make_test_context(char const *name);
void gl_free_test_context(struct gltest *test);
struct glstate *gl_test_get_state(struct gltest *test);
void gl_test_swap_buffers(struct gltest *test);
void gl_test_wait_for_key(struct gltest *test);

