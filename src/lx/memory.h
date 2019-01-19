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

struct lxmem
{
	jmp_buf escape;
	struct lxref tag_free;
	union lxcell *raw_free;
	enum out_of_memory oom;
};
