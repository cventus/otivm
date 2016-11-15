
struct gl_api;
struct xylo;
struct xylo_shape;
struct xylo_glshape;
struct xylo_glshape_set;

struct xylo *make_xylo(struct gl_api *);
void free_xylo(struct xylo *);
void xylo_begin(struct xylo *xylo);
void xylo_end(struct xylo *xylo);
void xylo_draw_shape(struct xylo *xylo, struct xylo_glshape *shape);
struct xylo_glshape_set *xylo_make_glshape_set(
	struct gl_api *gl,
	size_t n,
	struct xylo_shape const shapes[n]);

void xylo_set_shape_set(struct xylo *xylo, struct xylo_glshape_set *set);
int xylo_free_glshape_set(
	struct xylo_glshape_set *set,
	struct gl_api *gl);

struct xylo_glshape *xylo_get_glshape(
	struct xylo_glshape_set *set,
	size_t i);

void xylo_set_to_clip(struct xylo *xylo, float const *matrix);
void xylo_set_to_world(struct xylo *xylo, float const *matrix);

