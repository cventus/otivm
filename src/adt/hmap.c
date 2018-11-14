#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdalign.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include <assert.h>

#include "base/mem.h"
#include "adt/hmap.h"

#define MIN_CAP 16
#define MIN_OFFSET 4
#define MAX_OFFSET (UCHAR_MAX - 1)
#define INVALID_OFFSET UCHAR_MAX

/* A pair is a key-value pair, whose size depends on the type of values that
   are stored in the hash map. A copy of the key is stored right before this
   struct, follwed by the value. There are utiltiy functions to retrieve
   pointers to the key and value fields from the pointer to a struct member. */
typedef struct hmap_pair
{
	size_t keylen;
} pair;

/* The hash map table consists of an array of buckets. A bucket can have a
   pointer to a pair if it is occupied, or a pointer to NULL if it is vacant
   and a new member can be stored there. Buckets are also part of chains, which
   are lists of buckets, where each key hashed to the same index. The chain
   starts in the hashed to buckets, and the `first` field in this location is
   the offset to the first member in the chain. Consecutive members in the
   chain are found by following the `next`, and `prev` pointers in chain
   members. These are offsets (that wrap around the end of the table) from the
   chain bucket (the one that has a `first` offset), i.e. it is not relative to
   the current bucket. The `offset` field is the same value as the `next`
   bucket's `prev`, and vice versa, and simplifies the algorithms a bit.
   The `next` field is always greater than `offset`, and `prev` is always
   smaller than `offset`, except that they can also be INVALID_OFFSET, which
   signifies the end of a chain.

   This structure is used to make lookups fast and local (i.e. the chain
   buckets are probably close to each other in memory and are cache friendly)
   which is one of the goals of hopscotch hashing. */
typedef struct hmap_bucket
{
        unsigned char offset, first, next, prev;
        struct hmap_pair *pair;
} bucket;

typedef struct hmap_table
{
	size_t const max_offset, cap;
	struct hmap_bucket buckets[];
} table;

typedef struct hmap hmap;

