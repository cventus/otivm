static inline struct lxref ref_advance(struct lxref ref, lxuint n)
{
	size_t cells, offset;

	offset = ref.offset + n;
	cells = offset / CELL_SIZE;
	ref.cell += cells * (CELL_SIZE + 1);
	ref.offset = offset % CELL_SIZE;
	return ref;
}

static inline lxint ref_diff(struct lxref a, struct lxref b)
{
	ptrdiff_t cells, offset;

	cells = a.cell - b.cell;
	cells -= cells / (CELL_SIZE + 1); /* cells * 4/5 */
	offset = (ptrdiff_t)a.offset - (ptrdiff_t)b.offset;
	return cells + offset;
}

static inline struct lxref backward(struct lxref ref)
{
	union lxcell const *newc = ref.cell;
	unsigned newoff = (ref.offset - 1) & OFFSET_MASK;
	if ((newoff ^ OFFSET_MASK) == 0) { newc -= SPAN_LENGTH; }
	return (struct lxref) { ref.tag, newoff, newc };
}

static inline struct lxref forward(struct lxref ref)
{
	union lxcell const *newc = ref.cell;
	unsigned newoff = (ref.offset + 1) & OFFSET_MASK;
	if (newoff == 0) { newc += SPAN_LENGTH; }
	return (struct lxref) { ref.tag, newoff, newc };
}

static inline lxtag *ref_tag(struct lxref ref)
{
	return (lxtag *)ref.cell->t + ref.offset;
}

static inline union lxcell *ref_data(struct lxref ref)
{
	return (union lxcell *)ref.cell + ref.offset + 1;
}

static inline void setref(union lxcell *c, struct lxref ref)
{
	/* the difference between cells is always a multiple of CELL_SIZE */
	c->i = (char *)ref.cell - (char *)c;
	assert((c->i & OFFSET_MASK) == 0);

	/* store offset in the unused least significant bits in the
	   positive/negative integer (assumes 2's complement) */
	c->i |= ref.offset & OFFSET_MASK;
}

static inline struct lxref deref(union lxcell const *c, enum lx_tag tag)
{
	return (struct lxref) {
		tag,
		c->i & OFFSET_MASK,
		(union lxcell const*)((char *)c + (c->i & ~OFFSET_MASK))
	};
}
