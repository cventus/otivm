
struct gl_state;
struct glx;
struct glx_drawable;
struct glx_config;

/* Initialize a new OpenGL context for glx. Configure the context according to
   `config` or use default configuration if it is NULL. Return NULL on
   failure. */
struct glx *glx_make(Display *display, struct glx_config const *config);

/* Destroy a glx context created with `gl_make_xcontext` and frees the memory
   that `glx` points to. */
void glx_free(struct glx *glx);

/* Get an XVisualInfo for creating new X windows. The pointer that is returned,
   unless NULL, should be passed to `XFree()`. */
XVisualInfo *glx_visual_info(struct glx *glx);

/* Get display connection that the context uses */
Display *glx_display(struct glx *);

/* Get the API function pointers */
struct gl_state *glx_state(struct glx *);

/* create a renderble framebuffer from a Pixmap */
struct glx_drawable *glx_make_drawable_pixmap(struct glx *, Pixmap);

/* create a renderble framebuffer from a Window */
struct glx_drawable *glx_make_drawable_window(struct glx *, Window);

/* Notify that the drawable will no longer be used */
void glx_destroy_drawable(struct glx *, struct glx_drawable *);

/* Swap buffers */
void glx_swap_buffers(struct glx *, struct glx_drawable *window);

/* Make the context current with a specific drawable */
int glx_make_current(struct glx *, struct glx_drawable *);

