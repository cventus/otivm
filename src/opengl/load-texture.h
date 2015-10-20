
struct rescache;
struct glstate;
struct glcache;
struct gltexture;

struct rescache *gl_make_textures_cache(struct glstate *state);

struct gltexture const *gl_load_texture(
	struct glcache *cache,
	char const *filename);

void gl_release_texture(struct glcache *cache, struct gltexture const *texture);

