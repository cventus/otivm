
#include <stddef.h>

/* Get the first node in the list, in which `node` is a member. */
void *list_head(void *node);

/* Get the last node in the list, in which `node` is a member. */
void *list_tail(void *node);

/* Return the next node in the list, or NULL if `node` is the tail. */
void *list_next(void *node);

/* Return the previous node in the list, or NULL if `node` is the head. */
void *list_prev(void *node);

/* Insert a new member of the given size into the list after `node`, or if
   `node` is NULL, create a new list. Return pointer to new node or NULL if
    allocation fails. */
void *list_append(void *node, size_t size);

/* Insert a new member of the given size into the list before `node`, or if
   `node` is NULL, create a new list. Return pointer to new node or NULL if
    allocation fails. */
void *list_prepend(void *node, size_t size);

/* Return the number of elements in the list, in which `node` is a member. 
   NULL is treated as an empty list, and has a length of 0. */
size_t list_length(void *node);

/* Remove and free `node` from its list. Return NULL if `node` is the only
   element in the list, otherwise the next node is returned unless
   `node` was the tail, in which case, the previous node is returned */
void *list_remove(void *node);

/* Free the entire list to which `node` belongs. Every reference to the list
   are invalidated. */
void list_free(void *node);

