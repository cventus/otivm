#include "glapi/core.h"

struct gl_api;

void (*gl_get_proc(struct gl_api *, char const *))(void);
int gl_has_ext(struct gl_api *, char const *);

struct gl_ARB_ES2_compatibility *gl_resolve_ARB_ES2_compatibility(struct gl_api *api, struct gl_ARB_ES2_compatibility *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->ReleaseShaderCompiler = (void (APIENTRYP)(void))gl_get_proc(api, "glReleaseShaderCompiler");
	ext->ShaderBinary = (void (APIENTRYP)(GLsizei , const GLuint *, GLenum , const void *, GLsizei ))gl_get_proc(api, "glShaderBinary");
	ext->GetShaderPrecisionFormat = (void (APIENTRYP)(GLenum , GLenum , GLint *, GLint *))gl_get_proc(api, "glGetShaderPrecisionFormat");
	ext->DepthRangef = (void (APIENTRYP)(GLfloat , GLfloat ))gl_get_proc(api, "glDepthRangef");
	ext->ClearDepthf = (void (APIENTRYP)(GLfloat ))gl_get_proc(api, "glClearDepthf");

	return ext;
}

struct gl_ARB_ES3_1_compatibility *gl_resolve_ARB_ES3_1_compatibility(struct gl_api *api, struct gl_ARB_ES3_1_compatibility *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->MemoryBarrierByRegion = (void (APIENTRYP)(GLbitfield ))gl_get_proc(api, "glMemoryBarrierByRegion");

	return ext;
}

struct gl_ARB_ES3_2_compatibility *gl_resolve_ARB_ES3_2_compatibility(struct gl_api *api, struct gl_ARB_ES3_2_compatibility *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->PrimitiveBoundingBoxARB = (void (APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat , GLfloat , GLfloat , GLfloat , GLfloat ))gl_get_proc(api, "glPrimitiveBoundingBoxARB");

	return ext;
}

struct gl_ARB_base_instance *gl_resolve_ARB_base_instance(struct gl_api *api, struct gl_ARB_base_instance *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->DrawArraysInstancedBaseInstance = (void (APIENTRYP)(GLenum , GLint , GLsizei , GLsizei , GLuint ))gl_get_proc(api, "glDrawArraysInstancedBaseInstance");
	ext->DrawElementsInstancedBaseInstance = (void (APIENTRYP)(GLenum , GLsizei , GLenum , const void *, GLsizei , GLuint ))gl_get_proc(api, "glDrawElementsInstancedBaseInstance");
	ext->DrawElementsInstancedBaseVertexBaseInstance = (void (APIENTRYP)(GLenum , GLsizei , GLenum , const void *, GLsizei , GLint , GLuint ))gl_get_proc(api, "glDrawElementsInstancedBaseVertexBaseInstance");

	return ext;
}

struct gl_ARB_bindless_texture *gl_resolve_ARB_bindless_texture(struct gl_api *api, struct gl_ARB_bindless_texture *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->GetTextureHandleARB = (GLuint64 (APIENTRYP)(GLuint ))gl_get_proc(api, "glGetTextureHandleARB");
	ext->GetTextureSamplerHandleARB = (GLuint64 (APIENTRYP)(GLuint , GLuint ))gl_get_proc(api, "glGetTextureSamplerHandleARB");
	ext->MakeTextureHandleResidentARB = (void (APIENTRYP)(GLuint64 ))gl_get_proc(api, "glMakeTextureHandleResidentARB");
	ext->MakeTextureHandleNonResidentARB = (void (APIENTRYP)(GLuint64 ))gl_get_proc(api, "glMakeTextureHandleNonResidentARB");
	ext->GetImageHandleARB = (GLuint64 (APIENTRYP)(GLuint , GLint , GLboolean , GLint , GLenum ))gl_get_proc(api, "glGetImageHandleARB");
	ext->MakeImageHandleResidentARB = (void (APIENTRYP)(GLuint64 , GLenum ))gl_get_proc(api, "glMakeImageHandleResidentARB");
	ext->MakeImageHandleNonResidentARB = (void (APIENTRYP)(GLuint64 ))gl_get_proc(api, "glMakeImageHandleNonResidentARB");
	ext->UniformHandleui64ARB = (void (APIENTRYP)(GLint , GLuint64 ))gl_get_proc(api, "glUniformHandleui64ARB");
	ext->UniformHandleui64vARB = (void (APIENTRYP)(GLint , GLsizei , const GLuint64 *))gl_get_proc(api, "glUniformHandleui64vARB");
	ext->ProgramUniformHandleui64ARB = (void (APIENTRYP)(GLuint , GLint , GLuint64 ))gl_get_proc(api, "glProgramUniformHandleui64ARB");
	ext->ProgramUniformHandleui64vARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLuint64 *))gl_get_proc(api, "glProgramUniformHandleui64vARB");
	ext->IsTextureHandleResidentARB = (GLboolean (APIENTRYP)(GLuint64 ))gl_get_proc(api, "glIsTextureHandleResidentARB");
	ext->IsImageHandleResidentARB = (GLboolean (APIENTRYP)(GLuint64 ))gl_get_proc(api, "glIsImageHandleResidentARB");
	ext->VertexAttribL1ui64ARB = (void (APIENTRYP)(GLuint , GLuint64EXT ))gl_get_proc(api, "glVertexAttribL1ui64ARB");
	ext->VertexAttribL1ui64vARB = (void (APIENTRYP)(GLuint , const GLuint64EXT *))gl_get_proc(api, "glVertexAttribL1ui64vARB");
	ext->GetVertexAttribLui64vARB = (void (APIENTRYP)(GLuint , GLenum , GLuint64EXT *))gl_get_proc(api, "glGetVertexAttribLui64vARB");

	return ext;
}

struct gl_ARB_blend_func_extended *gl_resolve_ARB_blend_func_extended(struct gl_api *api, struct gl_ARB_blend_func_extended *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->BindFragDataLocationIndexed = (void (APIENTRYP)(GLuint , GLuint , GLuint , const GLchar *))gl_get_proc(api, "glBindFragDataLocationIndexed");
	ext->GetFragDataIndex = (GLint (APIENTRYP)(GLuint , const GLchar *))gl_get_proc(api, "glGetFragDataIndex");

	return ext;
}

struct gl_ARB_buffer_storage *gl_resolve_ARB_buffer_storage(struct gl_api *api, struct gl_ARB_buffer_storage *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->BufferStorage = (void (APIENTRYP)(GLenum , GLsizeiptr , const void *, GLbitfield ))gl_get_proc(api, "glBufferStorage");

	return ext;
}

struct gl_ARB_cl_event *gl_resolve_ARB_cl_event(struct gl_api *api, struct gl_ARB_cl_event *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->CreateSyncFromCLeventARB = (GLsync (APIENTRYP)(struct _cl_context *, struct _cl_event *, GLbitfield ))gl_get_proc(api, "glCreateSyncFromCLeventARB");

	return ext;
}

struct gl_ARB_clear_buffer_object *gl_resolve_ARB_clear_buffer_object(struct gl_api *api, struct gl_ARB_clear_buffer_object *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->ClearBufferData = (void (APIENTRYP)(GLenum , GLenum , GLenum , GLenum , const void *))gl_get_proc(api, "glClearBufferData");
	ext->ClearBufferSubData = (void (APIENTRYP)(GLenum , GLenum , GLintptr , GLsizeiptr , GLenum , GLenum , const void *))gl_get_proc(api, "glClearBufferSubData");

	return ext;
}

struct gl_ARB_clear_texture *gl_resolve_ARB_clear_texture(struct gl_api *api, struct gl_ARB_clear_texture *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->ClearTexImage = (void (APIENTRYP)(GLuint , GLint , GLenum , GLenum , const void *))gl_get_proc(api, "glClearTexImage");
	ext->ClearTexSubImage = (void (APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLenum , GLenum , const void *))gl_get_proc(api, "glClearTexSubImage");

	return ext;
}

struct gl_ARB_clip_control *gl_resolve_ARB_clip_control(struct gl_api *api, struct gl_ARB_clip_control *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->ClipControl = (void (APIENTRYP)(GLenum , GLenum ))gl_get_proc(api, "glClipControl");

	return ext;
}

struct gl_ARB_color_buffer_float *gl_resolve_ARB_color_buffer_float(struct gl_api *api, struct gl_ARB_color_buffer_float *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->ClampColorARB = (void (APIENTRYP)(GLenum , GLenum ))gl_get_proc(api, "glClampColorARB");

	return ext;
}

struct gl_ARB_compute_shader *gl_resolve_ARB_compute_shader(struct gl_api *api, struct gl_ARB_compute_shader *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->DispatchCompute = (void (APIENTRYP)(GLuint , GLuint , GLuint ))gl_get_proc(api, "glDispatchCompute");
	ext->DispatchComputeIndirect = (void (APIENTRYP)(GLintptr ))gl_get_proc(api, "glDispatchComputeIndirect");

	return ext;
}

struct gl_ARB_compute_variable_group_size *gl_resolve_ARB_compute_variable_group_size(struct gl_api *api, struct gl_ARB_compute_variable_group_size *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->DispatchComputeGroupSizeARB = (void (APIENTRYP)(GLuint , GLuint , GLuint , GLuint , GLuint , GLuint ))gl_get_proc(api, "glDispatchComputeGroupSizeARB");

	return ext;
}

struct gl_ARB_copy_buffer *gl_resolve_ARB_copy_buffer(struct gl_api *api, struct gl_ARB_copy_buffer *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->CopyBufferSubData = (void (APIENTRYP)(GLenum , GLenum , GLintptr , GLintptr , GLsizeiptr ))gl_get_proc(api, "glCopyBufferSubData");

	return ext;
}

struct gl_ARB_copy_image *gl_resolve_ARB_copy_image(struct gl_api *api, struct gl_ARB_copy_image *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->CopyImageSubData = (void (APIENTRYP)(GLuint , GLenum , GLint , GLint , GLint , GLint , GLuint , GLenum , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei ))gl_get_proc(api, "glCopyImageSubData");

	return ext;
}

struct gl_ARB_debug_output *gl_resolve_ARB_debug_output(struct gl_api *api, struct gl_ARB_debug_output *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->DebugMessageControlARB = (void (APIENTRYP)(GLenum , GLenum , GLenum , GLsizei , const GLuint *, GLboolean ))gl_get_proc(api, "glDebugMessageControlARB");
	ext->DebugMessageInsertARB = (void (APIENTRYP)(GLenum , GLenum , GLuint , GLenum , GLsizei , const GLchar *))gl_get_proc(api, "glDebugMessageInsertARB");
	ext->DebugMessageCallbackARB = (void (APIENTRYP)(GLDEBUGPROCARB , const void *))gl_get_proc(api, "glDebugMessageCallbackARB");
	ext->GetDebugMessageLogARB = (GLuint (APIENTRYP)(GLuint , GLsizei , GLenum *, GLenum *, GLuint *, GLenum *, GLsizei *, GLchar *))gl_get_proc(api, "glGetDebugMessageLogARB");

	return ext;
}

