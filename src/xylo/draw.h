#include <stddef.h>

struct xylo;
struct xylo_view;
struct xylo_draw;
struct xylo_dlist;
struct xylo_dshape;
struct xylo_glshape;

void xylo_draw(
	struct xylo *xylo,
	struct xylo_view const *view,
	struct xylo_draw const *draw);

void xylo_init_dlist(struct xylo_dlist *list);

void xylo_term_dlist(struct xylo_dlist *list);

struct xylo_dlist *xylo_dlist_cast(struct xylo_draw *d);

int xylo_dlist_insert(
	struct xylo_dlist *list,
	struct xylo_draw *element,
	ptrdiff_t index);

int xylo_dlist_append(
	struct xylo_dlist *list,
	struct xylo_draw *element);

size_t xylo_dlist_length(struct xylo_dlist *list);

int xylo_dlist_remove(struct xylo_dlist *list, ptrdiff_t index);

ptrdiff_t xylo_dlist_indexof(struct xylo_dlist *list, struct xylo_draw *needle);

void xylo_init_dshape(
	struct xylo_dshape *shape,
	float const color[4],
	struct xylo_glshape const *glshape);

void xylo_init_dshape_id(
	struct xylo_dshape *shape,
	unsigned id,
	float const color[4],
	struct xylo_glshape const *glshape);

void xylo_term_dshape(struct xylo_dshape *shape);

struct xylo_dshape *xylo_dshape_cast(struct xylo_draw *d);
