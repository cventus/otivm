/* intrusive tree */
struct itree
{
	struct ilist siblings, children, *parent;
};

/* initialize a tree node which can receive child nodes or be added as a child
   to been part of another tree and removed) to the end of the list of children
   of `tree` */
void itree_init(struct itree *node);

/* make `scion` (which has been previously initialized and has possibly been
   pruned from another tree) a child of `stock` and insert it before node
   `next` in the the list of children or append it if `p` is NULL */
void itree_graft(struct itree *stock, struct itree *scion, struct itree *next);

/* unlink `node` from the parent and siblings of a tree, but keep children
   intact as an independent tree (it is also possible to graft the node back
   some place later) and return what previously was the next sibling */
struct itree *itree_prune(struct itree *node);

/* return pointer to parent node or NULL (not same as itree.parent) */
struct itree *itree_parent(struct itree *node);

/* return pointer to first child or NULL */
struct itree *itree_first_child(struct itree *node);

/* return pointer to last child or NULL */
struct itree *itree_last_child(struct itree *node);

/* return pointer to next sibling or NULL */
struct itree *itree_next_sibling(struct itree *node);

/* return pointer to previous sibling or NULL */
struct itree *itree_prev_sibling(struct itree *node);

/* return path length to the root */
size_t itree_depth(struct itree const *node);

/* return number of child nodes of `node` */
size_t itree_child_count(struct itree const *node);