struct gl_ARB_direct_state_access *gl_resolve_ARB_direct_state_access(struct gl_api *api, struct gl_ARB_direct_state_access *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->CreateTransformFeedbacks = (void (APIENTRYP)(GLsizei , GLuint *))gl_get_proc(api, "glCreateTransformFeedbacks");
	ext->TransformFeedbackBufferBase = (void (APIENTRYP)(GLuint , GLuint , GLuint ))gl_get_proc(api, "glTransformFeedbackBufferBase");
	ext->TransformFeedbackBufferRange = (void (APIENTRYP)(GLuint , GLuint , GLuint , GLintptr , GLsizeiptr ))gl_get_proc(api, "glTransformFeedbackBufferRange");
	ext->GetTransformFeedbackiv = (void (APIENTRYP)(GLuint , GLenum , GLint *))gl_get_proc(api, "glGetTransformFeedbackiv");
	ext->GetTransformFeedbacki_v = (void (APIENTRYP)(GLuint , GLenum , GLuint , GLint *))gl_get_proc(api, "glGetTransformFeedbacki_v");
	ext->GetTransformFeedbacki64_v = (void (APIENTRYP)(GLuint , GLenum , GLuint , GLint64 *))gl_get_proc(api, "glGetTransformFeedbacki64_v");
	ext->CreateBuffers = (void (APIENTRYP)(GLsizei , GLuint *))gl_get_proc(api, "glCreateBuffers");
	ext->NamedBufferStorage = (void (APIENTRYP)(GLuint , GLsizeiptr , const void *, GLbitfield ))gl_get_proc(api, "glNamedBufferStorage");
	ext->NamedBufferData = (void (APIENTRYP)(GLuint , GLsizeiptr , const void *, GLenum ))gl_get_proc(api, "glNamedBufferData");
	ext->NamedBufferSubData = (void (APIENTRYP)(GLuint , GLintptr , GLsizeiptr , const void *))gl_get_proc(api, "glNamedBufferSubData");
	ext->CopyNamedBufferSubData = (void (APIENTRYP)(GLuint , GLuint , GLintptr , GLintptr , GLsizeiptr ))gl_get_proc(api, "glCopyNamedBufferSubData");
	ext->ClearNamedBufferData = (void (APIENTRYP)(GLuint , GLenum , GLenum , GLenum , const void *))gl_get_proc(api, "glClearNamedBufferData");
	ext->ClearNamedBufferSubData = (void (APIENTRYP)(GLuint , GLenum , GLintptr , GLsizeiptr , GLenum , GLenum , const void *))gl_get_proc(api, "glClearNamedBufferSubData");
	ext->MapNamedBuffer = (void *(APIENTRYP)(GLuint , GLenum ))gl_get_proc(api, "glMapNamedBuffer");
	ext->MapNamedBufferRange = (void *(APIENTRYP)(GLuint , GLintptr , GLsizeiptr , GLbitfield ))gl_get_proc(api, "glMapNamedBufferRange");
	ext->UnmapNamedBuffer = (GLboolean (APIENTRYP)(GLuint ))gl_get_proc(api, "glUnmapNamedBuffer");
	ext->FlushMappedNamedBufferRange = (void (APIENTRYP)(GLuint , GLintptr , GLsizeiptr ))gl_get_proc(api, "glFlushMappedNamedBufferRange");
	ext->GetNamedBufferParameteriv = (void (APIENTRYP)(GLuint , GLenum , GLint *))gl_get_proc(api, "glGetNamedBufferParameteriv");
	ext->GetNamedBufferParameteri64v = (void (APIENTRYP)(GLuint , GLenum , GLint64 *))gl_get_proc(api, "glGetNamedBufferParameteri64v");
	ext->GetNamedBufferPointerv = (void (APIENTRYP)(GLuint , GLenum , void **))gl_get_proc(api, "glGetNamedBufferPointerv");
	ext->GetNamedBufferSubData = (void (APIENTRYP)(GLuint , GLintptr , GLsizeiptr , void *))gl_get_proc(api, "glGetNamedBufferSubData");
	ext->CreateFramebuffers = (void (APIENTRYP)(GLsizei , GLuint *))gl_get_proc(api, "glCreateFramebuffers");
	ext->NamedFramebufferRenderbuffer = (void (APIENTRYP)(GLuint , GLenum , GLenum , GLuint ))gl_get_proc(api, "glNamedFramebufferRenderbuffer");
	ext->NamedFramebufferParameteri = (void (APIENTRYP)(GLuint , GLenum , GLint ))gl_get_proc(api, "glNamedFramebufferParameteri");
	ext->NamedFramebufferTexture = (void (APIENTRYP)(GLuint , GLenum , GLuint , GLint ))gl_get_proc(api, "glNamedFramebufferTexture");
	ext->NamedFramebufferTextureLayer = (void (APIENTRYP)(GLuint , GLenum , GLuint , GLint , GLint ))gl_get_proc(api, "glNamedFramebufferTextureLayer");
	ext->NamedFramebufferDrawBuffer = (void (APIENTRYP)(GLuint , GLenum ))gl_get_proc(api, "glNamedFramebufferDrawBuffer");
	ext->NamedFramebufferDrawBuffers = (void (APIENTRYP)(GLuint , GLsizei , const GLenum *))gl_get_proc(api, "glNamedFramebufferDrawBuffers");
	ext->NamedFramebufferReadBuffer = (void (APIENTRYP)(GLuint , GLenum ))gl_get_proc(api, "glNamedFramebufferReadBuffer");
	ext->InvalidateNamedFramebufferData = (void (APIENTRYP)(GLuint , GLsizei , const GLenum *))gl_get_proc(api, "glInvalidateNamedFramebufferData");
	ext->InvalidateNamedFramebufferSubData = (void (APIENTRYP)(GLuint , GLsizei , const GLenum *, GLint , GLint , GLsizei , GLsizei ))gl_get_proc(api, "glInvalidateNamedFramebufferSubData");
	ext->ClearNamedFramebufferiv = (void (APIENTRYP)(GLuint , GLenum , GLint , const GLint *))gl_get_proc(api, "glClearNamedFramebufferiv");
	ext->ClearNamedFramebufferuiv = (void (APIENTRYP)(GLuint , GLenum , GLint , const GLuint *))gl_get_proc(api, "glClearNamedFramebufferuiv");
	ext->ClearNamedFramebufferfv = (void (APIENTRYP)(GLuint , GLenum , GLint , const GLfloat *))gl_get_proc(api, "glClearNamedFramebufferfv");
	ext->ClearNamedFramebufferfi = (void (APIENTRYP)(GLuint , GLenum , GLint , GLfloat , GLint ))gl_get_proc(api, "glClearNamedFramebufferfi");
	ext->BlitNamedFramebuffer = (void (APIENTRYP)(GLuint , GLuint , GLint , GLint , GLint , GLint , GLint , GLint , GLint , GLint , GLbitfield , GLenum ))gl_get_proc(api, "glBlitNamedFramebuffer");
	ext->CheckNamedFramebufferStatus = (GLenum (APIENTRYP)(GLuint , GLenum ))gl_get_proc(api, "glCheckNamedFramebufferStatus");
	ext->GetNamedFramebufferParameteriv = (void (APIENTRYP)(GLuint , GLenum , GLint *))gl_get_proc(api, "glGetNamedFramebufferParameteriv");
	ext->GetNamedFramebufferAttachmentParameteriv = (void (APIENTRYP)(GLuint , GLenum , GLenum , GLint *))gl_get_proc(api, "glGetNamedFramebufferAttachmentParameteriv");
	ext->CreateRenderbuffers = (void (APIENTRYP)(GLsizei , GLuint *))gl_get_proc(api, "glCreateRenderbuffers");
	ext->NamedRenderbufferStorage = (void (APIENTRYP)(GLuint , GLenum , GLsizei , GLsizei ))gl_get_proc(api, "glNamedRenderbufferStorage");
	ext->NamedRenderbufferStorageMultisample = (void (APIENTRYP)(GLuint , GLsizei , GLenum , GLsizei , GLsizei ))gl_get_proc(api, "glNamedRenderbufferStorageMultisample");
	ext->GetNamedRenderbufferParameteriv = (void (APIENTRYP)(GLuint , GLenum , GLint *))gl_get_proc(api, "glGetNamedRenderbufferParameteriv");
	ext->CreateTextures = (void (APIENTRYP)(GLenum , GLsizei , GLuint *))gl_get_proc(api, "glCreateTextures");
	ext->TextureBuffer = (void (APIENTRYP)(GLuint , GLenum , GLuint ))gl_get_proc(api, "glTextureBuffer");
	ext->TextureBufferRange = (void (APIENTRYP)(GLuint , GLenum , GLuint , GLintptr , GLsizeiptr ))gl_get_proc(api, "glTextureBufferRange");
	ext->TextureStorage1D = (void (APIENTRYP)(GLuint , GLsizei , GLenum , GLsizei ))gl_get_proc(api, "glTextureStorage1D");
	ext->TextureStorage2D = (void (APIENTRYP)(GLuint , GLsizei , GLenum , GLsizei , GLsizei ))gl_get_proc(api, "glTextureStorage2D");
	ext->TextureStorage3D = (void (APIENTRYP)(GLuint , GLsizei , GLenum , GLsizei , GLsizei , GLsizei ))gl_get_proc(api, "glTextureStorage3D");
	ext->TextureStorage2DMultisample = (void (APIENTRYP)(GLuint , GLsizei , GLenum , GLsizei , GLsizei , GLboolean ))gl_get_proc(api, "glTextureStorage2DMultisample");
	ext->TextureStorage3DMultisample = (void (APIENTRYP)(GLuint , GLsizei , GLenum , GLsizei , GLsizei , GLsizei , GLboolean ))gl_get_proc(api, "glTextureStorage3DMultisample");
	ext->TextureSubImage1D = (void (APIENTRYP)(GLuint , GLint , GLint , GLsizei , GLenum , GLenum , const void *))gl_get_proc(api, "glTextureSubImage1D");
	ext->TextureSubImage2D = (void (APIENTRYP)(GLuint , GLint , GLint , GLint , GLsizei , GLsizei , GLenum , GLenum , const void *))gl_get_proc(api, "glTextureSubImage2D");
	ext->TextureSubImage3D = (void (APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLenum , GLenum , const void *))gl_get_proc(api, "glTextureSubImage3D");
	ext->CompressedTextureSubImage1D = (void (APIENTRYP)(GLuint , GLint , GLint , GLsizei , GLenum , GLsizei , const void *))gl_get_proc(api, "glCompressedTextureSubImage1D");
	ext->CompressedTextureSubImage2D = (void (APIENTRYP)(GLuint , GLint , GLint , GLint , GLsizei , GLsizei , GLenum , GLsizei , const void *))gl_get_proc(api, "glCompressedTextureSubImage2D");
	ext->CompressedTextureSubImage3D = (void (APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLenum , GLsizei , const void *))gl_get_proc(api, "glCompressedTextureSubImage3D");
	ext->CopyTextureSubImage1D = (void (APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLsizei ))gl_get_proc(api, "glCopyTextureSubImage1D");
	ext->CopyTextureSubImage2D = (void (APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLint , GLsizei , GLsizei ))gl_get_proc(api, "glCopyTextureSubImage2D");
	ext->CopyTextureSubImage3D = (void (APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLint , GLint , GLsizei , GLsizei ))gl_get_proc(api, "glCopyTextureSubImage3D");
	ext->TextureParameterf = (void (APIENTRYP)(GLuint , GLenum , GLfloat ))gl_get_proc(api, "glTextureParameterf");
	ext->TextureParameterfv = (void (APIENTRYP)(GLuint , GLenum , const GLfloat *))gl_get_proc(api, "glTextureParameterfv");
	ext->TextureParameteri = (void (APIENTRYP)(GLuint , GLenum , GLint ))gl_get_proc(api, "glTextureParameteri");
	ext->TextureParameterIiv = (void (APIENTRYP)(GLuint , GLenum , const GLint *))gl_get_proc(api, "glTextureParameterIiv");
	ext->TextureParameterIuiv = (void (APIENTRYP)(GLuint , GLenum , const GLuint *))gl_get_proc(api, "glTextureParameterIuiv");
	ext->TextureParameteriv = (void (APIENTRYP)(GLuint , GLenum , const GLint *))gl_get_proc(api, "glTextureParameteriv");
	ext->GenerateTextureMipmap = (void (APIENTRYP)(GLuint ))gl_get_proc(api, "glGenerateTextureMipmap");
	ext->BindTextureUnit = (void (APIENTRYP)(GLuint , GLuint ))gl_get_proc(api, "glBindTextureUnit");
	ext->GetTextureImage = (void (APIENTRYP)(GLuint , GLint , GLenum , GLenum , GLsizei , void *))gl_get_proc(api, "glGetTextureImage");
	ext->GetCompressedTextureImage = (void (APIENTRYP)(GLuint , GLint , GLsizei , void *))gl_get_proc(api, "glGetCompressedTextureImage");
	ext->GetTextureLevelParameterfv = (void (APIENTRYP)(GLuint , GLint , GLenum , GLfloat *))gl_get_proc(api, "glGetTextureLevelParameterfv");
	ext->GetTextureLevelParameteriv = (void (APIENTRYP)(GLuint , GLint , GLenum , GLint *))gl_get_proc(api, "glGetTextureLevelParameteriv");
	ext->GetTextureParameterfv = (void (APIENTRYP)(GLuint , GLenum , GLfloat *))gl_get_proc(api, "glGetTextureParameterfv");
	ext->GetTextureParameterIiv = (void (APIENTRYP)(GLuint , GLenum , GLint *))gl_get_proc(api, "glGetTextureParameterIiv");
	ext->GetTextureParameterIuiv = (void (APIENTRYP)(GLuint , GLenum , GLuint *))gl_get_proc(api, "glGetTextureParameterIuiv");
	ext->GetTextureParameteriv = (void (APIENTRYP)(GLuint , GLenum , GLint *))gl_get_proc(api, "glGetTextureParameteriv");
	ext->CreateVertexArrays = (void (APIENTRYP)(GLsizei , GLuint *))gl_get_proc(api, "glCreateVertexArrays");
	ext->DisableVertexArrayAttrib = (void (APIENTRYP)(GLuint , GLuint ))gl_get_proc(api, "glDisableVertexArrayAttrib");
	ext->EnableVertexArrayAttrib = (void (APIENTRYP)(GLuint , GLuint ))gl_get_proc(api, "glEnableVertexArrayAttrib");
	ext->VertexArrayElementBuffer = (void (APIENTRYP)(GLuint , GLuint ))gl_get_proc(api, "glVertexArrayElementBuffer");
	ext->VertexArrayVertexBuffer = (void (APIENTRYP)(GLuint , GLuint , GLuint , GLintptr , GLsizei ))gl_get_proc(api, "glVertexArrayVertexBuffer");
	ext->VertexArrayVertexBuffers = (void (APIENTRYP)(GLuint , GLuint , GLsizei , const GLuint *, const GLintptr *, const GLsizei *))gl_get_proc(api, "glVertexArrayVertexBuffers");
	ext->VertexArrayAttribBinding = (void (APIENTRYP)(GLuint , GLuint , GLuint ))gl_get_proc(api, "glVertexArrayAttribBinding");
	ext->VertexArrayAttribFormat = (void (APIENTRYP)(GLuint , GLuint , GLint , GLenum , GLboolean , GLuint ))gl_get_proc(api, "glVertexArrayAttribFormat");
	ext->VertexArrayAttribIFormat = (void (APIENTRYP)(GLuint , GLuint , GLint , GLenum , GLuint ))gl_get_proc(api, "glVertexArrayAttribIFormat");
	ext->VertexArrayAttribLFormat = (void (APIENTRYP)(GLuint , GLuint , GLint , GLenum , GLuint ))gl_get_proc(api, "glVertexArrayAttribLFormat");
	ext->VertexArrayBindingDivisor = (void (APIENTRYP)(GLuint , GLuint , GLuint ))gl_get_proc(api, "glVertexArrayBindingDivisor");
	ext->GetVertexArrayiv = (void (APIENTRYP)(GLuint , GLenum , GLint *))gl_get_proc(api, "glGetVertexArrayiv");
	ext->GetVertexArrayIndexediv = (void (APIENTRYP)(GLuint , GLuint , GLenum , GLint *))gl_get_proc(api, "glGetVertexArrayIndexediv");
	ext->GetVertexArrayIndexed64iv = (void (APIENTRYP)(GLuint , GLuint , GLenum , GLint64 *))gl_get_proc(api, "glGetVertexArrayIndexed64iv");
	ext->CreateSamplers = (void (APIENTRYP)(GLsizei , GLuint *))gl_get_proc(api, "glCreateSamplers");
	ext->CreateProgramPipelines = (void (APIENTRYP)(GLsizei , GLuint *))gl_get_proc(api, "glCreateProgramPipelines");
	ext->CreateQueries = (void (APIENTRYP)(GLenum , GLsizei , GLuint *))gl_get_proc(api, "glCreateQueries");
	ext->GetQueryBufferObjecti64v = (void (APIENTRYP)(GLuint , GLuint , GLenum , GLintptr ))gl_get_proc(api, "glGetQueryBufferObjecti64v");
	ext->GetQueryBufferObjectiv = (void (APIENTRYP)(GLuint , GLuint , GLenum , GLintptr ))gl_get_proc(api, "glGetQueryBufferObjectiv");
	ext->GetQueryBufferObjectui64v = (void (APIENTRYP)(GLuint , GLuint , GLenum , GLintptr ))gl_get_proc(api, "glGetQueryBufferObjectui64v");
	ext->GetQueryBufferObjectuiv = (void (APIENTRYP)(GLuint , GLuint , GLenum , GLintptr ))gl_get_proc(api, "glGetQueryBufferObjectuiv");

	return ext;
}

