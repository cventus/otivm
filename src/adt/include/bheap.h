/* re-organize existing `array` with `nmemb` elements of size `size` into a
   min-heap structure suitable for subsequent insertions or removals, so that
   the minimum element is stored in the first location of the array, where
   `compar` (similar to qsort(3) and bsearch(3)) returns negative, zero, or
   positive if an element in the array is respectively less than, equal, or
   greater than another  */
void init_bheap(
	void *array,
	size_t nmemb,
	size_t size,
	int (*compar)(void const *, void const *));

/* insert new element pointed to by `elem` into the heap, where `heap` points
   to an area with space for at least `nmemb+1` elements, were each element is
   of size `size`, and `compar` works as for `init_bheap` */
void *bheap_insert(
	void const *elem,
	void *heap,
	size_t nmemb,
	size_t size,
	int (*compar)(void const *, void const *));

/* remove the element in `heap` that `elem` points to (if `elem` equals `heap`
   then the minimum element is removed) */
void bheap_remove(
	void *elem,
	void *heap,
	size_t nmemb,
	size_t size,
	int (*compar)(void const *, void const *));
