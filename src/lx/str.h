static inline union lxvalue ref_to_string(struct lxref ref)
{
	assert(ref.tag == lx_string_tag);
	assert(ref.offset == 0);
	return (union lxvalue) {
		.tag = lx_string_tag,
		.s = (char const *)(ref.cell + 1)
	};
}

static inline struct lxref string_to_ref(union lxvalue val)
{
	assert(val.tag == lx_string_tag);
	return mkref(lx_string_tag, 0, (union lxcell *)val.s - 1);
}

static inline struct lxref deref_string(union lxcell const *c)
{
	return deref(c, lx_string_tag);
}
