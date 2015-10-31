
#include <string.h>
#include <GL/gl.h>

#include <gm/vector.h>

#include "types.h"
#include "decl.h"

int gl_init_state(struct glstate *state)
{
        if (gl_resolve_functions(&state->f)) { return -1; }
        if (gl_init_cache(&state->cache, state)) { return -1; }
	return 0;
}

int gl_free_state(struct glstate *state)
{
        if (gl_free_cache(&state->cache)) { return -1; }
	return 0;
}

struct glmaterial *gl_default_material(struct glstate *state)
{
	return &state->defmat;
}

int gl_is_new_extension_supported(struct glstate *state, const char *target)
{
	GLint i, n;

	if (target == NULL) { return 0; }

	glGetIntegerv(GL_NUM_EXTENSIONS, &n);
	for (i = 0; i < n; i++) {
		char const *extension =
			(char const*)state->f.glGetStringi(GL_EXTENSIONS, i);
		if (strcmp(extension, target) == 0) {
			return 1;
		}
	}
	return 0;
}

int gl_is_extension_supported(const char *extensions, const char *target)
{
	if (target == NULL) { return 0; }

	size_t const tlen = strlen(target);
	if (tlen == 0 || strchr(target, ' ')) { return 0; }

	size_t const extslen = strlen(extensions);
	for (size_t i = 0; i + tlen <= extslen; ) {
		char const *start = extensions + i;
		char const *ext = start + strspn(start, " ");
		size_t const elen = strcspn(ext, " ");

		if (tlen == elen && memcmp(target, ext, tlen) == 0) {
			return 1;
		} else {
			i = (ext - extensions) + elen;
		}
	}
	return 0;
}

void gl_draw_geometry(struct glstate *state, struct glgeometry const *geo)
{
	state->f.glBindVertexArray(geo->vao);
	glDrawElements(geo->eb.mode, geo->eb.count, geo->eb.type, 0);
	state->f.glBindVertexArray(0);
}

void gl_draw_geometries(struct glstate *state, struct glgeometries const *geos)
{
	size_t i;
	for (i = 0; i < geos->n; i++) {
		gl_draw_geometry(state, geos->geo + i);
	}
}

