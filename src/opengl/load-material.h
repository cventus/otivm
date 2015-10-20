
struct rescache;
struct glstate;
struct glcache;
struct glmaterial;

struct rescache *gl_make_materials_cache(struct glstate *state);

void gl_release_material(struct glcache *, struct glmaterial const *);

struct glmaterial const *gl_load_wf_material(
	struct glcache *cache,
	char const *mtllib,
	char const *name);

