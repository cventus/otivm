/* Mark cell i as reachable and return the previous mark bits state in the
   least significant bits of the result. If a word is marked more than once
   then the second bit will be set (i.e. it is shared structure one way or
   another). */
static inline unsigned mark_bits(void *bitset, size_t i)
{
	unsigned char *p;
	size_t byte, bits;
	unsigned a, b;

	p = bitset;
	byte = i / (CHAR_BIT / 2);
	bits = i % (CHAR_BIT / 2);

	a = (p[byte] >> (bits*2)) & 0x3;
	b = (1 | (a << 1)) & 0x3;
	p[byte] |= b << (bits*2);

	return a;
}

/* Mark cell i as shared (both bits set) and return the previous mark bits
   state in the least significant bits of the result. */
static inline unsigned mark_shared_bits(void *bitset, size_t i)
{
	unsigned char *p;
	size_t byte, bits;
	unsigned a;

	p = bitset;
	byte = i / (CHAR_BIT / 2);
	bits = i % (CHAR_BIT / 2);
	a = (p[byte] >> (bits*2));
	p[byte] |= 0x3 << (bits*2);

	return a;
}

/* Clear the two mark bits of cell i. */
static inline void clear_bits(void *bitset, size_t i)
{
	unsigned char *p;
	size_t byte, bits;

	p = bitset;
	byte = i / (CHAR_BIT / 2);
	bits = i % (CHAR_BIT / 2);

	p[byte] &= ~(0x3 << (bits*2));
}

/* Return the two mark bits of cell i in the least significant bits of the
   result. */
static inline unsigned get_bits(void const *bitset, size_t i)
{
	unsigned char const *p;
	size_t byte, bits;

	p = bitset;
	byte = i / (CHAR_BIT / 2);
	bits = i % (CHAR_BIT / 2);

	return (p[byte] >> (2*bits)) & 0x3;
}

/* Get cell offset. */
static inline size_t ref_offset(union lxcell const *space, struct lxref ref)
{
	size_t i = ref.cell - space;
	/* don't count tag cells in bitmap offset */
	return i - (i + CELL_SPAN) / (CELL_SPAN + 1) + ref.offset;
}

struct lxlist lx_shared_head(
	struct lxlist list,
	union lxcell const *from,
	void const *bitset);

/* Recursively mark reachable nodes with two bits to find shared list
   structure. */
void lx_count_refs(
	union lxvalue root,
	union lxcell const *from,
	union lxcell *stack_max,
	void *bitset);
