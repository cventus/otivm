#ifndef LX_BITS
#define LX_BITS 32
#endif

#if LX_BITS == 16
/* No 16-bit floating point number support */
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

/* primitive types */
#define lxheap MANGLE(heap)
#define lxcell MANGLE(cell)
#define lxmem MANGLE(mem)
#define lxvalue MANGLE(value)
#define lxref MANGLE(ref)
#define lxlist MANGLE(list)
#define lxresult MANGLE(result)
#define lxalloc MANGLE(alloc)

/* converter functions */
#define lx_list MANGLE(list)
#define lx_bool MANGLE(bool)
#define lx_int MANGLE(int)
#define lx_float MANGLE(float)

/* heap API */
#define lx_make_heap MANGLE(make_heap)
#define lx_free_heap MANGLE(free_heap)
#define lx_heap_size MANGLE(heap_size)
#define lx_heap_root MANGLE(heap_root)
#define lx_modify MANGLE(modify)

/* list API */
#define lx_empty_list MANGLE(empty_list)
#define lx_is_empty_list MANGLE(is_empty_list)
#define lx_is_list MANGLE(is_list)
#define lx_cons MANGLE(cons)
#define lx_car MANGLE(car)
#define lx_cdr MANGLE(cdr)
#define lx_drop MANGLE(drop)
#define lx_nth MANGLE(nth)
#define lx_length MANGLE(length)
#define lx_equals MANGLE(equals)

/* internal API */
#define lx_shared_head MANGLE(shared_head)
#define lx_count_refs MANGLE(count_refs)
#define lx_compact MANGLE(compact)
#define lx_resize_heap MANGLE(resize_heap)
#define lx_gc MANGLE(gc)

/* Make an integer constant representing a tag with two cdr-code bits and
   six type tag bits: CCTTTTTT */
#define mktag(cdr, type) ((lxtag)(((lxtag)(cdr) << 6)|((lxtag)(type) & 0x3f)))

/* Asserted value of sizeof(union lxXY_cell) */
#define CELL_SIZE ((lxint)LX_BITS / 8)

/* Number of headers/cells per span. Must be a power of two. */
#define CELL_SPAN CELL_SIZE

/* Total number of cells in a span (header segment and cell segment) */
#define SPAN_LENGTH (CELL_SPAN + 1)

/* LSB of a pointer holding the offset of the header and cell */
#define OFFSET_MASK (CELL_SPAN - (lxint)1)

union lxcell;
struct lxmem;
struct lxheap;

struct lx_config
{
	size_t max_size;
};

struct lxref
{
	unsigned char tag, offset;
	union lxcell const *cell;
};

struct lxlist
{
	union {
		unsigned char tag;
		struct lxref ref;
	};
};

union lxvalue
{
	struct lxlist list;
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

struct lxresult
{
	int status;
	union lxvalue value;
};

struct lxheap *lx_make_heap(size_t init_size, struct lx_config const *config);
void lx_free_heap(struct lxheap *heap);

size_t lx_heap_size(struct lxheap const *heap);
union lxvalue lx_heap_root(struct lxheap const *heap);

struct lxresult lx_modify(
	struct lxheap *heap,
	union lxvalue modify(struct lxmem *, union lxvalue, void *),
	void *param);

/* recursively compare values for equality */
bool lx_equals(union lxvalue a, union lxvalue b);

/* prepend an element to a list */
struct lxlist lx_cons(struct lxmem *, union lxvalue, struct lxlist);

/* get first element of (non-empty) list */
union lxvalue lx_car(struct lxlist list);

/* remove first element from (non-empty) list */
struct lxlist lx_cdr(struct lxlist list);

/* i repetitions of `cdr` */
struct lxlist lx_drop(struct lxlist list, lxint i);

/* equivalent of `car(drop(list, i))` */
union lxvalue lx_nth(struct lxlist list, lxint i);

/* number of elements in list */
lxint lx_length(struct lxlist list);

/* wrap a list in a tagged union */
static inline union lxvalue lx_list(struct lxlist list)
{
	return (union lxvalue) { .list = list };
}

/* wrap a boolean in a tagged union */
static inline union lxvalue lx_bool(bool b)
{
	return (union lxvalue) { .tag = lx_bool_tag, .b = b };
}

/* wrap an integer in a tagged union */
static inline union lxvalue lx_int(lxint i)
{
	return (union lxvalue) { .tag = lx_int_tag, .i = i };
}
#ifdef lxfloat

/* wrap a floating point number in a tagged union */
static inline union lxvalue lx_float(lxfloat f)
{
	return (union lxvalue) { .tag = lx_float_tag, .f = f };
}
#endif

/* create an empty list value */
static inline struct lxlist lx_empty_list(void)
{
	return (struct lxlist) { .ref = { lx_nil_tag, 0, 0 } };
}

/* compare a list against the empty list */
static inline bool lx_is_empty_list(struct lxlist list)
{
	return list.tag == lx_nil_tag;
}

/* check if a value is a list (i.e. of type list or empty list) */
static inline bool lx_is_list(union lxvalue val)
{
	return val.tag <= lx_list_tag;
}
