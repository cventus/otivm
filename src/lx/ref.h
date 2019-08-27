static inline struct lxref mkref(
	enum lx_tag tag,
	unsigned offset,
	union lxcell const *c)
{
	return (struct lxref) { tag, offset, c };
}

static inline bool ref_lt(struct lxref a, struct lxref b)
{
        return a.cell < b.cell || (a.cell == b.cell && a.offset < b.offset);
}

static inline bool ref_eq(struct lxref a, struct lxref b)
{
        return a.cell == b.cell && a.offset == b.offset;
}

static inline lxtag *ref_tag(struct lxref ref)
{
	assert(ref.tag == lx_list_tag || ref.tag == lx_tree_tag);
	return (lxtag *)ref.cell->t + ref.offset;
}

static inline union lxcell *ref_data(struct lxref ref)
{
	assert(ref.tag == lx_list_tag || ref.tag == lx_tree_tag);
	return (union lxcell *)ref.cell + ref.offset + 1;
}

static inline struct lxref ref_advance(struct lxref ref, lxuint n)
{
	size_t cells, offset;

	offset = ref.offset + n;
	cells = offset/CELL_SIZE;
	return mkref(ref.tag, offset%CELL_SIZE, ref.cell + cells*SPAN_LENGTH);
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
	return mkref(ref.tag, newoff, newc);
}

static inline struct lxref forward(struct lxref ref)
{
	union lxcell const *newc = ref.cell;
	unsigned newoff = (ref.offset + 1) & OFFSET_MASK;
	if (newoff == 0) { newc += SPAN_LENGTH; }
	return mkref(ref.tag, newoff, newc);
}

/* Auto-relative references must refer to a tag word in a span, which is at
   an absolute offset no less than sizeof(union lxcell). A zero offset is
   therefore illegal and can be used to indicate NULL */
static inline void setnilref(union lxcell *c)
{
	c->i = 0;
}

static inline bool isnilref(union lxcell const *c)
{
	return c->i == 0;
}

/* cross references are offsets from the start of a memory region */
static inline void setxref(
	union lxcell *c,
	union lxcell const *begin,
	struct lxref ref)
{
	if (ref.cell == NULL) {
		setnilref(c);
	} else {
		/* the difference between cells is always a multiple of
		   CELL_SIZE */
		c->i = (unsigned char *)ref.cell - (unsigned char *)begin;
		assert((c->i & OFFSET_MASK) == 0);

		/* store offset in the unused least significant bits in the
		   positive/negative integer (assumes 2's complement) */
		c->i |= ref.offset & OFFSET_MASK;
	}
}

static inline struct lxref dexref(
	union lxcell const *c,
	union lxcell const *begin,
	enum lx_tag tag)
{
	ptrdiff_t cell_offset, span_offset;
	unsigned char const *p;

	if (isnilref(c)) {
		return mkref(tag, 0, NULL);
	} else {
		cell_offset = c->i & OFFSET_MASK;
		span_offset = c->i & ~OFFSET_MASK;
		p = (unsigned char const *)begin + span_offset;

		return mkref(tag, cell_offset, (union lxcell const*)p);
	};
}

/* auto-relative references are offsets relative to the cell itself */
static inline void setref(union lxcell *c, struct lxref ref)
{
	setxref(c, c, ref);
}

static inline struct lxref deref(union lxcell const *c, enum lx_tag tag)
{
	return dexref(c, c, tag);
}
