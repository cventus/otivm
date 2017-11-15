struct gl_api;
struct xylo_shapes;

int xylo_init_shapes(struct xylo_shapes *shapes, struct gl_api *api);
void xylo_term_shapes(struct xylo_shapes *shapes, struct gl_api *api);

/* set shape solid color */
void xylo_shapes_set_color4fv(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	float const color[4]);

/* set model-view-projection matrix */
void xylo_shapes_set_mvp(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	float const *mvp);

void xylo_shapes_set_sample_clip(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	GLsizei n,
	float const *sample_clip);

void xylo_shapes_set_sample_offset(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	GLsizei n,
	float const *sample_offset);

/* set object ID */
void xylo_shapes_set_object_id(
	struct xylo_shapes *shapes,
	struct gl_core33 const *restrict gl,
	unsigned object_id);