struct gl_ARB_draw_buffers *gl_resolve_ARB_draw_buffers(struct gl_api *api, struct gl_ARB_draw_buffers *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->DrawBuffersARB = (void (APIENTRYP)(GLsizei , const GLenum *))gl_get_proc(api, "glDrawBuffersARB");

	return ext;
}

struct gl_ARB_draw_buffers_blend *gl_resolve_ARB_draw_buffers_blend(struct gl_api *api, struct gl_ARB_draw_buffers_blend *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->BlendEquationiARB = (void (APIENTRYP)(GLuint , GLenum ))gl_get_proc(api, "glBlendEquationiARB");
	ext->BlendEquationSeparateiARB = (void (APIENTRYP)(GLuint , GLenum , GLenum ))gl_get_proc(api, "glBlendEquationSeparateiARB");
	ext->BlendFunciARB = (void (APIENTRYP)(GLuint , GLenum , GLenum ))gl_get_proc(api, "glBlendFunciARB");
	ext->BlendFuncSeparateiARB = (void (APIENTRYP)(GLuint , GLenum , GLenum , GLenum , GLenum ))gl_get_proc(api, "glBlendFuncSeparateiARB");

	return ext;
}

struct gl_ARB_draw_elements_base_vertex *gl_resolve_ARB_draw_elements_base_vertex(struct gl_api *api, struct gl_ARB_draw_elements_base_vertex *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->DrawElementsBaseVertex = (void (APIENTRYP)(GLenum , GLsizei , GLenum , const void *, GLint ))gl_get_proc(api, "glDrawElementsBaseVertex");
	ext->DrawRangeElementsBaseVertex = (void (APIENTRYP)(GLenum , GLuint , GLuint , GLsizei , GLenum , const void *, GLint ))gl_get_proc(api, "glDrawRangeElementsBaseVertex");
	ext->DrawElementsInstancedBaseVertex = (void (APIENTRYP)(GLenum , GLsizei , GLenum , const void *, GLsizei , GLint ))gl_get_proc(api, "glDrawElementsInstancedBaseVertex");
	ext->MultiDrawElementsBaseVertex = (void (APIENTRYP)(GLenum , const GLsizei *, GLenum , const void *const*, GLsizei , const GLint *))gl_get_proc(api, "glMultiDrawElementsBaseVertex");

	return ext;
}

struct gl_ARB_draw_indirect *gl_resolve_ARB_draw_indirect(struct gl_api *api, struct gl_ARB_draw_indirect *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->DrawArraysIndirect = (void (APIENTRYP)(GLenum , const void *))gl_get_proc(api, "glDrawArraysIndirect");
	ext->DrawElementsIndirect = (void (APIENTRYP)(GLenum , GLenum , const void *))gl_get_proc(api, "glDrawElementsIndirect");

	return ext;
}

struct gl_ARB_draw_instanced *gl_resolve_ARB_draw_instanced(struct gl_api *api, struct gl_ARB_draw_instanced *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->DrawArraysInstancedARB = (void (APIENTRYP)(GLenum , GLint , GLsizei , GLsizei ))gl_get_proc(api, "glDrawArraysInstancedARB");
	ext->DrawElementsInstancedARB = (void (APIENTRYP)(GLenum , GLsizei , GLenum , const void *, GLsizei ))gl_get_proc(api, "glDrawElementsInstancedARB");

	return ext;
}

struct gl_ARB_fragment_program *gl_resolve_ARB_fragment_program(struct gl_api *api, struct gl_ARB_fragment_program *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->ProgramStringARB = (void (APIENTRYP)(GLenum , GLenum , GLsizei , const void *))gl_get_proc(api, "glProgramStringARB");
	ext->BindProgramARB = (void (APIENTRYP)(GLenum , GLuint ))gl_get_proc(api, "glBindProgramARB");
	ext->DeleteProgramsARB = (void (APIENTRYP)(GLsizei , const GLuint *))gl_get_proc(api, "glDeleteProgramsARB");
	ext->GenProgramsARB = (void (APIENTRYP)(GLsizei , GLuint *))gl_get_proc(api, "glGenProgramsARB");
	ext->ProgramEnvParameter4dARB = (void (APIENTRYP)(GLenum , GLuint , GLdouble , GLdouble , GLdouble , GLdouble ))gl_get_proc(api, "glProgramEnvParameter4dARB");
	ext->ProgramEnvParameter4dvARB = (void (APIENTRYP)(GLenum , GLuint , const GLdouble *))gl_get_proc(api, "glProgramEnvParameter4dvARB");
	ext->ProgramEnvParameter4fARB = (void (APIENTRYP)(GLenum , GLuint , GLfloat , GLfloat , GLfloat , GLfloat ))gl_get_proc(api, "glProgramEnvParameter4fARB");
	ext->ProgramEnvParameter4fvARB = (void (APIENTRYP)(GLenum , GLuint , const GLfloat *))gl_get_proc(api, "glProgramEnvParameter4fvARB");
	ext->ProgramLocalParameter4dARB = (void (APIENTRYP)(GLenum , GLuint , GLdouble , GLdouble , GLdouble , GLdouble ))gl_get_proc(api, "glProgramLocalParameter4dARB");
	ext->ProgramLocalParameter4dvARB = (void (APIENTRYP)(GLenum , GLuint , const GLdouble *))gl_get_proc(api, "glProgramLocalParameter4dvARB");
	ext->ProgramLocalParameter4fARB = (void (APIENTRYP)(GLenum , GLuint , GLfloat , GLfloat , GLfloat , GLfloat ))gl_get_proc(api, "glProgramLocalParameter4fARB");
	ext->ProgramLocalParameter4fvARB = (void (APIENTRYP)(GLenum , GLuint , const GLfloat *))gl_get_proc(api, "glProgramLocalParameter4fvARB");
	ext->GetProgramEnvParameterdvARB = (void (APIENTRYP)(GLenum , GLuint , GLdouble *))gl_get_proc(api, "glGetProgramEnvParameterdvARB");
	ext->GetProgramEnvParameterfvARB = (void (APIENTRYP)(GLenum , GLuint , GLfloat *))gl_get_proc(api, "glGetProgramEnvParameterfvARB");
	ext->GetProgramLocalParameterdvARB = (void (APIENTRYP)(GLenum , GLuint , GLdouble *))gl_get_proc(api, "glGetProgramLocalParameterdvARB");
	ext->GetProgramLocalParameterfvARB = (void (APIENTRYP)(GLenum , GLuint , GLfloat *))gl_get_proc(api, "glGetProgramLocalParameterfvARB");
	ext->GetProgramivARB = (void (APIENTRYP)(GLenum , GLenum , GLint *))gl_get_proc(api, "glGetProgramivARB");
	ext->GetProgramStringARB = (void (APIENTRYP)(GLenum , GLenum , void *))gl_get_proc(api, "glGetProgramStringARB");
	ext->IsProgramARB = (GLboolean (APIENTRYP)(GLuint ))gl_get_proc(api, "glIsProgramARB");

	return ext;
}

struct gl_ARB_framebuffer_no_attachments *gl_resolve_ARB_framebuffer_no_attachments(struct gl_api *api, struct gl_ARB_framebuffer_no_attachments *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->FramebufferParameteri = (void (APIENTRYP)(GLenum , GLenum , GLint ))gl_get_proc(api, "glFramebufferParameteri");
	ext->GetFramebufferParameteriv = (void (APIENTRYP)(GLenum , GLenum , GLint *))gl_get_proc(api, "glGetFramebufferParameteriv");

	return ext;
}

