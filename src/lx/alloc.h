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

static inline size_t alloc_free_count(struct lxalloc const *alloc)
{
	return ref_cell(alloc->tag_free) - alloc->raw_free;
}

static inline size_t alloc_low_used_count(struct lxalloc const *alloc)
{
	return alloc->raw_free - alloc->min_addr;
}

static inline size_t alloc_high_used_count(struct lxalloc const *alloc)
{
	return alloc->max_addr - ref_cell(alloc->tag_free);
}
