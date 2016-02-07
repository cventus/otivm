
struct glx *glx_init(struct glx *, Display *, struct glx_config const *);
void glx_term(struct glx *glx);

struct glx
{
	struct gl_state state;
	Display *display;
	GLXFBConfig fbconfig;
	GLXContext context;
	struct hmap drawables;
};

struct glx_drawable
{
	XID xid;
	GLXDrawable glxid;
	void (*destroy)(struct glx *, struct glx_drawable *);
};

