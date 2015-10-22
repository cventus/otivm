
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
Usage: %s [-n] [-l] [-v] [-p <line prefix>] [-s seed] [<test id>...]\n\
 -n\tNo fork; run all tests in the same process (simpler for debugging)\n\
 -l\tList tests, but do not run them\n\
 -v\tInvert selection, only run or list tests not specified in the command\n\
\tline.\n\
 -p\tUse the specified prefix instead of a tab on each line\n\
 -s\tRandom seed used to seed srand() before each test (default 1)\n\
", argv0);
}

static int bisset(unsigned char *mask, unsigned bit)
{
	return mask[bit / CHAR_BIT] & (1 << (bit % CHAR_BIT));
}

static void bset(unsigned char *mask, unsigned bit)
{
	mask[bit / CHAR_BIT] |= (1 << (bit % CHAR_BIT));
}

static void bclear(unsigned char *mask, unsigned bit)
{
	mask[bit / CHAR_BIT] &= ~(1 << (bit % CHAR_BIT));
}

static unsigned char *parse_test_mask(char *ids[], int n, int invert, size_t m)
{
	unsigned char *mask;
	char *end;
	size_t sz, id;
	int i;

	sz = (m + CHAR_BIT - 1) / CHAR_BIT;
	CHECK(mask = malloc(sz), mask);
	memset(mask, n == 0 || invert ? ~0 : 0, sz);

	for (i = 0; i < n; i++) {
		id = (size_t)strtol(ids[i], &end, 0);
		if (ids[i] == '\0' || *end != '\0' || id < 1 || id > m) {
			fprintf(stderr, "invalid id: \"%s\"\n", ids[i]);
			exit(EXIT_FAILURE);
		}
		(invert ? bclear : bset)(mask, id - 1);
	}
	return mask;
}

static size_t count_tests(struct test const *t)
{
	size_t i;

	for (i = 0; t[i].fn; i++) { }

	return i;
}

static void list_tests(unsigned char *mask)
{
	size_t i;

	for (i = 0; tests[i].fn; i++) {
		if (bisset(mask, i)) {
			printf("%zd\t%s\n", i + 1, tests[i].desc);
		}
	}
}

static void print_skipped(unsigned id)
{
	printf("ok %u %s # SKIPPED\n", id + 1, tests[id].desc);
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

int main(int argc, char **argv)
{
	char *endp;
	const char *prefix;
	unsigned char *test_mask;
	int n_fail, can_fork, list, invert, fd, opt, err;
	unsigned long seed;
	size_t i, n_tests;

	/* set default options */
	can_fork = 1;
	list = 0;
	invert = 0;
	prefix = "\t";
	seed = 1;

	/* parse command line arguments */
	while ((opt = getopt(argc, argv, "nlvp:s:")) != -1) {
		switch (opt) {
		case 'n': can_fork = 0; break;
		case 'v': invert = 1; break;
		case 'l': list = 1; break;
		case 'p': prefix = optarg; break;
		case 's':
			seed = strtoul(optarg, &endp, 0);
			if (endp == optarg || *endp != '\0') {
				die("Bad random seed: %s\n", optarg);
			}
			if (seed > RAND_MAX) {
				die("Random seed out of range [0, %lu]: %s\n",
				    (unsigned long)RAND_MAX, optarg);
			}
			break;
		default: usage(argv[0]); break;
		}
	}

	n_tests = count_tests(tests);
	test_mask = parse_test_mask(argv+optind, argc-optind, invert, n_tests);

	if (list) {
		list_tests(test_mask);
		free(test_mask);
		exit(0);
	} 

	CHECK(fd = open_temp_file(), fd >= 0);
	n_fail = 0;
	printf("1..%zd\n", n_tests);
	for (i = 0; i < n_tests; i++) {
		if (!bisset(test_mask, i)) {
			print_skipped(i);
			continue;
		}
		ok = 0;
		srand((unsigned int)seed);
		CHECK(err = ftruncate(fd, 0), err == 0);
		err = (can_fork ? fork_test : run_test)(i, fd, prefix);
		if (err || ok) {
			n_fail++;
		}
	}
	free(test_mask);
	exit(n_fail);
} 

