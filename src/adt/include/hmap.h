
/* A key in the hash map is an arbitrary byte sequence. It is always stored
   in a location with maximum alignment requriements for a key with of the
   given size. */
struct hmap_key
{
	void const *key;
	size_t len;
};

/* Allocate a new hash map with values of size `size` and alignment
   `align`. */
struct hmap *hmap_make(size_t size, size_t align);

/* Free the hash table and all of its key-value mappings. Make sure that any
   resources the values refer to are free'd or accessible through other means
   before calling this function. */
void hmap_free(struct hmap *hm);

/* Initialize a hash map into location `buf` with values of size `size` and
   alignment `align`. */
void hmap_init(struct hmap *buf, size_t size, size_t align);

/* Free the key-value mappings of the hash map `buf` (but don't free it). Make
   sure that any resources the values refer to are free'd or accessible
   through other means before calling this function. */
void hmap_deinit(struct hmap *buf);

/* Return maximum size of table. */
size_t hmap_capacity(struct hmap *hm);

/* Return number of members in hash table. */
size_t hmap_nmemb(struct hmap *hm);

/* Load factor (nmemb/capacity) */
double hmap_load(struct hmap *hm);

/* Create a new key-value mapping unless one already exists. Returns NULL if
   key is already associated with a value, or if memory has run out and
   allocation fails. The function `hmap_get()` can be used to resolve this
   ambiguity if necessary. The returned pointer, unless NULL, points to a block
   of memory of the size and alignment that was provided to `hmap_make()`. */
void *hmap_new(struct hmap *hm, void const *key, size_t keylen);

/* Attempt to lookup the value for the key, but don't create a new mapping if
   one doesn't exist yet. This function performs no allocation and returns NULL
   if no mapping was found. The returned pointer, unless NULL, points to a
   block of memory of the size and alignment that was provided to
   `hmap_make()`.  */
void *hmap_get(struct hmap *hm, void const *key, size_t keylen);

/* Lookup the value for the key and return it, or create a new mapping if one
   doesn't exist yet. Return NULL if allocation was necessary and memory was
   exhausted. The returned pointer, unless NULL, points to a block of memory
   with the size and alignment that was provided to `hmap_make()`. */
void *hmap_put(struct hmap *hm, void const *key, size_t keylen);

/* Remove the specified key-value mapping and free the memory associated with
   it. Return non-zero if the key was not associated with a value. */
int hmap_remove(struct hmap *hm, void const *key, size_t keylen);

/* Get a pointer to an arbitrary key-value mapping stored in the hash map, that
   can be used in combination with `hmap_next()` to enumerate the keys and
   values in the hash map. Return NULL if the hash map is empty. The returned
   value, if not NULL, stays valid until a key is inserted or removed, or until
   the table has is freed.*/
struct hmap_bucket *hmap_first(struct hmap *hm);

/* Get a pointer to the next key-value mapping from `bucket`, which was
   returned previously by `hmap_first()` or `hmap_next()`. Return NULL if
   `bucket` was the last mapping in the data structure. The returned value, if
   not NULL, stays valid until a key is inserted or removed, or until table has
   is freed. */
struct hmap_bucket *hmap_next(struct hmap *hm, struct hmap_bucket *bucket);

/* Retrieve the key of an bucket. */
struct hmap_key hmap_key(struct hmap *hm, struct hmap_bucket *bucket);

/* Retrieve the value of an bucket. The returned pointer points to a block of
   memory of the size and alignment that was provided to `hmap_make()`. */
void *hmap_value(struct hmap *hm, struct hmap_bucket *bucket);

/* Interface for using strings as keys. `strkey` is interpreted as a
   nul-terminated string. The nul-terminator is part of the key. */
void *hmap_news(struct hmap *hm, char const *strkey);
void *hmap_gets(struct hmap *hm, char const *strkey);
void *hmap_puts(struct hmap *hm, char const *strkey);
int hmap_removes(struct hmap *hm, char const *strkey);

/* Interface for using integers as keys. */
void *hmap_newl(struct hmap *hm, long key);
void *hmap_getl(struct hmap *hm, long key);
void *hmap_putl(struct hmap *hm, long key);
int hmap_removel(struct hmap *hm, long key);

