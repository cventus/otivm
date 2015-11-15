
/* Write buffer structure for appending data. */
struct wbuf { void *begin, *end, *bound; };

/* Initialize an empty write buffer. */
void wbuf_init(struct wbuf buf[static 1]);

/* Ensure that there is `size` free space, reallocating the buffer if
   necessary. Return zero on success, negative on error. This call might
   re-allocate the buffer, and any pointer into is potentially invalidated.
   This function can be used to give an empty buffer an initial size. */
int wbuf_reserve(struct wbuf buf[static 1], size_t size);

/* Shrink `bound` to `end` which allows the operating system to potentially
   free up some memory. Return zero on success. */
int wbuf_trim(struct wbuf buf[static 1]);

/* Free buffer and clear out the structure. */
void wbuf_free(struct wbuf buf[static 1]);

/* Return the amount of used space in the buffer, i.e. the number of chars that
   have been allocated or written. This is the offset of the current end
   pointer. */
size_t wbuf_size(struct wbuf const buf[static 1]);

/* Return the amount of available space in the buffer, i.e. how much can be
   written until before the buffer have to be expanded. */
size_t wbuf_available(struct wbuf const buf[static 1]);

/* Total size of the write buffer array, pointed to by `buf->begin`. */
size_t wbuf_capacity(struct wbuf const buf[static 1]);

/* Return the number of elements in `buf`, if its contents is interpreted as
   an array of objects of size `elem_size`. */
size_t wbuf_nmemb(struct wbuf const buf[static 1], size_t elem_size);

/* Get a pointer that points `offset` bytes into the buffer. */
void *wbuf_get(struct wbuf buf[static 1], size_t offset);

/* Insert padding at the end of the buffer, if necessary, so that the next
   allocation has the specified alignment. Return zero on success. */
int wbuf_align(struct wbuf buf[static 1], size_t align);

/* Reserve `size` bytes at the end of the buffer, and return a pointer to the
   newly allocated area, or NULL if memory allocation fails. */
void *wbuf_alloc(struct wbuf buf[static 1], size_t size);

/* Reserve `size` bytes at the end of the buffer and initialize it with a copy
   of `data`. Return a pointer to the newly allocated area, or NULL if memory
   allocation fails. */
void *wbuf_write(struct wbuf buf[static 1], void const *data, size_t size);

/* Shrink buffer by `size` bytes from the end of the buffer. Returns non-zero
   if buffer reallocation failed. */
int wbuf_retract(struct wbuf buf[static 1], size_t size);

/* Copy the last `size` bytes from the end of the buffer into `data`, and then
   retract by `size` bytes. Returns non-zero if buffer reallocation failed or
   if `size` is greater than the size of the buffer.

   If `buf` is a homogenous, and the order of elements doesn't matter, then
   this function can be used to remove an element from the middle of the array
   by copying the last element somewhere into the middle and then retracting
   the size of the buffer. */
int wbuf_pop(struct wbuf buf[static 1], void *data, size_t size);

/* Concatenate the data from `src` to the end of `dest`. Return the address of
   the area in dest into which `src` was copied, or NULL if allocation fails. */
void *wbuf_concat(struct wbuf dest[static 1], struct wbuf const src[static 1]);

/* Copy `wbuf_size(src)` bytes from the buffer to `dest`. */
void *wbuf_copy(void *dest, struct wbuf const src[static 1]);

