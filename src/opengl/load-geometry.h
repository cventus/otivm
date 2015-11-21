
struct rescache;
struct gl_state;
struct gl_cache;
struct gl_geometries;

struct rescache *gl_make_geometries_cache(struct gl_state *state);

struct gl_geometries const *gl_load_geometry(
	struct gl_cache *cache,
	char const *filename);

void gl_release_geometry(
	struct gl_cache *cache,
	struct gl_geometries const *geometry);

