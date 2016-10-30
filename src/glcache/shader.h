
struct gl_api;
struct gl_shader;
struct gl_program;

int gl_shader_init(
	struct gl_api *gl,
	struct gl_shader *shader,
	GLenum type,
	char const *source);

void gl_shader_term(struct gl_api *gl, struct gl_shader const *shader);
GLint gl_shader_type(struct gl_api *gl, struct gl_shader const *shader);
char *gl_get_shader_info_log(struct gl_api *, struct gl_shader const *);

int gl_program_init(
	struct gl_api *,
	struct gl_program *program,
	struct gl_shader const *const *shaders,
	size_t nshaders);
void gl_program_term(struct gl_api *, struct gl_program const *program);
char *gl_get_program_info_log(struct gl_api *, struct gl_program const *);

