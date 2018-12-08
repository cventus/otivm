/* composite Rational Quadratic Bezier curve scene */

struct rb2_buffer {
	struct wbuf points;
};

void rb2_init_buffer(struct qbc_buffer *buf);
void rb2_term_buffer(struct qbc_buffer *buf);

/* Extend path with two new control points. The second one is the new end point
 * and its location is provided in parameters x and y. The position of the
 * first point is interpolated based on the previous point and the new end
 * point. If the buffer is empty, only a single starting point is added. 
 */
float *rb2_buffer_interpolate(struct qbc_buffer *buf, float x, float y);

/* Extend path with two new control points, (x2, y2) becomes the new end. */
float *rb2_buffer_append(
	struct qbc_buffer *buf,
	float x1,
	float y1,
	float x2,
	float y2);

/* Find a location l on the composite curve that's closest to point input point
 * (x, y). Let i = floor(l) and t = l - i, then the control point P0 is at
 * *i+0, P1 at *i+1, and P2 at *i+2, and the offset in *t. The closest curve
 * position is found by evaluating the rational quadratic bezier function with
 * those parameters.
 */
float rb2_buffer_closest(struct qbc_buffer *, float x, float y);

/* Split the curve between curve points i and i+1 (i.e. control points i*2 and
 * (i+1)*2) at t in (0, 1) so that the shape of the curve is preserved.
 */
float *rb2_buffer_split(struct qbc_buffer *buf, unsigned i, float t);

/* Get control point i (x, y, w) */
float *qbc_buffer_point(struct qbc_buffer *buf, unsigned i);

struct rb2_scene_config {
	float point_sz[2];
};


/* Generates one kind of point marker for each even point (ones on the curve),
 * and another kind for the odd ones (curving control points). Adds point for
 * closest point on the curve, when a curve is about to be split one.
 */
struct plain_scene *create_rb2_scene(
	struct qbc_buffer *qbc,
	struct rb2_scene_config const *);

extern struct plain_t_scene const qbc_t_scene;


/*


create buffer
create scene
...


read input
- ctrl -> show new point
- shift -> show split point
- ctrl+click/shift+click -> add/split point
- drag -> move selected point/select many points
- right-click -> select point
- right-drag -> select area point
- right-drag -> select area point
- or why not modal
...


handle input
- create new point
- calculate closest point
- Check which point was clicked, if a selection event happened
...


Pan view
- kinetic animation
...


Query viewport
- if dirty
- if new view is OOB

...

Create/update vertex buffers
...


render polygons
render scene
render buttons


*/

