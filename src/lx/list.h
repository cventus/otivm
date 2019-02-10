enum cdr_code
{
	cdr_nil = 0,
	cdr_link = 1,
	cdr_adjacent = 2
};

static inline enum cdr_code lxtag_cdr(lxtag h)
{
	return h >> 6;
}

static inline enum lx_tag lxtag_tag(lxtag h)
{
	return h & 0x3f;
}

static inline enum cdr_code list_cdr_code(struct lxlist list)
{
	return lxtag_cdr(*ref_tag(list.ref));
}

static inline enum lx_tag list_car_tag(struct lxlist list)
{
	return lxtag_tag(*ref_tag(list.ref));
}

static inline union lxcell const *list_car(struct lxlist list)
{
	return ref_data(list.ref);
}

static inline struct lxlist ref_to_list(struct lxref ref)
{
	return (struct lxlist) { .ref = ref };
}

static inline struct lxlist list_backward(struct lxlist list)
{
	return ref_to_list(backward(list.ref));
}

static inline struct lxlist list_forward(struct lxlist list)
{
	return ref_to_list(forward(list.ref));
}

static inline struct lxlist deref_list(union lxcell const *c)
{
	return ref_to_list(deref(c, lx_list_tag));
}

static inline bool list_eq(struct lxlist a, struct lxlist b)
{
	return a.ref.cell == b.ref.cell && a.ref.offset == b.ref.offset;
}
