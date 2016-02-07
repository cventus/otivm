
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdalign.h>
#include <setjmp.h>
#include "include/wf.h"

#include <text/token.h>
#include <text/re.h>
#include <base/wbuf.h>
#include <base/list.h>
#include <base/mem.h>

#include <gm/vector.h>
#include <gm/plane.h>

#include "private.h"

enum token {
	UNKNOWN,
	END_OF_FILE,
	NEWLINE,
	COMMENT,
	GEO_VERTEX,
	TEX_VERTEX,
	NORMAL,
	FACE,
	LOAD_MATERIAL,
	USE_MATERIAL
};

enum vertex_components
{
	POS = 0, UV, NORM, N_vertex_components
};

struct tgroup
{
	struct wbuf vertices;
	char const *mtlname;
};

struct obj_buffer
{
	unsigned count[N_vertex_components];
	size_t nmtllib;
	struct wbuf pos, norm, uv, mtllib;
	struct tgroup *groups;
};

static enum token classify(char const *kw)
{
	struct keyword const keywords[] = {
		{ "\n", NEWLINE },
		{ "#", COMMENT },
		{ "v", GEO_VERTEX },
		{ "vt", TEX_VERTEX },
		{ "vn", NORMAL },
		{ "f", FACE },
		{ "mtllib", LOAD_MATERIAL },
		{ "usemtl", USE_MATERIAL }
	};

	return wf_parse_keyword(kw, keywords, length_of(keywords), UNKNOWN);
}

static enum token next_keyword(FILE *fp)
{
	char directive[10];
	
	if (wf_next_token(directive, sizeof directive, fp) == 0) {
		return END_OF_FILE;
	} else {
		return classify(directive);
	}
} 

static int set_index(long idx, size_t max, unsigned int *p)
{
	if (idx == 0) { return -1; }
	if (idx < 0) {
		size_t diff = -(idx + 1);
		if (diff > max) { return -1; }
		*p = max - diff;
	} else {
		if ((size_t)idx > max) { return -1; }
		*p = idx;
	}
	return 0;
}

#define VERTEX_RE "^-?\\d+(?:/(-?\\d+)(?:/(-?\\d+))?)?$"

/* Parse vertex of the format "v/vt/vn", where vertex and normal indicies are
   counted from one, and zero indicates a missing value. */
static int parse_vertex(unsigned idx[3], unsigned count[3], char const *token)
{
	long val, i;
	struct recap matches[3];

	/* Check format with regular-expression */
	assert(recount(VERTEX_RE) == 3);
	if (!recap(VERTEX_RE, token, matches)) { return -1; }

	/* Position index */
	val = strtol(token, 0, 10);
	if (set_index(val, count[POS], idx)) { return -1; }

	/* Texture coordinate and normal indicies */
	for (i = UV; i < N_vertex_components; i++) {
		if (matches[i].offset >= 0) {
			val = strtol(token + matches[i].offset, 0, 10);
			if (set_index(val, count[i], idx + i)) { return -1; }
		} else {
			idx[i] = 0;
		}
	}
	return 0;
}

static int push_triangle(struct wbuf *vertices, unsigned int *tri)
{
	return wbuf_write(vertices, tri, 9 * sizeof *tri) ? 0 : -1;
}

static int parse_face(struct wbuf *vertices, unsigned count[3], FILE *fp)
{
	int i;
	char token[100];
	unsigned int tri[3*3];
	
	/* Read one trinangle at a minimum */
	for (i = 0; i < 3; i++) {
		if (wf_next_token(token, sizeof token, fp) == 0) { return -1; }
		if (parse_vertex(tri + i*3, count, token)) { return -1; }
	}
	if (push_triangle(vertices, tri)) { return -1; }

	/* OBJ supports faces with arbitrary number of vertices. These can be
	   turned into triangles by using the initial and previous vertices.
	   Note: we don't enforce that the vertices in a face lie in the same
	   plane. */
	for (i = 0; ; i++) {
		if (wf_next_argument(token, sizeof token, fp) == 0) { break; }
		memcpy(tri + 3, tri + 6, 3*sizeof *tri);
		if (parse_vertex(tri + 6, count, token)) { return -1; }
		if (push_triangle(vertices, tri)) { return -1; }
	}

	return 1 + i;
}

