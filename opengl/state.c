
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

