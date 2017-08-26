#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 199309L
#error "_POSIX_C_SOURCE >= 199309L required for clock_gettime"
#endif
#if !defined(_POSIX_TIMERS) || _POSIX_TIMERS < 1
#error "clock_gettime not supported!"
#endif

#include "../include/tempo.h"
#include "../include/posix.h"

static int try_clock(clockid_t clk_id, struct pfclock *c)
{
	struct timespec ts;
	int res = clock_gettime(clk_id, &ts);
	if (res == 0) {
		c->clk_id = clk_id;
		c->ps_epoch = ts.tv_sec;
	}
	return res;
}

int init_posix_pfclock(struct pfclock *c)
{
	if (sysconf(_POSIX_MONOTONIC_CLOCK) > 0) {
		if (try_clock(CLOCK_MONOTONIC, c) == 0) {
			return 0;
		}
	}
	return try_clock(CLOCK_REALTIME, c);
}

usec64 pfclock_usec(struct pfclock *c)
{
	struct timespec ts;
	usec64 s, us;

	(void)clock_gettime(c->clk_id, &ts);

	s = ts.tv_sec - c->ps_epoch;
	us = ts.tv_nsec / 1000;
	return s * 1000000 + us;
}

struct pfclock *pfclock_make(void)
{
	struct pfclock *p;
	p = malloc(sizeof *p);
	if (p) {
		if (init_posix_pfclock(p)) {
			free(p);
			return NULL;
		}
	}
	return p;
}

void pfclock_free(struct pfclock *p)
{
	free(p);
}
