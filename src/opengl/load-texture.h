
struct rescache;
struct gl_state;
struct gl_cache;
struct gl_texture;

struct rescache *gl_make_textures_cache(struct gl_state *state);

struct gl_texture const *gl_load_texture(
	struct gl_cache *cache,
	char const *filename);

void gl_release_texture(
	struct gl_cache *cache,
	struct gl_texture const *texture);

