
#include <GL/glx.h>

#include "../decl.h"

void (*gl_get_proc(char const *name))(void)
{
	return glXGetProcAddressARB((GLubyte const *)name);
}