struct gl_ARB_framebuffer_object *gl_resolve_ARB_framebuffer_object(struct gl_api *api, struct gl_ARB_framebuffer_object *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->IsRenderbuffer = (GLboolean (APIENTRYP)(GLuint ))gl_get_proc(api, "glIsRenderbuffer");
	ext->BindRenderbuffer = (void (APIENTRYP)(GLenum , GLuint ))gl_get_proc(api, "glBindRenderbuffer");
	ext->DeleteRenderbuffers = (void (APIENTRYP)(GLsizei , const GLuint *))gl_get_proc(api, "glDeleteRenderbuffers");
	ext->GenRenderbuffers = (void (APIENTRYP)(GLsizei , GLuint *))gl_get_proc(api, "glGenRenderbuffers");
	ext->RenderbufferStorage = (void (APIENTRYP)(GLenum , GLenum , GLsizei , GLsizei ))gl_get_proc(api, "glRenderbufferStorage");
	ext->GetRenderbufferParameteriv = (void (APIENTRYP)(GLenum , GLenum , GLint *))gl_get_proc(api, "glGetRenderbufferParameteriv");
	ext->IsFramebuffer = (GLboolean (APIENTRYP)(GLuint ))gl_get_proc(api, "glIsFramebuffer");
	ext->BindFramebuffer = (void (APIENTRYP)(GLenum , GLuint ))gl_get_proc(api, "glBindFramebuffer");
	ext->DeleteFramebuffers = (void (APIENTRYP)(GLsizei , const GLuint *))gl_get_proc(api, "glDeleteFramebuffers");
	ext->GenFramebuffers = (void (APIENTRYP)(GLsizei , GLuint *))gl_get_proc(api, "glGenFramebuffers");
	ext->CheckFramebufferStatus = (GLenum (APIENTRYP)(GLenum ))gl_get_proc(api, "glCheckFramebufferStatus");
	ext->FramebufferTexture1D = (void (APIENTRYP)(GLenum , GLenum , GLenum , GLuint , GLint ))gl_get_proc(api, "glFramebufferTexture1D");
	ext->FramebufferTexture2D = (void (APIENTRYP)(GLenum , GLenum , GLenum , GLuint , GLint ))gl_get_proc(api, "glFramebufferTexture2D");
	ext->FramebufferTexture3D = (void (APIENTRYP)(GLenum , GLenum , GLenum , GLuint , GLint , GLint ))gl_get_proc(api, "glFramebufferTexture3D");
	ext->FramebufferRenderbuffer = (void (APIENTRYP)(GLenum , GLenum , GLenum , GLuint ))gl_get_proc(api, "glFramebufferRenderbuffer");
	ext->GetFramebufferAttachmentParameteriv = (void (APIENTRYP)(GLenum , GLenum , GLenum , GLint *))gl_get_proc(api, "glGetFramebufferAttachmentParameteriv");
	ext->GenerateMipmap = (void (APIENTRYP)(GLenum ))gl_get_proc(api, "glGenerateMipmap");
	ext->BlitFramebuffer = (void (APIENTRYP)(GLint , GLint , GLint , GLint , GLint , GLint , GLint , GLint , GLbitfield , GLenum ))gl_get_proc(api, "glBlitFramebuffer");
	ext->RenderbufferStorageMultisample = (void (APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei ))gl_get_proc(api, "glRenderbufferStorageMultisample");
	ext->FramebufferTextureLayer = (void (APIENTRYP)(GLenum , GLenum , GLuint , GLint , GLint ))gl_get_proc(api, "glFramebufferTextureLayer");

	return ext;
}

struct gl_ARB_geometry_shader4 *gl_resolve_ARB_geometry_shader4(struct gl_api *api, struct gl_ARB_geometry_shader4 *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->ProgramParameteriARB = (void (APIENTRYP)(GLuint , GLenum , GLint ))gl_get_proc(api, "glProgramParameteriARB");
	ext->FramebufferTextureARB = (void (APIENTRYP)(GLenum , GLenum , GLuint , GLint ))gl_get_proc(api, "glFramebufferTextureARB");
	ext->FramebufferTextureLayerARB = (void (APIENTRYP)(GLenum , GLenum , GLuint , GLint , GLint ))gl_get_proc(api, "glFramebufferTextureLayerARB");
	ext->FramebufferTextureFaceARB = (void (APIENTRYP)(GLenum , GLenum , GLuint , GLint , GLenum ))gl_get_proc(api, "glFramebufferTextureFaceARB");

	return ext;
}

struct gl_ARB_get_program_binary *gl_resolve_ARB_get_program_binary(struct gl_api *api, struct gl_ARB_get_program_binary *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->GetProgramBinary = (void (APIENTRYP)(GLuint , GLsizei , GLsizei *, GLenum *, void *))gl_get_proc(api, "glGetProgramBinary");
	ext->ProgramBinary = (void (APIENTRYP)(GLuint , GLenum , const void *, GLsizei ))gl_get_proc(api, "glProgramBinary");
	ext->ProgramParameteri = (void (APIENTRYP)(GLuint , GLenum , GLint ))gl_get_proc(api, "glProgramParameteri");

	return ext;
}

struct gl_ARB_get_texture_sub_image *gl_resolve_ARB_get_texture_sub_image(struct gl_api *api, struct gl_ARB_get_texture_sub_image *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->GetTextureSubImage = (void (APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLenum , GLenum , GLsizei , void *))gl_get_proc(api, "glGetTextureSubImage");
	ext->GetCompressedTextureSubImage = (void (APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLsizei , void *))gl_get_proc(api, "glGetCompressedTextureSubImage");

	return ext;
}

struct gl_ARB_gpu_shader_fp64 *gl_resolve_ARB_gpu_shader_fp64(struct gl_api *api, struct gl_ARB_gpu_shader_fp64 *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->Uniform1d = (void (APIENTRYP)(GLint , GLdouble ))gl_get_proc(api, "glUniform1d");
	ext->Uniform2d = (void (APIENTRYP)(GLint , GLdouble , GLdouble ))gl_get_proc(api, "glUniform2d");
	ext->Uniform3d = (void (APIENTRYP)(GLint , GLdouble , GLdouble , GLdouble ))gl_get_proc(api, "glUniform3d");
	ext->Uniform4d = (void (APIENTRYP)(GLint , GLdouble , GLdouble , GLdouble , GLdouble ))gl_get_proc(api, "glUniform4d");
	ext->Uniform1dv = (void (APIENTRYP)(GLint , GLsizei , const GLdouble *))gl_get_proc(api, "glUniform1dv");
	ext->Uniform2dv = (void (APIENTRYP)(GLint , GLsizei , const GLdouble *))gl_get_proc(api, "glUniform2dv");
	ext->Uniform3dv = (void (APIENTRYP)(GLint , GLsizei , const GLdouble *))gl_get_proc(api, "glUniform3dv");
	ext->Uniform4dv = (void (APIENTRYP)(GLint , GLsizei , const GLdouble *))gl_get_proc(api, "glUniform4dv");
	ext->UniformMatrix2dv = (void (APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glUniformMatrix2dv");
	ext->UniformMatrix3dv = (void (APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glUniformMatrix3dv");
	ext->UniformMatrix4dv = (void (APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glUniformMatrix4dv");
	ext->UniformMatrix2x3dv = (void (APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glUniformMatrix2x3dv");
	ext->UniformMatrix2x4dv = (void (APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glUniformMatrix2x4dv");
	ext->UniformMatrix3x2dv = (void (APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glUniformMatrix3x2dv");
	ext->UniformMatrix3x4dv = (void (APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glUniformMatrix3x4dv");
	ext->UniformMatrix4x2dv = (void (APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glUniformMatrix4x2dv");
	ext->UniformMatrix4x3dv = (void (APIENTRYP)(GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glUniformMatrix4x3dv");
	ext->GetUniformdv = (void (APIENTRYP)(GLuint , GLint , GLdouble *))gl_get_proc(api, "glGetUniformdv");

	return ext;
}

struct gl_ARB_gpu_shader_int64 *gl_resolve_ARB_gpu_shader_int64(struct gl_api *api, struct gl_ARB_gpu_shader_int64 *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->Uniform1i64ARB = (void (APIENTRYP)(GLint , GLint64 ))gl_get_proc(api, "glUniform1i64ARB");
	ext->Uniform2i64ARB = (void (APIENTRYP)(GLint , GLint64 , GLint64 ))gl_get_proc(api, "glUniform2i64ARB");
	ext->Uniform3i64ARB = (void (APIENTRYP)(GLint , GLint64 , GLint64 , GLint64 ))gl_get_proc(api, "glUniform3i64ARB");
	ext->Uniform4i64ARB = (void (APIENTRYP)(GLint , GLint64 , GLint64 , GLint64 , GLint64 ))gl_get_proc(api, "glUniform4i64ARB");
	ext->Uniform1i64vARB = (void (APIENTRYP)(GLint , GLsizei , const GLint64 *))gl_get_proc(api, "glUniform1i64vARB");
	ext->Uniform2i64vARB = (void (APIENTRYP)(GLint , GLsizei , const GLint64 *))gl_get_proc(api, "glUniform2i64vARB");
	ext->Uniform3i64vARB = (void (APIENTRYP)(GLint , GLsizei , const GLint64 *))gl_get_proc(api, "glUniform3i64vARB");
	ext->Uniform4i64vARB = (void (APIENTRYP)(GLint , GLsizei , const GLint64 *))gl_get_proc(api, "glUniform4i64vARB");
	ext->Uniform1ui64ARB = (void (APIENTRYP)(GLint , GLuint64 ))gl_get_proc(api, "glUniform1ui64ARB");
	ext->Uniform2ui64ARB = (void (APIENTRYP)(GLint , GLuint64 , GLuint64 ))gl_get_proc(api, "glUniform2ui64ARB");
	ext->Uniform3ui64ARB = (void (APIENTRYP)(GLint , GLuint64 , GLuint64 , GLuint64 ))gl_get_proc(api, "glUniform3ui64ARB");
	ext->Uniform4ui64ARB = (void (APIENTRYP)(GLint , GLuint64 , GLuint64 , GLuint64 , GLuint64 ))gl_get_proc(api, "glUniform4ui64ARB");
	ext->Uniform1ui64vARB = (void (APIENTRYP)(GLint , GLsizei , const GLuint64 *))gl_get_proc(api, "glUniform1ui64vARB");
	ext->Uniform2ui64vARB = (void (APIENTRYP)(GLint , GLsizei , const GLuint64 *))gl_get_proc(api, "glUniform2ui64vARB");
	ext->Uniform3ui64vARB = (void (APIENTRYP)(GLint , GLsizei , const GLuint64 *))gl_get_proc(api, "glUniform3ui64vARB");
	ext->Uniform4ui64vARB = (void (APIENTRYP)(GLint , GLsizei , const GLuint64 *))gl_get_proc(api, "glUniform4ui64vARB");
	ext->GetUniformi64vARB = (void (APIENTRYP)(GLuint , GLint , GLint64 *))gl_get_proc(api, "glGetUniformi64vARB");
	ext->GetUniformui64vARB = (void (APIENTRYP)(GLuint , GLint , GLuint64 *))gl_get_proc(api, "glGetUniformui64vARB");
	ext->GetnUniformi64vARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLint64 *))gl_get_proc(api, "glGetnUniformi64vARB");
	ext->GetnUniformui64vARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLuint64 *))gl_get_proc(api, "glGetnUniformui64vARB");
	ext->ProgramUniform1i64ARB = (void (APIENTRYP)(GLuint , GLint , GLint64 ))gl_get_proc(api, "glProgramUniform1i64ARB");
	ext->ProgramUniform2i64ARB = (void (APIENTRYP)(GLuint , GLint , GLint64 , GLint64 ))gl_get_proc(api, "glProgramUniform2i64ARB");
	ext->ProgramUniform3i64ARB = (void (APIENTRYP)(GLuint , GLint , GLint64 , GLint64 , GLint64 ))gl_get_proc(api, "glProgramUniform3i64ARB");
	ext->ProgramUniform4i64ARB = (void (APIENTRYP)(GLuint , GLint , GLint64 , GLint64 , GLint64 , GLint64 ))gl_get_proc(api, "glProgramUniform4i64ARB");
	ext->ProgramUniform1i64vARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLint64 *))gl_get_proc(api, "glProgramUniform1i64vARB");
	ext->ProgramUniform2i64vARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLint64 *))gl_get_proc(api, "glProgramUniform2i64vARB");
	ext->ProgramUniform3i64vARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLint64 *))gl_get_proc(api, "glProgramUniform3i64vARB");
	ext->ProgramUniform4i64vARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLint64 *))gl_get_proc(api, "glProgramUniform4i64vARB");
	ext->ProgramUniform1ui64ARB = (void (APIENTRYP)(GLuint , GLint , GLuint64 ))gl_get_proc(api, "glProgramUniform1ui64ARB");
	ext->ProgramUniform2ui64ARB = (void (APIENTRYP)(GLuint , GLint , GLuint64 , GLuint64 ))gl_get_proc(api, "glProgramUniform2ui64ARB");
	ext->ProgramUniform3ui64ARB = (void (APIENTRYP)(GLuint , GLint , GLuint64 , GLuint64 , GLuint64 ))gl_get_proc(api, "glProgramUniform3ui64ARB");
	ext->ProgramUniform4ui64ARB = (void (APIENTRYP)(GLuint , GLint , GLuint64 , GLuint64 , GLuint64 , GLuint64 ))gl_get_proc(api, "glProgramUniform4ui64ARB");
	ext->ProgramUniform1ui64vARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLuint64 *))gl_get_proc(api, "glProgramUniform1ui64vARB");
	ext->ProgramUniform2ui64vARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLuint64 *))gl_get_proc(api, "glProgramUniform2ui64vARB");
	ext->ProgramUniform3ui64vARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLuint64 *))gl_get_proc(api, "glProgramUniform3ui64vARB");
	ext->ProgramUniform4ui64vARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLuint64 *))gl_get_proc(api, "glProgramUniform4ui64vARB");

	return ext;
}

