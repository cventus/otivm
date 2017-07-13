struct gl_geometries;
struct gl_cache;

/* Create a geometry set from a WF obj file */
int gl_geometries_init_wfobj(
	struct gl_cache *,
	struct gl_geometries *,
	char const *);

void gl_geometries_term(struct gl_cache *cache, struct gl_geometries *geos);
