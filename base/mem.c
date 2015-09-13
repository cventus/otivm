
#include <stddef.h>
#include "include/mem.h"

size_t align_to(size_t offset, size_t align)
{
	size_t mis, pad;

	if (align == 0) return offset;

	mis = offset % align;
	if (mis == 0) return offset;

	pad = align - mis;
	return offset + pad;
}

