#include <ok/ok.h>
#include <tempo/tempo.h>

#define T0 10
#define T1 20
#define T2 30
#define T3 40
#define T4 50

#define testfn(f) { &f, #f }
#define statusof(x) ((x) ? 0 : -1)

struct stopwatch sw;

static int zero_elapsed_time(void)
{
	stopwatch_start(&sw, T0);
	return statusof(stopwatch_elapsed(&sw, T0) == 0);
}

static int some_elapsed_time(void)
{
	stopwatch_start(&sw, T0);
	return statusof(stopwatch_elapsed(&sw, T2) == T2 - T0);
}

static int pauses(void)
{
	stopwatch_start(&sw, T0);
	stopwatch_pause(&sw, T1);
	return statusof(stopwatch_elapsed(&sw, T2) == T1 - T0);
}

static int resumes(void)
{
	stopwatch_start(&sw, T0);
	stopwatch_pause(&sw, T1);
	stopwatch_resume(&sw, T2);
	return statusof(stopwatch_elapsed(&sw, T3) == (T1 - T0) + (T3 - T2));
}

struct test const tests[] = {
	testfn(zero_elapsed_time),
	testfn(some_elapsed_time),
	testfn(pauses),
	testfn(resumes),
	{ 0, 0 }
};
