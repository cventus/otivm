
struct gl_state;
struct gl_test;

/* Print all `glGetError` flags to stderr, prefixed with a formatted string. */
void gl_print_errors(char const *fmt, ...);

/* Inintialize a test context with first parameter, call function, free
   context. Return value of callback function. */
int gl_run_test(char const *, int (*)(struct gl_state *, struct gl_test *));

struct gl_test *gl_make_test_context(char const *name);
void gl_free_test_context(struct gl_test *test);
struct gl_state *gl_test_get_state(struct gl_test *test);
void gl_test_swap_buffers(struct gl_test *test);
void gl_test_wait_for_key(struct gl_test *test);

int gl_enable_debug_output(struct gl_state *);
int gl_disable_debug_output(struct gl_state *);

