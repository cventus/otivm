#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "ok/ok.h"
#include "base/mem.h"
#include "adt/ilist.h"
#include "adt/itree.h"

static char const *strcpbrk(char const *str, char const *reject)
{
	return str + strspn(str, reject);
}

static int validate_tree(struct itree *tree, char const *desc, va_list ap)
{
	static char const *const ws = " \t\n";

	char const *p;
	struct { int name; struct itree *node, *child; } stack[32], *tip, *s;
	/* use explicit stack of state that needs to be saved during traversal
	   over children (because ap is undefined if va_arg(ap) occurs in a
	   called function is undefined) (failed to make it worse) */
	tip = stack + length_of(stack);
	s = stack;
	s->node = tree;
	p = desc + strspn(desc, ws);
recurse:
	if (*p == '\0') { return -1; } 
	if (*p == ')') { return s == stack ? -1 : s[-1].name; } 
	if (*p == '(') {
		/* expanded node - check equality and that of children */
		p = strcpbrk(p + 1, ws);
		if (*p == '\0' || *p == '(' || *p == ')') { return -1; }
		s->name = *p;
		if (s->node != va_arg(ap, struct itree *)) { return s->name; }

		/* start iteration through children */
		s->child = itree_first_child(s->node);
next:		p = strcpbrk(p + 1, ws);
		if (s->child != NULL) {
			if (s == tip) { return -2; } /* stack overflow */
			s[1].node = s->child;
			s++;
			goto recurse;
		} else if (*p != ')') {
			/* check end of child list */
			return s->name;
		}
	} else {
		/* symbolic child node - only check equality */
		s->name = *p;
		if (s->node != va_arg(ap, struct itree *)) { return s->name; }
	}
	if (s == stack) {
		return 0; /* done */
	} else {
		/* pop stack and continue with next child */
		if (itree_parent(s->node) != s[-1].node) { return s->name; }
		s--;
		s->child = itree_next_sibling(s->child);
		goto next;
	}
}

static int check_shape(struct itree *tree, char const *desc, ...)
{
	va_list ap;
	int result;

	va_start(ap, desc);
	result = validate_tree(tree, desc, ap);
	va_end(ap);
	return result;
}

void assert_shape(struct itree *tree, char const *desc, ...)
{
	va_list ap;
	int result;

	va_start(ap, desc);
	result = validate_tree(tree, desc, ap);
	va_end(ap);
	if (result) {
		fail_test("mismatch of %s at %c\n", desc, result);
	}
}

int test_initialize_an_intrusive_tree_node(void)
{
	struct itree node;
	itree_init(&node);
	if (node.parent != NULL) { fail_test("parent is not null"); }
	if (!clist_singleton(&node.children)) {
		fail_test("children is not a singleton circular list!\n");
	}
	if (!llist_singleton(&node.siblings)) {
		fail_test("siblings is not a singleton linear list!\n");
	}
	return 0;
}

int test_get_parent_of_tree_node(void)
{
	struct itree root, child1, child2, grandchild;

	itree_init(&root);
	itree_init(&child1);
	itree_init(&child2);
	itree_init(&grandchild);

	itree_graft(&root, &child1, NULL);
	itree_graft(&root, &child2, NULL);
	itree_graft(&child1, &grandchild, NULL);

	if (itree_parent(&root) != NULL) {
		fail_test("root node has an invalid parent!\n");
	}
	if (itree_parent(&child1) != &root) {
		fail_test("child node 1 has an invalid parent!\n");
	}
	if (itree_parent(&child2) != &root) {
		fail_test("child node 2 has an invalid parent!\n");
	}
	if (itree_parent(&grandchild) != &child1) {
		fail_test("grand child node has an invalid parent!\n");
	}
	return 0;
}

int test_fetch_the_first_child_of_a_tree_node(void)
{
	struct itree root, child1, child2;

	itree_init(&root);
	itree_init(&child1);
	itree_init(&child2);
	if (itree_first_child(&root) != NULL) {
		fail_test("empty tree has a first child!\n");
	}
	itree_graft(&root, &child1, NULL);
	if (itree_first_child(&root) != &child1) {
		fail_test("no first child found!\n");
	}
	itree_graft(&root, &child2, NULL);
	if (itree_first_child(&root) != &child1) {
		fail_test("order of children changed!\n");
	}
	return 0;
}

int test_fetch_the_last_child_of_a_tree_node(void)
{
	struct itree root, child1, child2;

	itree_init(&root);
	itree_init(&child1);
	itree_init(&child2);
	if (itree_last_child(&root) != NULL) {
		fail_test("empty tree has a last child!\n");
	}
	itree_graft(&root, &child1, NULL);
	if (itree_last_child(&root) != &child1) {
		fail_test("no child last found!\n");
	}
	itree_graft(&root, &child2, NULL);
	if (itree_last_child(&root) != &child2) {
		fail_test("unexpected order of children!\n");
	}
	return 0;
}

