
#include "api/core-3_3.inc"

struct gl_core
{
#define gl_def_member(ret,name,args) ret (GLAPIENTRY *name) args;
	GL_CORE_33_API(gl_def_member)
};

