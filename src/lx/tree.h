static inline struct lxtree ref_to_tree(struct lxvalue ref)
{
	return (struct lxtree) { .value = ref };
}

static inline struct lxtree deref_tree(union lxcell const *c)
{
	return ref_to_tree(deref(c, lx_tree_tag));
}

static inline bool tree_eq(struct lxtree a, struct lxtree b)
{
	return ref_eq(a.value, b.value);
}

static inline bool is_leaf_node(struct lxtree tree)
{
	return lx_tree_size(tree) == 1;
}