int test_get_next_sibling(void)
{
	struct itree root, child1, child2;

	itree_init(&root);
	itree_init(&child1);
	itree_init(&child2);
	itree_graft(&root, &child1, NULL);
	itree_graft(&root, &child2, NULL);

	if (itree_next_sibling(&root) != NULL) {
		fail_test("root node has a next sibling");
	}
	if (itree_next_sibling(&child1) != &child2) {
		fail_test("next sibling of child1 is not child2\n");
	}
	if (itree_next_sibling(&child2) != NULL) { 
		fail_test("next sibling of child2 is not NULL\n");
	}
	return 0;
}

int test_get_previous_sibling(void)
{
	struct itree root, child1, child2;

	itree_init(&root);
	itree_init(&child1);
	itree_init(&child2);
	itree_graft(&root, &child1, NULL);
	itree_graft(&root, &child2, NULL);

	if (itree_prev_sibling(&root) != NULL) {
		fail_test("root node has a next sibling");
	}
	if (itree_prev_sibling(&child1) != NULL) {
		printf("prev sibling of child1 is not NULL\n");
		return -1;
	}
	if (itree_prev_sibling(&child2) != &child1) { 
		printf("prev sibling of child2 is not NULL\n");
		return -1;
	}
	return 0;
}

int test_node_depth(void)
{
	struct itree a, b, c;

	itree_init(&a);
	itree_init(&b);
	itree_init(&c);

	itree_graft(&a, &b, 0);
	itree_graft(&b, &c, 0);

	if (itree_depth(&a) != 0) { return -1; }
	if (itree_depth(&b) != 1) { return -1; }
	if (itree_depth(&c) != 2) { return -1; }

	return 0;
}

int test_number_of_children(void)
{
	struct itree a, b, c;

	itree_init(&a);
	itree_init(&b);
	itree_init(&c);

	if (itree_child_count(&a) != 0) { return -1; }

	itree_graft(&a, &b, 0);
	if (itree_child_count(&a) != 1) { return -1; }

	itree_graft(&a, &c, 0);
	if (itree_child_count(&a) != 2) { return -1; }

	return 0;
}

int test_test_function(void)
{
	struct itree n[4], *a, *b, *c, *d;

	itree_init(a = n + 0);
	itree_init(b = n + 1);
	itree_init(c = n + 2);
	itree_init(d = n + 3);
	itree_graft(a, b, NULL);
	itree_graft(a, c, NULL);
	itree_graft(c, d, NULL);
	
	/* syntax errors */
	if (check_shape(a, "  ", a) >= 0) { return -1; }
	if (check_shape(a, "(a", a) >= 0) { return -1; }
	if (check_shape(a, ")a", a) >= 0) { return -1; }
	if (check_shape(a, "((a))", a) >= 0) { return -1; }

	/* correct usages */
	if (check_shape(a, "a", a) != 0) { return -1; }
	if (check_shape(a, "(abc)", a, b, c) != 0) { return -1; }
	if (check_shape(a, " (  a   b    c )   ", a, b, c) != 0) { return -1; }
	if (check_shape(a, "(a(b)c)", a, b, c) != 0) { return -1; }
	if (check_shape(a, "(a(b)(cd))", a, b, c, d) != 0) { return -1; }

	/* shape mismatch */
	if (check_shape(a, "a", b) == 0) { return -1; }
	if (check_shape(a, "(abc)", a, c, b) == 0) { return -1; }
	if (check_shape(a, "(a(bc))", a, b, c) == 0) { return -1; }
	if (check_shape(a, "(a(b)(cd))", a, b, d, c) == 0) { return -1; }

	return 0;
}

int test_graft_and_prune_nodes(void)
{
	struct itree n[7], *a, *b, *c, *d, *e, *f, *g;

	itree_init(a = n + 0);
	itree_init(b = n + 1);
	itree_init(c = n + 2);
	itree_init(d = n + 3);
	itree_init(e = n + 4);
	itree_init(f = n + 5);
	itree_init(g = n + 6);

	itree_graft(a, b, NULL);
	itree_graft(a, c, NULL);
	itree_graft(b, d, NULL);
	itree_graft(b, e, NULL);
	itree_graft(c, f, NULL);
	itree_graft(c, g, NULL);

	assert_shape(a, "(a(bde)(cfg))", a, b, d, e, c, f, g);

	itree_prune(b);
	assert_shape(a, "(a(cfg))", a, c, f, g);
	assert_shape(b, "(bde)", b, d, e);

	itree_prune(c);
	assert_shape(a, "(a)", a);

	itree_graft(b, a, e);
	assert_shape(b, "(b(d)(a)(e))", b, d, a, e);

	itree_graft(f, b, NULL);
	assert_shape(c, "(c(fb)(g))", c, f, b, g);

	return 0;
}
