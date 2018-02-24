typedef int eref;

struct eset
{
	struct wbuf edges, data;
	eref free;
};

void init_quadedge(eref *, float2 **data, eref e0);

void init_eset(struct eset *set);
eref eset_max_edge(struct eset *set);
int eset_alloc(struct eset *set, size_t n, ...);
void term_eset(struct eset *set);
void eset_splice(struct eset *, eref a, eref b);
void eset_connect(struct eset *, eref a, eref b, eref c);
void eset_delete(struct eset *, eref e);

static inline eref mkref(eref e0, unsigned r) { return (e0 & ~0x3)|(r & 0x3); }
static inline eref rot(eref e) { return mkref(e, e + 1); }
static inline eref sym(eref e) { return mkref(e, e + 2); }
static inline eref invrot(eref e) { return mkref(e, e + 3); }

static inline eref onext(struct eset *set, eref e)
{
	assert(e >= 0);
	assert((size_t)e < wbuf_nmemb(&set->edges, sizeof(eref)));
	return ((eref *)set->edges.begin)[e];
}

static inline eref oprev(struct eset *set, eref e)
{
	return rot(onext(set, rot(e)));
}

static inline eref dnext(struct eset *set, eref e)
{
	return sym(onext(set, sym(e)));
}

static inline eref dprev(struct eset *set, eref e)
{
	return invrot(onext(set, invrot(e)));
}

static inline eref lnext(struct eset *set, eref e)
{
	return rot(onext(set, invrot(e)));
}

static inline eref lprev(struct eset *set, eref e)
{
	return sym(onext(set, e));
}

static inline eref rnext(struct eset *set, eref e)
{
	return invrot(onext(set, rot(e)));
}

static inline eref rprev(struct eset *set, eref e)
{
	return onext(set, sym(e));
}

static inline float2 **org(struct eset *set, eref e)
{
	assert(e >= 0);
	assert(((size_t)e >> 1) < wbuf_nmemb(&set->data, sizeof(float2*)));
	return (float2 **)set->data.begin + (e >> 1);
}

static inline float2 **dest(struct eset *set, eref e)
{
	return org(set, sym(e));
}