static int push_default_uv(struct obj_buffer *obj, unsigned (*idx)[3])
{
	/* Equilateral triangle in the unit square, with one vertex at (1,1) */
	static float const defuv[3][2] = {
		{ 0.0f, 0.732050808f },
		{ 1.0f, 1.0f },
		{ 0.732050808f, 0.0f }
	};

	if (!wbuf_write(&obj->uv, defuv, sizeof defuv)) { return -1; }
	(*idx)[0] = obj->count[UV] + 0;
	(*idx)[1] = obj->count[UV] + 1;
	(*idx)[2] = obj->count[UV] + 2;
	obj->count[UV] += 3;

	return 0;
}

/* Decrement a one-indexed compomnent to a zero-index one, unless one of the
   components already is zero. */
static int dec_one_indexed(unsigned triangle[3][3], int component)
{
	size_t i;

	/* Is any of them zero already? */
	for (i = 0; i < 3; i++) {
		if (triangle[i][component] == 0) {
			return -1;
		}
	}

	/* All is fine */
	for (i = 0; i < 3; i++) {
		triangle[i][component]--;
	}
	return 0;
}

/* Move from one based to zero-based index. */
static int fix_pos(struct wbuf *idxbuf)
{
	unsigned (*ind)[3][3], (*triangle)[3];
	size_t i, nind;

	ind = idxbuf->begin;
	nind = wbuf_nmemb(idxbuf, sizeof *ind);

	for (i = 0; i < nind; i++) {
		triangle = ind[i];
		if (dec_one_indexed(triangle, POS)) { return -1; }
	}
	return 0;
}

/* Add missing normal or texture coordinate vectors, and move from one based
   to zero-based index. In case of failure (allocation failure) the contents of
   `idxbuf` is left in an inconsistent state and shouldn't be used anymore. */
static int fix_uv(struct obj_buffer *obj, struct wbuf *idxbuf)
{
	unsigned (*ind)[3][3], (*triangle)[3], defuv[3];
	size_t i, j, nind;
	int has_defuv;

	has_defuv = 0;
	ind = idxbuf->begin;
	nind = wbuf_nmemb(idxbuf, sizeof *ind);

	for (i = 0; i < nind; i++) {
		triangle = ind[i];
		if (dec_one_indexed(triangle, UV)) {
			/* Lazily add default uv-coordinates */
			if (!has_defuv) {
				if (push_default_uv(obj, &defuv)) {
					return -1;
				}
				has_defuv = 1;
			}
			/* Change all components */
			for (j = 0; j < 3; j++) {
				triangle[j][UV] = defuv[j];
			}
		}
	}

	return 0;
}

/* Add missing normal or texture coordinate vectors */
static int fix_norm(struct obj_buffer *obj, struct wbuf *idxbuf)
{
	float n[3], (*pos)[3], (*v0)[3], (*v1)[3], (*v2)[3];
	unsigned (*ind)[3][3], (*triangle)[3], nidx;
	size_t i, j, nind;

	pos = obj->pos.begin;
	ind = idxbuf->begin;
	nind = wbuf_nmemb(idxbuf, sizeof *ind);

	/* Scan through each vertex */
	for (i = 0; i < nind; i++) {
		triangle = ind[i];
		if (dec_one_indexed(triangle, NORM)) {
			v0 = pos + triangle[0][POS];
			v1 = pos + triangle[1][POS];
			v2 = pos + triangle[2][POS];
			if (defines_planef(*v0, *v1, *v2)) {
				(void)plane_normalf(n, *v0, *v1, *v2);
				if(!wbuf_write(&obj->norm, n, sizeof n)) {
					return -1;
				}
				nidx = obj->count[NORM]++;
			} else {
				/* Point or line segment, but not a triangle.
				   Just use any normal. We'll make sure later
				   that it points to a valid index. */
				nidx = 0;
			}
			for (j = 0; j < 3; j++) {
				if (triangle[j][NORM] == 0) {
					triangle[j][NORM] = nidx;
				} else {
					triangle[j][NORM]--;
				}
			}
		}
	}
	if (obj->count[NORM] == 0 && nind > 0) {
		/* Pathological case: Every face was badly formed and didn't
		   define any normal vector. Push a single vector. */
		static float const defnorm[3] = { 0.f, 1.f, 0.f };
		if(!wbuf_write(&obj->norm, defnorm, sizeof defnorm)) {
			return -1;
		}
	}
	return 0;
}

