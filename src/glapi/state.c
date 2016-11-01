
#include <assert.h>
#include <string.h>
#include <GL/gl.h>

#include <gm/vector.h>

#include "api.inc"
#include "include/core.h"
#include "include/dbgmsg.h"
#include "types.h"
#include "fwd.h"

/* Called once the context has been created. */
int gl_init_api(struct gl_api *api)
{
	assert(api != NULL);
	(void)memset(api, 0, sizeof *api);
	return 0;
}

int gl_term_api(struct gl_api *api)
{
	assert(api != NULL);
	(void)memset(api, 0xff, sizeof *api);
	return 0;
}

int gl_has_ext(struct gl_api *api, const char *target)
{
	GLint i, n;
	struct gl_core const *core;

	core = gl_get_core(api);
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

#define def_gl_get_api(field) \
struct gl_##field const *gl_get_##field(struct gl_api *api) \
{ \
	if (!api->field##_init) { \
		if (!gl_is_current(api)) { \
			return NULL; \
		} else { \
			api->field##_init = 1; \
			if (gl_resolve_##field(api, &api->field)) { \
				api->no_##field = 1; \
			} \
		} \
	} \
	return api->no_##field ? NULL : &api->field; \
}

GL_API(def_gl_get_api)

