
struct gl_geometries;
struct gl_state;

int gl_load_wfobj(struct gl_state *, struct gl_geometries *, char const *);
void gl_free_wfgeo(struct gl_state *gl, struct gl_geometries *);

