#define GL_CORE_33_API(F) \
 \
/* Buffer Objects */ \
F(void,GenBuffers,(GLsizei, GLuint *)) \
F(void,DeleteBuffers,(GLsizei, GLuint const *)) \
F(void,BindBuffer,(GLenum, GLuint)) \
F(void,BindBufferBase,(GLenum, GLuint, GLuint)) \
F(void,BindBufferRange,(GLenum, GLuint, GLuint, GLintptr, GLsizeiptr)) \
F(void,BufferData,(GLenum, GLsizeiptr, GLvoid const *, GLenum)) \
F(void,BufferSubData,(GLenum, GLintptr, GLsizeiptr, GLvoid const *)) \
F(void,GetBufferSubData,(GLenum, GLintptr, GLsizeiptr, GLvoid *)) \
F(void *,MapBuffer,(GLenum, GLenum)) \
F(GLboolean,UnmapBuffer,(GLenum)) \
F(void,GetBufferParameteriv,(GLenum, GLenum, GLint *)) \
F(void,GetBufferPointerv,(GLenum, GLenum, GLvoid **)) \
F(GLboolean,IsBuffer,(GLuint)) \
F(void,TexBuffer,(GLenum, GLenum, GLuint)) \
 \
/* Shaders */ \
F(GLuint,CreateShader,(GLenum)) \
F(void,DeleteShader,(GLuint)) \
F(GLboolean,IsShader,(GLuint)) \
F(void,ShaderSource,(GLuint, GLsizei, GLchar const **, GLint const *)) \
F(void,GetShaderSource,(GLuint, GLsizei, GLsizei *, GLchar *)) \
F(void,CompileShader,(GLuint)) \
F(void,GetShaderiv,(GLuint, GLenum, GLint *)) \
F(void,GetShaderInfoLog,(GLuint, GLsizei, GLsizei *, GLchar *)) \
 \
/* Programs */ \
F(GLuint,CreateProgram,(void)) \
F(void,DeleteProgram,(GLuint)) \
F(void,AttachShader,(GLuint, GLuint)) \
F(void,DetachShader,(GLuint, GLuint)) \
F(void,GetAttachedShaders,(GLuint, GLsizei, GLsizei *, GLuint *)) \
F(void,GetProgramiv,(GLuint, GLenum, GLint *)) \
F(void,GetProgramInfoLog,(GLuint, GLsizei, GLsizei *, GLchar *)) \
F(GLint,GetFragDataLocation,(GLuint, GLchar const *)) \
F(void,BindFragDataLocation,(GLuint, GLuint, GLchar const *)) \
F(void,LinkProgram,(GLuint)) \
F(void,UseProgram,(GLuint)) \
F(GLint,GetUniformLocation,(GLuint, GLchar const *)) \
F(GLuint,GetUniformBlockIndex,(GLuint, GLchar const *)) \
F(void,UniformBlockBinding,(GLuint, GLuint, GLuint)) \
F(GLint,GetAttribLocation,(GLuint, GLchar const *)) \
F(void,BindAttribLocation,(GLuint, GLuint, GLchar const *)) \
F(void,EnableVertexAttribArray,(GLuint)) \
F(void,DisableVertexAttribArray,(GLuint)) \
F(GLint,VertexAttribPointer,(GLuint, GLint, GLenum, GLboolean, GLsizei, GLvoid const *)) \
 \
/* Uniforms */ \
F(void,Uniform1f,(GLint, GLfloat)) \
F(void,Uniform2f,(GLint, GLfloat, GLfloat)) \
F(void,Uniform3f,(GLint, GLfloat, GLfloat, GLfloat)) \
F(void,Uniform4f,(GLint, GLfloat, GLfloat, GLfloat, GLfloat)) \
F(void,Uniform1fv,(GLint, GLsizei, GLfloat const *)) \
F(void,Uniform2fv,(GLint, GLsizei, GLfloat const *)) \
F(void,Uniform3fv,(GLint, GLsizei, GLfloat const *)) \
F(void,Uniform4fv,(GLint, GLsizei, GLfloat const *)) \
F(void,Uniform1i,(GLint, GLint)) \
F(void,Uniform2i,(GLint, GLint, GLint)) \
F(void,Uniform3i,(GLint, GLint, GLint, GLint)) \
F(void,Uniform4i,(GLint, GLint, GLint, GLint, GLint)) \
F(void,Uniform1iv,(GLint, GLsizei, GLint const *)) \
F(void,Uniform2iv,(GLint, GLsizei, GLint const *)) \
F(void,Uniform3iv,(GLint, GLsizei, GLint const *)) \
F(void,Uniform4iv,(GLint, GLsizei, GLint const *)) \
F(void,UniformMatrix2fv,(GLint, GLsizei, GLboolean, GLfloat const *)) \
F(void,UniformMatrix3fv,(GLint, GLsizei, GLboolean, GLfloat const *)) \
F(void,UniformMatrix4fv,(GLint, GLsizei, GLboolean, GLfloat const *)) \
F(void,UniformMatrix2x3fv,(GLint, GLsizei, GLboolean, GLfloat const *)) \
F(void,UniformMatrix3x2fv,(GLint, GLsizei, GLboolean, GLfloat const *)) \
F(void,UniformMatrix2x4fv,(GLint, GLsizei, GLboolean, GLfloat const *)) \
F(void,UniformMatrix4x2fv,(GLint, GLsizei, GLboolean, GLfloat const *)) \
F(void,UniformMatrix3x4fv,(GLint, GLsizei, GLboolean, GLfloat const *)) \
F(void,UniformMatrix4x3fv,(GLint, GLsizei, GLboolean, GLfloat const *)) \
 \
