struct xylo_quincunx;
struct gl_api;
struct gl_core33;

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
void xylo_quincunx_set_center_unit(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl,
	GLuint unit);

/* specify texture unit holding the pixel corner samples */
void xylo_quincunx_set_corner_unit(
	struct xylo_quincunx *quincunx,
	struct gl_core33 const *restrict gl,
	GLuint unit);
