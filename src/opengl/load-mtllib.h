
struct rescache;
struct gl_state;
struct wf_mtllib;
struct gl_cache;

struct rescache *gl_make_wf_mtllibs_cache(struct gl_state *state);

struct wf_mtllib const *const *gl_load_wf_mtllib(
	struct gl_cache *,
	char const *file);

void gl_release_wf_mtllib(struct gl_cache *, struct wf_mtllib const *const *);

