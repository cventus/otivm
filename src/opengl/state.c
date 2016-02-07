
#include <string.h>
#include <GL/gl.h>

#include <gm/vector.h>

#include "types.h"
#include "decl.h"

int gl_state_init(struct gl_state *state)
{
        if (gl_resolve_functions(&state->f)) { return -1; }
        if (gl_cache_init(&state->cache, state)) { return -1; }
	return 0;
}

int gl_state_term(struct gl_state *state)
{
        if (gl_cache_term(&state->cache)) { return -1; }
	return 0;
}

struct gl_material *gl_default_material(struct gl_state *state)
{
	return &state->defmat;
}

int gl_is_new_extension_supported(struct gl_state *state, const char *target)
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

void gl_draw_geometry(struct gl_state *state, struct gl_geometry const *geo)
{
	state->f.glBindVertexArray(geo->vao);
	glDrawElements(geo->eb.mode, geo->eb.count, geo->eb.type, 0);
	state->f.glBindVertexArray(0);
}

void gl_draw_geometries(struct gl_state *state, struct gl_geometries const *geos)
{
	size_t i;
	for (i = 0; i < geos->n; i++) {
		gl_draw_geometry(state, geos->geo + i);
	}
}

