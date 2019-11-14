#define ceil_div(a, b) (((a) + (b) - 1)/(b))

union lxcell;

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

enum lx_state_status {
	lx_state_ok = 0,
	lx_state_heap_size,
};

struct lxstate
{
	jmp_buf restart;
	struct lxheap *heap;
	struct lxalloc alloc;
	volatile int iterations;
	volatile enum lx_state_status status;
};

void lx_handle_out_of_memory(struct lxstate *state);
void lx_set_cell_data(union lxcell *data, struct lxvalue val);
int lx_reserve_tagged(struct lxalloc *, size_t n, struct lxvalue *ref);

struct lxvalue lx_compact(
	struct lxvalue root,
	union lxcell *from,
	struct lxalloc *to,
	void *bitset,
	size_t bitset_size);
