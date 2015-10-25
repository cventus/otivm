
/* Skip the currently executing test */
void skip_test(const char *message);

/* Mark test as unfinished for the given reason, but continue execution */
void todo(const char *message);

/* Stop all testing because continuing tests do not make sense (e.g. necessary
   external resource is unavailable) */
void bail_out(const char *message);

extern struct test {
	/* test function; zero is success */
	int (*function)(void);

	/* single sentence description */
	const char *description;
} tests[];

extern size_t n_tests;

/* Global flag that can override the result of a function. Always initialized
   to zero before a test is run. */
extern int ok;

