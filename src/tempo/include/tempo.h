#include <stdint.h>

typedef uint64_t usec64;

/* Opaque struct of platform specific state for creating accurate time
   stamps. */
struct pfclock;

/* Get microseconds since a previously selected point in time (process specific
  epoch), similar to time(2). */
usec64 pfclock_usec(struct pfclock *);

/* struct stopwatch - calculate elapsed time based on timestamps with support
   for pausing and store times in microsecond precision. A separate source to
   samples (monotone) time stamps is needed and a single source can be used to
   synchonize many stopwatches. All the operations should be provided with time
   stamps generated from the same source, or the time stamps should be
   semantically meaningful and synchonized (e.g. UNIX epoch time). */
struct stopwatch
{
	usec64 start, pause;
};

/* (Re-)initialize stop watch `sw` starting time to `t`. Elapsed times are
   based on this instant. Call this at least once before any of the other
   functions. */
void stopwatch_start(struct stopwatch *sw, usec64 t);

/* Pause stop watch `sw` based on current time `t`. While paused, the
   elapsed time will not increase with `t`. The stopwatch is guaranteed to be
   in the *paused* state after return, but non-zero is returned if time seem to
   be moving backwards. */
int stopwatch_pause(struct stopwatch *sw, usec64 t);

/* Resume stop watch `sw` based on current time `t`. The time spent in the
   *paused* state will be taken into consideration when calculating future
   delta times. The stopwatch is guaranteed to be in the *running* state after
   return, but non-zero is returned if time seem to be moving backwards. */
int stopwatch_resume(struct stopwatch *sw, usec64 t);

/* Return true if the stopwatch is currently paused. */
int stopwatch_is_paused(struct stopwatch const *sw);

/* Get time elapsed between the starting time and current time `t`. If the
   stopwatch is paused then the elapsed time will seemingly not advance
   despite increases in `t` until resumed. */
usec64 stopwatch_elapsed(struct stopwatch const *sw, usec64 t);
