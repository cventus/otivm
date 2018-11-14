struct xylo;

enum xylo_aa {
	/* no anti-aliasing - possibly aliased result */
	XYLO_AA_NONE,

	/* quincunx anti-aliasing - five-samples per pixel, four of which are
	   shared by every 2x2 pixels */
	XYLO_AA_QUINCUNX,

	/* rotated grid anti-aliased - four samples per pixel, avoiding sampling
	   with the same X or Y coordinate per pixel  */
	XYLO_AA_RGSS
};

enum xylo_aa xylo_get_aa(struct xylo *);
void xylo_set_aa(struct xylo *, enum xylo_aa aa);
