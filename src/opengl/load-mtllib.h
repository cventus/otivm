
struct rescache;
struct glstate;
struct wf_mtllib;
struct glcache;

struct rescache *gl_make_wf_mtllibs_cache(struct glstate *state);

struct wf_mtllib const *const *gl_load_wf_mtllib(
	struct glcache *,
	char const *file);

void gl_release_wf_mtllib(struct glcache *, struct wf_mtllib const *const *);

