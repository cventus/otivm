/* Skip the currently executing test */
void skip_test(char const *fmt, ...);

/* Mark test as unfinished for the given reason */
void todo_test(char const *fmt, ...);

/* End test early and fail the test. If `fmt` isn't NULL, a message is printed
   to the output first. */
void fail_test(char const *fmt, ...);

/* Stop all testing because continuing tests do not make sense (e.g. necessary
   external resource is unavailable). This function doesn't return. */
void bail_out(char const *fmt, ...);

/* Mark the test as interactive, and end the test immediately (as skipped with
   the provided message) in case the test isn't running in interactive mode. */
void interactive_test(char const *fmt, ...);

/* Check if the test is running in interactive mode (e.g. as specified by a
   command line flag). */
int is_test_interactive(void);

void before_tests(void);
void after_tests(void);
void before_each_test(void);
void after_each_test(void);

/* Test structure, and test array. Define this in your tests and terminate it
   with { NULL, NULL }. */
extern struct test {
	int (*fn)(void); /* test function; zero is success */
	char const *desc;
} const tests[];

/* Global flag that can override the result of a function. Always initialized
   to zero before a test is run. */
extern int ok;
