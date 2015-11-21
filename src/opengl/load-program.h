
struct rescache;
struct gl_state;
struct gl_program;

struct rescache *gl_make_programs_cache(struct gl_state *state);

struct gl_program const *gl_load_program(
	struct gl_cache *,
	char const *const *shader_keys,
	size_t nshader_keys);

void gl_release_program(struct gl_cache *, struct gl_program const *);

