/* dynamically growing (but not shrinking) memory pool of fixed size blocks */
struct mempool
{
	size_t nmemb, buffer_nmemb, size;
	struct wbuf buffers;
	void **free;
};

void mempool_init(struct mempool *pool, size_t buffer_nmemb, size_t size);
void mempool_term(struct mempool *pool);
size_t mempool_size(struct mempool *pool);
void *mempool_alloc(struct mempool *pool);
void mempool_free(struct mempool *pool, void *p);
