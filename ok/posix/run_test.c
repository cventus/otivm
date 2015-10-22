
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>

#include <ok/ok.h>

#include "private.h"
#include "check.h"

static struct state
{
	int init;
	jmp_buf skip;
	struct test_result *result;
} g_state = { 0 };

struct test_result
{
	enum { TEST = 0, FAIL, SKIP, BAIL_OUT, TODO } mode;
	int success;
	int signo;
	char message[1000];
};

static void set_message(const char *fmt, va_list ap)
{
	if (g_state.init) {
		if (fmt) {
			size_t sz = sizeof g_state.result->message;
			vsnprintf(g_state.result->message, sz, fmt, ap);
			g_state.result->message[sz - 1] = 0;
		}
	}
}

static void stop_test(int mode)
{
	if (g_state.init) { longjmp(g_state.skip, mode); }
}

void fail_test(char const *fmt, ...)
{
	va_list ap;
	if (fmt) {
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
	}
	stop_test(FAIL);
}

void todo_test(char const *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	set_message(fmt, ap);
	va_end(ap);
	if (g_state.init) { g_state.result->mode = TODO; }
}

void skip_test(char const *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	set_message(fmt, ap);
	va_end(ap);
	stop_test(SKIP);
}

void bail_out(char const *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	set_message(fmt, ap);
	va_end(ap);
	stop_test(BAIL_OUT);
}

static void do_bail_out(const char *msg)
{
	if (!msg) msg = "";
	printf("Bail out!%s%s\n", msg[0] ? " " : "", msg);
	exit(EXIT_FAILURE);
}

/* Write file contents to stdout, prefixed */
static void echo_file(int fd, const char *prefix)
{
	char buffer[BUFSIZ];
	int indent, n, i;

	indent = 1;
	CHECK(lseek(fd, 0, SEEK_SET) == 0);
	while (n = read(fd, buffer, sizeof buffer), n > 0) {
		if (indent) printf("%s", prefix);
		for (i = 0; i < n - 1; i++) {
			putchar(buffer[i]);
			if (buffer[i] == '\n') printf("%s", prefix);
		}
		putchar(buffer[n - 1]);
		indent = (buffer[n - 1] == '\n');
	}
	CHECK(n == 0);
}

static void print_directive(int directive, const char *msg)
{
	const char *ds = 0;

	if (directive == SKIP) { ds = "SKIPPED"; }
	if (directive == TODO) { ds = "TODO"; }

	if (ds) {
		printf(" # %s%s%s", ds, msg && *msg ? " " : "", msg);
	}
}


static int read_all(int fd, void *buf, int sz)
{
	char *p;
	int n, n_read;

	p = buf;
	n_read = 0;
	do {
		CHECK(n = read(fd, p, sz - n_read), n >= 0);
		n_read += n;
		if (n > 0) p += n;
	} while (n_read < sz && n > 0);
	return n_read;
}

static void exec_test(int (*fn)(void), struct test_result *result)
{
	switch (setjmp(g_state.skip)) {
	case TEST:
		/* Initialize state */
		g_state.init = 1;
		g_state.result = result;

		/* Run test */
		result->mode = TEST;
		result->success = (*fn)();
		break;

	case FAIL:
		/* Test was prematurely aborted due to failure. */
		result->mode = FAIL;
		result->success = -1;
		break;

	case SKIP: 
		/* Test was skipped */
		result->mode = SKIP;
		result->success = 0;
		break;

	case BAIL_OUT: 
		/* Testing should end */
		result->mode = BAIL_OUT;
		result->success = -1;
		break;

	default:
		/* Unkown case */
		abort();
	}

	g_state.result = 0;
	g_state.init = 0;
}

int run_test(int id, int fd, const char *prefix)
{
	int err, outfd;
	struct test const *t;
	struct test_result result = { TEST, -1, -1, { 0 } };

	t = tests + id;

	/* Redirect stdout to file */
	CHECK(outfd = dup(STDOUT_FILENO), outfd >= 0);
	CHECK(err = dup2(fd, STDOUT_FILENO), err != -1);

	/* Run test function */
	exec_test(t->fn, &result);
	
	/* Restore stdout */
	CHECK(err = dup2(outfd, STDOUT_FILENO), err != -1);
	close(outfd);

	/* In case of a bail out, exit as soon as possible */
	if (result.mode == BAIL_OUT) {
		do_bail_out(result.message);
	}

	/* Print primary test result output */
	printf("%sok %d %s", result.success ? "not " : "", id + 1, t->desc);
	print_directive(result.mode, result.message);
	printf("\n");

	/* Write child output to stdout as additional diagnostics, prefixing
	   each line with a tab */
	echo_file(fd, prefix);

	return result.success;

}

/* Run test in sub-process, and store its standard output in a temporary file,
   and write to it later. */
int fork_test(int id, int fd, const char *prefix)
{
	char sigmsg[100] = { '\0' };
	int pipefd[2], err;
	struct test_result result = { TEST, -1, -1, { 0 } };
	siginfo_t info;
	pid_t pid;
	struct test const *t;

	t = tests + id;

	/* Create pipe for IPC */
	CHECK(pipe(pipefd) == 0);

	/* Flush streams before forking */
	CHECK(err = fflush(NULL), err == 0);

	/* Create sub-process to run test in */
	CHECK(pid = fork(), pid >= 0);
	if (pid == 0) {
		/* Close pipe read end */
		close(pipefd[0]);

		/* Redirect standard output and standard error to the file */
		CHECK(err = dup2(fd, STDOUT_FILENO), err != -1);
		CHECK(err = dup2(fd, STDERR_FILENO), err != -1);

		/* Run test function */
		exec_test(t->fn, &result);

		/* Write result to pipe (if we haven't already crashed) */
		write(pipefd[1], &result, sizeof result);

		/* Test is done */
		exit(result.success == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
	}

	/* Close write end in parent */
	close(pipefd[1]);

	/* Get results from child process */
	if (read_all(pipefd[0], &result, sizeof result) != sizeof result) {
		/* Child crashed */
		result.success = -1;
		result.mode = TEST;
	}

	/* Wait for test to finish */
	CHECK(err = waitid(P_PID, pid, &info, WEXITED), err == 0);

	/* In case of a bail out, exit as soon as possible */
	if (result.mode == BAIL_OUT) {
		do_bail_out(result.message);
	}

	/* Did the test terminate by an unhandled signal? */
	if (info.si_code == CLD_DUMPED || info.si_code == CLD_KILLED) {
		snprintf(sigmsg, sizeof sigmsg, " (signal %d)", info.si_status);
		result.success = (info.si_status == result.signo) ? 0 : 1;
	}

	/* Print primary test result output */
	printf("%sok %d%s %s", result.success ? "not " : "", id + 1, sigmsg,
	       t->desc);
	print_directive(result.mode, result.message);
	printf("\n");

	/* Write child output to stdout as additional diagnostics, prefixing
	   each line with a string */
	echo_file(fd, prefix);

	/* Free all resources */
	close(pipefd[0]);

	return result.success;
}

