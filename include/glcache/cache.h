
struct gl_cache;
struct gl_geometries;
struct gl_material;
struct gl_program;
struct gl_shader;
struct gl_texture;

struct gl_geometries const *gl_load_geometry(
	struct gl_cache *cache,
	char const *filename);

void gl_release_geometry(
	struct gl_cache *cache,
	struct gl_geometries const *geometry);

struct gl_material const *gl_load_wf_material(
	struct gl_cache *cache,
	char const *mtllib,
	char const *name);

void gl_release_material(struct gl_cache *, struct gl_material const *);

struct gl_program const *gl_load_program(
	struct gl_cache *,
	char const *const *shader_filenames,
	size_t nshader_filenames);

void gl_release_program(struct gl_cache *, struct gl_program const *);

struct gl_shader const *gl_load_shader(struct gl_cache *, char const *filename);
void gl_release_shader(struct gl_cache *, struct gl_shader const *shader);

struct gl_texture const *gl_load_texture(
	struct gl_cache *cache,
	char const *filename);

void gl_release_texture(
	struct gl_cache *cache,
	struct gl_texture const *texture);

