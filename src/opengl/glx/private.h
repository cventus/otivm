
struct gl_xstate
{
	struct gl_state state;
	Display *display;
	GLXFBConfig fbconfig;
	GLXContext context;
	struct gl_xdrawable *drawable;
	size_t ndrawables;
};

struct gl_xdrawable
{
	XID xid;
	GLXDrawable glxid;
	void (*destroy)(struct gl_xstate *, struct gl_xdrawable *);
};

