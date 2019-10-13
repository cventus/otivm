#define ceil_div(a, b) (((a) + (b) - 1)/(b))

typedef unsigned char lxtag;

static inline struct lxref mkref(enum lx_tag, unsigned, union lxcell const *);

enum out_of_memory
{
	OOM_COMPACT = 1,
	OOM_GROW
};

union lxcell
{
	lxtag t[CELL_SIZE];
	lxint i;
	lxfloat f;
};

/* Struct lxalloc contains pointers into a memory region for allocating memory
   from both ends. Allocation structures are used in two cases: 1) the
   allocation space used by cons, and 2) to-space into which live objects are
   copied during garbage collection. There is no explicit type information
   stored in lxalloc, but the type of allocator is implied by the code path. */
struct lxalloc
{
	/* minimum/maximum addresses used by allocator */
	union lxcell *min_addr, *max_addr;

	/* list allocation pointer, moves towards `raw_free` */
	struct lxref tag_free;

	/* non-list allocation pointer, moves towards `tag_free` */
	union lxcell *raw_free;
};

struct lxmem
{
	jmp_buf escape;
	struct lxalloc alloc;
	enum out_of_memory oom;
};

/* Tagged cells are allocated from high addresses towards low addresses in a
   *cons*-space, so that cons can create compact lists directly when the tail
   was allocated just before. */
static inline void init_cons(
	struct lxalloc *alloc,
	union lxcell *min_addr,
	size_t size)
{
	alloc->min_addr = min_addr;
	alloc->max_addr = min_addr + size;

	alloc->raw_free = min_addr;
	alloc->tag_free = mkref(lx_list_tag, 0, alloc->max_addr);
}

/* A to-space is the target space during garbage collection. During that time
   tagged cells are allocated from low addresses to high so that lists can be
   copied from front to back without knowing the length of the list. */
static inline void init_tospace(
	struct lxalloc *alloc,
	union lxcell *min_addr,
	size_t size)
{
	alloc->min_addr = min_addr;
	alloc->max_addr = min_addr + size;

	alloc->raw_free = alloc->max_addr;
	alloc->tag_free = mkref(lx_list_tag, 0, alloc->min_addr);
}

/* swap raw_free and tag_free */
static inline void swap_allocation_pointers(struct lxalloc *alloc)
{
	union lxcell *cell;

	cell = alloc->raw_free;
	alloc->raw_free = (union lxcell *)alloc->tag_free.cell;
	if (alloc->tag_free.offset) {
		alloc->raw_free += 1 + alloc->tag_free.offset;
	}
	alloc->tag_free.offset = 0;
	alloc->tag_free.cell = cell;
}

static inline size_t alloc_cell_count(struct lxalloc const *alloc)
{
	return alloc->max_addr - alloc->min_addr;
}

static inline size_t alloc_free_count(struct lxalloc const *alloc)
{
	return alloc->tag_free.cell - alloc->raw_free;
}

static inline size_t alloc_low_used_count(struct lxalloc const *alloc)
{
	return alloc->raw_free - alloc->min_addr;
}

static inline size_t alloc_high_used_count(struct lxalloc const *alloc)
{
	return alloc->max_addr - alloc->tag_free.cell;
}

static inline size_t mark_cell_count(size_t semispace_cells)
{
	return ceil_div(semispace_cells * 2, LX_BITS);
}

void lx_set_cell_data(union lxcell *data, union lxvalue val);
int lx_reserve_tagged(struct lxalloc *, size_t n, struct lxref *ref);

union lxvalue lx_compact(
	union lxvalue root,
	union lxcell *from,
	struct lxalloc *to,
	void *bitset,
	size_t bitset_size);
