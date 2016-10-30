
struct gl_state;
struct gl_core;
struct gl_dbgmsg;

int gl_init_state(struct gl_state *);
int gl_term_state(struct gl_state *);

void (*gl_get_proc(char const *))(void);

int gl_find_ext(char const *extensions, char const *target);
int gl_has_ext(struct gl_state *state, char const *target);

int gl_resolve_core(struct gl_state *, struct gl_core *core);
int gl_resolve_dbgmsg(struct gl_state *, struct gl_dbgmsg *dbgmsg);

struct gl_core const *gl_get_core(struct gl_state *state);
struct gl_dbgmsg const *gl_get_dbgmsg(struct gl_state *state);

int gl_is_current(struct gl_state *);

