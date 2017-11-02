#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <base/mem.h>
#include <base/gbuf.h>
#include <gm/matrix.h>
#include <glapi/core.h>
#include <glapi/api.h>

#include "private.h"
#include "types.h"
#include "fb.h"
#include "xylo.h"
#include "glshape.h"
#include "shapes.h"
#include "quincunx.h"

#include "include/xylo.h"
#include "include/draw.h"
#include "include/types.h"


#define ALL_BUFFERS \
	(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

static void xylo_draw_rec(struct xylo *, float const *, struct xylo_draw *);

static void resize_fb(
	struct gl_core33 const *restrict gl,
	struct xylo_fb *fb,
	GLint size[2])
{
	if (fb->width < size[0] || fb->height < size[1]) {
		xylo_fb_resize(gl, fb, size[0], size[1]);
	}
}

void xylo_draw(
	struct xylo *xylo,
	struct xylo_view const *view,
	struct xylo_draw const *draw)
{
	struct gl_core33 const *restrict gl;
	float corner_projection[16], transform[16],  translate[16], scale[16];
	GLint viewport[4];
	GLint size[2], corner_size[2];

	gl = gl_get_core33(xylo->api);
	gl->GetIntegerv(GL_VIEWPORT, viewport);
	xylo_begin(xylo);

	size[0] = viewport[2];
	size[1] = viewport[3];
	corner_size[0] = size[0] + 1;
	corner_size[1] = size[1] + 1;

	gl->UseProgram(xylo->shapes.program);

	/* draw center samples */
	resize_fb(gl, &xylo->center_samples, size);
	gl->BindFramebuffer(GL_DRAW_FRAMEBUFFER, xylo->center_samples.fbo);
	gl->Viewport(0, 0, size[0], size[1]);
	gl->Clear(ALL_BUFFERS);
	xylo_draw_rec(xylo, view->projection, (struct xylo_draw *)draw);

	/* draw corner samples */
	resize_fb(gl, &xylo->corner_samples, corner_size);
	gl->BindFramebuffer(GL_DRAW_FRAMEBUFFER, xylo->corner_samples.fbo);
	gl->Viewport(0, 0, corner_size[0], corner_size[1]);
	gl->Clear(ALL_BUFFERS);
	/* move one pixel up+left in clip space */
	(void)m44scalef(
		scale,
		size[0]/(float)corner_size[0],
		size[1]/(float)corner_size[1],
		1.f);
	(void)m44translatef(
		translate,
		0.5f/corner_size[0],
		0.5f/corner_size[1],
		0.f);
	(void)m44mulf(transform, translate, scale);
	(void)m44mulf(corner_projection, transform, view->projection);
	xylo_draw_rec(xylo, corner_projection, (struct xylo_draw *)draw);

	/* draw samples to target */
	gl->BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	gl->Viewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	gl->UseProgram(xylo->quincunx.program);

	/* one pixel in texture space */
	xylo_quincunx_set_pixel_size(
		&xylo->quincunx,
		gl,
		1.0f/corner_size[0],
		1.0f/corner_size[1]);

	gl->ActiveTexture(GL_TEXTURE0);
	gl->BindTexture(GL_TEXTURE_2D, xylo->center_samples.color);
	xylo_quincunx_set_center_unit(&xylo->quincunx, gl, 0);

	gl->ActiveTexture(GL_TEXTURE1);
	gl->BindTexture(GL_TEXTURE_2D, xylo->corner_samples.color);
	xylo_quincunx_set_corner_unit(&xylo->quincunx, gl, 1);

	/* compose center and corner samples */
	xylo_quincunx_draw(&xylo->quincunx, gl);

	xylo_end(xylo);
}

static void draw_glshape(struct xylo *xylo, struct xylo_glshape const *shape)
{
	struct gl_core33 const *restrict gl = gl_get_core33(xylo->api);

	/* Step one - Fill in stencil buffer*/
	gl->ColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	gl->StencilFunc(GL_ALWAYS, 0x00, 0x01);
	gl->StencilOp(GL_KEEP, GL_KEEP, GL_INVERT);

	xylo_glshape_draw(gl, shape);

	/* Step two - fill in color without overdraw and erase stencil */
	gl->ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	gl->StencilFunc(GL_EQUAL, 0x01, 0x01);
	gl->StencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

	xylo_glshape_draw(gl, shape);
}

static void transform_to_modelview(float *dest, struct xylo_dshape *shape)
{
	(void)m44idf(dest);
	dest[0*4 + 0] = shape->m22[0];
	dest[0*4 + 1] = shape->m22[1];
	dest[1*4 + 0] = shape->m22[2];
	dest[1*4 + 1] = shape->m22[3];
	dest[3*4 + 0] = shape->pos[0];
	dest[3*4 + 1] = shape->pos[1];
}

static void xylo_draw_rec(
	struct xylo *xylo,
	float const *proj,
	struct xylo_draw *draw)
{
	struct gl_core33 const *restrict gl;
	struct xylo_dshape *shape;
	struct xylo_dlist *list;
	struct xylo_draw **p, **q;
	float mvp[16], mv[16];
	int i;

	assert(xylo != 0);
	assert(proj != 0);
	assert(draw != 0);
	switch (draw->type) {
	case xylo_dshape:
		shape = xylo_dshape_cast(draw);
		transform_to_modelview(mv, shape);
		gl = gl_get_core33(xylo->api);
		xylo_shapes_set_object_id(&xylo->shapes, gl, shape->id);
		xylo_shapes_set_mvp(&xylo->shapes, gl, m44mulf(mvp, proj, mv));
		xylo_shapes_set_color4fv(&xylo->shapes, gl, shape->color);
		draw_glshape(xylo, shape->glshape);
		break;

	case xylo_dlist:
		list = xylo_dlist_cast(draw);
		for (i = 0; i < 2; i++) {
			p = list->elements.begin[i];
			q = list->elements.end[i];
			for (; p < q; p++) {
				xylo_draw_rec(xylo, proj, *p);
			}
		}
		break;

	case xylo_dmax:
	default:
		assert("Unreachable" && 0);
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
	float const color[4],
	struct xylo_glshape const *glshape)
{
	shape->draw.type = xylo_dshape;
	shape->id = 0;
	shape->pos[0] = shape->pos[1] = 0.f;
	shape->m22[0] = shape->m22[3] = 1.f;
	shape->m22[1] = shape->m22[2] = 0.f;
	shape->glshape = glshape;
	(void)memcpy(shape->color, color, sizeof shape->color);
}

void xylo_init_dshape_id(
	struct xylo_dshape *shape,
	unsigned id,
	float const color[4],
	struct xylo_glshape const *glshape)
{
	shape->draw.type = xylo_dshape;
	shape->id = id;
	shape->pos[0] = shape->pos[1] = 0.f;
	shape->m22[0] = shape->m22[3] = 1.f;
	shape->m22[1] = shape->m22[2] = 0.f;
	shape->glshape = glshape;
	(void)memcpy(shape->color, color, sizeof shape->color);
}

void xylo_term_dshape(struct xylo_dshape *shape)
{
	shape->glshape = NULL;
}

struct xylo_dshape *xylo_dshape_cast(struct xylo_draw *d)
{
	return container_of(d, struct xylo_dshape, draw);
}
