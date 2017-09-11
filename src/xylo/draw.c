#include <stddef.h>
#include <assert.h>
#include <base/mem.h>
#include <base/gbuf.h>
#include <stdio.h>
#include <gm/matrix.h>

#include "private.h"
#include "include/draw.h"

void xylo_draw(struct xylo *xylo, struct xylo_draw *draw)
{
	int i;
	struct xylo_dshape *shape;
	struct xylo_dlist *list;
	struct xylo_draw **p, **q;
	float to_world[16];

	assert(xylo != 0);
	assert(draw != 0);
	switch (draw->type) {
	case xylo_dshape:
		shape = xylo_dshape_cast(draw);
		(void)m44idf(to_world);
		to_world[0*4 + 0] = shape->m22[0];
		to_world[0*4 + 1] = shape->m22[1];
		to_world[1*4 + 0] = shape->m22[2];
		to_world[1*4 + 1] = shape->m22[3];
		to_world[3*4 + 0] = shape->pos[0];
		to_world[3*4 + 1] = shape->pos[1];

		xylo_set_to_world(xylo, to_world);
		xylo_draw_glshape(xylo, shape->glshape);
		break;
	
	case xylo_dlist:
		list = xylo_dlist_cast(draw);
		for (i = 0; i < 2; i++) {
			p = list->elements.begin[i];
			q = list->elements.end[i];
			for (; p < q; p++) {
				xylo_draw(xylo, *p);
			}
		}
		break;

	case xylo_dmax:
	default:
		assert(0);
		break;
	}
}

void xylo_init_dlist(struct xylo_dlist *list)
{
	list->draw.type = xylo_dlist;
	gbuf_init(&list->elements);
}

void xylo_term_dlist(struct xylo_dlist *list)
{
	gbuf_term(&list->elements);
}

struct xylo_dlist *xylo_dlist_cast(struct xylo_draw *d)
{
	return container_of(d, struct xylo_dlist, draw);
}

#define DLIST_ELEM_SIZE (sizeof(struct xylo_draw *))

int xylo_dlist_insert(
	struct xylo_dlist *list,
	struct xylo_draw *element,
	ptrdiff_t index)
{
	int ret;
	struct gbuf *elems;

	assert(list != NULL);
	assert(element != NULL);

	if (index < 0 || (size_t)index > xylo_dlist_length(list)) {
		return -1;
	}
	elems = &list->elements;
	ret = gbuf_move_to(elems, index * sizeof element);
	if (ret) { return -2; }
	ret = gbuf_write(elems, &element, sizeof element) ? 0 : -3;
	return ret;
}

int xylo_dlist_append(
	struct xylo_dlist *list,
	struct xylo_draw *element)
{
	return xylo_dlist_insert(list, element, xylo_dlist_length(list));
}

size_t xylo_dlist_length(struct xylo_dlist *list)
{
	assert(list != NULL);
	return gbuf_nmemb(&list->elements, DLIST_ELEM_SIZE);
}

int xylo_dlist_remove(struct xylo_dlist *list, ptrdiff_t index)
{
	if (index < 0 || (size_t)index > xylo_dlist_length(list)) {
		return -1;
	}
	gbuf_erase(&list->elements, index*DLIST_ELEM_SIZE, DLIST_ELEM_SIZE);
	return 0;
}

ptrdiff_t xylo_dlist_indexof(struct xylo_dlist *list, struct xylo_draw *needle)
{
	ptrdiff_t i, j;
	struct xylo_draw *p, *q;

	for (i = j = 0; i < 2; i++) {
		p = list->elements.begin[i];
		q = list->elements.end[i];
		for (; p < q; p++, j++) {
			if (p == needle) { return j; }
		}
	}
	return -1;
}

void xylo_init_dshape(
	struct xylo_dshape *shape,
	struct xylo_glshape const *glshape)
{
	shape->draw.type = xylo_dshape;
	shape->pos[0] = shape->pos[1] = 0.f;
	shape->m22[0] = shape->m22[3] = 1.f;
	shape->m22[1] = shape->m22[2] = 0.f;
	shape->glshape = glshape;
}

void xylo_term_dshape(struct xylo_dshape *shape)
{
	shape->glshape = NULL;
}

struct xylo_dshape *xylo_dshape_cast(struct xylo_draw *d)
{
	return container_of(d, struct xylo_dshape, draw);
}
