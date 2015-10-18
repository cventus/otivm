
/* Create a new resource cache with a single resource constructor.

   The function `make` takes a key (e.g. a file name) of the provided size
   and creates an associated value to store in a record pointed to by `data`
   of size `data_size` and returns zero on success. The function `free` is
   passed a pointer to a data structure initialized by `make`. The pointers
   `key` and `data` are valid until `free` is called on them. Both the
   constructor `make` and the destructor `free` are passed `link` as their
   fourth argument, which can be used for an extended context.

   The lifetime of the data structure is managed by the resource cache,
   which allocates a block to store it and a copy of the key. For this
   purpose it needs to know the alignments of the data and key. */
struct rescache *make_rescache(
	size_t data_size,
	size_t data_align,
	size_t key_align,
	int (*make)(void const *key, size_t size, void *data, void *link),
	void (*free)(void const *key, size_t size, void *data, void *link),
	void *link);

/* Create a new resource cache with several resource constructors.

   The array of function pointers `make` of length `nmake` contain resource
   constructors, each of which take a key (e.g. a file name) of the provided
   size and creates an associated value to store in a record pointed to by
   `data` of size `data_size` and returns zero on success. The constructors
   are called in the order they appear in the array, until any constructor
   returns non-zero. The function `free` is passed a pointer to a data
   structure initialized by one of the constructors in `make`, which probably
   requires that they are of the same type. The pointers `key` and `data` are
   valid until `free` is called on them. Both the constructors in `make` and
   the destructor `free` are passed `link` as their fourth argument, which can
   be used for an extended context.

   The lifetime of the data structure is managed by the resource cache,
   which allocates a block to store it and a copy of the key. For this
   purpose it needs to know the alignments of the data and key. */
struct rescache *make_rescachen(
	size_t data_size,
	size_t data_align,
	size_t key_align,
	int (*const make[])(void const *key, size_t, void *data, void *link),
	size_t nmake,
	void (*free)(void const *key, size_t, void *data, void *link),
	void *link);

/* Attempt to free the resource cache and all the resources. This function
   should only be called when the resource cache is empty, or else there's
   likely a bug in the program, e.g. some resource is still referenced.
   Return zero on success, i.e. all resources could be freed. */
int free_rescache(struct rescache *r);

/* Return the number of resources in the cache, used or unreferenced. */
size_t rescache_size(struct rescache *r);

/* Return the number of resources in the cache that have a zero reference
   count. */
size_t rescache_unused(struct rescache *r);

/* Free unused resources in the cache, passing them to `free`. Return
   number of resources that were freed. */
size_t rescache_clean(struct rescache *r);

/* Load a resource identified by `key`, or return a previously loaded
   resource with the same key. The first `size` bytes starting at `key` are
   used to compare it with future calls to this function, so make sure the
   key is in a canonical form (strings possibly in lowercase, all internal
   padding zeroed out with memset, etc.). */
void *rescache_load(struct rescache *r, void const *key, size_t size);

/* Load a string resource with the key of size `strlen(key) + 1`. */
void *rescache_loads(struct rescache *r, char const *key);

/* Release a resource by decrementing its reference count. */
void rescache_release(struct rescache *r, void const *data);

/* Release a resource by decrementing its reference count and free it 
   immediately if it reached zero. Usually resources with a reference count
   of zero are kept until `rescache_clean()` is called. */
void rescache_release_and_free(struct rescache *r, void const *data);

