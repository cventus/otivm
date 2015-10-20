
struct glgeometries;
struct glstate;

struct glgeometries const *gl_load_wfobj(struct glstate *, char const *);
void gl_free_wfgeo(struct glstate *gl, struct glgeometries const *geos);

