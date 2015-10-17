
struct glxstate
{
	struct glstate state;
	Display *display;
	GLXFBConfig fbconfig;
	GLXContext context;
	struct glxdrawable *drawable;
	size_t ndrawables;
};

struct glxconfig
{
	_Bool
		debug: 1,
		forward_compatible: 1,
		window: 1,
		pixmap: 1,
		pbuffer: 1;

	int vmajor, vminor;
	int sample_buffers, samples;
};

