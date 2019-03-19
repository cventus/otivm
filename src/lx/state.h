struct lxstate
{
	struct lx_config config;
	union lxvalue root;
	union lxcell *begin, *mid, *end;
	struct lxalloc alloc;
};

int lx_resize_heap(struct lxstate *state, size_t new_size);
int lx_gc(struct lxstate *state);
