
/* Create a new resource cache. The `make` function pointer array of length
   `nmake` contains functions that can load a resource based on `key` (which
   is typically a file name) and turn it into a more convenient internal
   format of size `data_size`. A function in `make` takes the key and stores
   the result in a record pointed to by `data` and returns zero on success. The
   function `free` frees objects of the internal format. Both the constructors
   and the destructor functions are passed `link` as their third argument, and
   can be used for context. */
struct rescache *make_rescache(
	size_t data_size,
	size_t data_align,
	int (*const make)(char const *key, void *data, void *link),
	void (*free)(char const *key, void *data, void *link),
	void *link);

struct rescache *make_rescachen(
	size_t data_size,
	size_t data_align,
	int (*const make[])(char const *key, void *data, void *link),
	size_t nmake,
	void (*free)(char const *key, void *data, void *link),
	void *link);

/* Free the resource cache and all the resources. Should only be called when
   the resource is empty, or else there's likely a bug in the program, e.g.
   some resource is still referenced. Return zero on success. */
int free_rescache(struct rescache *r);

/* Return the number of resources in the cache. */
size_t rescache_size(struct rescache *r);

/* Return the number of resources in the cache that have a zero reference
   count. */
size_t rescache_unused(struct rescache *r);

/* Free unused resources in the cache, passing them to `destruct`. Return
   number of resources that were freed. */
size_t rescache_clean(struct rescache *r);

/* Load a resource */
void *rescache_load(struct rescache *r, char const *key);

/* Increase reference count of resource */
void rescache_ref(struct rescache *r, void const *data);

/* Release a resource */
void rescache_release(struct rescache *r, void const *data);

