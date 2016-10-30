
#define STATE_FIELDS(F) \
	F(core) \
	F(dbgmsg) \

#define gl_state_field_flags(field) _Bool field ## _init: 1, no_ ## field: 1;
#define gl_state_field_struct(field) struct gl_ ## field field;

struct gl_state
{
	STATE_FIELDS(gl_state_field_flags)
	STATE_FIELDS(gl_state_field_struct)
};

