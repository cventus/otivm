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
#define lxtree MANGLE(tree)
#define lxresult MANGLE(result)
#define lxalloc MANGLE(alloc)

/* conversion functions */
#define lx_list MANGLE(list)
#define lx_tree MANGLE(tree)
#define lx_bool MANGLE(bool)
#define lx_int MANGLE(int)
#define lx_float MANGLE(float)

/* heap API */
#define lx_make_heap MANGLE(make_heap)
#define lx_free_heap MANGLE(free_heap)
#define lx_heap_size MANGLE(heap_size)
#define lx_heap_value MANGLE(heap_value)
#define lx_modify MANGLE(modify)
#define lx_gc MANGLE(gc)

/* list API */
#define lx_empty_list MANGLE(empty_list)
#define lx_is_empty_list MANGLE(is_empty_list)
#define lx_cons MANGLE(cons)
#define lx_car MANGLE(car)
#define lx_cdr MANGLE(cdr)
#define lx_drop MANGLE(drop)
#define lx_nth MANGLE(nth)
#define lx_length MANGLE(length)
#define lx_equals MANGLE(equals)

/* tree API */
#define lx_empty_tree MANGLE(empty_tree)
#define lx_is_empty_tree MANGLE(is_empty_tree)
#define lx_tree_entry MANGLE(tree_entry)
#define lx_tree_left MANGLE(tree_left)
#define lx_tree_right MANGLE(tree_right)
#define lx_tree_size MANGLE(tree_size)
#define lx_tree_nth MANGLE(tree_nth)

/* internal API */
#define lx_shared_head MANGLE(shared_head)
#define lx_count_refs MANGLE(count_refs)
#define lx_compact MANGLE(compact)
#define lx_resize_heap MANGLE(resize_heap)
#define lx_reserve_tagged MANGLE(reserve_tagged)
#define lx_set_cell_data MANGLE(set_cell_data)

#define TAG_BIT 3
#define TAG_MASK ((1 << TAG_BIT) - 1)
#define MAX_SEGMENT_LENGTH ((1 << (CHAR_BIT - TAG_BIT)) - 2)

/* Make an integer constant representing a tag with five segment length bits
   and three type tag bits: LLLLLTTT */
#define mktag(len, type) \
	((lxtag)(((lxtag)(len) << TAG_BIT) | ((lxtag)(type) & TAG_MASK)))

/* Asserted value of sizeof(union lxXY_cell) */
#define CELL_SIZE ((lxint)LX_BITS / 8)

/* Number of headers/cells per span. */
#define CELL_SPAN CELL_SIZE

/* Total number of cells in a span (one tag cell and CELL_SIZE data cells) */
#define SPAN_LENGTH (CELL_SPAN + 1)

/* LSB of a reference holding the offset of the tag and cell */
#define OFFSET_MASK (CELL_SPAN - 1)

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

struct lxtree
{
	union {
		unsigned char tag;
		struct lxref ref;
	};
};

union lxvalue
{
	struct lxlist list;
	struct lxtree tree;
	struct {
		unsigned char tag;
		union {
			bool b;
			lxint i;
			char const *s;
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
union lxvalue lx_heap_value(struct lxheap const *heap);

struct lxresult lx_modify(
	struct lxheap *heap,
	union lxvalue modify(struct lxmem *, union lxvalue, void *),
	void *param);

int lx_gc(struct lxheap *heap);

/* recursively compare values for equality */
bool lx_equals(union lxvalue a, union lxvalue b);

/* compare values by built-in ordering */
int lx_compare(union lxvalue a, union lxvalue b);

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

/* strlen(3) of string value */
size_t lx_strlen(union lxvalue string);

/* copy the first n bytes of s into a heap */
union lxvalue lx_strndup(struct lxmem *, char const *s, size_t n);

/* copy s into a heap */
union lxvalue lx_strdup(struct lxmem *mem, char const *s);

/* allocate a new formatted string in the heap */
union lxvalue lx_sprintf(struct lxmem *, char const *fmt, ...);

/* allocate a new formatted string in the heap */
union lxvalue lx_vsprintf(struct lxmem *, char const *fmt, va_list ap);

/* wrap a list in a tagged union */
static inline union lxvalue lx_list(struct lxlist list)
{
	return (union lxvalue) { .list = list };
}

/* wrap a tree in a tagged union */
static inline union lxvalue lx_tree(struct lxtree tree)
{
	return (union lxvalue) { .tree = tree };
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
	return (struct lxlist) { .ref = { lx_list_tag, 0, 0 } };
}

/* compare a list against the empty list */
static inline bool lx_is_empty_list(struct lxlist list)
{
	return list.tag == lx_list_tag && list.ref.cell == NULL;
}

/* create an empty list value */
static inline struct lxtree lx_empty_tree(void)
{
	return (struct lxtree) { .ref = { lx_tree_tag, 0, 0 } };
}

/* compare a list against the empty list */
static inline bool lx_is_empty_tree(struct lxtree tree)
{
	return tree.tag == lx_tree_tag && tree.ref.cell == NULL;
}

/* low level API for traversal */
struct lxlist lx_tree_entry(struct lxtree);
struct lxtree lx_tree_left(struct lxtree);
struct lxtree lx_tree_right(struct lxtree);

size_t lx_tree_size(struct lxtree);

struct lxlist lx_tree_nth(struct lxtree tree, lxint n);
