
struct glstate;
struct glshader;
struct glprogram;

int gl_make_shader(
	struct glstate *state,
	struct glshader *shader,
	GLenum type,
	char const *source);
void gl_free_shader(struct glstate *state, struct glshader const *shader);
GLint gl_shader_type(struct glstate *state, struct glshader const *shader);
char *gl_get_shader_info_log(struct glstate *, struct glshader const *);

int gl_make_program(
	struct glstate *state,
	struct glprogram *program,
	struct glshader const *const *shaders,
	size_t nshaders);
void gl_free_program(struct glstate *state, struct glprogram const *program);
char *gl_get_program_info_log(struct glstate *, struct glprogram const *);

