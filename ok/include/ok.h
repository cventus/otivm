
/* Skip the currently executing test */
void skip_test(const char *message);

/* Stop all testing because continuing tests do not make sense (e.g. necessary
   external resource is unavailable) */
void bail_out(const char *message);

extern struct test {
	int (*function)(void);		/* test function; zero is success */
	const char *description;	/* single sentence description */
	int signal;			/* expected signal number or -1 */
} tests[];

extern size_t n_tests;

