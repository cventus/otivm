#ifndef OK_H_INCLUDED
#define OK_H_INCLUDED

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

extern struct test {
	int (*fn)(void); /* test function; zero is success */
	char const *desc;
} const tests[];

/* Global flag that can override the result of a function. Always initialized
   to zero before a test is run. */
extern int ok;

#endif

