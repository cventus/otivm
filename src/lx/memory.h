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

static inline void init_space(struct lxspace *space, union lxcell *p, size_t n)
{
	space->begin = p;
	space->end = p + n;

	space->raw_free = space->begin;
	space->tag_free.offset = 0;
	space->tag_free.cell = space->end;
}

static inline size_t cell_count(struct lxspace const *space)
{
	size_t used_cells = space->end - space->tag_free.cell;
	size_t used_frames = ceil_div(used_cells, CELL_SIZE + 1);
	return used_frames;
}

static inline size_t mark_cell_count(struct lxspace const *space)
{
	return (cell_count(space) * 2 + LX_BITS - 1) / LX_BITS;
}

static inline size_t space_available(struct lxspace const *space)
{
	size_t empty_cells, mark_cells;

	empty_cells = space->tag_free.cell - space->raw_free;
	/* reserve two mark-bits per cell */
	mark_cells = mark_cell_count(space);

	return empty_cells >= mark_cells ? empty_cells - mark_cells : 0;
}
