
#include <stdio.h>

struct wf_map
{
	char const *filename;
};

struct wf_material
{
	char const *name;
	int illum;
	struct wf_map const map_ka, map_kd, map_ks, map_ns, map_d, map_bump;
	float ka[3], kd[3], ks[3], ns, d;
};

struct wf_mtllib
{
	size_t n;
	struct wf_material m[];
};

/* The wf_object struct contains (some of) the contents of an OBJ file. */
struct wf_object
{
	/* List of material libraries, in which the materials are defined */
	char const *const *mtllib;

	/* Triangles grouped by material */
	struct wf_triangles
	{
		char const *mtlname;
		/* Each triangle consists of 3 vertices, and each vertex is
		   three indicies into the pos, uv, and norm arrays,
		   respectively. The indicies are zero based. */
		unsigned const (*indicies)[3][3];
		size_t n;
	} const *groups;

	/* Shared vertex data arrays */
	float const (*pos)[3], (*uv)[2], (*norm)[3];

	/* Array lengths */
	size_t nmtllib, npos, nuv, nnorm, ngroups;
};

struct wf_mtllib const *wf_parse_mtllib(FILE *fp);
struct wf_object const *wf_parse_object(FILE *fp);

void wf_free_object(struct wf_object const *obj);
void wf_free_mtllib(struct wf_mtllib const *obj);

