struct xylo_rgss;
struct gl_api;
struct gl_core33;

int xylo_init_rgss(struct xylo_rgss *rgss, struct gl_api *api);
void xylo_term_rgss(struct xylo_rgss *rgss, struct gl_api *api);

/* compose center and corner samples */
void xylo_rgss_draw(
	struct xylo_rgss *rgss,
	struct gl_core33 const *restrict gl);

/* specify texture unit holding the pixel center samples */
void xylo_rgss_set_tex_unit(
	struct xylo_rgss *rgss,
	struct gl_core33 const *restrict gl,
	GLuint unit);
