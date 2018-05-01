struct gl_api;
struct gl_core33;
struct xylo;
struct xylo_outline_set;
struct xylo_mesh_set;

struct xylo *make_xylo(struct gl_api *api);
int init_xylo(struct xylo *dest, struct gl_api *api);
void term_xylo(struct xylo *xylo);
void free_xylo(struct xylo *xylo);

/* set up OpenGL state; call before drawing */
void xylo_begin(struct xylo *xylo);

/* restore OpenGL state; call after drawing */
void xylo_end(struct xylo *xylo);

/* specify which shapes to draw - binds a vertex array object */
void xylo_set_outline_set(struct xylo *xylo, struct xylo_outline_set *set);

void xylo_set_mesh_set(struct xylo *xylo, struct xylo_mesh_set *set);

/* wrapper of glGet */
GLuint xylo_get_uint(struct gl_core33 const *restrict gl, GLenum t);

/* retrieve id of object most recently drawn at a pixel location */ 
unsigned xylo_get_object_id(struct xylo *xylo, GLsizei x, GLsizei y);