struct gl_ARB_imaging *gl_resolve_ARB_imaging(struct gl_api *api, struct gl_ARB_imaging *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->BlendColor = (void (APIENTRYP)(GLfloat , GLfloat , GLfloat , GLfloat ))gl_get_proc(api, "glBlendColor");
	ext->BlendEquation = (void (APIENTRYP)(GLenum ))gl_get_proc(api, "glBlendEquation");

	return ext;
}

struct gl_ARB_indirect_parameters *gl_resolve_ARB_indirect_parameters(struct gl_api *api, struct gl_ARB_indirect_parameters *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->MultiDrawArraysIndirectCountARB = (void (APIENTRYP)(GLenum , GLintptr , GLintptr , GLsizei , GLsizei ))gl_get_proc(api, "glMultiDrawArraysIndirectCountARB");
	ext->MultiDrawElementsIndirectCountARB = (void (APIENTRYP)(GLenum , GLenum , GLintptr , GLintptr , GLsizei , GLsizei ))gl_get_proc(api, "glMultiDrawElementsIndirectCountARB");

	return ext;
}

struct gl_ARB_instanced_arrays *gl_resolve_ARB_instanced_arrays(struct gl_api *api, struct gl_ARB_instanced_arrays *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->VertexAttribDivisorARB = (void (APIENTRYP)(GLuint , GLuint ))gl_get_proc(api, "glVertexAttribDivisorARB");

	return ext;
}

struct gl_ARB_internalformat_query *gl_resolve_ARB_internalformat_query(struct gl_api *api, struct gl_ARB_internalformat_query *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->GetInternalformativ = (void (APIENTRYP)(GLenum , GLenum , GLenum , GLsizei , GLint *))gl_get_proc(api, "glGetInternalformativ");

	return ext;
}

struct gl_ARB_internalformat_query2 *gl_resolve_ARB_internalformat_query2(struct gl_api *api, struct gl_ARB_internalformat_query2 *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->GetInternalformati64v = (void (APIENTRYP)(GLenum , GLenum , GLenum , GLsizei , GLint64 *))gl_get_proc(api, "glGetInternalformati64v");

	return ext;
}

struct gl_ARB_invalidate_subdata *gl_resolve_ARB_invalidate_subdata(struct gl_api *api, struct gl_ARB_invalidate_subdata *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->InvalidateTexSubImage = (void (APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei ))gl_get_proc(api, "glInvalidateTexSubImage");
	ext->InvalidateTexImage = (void (APIENTRYP)(GLuint , GLint ))gl_get_proc(api, "glInvalidateTexImage");
	ext->InvalidateBufferSubData = (void (APIENTRYP)(GLuint , GLintptr , GLsizeiptr ))gl_get_proc(api, "glInvalidateBufferSubData");
	ext->InvalidateBufferData = (void (APIENTRYP)(GLuint ))gl_get_proc(api, "glInvalidateBufferData");
	ext->InvalidateFramebuffer = (void (APIENTRYP)(GLenum , GLsizei , const GLenum *))gl_get_proc(api, "glInvalidateFramebuffer");
	ext->InvalidateSubFramebuffer = (void (APIENTRYP)(GLenum , GLsizei , const GLenum *, GLint , GLint , GLsizei , GLsizei ))gl_get_proc(api, "glInvalidateSubFramebuffer");

	return ext;
}

struct gl_ARB_map_buffer_range *gl_resolve_ARB_map_buffer_range(struct gl_api *api, struct gl_ARB_map_buffer_range *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->MapBufferRange = (void *(APIENTRYP)(GLenum , GLintptr , GLsizeiptr , GLbitfield ))gl_get_proc(api, "glMapBufferRange");
	ext->FlushMappedBufferRange = (void (APIENTRYP)(GLenum , GLintptr , GLsizeiptr ))gl_get_proc(api, "glFlushMappedBufferRange");

	return ext;
}

struct gl_ARB_multi_bind *gl_resolve_ARB_multi_bind(struct gl_api *api, struct gl_ARB_multi_bind *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->BindBuffersBase = (void (APIENTRYP)(GLenum , GLuint , GLsizei , const GLuint *))gl_get_proc(api, "glBindBuffersBase");
	ext->BindBuffersRange = (void (APIENTRYP)(GLenum , GLuint , GLsizei , const GLuint *, const GLintptr *, const GLsizeiptr *))gl_get_proc(api, "glBindBuffersRange");
	ext->BindTextures = (void (APIENTRYP)(GLuint , GLsizei , const GLuint *))gl_get_proc(api, "glBindTextures");
	ext->BindSamplers = (void (APIENTRYP)(GLuint , GLsizei , const GLuint *))gl_get_proc(api, "glBindSamplers");
	ext->BindImageTextures = (void (APIENTRYP)(GLuint , GLsizei , const GLuint *))gl_get_proc(api, "glBindImageTextures");
	ext->BindVertexBuffers = (void (APIENTRYP)(GLuint , GLsizei , const GLuint *, const GLintptr *, const GLsizei *))gl_get_proc(api, "glBindVertexBuffers");

	return ext;
}

struct gl_ARB_multi_draw_indirect *gl_resolve_ARB_multi_draw_indirect(struct gl_api *api, struct gl_ARB_multi_draw_indirect *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->MultiDrawArraysIndirect = (void (APIENTRYP)(GLenum , const void *, GLsizei , GLsizei ))gl_get_proc(api, "glMultiDrawArraysIndirect");
	ext->MultiDrawElementsIndirect = (void (APIENTRYP)(GLenum , GLenum , const void *, GLsizei , GLsizei ))gl_get_proc(api, "glMultiDrawElementsIndirect");

	return ext;
}

struct gl_ARB_multisample *gl_resolve_ARB_multisample(struct gl_api *api, struct gl_ARB_multisample *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->SampleCoverageARB = (void (APIENTRYP)(GLfloat , GLboolean ))gl_get_proc(api, "glSampleCoverageARB");

	return ext;
}

struct gl_ARB_occlusion_query *gl_resolve_ARB_occlusion_query(struct gl_api *api, struct gl_ARB_occlusion_query *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->GenQueriesARB = (void (APIENTRYP)(GLsizei , GLuint *))gl_get_proc(api, "glGenQueriesARB");
	ext->DeleteQueriesARB = (void (APIENTRYP)(GLsizei , const GLuint *))gl_get_proc(api, "glDeleteQueriesARB");
	ext->IsQueryARB = (GLboolean (APIENTRYP)(GLuint ))gl_get_proc(api, "glIsQueryARB");
	ext->BeginQueryARB = (void (APIENTRYP)(GLenum , GLuint ))gl_get_proc(api, "glBeginQueryARB");
	ext->EndQueryARB = (void (APIENTRYP)(GLenum ))gl_get_proc(api, "glEndQueryARB");
	ext->GetQueryivARB = (void (APIENTRYP)(GLenum , GLenum , GLint *))gl_get_proc(api, "glGetQueryivARB");
	ext->GetQueryObjectivARB = (void (APIENTRYP)(GLuint , GLenum , GLint *))gl_get_proc(api, "glGetQueryObjectivARB");
	ext->GetQueryObjectuivARB = (void (APIENTRYP)(GLuint , GLenum , GLuint *))gl_get_proc(api, "glGetQueryObjectuivARB");

	return ext;
}

struct gl_ARB_parallel_shader_compile *gl_resolve_ARB_parallel_shader_compile(struct gl_api *api, struct gl_ARB_parallel_shader_compile *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->MaxShaderCompilerThreadsARB = (void (APIENTRYP)(GLuint ))gl_get_proc(api, "glMaxShaderCompilerThreadsARB");

	return ext;
}

struct gl_ARB_point_parameters *gl_resolve_ARB_point_parameters(struct gl_api *api, struct gl_ARB_point_parameters *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->PointParameterfARB = (void (APIENTRYP)(GLenum , GLfloat ))gl_get_proc(api, "glPointParameterfARB");
	ext->PointParameterfvARB = (void (APIENTRYP)(GLenum , const GLfloat *))gl_get_proc(api, "glPointParameterfvARB");

	return ext;
}

struct gl_ARB_program_interface_query *gl_resolve_ARB_program_interface_query(struct gl_api *api, struct gl_ARB_program_interface_query *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->GetProgramInterfaceiv = (void (APIENTRYP)(GLuint , GLenum , GLenum , GLint *))gl_get_proc(api, "glGetProgramInterfaceiv");
	ext->GetProgramResourceIndex = (GLuint (APIENTRYP)(GLuint , GLenum , const GLchar *))gl_get_proc(api, "glGetProgramResourceIndex");
	ext->GetProgramResourceName = (void (APIENTRYP)(GLuint , GLenum , GLuint , GLsizei , GLsizei *, GLchar *))gl_get_proc(api, "glGetProgramResourceName");
	ext->GetProgramResourceiv = (void (APIENTRYP)(GLuint , GLenum , GLuint , GLsizei , const GLenum *, GLsizei , GLsizei *, GLint *))gl_get_proc(api, "glGetProgramResourceiv");
	ext->GetProgramResourceLocation = (GLint (APIENTRYP)(GLuint , GLenum , const GLchar *))gl_get_proc(api, "glGetProgramResourceLocation");
	ext->GetProgramResourceLocationIndex = (GLint (APIENTRYP)(GLuint , GLenum , const GLchar *))gl_get_proc(api, "glGetProgramResourceLocationIndex");

	return ext;
}

struct gl_ARB_provoking_vertex *gl_resolve_ARB_provoking_vertex(struct gl_api *api, struct gl_ARB_provoking_vertex *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->ProvokingVertex = (void (APIENTRYP)(GLenum ))gl_get_proc(api, "glProvokingVertex");

	return ext;
}

struct gl_ARB_robustness *gl_resolve_ARB_robustness(struct gl_api *api, struct gl_ARB_robustness *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->GetGraphicsResetStatusARB = (GLenum (APIENTRYP)(void))gl_get_proc(api, "glGetGraphicsResetStatusARB");
	ext->GetnTexImageARB = (void (APIENTRYP)(GLenum , GLint , GLenum , GLenum , GLsizei , void *))gl_get_proc(api, "glGetnTexImageARB");
	ext->ReadnPixelsARB = (void (APIENTRYP)(GLint , GLint , GLsizei , GLsizei , GLenum , GLenum , GLsizei , void *))gl_get_proc(api, "glReadnPixelsARB");
	ext->GetnCompressedTexImageARB = (void (APIENTRYP)(GLenum , GLint , GLsizei , void *))gl_get_proc(api, "glGetnCompressedTexImageARB");
	ext->GetnUniformfvARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLfloat *))gl_get_proc(api, "glGetnUniformfvARB");
	ext->GetnUniformivARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLint *))gl_get_proc(api, "glGetnUniformivARB");
	ext->GetnUniformuivARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLuint *))gl_get_proc(api, "glGetnUniformuivARB");
	ext->GetnUniformdvARB = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLdouble *))gl_get_proc(api, "glGetnUniformdvARB");

	return ext;
}

