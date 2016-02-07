
struct gl_geometries;
struct gl_state;

/* Create a geometry set from a WF obj file */
int gl_geometries_init_wfobj(
	struct gl_state *,
	struct gl_geometries *,
	char const *);

void gl_geometries_term(struct gl_state *state, struct gl_geometries *geos);

