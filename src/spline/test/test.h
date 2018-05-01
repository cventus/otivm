static inline void assert_bool(char const *expr, int val, int target)
{
	if (!val != !target) {
		printf("not %s: %s \n", target ? "true" : "false",  expr);
		ok = -1;
	}
}

static inline void assert_near_eq(
	double a,
	char const *a_exp,
	double b,
	char const *b_exp)
{
	if (fabs(a - b) > 1e-6) {
		printf("%s = %f != %f = %s\n", a_exp, a, b, b_exp);
		ok = -1;
	}
}

#define assert_true(x) assert_bool(#x, x, 1)
#define assert_false(x) assert_bool(#x, x, 0)
#define assert_near_eq(a, b) assert_near_eq(a, #a, b, #b)
