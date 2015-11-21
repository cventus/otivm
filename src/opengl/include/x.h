
struct gl_xstate;
struct gl_xdrawable;
struct gl_xconfig;

/* Initialize a new OpenGL context for glx.  Store context information in
   `buf`, or into a fresh heap allocated object if it is NULL. Configure the
   context according to `config` or use default configuration if it is NULL.
   Return `buf` or a newly allocated object on success, NULL on failure. */
struct gl_xstate *gl_make_xcontext(
	struct gl_xstate *buf,
	Display *display,
	struct gl_xconfig const *config);

Display *gl_get_display(struct gl_xstate *glxstate);

/* Destroy a glx context created with `gl_make_xcontext`. Does not `free(3)`
   the memory that `state` points to. */
void gl_free_xcontext(struct gl_xstate *state);

/* Get XVisualInfo for creating new X windows. The pointer that is returned,
   unless NULL, should be passed to `XFree()`. */
XVisualInfo *gl_visual_info(struct gl_xstate *context);

struct gl_xdrawable *gl_add_xpixmap(struct gl_xstate *xstate, Pixmap pixmap);

struct gl_xdrawable *gl_add_xwindow(struct gl_xstate *xstate, Window window);

void gl_destroy_drawable(
	struct gl_xstate *xstate,
	struct gl_xdrawable *drawable);

int gl_make_current(struct gl_xstate *xstate, struct gl_xdrawable *drawable);

