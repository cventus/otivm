#include "glapi/core.h"

struct gl_api;

void (*gl_get_proc(struct gl_api *, char const *))(void);
int gl_has_ext(struct gl_api *, char const *);

struct gl_KHR_blend_equation_advanced *gl_resolve_KHR_blend_equation_advanced(struct gl_api *api, struct gl_KHR_blend_equation_advanced *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->BlendBarrierKHR = (void (APIENTRYP)(void))gl_get_proc(api, "glBlendBarrierKHR");

	return ext;
}

struct gl_KHR_debug *gl_resolve_KHR_debug(struct gl_api *api, struct gl_KHR_debug *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->DebugMessageControl = (void (APIENTRYP)(GLenum , GLenum , GLenum , GLsizei , const GLuint *, GLboolean ))gl_get_proc(api, "glDebugMessageControl");
	ext->DebugMessageInsert = (void (APIENTRYP)(GLenum , GLenum , GLuint , GLenum , GLsizei , const GLchar *))gl_get_proc(api, "glDebugMessageInsert");
	ext->DebugMessageCallback = (void (APIENTRYP)(GLDEBUGPROC , const void *))gl_get_proc(api, "glDebugMessageCallback");
	ext->GetDebugMessageLog = (GLuint (APIENTRYP)(GLuint , GLsizei , GLenum *, GLenum *, GLuint *, GLenum *, GLsizei *, GLchar *))gl_get_proc(api, "glGetDebugMessageLog");
	ext->PushDebugGroup = (void (APIENTRYP)(GLenum , GLuint , GLsizei , const GLchar *))gl_get_proc(api, "glPushDebugGroup");
	ext->PopDebugGroup = (void (APIENTRYP)(void))gl_get_proc(api, "glPopDebugGroup");
	ext->ObjectLabel = (void (APIENTRYP)(GLenum , GLuint , GLsizei , const GLchar *))gl_get_proc(api, "glObjectLabel");
	ext->GetObjectLabel = (void (APIENTRYP)(GLenum , GLuint , GLsizei , GLsizei *, GLchar *))gl_get_proc(api, "glGetObjectLabel");
	ext->ObjectPtrLabel = (void (APIENTRYP)(const void *, GLsizei , const GLchar *))gl_get_proc(api, "glObjectPtrLabel");
	ext->GetObjectPtrLabel = (void (APIENTRYP)(const void *, GLsizei , GLsizei *, GLchar *))gl_get_proc(api, "glGetObjectPtrLabel");
	ext->GetPointerv = (void (APIENTRYP)(GLenum , void **))gl_get_proc(api, "glGetPointerv");
	ext->DebugMessageControlKHR = (void (APIENTRYP)(GLenum , GLenum , GLenum , GLsizei , const GLuint *, GLboolean ))gl_get_proc(api, "glDebugMessageControlKHR");
	ext->DebugMessageInsertKHR = (void (APIENTRYP)(GLenum , GLenum , GLuint , GLenum , GLsizei , const GLchar *))gl_get_proc(api, "glDebugMessageInsertKHR");
	ext->DebugMessageCallbackKHR = (void (APIENTRYP)(GLDEBUGPROCKHR , const void *))gl_get_proc(api, "glDebugMessageCallbackKHR");
	ext->GetDebugMessageLogKHR = (GLuint (APIENTRYP)(GLuint , GLsizei , GLenum *, GLenum *, GLuint *, GLenum *, GLsizei *, GLchar *))gl_get_proc(api, "glGetDebugMessageLogKHR");
	ext->PushDebugGroupKHR = (void (APIENTRYP)(GLenum , GLuint , GLsizei , const GLchar *))gl_get_proc(api, "glPushDebugGroupKHR");
	ext->PopDebugGroupKHR = (void (APIENTRYP)(void))gl_get_proc(api, "glPopDebugGroupKHR");
	ext->ObjectLabelKHR = (void (APIENTRYP)(GLenum , GLuint , GLsizei , const GLchar *))gl_get_proc(api, "glObjectLabelKHR");
	ext->GetObjectLabelKHR = (void (APIENTRYP)(GLenum , GLuint , GLsizei , GLsizei *, GLchar *))gl_get_proc(api, "glGetObjectLabelKHR");
	ext->ObjectPtrLabelKHR = (void (APIENTRYP)(const void *, GLsizei , const GLchar *))gl_get_proc(api, "glObjectPtrLabelKHR");
	ext->GetObjectPtrLabelKHR = (void (APIENTRYP)(const void *, GLsizei , GLsizei *, GLchar *))gl_get_proc(api, "glGetObjectPtrLabelKHR");
	ext->GetPointervKHR = (void (APIENTRYP)(GLenum , void **))gl_get_proc(api, "glGetPointervKHR");

	return ext;
}

struct gl_KHR_robustness *gl_resolve_KHR_robustness(struct gl_api *api, struct gl_KHR_robustness *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->GetGraphicsResetStatus = (GLenum (APIENTRYP)(void))gl_get_proc(api, "glGetGraphicsResetStatus");
	ext->ReadnPixels = (void (APIENTRYP)(GLint , GLint , GLsizei , GLsizei , GLenum , GLenum , GLsizei , void *))gl_get_proc(api, "glReadnPixels");
	ext->GetnUniformfv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLfloat *))gl_get_proc(api, "glGetnUniformfv");
	ext->GetnUniformiv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLint *))gl_get_proc(api, "glGetnUniformiv");
	ext->GetnUniformuiv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLuint *))gl_get_proc(api, "glGetnUniformuiv");
	ext->GetGraphicsResetStatusKHR = (GLenum (APIENTRYP)(void))gl_get_proc(api, "glGetGraphicsResetStatusKHR");
	ext->ReadnPixelsKHR = (void (APIENTRYP)(GLint , GLint , GLsizei , GLsizei , GLenum , GLenum , GLsizei , void *))gl_get_proc(api, "glReadnPixelsKHR");
	ext->GetnUniformfvKHR = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLfloat *))gl_get_proc(api, "glGetnUniformfvKHR");
	ext->GetnUniformivKHR = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLint *))gl_get_proc(api, "glGetnUniformivKHR");
	ext->GetnUniformuivKHR = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLuint *))gl_get_proc(api, "glGetnUniformuivKHR");

	return ext;
}
