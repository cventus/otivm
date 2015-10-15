
struct glxstate;
struct glxdrawable;
struct glxconfig;

/* Initialize a new OpenGL context for glx.  Store context information in
   `buf`, or into a fresh heap allocated object if it is NULL. Configure the
   context according to `config` or use default configuration if it is NULL.
   Return `buf` or a newly allocated object on success, NULL on failure. */
struct glxstate *gl_make_xcontext(
	struct glxstate *buf,
	Display *display,
	struct glxconfig const *config);

/* Destroy a glx context created with `gl_make_xcontext`. Does not `free(3)`
   the memory that `state` points to. */
void gl_free_xcontext(struct glxstate *state);

/* Get XVisualInfo for creating new X windows. The pointer that is returned,
   unless NULL, should be passed to `XFree()`. */
XVisualInfo *gl_visual_info(struct glxstate *context);

struct glxdrawable *gl_add_xpixmap(struct glxstate *xstate, Pixmap pixmap);
struct glxdrawable *gl_add_xwindow(struct glxstate *xstate, Window window);

int gl_make_current(struct glxstate *xstate, struct glxdrawable *drawable);

