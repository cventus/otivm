#ifndef LX_BITS 
#define LX_BITS 32
#endif

#if LX_BITS == 16
#undef lxfloat
#elif LX_BITS == 32
#define lxfloat float
#elif LX_BITS == 64
#define lxfloat double
#else
#error "LX_BITS must be 16, 32, or 64"
#endif

#define JOIN_(a, b) a ## b
#define JOIN(a, b) JOIN_(a, b)
#define MANGLE(name) JOIN(JOIN(lx, LX_BITS), JOIN(_, name))

/* mangle names to include cell bit size */
#define lxint JOIN(int, JOIN(LX_BITS, _t))
#define lxuint JOIN(uint, JOIN(LX_BITS, _t))

#define lxvalue MANGLE(value)

#define lx_bool MANGLE(bool)
#define lx_int MANGLE(int)
#define lx_float MANGLE(float)

#define lx_car MANGLE(car)
#define lx_cdr MANGLE(cdr)
#define lx_tail MANGLE(tail)
#define lx_ref MANGLE(ref)
#define lx_length MANGLE(length)
#define lx_equals MANGLE(lx_equals)

struct lx_list
{
	unsigned char tag;
	uint16_t offset;
	void const *cell;
};

union lxvalue
{
	struct lx_list list;
	struct {
		unsigned char tag;
		union {
			bool b;
			lxint i;
#ifdef lxfloat
			lxfloat f;
#endif
		};
	};
};

bool lx_equals(union lxvalue a, union lxvalue b);

static inline union lxvalue lx_bool(bool b)
{
	return (union lxvalue) { .tag = lx_bool_tag, .b = b };
}

static inline union lxvalue lx_int(lxint i)
{
	return (union lxvalue) { .tag = lx_int_tag, .i = i };
}
#ifdef lxfloat

static inline union lxvalue lx_float(lxfloat f)
{
	return (union lxvalue) { .tag = lx_float_tag, .f = f };
}
#endif

/* get first element of list */
union lxvalue lx_car(struct lx_list list);

/* remove first element from list */
struct lx_list lx_cdr(struct lx_list list);

/* i repetitions of `cdr` */
struct lx_list lx_drop(struct lx_list list, lxint i);

/* equivalent of `car(drop(list, i))` */
union lxvalue lx_nth(struct lx_list list, lxint i);

/* number of elements in list */
lxint lx_length(struct lx_list list);
