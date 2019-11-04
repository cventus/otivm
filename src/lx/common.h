enum lx_tag
{
	lx_list_tag = 0,
	lx_tree_tag,
	lx_bool_tag,
	lx_int_tag,
	lx_float_tag,
	lx_string_tag,
};

enum lx_read_status {
	LX_READ_OK = 0,
	LX_READ_INCOMPLETE,
	LX_READ_UNEXPECTED,
	LX_READ_SHARP,
	LX_READ_STRING,
	LX_READ_NUMBER,
	LX_READ_ENTRY,
	LX_READ_HEAP_SIZE,
};