/* Vertex Array Objects */ \
F(void,GenVertexArrays,(GLsizei, GLuint *)) \
F(void,BindVertexArray,(GLuint)) \
F(void,DeleteVertexArrays,(GLsizei, GLuint const *)) \
F(GLboolean,IsVertexArray,(GLuint)) \
 \
/* Texture Objects */ \
F(void,GenTextures,(GLsizei, GLuint *)) \
F(void,BindTexture,(GLenum, GLuint)) \
F(void,DeleteTextures,(GLsizei, GLuint const *)) \
F(GLboolean,IsTexture,(GLuint)) \
F(void,ActiveTexture,(GLenum)) \
F(void,TexParameterf,(GLenum, GLenum, GLfloat)) \
F(void,TexParameteri,(GLenum, GLenum, GLint)) \
F(void,GetTexParameterfv,(GLenum, GLenum, GLfloat *)) \
F(void,GetTexParameteriv,(GLenum, GLenum, GLint *)) \
F(void,GetTexParameterIiv,(GLenum, GLenum, GLint *)) \
F(void,GetTexParameterIuiv,(GLenum, GLenum, GLuint *)) \
F(void,TexImage1D,(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, GLvoid const *)) \
F(void,TexImage2D,(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, GLvoid const *)) \
F(void,TexImage2DMultisample,(GLenum, GLsizei, GLint, GLsizei, GLsizei, GLboolean)) \
F(void,TexImage3D,(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, GLvoid const *)) \
F(void,TexImage3DMultisample,(GLenum, GLsizei, GLint, GLsizei, GLsizei, GLsizei, GLboolean)) \
 \
/* Draw */ \
F(void,MultiDrawArrays,(GLenum, GLint const *, GLsizei const *, GLsizei)) \
F(void,MultiDrawElements,(GLenum, GLsizei const *, GLenum, void const *const *, GLsizei)) \
 \
/* Renderbuffers */ \
F(void,GenRenderbuffers,(GLsizei, GLuint *)) \
F(void,BindRenderbuffer,(GLenum, GLuint)) \
F(void,DeleteRenderbuffers,(GLsizei, GLuint *)) \
F(GLboolean,IsRenderbuffer,(GLuint)) \
F(void,RenderbufferStorage,(GLenum, GLenum, GLsizei, GLsizei)) \
F(void,RenderbufferStorageMultisample,(GLenum, GLsizei, GLenum, GLsizei, GLsizei)) \
 \
/* Framebuffers */ \
F(void,GenFramebuffers,(GLsizei, GLuint *)) \
F(void,BindFramebuffer,(GLenum, GLuint)) \
F(void,DeleteFramebuffers,(GLsizei, GLuint *)) \
F(GLboolean,IsFramebuffer,(GLuint)) \
F(void,DrawBuffers,(GLsizei, GLenum const *)) \
F(void,FramebufferRenderbuffer,(GLenum, GLenum, GLenum, GLuint)) \
F(void,FramebufferTexture,(GLenum, GLenum, GLuint, GLint)) \
F(void,FramebufferTexture1D,(GLenum, GLenum, GLenum, GLuint, GLint)) \
F(void,FramebufferTexture2D,(GLenum, GLenum, GLenum, GLuint, GLint)) \
F(void,FramebufferTexture3D,(GLenum, GLenum, GLenum, GLuint, GLint, GLint)) \
F(void,FramebufferTextureLayer,(GLenum, GLenum, GLuint, GLint, GLint)) \
F(GLenum,CheckFramebufferStatus,(GLenum)) \
F(void,BlitFramebuffer,(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum)) \
 \
/* Other */ \
F(void,GetBooleanv,(GLenum name, GLboolean *)) \
F(void,GetDoublev,(GLenum name, GLdouble *)) \
F(void,GetFloatv,(GLenum name, GLfloat *)) \
F(void,GetIntegerv,(GLenum name, GLint *)) \
F(void,GetInteger64v,(GLenum name, GLint64 *)) \
F(GLubyte const *,GetStringi,(GLenum name, GLuint index)) \

