
struct gl_api;
struct gl_test;

/* Inintialize a test context with first parameter, call function, free
   context. Return value of callback function. */
int gl_run_test(char const *, int (*)(struct gl_api *, struct gl_test *));

struct gl_test *gl_test_make(char const *name);
void gl_test_free(struct gl_test *test);

struct gl_api *gl_test_api(struct gl_test *test);
void gl_test_swap_buffers(struct gl_test *test);
void gl_test_wait_for_key(struct gl_test *test);

