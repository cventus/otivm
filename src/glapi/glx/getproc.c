#include <GL/glx.h>

#include "../fwd.h"

void (*gl_get_proc(struct gl_api *api, char const *name))(void)
{
	(void)api;
	return glXGetProcAddressARB((GLubyte const *)name);
}
