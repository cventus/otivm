struct gl_core33;
struct xylo_fb;

void xylo_init_fb(
	struct gl_core33 const *restrict gl,
	struct xylo_fb *fb,
	_Bool object_buffer);

void xylo_term_fb(struct gl_core33 const *restrict gl, struct xylo_fb *fb);

void xylo_fb_resize(
	struct gl_core33 const *restrict gl,
	struct xylo_fb *fb,
	GLsizei width,
	GLsizei height);

unsigned xylo_fb_object_id(
	struct gl_core33 const *restrict gl,
	struct xylo_fb *fb,
	GLsizei x,
	GLsizei y);
