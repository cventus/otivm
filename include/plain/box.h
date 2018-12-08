
struct plain_box;
struct plain_viewport;
struct plain_rectf;
struct plain_scene;

struct plain_rectf { float pt[2], sz[2]; };

/* A box is a rectangle in a scene which has a scene defined ID for associating
 * its location and size with other properties. In addition to location and
 * size, boxes have an inherent stacking order.
 */
struct plain_box {
	unsigned int id, stack;
	struct plain_rectf scene; /* relative to scene origin */
};

struct plain_t_scene {
	/* Get boxes in the specific viewport. If the viewport isn't rendered
	 * in the *natural* size on screen, then the contents will appear
	 * zoomed by effectively emulating a smaller or larger screen. On the
	 * downside, boxes which have sizes adapted to e.g. physical size will
	 * then be miscalibrated.
	 */
	int (*viewport)(
		struct plain_scene *,
		struct plain_rectf const *,
		struct plain_viewport **viewport);
};

/* Find the first box, if any, that intersects the point pt. */
struct plain_box const *plain_viewport_point(
	struct plain_viewport *,
	float const pt[2]);

