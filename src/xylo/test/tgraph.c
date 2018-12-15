#include <stdio.h>
#include <string.h>

#include "ok/ok.h"
#include "base/mem.h"
#include "base/wbuf.h"
#include "base/mempool.h"
#include "adt/ilist.h"
#include "adt/itree.h"
#include "xylo/tgraph.h"

static void path_concat(void *dest, void const *child, void const *parent)
{
	(void)strcpy(dest, parent);
	(void)strcat(dest, "->");
	(void)strcat(dest, child);
}

static int parent(int i)
{
	if (i < 2) return -1;
	if (i < 6) return i & 0x1;
	if (i < 22) return ((i - 6) & 0x3) + 2;
	if (i < 1046) return ((i - 22) & 0xf) + 6;
	return -1; 
}

static int next_sibling(int i)
{
	if (i >= 2 && i < 4) return i + 2;
	if (i >= 6 && i < 18) return i + 4;
	if (i >= 22 && i < 1030) return i + 16;
	return -1; 
}

static struct xylo_tnode *get_parent(int i, struct xylo_tnode **nodes)
{
	return parent(i) < 0 ? NULL : nodes[parent(i)];
}

static void init_id(char *buf, int i)
{
	(void)sprintf(buf, "n%04d", i);
}

static char *make_path(char *buf, int i)
{
	char temp[32];
	init_id(buf, i);
	while (i = parent(i), i >= 0) {
		(void)strcpy(temp, buf);
		init_id(buf, i);
		(void)strcat(strcat(buf, "->"), temp);
	}
	return buf;
}

int test_world_transforms_update_from_the_root_to_leaves(void)
{
	struct xylo_tgraph *graph;
	struct xylo_tnode *nodes[2 + 4 + 16 + 1024];
	size_t i;
	char buf[32], path[32];
	char const *p, *q;

	graph = xylo_make_tgraph(sizeof buf);

	/* Add nodes and child nodes in a random order */
	for (i = 0; i < length_of(nodes); i++) {
		init_id(buf, i);
		nodes[i] = xylo_make_tnode(graph, get_parent(i, nodes), buf);
	}

	/* Update transformations */
	xylo_tgraph_transform(graph, path_concat);

	/* Each node has a unique transform, ensure that they have been
	   successfully applied and not corrupted in any way */
	for (i = 0; i < length_of(nodes); i++) {
		p = make_path(path, i);
		q = xylo_tnode_global(nodes[i]);
		if (strcmp(p, q)) {
			fail_test("Transform %s is not %s\n", q, p);
		}
	}

	xylo_free_tgraph(graph);
	return 0;
}

int test_check_transformation_locality_after_compaction(void)
{
	struct xylo_tgraph *graph;
	struct xylo_tnode *nodes[2 + 4 + 16 + 1024];
	size_t i;
	int j;
	char buf[32];
	char const *p, *q;

	graph = xylo_make_tgraph(sizeof buf);

	/* Add nodes and child nodes in a random order */
	for (i = 0; i < length_of(nodes); i++) {
		init_id(buf, i);
		nodes[i] = xylo_make_tnode(graph, get_parent(i, nodes), buf);
	}

	/* Move transforms around to place related ones close to one another */
	xylo_tgraph_compact(graph);

	/* After a compaction the transformation of two consecutive siblings
	   (in the order they were added to a tree) should be tightly packed
	   and next to one another */
	for (i = 0; i < length_of(nodes); i++) {
		j = next_sibling(i);
		if (j >= 0) {
			p = xylo_tnode_local(nodes[i]);
			q = xylo_tnode_local(nodes[j]);
			if (p + sizeof buf != q) {
				fail_test("nonlocal: %d and %d\n", i, j);
			}
			p = xylo_tnode_global(nodes[i]);
			q = xylo_tnode_global(nodes[j]);
			if (p + sizeof buf != q) {
				fail_test("nonlocal: %d and %d\n", i, j);
			}
		}
	}
	xylo_free_tgraph(graph);

	return 0;
}
