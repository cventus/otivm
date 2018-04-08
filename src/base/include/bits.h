#define MSB (1 << (CHAR_BIT - 1))

static inline size_t bits_size(size_t n)
{
	return (n + CHAR_BIT - 1) / CHAR_BIT;
}

static inline void bits_set(void *bits, size_t i)
{
	((unsigned char *)bits)[i / CHAR_BIT] |= (MSB >> (i % CHAR_BIT));
}

static inline _Bool bits_get(void *bits, size_t i)
{
	return ((unsigned char *)bits)[i / CHAR_BIT] & (MSB >> (i % CHAR_BIT));
}
