#ifndef LX_BITS
#define LX_BITS 32
#endif

#if LX_BITS == 16
#define lxfloat float
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
#define lxstate MANGLE(state)
#define lxvalue MANGLE(value)
#define lxalloc MANGLE(alloc)
#define lxread MANGLE(read)

/* reference values */
#define lxlist MANGLE(list)
#define lxmap MANGLE(map)
#define lxset MANGLE(set)
#define lxstring MANGLE(string)

/* value constructors */
#define lx_valueb MANGLE(valueb)
#define lx_valuei MANGLE(valuei)
#define lx_valuef MANGLE(valuef)

/* conversion functions */
#define lx_list MANGLE(list)
#define lx_is_list MANGLE(is_list)
#define lx_map MANGLE(map)
#define lx_is_map MANGLE(is_map)
#define lx_string MANGLE(string)
#define lx_is_string MANGLE(is_string)
#define lx_bool MANGLE(bool)
#define lx_is_bool MANGLE(is_bool)
#define lx_int MANGLE(int)
#define lx_is_int MANGLE(is_int)
#define lx_float MANGLE(float)
#define lx_is_float MANGLE(is_float)

/* heap API */
#define lx_make_heap MANGLE(make_heap)
#define lx_free_heap MANGLE(free_heap)
#define lx_heap_size MANGLE(heap_size)
#define lx_heap_value MANGLE(heap_value)
#define lx_heap_read MANGLE(heap_read)
#define lx_handle_out_of_memory MANGLE(handle_out_of_memory)
#define lx_prepare_start MANGLE(prepare_start)
#define lx_end MANGLE(end)
#define lx_gc MANGLE(gc)
#define lx_init_cons MANGLE(init_cons)
#define lx_init_tospace MANGLE(init_tospace)

/* generic API */
#define lx_equals MANGLE(equals)
#define lx_compare MANGLE(compare)
#define lx_read MANGLE(read)
#define lx_write MANGLE(write)
#define lx_write_pretty MANGLE(write_pretty)

/* list API */
#define lx_empty_list MANGLE(empty_list)
#define lx_is_empty_list MANGLE(is_empty_list)
#define lx_cons MANGLE(cons)
#define lx_car MANGLE(car)
#define lx_carb MANGLE(carb)
#define lx_cari MANGLE(cari)
#define lx_carf MANGLE(carf)
#define lx_cdr MANGLE(cdr)
#define lx_drop MANGLE(drop)
#define lx_nth MANGLE(nth)
#define lx_nthb MANGLE(nthb)
#define lx_nthi MANGLE(nthi)
#define lx_nthf MANGLE(nthf)
#define lx_length MANGLE(length)
#define lx_reverse MANGLE(reverse)

/* tuples/list short-hands */
#define lx_single MANGLE(single)
#define lx_pair MANGLE(pair)
#define lx_triple MANGLE(triple)

/* map API */
#define lx_empty_map MANGLE(empty_map)
#define lx_is_empty_map MANGLE(is_empty_map)
#define lx_map_size MANGLE(map_size)
#define lx_map_set MANGLE(map_set)
#define lx_map_assoc MANGLE(map_assoc)
#define lx_map_ref MANGLE(map_ref)
#define lx_map_remove MANGLE(map_remove)
#define lx_map_entry MANGLE(map_entry)
#define lx_map_left MANGLE(map_left)
#define lx_map_right MANGLE(map_right)
#define lx_map_nth MANGLE(map_nth)
#define lx_map_filter MANGLE(map_filter)
#define lx_map_union MANGLE(map_union)
#define lx_map_isect MANGLE(map_isect)
#define lx_map_diff MANGLE(map_diff)

/* string API */
#define lx_strlen MANGLE(strlen)
#define lx_strdup MANGLE(strdup)
#define lx_strndup MANGLE(strndup)
#define lx_sprintf MANGLE(sprintf)
#define lx_vsprintf MANGLE(vsprintf)

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

struct lxstate;
struct lxheap;

typedef unsigned char lxtag;

