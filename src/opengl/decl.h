
struct gl_state;
struct gl_cache;
struct gl_geometries;
struct gl_material;
struct gl_core;
struct gl_shader;

void gl_draw_geometries(
	struct gl_state *state,
	struct gl_geometries const *geos);

int gl_init_state(struct gl_state *gl);
int gl_free_state(struct gl_state *gl);

void gl_print_info(void);
void (*gl_get_proc(char const *name))(void);

int gl_is_extension_supported(const char *extensions, const char *target);
int gl_is_new_extension_supported(struct gl_state *state, const char *target);

int gl_init_cache(struct gl_cache *cache, struct gl_state *gl);
int gl_free_cache(struct gl_cache *cache);

int gl_resolve_functions(struct gl_core *core);

struct gl_material *gl_default_material(struct gl_state *);

