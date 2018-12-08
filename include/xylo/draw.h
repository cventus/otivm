/* Tree-shaped group of groups of drawable shapes */
struct xylo;
struct xylo_outline;
struct xylo_outline_set;
struct xylo_mesh;
struct xylo_mesh_set;

enum xylo_dtype
{
	xylo_dlist,
	xylo_doutline,
	xylo_dmesh,

	xylo_dmax
};

struct xylo_draw
{
	enum xylo_dtype type;
};

struct xylo_draw_transform
{
	float m22[4], pos[2];
};

struct xylo_draw_style
{
	float color[4];
};

struct xylo_dlist
{
	struct xylo_draw draw;
	struct gbuf elements; /* struct xylo_draw *[] */
};

struct xylo_doutline
{
	struct xylo_draw draw;

	unsigned id;
	struct xylo_draw_transform transform;
	struct xylo_draw_style style;
	struct xylo_outline const *outline;
};

struct xylo_dmesh
{
	struct xylo_draw draw;

	unsigned id;
	struct xylo_draw_transform transform;
	struct xylo_draw_style style;
	struct xylo_mesh const *mesh;
};

void xylo_init_dlist(struct xylo_dlist *);
void xylo_term_dlist(struct xylo_dlist *);
struct xylo_dlist *xylo_dlist_cast(struct xylo_draw *);
size_t xylo_dlist_length(struct xylo_dlist *list);
int xylo_dlist_insert(
	struct xylo_dlist *list,
	struct xylo_draw *element,
	ptrdiff_t index);
int xylo_dlist_append(struct xylo_dlist *list, struct xylo_draw *element);
int xylo_dlist_remove(struct xylo_dlist *, ptrdiff_t index);
ptrdiff_t xylo_dlist_indexof(struct xylo_dlist *, struct xylo_draw *d);

void xylo_init_doutline(struct xylo_doutline *, struct xylo_outline const *);
void xylo_term_doutline(struct xylo_doutline *);
struct xylo_doutline *xylo_doutline_cast(struct xylo_draw *);

void xylo_init_dmesh(struct xylo_dmesh *, struct xylo_mesh const *);
void xylo_term_dmesh(struct xylo_dmesh *);
struct xylo_dmesh *xylo_dmesh_cast(struct xylo_draw *);
