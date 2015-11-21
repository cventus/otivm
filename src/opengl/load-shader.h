
struct rescache;
struct gl_state;
struct gl_shader;

struct rescache *gl_make_shaders_cache(struct gl_state *state);

struct gl_shader const *gl_load_shader(struct gl_cache *, char const *filename);
void gl_release_shader(struct gl_cache *cache, struct gl_shader const *shader);

