
#define container_of(p,T,member) (T *)((char *)(p) - offsetof(T,member))
#define length_of(array) (sizeof (array) / sizeof 0[array])

static inline _Bool is_power_of_2(size_t val)
{
	return val > 0 && !(val & (val - 1));
}

/* Round offset up to the next multiple of `align` (which is a power of two),
   if offset isn't already a multiple. */
static inline size_t align_to(size_t offset, size_t align)
{
	return (offset + align - 1) & ~(align - 1);
}

