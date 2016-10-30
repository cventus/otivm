
struct gl_state;
struct gl_cache;
struct gl_geometries;
struct gl_material;
struct gl_core;
struct gl_shader;
struct gl_cache;
struct wf_mtllib;

void gl_draw_geometries(
	struct gl_state *state,
	struct gl_geometries const *geos);

struct gl_cache *gl_make_cache(struct gl_state *state);
int gl_free_cache(struct gl_cache *cache);

int gl_cache_init(struct gl_cache *cache, struct gl_state *gl);
int gl_cache_term(struct gl_cache *cache);

struct gl_material *gl_default_material(struct gl_cache *);

struct wf_mtllib const *const *gl_load_wf_mtllib(
	struct gl_cache *,
	char const *file);

void gl_release_wf_mtllib(struct gl_cache *, struct wf_mtllib const *const *);


