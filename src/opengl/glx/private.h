
struct gl_xstate *gl_make_xcontext_buf(
	struct gl_xstate *buf,
	Display *display,
	struct gl_xconfig const *config);

struct gl_xstate
{
	struct gl_state state;
	Display *display;
	GLXFBConfig fbconfig;
	GLXContext context;
	struct hmap drawables;
};

struct gl_xdrawable
{
	XID xid;
	GLXDrawable glxid;
	void (*destroy)(struct gl_xstate *, struct gl_xdrawable *);
};

