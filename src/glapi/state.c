#include <assert.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>

#include "gm/vector.h"
#include "glapi/api.h"
#include "glapi/core.h"
#include "types.h"
#include "fwd.h"

static const struct gl_api empty = { 0 };

static GLint get_int(struct gl_api *api, GLenum val)
{
	GLint n;
	void (*fn)(void);
	
	fn = gl_get_proc(api, "glGetIntegerv");
	((void (APIENTRY *)(GLenum, GLint *))fn)(val, &n);
	return n;
}

/* Called once the context has been created. */
int gl_init_api(struct gl_api *api)
{
	assert(api != NULL);
	*api = empty;
	return 0;
}

int gl_term_api(struct gl_api *api)
{
	assert(api != NULL);
	if (api->core30) { free(api->core30); }
	*api = empty;
	return 0;
}

int gl_has_ext(struct gl_api *api, char const *target)
{
	GLint i, n;
	GLubyte const *ext;
	struct gl_core30 const *restrict gl;

	gl = gl_get_core30(api);
	if (gl == NULL || target == NULL || target[0] == '\0') { return 0; }
	for (gl->GetIntegerv(GL_NUM_EXTENSIONS, &n), i = 0; i < n; i++) {
		ext = gl->GetStringi(GL_EXTENSIONS, i);
		if (strcmp((char const*)ext, target) == 0) {
			return 1;
		}
	}
	return 0;
}

GLint gl_version_major(struct gl_api *api) {
	return get_int(api, GL_MAJOR_VERSION);
}

GLint gl_version_minor(struct gl_api *api) {
	return get_int(api, GL_MINOR_VERSION);
}

int gl_find_ext(char const *extensions, char const *target)
{
	char const *start, *ext;
	size_t i, tlen, extslen, elen;

	if (target == NULL) { return 0; }

	tlen = strlen(target);
	if (tlen == 0 || strchr(target, ' ')) { return 0; }

	extslen = strlen(extensions);
	for (i = 0; i + tlen <= extslen; i = (ext - extensions) + elen) {
		start = extensions + i;
		ext = start + strspn(start, " ");
		elen = strcspn(ext, " ");

		if (tlen == elen && memcmp(target, ext, tlen) == 0) {
			return 1;
		}
	}
	return 0;
}

static void *jmpmalloc(size_t size, jmp_buf env)
{
	void *p;

	p = malloc(size);
	if (!p && size > 0){
		longjmp(env, 1);
	}
	return p;
}

static int select_api(struct gl_api *api)
{
	jmp_buf errenv;
	int alloc;
	union {
		union gl_combined_core30 *core30;
		union gl_combined_core31 *core31;
		union gl_combined_core32 *core32;
		union gl_combined_core33 *core33;
		union gl_combined_core40 *core40;
		union gl_combined_core41 *core41;
		union gl_combined_core42 *core42;
		union gl_combined_core43 *core43;
		union gl_combined_core44 *core44;
		union gl_combined_core45 *core45;
	} p;

	if (setjmp(errenv)) {
		return -1;
	}

	if (!gl_is_current(api)) {
		return -2;
	}
	alloc = 0;

#define init_field(field) \
	if (!alloc) { \
		p.field = jmpmalloc(sizeof(*p.field), errenv); \
		alloc = 1; \
	} \
	api->field = &p.field->field;

	/* major fall-through and type punning ahead */
	switch (gl_version_major(api)) {
	/* not supported */
	case -1:
	case 1:
	case 2: return -3;

	default:
	case 4:	switch (gl_version_minor(api)) {
		case -1: return -1;

		default:
		case 5: p.core45 = jmpmalloc(sizeof(*p.core45), errenv);
			alloc = 1;
			api->core45 = &p.core45->core45;
			(void)gl_resolve_core45(api, &p.core45->core45);
		case 4: if (!alloc) {
				p.core44 = jmpmalloc(sizeof(*p.core44), errenv);
				(void)gl_resolve_core44(api, &p.core44->core44);
				alloc = 1;
			}
			api->core44 = &p.core44->core44;
		case 3: if (!alloc) {
				p.core43 = jmpmalloc(sizeof(*p.core43), errenv);
				(void)gl_resolve_core43(api, &p.core43->core43);
				alloc = 1;
			}
			api->core43 = &p.core43->core43;
		case 2: if (!alloc) {
				p.core42 = jmpmalloc(sizeof(*p.core42), errenv);
				(void)gl_resolve_core42(api, &p.core42->core42);
				alloc = 1;
			}
			api->core42 = &p.core42->core42;
		case 1: if (!alloc) {
				p.core41 = jmpmalloc(sizeof(*p.core41), errenv);
				(void)gl_resolve_core41(api, &p.core41->core41);
				alloc = 1;
			}
			api->core41 = &p.core41->core41;
		case 0: if (!alloc) {
				p.core40 = jmpmalloc(sizeof(*p.core40), errenv);
				(void)gl_resolve_core40(api, &p.core40->core40);
				alloc = 1;
			}
			api->core40 = &p.core40->core40;
		}
		goto init33;

	case 3:	switch (gl_version_minor(api)) {
		case -1: return -1;

		default:
		case 3: p.core33 = jmpmalloc(sizeof(*p.core33), errenv);
			alloc = 1;
		init33:	api->core33 = &p.core33->core33;
			(void)gl_resolve_core33(api, &p.core33->core33);
		case 2: if (!alloc) {
				p.core32 = jmpmalloc(sizeof(*p.core32), errenv);
				(void)gl_resolve_core32(api, &p.core32->core32);
				alloc = 1;
			}
			api->core32 = &p.core32->core32;
		case 1: if (!alloc) {
				p.core31 = jmpmalloc(sizeof(*p.core31), errenv);
				(void)gl_resolve_core31(api, &p.core31->core31);
				alloc = 1;
			}
			api->core31 = &p.core31->core31;
		case 0: if (!alloc) {
				p.core30 = jmpmalloc(sizeof(*p.core30), errenv);
				(void)gl_resolve_core30(api, &p.core30->core30);
				alloc = 1;
			}
			api->core30 = &p.core30->core30;
		}
	}
	return 0;
}

#define def_gl_get_api(field) \
struct gl_##field const *gl_get_##field(struct gl_api *api) \
{ \
	if (!api->core30 && select_api(api) != 0) { \
		return NULL; \
	} \
	return api->field; \
}

def_gl_get_api(core30)
def_gl_get_api(core31)
def_gl_get_api(core32)
def_gl_get_api(core33)

def_gl_get_api(core40)
def_gl_get_api(core41)
def_gl_get_api(core42)
def_gl_get_api(core43)
def_gl_get_api(core44)
def_gl_get_api(core45)

struct gl_ARB_debug_output const *gl_get_ARB_debug_output(struct gl_api *api)
{
	if (api->ARB_debug_output.DebugMessageCallbackARB) {
		return &api->ARB_debug_output;
	} else if (gl_has_ext(api, "GL_ARB_debug_output")) {
		return gl_resolve_ARB_debug_output(api, &api->ARB_debug_output);
	} else {
		return NULL;
	}
}
