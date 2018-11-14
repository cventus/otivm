/* Gap buffer: wrapper around realloc and memmove

   When `lbegin` is non-NULL it points to a a block of memory and `rend`
   points one char past its end. The pointers `lend` and `rbegin` specify an
   area of available space in the middle: `gap` points at the first available
   location and `end` at the first occupied location or is the same as
   `bound`. Content is always added at the beginning of the gap and the whole
   buffer is dynamically resized if necessary. The gap can be moved using
   various functions. */
extern struct gbuf { void *begin[2], *end[2]; } const empty_gbuf;

/* Initialize a gap buffer to become similar to `empty_gbuf`. */
void gbuf_init(struct gbuf *buf);

/* Copy a gap buffer (including capacity and offset) */
int gbuf_init_copy(struct gbuf *dest, struct gbuf const *src);

/* Initialize a gap buffer with a pre-allocated memory block. If the block
   was returned by `malloc(3)` or its related functions, then the buffer
   effectively takes ownership of the block (and maybe reallocates it) if the
   typical `gbuf_alloc`, `gbuf_write`, `gbuf_trim`, `gbuf_term`, etc. functions
   are called on it. Alternatively, if only const functions or the static
   allocation functions `gbuf_salloc`, `gbuf_swrite`, or `gbuf_salign` (and
   others which do not re-allocate) are called on it, the buffer is at most
   written to, but is not freed or reallocated. In that case e.g. a statically
   allocated buffer can be used. */
void gbuf_init_buffer(struct gbuf *buf, void *buffer, size_t size);

/* Free buffer, if one has been allocated, and clear out the structure. The
   passed in `buf` itself is not freed. */
void gbuf_term(struct gbuf *buf);

/* Compare contents of a and b with `memcmp` without changing the offsets of
   either gap buffer. */
int gbuf_cmp(struct gbuf const *a, struct gbuf const *b);

/* Ensure that there is `size` available space in the gap for a subsequent
   allocation. Return zero on success, negative on error allocation error.
   This function can be used to give an empty buffer an initial size. */
int gbuf_reserve(struct gbuf *buf, size_t size);

/* Shrink the size of the gap to zero and re-allocate the buffer into a smaller
   one, potentially freeing up some memory. The gap becomes empty
   (`lend` == `rbegin`) but the offset of the gap into the buffer stays the
   same. Return non-zero on re-allocation failure. */
int gbuf_trim(struct gbuf *buf);

/* Move gap to the beginning of the buffer so that the content is joined into a
   single contiguous array starting from `rbegin`. Semantically the same as
   `gbuf_move_to(buf, 0)`. */
void gbuf_prepend(struct gbuf *buf);

/* Move gap to end of buffer so that the content is joined into a single
   contiguous array starting from `lbegin`. This function can be called before
   accessing the buffer as an array. Semantically the same as
   `gbuf_move_to(buf, gbuf_size(buf))`. */
void gbuf_append(struct gbuf *buf);

/* Return the amount of content in the buffer, i.e. the number of chars that
   have been allocated or written. */
size_t gbuf_size(struct gbuf const *buf);

/* Return the size of the gap, i.e. how much can be written before the
   buffer have to be expanded. */
size_t gbuf_available(struct gbuf const *buf);

/* Total size of the allocated gap buffer array, which `buf->lbegin` points
   to. */
size_t gbuf_capacity(struct gbuf const *buf);

/* Return non-zero if the gab buffer ranges can be treated as arrays containing
   elements of the given size and alignment. In other words, begin[0] to end[0]
   and begin[1] to end[1] are properly aligned arrays containing elements of
   the given size and the next allocation will be properly aligned. */
int gbuf_isuniform(struct gbuf const *buf, size_t size, size_t align);

/* Return the number of elements in `buf`, if its contents is interpreted as
   an array of objects of size `elem_size`. If `gbuf_size(buf)` isn't a
   multiple of `elem_size` the result is rounded down. */
size_t gbuf_nmemb(struct gbuf const *buf, size_t elem_size);

/* Get a pointer that points `offset` bytes into the buffer content (not
   including the gap). */
void *gbuf_get(struct gbuf *buf, size_t offset);

/* Get the current byte offset of the insertion point (the gap). */
size_t gbuf_offset(struct gbuf const *buf);

/* Move the point of insertion (the gap) so that it begins `offset` bytes from
   the start of the buffer. Return non-zero if `offset` is greater than
   `gbuf_size(buf)`. */
int gbuf_move_to(struct gbuf *buf, size_t offset);

/* Move the point of insertion (the gap) by `rel_offset` bytes from its current
   location. Return non-zero if the resulting offset is negative or greater
   than `gbuf_size(buf)`. */
int gbuf_move_by(struct gbuf *buf, ptrdiff_t rel_offset);

/* Insert padding at the beginning of the gap, if necessary, so that the next
   allocation has the specified alignment. Return zero on success, and non-zero
   on allocation failure. */
int gbuf_align(struct gbuf *buf, size_t align);

/* Insert padding at the beginning of the gap, if necessary, so that the next
   allocation has the specified alignment. Return zero on success, and non-zero
   if there's not enough space in the gap buffer. This function never allocates
   memory dynamically. */
int gbuf_salign(struct gbuf *buf, size_t align);

/* Reserve `size` bytes from the beginning of the gap and return a pointer to
   the newly allocated area, or NULL if memory allocation fails. */
void *gbuf_alloc(struct gbuf *buf, size_t size);

/* Reserve `size` bytes from the beginning of the gap and return a pointer to
   the newly allocated area, or NULL if there is not enough space in the gap
   buffer. This function never allocates memory dynamically. */
void *gbuf_salloc(struct gbuf *buf, size_t size);

/* Remove content from buffer by expanding the gap towards `lbegin` by `size`
   bytes. Return non-zero if `size` is greater than the size of the buffer. */
int gbuf_retract(struct gbuf *buf, size_t size);

/* Remove content from buffer by expanding the gap towards `rend` by `size`
   bytes. Return non-zero if `size` is greater than the size of the buffer. */
int gbuf_delete(struct gbuf *buf, size_t size);

/* Remove content from buffer range [offset, offset + size) by moving the gap,
   if necessary, and expanding the gap extents. Return non-zero if the range is
   outside of the buffer. */
int gbuf_erase(struct gbuf *buf, size_t offset, size_t size);

/* Reserve `size` bytes from the beginning of the gap and initialize it with a
   copy of `data`. Return a pointer to the newly allocated area, or NULL if
   memory allocation fails. */
void *gbuf_write(struct gbuf *buf, void const *data, size_t size);

/* Reserve `size` bytes from the beginning of the gap and initialize it with a
   copy of `data`. Return a pointer to the newly allocated area, or NULL if
   there isn't enough space in the gap. This function never allocates memory
   dynamically. */
void *gbuf_swrite(struct gbuf *buf, void const *data, size_t size);

/* Copy `gbuf_size(src)` bytes from the buffer to `dest`. Return `dest`. */
void *gbuf_copy(void *dest, struct gbuf const *src);

/* Copy `size` bytes starting from `offset` into the region `dest` points at.
   Return non-zero if `offset+size` is greater than `gbuf_size(buf)`. */
int gbuf_read(void *dest, struct gbuf *src, size_t offset, size_t size);
