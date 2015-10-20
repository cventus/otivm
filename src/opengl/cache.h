
struct glshader;
struct glprogram;

struct glshader const *gl_load_shader(struct glcache *, char const *filename);
void gl_release_shader(struct glcache *, struct glshader const *);

