#include <stddef.h>
#include <stdalign.h>
#include <stdint.h>
#include "include/mem.h"

int memblk_extent(
	size_t offset,
	size_t *extent,
	size_t nmemb,
	size_t size,
	size_t align)
{
	size_t off = align_to(offset, align);
	if ((SIZE_MAX - offset) / size < nmemb) {
		return -1;
	} else {
		*extent = off + size*nmemb;
		return 0;
	}
}
