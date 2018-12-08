#include <stddef.h>
#include <stdalign.h>
#include <stdint.h>
#include "base/mem.h"

int memblk_init(struct memblk *field, size_t nmemb, size_t size)
{
	if (SIZE_MAX / size < nmemb) {
		return -1;
	} else {
		field->offset = 0;
		field->extent = size * nmemb;
		return 0;
	}
}

int memblk_push(
	struct memblk *field,
	size_t nmemb,
	size_t size,
	size_t align)
{
	size_t offset = align_to(field[-1].extent, align);
	if ((SIZE_MAX - offset) / size < nmemb) {
		return -1;
	} else {
		field->offset = offset;
		field->extent = offset + size * nmemb;
		return 0;
	}
}
