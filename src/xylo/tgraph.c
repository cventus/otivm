#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include <base/mem.h>
#include <base/wbuf.h>
#include <base/mempool.h>
#include <adt/ilist.h>
#include <adt/itree.h>

#include "include/tgraph.h"

#define NODE_BLOCK_SIZE 100
#define TRANSFORM_SPACE_SIZE 200

struct xylo_tnode
{
	float *local, *global;
	struct itree tree;
};

struct xylo_tgraph
{
	struct itree root;
	struct mempool nodes;
	struct wbuf local_tfm, global_tfm;
	size_t transform_size;
};

void *xylo_tnode_local(struct xylo_tnode *node)
{
	assert(node != NULL);
	return node->local;
}

void const *xylo_tnode_global(struct xylo_tnode const *node)
{
	assert(node != NULL);
	return node->global;
}

void xylo_init_tgraph(struct xylo_tgraph *graph, size_t transform_size)
{
	assert(graph != NULL);
	graph->transform_size = transform_size;
	itree_init(&graph->root);
	mempool_init(&graph->nodes, NODE_BLOCK_SIZE, sizeof (struct xylo_tnode));
	wbuf_init(&graph->local_tfm);
	wbuf_init(&graph->global_tfm);
}

void xylo_term_tgraph(struct xylo_tgraph *graph)
{
	assert(graph != NULL);
	graph->transform_size = 0;
	itree_init(&graph->root);
	mempool_term(&graph->nodes);
	wbuf_term(&graph->local_tfm);
	wbuf_term(&graph->global_tfm);
}

struct xylo_tgraph *xylo_make_tgraph(size_t transform_size)
{
	struct xylo_tgraph *graph;
	graph = malloc(sizeof *graph);
	if (graph) { xylo_init_tgraph(graph, transform_size); }
	return graph;
}

void xylo_free_tgraph(struct xylo_tgraph *graph)
{
	assert(graph != NULL);
	xylo_term_tgraph(graph);
	free(graph);
}

static void xylo_init_tnode(
	struct xylo_tnode *node,
	struct itree *parent,
	float *local,
	float *global)
{
	itree_init(&node->tree);
	itree_graft(parent, &node->tree, NULL);
	node->local = local;
	node->global = global;
}

static int queue_push(struct wbuf queue[2], void const *data, size_t size)
{
	assert(wbuf_size(queue + 1) % size == 0);
	return wbuf_write(queue + 1, data, size) ? 0 : -1;
}

static int queue_pop(struct wbuf queue[2], void *data, size_t size)
{
	assert(wbuf_size(queue + 0) % size == 0);
	if (wbuf_size(queue + 0) == 0) {
		assert(wbuf_size(queue + 1) % size == 0);
		if (wbuf_reserve(queue + 0, wbuf_size(queue + 1))) {
			return -1;
		}
		while (wbuf_pop(queue + 1, queue[0].end, size) == 0) {
			(void)wbuf_alloc(queue + 0, size);
		}
	}
	return wbuf_pop(queue + 0, data, size);
}

static int enqueue_children(struct itree *node, struct wbuf queue[2])
{
	struct itree *p;
	struct xylo_tnode *q;
	for (p = itree_first_child(node); p; p = itree_next_sibling(p)) {
		q = container_of(p, struct xylo_tnode, tree);
		if (queue_push(queue, &q, sizeof q)) { return -1; }
	}
	return 0;
}

/* Compact the transformations */
static int graph_tfm_buffers(
	struct xylo_tgraph *graph,
	struct wbuf *local,
	struct wbuf *global,
	int write_global)
{
	struct wbuf queue[2];
	struct xylo_tnode *p;
	struct itree *t;
	int result;
	size_t size;

	assert(graph != NULL);

	size = graph->transform_size;
	assert(wbuf_capacity(local) / size >= graph->nodes.nmemb);
	assert(wbuf_capacity(global) / size >= graph->nodes.nmemb);

	/* traverse breadth-first with double-stack queue */
	wbuf_init(queue + 0);
	wbuf_init(queue + 1);
	result = 0;

	/* roots alias their local and global transforms */
	for (t = itree_first_child(&graph->root); t; t = itree_next_sibling(t)) {
		p = container_of(t, struct xylo_tnode, tree);
		if (enqueue_children(&p->tree, queue)) {
			result = -1;
			goto out;
		}
		p->local = p->global = wbuf_swrite(local, p->local, size);
		assert(p->local != NULL);
	}
	/* child nodes have separate global and local transforms */
	while (queue_pop(queue, &p, sizeof p) == 0) {
		if (enqueue_children(&p->tree, queue)) {
			result = -1;
			goto out;
		}
		p->local = wbuf_swrite(local, p->local, size);
		/* allow the global table to alias the previous local */
		if (write_global) {
			p->global = wbuf_swrite(global, p->global, size);
		} else {
			p->global = wbuf_salloc(global, size);
		}
		assert(p->local != NULL);
		assert(p->global != NULL);
	}
out:	wbuf_term(queue + 0);
	wbuf_term(queue + 1);
	return result;
}

