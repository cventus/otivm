
#if 0

struct plain_prop_type
{
	char const *name;
	size_t size, align;
};

extern struct plain_prop_type const plain_prop_term;

struct plain_prop *plain_next_prop(struct plain_prop *prop)
{
	if (prop->type == &plain_prop_term) { return NULL; }
	struct plain_prop_type *type = prop->type;
	assert(type->align > 0);
	size_t start = align_to(sizeof(type), type->align);
	size_t end = start + type->size;
	size_t next = align_to(end, align_of(type));
	char  *p = prop;
	return p + next;
}

struct plain_prop
{
	struct plain_prop_type const *type
};

struct plain_prop_min_width
{
	float min_width;
};

struct plain_prop_min_height
{
	float min_height;
};

#define plain_def_prop_type(typename) \
	{ #typename, sizeof(typename), alignof(typename) }

struct plain_prop_type const
	plain_prop_term = { 0, 0, 0 },
	prop_prop_min_width = plain_def_prop_type(struct plain_prop_min_width),
	prop_prop_min_height = plain_def_prop_type(struct plain_prop_min_height),


struct plain_prop_type const prop_min_height = {
	"min_height",
	sizeof(struct plain_prop_min_height),
	alignof(struct plain_prop_min_height)
};

struct plain_node
{
	struct plain_prop *props;
	struct plain_node *children;
};

struct plain_sheet
{
	struct plain_renderer *renderer;
};

#endif

struct plain_box
{
	GLfloat pos[3];
	GLfloat size[2];
	GLfloat z[2];
	GLfloat u[2], v[2];
	GLuchar color[4];
};

