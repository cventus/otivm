
#include "api/dbgmsg.inc"

struct gl_dbgmsg
{
#define gl_def_member(ret,name,args) ret (GLAPIENTRY *name) args;
	GL_DEBUG_MESSAGE_CALLBACK_ARB(gl_def_member)
};