union lxcell
{
	lxtag t[CELL_SIZE];
	lxint i;
	lxfloat f;
};

struct lx_config
{
	size_t max_size;
};

struct lxvalue
{
	unsigned char tag, offset;
	union {
		bool b;
		lxint i;
		lxfloat f;
		char const *s;
	};
};

struct lxlist { struct lxvalue value; };
struct lxmap { struct lxvalue value; };
struct lxset { struct lxvalue value; };
struct lxstring { struct lxvalue value; };

struct lxread {
	enum lx_read_status status;
	char const *where;
	struct lxvalue value;
};

struct lxheap *lx_make_heap(size_t init_size, struct lx_config const *config);
void lx_free_heap(struct lxheap *heap);

size_t lx_heap_size(struct lxheap const *heap);
struct lxvalue lx_heap_value(struct lxheap const *heap);
struct lxread lx_heap_read(struct lxheap *heap, char const *str);

/*
Memory can only be allocated from an LX heap within a region of code started by
lx_start() and ended with lx_end(). lx_start() initializes an lxstate object,
which is used to keep track of allocations, and marks the restart-point using
setjmp(3) which is returned to after the heap has been garbage collected. The
lx_end() function marks the end of the update and assigns a new value to the
heap. Any reference into the heap becomes invalid at lx_start(). lx_start()
returns a negative value on error and non-negative on success.

When allocation from the heap fails it will be garbage collected and an
internal call to longjmp(3) will return control to the start of the loop and
the code within the start/end region will run again. The since the code within
the region might be executed more than once it is not advisable to perform I/O
or allocate memory with malloc(3) or otherwise modify local or global variables.

lx_start() is essentially setjmp(3), which cannot be used as a general
expression, and `lx_start` has the same limitations. It's possible there's a
longjmp(3) back to the start until `lx_end` has been reached. Don't modify
local variables within lx_start()/lx_end() that were initialized before it.
Don't store the result of lx_start() in a variable; only call it within an
if-statement and check if its value is less than zero or greater than or equal
to zero.

If lx_end() is not called, e.g. by returning from the function that contains
the region, the heap remains in a consistent state and references to allocated
object will stay valid until the heap is garbage collected.

Example:

    struct lxheap *heap = ...;
    struct lxlist list;
    struct lxstate state[1];

    if (lx_start(state, heap) < 0) {
        fprintf(stderr, "Failed to update heap\n");
        return -1;
    }

    list = lx_cons(state, lx_valuei(42), lx_empty_list());
    lx_end(heap, list.value);
*/
#define lx_start(state, heap) setjmp(*lx_prepare_start(state, heap))
void lx_end(struct lxstate *, struct lxvalue);

/* used internally by lx_start */
jmp_buf *lx_prepare_start(struct lxstate *, struct lxheap *);

void lx_gc(struct lxheap *heap);

/* recursively compare values for equality */
bool lx_equals(struct lxvalue a, struct lxvalue b);

/* compare values by built-in ordering */
int lx_compare(struct lxvalue a, struct lxvalue b);

/* deserialize lx string to value */
struct lxread lx_read(struct lxstate *, char const *str);

/* serialize value to lx string */
struct lxstring lx_write(struct lxstate *, struct lxvalue value);

/* pretty print value into string */
struct lxstring lx_write_pretty(struct lxstate *, struct lxvalue value);

/* create an empty list value */
static inline struct lxlist lx_empty_list(void)
{
	return (struct lxlist) {
		.value = { .tag = lx_list_tag, .offset = 0, .s = 0 }
	};
}

/* compare a list against the empty list */
static inline bool lx_is_empty_list(struct lxlist list)
{
	return list.value.s == NULL;
}

/* prepend an element to a list */
struct lxlist lx_cons(struct lxstate *, struct lxvalue, struct lxlist);

struct lxlist lx_single(struct lxstate *, struct lxvalue);
struct lxlist lx_pair(struct lxstate *, struct lxvalue, struct lxvalue);
struct lxlist lx_triple(struct lxstate *,
	struct lxvalue,
	struct lxvalue,
	struct lxvalue);

