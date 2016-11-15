#define GL_CORE_33_API(F) \
 \
/* Buffer Objects */ \
F(void,GenBuffers,(GLsizei, GLuint *)) \
F(void,DeleteBuffers,(GLsizei, GLuint const *)) \
F(void,BindBuffer,(GLenum, GLuint)) \
F(void,BufferData,(GLenum, GLsizeiptr, GLvoid const *, GLenum)) \
F(void,BufferSubData,(GLenum, GLintptr, GLsizeiptr, GLvoid const *)) \
F(void,GetBufferSubData,(GLenum, GLintptr, GLsizeiptr, GLvoid *)) \
F(void *,MapBuffer,(GLenum, GLenum)) \
F(GLboolean,UnmapBuffer,(GLenum)) \
F(void,GetBufferParameteriv,(GLenum, GLenum, GLint *)) \
F(void,GetBufferPointerv,(GLenum, GLenum, GLvoid **)) \
F(GLboolean,IsBuffer,(GLuint)) \
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
F(void,GetProgramiv,(GLuint, GLenum, GLint *)) \
F(void,GetProgramInfoLog,(GLuint, GLsizei, GLsizei *, GLchar *)) \
F(GLint,GetFragDataLocation,(GLuint, GLchar const *)) \
F(void,BindFragDataLocation,(GLuint, GLuint, GLchar const *)) \
F(void,LinkProgram,(GLuint)) \
F(void,UseProgram,(GLuint)) \
F(GLint,GetUniformLocation,(GLuint, GLchar const *)) \
F(GLint,GetAttribLocation,(GLuint, GLchar const *)) \
F(void,BindAttribLocation,(GLuint, GLuint, GLchar const *)) \
F(void,EnableVertexAttribArray,(GLuint)) \
F(void,DisableVertexAttribArray,(GLuint)) \
F(GLint,VertexAttribPointer,(GLuint, GLint, GLenum, GLboolean, GLsizei, GLvoid const *)) \
 \
/* Uniforms */ \
F(void,Uniform1f,(GLint, GLfloat)) \
F(void,Uniform3f,(GLint, GLfloat, GLfloat, GLfloat)) \
F(void,UniformMatrix4fv,(GLint, GLsizei, GLboolean, GLfloat const *)) \
 \
/* Vertex Array Objects */ \
F(void,GenVertexArrays,(GLsizei, GLuint *)) \
F(void,BindVertexArray,(GLuint)) \
F(void,DeleteVertexArrays,(GLsizei, GLuint const *)) \
F(GLboolean,IsVertexArray,(GLuint)) \
 \
/* Draw */ \
F(void,MultiDrawArrays,(GLenum, GLint const *, GLsizei const *, GLsizei)) \
F(void,MultiDrawElements,(GLenum, GLsizei const *, GLenum, void const *const *, GLsizei)) \
 \
/* Other */ \
F(GLubyte const *,GetStringi,(GLenum name, GLuint index)) \

