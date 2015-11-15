
struct xw_delegate
{
	/* The window is created. This is the first callback called, which is
	   typically followed by a call to map. */
	void (*create)(Window window, void *context);

	/* The window is mapped. */
	void (*map)(void *context);

	/* The window is unmapped. */
	void (*unmap)(void *context);

	/* Called when the window manager (user) want to close the window. If
	   this function pointer is NULL, the default behavior is to just call
	   XDestroyWindow(3) on it directly. Use this callback to display a
	   confirmation dialog to the user before closing. */
	void (*close)(void *context);

	/* The user resized the window. */
	void (*resize)(int width, int height, void *context);

	/* The window has been removed and will never receive more events or
	   appear on the screen again. */
	void (*destroy)(void *context);

	/* The window become totally obscured, or was obscured and is now at
	   least partially visible. */
	void (*visibility)(int is_visible, void *context);

	/* The window receives or loses keyboard focus. */
	void (*focus)(int has_focus, void *context);

	/* Text input from the keyboard. The string `utf8` that is `size` bytes
	   long (not including null-terminator) contains the character(s) that
	   was input. It is freed after the callback returns. */
	void (*text)(char const *utf8, size_t size, void *context);

	void (*key_press)(
		KeySym keysym,
		int state,
		int is_repeat,
		void *context);

	void (*key_release)(KeySym keysym, int state, void *context);

	void (*button_press)(
		int button,
		int state,
		int x,
		int y,
		void *context);

	void (*button_release)(
		int button,
		int state,
		int x,
		int y,
		void *context);

	void (*pointer_motion)(int state, int x, int y, void *context);

	/* Pointer entered window */
	void (*pointer_enter)(int state, int x, int y, void *context);

	/* Pointer left window */
	void (*pointer_leave)(int state, int x, int y, void *context);
};

