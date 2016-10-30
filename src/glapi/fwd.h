
struct gl_api;
struct gl_core;
struct gl_dbgmsg;

int gl_init_api(struct gl_api *);
int gl_term_api(struct gl_api *);

int gl_is_current(struct gl_api *);
void (*gl_get_proc(char const *))(void);

int gl_find_ext(char const *extensions, char const *target);
int gl_has_ext(struct gl_api *, char const *target);

int gl_resolve_core(struct gl_api *, struct gl_core *core);
int gl_resolve_dbgmsg(struct gl_api *, struct gl_dbgmsg *dbgmsg);

struct gl_core const *gl_get_core(struct gl_api *);
struct gl_dbgmsg const *gl_get_dbgmsg(struct gl_api *);

