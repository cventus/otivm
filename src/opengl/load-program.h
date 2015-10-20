
struct rescache;
struct glstate;
struct glprogram;

struct rescache *gl_make_programs_cache(struct glstate *state);

struct glprogram const *gl_load_program(
	struct glcache *,
	char const *const *shader_keys,
	size_t nshader_keys);

void gl_release_program(struct glcache *, struct glprogram const *);

