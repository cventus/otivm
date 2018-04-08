struct gl_api;
struct gl_core33;

struct xylo_quincunx;
struct xylo_rgss;

int xylo_init_quincunx(struct xylo_quincunx *quincunx, struct gl_api *api);
void xylo_term_quincunx(struct xylo_quincunx *quincunx, struct gl_api *api);

/* compose center and corner samples */
void xylo_quincunx_draw(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl);

/* specify the normalized width of a pixel */
void xylo_quincunx_set_pixel_size(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl,
	float x_size, float y_size);

/* specify the normalized width of a pixel */
void xylo_quincunx_set_pixel_size2fv(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl,
	float const *pixel_size);

/* specify texture unit holding the pixel center samples */
void xylo_quincunx_set_tex_unit(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl,
	GLuint unit);

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
