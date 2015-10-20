
struct rescache;
struct glstate;
struct glshader;

struct rescache *gl_make_shaders_cache(struct glstate *state);

struct glshader const *gl_load_shader(struct glcache *, char const *filename);
void gl_release_shader(struct glcache *cache, struct glshader const *shader);

