struct lxheap
{
	/* copy of heap configuration */
	struct lx_config config;
	struct lxvalue root;
	union lxcell *first, *second, *bitset, *end;
	struct lxalloc alloc;
};

int lx_resize_heap(struct lxheap *heap, size_t new_size);
void lx_gc(struct lxheap *heap);
