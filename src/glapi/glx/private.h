
struct glx_context *glx_init_context(
	struct glx_context *,
	Display *,
	struct glx_config const *);

void glx_term_context(struct glx_context *);

struct glx_context
{
	struct gl_api api;
	Display *display;
	GLXFBConfig fbconfig;
	GLXContext context;
	struct hmap drawables;
};

struct glx_drawable
{
	XID xid;
	GLXDrawable glxid;
	void (*destroy)(struct glx_context *, struct glx_drawable *);
};

