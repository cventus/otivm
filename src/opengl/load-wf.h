
struct glgeometries;
struct glstate;

int gl_load_wfobj(struct glstate *, struct glgeometries *, char const *);
void gl_free_wfgeo(struct glstate *gl, struct glgeometries *);

