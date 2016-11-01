
#define gl_api_field_flags(field) _Bool field ## _init: 1, no_ ## field: 1;
#define gl_api_field_struct(field) struct gl_ ## field field;

struct gl_api
{
	GL_API(gl_api_field_flags)
	GL_API(gl_api_field_struct)
};