/* get first element of (non-empty) list */
struct lxvalue lx_car(struct lxlist list);
bool lx_carb(struct lxlist list);
lxint lx_cari(struct lxlist list);
lxfloat lx_carf(struct lxlist list);

/* remove first element from (non-empty) list */
struct lxlist lx_cdr(struct lxlist list);

/* i repetitions of `cdr` */
struct lxlist lx_drop(struct lxlist list, lxint i);

/* equivalent of `car(drop(list, i))` */
struct lxvalue lx_nth(struct lxlist list, lxint i);
bool lx_nthb(struct lxlist list, lxint i);
lxint lx_nthi(struct lxlist list, lxint i);
lxfloat lx_nthf(struct lxlist list, lxint i);

struct lxlist lx_list_set(
	struct lxstate *,
	struct lxlist l,
	lxint i,
	struct lxvalue value);

struct lxlist lx_set_to_list(struct lxstate *, struct lxset);
struct lxlist lx_map_to_list(struct lxstate *, struct lxmap);

struct lxlist lx_sort(
	struct lxstate *,
	struct lxlist,
	int cmp(struct lxvalue, struct lxvalue));

/* reverse list */
struct lxlist lx_reverse(struct lxstate *, struct lxlist list);

/* find first tail of list which car is val */
struct lxlist lx_member(struct lxlist list, struct lxvalue val);

/* retrieve association from association list */
struct lxlist lx_assoc(struct lxlist alist, struct lxvalue key);

/* number of elements in list */
lxint lx_length(struct lxlist list);

/* compare length of list against len, maybe not having to compute the whole
   length of the list */
lxint lx_length_cmp(struct lxlist list, lxint len);

/* strlen(3) of string value */
size_t lx_strlen(struct lxstring string);

/* copy the first n bytes of s into a heap */
struct lxstring lx_strndup(struct lxstate *, char const *s, size_t n);

/* copy s into a heap */
struct lxstring lx_strdup(struct lxstate *, char const *s);

/* allocate a new formatted string in the heap */
struct lxstring lx_sprintf(struct lxstate *, char const *fmt, ...);

/* allocate a new formatted string in the heap */
struct lxstring lx_vsprintf(struct lxstate *, char const *fmt, va_list ap);

/* conversion functions */
static inline struct lxlist lx_list(struct lxvalue value)
{
	assert(value.tag == lx_list_tag);
	return (struct lxlist) { .value = value };
}

static inline struct lxmap lx_map(struct lxvalue value)
{
	assert(value.tag == lx_map_tag);
	return (struct lxmap) { .value = value };
}

static inline bool lx_bool(struct lxvalue value)
{
	assert(value.tag == lx_bool_tag);
	return value.b;
}

static inline lxint lx_int(struct lxvalue value)
{
	assert(value.tag == lx_int_tag);
	return value.i;
}

static inline lxfloat lx_float(struct lxvalue value)
{
	assert(value.tag == lx_float_tag);
	return value.f;
}

static inline struct lxstring lx_string(struct lxvalue value)
{
	assert(value.tag == lx_string_tag);
	assert(value.offset == 0);
	return (struct lxstring) { .value = value };
}

static inline bool lx_is_list(struct lxvalue value, struct lxlist *out)
{
	return value.tag == lx_list_tag && (*out = lx_list(value), 1);
}

static inline bool lx_is_map(struct lxvalue value, struct lxmap *out)
{
	return value.tag == lx_map_tag && (*out = lx_map(value), 1);
}

static inline bool lx_is_bool(struct lxvalue value, bool *out)
{
	return value.tag == lx_bool_tag && (*out = lx_bool(value), 1);
}

static inline bool lx_is_int(struct lxvalue value, lxint *out)
{
	return value.tag == lx_int_tag && (*out = lx_int(value), 1);
}

static inline bool lx_is_float(struct lxvalue value, lxfloat *out)
{
	return value.tag == lx_float_tag && (*out = lx_float(value), 1);
}

