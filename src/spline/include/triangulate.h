/* A list of triangles for some list of vertices */
struct triangle_set
{
	size_t n;
	unsigned indices[][3];
};

/* Return the size of a `struct triangle_set`, suitable for allocation, that
   can hold the given number of triangles */
size_t triangle_set_size(size_t triangle_count);

/* Create a constrained delauney triangulation of the vertices, where the
   optional triangles in `constraint` are maintained. Degenerate triangles,
   where an index is repeated, can be used to specify edges. */
struct triangle_set *triangulate(
	float const (*vertices)[2],
	size_t vertex_count,
	struct triangle_set const *constraints);
