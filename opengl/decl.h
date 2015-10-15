
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

struct glgeometries const *gl_load_wfobj(struct glstate *, char const *);
void gl_free_wfgeo(struct glstate *gl, struct glgeometries const *geos);

int gl_resolve_functions(struct glfn *glfn);
int gl_make_shader(struct glstate *, char const *, GLenum, struct glshader *);
char *gl_get_shader_info_log(struct glstate *, struct glshader *);
GLint gl_shader_type(struct glstate *gl, struct glshader *shader);

struct glmaterial *gl_default_material(struct glstate *);

struct gltexture *gl_load_texture(struct glcache *, char const *path);
void gl_release_texture(struct glcache *, struct gltexture *texture);

struct glshader *gl_load_vertex_shader(struct glcache *, char const *path);
void gl_release_vertex_shader(struct glcache *, struct glshader *shader);

struct glshader *gl_load_fragment_shader(struct glcache *, char const *path);
void gl_release_fragment_shader(struct glcache *, struct glshader *shader);

//struct glmaterial *gl_new_material(struct glcache *);
struct glmaterial const *gl_load_material(struct glcache *, char const *key);
void gl_release_material(struct glcache *, struct glmaterial const *material);

struct glgeometries const *const *gl_load_geometry(
	struct glcache *cache,
	char const *filename);
void gl_release_geometry(
	struct glcache *cache,
	struct glgeometries const *const *geos);

struct wf_mtllib const **gl_load_wf_mtllib(struct glcache *, char const *key);
void gl_release_wf_mtllib(struct glcache *, struct wf_mtllib const **mtllib);

