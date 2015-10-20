
struct glstate;
struct glcache;
struct glgeometries;
struct glmaterial;
struct glfn;
struct glshader;

int gl_init_state(struct glstate *gl);
int gl_free_state(struct glstate *gl);

void gl_print_info(void);
void (*gl_get_proc(char const *name))(void);

int gl_is_extension_supported(const char *extensions, const char *target);

int gl_init_cache(struct glcache *cache, struct glstate *gl);
int gl_free_cache(struct glcache *cache);

int gl_resolve_functions(struct glfn *glfn);

struct glmaterial *gl_default_material(struct glstate *);


