struct plain_transform {
	float pos[2], m22[4];
};

struct plain_state {
	struct plain_fsm const *fsm;
	usec entered;
	int transition;
};

enum plain_type
{
	plain_list,
	plain_shape,
	plain_text
};

struct plain_renderable
{
	enum plain_type type;
};

struct plain_list {
	enum plain_render type;
	size_t n;
};

struct plain_shape {
	enum plain_render type;
	struct plain_transform transform;
	struct xylo_glshape *shape;
};

struct plain_animated_shape {
	enum plain_render type;
	struct plain_transform transform;
	struct xylo_glshape *shape;
	struct plain_state state;
};

struct plain_text {
	enum plain_render type;
	struct plain_transform transform;
	struct xylo_glshape *shape;
	char const *utf8;
};

extern struct plain_render const *const plain_end;

enum plain_header *plain_make_list(struct plain *, ...);
