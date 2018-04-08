/* Tree-shaped group of groups of drawable shapes */
struct xylo;
struct xylo_outline;
struct xylo_outline_set;

enum xylo_dtype
{
	xylo_dlist,
	xylo_dshape,

	xylo_dmax
};

struct xylo_draw
{
	enum xylo_dtype type;
};

struct xylo_dlist
{
	struct xylo_draw draw;
	struct gbuf elements; /* struct xylo_draw *[] */
};

struct xylo_dshape
{
	struct xylo_draw draw;

	/* transformation */
	float m22[4], pos[2];

	/* shape to draw */
	unsigned id;
	float color[4];
	struct xylo_outline const *outline;
};

void xylo_init_dlist(struct xylo_dlist *);
void xylo_term_dlist(struct xylo_dlist *);
struct xylo_dlist *xylo_dlist_cast(struct xylo_draw *);
size_t xylo_dlist_length(struct xylo_dlist *list);
int xylo_dlist_insert(
	struct xylo_dlist *list,
	struct xylo_draw *element,
	ptrdiff_t index);
int xylo_dlist_append(
	struct xylo_dlist *list,
	struct xylo_draw *element);
int xylo_dlist_remove(struct xylo_dlist *, ptrdiff_t index);
ptrdiff_t xylo_dlist_indexof(struct xylo_dlist *, struct xylo_draw *d);

void xylo_init_dshape(
	struct xylo_dshape *,
	float const color[4],
	struct xylo_outline const *);
void xylo_init_dshape_id(
	struct xylo_dshape *,
	unsigned id,
	float const color[4],
	struct xylo_outline const *);
void xylo_term_dshape(struct xylo_dshape *);
struct xylo_dshape *xylo_dshape_cast(struct xylo_draw *);