struct gl_ARB_sample_locations *gl_resolve_ARB_sample_locations(struct gl_api *api, struct gl_ARB_sample_locations *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->FramebufferSampleLocationsfvARB = (void (APIENTRYP)(GLenum , GLuint , GLsizei , const GLfloat *))gl_get_proc(api, "glFramebufferSampleLocationsfvARB");
	ext->NamedFramebufferSampleLocationsfvARB = (void (APIENTRYP)(GLuint , GLuint , GLsizei , const GLfloat *))gl_get_proc(api, "glNamedFramebufferSampleLocationsfvARB");
	ext->EvaluateDepthValuesARB = (void (APIENTRYP)(void))gl_get_proc(api, "glEvaluateDepthValuesARB");

	return ext;
}

struct gl_ARB_sample_shading *gl_resolve_ARB_sample_shading(struct gl_api *api, struct gl_ARB_sample_shading *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->MinSampleShadingARB = (void (APIENTRYP)(GLfloat ))gl_get_proc(api, "glMinSampleShadingARB");

	return ext;
}

struct gl_ARB_sampler_objects *gl_resolve_ARB_sampler_objects(struct gl_api *api, struct gl_ARB_sampler_objects *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->GenSamplers = (void (APIENTRYP)(GLsizei , GLuint *))gl_get_proc(api, "glGenSamplers");
	ext->DeleteSamplers = (void (APIENTRYP)(GLsizei , const GLuint *))gl_get_proc(api, "glDeleteSamplers");
	ext->IsSampler = (GLboolean (APIENTRYP)(GLuint ))gl_get_proc(api, "glIsSampler");
	ext->BindSampler = (void (APIENTRYP)(GLuint , GLuint ))gl_get_proc(api, "glBindSampler");
	ext->SamplerParameteri = (void (APIENTRYP)(GLuint , GLenum , GLint ))gl_get_proc(api, "glSamplerParameteri");
	ext->SamplerParameteriv = (void (APIENTRYP)(GLuint , GLenum , const GLint *))gl_get_proc(api, "glSamplerParameteriv");
	ext->SamplerParameterf = (void (APIENTRYP)(GLuint , GLenum , GLfloat ))gl_get_proc(api, "glSamplerParameterf");
	ext->SamplerParameterfv = (void (APIENTRYP)(GLuint , GLenum , const GLfloat *))gl_get_proc(api, "glSamplerParameterfv");
	ext->SamplerParameterIiv = (void (APIENTRYP)(GLuint , GLenum , const GLint *))gl_get_proc(api, "glSamplerParameterIiv");
	ext->SamplerParameterIuiv = (void (APIENTRYP)(GLuint , GLenum , const GLuint *))gl_get_proc(api, "glSamplerParameterIuiv");
	ext->GetSamplerParameteriv = (void (APIENTRYP)(GLuint , GLenum , GLint *))gl_get_proc(api, "glGetSamplerParameteriv");
	ext->GetSamplerParameterIiv = (void (APIENTRYP)(GLuint , GLenum , GLint *))gl_get_proc(api, "glGetSamplerParameterIiv");
	ext->GetSamplerParameterfv = (void (APIENTRYP)(GLuint , GLenum , GLfloat *))gl_get_proc(api, "glGetSamplerParameterfv");
	ext->GetSamplerParameterIuiv = (void (APIENTRYP)(GLuint , GLenum , GLuint *))gl_get_proc(api, "glGetSamplerParameterIuiv");

	return ext;
}

struct gl_ARB_separate_shader_objects *gl_resolve_ARB_separate_shader_objects(struct gl_api *api, struct gl_ARB_separate_shader_objects *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->UseProgramStages = (void (APIENTRYP)(GLuint , GLbitfield , GLuint ))gl_get_proc(api, "glUseProgramStages");
	ext->ActiveShaderProgram = (void (APIENTRYP)(GLuint , GLuint ))gl_get_proc(api, "glActiveShaderProgram");
	ext->CreateShaderProgramv = (GLuint (APIENTRYP)(GLenum , GLsizei , const GLchar *const*))gl_get_proc(api, "glCreateShaderProgramv");
	ext->BindProgramPipeline = (void (APIENTRYP)(GLuint ))gl_get_proc(api, "glBindProgramPipeline");
	ext->DeleteProgramPipelines = (void (APIENTRYP)(GLsizei , const GLuint *))gl_get_proc(api, "glDeleteProgramPipelines");
	ext->GenProgramPipelines = (void (APIENTRYP)(GLsizei , GLuint *))gl_get_proc(api, "glGenProgramPipelines");
	ext->IsProgramPipeline = (GLboolean (APIENTRYP)(GLuint ))gl_get_proc(api, "glIsProgramPipeline");
	ext->GetProgramPipelineiv = (void (APIENTRYP)(GLuint , GLenum , GLint *))gl_get_proc(api, "glGetProgramPipelineiv");
	ext->ProgramParameteri = (void (APIENTRYP)(GLuint , GLenum , GLint ))gl_get_proc(api, "glProgramParameteri");
	ext->ProgramUniform1i = (void (APIENTRYP)(GLuint , GLint , GLint ))gl_get_proc(api, "glProgramUniform1i");
	ext->ProgramUniform1iv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLint *))gl_get_proc(api, "glProgramUniform1iv");
	ext->ProgramUniform1f = (void (APIENTRYP)(GLuint , GLint , GLfloat ))gl_get_proc(api, "glProgramUniform1f");
	ext->ProgramUniform1fv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLfloat *))gl_get_proc(api, "glProgramUniform1fv");
	ext->ProgramUniform1d = (void (APIENTRYP)(GLuint , GLint , GLdouble ))gl_get_proc(api, "glProgramUniform1d");
	ext->ProgramUniform1dv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLdouble *))gl_get_proc(api, "glProgramUniform1dv");
	ext->ProgramUniform1ui = (void (APIENTRYP)(GLuint , GLint , GLuint ))gl_get_proc(api, "glProgramUniform1ui");
	ext->ProgramUniform1uiv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLuint *))gl_get_proc(api, "glProgramUniform1uiv");
	ext->ProgramUniform2i = (void (APIENTRYP)(GLuint , GLint , GLint , GLint ))gl_get_proc(api, "glProgramUniform2i");
	ext->ProgramUniform2iv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLint *))gl_get_proc(api, "glProgramUniform2iv");
	ext->ProgramUniform2f = (void (APIENTRYP)(GLuint , GLint , GLfloat , GLfloat ))gl_get_proc(api, "glProgramUniform2f");
	ext->ProgramUniform2fv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLfloat *))gl_get_proc(api, "glProgramUniform2fv");
	ext->ProgramUniform2d = (void (APIENTRYP)(GLuint , GLint , GLdouble , GLdouble ))gl_get_proc(api, "glProgramUniform2d");
	ext->ProgramUniform2dv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLdouble *))gl_get_proc(api, "glProgramUniform2dv");
	ext->ProgramUniform2ui = (void (APIENTRYP)(GLuint , GLint , GLuint , GLuint ))gl_get_proc(api, "glProgramUniform2ui");
	ext->ProgramUniform2uiv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLuint *))gl_get_proc(api, "glProgramUniform2uiv");
	ext->ProgramUniform3i = (void (APIENTRYP)(GLuint , GLint , GLint , GLint , GLint ))gl_get_proc(api, "glProgramUniform3i");
	ext->ProgramUniform3iv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLint *))gl_get_proc(api, "glProgramUniform3iv");
	ext->ProgramUniform3f = (void (APIENTRYP)(GLuint , GLint , GLfloat , GLfloat , GLfloat ))gl_get_proc(api, "glProgramUniform3f");
	ext->ProgramUniform3fv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLfloat *))gl_get_proc(api, "glProgramUniform3fv");
	ext->ProgramUniform3d = (void (APIENTRYP)(GLuint , GLint , GLdouble , GLdouble , GLdouble ))gl_get_proc(api, "glProgramUniform3d");
	ext->ProgramUniform3dv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLdouble *))gl_get_proc(api, "glProgramUniform3dv");
	ext->ProgramUniform3ui = (void (APIENTRYP)(GLuint , GLint , GLuint , GLuint , GLuint ))gl_get_proc(api, "glProgramUniform3ui");
	ext->ProgramUniform3uiv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLuint *))gl_get_proc(api, "glProgramUniform3uiv");
	ext->ProgramUniform4i = (void (APIENTRYP)(GLuint , GLint , GLint , GLint , GLint , GLint ))gl_get_proc(api, "glProgramUniform4i");
	ext->ProgramUniform4iv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLint *))gl_get_proc(api, "glProgramUniform4iv");
	ext->ProgramUniform4f = (void (APIENTRYP)(GLuint , GLint , GLfloat , GLfloat , GLfloat , GLfloat ))gl_get_proc(api, "glProgramUniform4f");
	ext->ProgramUniform4fv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLfloat *))gl_get_proc(api, "glProgramUniform4fv");
	ext->ProgramUniform4d = (void (APIENTRYP)(GLuint , GLint , GLdouble , GLdouble , GLdouble , GLdouble ))gl_get_proc(api, "glProgramUniform4d");
	ext->ProgramUniform4dv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLdouble *))gl_get_proc(api, "glProgramUniform4dv");
	ext->ProgramUniform4ui = (void (APIENTRYP)(GLuint , GLint , GLuint , GLuint , GLuint , GLuint ))gl_get_proc(api, "glProgramUniform4ui");
	ext->ProgramUniform4uiv = (void (APIENTRYP)(GLuint , GLint , GLsizei , const GLuint *))gl_get_proc(api, "glProgramUniform4uiv");
	ext->ProgramUniformMatrix2fv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *))gl_get_proc(api, "glProgramUniformMatrix2fv");
	ext->ProgramUniformMatrix3fv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *))gl_get_proc(api, "glProgramUniformMatrix3fv");
	ext->ProgramUniformMatrix4fv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *))gl_get_proc(api, "glProgramUniformMatrix4fv");
	ext->ProgramUniformMatrix2dv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glProgramUniformMatrix2dv");
	ext->ProgramUniformMatrix3dv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glProgramUniformMatrix3dv");
	ext->ProgramUniformMatrix4dv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glProgramUniformMatrix4dv");
	ext->ProgramUniformMatrix2x3fv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *))gl_get_proc(api, "glProgramUniformMatrix2x3fv");
	ext->ProgramUniformMatrix3x2fv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *))gl_get_proc(api, "glProgramUniformMatrix3x2fv");
	ext->ProgramUniformMatrix2x4fv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *))gl_get_proc(api, "glProgramUniformMatrix2x4fv");
	ext->ProgramUniformMatrix4x2fv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *))gl_get_proc(api, "glProgramUniformMatrix4x2fv");
	ext->ProgramUniformMatrix3x4fv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *))gl_get_proc(api, "glProgramUniformMatrix3x4fv");
	ext->ProgramUniformMatrix4x3fv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLfloat *))gl_get_proc(api, "glProgramUniformMatrix4x3fv");
	ext->ProgramUniformMatrix2x3dv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glProgramUniformMatrix2x3dv");
	ext->ProgramUniformMatrix3x2dv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glProgramUniformMatrix3x2dv");
	ext->ProgramUniformMatrix2x4dv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glProgramUniformMatrix2x4dv");
	ext->ProgramUniformMatrix4x2dv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glProgramUniformMatrix4x2dv");
	ext->ProgramUniformMatrix3x4dv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glProgramUniformMatrix3x4dv");
	ext->ProgramUniformMatrix4x3dv = (void (APIENTRYP)(GLuint , GLint , GLsizei , GLboolean , const GLdouble *))gl_get_proc(api, "glProgramUniformMatrix4x3dv");
	ext->ValidateProgramPipeline = (void (APIENTRYP)(GLuint ))gl_get_proc(api, "glValidateProgramPipeline");
	ext->GetProgramPipelineInfoLog = (void (APIENTRYP)(GLuint , GLsizei , GLsizei *, GLchar *))gl_get_proc(api, "glGetProgramPipelineInfoLog");

	return ext;
}

