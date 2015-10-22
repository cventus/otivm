#ifndef OK_H_INCLUDED
#define OK_H_INCLUDED

/* Skip the currently executing test */
void skip_test(const char *message);

/* Mark test as unfinished for the given reason */
void todo(const char *message);

/* Stop all testing because continuing tests do not make sense (e.g. necessary
   external resource is unavailable). This function doesn't return. */
void bail_out(const char *message);

extern struct test {
	int (*fn)(void); /* test function; zero is success */
	char const *desc;
} const tests[];

/* Global flag that can override the result of a function. Always initialized
   to zero before a test is run. */
extern int ok;

#endif

