struct lxstate
{
	struct lx_config config;
	union lxvalue root;
	union lxcell *begin, *mid, *end;
	struct lxalloc alloc;
};
