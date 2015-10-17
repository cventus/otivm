
#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <ok/ok.h>
#include "private.h"
#include "check.h"

/* Definition of global test status flag. */
int ok = 0;

static void die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

static void usage(const char *argv0)
{
	die("\
Usage: %s [-n] [-l] [-v] [-p <line prefix>] [-s seed] [<test>...]\n\
 -n\tNo fork; run all tests in the same process (simpler for debugging)\n\
 -l\tList tests, but do not run them\n\
 -v\tInvert selection, only run or list tests not specified in the command\n\
\tline.\n\
 -p\tUse the specified prefix instead of a tab on each line\n\
 -s\tRandom seed used to seed srand() before each test (default 1)\n\
", argv0);
}

static int match_description(char const *pattern)
{
	int i;

	for (i = 0; tests[i].fn && tests[i].desc; i++) {
		if (strstr(tests[i].desc, pattern)) {
			return i;
		}
	}
	return -1;
}

static int is_listed(int id, char *ids[], int n_ids, int invert, size_t m)
{
	int i;
	char *end;
	long val;

	if (n_ids == 0) { return 1; }

	for (i = 0; i < n_ids; i++) {
		val = strtol(ids[i], &end, 0);
		if (ids[i] == '\0' || *end != '\0') {
			val = match_description(ids[i]);
			if (val < 0) {
				die("no such test: \"%s\"\n", ids[i]);
			}
			val++; /* Index zero based, ids one based */
		}
		if (val < 1 || (size_t)val > m) {
			die("id %ld out of range [1..%zd]\n", val, m);
		}
		if (val == id) { return !invert; }
	}
	return invert;
}

static size_t count_tests(struct test const *t)
{
	size_t i;

	for (i = 0; t[i].fn; i++) { }
	return i;
}

/* Create file to redirect test stdout and stderr into. */
static int open_temp_file(void)
{
	char template[] = "/tmp/test-output.XXXXXX";
	int fd, err;

	CHECK(fd = mkstemp(template), fd >= 0);
	CHECK(err = unlink(template), err == 0);

	return fd;
}

struct options
{
	int can_fork, list, invert;
	unsigned int seed;
	char const *prefix;
};

static int parse_options(int argc, char **argv, struct options *options)
{
	char *endp;
	int opt;
	unsigned long seed;

	/* set default options */
	options->can_fork = 1;
	options->list = 0;
	options->invert = 0;
	options->prefix = "\t";
	options->seed = 1;

	/* parse command line arguments */
	while ((opt = getopt(argc, argv, "nlvp:s:")) != -1) {
		switch (opt) {
		case 'n': options->can_fork = 0; break;
		case 'v': options->invert = 1; break;
		case 'l': options->list = 1; break;
		case 'p': options->prefix = optarg; break;
		case 's':
			seed = strtoul(optarg, &endp, 0);
			if (endp == optarg || *endp != '\0') {
				die("Bad random seed: %s\n", optarg);
			}
			if (seed > RAND_MAX) {
				die("Random seed out of range [0, %lu]: %s\n",
				    (unsigned long)RAND_MAX, optarg);
			}
			options->seed = (unsigned int)seed;
			break;
		default: usage(argv[0]); break;
		}
	}

	return optind;
}

int main(int argc, char **argv)
{
	struct options o;
	char **ids;
	int n_ids, n_fail, fd, opts, err;
	size_t i, n_tests;

	opts = parse_options(argc, argv, &o);
	ids = argv + opts;
	n_ids = argc - opts;
	n_tests = count_tests(tests);

	/* List tests */
	if (o.list) {
		for (i = 0; i < n_tests; i++) {
			if (is_listed(i + 1, ids, n_ids, o.invert, n_tests)) {
				printf("%zd\t%s\n", i + 1, tests[i].desc);
			}
		}
		exit(0);
	} 

	/* Run tests */
	CHECK(fd = open_temp_file(), fd >= 0);
	n_fail = 0;
	printf("1..%zd\n", n_tests);
	for (i = 0; i < n_tests; i++) {
		if (!is_listed(i + 1, ids, n_ids, o.invert, n_tests)) {
			printf("ok %zd %s # SKIPPED\n", i + 1, tests[i].desc);
			continue;
		}
		ok = 0;
		srand(o.seed);
		CHECK(err = ftruncate(fd, 0), err == 0);
		err = (o.can_fork ? fork_test : run_test)(i, fd, o.prefix);
		if (err || ok) { n_fail++; }
	}
	exit(n_fail);
} 

