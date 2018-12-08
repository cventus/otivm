
#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ok/ok.h"

FILE *open_str(char const *string)
{
	char filename[] = "/tmp/ok.XXXXXX";
	int fd, ro, len;
	FILE *fp;

	len = strlen(string);
	if (fd = mkstemp(filename), fd < 0) {
		fail_test("open_str(): Couldn't open temporary file\n");
	}
	if (write(fd, string, strlen(string)) != len) {
		(void)close(fd);
		(void)unlink(filename);
		fail_test("open_str(): Couldn't write file contents\n");
	}
	ro = open(filename, O_RDONLY);
	if (ro < 0) {
		(void)close(fd);
		(void)unlink(filename);
		fail_test("open_str(): failed to open file as read only\n");
	}
	(void)close(fd);
	(void)unlink(filename);
	if (fp = fdopen(ro, "r"), fp == NULL) {
		(void)close(ro);
		fail_test("open_str(): failed to open stream\n");
	}
	return fp;
}

