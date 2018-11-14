
#include <X11/Xlib.h>
#include "base/mem.h"

#define E(x) { x, #x }
static struct {
	int type;
	char const *name;
} const event_names[] = {
	E(KeyPress),
	E(KeyRelease),
	E(ButtonPress),
	E(ButtonRelease),
	E(MotionNotify),
	E(EnterNotify),
	E(LeaveNotify),
	E(FocusIn),
	E(FocusOut),
	E(KeymapNotify),
	E(Expose),
	E(GraphicsExpose),
	E(NoExpose),
	E(VisibilityNotify),
	E(CreateNotify),
	E(DestroyNotify),
	E(UnmapNotify),
	E(MapNotify),
	E(MapRequest),
	E(ReparentNotify),
	E(ConfigureNotify),
	E(ConfigureRequest),
	E(GravityNotify),
	E(ResizeRequest),
	E(CirculateNotify),
	E(CirculateRequest),
	E(PropertyNotify),
	E(SelectionClear),
	E(SelectionRequest),
	E(SelectionNotify),
	E(ColormapNotify),
	E(ClientMessage),
	E(MappingNotify),
	E(GenericEvent)
};
#undef E

char const *xw_event_name(int type)
{
	size_t i;

	for (i = 0; i < length_of(event_names); i++) {
		if (event_names[i].type == type) {
			return event_names[i].name;
		}
	}
	return "UnknownEvent";
}

