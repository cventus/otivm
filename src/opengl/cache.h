
struct gl_shader;
struct gl_program;

struct gl_shader const *gl_load_shader(struct gl_cache *, char const *filename);
void gl_release_shader(struct gl_cache *, struct gl_shader const *);