struct gl_ARB_shader_atomic_counters *gl_resolve_ARB_shader_atomic_counters(struct gl_api *api, struct gl_ARB_shader_atomic_counters *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->GetActiveAtomicCounterBufferiv = (void (APIENTRYP)(GLuint , GLuint , GLenum , GLint *))gl_get_proc(api, "glGetActiveAtomicCounterBufferiv");

	return ext;
}

struct gl_ARB_shader_image_load_store *gl_resolve_ARB_shader_image_load_store(struct gl_api *api, struct gl_ARB_shader_image_load_store *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->BindImageTexture = (void (APIENTRYP)(GLuint , GLuint , GLint , GLboolean , GLint , GLenum , GLenum ))gl_get_proc(api, "glBindImageTexture");
	ext->MemoryBarrier = (void (APIENTRYP)(GLbitfield ))gl_get_proc(api, "glMemoryBarrier");

	return ext;
}

struct gl_ARB_shader_storage_buffer_object *gl_resolve_ARB_shader_storage_buffer_object(struct gl_api *api, struct gl_ARB_shader_storage_buffer_object *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->ShaderStorageBlockBinding = (void (APIENTRYP)(GLuint , GLuint , GLuint ))gl_get_proc(api, "glShaderStorageBlockBinding");

	return ext;
}

struct gl_ARB_shader_subroutine *gl_resolve_ARB_shader_subroutine(struct gl_api *api, struct gl_ARB_shader_subroutine *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->GetSubroutineUniformLocation = (GLint (APIENTRYP)(GLuint , GLenum , const GLchar *))gl_get_proc(api, "glGetSubroutineUniformLocation");
	ext->GetSubroutineIndex = (GLuint (APIENTRYP)(GLuint , GLenum , const GLchar *))gl_get_proc(api, "glGetSubroutineIndex");
	ext->GetActiveSubroutineUniformiv = (void (APIENTRYP)(GLuint , GLenum , GLuint , GLenum , GLint *))gl_get_proc(api, "glGetActiveSubroutineUniformiv");
	ext->GetActiveSubroutineUniformName = (void (APIENTRYP)(GLuint , GLenum , GLuint , GLsizei , GLsizei *, GLchar *))gl_get_proc(api, "glGetActiveSubroutineUniformName");
	ext->GetActiveSubroutineName = (void (APIENTRYP)(GLuint , GLenum , GLuint , GLsizei , GLsizei *, GLchar *))gl_get_proc(api, "glGetActiveSubroutineName");
	ext->UniformSubroutinesuiv = (void (APIENTRYP)(GLenum , GLsizei , const GLuint *))gl_get_proc(api, "glUniformSubroutinesuiv");
	ext->GetUniformSubroutineuiv = (void (APIENTRYP)(GLenum , GLint , GLuint *))gl_get_proc(api, "glGetUniformSubroutineuiv");
	ext->GetProgramStageiv = (void (APIENTRYP)(GLuint , GLenum , GLenum , GLint *))gl_get_proc(api, "glGetProgramStageiv");

	return ext;
}

struct gl_ARB_shading_language_include *gl_resolve_ARB_shading_language_include(struct gl_api *api, struct gl_ARB_shading_language_include *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->NamedStringARB = (void (APIENTRYP)(GLenum , GLint , const GLchar *, GLint , const GLchar *))gl_get_proc(api, "glNamedStringARB");
	ext->DeleteNamedStringARB = (void (APIENTRYP)(GLint , const GLchar *))gl_get_proc(api, "glDeleteNamedStringARB");
	ext->CompileShaderIncludeARB = (void (APIENTRYP)(GLuint , GLsizei , const GLchar *const*, const GLint *))gl_get_proc(api, "glCompileShaderIncludeARB");
	ext->IsNamedStringARB = (GLboolean (APIENTRYP)(GLint , const GLchar *))gl_get_proc(api, "glIsNamedStringARB");
	ext->GetNamedStringARB = (void (APIENTRYP)(GLint , const GLchar *, GLsizei , GLint *, GLchar *))gl_get_proc(api, "glGetNamedStringARB");
	ext->GetNamedStringivARB = (void (APIENTRYP)(GLint , const GLchar *, GLenum , GLint *))gl_get_proc(api, "glGetNamedStringivARB");

	return ext;
}

struct gl_ARB_sparse_buffer *gl_resolve_ARB_sparse_buffer(struct gl_api *api, struct gl_ARB_sparse_buffer *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->BufferPageCommitmentARB = (void (APIENTRYP)(GLenum , GLintptr , GLsizeiptr , GLboolean ))gl_get_proc(api, "glBufferPageCommitmentARB");
	ext->NamedBufferPageCommitmentEXT = (void (APIENTRYP)(GLuint , GLintptr , GLsizeiptr , GLboolean ))gl_get_proc(api, "glNamedBufferPageCommitmentEXT");
	ext->NamedBufferPageCommitmentARB = (void (APIENTRYP)(GLuint , GLintptr , GLsizeiptr , GLboolean ))gl_get_proc(api, "glNamedBufferPageCommitmentARB");

	return ext;
}

struct gl_ARB_sparse_texture *gl_resolve_ARB_sparse_texture(struct gl_api *api, struct gl_ARB_sparse_texture *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->TexPageCommitmentARB = (void (APIENTRYP)(GLenum , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLboolean ))gl_get_proc(api, "glTexPageCommitmentARB");

	return ext;
}

struct gl_ARB_gl_spirv *gl_resolve_ARB_gl_spirv(struct gl_api *api, struct gl_ARB_gl_spirv *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->SpecializeShaderARB = (void (APIENTRYP)(GLuint , const GLchar *, GLuint , const GLuint *, const GLuint *))gl_get_proc(api, "glSpecializeShaderARB");

	return ext;
}

struct gl_ARB_sync *gl_resolve_ARB_sync(struct gl_api *api, struct gl_ARB_sync *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->FenceSync = (GLsync (APIENTRYP)(GLenum , GLbitfield ))gl_get_proc(api, "glFenceSync");
	ext->IsSync = (GLboolean (APIENTRYP)(GLsync ))gl_get_proc(api, "glIsSync");
	ext->DeleteSync = (void (APIENTRYP)(GLsync ))gl_get_proc(api, "glDeleteSync");
	ext->ClientWaitSync = (GLenum (APIENTRYP)(GLsync , GLbitfield , GLuint64 ))gl_get_proc(api, "glClientWaitSync");
	ext->WaitSync = (void (APIENTRYP)(GLsync , GLbitfield , GLuint64 ))gl_get_proc(api, "glWaitSync");
	ext->GetInteger64v = (void (APIENTRYP)(GLenum , GLint64 *))gl_get_proc(api, "glGetInteger64v");
	ext->GetSynciv = (void (APIENTRYP)(GLsync , GLenum , GLsizei , GLsizei *, GLint *))gl_get_proc(api, "glGetSynciv");

	return ext;
}

struct gl_ARB_tessellation_shader *gl_resolve_ARB_tessellation_shader(struct gl_api *api, struct gl_ARB_tessellation_shader *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->PatchParameteri = (void (APIENTRYP)(GLenum , GLint ))gl_get_proc(api, "glPatchParameteri");
	ext->PatchParameterfv = (void (APIENTRYP)(GLenum , const GLfloat *))gl_get_proc(api, "glPatchParameterfv");

	return ext;
}

struct gl_ARB_texture_barrier *gl_resolve_ARB_texture_barrier(struct gl_api *api, struct gl_ARB_texture_barrier *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->TextureBarrier = (void (APIENTRYP)(void))gl_get_proc(api, "glTextureBarrier");

	return ext;
}

struct gl_ARB_texture_buffer_object *gl_resolve_ARB_texture_buffer_object(struct gl_api *api, struct gl_ARB_texture_buffer_object *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->TexBufferARB = (void (APIENTRYP)(GLenum , GLenum , GLuint ))gl_get_proc(api, "glTexBufferARB");

	return ext;
}

struct gl_ARB_texture_buffer_range *gl_resolve_ARB_texture_buffer_range(struct gl_api *api, struct gl_ARB_texture_buffer_range *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->TexBufferRange = (void (APIENTRYP)(GLenum , GLenum , GLuint , GLintptr , GLsizeiptr ))gl_get_proc(api, "glTexBufferRange");

	return ext;
}

struct gl_ARB_texture_compression *gl_resolve_ARB_texture_compression(struct gl_api *api, struct gl_ARB_texture_compression *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->CompressedTexImage3DARB = (void (APIENTRYP)(GLenum , GLint , GLenum , GLsizei , GLsizei , GLsizei , GLint , GLsizei , const void *))gl_get_proc(api, "glCompressedTexImage3DARB");
	ext->CompressedTexImage2DARB = (void (APIENTRYP)(GLenum , GLint , GLenum , GLsizei , GLsizei , GLint , GLsizei , const void *))gl_get_proc(api, "glCompressedTexImage2DARB");
	ext->CompressedTexImage1DARB = (void (APIENTRYP)(GLenum , GLint , GLenum , GLsizei , GLint , GLsizei , const void *))gl_get_proc(api, "glCompressedTexImage1DARB");
	ext->CompressedTexSubImage3DARB = (void (APIENTRYP)(GLenum , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLenum , GLsizei , const void *))gl_get_proc(api, "glCompressedTexSubImage3DARB");
	ext->CompressedTexSubImage2DARB = (void (APIENTRYP)(GLenum , GLint , GLint , GLint , GLsizei , GLsizei , GLenum , GLsizei , const void *))gl_get_proc(api, "glCompressedTexSubImage2DARB");
	ext->CompressedTexSubImage1DARB = (void (APIENTRYP)(GLenum , GLint , GLint , GLsizei , GLenum , GLsizei , const void *))gl_get_proc(api, "glCompressedTexSubImage1DARB");
	ext->GetCompressedTexImageARB = (void (APIENTRYP)(GLenum , GLint , void *))gl_get_proc(api, "glGetCompressedTexImageARB");

	return ext;
}

struct gl_ARB_texture_multisample *gl_resolve_ARB_texture_multisample(struct gl_api *api, struct gl_ARB_texture_multisample *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->TexImage2DMultisample = (void (APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei , GLboolean ))gl_get_proc(api, "glTexImage2DMultisample");
	ext->TexImage3DMultisample = (void (APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei , GLsizei , GLboolean ))gl_get_proc(api, "glTexImage3DMultisample");
	ext->GetMultisamplefv = (void (APIENTRYP)(GLenum , GLuint , GLfloat *))gl_get_proc(api, "glGetMultisamplefv");
	ext->SampleMaski = (void (APIENTRYP)(GLuint , GLbitfield ))gl_get_proc(api, "glSampleMaski");

	return ext;
}

struct gl_ARB_texture_storage *gl_resolve_ARB_texture_storage(struct gl_api *api, struct gl_ARB_texture_storage *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->TexStorage1D = (void (APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei ))gl_get_proc(api, "glTexStorage1D");
	ext->TexStorage2D = (void (APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei ))gl_get_proc(api, "glTexStorage2D");
	ext->TexStorage3D = (void (APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei , GLsizei ))gl_get_proc(api, "glTexStorage3D");

	return ext;
}

struct gl_ARB_texture_storage_multisample *gl_resolve_ARB_texture_storage_multisample(struct gl_api *api, struct gl_ARB_texture_storage_multisample *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->TexStorage2DMultisample = (void (APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei , GLboolean ))gl_get_proc(api, "glTexStorage2DMultisample");
	ext->TexStorage3DMultisample = (void (APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei , GLsizei , GLboolean ))gl_get_proc(api, "glTexStorage3DMultisample");

	return ext;
}

