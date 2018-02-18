#include <stdint.h>
#include <stddef.h>

#include "include/triangulate.h"

#define TRIANGLE_SET_SIZE sizeof (struct triangle_set)
#define INDEX_SIZE sizeof (((struct triangle_set *)0)->indices[0])
#define MAX_TRIANGLES ((SIZE_MAX - TRIANGLE_SET_SIZE) / INDEX_SIZE)

size_t triangle_set_size(size_t triangle_count)
{
	if (MAX_TRIANGLES <= triangle_count) {
		return 0;
	} else {
		return TRIANGLE_SET_SIZE + triangle_count * INDEX_SIZE;
	}
}
