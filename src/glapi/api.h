struct gl_api;
#include "api.inc"
#define decl_gl_get_api(field)\
struct gl_##field const *gl_get_##field(struct gl_api *);
GL_API(decl_gl_get_api)
