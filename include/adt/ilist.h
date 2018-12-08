/* intrusive linked list node - for use with container_of() */
struct ilist
{
	struct ilist *prev, *next;
};

/* lists can be linear or circular
    - the `prev` pointer of the first node, and the `next` pointer of the
      last node in a linear list are NULL, and typically you maintain a
      separate pointer to the first node in the list
    - in a circular list there is no inherent first and last node since every
      node is reachable from any other, but it is possible to treat one node
      as the `start` node which keeps pointers to the first and last elements
      in the list
 */

/* initialize a new linked list and make `list` the only element */
void llist_init(struct ilist *list);
void clist_init(struct ilist *list);

/* return non-zero if this is the only element in the list */
int llist_singleton(struct ilist const *list);
int clist_singleton(struct ilist const *list);

/* insert `node` (which isn't part of any other list and hasn't necessarily
   been initialized) to follow the node to which `list` points */
void llist_insert_next(struct ilist *list, struct ilist *node);
void clist_insert_next(struct ilist *list, struct ilist *node);

/* insert `node` (which isn't part of any other list and hasn't necessarily
   been initialized) to precede the node to which `list` points */
void llist_insert_prev(struct ilist *list, struct ilist *node);
void clist_insert_prev(struct ilist *list, struct ilist *node);

/* swap places of `a` and `b` (which do not have to be part of the same
   lists) */
void llist_swap(struct ilist *a, struct ilist *b);
void clist_swap(struct ilist *a, struct ilist *b);

/* remove `node` from the list it is part of by only unlinking it from its
   neighbours but do not touch `node` itself (i.e. its links remain the same
   and might be invalid and should be re-initialized) */
void llist_remove(struct ilist *node);
void clist_remove(struct ilist *node);

/* calculate the lenght of the linearly linked list starting from `node` (and
   following only `next` pointers) */
size_t llist_length(struct ilist *node);
size_t clist_length(struct ilist *list);
