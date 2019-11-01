static inline struct lxstring ref_to_string(struct lxvalue ref)
{
	return (struct lxstring) { .value = ref };
}

static inline struct lxstring deref_string(union lxcell const *c)
{
	return ref_to_string(deref(c, lx_string_tag));
}
