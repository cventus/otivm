
#define container_of(p,T,member) (T *)((char *)(p) - offsetof(T,member))
#define length_of(array) (sizeof (array) / sizeof 0[array])

size_t align_to(size_t offset, size_t align);

