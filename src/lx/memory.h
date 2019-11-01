#define ceil_div(a, b) (((a) + (b) - 1)/(b))

union lxcell;

enum out_of_memory
{
	OOM_COMPACT = 1,
	OOM_GROW
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

	/* non-list allocation pointer, moves towards `tag_free` */
	union lxcell *raw_free;

	/* list allocation pointer, moves towards `raw_free` */
	struct lxvalue tag_free;
};

struct lxmem
{
	jmp_buf escape;
	struct lxalloc alloc;
	enum out_of_memory oom;
};

void lx_set_cell_data(union lxcell *data, struct lxvalue val);
int lx_reserve_tagged(struct lxalloc *, size_t n, struct lxvalue *ref);

struct lxvalue lx_compact(
	struct lxvalue root,
	union lxcell *from,
	struct lxalloc *to,
	void *bitset,
	size_t bitset_size);
