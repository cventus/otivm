
struct gl_state;
struct gl_shader;
struct gl_program;

int gl_make_shader(
	struct gl_state *state,
	struct gl_shader *shader,
	GLenum type,
	char const *source);

void gl_free_shader(struct gl_state *state, struct gl_shader const *shader);
GLint gl_shader_type(struct gl_state *state, struct gl_shader const *shader);
char *gl_get_shader_info_log(struct gl_state *, struct gl_shader const *);

int gl_make_program(
	struct gl_state *state,
	struct gl_program *program,
	struct gl_shader const *const *shaders,
	size_t nshaders);
void gl_free_program(struct gl_state *state, struct gl_program const *program);
char *gl_get_program_info_log(struct gl_state *, struct gl_program const *);