static int push_vectorf(double *vecd, size_t dim, struct wbuf *buf)
{
	float *vecf;
	size_t i;

	vecf = wbuf_alloc(buf, dim *sizeof *vecf);
	if (!vecf) { return -1; }
	for (i = 0; i < dim; i++) { vecf[i] = vecd[i]; }
	return 0;
}

static struct tgroup *get_group(struct obj_buffer *obj, char const *mtlname)
{
	struct tgroup *g;
	size_t len;

	/* Look for existing face list that uses the same material, and leave
	   g pointing at the final group if it's not found. */
	if ((g = obj->groups)) {
		for (; ; g = list_next(g)) {
			if (streq(g->mtlname, mtlname)) { return g; }
			if (!list_next(g)) { break; }
		}
	}

	/* Add a new entry at the end of the list */
	len = mtlname ? strlen(mtlname) + 1 : 0;
	g = list_append(g, sizeof *g + len);
	if (!g) { return NULL; }
	wbuf_init(&g->vertices);
	g->mtlname = mtlname ? strcpy((char *)(g + 1), mtlname) : NULL;
	if (!obj->groups) { obj->groups = g; }
	return g;
}

/* Parse a list of file names, until end-of-line, and store them as strings,
   consecutively in `string_buffer`. Return the number of filenames read. */
static int parse_filenames(struct wbuf *string_buffer, FILE *fp)
{
	/* TODO: Handle spaces in filenames? Escaped spaces and quotes? */
	int n, res;

	n = 0;
	while (1) {
		res = wf_parse_filename(string_buffer, fp);
		if (res < 0) { return -1; }
		if (res == 0) { return n; }
		n++;
	}
}

/* Write string, including nul-terminator */
static char *wbuf_write_str0(struct wbuf *buf, char const *str)
{
	return wbuf_write(buf, str, strlen(str) + 1);
}

static char *wbuf_write_str0j(struct wbuf *buf, char const *str, jmp_buf j)
{
	char *p = wbuf_write_str0(buf, str);
	if (!p) { longjmp(j, -1); }
	return p;
}

static void *wbuf_allocj(struct wbuf *buf, size_t size, jmp_buf j)
{
	void *p = wbuf_alloc(buf, size);
	if (!p) { longjmp(j, -1); }
	return p;
}

static void *wbuf_concatj(struct wbuf *dest, struct wbuf const *src, jmp_buf j)
{
	void *p = wbuf_concat(dest, src);
	if (!p) { longjmp(j, -1); }
	return p;
}

static void wbuf_alignj(struct wbuf *buf, size_t align, jmp_buf j)
{
	if (wbuf_align(buf, align)) { longjmp(j, -1); }
}

static void init_obj_buffer(struct obj_buffer *obj)
{
	memset(obj->count, 0, sizeof obj->count);
	wbuf_init(&obj->pos);
	wbuf_init(&obj->norm);
	wbuf_init(&obj->uv);
	wbuf_init(&obj->mtllib);
	obj->nmtllib = 0;
	obj->groups = NULL;
}

static void free_obj_buffer(struct obj_buffer *obj)
{
	struct tgroup *g;

	wbuf_term(&obj->pos);
	wbuf_term(&obj->norm);
	wbuf_term(&obj->uv);
	wbuf_term(&obj->mtllib);
	for (g = obj->groups; g; g = list_remove(g)) {
		wbuf_term(&g->vertices);
	}
}

