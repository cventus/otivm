static inline struct lxvalue mkref(
	enum lx_tag tag,
	unsigned offset,
	union lxcell const *c)
{
	return (struct lxvalue) {
		.tag = tag,
		.offset = offset,
		.s = (char const *)c
	};
}

static inline union lxcell *ref_cell(struct lxvalue ref)
{
	return (union lxcell *)ref.s;
}

static inline lxtag *ref_tag(struct lxvalue ref)
{
	assert(ref.tag == lx_list_tag || ref.tag == lx_map_tag);
	return ref_cell(ref)->t + ref.offset;
}

static inline union lxcell *ref_data(struct lxvalue ref)
{
	assert(ref.tag == lx_list_tag || ref.tag == lx_map_tag);
	return ref_cell(ref) + ref.offset + 1;
}

static inline bool ref_lt(struct lxvalue a, struct lxvalue b)
{
        if (ref_cell(a) < ref_cell(b)) { return true; }
        if (ref_cell(a) > ref_cell(b)) { return false; }
	return a.offset < b.offset;
}

static inline bool ref_eq(struct lxvalue a, struct lxvalue b)
{
        return a.s == b.s && a.offset == b.offset;
}

static inline bool ref_is_nil(struct lxvalue ref)
{
        return ref.s == NULL && ref.offset == 0;
}

static inline struct lxvalue ref_advance(struct lxvalue ref, lxuint n)
{
	size_t cells, offset;
	union lxcell const *cell;

	offset = ref.offset + n;
	cells = offset/CELL_SIZE;
	cell = ref_cell(ref) + cells*SPAN_LENGTH;
	return mkref(ref.tag, offset%CELL_SIZE, cell);
}

static inline lxint ref_diff(struct lxvalue a, struct lxvalue b)
{
	ptrdiff_t cells, offset;

	cells = ref_cell(a) - ref_cell(b);
	cells -= cells / (CELL_SIZE + 1); /* cells * 4/5 */
	offset = (ptrdiff_t)a.offset - (ptrdiff_t)b.offset;
	return cells + offset;
}

static inline struct lxvalue backward(struct lxvalue ref)
{
	union lxcell const *newc = ref_cell(ref);
	unsigned newoff = (ref.offset - 1) & OFFSET_MASK;
	if ((newoff ^ OFFSET_MASK) == 0) { newc -= SPAN_LENGTH; }
	return mkref(ref.tag, newoff, newc);
}

static inline struct lxvalue forward(struct lxvalue ref)
{
	union lxcell const *newc = ref_cell(ref);
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
	struct lxvalue ref)
{
	if (ref_is_nil(ref)) {
		setnilref(c);
	} else {
		/* the difference between cells is always a multiple of
		   CELL_SIZE */
		c->i = (unsigned char *)ref.s - (unsigned char *)begin;
		assert((c->i & OFFSET_MASK) == 0);

		/* store offset in the unused least significant bits in the
		   positive/negative integer (assumes 2's complement) */
		c->i |= ref.offset & OFFSET_MASK;
	}
}

static inline struct lxvalue dexref(
	union lxcell const *c,
	union lxcell const *begin,
	enum lx_tag tag)
{
	ptrdiff_t cell_offset, span_offset;
	unsigned char *p;

	if (isnilref(c)) {
		return mkref(tag, 0, NULL);
	} else {
		cell_offset = c->i & OFFSET_MASK;
		span_offset = c->i & ~OFFSET_MASK;
		p = (unsigned char *)begin + span_offset;
		return mkref(tag, cell_offset, (union lxcell *)p);
	};
}

/* auto-relative references are offsets relative to the cell itself */
static inline void setref(union lxcell *c, struct lxvalue ref)
{
	setxref(c, c, ref);
}

static inline struct lxvalue deref(union lxcell const *c, enum lx_tag tag)
{
	return dexref(c, c, tag);
}
