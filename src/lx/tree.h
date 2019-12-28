static inline struct lxmap ref_to_map(struct lxvalue ref)
{
	return (struct lxmap) { .value = ref };
}

static inline struct lxmap deref_map(union lxcell const *c)
{
	return ref_to_map(deref(c, lx_map_tag));
}

static inline bool map_eq(struct lxmap a, struct lxmap b)
{
	return ref_eq(a.value, b.value);
}

static inline bool is_leaf_node(struct lxmap map)
{
	return lx_map_size(map) == 1;
}
