/* Write buffer structure for appending data. */
struct wbuf { void *begin, *end, *bound; };


/* Initialize an empty write buffer. */
void wbuf_init(struct wbuf buf[static 1]);

/* Initialize a write buffer with a pre-allocated memory block. If the block
   was returned by `malloc(3)` or its related functions, then the buffer
   effectively takes ownership of the block (and maybe reallocates it) if the
   typical `wbuf_alloc`, `wbuf_write`, `wbuf_trim`, `term_wbuf`, etc. functions
   are called on it. Alternatively, if only const functions or the static
   allocation functions `wbuf_salloc`, `wbuf_swrite`, or `wbuf_salign` (and
   others which do not re-allocate) are called on it, the buffer is at most
   written to, but is not freed or reallocated. In that case pointers into the
   block remain valid and e.g. a statically allocated buffer can be used. */
void wbuf_init_buffer(struct wbuf buf[static 1], void *buffer, size_t size);

/* Free the underlying buffer and clear out the structure. The passed in `buf`
   itself is not freed. */
void wbuf_term(struct wbuf buf[static 1]);

/* Ensure that there is `size` free space, reallocating the buffer if
   necessary. Return zero on success, negative on error. This call might
   re-allocate the buffer, and any pointer into it is potentially invalidated.
   This function can be used to give an empty buffer an initial size. */
int wbuf_reserve(struct wbuf buf[static 1], size_t size);

/* Reallocate the underlying buffer so that there is no available space left.
   This might free up some memory. Return zero on success. */
int wbuf_trim(struct wbuf buf[static 1]);

/* Return the amount of used space in the buffer, i.e. the number of chars that
   have been allocated or written. This is the offset of the current end
   pointer. */
size_t wbuf_size(struct wbuf const buf[static 1]);

/* Return the amount of available space in the buffer, i.e. how much can be
   written until before the buffer have to be expanded. */
size_t wbuf_available(struct wbuf const buf[static 1]);

/* Total size of the allocated write buffer array, which `buf->begin` points
   to. */
size_t wbuf_capacity(struct wbuf const buf[static 1]);

/* Return the number of elements in `buf`, if its contents is interpreted as
   an array of objects of size `elem_size`. */
size_t wbuf_nmemb(struct wbuf const buf[static 1], size_t elem_size);

/* Get a pointer that points `offset` bytes into the buffer. */
void *wbuf_get(struct wbuf const buf[static 1], size_t offset);

/* Insert padding at the end of the buffer, if necessary, so that the next
   allocation has the specified alignment. Return zero on success, and non-zero
   on allocation failure. */
int wbuf_align(struct wbuf buf[static 1], size_t align);

/* Reserve `size` bytes at the end of the buffer, and return a pointer to the
   newly allocated area, or NULL if memory allocation fails. */
void *wbuf_alloc(struct wbuf buf[static 1], size_t size);

/* Reserve `size` bytes at the end of the buffer and initialize it with a copy
   of `data`. Return a pointer to the newly allocated area, or NULL if memory
   allocation fails. */
void *wbuf_write(struct wbuf buf[static 1], void const *data, size_t size);

/* Insert padding at the end of the buffer, if necessary, so that the next
   allocation has the specified alignment. Return zero on success, and non-zero
   if there's insufficient available space in the underlying buffer. This
   function never attempts to reallocate or free the underlying buffer. */
int wbuf_salign(struct wbuf buf[static 1], size_t align);

/* Reserve `size` bytes at the end of the buffer, and return a pointer to the
   newly allocated area, or NULL if there is insufficient available space in
   the underlying buffer. This function never attempts to reallocate or free
   the underlying buffer. */
void *wbuf_salloc(struct wbuf buf[static 1], size_t size);

/* Reserve `size` bytes at the end of the buffer and initialize it with a copy
   of `data`. Return a pointer to the newly allocated area, or NULL if there
   is insufficient available space in the underlying buffer. This function
   never attempts to reallocate or free the underlying buffer. */
void *wbuf_swrite(struct wbuf buf[static 1], void const *data, size_t size);

/* Remove `size` bytes of content from the end of the buffer. Return non-zero
   if `size` is greater than the size of the underlying buffer. This function
   never attempts to reallocate or free the underlying buffer. */
int wbuf_retract(struct wbuf buf[static 1], size_t size);

/* Copy the last `size` bytes from the end of the buffer into `data`, and then
   retract by `size` bytes. Returns non-zero if `size` is greater than the size
   of the buffer. This function never attempts to reallocate or free the
   underlying buffer.

   If the buffer contains a homogenous array where the order of elements
   doesn't matter then this function can be used to remove elements. For
   instance,

       wbuf_pop(items, wbuf_get(items, i*item_size), item_size);

   erases the item at index `i` by writing `item_size` bytes from the end of
   the buffer into its location, after which the array length decreases by one
   element. */
int wbuf_pop(struct wbuf buf[static 1], void *data, size_t size);

/* Concatenate the data from `src` to the end of `dest`. Return the address of
   the area in dest into which `src` was copied, or NULL if allocation fails. */
void *wbuf_concat(struct wbuf dest[static 1], struct wbuf const src[static 1]);

/* Copy `wbuf_size(src)` bytes from the buffer to `dest`. */
void *wbuf_copy(void *dest, struct wbuf const src[static 1]);
