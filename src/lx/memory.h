#define ceil_div(a, b) (((a) + (b) - 1)/(b))

typedef unsigned char lxtag;

enum out_of_memory
{
	OOM_COMPACT = 1,
	OOM_GROW
};

union lxcell
{
	lxtag t[CELL_SIZE];
	lxint i;
#ifdef lxfloat
	lxfloat f;
#endif
};

/* An lxspace is a memory region from which you can allocate memory from both
   ends. There are three types of spaces: 1) the allocation space used by cons,
   2) the from-space which contains live and garbage objects during garbage
   collection, and 3) to-space into which live objects are copied during
   garbage collection. There is no explicit type information stored with
   spaces, but the type of space known inplicitly from the code path. */
struct lxspace
{
	struct lxref tag_free;
	union lxcell *begin, *end, *raw_free;
};

struct lxmem
{
	jmp_buf escape;
	struct lxspace space;
	enum out_of_memory oom;
};

/* Tagged cells are allocated from high addresses towards low addresses in an
   allocation space, so that cons can create compact lists directly when the
   tail was allocated immediately prior. */
static inline void init_allocspace(
	struct lxspace *space,
	union lxcell *p,
	size_t n)
{
	space->begin = p;
	space->end = p + n;

	space->raw_free = space->begin;
	space->tag_free.offset = 0;
	space->tag_free.cell = space->end;
}

static inline void set_tospace(struct lxspace *space)
{
	space->tag_free.offset = 0;
	space->tag_free.cell = space->begin;
	space->raw_free = space->end;
}

/* In a to-space, tagged cells are allocated from low addresses to high so that
   lists can be copied from front to back without knowing the length of the
   list. */
static inline void init_tospace(
	struct lxspace *space,
	union lxcell *p,
	size_t n)
{
	space->begin = p;
	space->end = p + n;

	set_tospace(space);
}

static inline void tospace_to_allocspace(struct lxspace *space)
{
	union lxcell *cell;

	cell = space->raw_free;
	space->raw_free = (union lxcell *)space->tag_free.cell;
	if (space->tag_free.offset) {
		space->raw_free += 1 + space->tag_free.offset;
	}
	space->tag_free.offset = 0;
	space->tag_free.cell = cell;
}

static inline size_t mark_cell_count(size_t tagged_cells)
{
	return (tagged_cells * 2 + LX_BITS - 1) / LX_BITS;
}

static inline size_t space_size(struct lxspace const *space)
{
	return space->end - space->begin;
}

static inline size_t space_span_cells(struct lxspace const *space)
{
	return space->end - space->tag_free.cell;
}

static inline size_t space_spans(struct lxspace const *space)
{
	return space_span_cells(space) / SPAN_LENGTH;
}

static inline size_t space_tagged_cells(struct lxspace const *space)
{
	return space_spans(space) * CELL_SPAN;
}

static inline size_t space_raw_cells(struct lxspace const *space)
{
	return space->raw_free - space->begin;
}

/* total amount of cells used (including garbage and mark bits) */
static inline size_t space_used(struct lxspace const *space)
{
	size_t raw_cells, spans, mark_cells;

	raw_cells = space->raw_free - space->begin;
	spans = space_spans(space);
	mark_cells = mark_cell_count(spans * CELL_SPAN);

	return raw_cells + spans*SPAN_LENGTH + mark_cells;
}

union lxvalue lx_compact(
	union lxvalue root,
	struct lxspace *from,
	struct lxspace *to);