struct gl_ARB_texture_view *gl_resolve_ARB_texture_view(struct gl_api *api, struct gl_ARB_texture_view *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->TextureView = (void (APIENTRYP)(GLuint , GLenum , GLuint , GLenum , GLuint , GLuint , GLuint , GLuint ))gl_get_proc(api, "glTextureView");

	return ext;
}

struct gl_ARB_timer_query *gl_resolve_ARB_timer_query(struct gl_api *api, struct gl_ARB_timer_query *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->QueryCounter = (void (APIENTRYP)(GLuint , GLenum ))gl_get_proc(api, "glQueryCounter");
	ext->GetQueryObjecti64v = (void (APIENTRYP)(GLuint , GLenum , GLint64 *))gl_get_proc(api, "glGetQueryObjecti64v");
	ext->GetQueryObjectui64v = (void (APIENTRYP)(GLuint , GLenum , GLuint64 *))gl_get_proc(api, "glGetQueryObjectui64v");

	return ext;
}

struct gl_ARB_transform_feedback2 *gl_resolve_ARB_transform_feedback2(struct gl_api *api, struct gl_ARB_transform_feedback2 *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->BindTransformFeedback = (void (APIENTRYP)(GLenum , GLuint ))gl_get_proc(api, "glBindTransformFeedback");
	ext->DeleteTransformFeedbacks = (void (APIENTRYP)(GLsizei , const GLuint *))gl_get_proc(api, "glDeleteTransformFeedbacks");
	ext->GenTransformFeedbacks = (void (APIENTRYP)(GLsizei , GLuint *))gl_get_proc(api, "glGenTransformFeedbacks");
	ext->IsTransformFeedback = (GLboolean (APIENTRYP)(GLuint ))gl_get_proc(api, "glIsTransformFeedback");
	ext->PauseTransformFeedback = (void (APIENTRYP)(void))gl_get_proc(api, "glPauseTransformFeedback");
	ext->ResumeTransformFeedback = (void (APIENTRYP)(void))gl_get_proc(api, "glResumeTransformFeedback");
	ext->DrawTransformFeedback = (void (APIENTRYP)(GLenum , GLuint ))gl_get_proc(api, "glDrawTransformFeedback");

	return ext;
}

struct gl_ARB_transform_feedback3 *gl_resolve_ARB_transform_feedback3(struct gl_api *api, struct gl_ARB_transform_feedback3 *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->DrawTransformFeedbackStream = (void (APIENTRYP)(GLenum , GLuint , GLuint ))gl_get_proc(api, "glDrawTransformFeedbackStream");
	ext->BeginQueryIndexed = (void (APIENTRYP)(GLenum , GLuint , GLuint ))gl_get_proc(api, "glBeginQueryIndexed");
	ext->EndQueryIndexed = (void (APIENTRYP)(GLenum , GLuint ))gl_get_proc(api, "glEndQueryIndexed");
	ext->GetQueryIndexediv = (void (APIENTRYP)(GLenum , GLuint , GLenum , GLint *))gl_get_proc(api, "glGetQueryIndexediv");

	return ext;
}

struct gl_ARB_transform_feedback_instanced *gl_resolve_ARB_transform_feedback_instanced(struct gl_api *api, struct gl_ARB_transform_feedback_instanced *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->DrawTransformFeedbackInstanced = (void (APIENTRYP)(GLenum , GLuint , GLsizei ))gl_get_proc(api, "glDrawTransformFeedbackInstanced");
	ext->DrawTransformFeedbackStreamInstanced = (void (APIENTRYP)(GLenum , GLuint , GLuint , GLsizei ))gl_get_proc(api, "glDrawTransformFeedbackStreamInstanced");

	return ext;
}

struct gl_ARB_uniform_buffer_object *gl_resolve_ARB_uniform_buffer_object(struct gl_api *api, struct gl_ARB_uniform_buffer_object *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->GetUniformIndices = (void (APIENTRYP)(GLuint , GLsizei , const GLchar *const*, GLuint *))gl_get_proc(api, "glGetUniformIndices");
	ext->GetActiveUniformsiv = (void (APIENTRYP)(GLuint , GLsizei , const GLuint *, GLenum , GLint *))gl_get_proc(api, "glGetActiveUniformsiv");
	ext->GetActiveUniformName = (void (APIENTRYP)(GLuint , GLuint , GLsizei , GLsizei *, GLchar *))gl_get_proc(api, "glGetActiveUniformName");
	ext->GetUniformBlockIndex = (GLuint (APIENTRYP)(GLuint , const GLchar *))gl_get_proc(api, "glGetUniformBlockIndex");
	ext->GetActiveUniformBlockiv = (void (APIENTRYP)(GLuint , GLuint , GLenum , GLint *))gl_get_proc(api, "glGetActiveUniformBlockiv");
	ext->GetActiveUniformBlockName = (void (APIENTRYP)(GLuint , GLuint , GLsizei , GLsizei *, GLchar *))gl_get_proc(api, "glGetActiveUniformBlockName");
	ext->UniformBlockBinding = (void (APIENTRYP)(GLuint , GLuint , GLuint ))gl_get_proc(api, "glUniformBlockBinding");
	ext->BindBufferRange = (void (APIENTRYP)(GLenum , GLuint , GLuint , GLintptr , GLsizeiptr ))gl_get_proc(api, "glBindBufferRange");
	ext->BindBufferBase = (void (APIENTRYP)(GLenum , GLuint , GLuint ))gl_get_proc(api, "glBindBufferBase");
	ext->GetIntegeri_v = (void (APIENTRYP)(GLenum , GLuint , GLint *))gl_get_proc(api, "glGetIntegeri_v");

	return ext;
}

struct gl_ARB_vertex_attrib_64bit *gl_resolve_ARB_vertex_attrib_64bit(struct gl_api *api, struct gl_ARB_vertex_attrib_64bit *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->VertexAttribL1d = (void (APIENTRYP)(GLuint , GLdouble ))gl_get_proc(api, "glVertexAttribL1d");
	ext->VertexAttribL2d = (void (APIENTRYP)(GLuint , GLdouble , GLdouble ))gl_get_proc(api, "glVertexAttribL2d");
	ext->VertexAttribL3d = (void (APIENTRYP)(GLuint , GLdouble , GLdouble , GLdouble ))gl_get_proc(api, "glVertexAttribL3d");
	ext->VertexAttribL4d = (void (APIENTRYP)(GLuint , GLdouble , GLdouble , GLdouble , GLdouble ))gl_get_proc(api, "glVertexAttribL4d");
	ext->VertexAttribL1dv = (void (APIENTRYP)(GLuint , const GLdouble *))gl_get_proc(api, "glVertexAttribL1dv");
	ext->VertexAttribL2dv = (void (APIENTRYP)(GLuint , const GLdouble *))gl_get_proc(api, "glVertexAttribL2dv");
	ext->VertexAttribL3dv = (void (APIENTRYP)(GLuint , const GLdouble *))gl_get_proc(api, "glVertexAttribL3dv");
	ext->VertexAttribL4dv = (void (APIENTRYP)(GLuint , const GLdouble *))gl_get_proc(api, "glVertexAttribL4dv");
	ext->VertexAttribLPointer = (void (APIENTRYP)(GLuint , GLint , GLenum , GLsizei , const void *))gl_get_proc(api, "glVertexAttribLPointer");
	ext->GetVertexAttribLdv = (void (APIENTRYP)(GLuint , GLenum , GLdouble *))gl_get_proc(api, "glGetVertexAttribLdv");

	return ext;
}

struct gl_ARB_vertex_attrib_binding *gl_resolve_ARB_vertex_attrib_binding(struct gl_api *api, struct gl_ARB_vertex_attrib_binding *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->BindVertexBuffer = (void (APIENTRYP)(GLuint , GLuint , GLintptr , GLsizei ))gl_get_proc(api, "glBindVertexBuffer");
	ext->VertexAttribFormat = (void (APIENTRYP)(GLuint , GLint , GLenum , GLboolean , GLuint ))gl_get_proc(api, "glVertexAttribFormat");
	ext->VertexAttribIFormat = (void (APIENTRYP)(GLuint , GLint , GLenum , GLuint ))gl_get_proc(api, "glVertexAttribIFormat");
	ext->VertexAttribLFormat = (void (APIENTRYP)(GLuint , GLint , GLenum , GLuint ))gl_get_proc(api, "glVertexAttribLFormat");
	ext->VertexAttribBinding = (void (APIENTRYP)(GLuint , GLuint ))gl_get_proc(api, "glVertexAttribBinding");
	ext->VertexBindingDivisor = (void (APIENTRYP)(GLuint , GLuint ))gl_get_proc(api, "glVertexBindingDivisor");

	return ext;
}

struct gl_ARB_vertex_type_2_10_10_10_rev *gl_resolve_ARB_vertex_type_2_10_10_10_rev(struct gl_api *api, struct gl_ARB_vertex_type_2_10_10_10_rev *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->VertexAttribP1ui = (void (APIENTRYP)(GLuint , GLenum , GLboolean , GLuint ))gl_get_proc(api, "glVertexAttribP1ui");
	ext->VertexAttribP1uiv = (void (APIENTRYP)(GLuint , GLenum , GLboolean , const GLuint *))gl_get_proc(api, "glVertexAttribP1uiv");
	ext->VertexAttribP2ui = (void (APIENTRYP)(GLuint , GLenum , GLboolean , GLuint ))gl_get_proc(api, "glVertexAttribP2ui");
	ext->VertexAttribP2uiv = (void (APIENTRYP)(GLuint , GLenum , GLboolean , const GLuint *))gl_get_proc(api, "glVertexAttribP2uiv");
	ext->VertexAttribP3ui = (void (APIENTRYP)(GLuint , GLenum , GLboolean , GLuint ))gl_get_proc(api, "glVertexAttribP3ui");
	ext->VertexAttribP3uiv = (void (APIENTRYP)(GLuint , GLenum , GLboolean , const GLuint *))gl_get_proc(api, "glVertexAttribP3uiv");
	ext->VertexAttribP4ui = (void (APIENTRYP)(GLuint , GLenum , GLboolean , GLuint ))gl_get_proc(api, "glVertexAttribP4ui");
	ext->VertexAttribP4uiv = (void (APIENTRYP)(GLuint , GLenum , GLboolean , const GLuint *))gl_get_proc(api, "glVertexAttribP4uiv");

	return ext;
}

struct gl_ARB_viewport_array *gl_resolve_ARB_viewport_array(struct gl_api *api, struct gl_ARB_viewport_array *ext)
{
	if (!gl_has_ext(api, "GL_ARB_base_instance")) {
		return 0;
	}
	ext->ViewportArrayv = (void (APIENTRYP)(GLuint , GLsizei , const GLfloat *))gl_get_proc(api, "glViewportArrayv");
	ext->ViewportIndexedf = (void (APIENTRYP)(GLuint , GLfloat , GLfloat , GLfloat , GLfloat ))gl_get_proc(api, "glViewportIndexedf");
	ext->ViewportIndexedfv = (void (APIENTRYP)(GLuint , const GLfloat *))gl_get_proc(api, "glViewportIndexedfv");
	ext->ScissorArrayv = (void (APIENTRYP)(GLuint , GLsizei , const GLint *))gl_get_proc(api, "glScissorArrayv");
	ext->ScissorIndexed = (void (APIENTRYP)(GLuint , GLint , GLint , GLsizei , GLsizei ))gl_get_proc(api, "glScissorIndexed");
	ext->ScissorIndexedv = (void (APIENTRYP)(GLuint , const GLint *))gl_get_proc(api, "glScissorIndexedv");
	ext->DepthRangeArrayv = (void (APIENTRYP)(GLuint , GLsizei , const GLdouble *))gl_get_proc(api, "glDepthRangeArrayv");
	ext->DepthRangeIndexed = (void (APIENTRYP)(GLuint , GLdouble , GLdouble ))gl_get_proc(api, "glDepthRangeIndexed");
	ext->GetFloati_v = (void (APIENTRYP)(GLenum , GLuint , GLfloat *))gl_get_proc(api, "glGetFloati_v");
	ext->GetDoublei_v = (void (APIENTRYP)(GLenum , GLuint , GLdouble *))gl_get_proc(api, "glGetDoublei_v");

	return ext;
}