static inline bool lx_is_string(struct lxvalue value, struct lxstring *out)
{
	return value.tag == lx_string_tag && (*out = lx_string(value), 1);
}

/* wrap a boolean in a tagged union */
static inline struct lxvalue lx_valueb(bool b)
{
	return (struct lxvalue) { .tag = lx_bool_tag, .offset = 0, .b = b };
}

/* wrap an integer in a tagged union */
static inline struct lxvalue lx_valuei(lxint i)
{
	return (struct lxvalue) { .tag = lx_int_tag, .offset = 0, .i = i };
}

/* wrap a floating point number in a tagged union */
static inline struct lxvalue lx_valuef(lxfloat f)
{
	return (struct lxvalue) { .tag = lx_float_tag, .offset = 0, .f = f };
}

static inline struct lxvalue lx_nil(void)
{
	return (struct lxvalue) {
		.tag = lx_nil_tag, .offset = 0, .s = 0
	};
}

static inline bool lx_is_nil(struct lxvalue value)
{
	return value.tag == lx_nil_tag;
}

/* create an empty set value */
static inline struct lxset lx_empty_set(void)
{
	return (struct lxset) {
		.value = { .tag = lx_set_tag, .offset = 0, .s = 0 }
	};
}

/* compare a set against the empty set */
static inline bool lx_is_empty_set(struct lxset set)
{
	return set.value.s == NULL;
}

size_t lx_set_size(struct lxset);
struct lxset lx_list_to_set(struct lxstate *, struct lxlist);
bool lx_set_has(struct lxset, struct lxvalue);
struct lxvalue lx_set_nth(struct lxset set, lxint n);
struct lxset lx_set_add(struct lxstate *, struct lxset, struct lxvalue);
struct lxset lx_set_remove(struct lxstate *, struct lxset, struct lxvalue);
struct lxset lx_set_union(struct lxstate *, struct lxset, struct lxset);
struct lxset lx_set_isect(struct lxstate *, struct lxset, struct lxset);
struct lxset lx_set_diff(struct lxstate *, struct lxset, struct lxset);

/* create an empty list value */
static inline struct lxmap lx_empty_map(void)
{
	return (struct lxmap) {
		.value = { .tag = lx_map_tag, .offset = 0, .s = 0 }
	};
}

struct lxvalue lx_set_entry(struct lxset);
struct lxset lx_set_left(struct lxset);
struct lxset lx_set_right(struct lxset);

struct lxmap lx_map_filter(
	struct lxstate *,
	struct lxmap map,
	bool predicate(struct lxvalue key, struct lxvalue value, void *),
	void *param);

/* compare a list against the empty list */
static inline bool lx_is_empty_map(struct lxmap map)
{
	return map.value.s == NULL;
}

size_t lx_map_size(struct lxmap);

struct lxmap lx_map_set(
	struct lxstate *,
	struct lxmap m,
	struct lxvalue key,
	struct lxvalue value);

struct lxmap lx_list_to_map(struct lxstate *, struct lxlist alist);

struct lxlist lx_map_assoc(struct lxmap, struct lxvalue key);
struct lxvalue lx_map_ref(struct lxmap, struct lxvalue key);
bool lx_map_has(struct lxmap, struct lxvalue key, struct lxvalue *value);
struct lxmap lx_map_remove(struct lxstate *, struct lxmap, struct lxvalue key);
struct lxmap lx_map_diff(struct lxstate *, struct lxmap, struct lxset);

struct lxmap lx_map_merge(struct lxstate *, struct lxmap, struct lxmap);
struct lxset lx_map_keys(struct lxstate *, struct lxmap);

struct lxlist lx_map_nth(struct lxmap map, lxint n);

/* low level API for traversal */
struct lxlist lx_map_entry(struct lxmap);
struct lxmap lx_map_left(struct lxmap);
struct lxmap lx_map_right(struct lxmap);

struct lxmap lx_map_filter(
	struct lxstate *,
	struct lxmap map,
	bool predicate(struct lxvalue key, struct lxvalue value, void *),
	void *param);
