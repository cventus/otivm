/* A list of triangles for some list of vertices */
struct triangle_set
{
	size_t n;
	unsigned (*indices)[3];
};

/* Create a constrained delauney triangulation of the vertices, where the
   optional edges in `constraint` are maintained. */
struct triangle_set *triangle_set_triangulate(
	float const (*vertices)[2],
	size_t vertex_count,
	unsigned const (*edges)[2],
	size_t edge_count);

void triangle_set_free(struct triangle_set *);
