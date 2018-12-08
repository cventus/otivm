/* A xylo_tgraph is a collection of transformation nodes (xylo_tnode) which
   form tree-shaped structures. A xylo_tgraph pools alloctions of nodes and
   attempts to keep the transformations of sibling nodes close to each other in
   the hope of achieving meaningful cache locality. The format of how to
   represent transformations is up to the user of the code. They need to have a
   constant size and an operation for combining the relative transformation of
   a child with the world transform of its parent. */
struct xylo_tgraph;

/* A xylo_tnode is a transformation node which represent a local transformation
   which is relative to its parent. After `xylo_tgraph_transform()` has been
   called it's possible to retrive the global transformation which is in world
   space */
struct xylo_tnode;

/* initiate a xylo_tgraph */
void xylo_init_tgraph(struct xylo_tgraph *graph, size_t transform_size);

/* terminate a xylo_tgraph */
void xylo_term_tgraph(struct xylo_tgraph *graph);

/* allocate and initiate a xylo_tgraph */
struct xylo_tgraph *xylo_make_tgraph(size_t transform_size);

/* terminate and deallocate a xylo_tgraph */
void xylo_free_tgraph(struct xylo_tgraph *graph);

/* propagate transformations from the roots to the leaves */
int xylo_tgraph_transform(
	struct xylo_tgraph *graph,
	void transform(void *dest, void const *child, void const *parent));

/* move transformations to improve memory locality */
int xylo_tgraph_compact(struct xylo_tgraph *graph);

/* Allocate transformation node from `graph`, which is a child of `parent` (or
   NULL if it's a root node) and initialize its local transform with
   `transform`, the size of which was defined when `graph` was initiated. */
struct xylo_tnode *xylo_make_tnode(
	struct xylo_tgraph *graph,
	struct xylo_tnode *parent,
	void const *transform);

/* Return pointer to the local transform of a transformation node. The
   returned pointer can be invalidated if a new node is created, so use it only
   temporarily as a target location when updating the relative transform. */
void *xylo_tnode_local(struct xylo_tnode *node);

/* Return a pointer to the global, or world, transform of a transformation
   node. The global transformation is only updated after
   `xylo_tgraph_transform()` have been called. The returned pointer can be
   invalidated if a new node is created, so don't hold on to it for long. */
void const *xylo_tnode_global(struct xylo_tnode const *node);

/* Release `node`, and all of its children, back to `graph` */
void xylo_free_tnode(struct xylo_tgraph *graph, struct xylo_tnode *node);
