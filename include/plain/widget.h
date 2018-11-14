
struct widget;
struct widget_class;
struct widget_buffer;

struct widget_buffer;

void *widget_buffer_alloc(struct widget_buffer *wb, size_t size, size_t align);

struct widget_class
{
	size_t align, (*size)(struct widget const *);
	int (*copy)(struct widget *dest, struct widget const *src);
	void (*create)(struct widget const *, struct widget_buffer *);
};

struct widgets *widgets(struct widget_buffer *buf, ...);
struct widgets *concat_widgets(struct widget_buffer *buf, ...);

struct widget *widgets_begin(struct widgets *ws);
struct widget *widgets_end(struct widgets *ws);
size_t widgets_length(struct widgets *ws);

struct widget
{
	struct widget_class const *wc;
	void *key;
	struct widgets *children;
};

struct button_properties
{
	char *label;
	int style;
};

struct button_state
{
	bool pressed: 1, hover: 1, focus: 1;
};

struct button_widget
{
	struct widget header;
	struct button_properties p;
	struct button_state state;
};

struct widget *plain_clickable(struct widget_buffer *buf, struct widget *child);
struct widget *plain_label(struct widget_buffer *buf, char *text);
struct widget *plain_border(struct widget_buffer *buf, struct widget *child);

struct widget *plain_button(struct widget_buffer *buf, char *label, int style)
{
	struct pbutton *button;
	size_t len;

	len = strlen(label);
	button = widget_buffer_alloc(buf, sizeof(*button), alignof(*button));
	button->p.label = strcpy(widget_buffer_alloc(buf, len + 1, 1), label);
	button->p.style = style;

	button->s.pressed = false;
	button->s.hover = false;
	button->s.focus = false;

	return (struct widget *)button;
}

struct widget *plain_button_realize(
	struct widget const *w,
	struct widget_buffer *buf)
{
	struct pbutton *button;
	char const *label;
	int style;
	
	button = (struct pbutton *)w;
	label = button->p.label;
	style = button->p.style;

	return
	plain_clickable(buf,
		button_on_click,
		button,
		plain_box(buf, style,
			plain_label(buf, label)));
}

void plain_button_realize(struct widget const *w, struct widget_buffer *buf)
{
	struct pbutton *button;
	char const *label;

	button = container_of(struct button_widget, header, w);
	label = button->p.label;

	(void)plain_box(buf,  plain_label(buf, label));
}

