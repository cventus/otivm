
struct rescache;
struct glstate;
struct glcache;
struct glgeometries;

struct rescache *gl_make_geometries_cache(struct glstate *state);

struct glgeometries const *const *gl_load_geometry(
	struct glcache *cache,
	char const *filename);

void gl_release_geometry(
	struct glcache *cache,
	struct glgeometries const *const *geometry);

