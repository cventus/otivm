
struct glxdrawable
{
	XID xid;
	GLXDrawable glxid;
	void (*destroy)(struct glxstate *, struct glxdrawable *);
};

