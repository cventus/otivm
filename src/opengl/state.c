
#include <assert.h>
#include <string.h>
#include <GL/gl.h>

#include <gm/vector.h>

#include "include/core.h"
#include "include/dbgmsg.h"
#include "types.h"
#include "fwd.h"

/* Called once the context has been created. */
int gl_init_state(struct gl_state *state)
{
	assert(state != NULL);
	(void)memset(state, 0, sizeof *state);
	return 0;
}

int gl_term_state(struct gl_state *state)
{
	assert(state != NULL);
	(void)memset(state, 0xff, sizeof *state);
	return 0;
}

int gl_has_ext(struct gl_state *state, const char *target)
{
	GLint i, n;
	struct gl_core const *core;

	core = gl_get_core(state);
	if (core == NULL || target == NULL || target[0] == '\0') { return 0; }
	for (glGetIntegerv(GL_NUM_EXTENSIONS, &n), i = 0; i < n; i++) {
		GLubyte const *ext = core->GetStringi(GL_EXTENSIONS, i);
		if (strcmp((char const*)ext, target) == 0) {
			return 1;
		}
	}
	return 0;
}

int gl_find_ext(char const *extensions, char const *target)
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

#define def_gl_get_api(API) \
struct gl_##API const *gl_get_##API(struct gl_state *state) \
{ \
	if (!state->API##_init) { \
		if (!gl_is_current(state)) { \
			return NULL; \
		} else { \
			state->API##_init = 1; \
			if (gl_resolve_##API(state, &state->API)) { \
				state->no_##API = 1; \
			} \
		} \
	} \
	return state->no_##API ? NULL : &state->API; \
}

STATE_FIELDS(def_gl_get_api)

