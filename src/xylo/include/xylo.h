struct gl_api;
struct xylo;
struct xylo_view;
struct xylo_draw;

struct xylo *make_xylo(struct gl_api *);
void xylo_begin(struct xylo *);
void xylo_draw(
	struct xylo *,
	struct xylo_view const *,
	struct xylo_draw const *);
void xylo_end(struct xylo *);
void free_xylo(struct xylo *);
