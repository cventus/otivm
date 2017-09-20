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

/* The functions `memblk_init()` and `memblk_push()` and the macro
   `memblk_offset()` can be used to safely calculate the sizes and offsets into
   a single contiguous block of memory housing several objects. This can be
   used to allocate space for a dynamically sized data structure and all of its
   parts with a single allocation.
  
   Allocate an array of type `struct memblk` (on the stack) as large as the
   number of fields in the block and use first `memblk_init()` to setup the
   first field, and then `memblk_push()` for consecutive ones, which will read
   from earlier entries in the array to calculate a properly aligned offset.
   The functions check for overflows and returns non-zero on overflow.

   Example usage:

       struct memblk blk[3];
       struct person *p;
       char *str;
       double *vec;

       len = strlen(name);
       if (memblk_init(blk + 0, 1, sizeof(*p))) { overflow }
       if (memblk_push(blk + 1, len, sizeof(*str), alignof(*str))) { overflow }
       if (memblk_push(blk + 2, n, sizeof(*vec), alignof(*vec))) { overflow }

       p = malloc(blk[2].extent);
       if (!p) { handle(memory); }
       str = memblk_offset(p, blk[1]);
       vec = memblk_offset(p, blk[2]);
 */
struct memblk { size_t offset, extent; };
int memblk_init(struct memblk *, size_t nmemb, size_t size);
int memblk_push(struct memblk *, size_t nmemb, size_t size, size_t align);
#define memblk_offset(blk, field) ((void *)((char *)(blk) + (field).offset))