/* Jenkins hash function */
static uint32_t jenkins(unsigned char const *key, size_t len)
{
	uint32_t hash, i;

	for(hash = i = 0; i < len; ++i) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

static size_t pair_offset(size_t keylen)
{
	return align_to(keylen, alignof(pair));
}

static size_t data_offset(hmap *hm)
{
	assert(hm != NULL);
	return align_to(sizeof (pair), hm->align);
}

static void *pair2data(hmap *hm, pair *p)
{
	assert(hm != NULL);
	assert(p != NULL);
	return (char *)p + data_offset(hm);
}

static void *pair2key(pair *p)
{
	assert(p != NULL);
	return (char *)p - pair_offset(p->keylen);
}

static pair *pair_make(hmap *hm, void const *key, size_t keylen)
{
	pair *p;
	char *q;
	size_t sz, poff, doff;

	/* first there's a key block, which consists of the key and padding */
	poff = pair_offset(keylen);
	if (poff < keylen) { goto fail; } else { sz = poff; }

	/* then there's the struct pair with possibly some padding up until
	   the data block */
	assert(sz % alignof(*p) == 0);
	doff = data_offset(hm);
	if (sz > SIZE_MAX - doff) { goto fail; } else { sz += doff; }

	/* finally there's the data block, which begins at a correctly aligned
	   offset */
	assert(sz % hm->align == 0);
	if (sz > SIZE_MAX - hm->size) { goto fail; } else { sz += hm->size; }

	q = malloc(sz);
	if (!q) { goto fail; }

	(void)memcpy(q, key, keylen);
	(void)memset(q + poff + doff, 0, hm->size);
	p = (pair *)(q + poff);
	p->keylen = keylen;
	return p;

fail:	return NULL;
}

static void pair_free(pair *p)
{
	assert(p != NULL);
	free(pair2key(p));
}

static size_t max_offset_for_cap(size_t cap)
{
	size_t max_offset;
	max_offset = cap / 10;
	if (max_offset < MIN_OFFSET) { max_offset = MIN_OFFSET; }
	if (max_offset > MAX_OFFSET) { max_offset = MAX_OFFSET; }
	return max_offset;
}

static bool key_equals(bucket *b, void const *key, size_t keylen)
{
	return b->pair->keylen == keylen &&
		memcmp(pair2key(b->pair), key, keylen) == 0;
}

static bucket *get_chain(table *t, void const *key, size_t keylen)
{
	return t->buckets + jenkins(key, keylen) % t->cap;
}

static size_t resolve_offset(size_t cap, size_t index, size_t offset)
{
	assert(index < cap);
	assert(offset < cap);
	if (cap - offset > index) {
		return index + offset;
	} else {
		return index + offset - cap;
	}
}

/* Get the head of the chain */ 
static bucket *bucket_to_chain(table *t, bucket *b)
{
	ptrdiff_t abs_offset;

	abs_offset = b - t->buckets;
	if (b->offset > abs_offset) {
		return (t->buckets + t->cap - b->offset) + abs_offset;
	} else {
		return t->buckets + abs_offset - b->offset;
	}
}

static bucket *resolve_bucket(table *t, bucket *chain, size_t offset)
{
	ptrdiff_t index;
	assert(t != NULL);
	assert(offset < t->cap);
	assert(chain != NULL);
	index = chain - t->buckets;
	assert(index >= 0);
	assert((size_t)index < t->cap);
	return t->buckets + resolve_offset(t->cap, index, offset);
}

/* Go through the chain of buckets and find the one that has the pair with the
   exact key. */
static bucket *chain_find_bucket(
	bucket *chain,
	table *t,
	void const *key,
	size_t keylen)
{
	bucket *p;
	size_t off;
	
	for (off = chain->first; off != INVALID_OFFSET; off = p->next) {
		p = resolve_bucket(t, chain, off);
		if (key_equals(p, key, keylen)) { return p; }
	}
	return NULL;
}

/* Get the (possibly wrapping) offset between two buckets in a table */
static size_t fwd_offset(table *t, bucket *chain, bucket *b)
{
	ptrdiff_t offset;

	assert(chain >= t->buckets);
	assert(chain < t->buckets + t->cap);
	assert(b >= t->buckets);
	assert(b < t->buckets + t->cap);
	
	offset = b - chain;
	return (offset < 0) ? t->cap + offset : (size_t)offset;
}

/* Unlink the bucket from its chain, and mark it as vacant */
static void clear_bucket(table *t, bucket *b) {
	bucket *chain, *q;

	chain = bucket_to_chain(t, b);
	if (chain->first == b->offset) {
		chain->first = b->next;
	} else {
		q = resolve_bucket(t, chain, b->prev);
		q->next = b->next;
	}
	if (b->next != INVALID_OFFSET) {
		q = resolve_bucket(t, chain, b->next);
		q->prev = b->prev;
	}
	b->pair = NULL;
	b->offset = INVALID_OFFSET;
	b->next = INVALID_OFFSET;
	b->prev = INVALID_OFFSET;
}

static void insert_bucket(table *t, bucket *chain, bucket *b, pair *p)
{
	bucket *q;
	size_t offset;

	offset = fwd_offset(t, chain, b);
	assert(offset < MAX_OFFSET);
	b->offset = offset;
	b->pair = p;
	if (chain->first == INVALID_OFFSET) {
		/* the first bucket in a chain (possibly chain == b) */
		chain->first = offset;
		b->next = INVALID_OFFSET;
		b->prev = INVALID_OFFSET;
	} else {
		/* insert into existing chain of chain */
		q = resolve_bucket(t, chain, chain->first);
		while (q->next != INVALID_OFFSET && q->next < offset) {
			q = resolve_bucket(t, chain, q->next);
		}
		b->next = q->next;
		b->prev = q->offset;
		q->next = offset;
		if (b->next != INVALID_OFFSET) {
			q = resolve_bucket(t, chain, b->next);
			q->prev = offset;
		}
	}
}

static int move_bucket(table *t, bucket *from, size_t abs_offset)
{
	bucket *chain, *to, *q;
	size_t new_offset;

	assert(t != NULL);
	assert(from != NULL);
	assert(from->pair != NULL);
	assert(abs_offset < t->cap);

	to = t->buckets + abs_offset;
	chain = bucket_to_chain(t, from);
	new_offset = fwd_offset(t, chain, to);
	if (new_offset >= MAX_OFFSET) { return -1; }
	to->offset = new_offset;
	to->pair = from->pair;
	clear_bucket(t, from);

	if (chain->first == INVALID_OFFSET) {
		/* single element chain */
		chain->first = to->offset;
		to->next = INVALID_OFFSET;
		to->prev = INVALID_OFFSET;
	} else if (to->offset < chain->first) {
		/* new first element */
		q = resolve_bucket(t, chain, chain->first);
		to->next = chain->first;
		to->prev = INVALID_OFFSET;
		q->prev = to->offset;
		chain->first = to->offset;
	} else {
		/* find location in chain */
		q = resolve_bucket(t, chain, chain->first);
		while (q->next != INVALID_OFFSET && q->next < to->offset) {
			q = resolve_bucket(t, chain, q->next);
		}
		to->next = q->next;
		to->prev = q->offset;
		q->next = to->offset;
		if (to->next != INVALID_OFFSET) {
			q = resolve_bucket(t, chain, to->next);
			q->prev = to->offset;
		}
	}
	return 0;
}

static int insert_pair(table *t, bucket *chain, pair *p)
{
	bucket *b, *end;
	size_t i, j, abs_offset;
	ptrdiff_t offset;

	offset = chain - t->buckets;
	assert(offset >= 0);
	assert((size_t)offset < t->cap);
	end = t->buckets + t->cap;
	/* linear probing to find a free slot */
	for (i = 0, b = chain; i < t->cap; i++) {
		if (b->pair == NULL) { break; }
		if (++b == end) { b = t->buckets; }
	}
	/* try to move the free slot closer to the hashed index */
	while (i >= t->max_offset) {
		for (j = i - 1, b = resolve_bucket(t, chain, j); ; j--) {
			abs_offset = offset + i;
			if (abs_offset >= t->cap) {
				abs_offset -= t->cap;
			}
			if (move_bucket(t, b, abs_offset) == 0) {
				i = j;
				break;
			}
			if (j == 0) {
				/* Couldn't find an bucket to move to the free
				   slot -- failure. */
				return -1;
			}
			if (b == t->buckets) { b = end - 1; } else { b--; }
		}
	}
	insert_bucket(t, chain, resolve_bucket(t, chain, i), p);
	return 0;
}

static table *table_make(size_t cap)
{
	table *t;
	bucket *buckets;
	size_t buckets_size, i;

	assert(cap > 0);

	if (SIZE_MAX / sizeof *buckets < cap) { return NULL; }
	buckets_size = sizeof *buckets * cap;
	if (SIZE_MAX - sizeof *t < buckets_size) { return NULL; }
	t = malloc(sizeof *t + buckets_size);
	if (t) {
		*(size_t *)&t->cap = cap;
		*(size_t *)&t->max_offset = max_offset_for_cap(cap);
		for (i = 0; i < cap; i++) {
			t->buckets[i].offset = INVALID_OFFSET;
			t->buckets[i].first = INVALID_OFFSET;
			t->buckets[i].next = INVALID_OFFSET;
			t->buckets[i].prev = INVALID_OFFSET;
			t->buckets[i].pair = NULL;
		}
	}
	return t;
}

static size_t grow_cap(size_t cap)
{
	return cap + (cap >> 1);
}

static int table_copy(table *dest, table const *src)
{
	bucket const *b, *end;
	bucket *chain;

	assert(dest != NULL);
	assert(src != NULL);

	for (b = src->buckets, end = src->buckets + src->cap; b < end; b++) {
		if (!b->pair) { continue; }
		chain = get_chain(dest, pair2key(b->pair), b->pair->keylen);
		if (insert_pair(dest, chain, b->pair) != 0) {
			return -1;
		}
	}
	return 0;
}

static void table_free(table *t, bool free_contents)
{
	size_t i;
	if (free_contents) {
		for (i = 0; i < t->cap; i++) {
			bucket *b = t->buckets + i;
			if (b->pair) {
				pair_free(b->pair);
			}
		}
	}
	free(t);
}

static void *hmap_new_table(hmap *hm, void const *key, size_t keylen)
{
	table *t;
	bucket *chain;
	pair *p;

	assert(hm->table == NULL);
	if (t = table_make(MIN_CAP), !t) { return NULL; }
	if (p = pair_make(hm, key, keylen), p == NULL) {
		table_free(t, false);
		return NULL;
	}
	chain = get_chain(t, key, keylen);
	assert(chain != NULL);
	chain->pair = p;
	chain->first = 0;
	chain->offset = 0;
	hm->table = t;
	hm->nmemb = 1;
	return pair2data(hm, p);
}

static int hmap_grow_table(hmap *hm)
{
	size_t cap;
	table *t;

	assert(hm->table != NULL);
	for (cap = grow_cap(hm->table->cap); ; cap = grow_cap(cap)) {
		t = table_make(cap);
		if (t == NULL) { return -1; }
		if (table_copy(t, hm->table) == 0) {
			table_free(hm->table, false);
			hm->table = t;
			return 0;
		} else {
			table_free(t, false);
		}
	}
}

void *hmap_new(hmap *hm, void const *key, size_t keylen)
{
	bucket *chain;
	pair *p;

	if (key == NULL || hm == NULL) { return NULL; }
	if (hm->table == NULL) { return hmap_new_table(hm, key, keylen); }
	chain = get_chain(hm->table, key, keylen);
	if (chain_find_bucket(chain, hm->table, key, keylen)) {
		/* Already occupied */
		return NULL;
	}
	if (p = pair_make(hm, key, keylen), p == NULL) { return NULL; }
	if (hm->nmemb == hm->table->cap) {
		if (hmap_grow_table(hm) == 0) {
			chain = get_chain(hm->table, key, keylen);
		} else {
			pair_free(p);
			return NULL;
		}
	}
	while (insert_pair(hm->table, chain, p) != 0) {
		if (hmap_grow_table(hm) != 0) {
			pair_free(p);
			return NULL;
		}
		/* find starting chain in new table */
		chain = get_chain(hm->table, key, keylen);
	}
	hm->nmemb++;
	return pair2data(hm, p);
}

void *hmap_get(hmap *hm, void const *key, size_t keylen)
{
	bucket *chain, *b;

	if (key == NULL || hm == NULL || hm->table == NULL) { return NULL; }
	chain = get_chain(hm->table, key, keylen);
	b = chain_find_bucket(chain, hm->table, key, keylen);
	return b ? pair2data(hm, b->pair) : NULL;
}

void *hmap_put(hmap *hm, void const *key, size_t keylen)
{
	bucket *chain, *b;
	pair *p;

	if (key == NULL || hm == NULL) { return NULL; }
	if (hm->table == NULL) { return hmap_new_table(hm, key, keylen); }
	chain = get_chain(hm->table, key, keylen);
	b = chain_find_bucket(chain, hm->table, key, keylen);
	if (b) {
		p = b->pair;
	} else {
		if (p = pair_make(hm, key, keylen), p == NULL) { return NULL; }
		if (hm->nmemb == hm->table->cap) {
			if (hmap_grow_table(hm) == 0) {
				chain = get_chain(hm->table, key, keylen);
			} else {
				pair_free(p);
				return NULL;
			}
		}
		while (insert_pair(hm->table, chain, p) != 0) {
			if (hmap_grow_table(hm) != 0) {
				pair_free(p);
				return NULL;
			}
			chain = get_chain(hm->table, key, keylen);
		}
		hm->nmemb++;
	}
	return pair2data(hm, p);
}

int hmap_remove(hmap *hm, void const *key, size_t keylen)
{
	bucket *chain, *b;

	if (key == NULL || hm == NULL || hm->table == NULL) { return -1; }
	chain = get_chain(hm->table, key, keylen);
	b = chain_find_bucket(chain, hm->table, key, keylen);
	if (!b) { return -1; }
	pair_free(b->pair);
	clear_bucket(hm->table, b);
	hm->nmemb--;
	return 0;
}

void hmap_init(struct hmap *hm, size_t size, size_t align)
{
	if (size == 0) {
		/* For zero sized members, the alignment doesn't matter. Make
		   it the smallest power of two so that the padding in pairs is
		   as small as possible. */
		align = 1;
	} else {
		assert(is_power_of_2(align));
	}

	if (!hm) { return; }

	hm->size = size;
	hm->align = align;
	hm->nmemb = 0;
	hm->table = NULL;
}

void hmap_term(struct hmap *hm)
{
	if (!hm) { return; }

	if (hm->table) { table_free(hm->table, true); }

	hm->nmemb = 0;
	hm->table = NULL;
}

hmap *hmap_make(size_t size, size_t align)
{
	hmap *hm;

	if (hm = malloc(sizeof *hm), !hm) { return NULL; }
	hmap_init(hm, size, align);

	return hm;
}

void hmap_free(hmap *hm)
{
	hmap_term(hm);
	free(hm);
}

size_t hmap_capacity(hmap *hm)
{
	return hm && hm->table ? hm->table->cap : 0;
}

size_t hmap_nmemb(hmap *hm)
{
	return hm ? hm->nmemb : 0;
}

double hmap_load(hmap *hm)
{
	return hmap_capacity(hm) ? hm->nmemb / (double)hmap_capacity(hm) : 0.0;
}

static bucket *linear_find(hmap *hm, bucket *b)
{
	bucket *p, *end;

	end = hm->table->buckets + hm->table->cap;
	if (b >= hm->table->buckets && b < end) {
		for (p = b; p < end; p++) {
			if (p->pair) {
				return p;
			}
		}
	}
	return NULL;
}

bucket *hmap_first(hmap *hm)
{
	return hm && hm->table ? linear_find(hm, hm->table->buckets) : NULL;
}

bucket *hmap_next(hmap *hm, bucket *b)
{
	return hm && hm->table && b ? linear_find(hm, b + 1) : NULL;
}

struct hmap_key hmap_key(hmap *hm, bucket *b)
{
	return (struct hmap_key){
		.key = (hm && b) ? pair2key(b->pair) : NULL,
		.size = (hm && b) ? b->pair->keylen : 0
	};
}

void *hmap_value(hmap *hm, bucket *b)
{
	return hm && b ? pair2data(hm, b->pair) : NULL;
}

void *hmap_news(hmap *hm, char const *strkey)
{
	return hmap_new(hm, strkey, strlen(strkey) + 1);
}

void *hmap_gets(hmap *hm, char const *strkey)
{
	return hmap_get(hm, strkey, strlen(strkey) + 1);
}

void *hmap_puts(hmap *hm, char const *strkey)
{
	return hmap_put(hm, strkey, strlen(strkey) + 1);
}

int hmap_removes(hmap *hm, char const *strkey)
{
	return hmap_remove(hm, strkey, strlen(strkey) + 1);
}

void *hmap_newl(hmap *hm, long longkey)
{
	return hmap_new(hm, &longkey, sizeof longkey);
}

void *hmap_getl(hmap *hm, long longkey)
{
	return hmap_get(hm, &longkey, sizeof longkey);
}

void *hmap_putl(hmap *hm, long longkey)
{
	return hmap_put(hm, &longkey, sizeof longkey);
}

int hmap_removel(hmap *hm, long longkey)
{
	return hmap_remove(hm, &longkey, sizeof longkey);
}

