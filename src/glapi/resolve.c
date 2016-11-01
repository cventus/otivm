#include <stddef.h>
#include <assert.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include "include/core.h"
#include "include/dbgmsg.h"
#include "api.h"

#include "fwd.h"

#define gl_def_api(ret, name, args) field->name = \
	(ret (GLAPIENTRY *) args)gl_get_proc("gl" #name);

int gl_resolve_core(struct gl_api *api, struct gl_core *field)
{
	assert(api != NULL);
	assert(field != NULL);
	assert(gl_is_current(api));

	/* Assumes the created context is 3.3 */
	GL_CORE_33_API(gl_def_api)

	return 0;
}

int gl_resolve_dbgmsg(struct gl_api *api, struct gl_dbgmsg *field)
{
	assert(api != NULL);
	assert(field != NULL);
	assert(gl_is_current(api));

	if (!gl_has_ext(api, "GL_ARB_debug_output")) {
		return -1;
	}
	GL_DEBUG_MESSAGE_CALLBACK_ARB(gl_def_api)

	return 0;
}

