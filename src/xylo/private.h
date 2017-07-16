struct gl_api;

struct xylo;
struct xylo_shape;
struct xylo_glshape;
struct xylo_glshape_set;

enum
{
	LINE_POS_ATTRIB = 0,
	WEIGHT_POS_ATTRIB = 1,
	WEIGHT_VAL_ATTRIB = 2
};

void xylo_begin(struct xylo *xylo);
void xylo_end(struct xylo *xylo);
void xylo_draw_glshape(struct xylo *xylo, struct xylo_glshape const *shape);

void xylo_set_shape_set(struct xylo *xylo, struct xylo_glshape_set *set);
struct xylo_glshape const *xylo_get_glshape(
	struct xylo_glshape_set *set,
	size_t i);

void xylo_set_to_clip(struct xylo *xylo, float const *matrix);
void xylo_set_to_world(struct xylo *xylo, float const *matrix);
