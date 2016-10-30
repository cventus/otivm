
struct rescache;
struct gl_cache;

struct rescache *gl_make_geometries_cache(struct gl_cache *cache);
struct rescache *gl_make_materials_cache(struct gl_cache *cache);
struct rescache *gl_make_wf_mtllibs_cache(struct gl_cache *cache);
struct rescache *gl_make_programs_cache(struct gl_cache *cache);
struct rescache *gl_make_shaders_cache(struct gl_cache *cache);
struct rescache *gl_make_textures_cache(struct gl_cache *cache);

