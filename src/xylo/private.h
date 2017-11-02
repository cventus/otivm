#define GLSL(version, src) ("#version " #version " core\n" #src)

enum
{
	FILL_COLOR_LOC = 0,
	FRAGMENT_ID_LOC = 1
};

enum
{
	ATTRIB_SHAPE_POS = 0,
	ATTRIB_QUADRATIC_POS = 1
};
