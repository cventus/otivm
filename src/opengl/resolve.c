#include <stddef.h>
#include <assert.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include "include/core.h"
#include "include/dbgmsg.h"

#include "fwd.h"

#define gl_def_api(ret, name, args) api->name = \
	(ret (GLAPIENTRY *) args)gl_get_proc("gl" #name);

int gl_resolve_core(struct gl_state *state, struct gl_core *api)
{
	assert(state != NULL);
	assert(api != NULL);

	/* Assumes the created context is 3.3 */
	GL_CORE_33_API(gl_def_api)

	return 0;
}

int gl_resolve_dbgmsg(struct gl_state *state, struct gl_dbgmsg *api)
{
	assert(state != NULL);
	assert(api != NULL);
	assert(gl_is_current(state));

	if (!gl_has_ext(state, "GL_ARB_debug_output")) {
		return -1;
	}

	GL_DEBUG_MESSAGE_CALLBACK_ARB(gl_def_api)

	return 0;
}