static int resize_tfm_spaces(struct xylo_tgraph *graph, size_t newsize)
{
	struct wbuf local, global;
	size_t nmemb;
	int result;

	assert(graph != NULL);
	assert(wbuf_size(&graph->local_tfm) == wbuf_size(&graph->global_tfm));
	if (wbuf_capacity(&graph->global_tfm) == newsize) { return 0; }

	nmemb = newsize / graph->transform_size;
	if (graph->nodes.nmemb > nmemb) { return -1; }
	wbuf_init(&global);
	wbuf_init(&local);
	if ((result = wbuf_reserve(&global, newsize))) { goto fail; }
	if ((result = wbuf_reserve(&local, newsize))) { goto fail; }

	/* if this doesn't succeed we've corrupted the nodes */
	result = graph_tfm_buffers(graph, &local, &global, 1);
	if (result == 0) {
		wbuf_swap(&local, &graph->local_tfm);
		wbuf_swap(&global, &graph->global_tfm);
	}
fail:	wbuf_term(&local);
	wbuf_term(&global);
	return result;
}

/* garbage collect and compact the transformations - easy because
   transforms aren't shared */
int xylo_tgraph_compact(struct xylo_tgraph *graph)
{
	struct wbuf *local, *global;
	int result;

	local = &graph->local_tfm;
	global = &graph->global_tfm;
	wbuf_rewind(local);
	wbuf_rewind(global);

	/* copy local, but global becomes garbage */
	wbuf_swap(local, global);
	result = graph_tfm_buffers(graph, local, global, 0);
	return result;
}

struct xylo_tnode *xylo_make_tnode(
	struct xylo_tgraph *graph,
	struct xylo_tnode *parent,
	void const *tfm)
{
	struct xylo_tnode *node;
	float *local, *global;
	struct itree *parent_tree;
	size_t tfm_size, tfm_nmemb, node_nmemb, newsize;

	assert(graph != NULL);
	tfm_size = graph->transform_size;
	local = wbuf_salloc(&graph->local_tfm, tfm_size);
	if (!local) {
		/* no more transforms available: garbage collect or grow */
		tfm_nmemb = wbuf_nmemb(&graph->global_tfm, tfm_size);
		node_nmemb = graph->nodes.nmemb;
		if (node_nmemb + (node_nmemb >> 1) >= tfm_nmemb) {
			/* expand new local space */
			newsize = tfm_nmemb + (tfm_nmemb >> 1);
			if (newsize == 0) { newsize = TRANSFORM_SPACE_SIZE; }
			if (resize_tfm_spaces(graph, newsize * tfm_size)) {
				return NULL;
			}
		} else {
			if (xylo_tgraph_compact(graph)) { return NULL; }
		}
		local = wbuf_salloc(&graph->global_tfm, tfm_size);
		assert(local != NULL);
	}
	(void)memcpy(local, tfm, tfm_size);
	if (parent) {
		/* size of global_tfm = size of local_tfm */
		global = wbuf_salloc(&graph->global_tfm, tfm_size);
		assert(global != NULL);
	} else {
		global = local;
	}
	node = mempool_alloc(&graph->nodes);
	if (!node) {
		wbuf_retract(&graph->local_tfm, tfm_size);
		wbuf_retract(&graph->global_tfm, tfm_size);
		return NULL;
	}
	parent_tree = parent ? &parent->tree : &graph->root;
	xylo_init_tnode(node, parent_tree, local, global);
	return node;
}

void xylo_free_tnode(struct xylo_tgraph *graph, struct xylo_tnode *node)
{
	struct itree *p;
	struct xylo_tnode *child;

	while (p = itree_first_child(&node->tree), p) {
		child = container_of(p, struct xylo_tnode, tree);
		xylo_free_tnode(graph, child);
	}
	(void)itree_prune(&node->tree);
	mempool_free(&graph->nodes, node);
}

int xylo_tgraph_transform(
	struct xylo_tgraph *graph,
	void transform(void *dest, void const *child, void const *parent))
{
	struct wbuf queue[2];
	struct xylo_tnode *p, *child;
	struct itree *t;
	int result;

	/* traverse breadth-first with double-stack queue */
	wbuf_init(queue + 0);
	wbuf_init(queue + 1);
	result = 0;
	if (enqueue_children(&graph->root, queue)) {
		result = -1;
		goto out;
	}
	while (queue_pop(queue, &p, sizeof p) == 0) {
		t = itree_first_child(&p->tree);
		while (t) {
			child = container_of(t, struct xylo_tnode, tree);
			if (queue_push(queue, &child, sizeof child)) {
				result = -1; 
				goto out;
			}
			t = itree_next_sibling(t);
			transform(child->global, child->local, p->global);
		}
	}
out:	wbuf_term(queue + 0);
	wbuf_term(queue + 1);
	return result;
}
