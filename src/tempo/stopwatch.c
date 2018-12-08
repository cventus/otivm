#include <stdint.h>

#include "tempo/tempo.h"

#define UNPAUSED (~(usec64)0)

void stopwatch_start(struct stopwatch *sw, usec64 t)
{
	sw->start = t;
	sw->pause = UNPAUSED;
}

int stopwatch_is_paused(struct stopwatch const *sw)
{
	return sw->pause != UNPAUSED;
}

int stopwatch_pause(struct stopwatch *sw, usec64 t)
{
	int res = 0;
	if (sw->pause == UNPAUSED) {
		if (t < sw->start) {
			/* Time went backwards! Still, let's do our best to
			   enter the paused state by assuming that no time has
			   passed. */
			res = -2;
			sw->pause = sw->start;
		} else {
			sw->pause = t;
		}
	} else {
		res = -1;
	}
	return res;
}

int stopwatch_resume(struct stopwatch *sw, usec64 t)
{
	int res = 0;
	if (sw->pause == UNPAUSED) {
		res = -1;
	} else {
		if (t < sw->pause) {
			/* Time went backwards! Don't want to remain in the
			   paused state in any case, but rather assume no time
			   has passed. */
			res = -2;
		} else {
			sw->start += t - sw->pause;
		}
		sw->pause = UNPAUSED;
	}
	return res;
}

usec64 stopwatch_elapsed(struct stopwatch const *sw, usec64 t)
{
	if (sw->pause == UNPAUSED) {
		if (t > sw->start) {
			return t - sw->start;
		} else {
			return (usec64)0;
		}
	} else {
		return sw->pause - sw->start;
	}
}