static int parse_obj(struct obj_buffer *obj, FILE *fp)
{
	char mtlname[500];
	double vec[3];
	struct tgroup *g;
	int i, n;

	g = NULL;

	/* Iterate over each line in the file */
	while (1) {
		switch (next_keyword(fp)) {
		case END_OF_FILE:
			/* Parsed successfully to the end of the stream */
			return 0;

		case COMMENT:
		case UNKNOWN:
			/* Ignore foreign directives and comment lines */
			wf_skip_line(fp);
			break;

		case NEWLINE:
			/* Empty line - just continue with next line. */
			continue;

		case USE_MATERIAL:
			/* Set vertex group based on material-name */
			if (wf_parse_mtlname(mtlname, sizeof mtlname, fp)) {
				return -1;
			}
			g = get_group(obj, mtlname);
			if (!g) { return -1; }
			break;

		case LOAD_MATERIAL:
			/* Add external material library - mtllib filename */
			n = parse_filenames(&obj->mtllib, fp);
			if (n < 0) { return -1; }
			obj->nmtllib += n;
			break;

		case GEO_VERTEX:
			/* Add geometric vertex - v v_x v_y v_z [v_w] */
			if (wf_parse_vector(vec, 3, fp)) { return -1; }
			if (push_vectorf(vec, 3, &obj->pos)) { return -1; }
			obj->count[POS]++;
			break;

		case TEX_VERTEX:
			/* Add texture vertex - vt u v */
			if (wf_parse_vector(vec, 2, fp)) { return -1; }
			for (i = 0; i < 2; i++) {
				if (vec[i] < 0.0 || vec[i] > 1.0) {
					return -1;
				}
			}
			if (push_vectorf(vec, 2, &obj->uv)) { return -1; }
			obj->count[UV]++;
			break;

		case NORMAL:
			if (wf_parse_vector(vec, 3, fp)) { return -1; }
			if (v3trynorm(vec, vec)) { return -1; }
			if (push_vectorf(vec, 3, &obj->norm)) { return -1; }
			obj->count[NORM]++;
			break;

		case FACE:
			if (g == NULL) {
				g = get_group(obj, NULL);
				if (!g) { return -1; }
			}
			n = parse_face(&g->vertices, obj->count, fp);
			if (n < 1) { return -1; }
			break;

		default:
			/* Unexpected token */ 
			abort();
			break;
		}

		/* Read next EOL */
		if (wf_expect_eol(fp)) { return -1; }
	}
}

static struct wf_object const *alloc_obj_block(struct obj_buffer *obj)
{
	struct wf_object *result;
	struct wbuf block;
	char const **mtllib;
	char *p;
	size_t i, ngroups, used;
	struct wf_triangles *groups;
	unsigned const (*indicies)[3][3];
	struct tgroup *g;

	/* Offsets for setting up pointers later */
	size_t off_mtllib, off_mtlstr, off_groups, off_grpstr, off_grpidx;
	size_t off_pos, off_uv, off_norm;
	jmp_buf errbuf;

	wbuf_init(&block);

	if (setjmp(errbuf)) {
		wbuf_term(&block);
		return NULL;
	}

	ngroups = list_length(obj->groups);

	/* 1. Place struct wf_object at the start of allocated area */
	wbuf_allocj(&block, sizeof *result, errbuf);

	/* 2. Array of pointer to string and string of strings */
	wbuf_alignj(&block, alignof(*result->mtllib), errbuf);
	off_mtllib = wbuf_size(&block);
	wbuf_allocj(&block, sizeof *result->mtllib * obj->nmtllib, errbuf);
	off_mtlstr = wbuf_size(&block);
	wbuf_concatj(&block, &obj->mtllib, errbuf);

