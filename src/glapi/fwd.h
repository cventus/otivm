struct gl_api;

int gl_init_api(struct gl_api *);
int gl_term_api(struct gl_api *);

int gl_is_current(struct gl_api *);
void (*gl_get_proc(struct gl_api *, char const *))(void);

int gl_find_ext(char const *extensions, char const *target);
int gl_has_ext(struct gl_api *, char const *target);
