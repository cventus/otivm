
struct rescache;
struct gl_state;
struct gl_cache;
struct gl_material;

struct rescache *gl_make_materials_cache(struct gl_state *state);

void gl_release_material(struct gl_cache *, struct gl_material const *);

struct gl_material const *gl_load_wf_material(
	struct gl_cache *cache,
	char const *mtllib,
	char const *name);

