static inline void *jmp_null(void *p, jmp_buf env)
{
	if (p == NULL) { longjmp(env, 1); }
	return p;
}

static inline void jmp_nz(int val, jmp_buf env)
{
	if (val != 0) { longjmp(env, val); }
}

void exit_null(void *p);
void exit_z(int val);
void exit_nz(int val);
