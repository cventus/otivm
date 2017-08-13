/* fixed size memory pool from a user allocated block */
struct fixpool { void *buffer, **free; };
void fixpool_init(struct fixpool *, void *buffer, size_t nmemb, size_t size);
#define fixpool_aligned(size) \
((size) % alignof (void *) == 0 && (size) >= sizeof(void *))
void *fixpool_alloc(struct fixpool *);
int fixpool_is_empty(struct fixpool *);
void fixpool_free(struct fixpool *, void *p);