	/* 3. Array of struct wf_triangles */
	wbuf_alignj(&block, alignof(*result->groups), errbuf);
	off_groups = wbuf_size(&block);
	wbuf_allocj(&block, sizeof *result->groups * ngroups, errbuf);
	off_grpstr = wbuf_size(&block);
	for (g = obj->groups; g; g = list_next(g)) {
		if (g->mtlname) { 
			wbuf_write_str0j(&block, g->mtlname, errbuf);
		}
	}

	/* 4. Arrays of unsigned[3][3] */
	wbuf_alignj(&block, alignof(unsigned), errbuf);
	off_grpidx = wbuf_size(&block);
	for (g = obj->groups; g; g = list_next(g)) {
		wbuf_concatj(&block, &g->vertices, errbuf);
	}

	/* 5. Arrays of float - pos, norm and uv */
	wbuf_alignj(&block, alignof(float), errbuf);
	off_pos = wbuf_size(&block);
	wbuf_concatj(&block, &obj->pos, errbuf);
	off_uv = wbuf_size(&block);
	wbuf_concatj(&block, &obj->uv, errbuf);
	off_norm = wbuf_size(&block);
	wbuf_concatj(&block, &obj->norm, errbuf);
	wbuf_trim(&block);

	/* Everything was allocated. Next we tie up pointers. */
	result = wbuf_get(&block, 0);

	/* Copy data */
	result->nmtllib = obj->nmtllib;
	result->npos = obj->count[POS];
	result->nuv = obj->count[UV];
	result->nnorm = obj->count[NORM];
	result->ngroups = ngroups;

	result->pos = result->npos ? wbuf_get(&block, off_pos) : NULL;
	result->uv = result->nuv ? wbuf_get(&block, off_uv) : NULL;
	result->norm = result->nnorm ? wbuf_get(&block, off_norm) : NULL;

	if (obj->nmtllib == 0) {
		result->mtllib = NULL;
	} else {
		mtllib = wbuf_get(&block, off_mtllib);
		p = wbuf_get(&block, off_mtlstr);
		for (i = 0; i < obj->nmtllib; i++) {
			mtllib[i] = p;
			p += strlen(p) + 1;
		}
		result->mtllib = mtllib;
	}

	/* Triangle groups */
	if (ngroups == 0) {
		result->groups = NULL;
	} else {
		groups = wbuf_get(&block, off_groups);
		indicies = wbuf_get(&block, off_grpidx);
		p = wbuf_get(&block, off_grpstr);
		for (g = obj->groups, i = 0; g; g = list_next(g), i++) {
			if (g->mtlname) {
				groups[i].mtlname = p;
				p += strlen(p) + 1;
			} else {
				groups[i].mtlname = NULL;
			}
			used = wbuf_size(&g->vertices) / sizeof *indicies;
			groups[i].n = used;
			if (used) {
				groups[i].indicies = indicies;
				indicies += used;
			} else {
				groups[i].indicies = NULL;
			}
		}
		result->groups = groups;
	}

	return result;
}

struct wf_object const *wf_parse_object(char const *filename)
{
	FILE *fp;
	struct wf_object const *result;

	fp = fopen(filename, "r");
	if (fp) {
		result = wf_fparse_object(fp);
		fclose(fp);
	} else {
		result = NULL;
	}
	return result;
}

struct wf_object const *wf_fparse_object(FILE *fp)
{
	struct wf_object const *result;
	struct obj_buffer obj;
	struct tgroup *t;

	assert(fp != NULL);
	init_obj_buffer(&obj);

	result = NULL;

	/* Parse the whole stream */
	if (parse_obj(&obj, fp)) { goto clean; }

	/* Change indicies and add default missing values */
	for (t = obj.groups; t; t = list_next(t)) {
		if (fix_pos(&t->vertices)) { goto clean; }
		if (fix_uv(&obj, &t->vertices)) { goto clean; }
		if (fix_norm(&obj, &t->vertices)) { goto clean; }
	}

	result = alloc_obj_block(&obj);

clean:	free_obj_buffer(&obj);

	return result;
}

void wf_free_object(struct wf_object const *obj)
{
	/* Every object is allocated into a single block. */
	free((void*)obj);
}

