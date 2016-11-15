
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

/* The function `memblk_extent()` and its helper macros `memblk_field()`,
   `memblk_array()`, and `memblk_offset()` can be used to safely calculate
   the size and offsets into a single contiguous block of memory. This can be
   used to allocate space for a dynamically sized data structure and all of its
   parts with a single allocation. The function checks for overflows and
   returns non-zero on overflow.

   Pass the offset into the block, the one-past-the end result pointer, and the
   number, size and alignment of the data block.

   Example usage:

       size_t pext, sext, size, len;
       struct person *p;
       char *str;
       double *vec;

       len = strlen(name);
       if (memblk_field(0, &pext, struct person)) { handle(overflow); }
       if (memblk_array(pext, &sext, len, char)) { handle(overflow); }
       if (memblk_array(sext, &size, n, double)) { handle(overflow); }

       p = malloc(size);
       if (!p) { handle(memory); }
       str = memblk_offset(p, pext, char);
       vec = memblk_offset(p, sext, double);
 */

#define memblk_field(offset, extent, what) memblk_array(offset, extent, 1, what)
#define memblk_array(offset, extent, nmemb, what) \
	memblk_extent(offset, extent, nmemb, sizeof(what), alignof(what))

int memblk_extent(
	size_t offset,
	size_t *extent,
	size_t nmemb,
	size_t size,
	size_t align);

#define memblk_offset(block, offset, what) \
	((void *)((char *)(block) + align_to(offset, alignof(what))))
