
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

#include <base/list.h>
#include <base/wbuf.h>
#include <base/mem.h>

#include "include/wf.h"
#include "private.h"

/*
 Not supported:
   Kx spectral file.rfl factor
   Kx xyz x y z
   Transimission filter Tf
   d -halo
   Map parameters
   sharpness
*/

enum token_type
{
	UNKNOWN = 0,

	END_OF_FILE,
	NEWLINE,
	COMMENT,

	NEW_MATERIAL,
	FIELD
};

enum field
{
	AMBIENT,
	DIFFUSE,
	SPECULAR,
	EXPONENT,
	DISSOLVE,

	ILLUMINATION,

	MAP_AMBIENT,
	MAP_DIFFUSE,
	MAP_SPECULAR,
	MAP_EXPONENT,
	MAP_DISSOLVE,
	MAP_BUMP
};

struct token
{
	enum token_type type;
	enum field field;
};

struct mtl_buffer
{
	struct wf_material *mtl_list;
	char *str_list;
};

struct field_parser
{
	size_t offset;
	int (*parse)(void *field, struct mtl_buffer *buffer, FILE *fp);
};

static int parse_color(void *field, struct mtl_buffer *buffer, FILE *fp);
static int parse_scalar(void *field, struct mtl_buffer *buffer, FILE *fp);
static int parse_int(void *field, struct mtl_buffer *buffer, FILE *fp);
static int parse_map(void *field, struct mtl_buffer *buffer, FILE *fp);

static struct wf_map *getmap(struct wf_material *m, size_t i);

#define mtl_offsetof(field) offsetof(struct wf_material, field)

static size_t maps[] = {
	mtl_offsetof(map_ka),
	mtl_offsetof(map_kd),
	mtl_offsetof(map_ks),
	mtl_offsetof(map_ns),
	mtl_offsetof(map_d),
	mtl_offsetof(map_bump)
};

static struct field_parser const field_parsers[] = {
	[AMBIENT] = { mtl_offsetof(ka), parse_color },
	[DIFFUSE] = { mtl_offsetof(kd), parse_color },
	[SPECULAR] = { mtl_offsetof(ks), parse_color },

	[EXPONENT] = { mtl_offsetof(ns), parse_scalar },
	[DISSOLVE] = { mtl_offsetof(d), parse_scalar },

	[ILLUMINATION] = { mtl_offsetof(illum), parse_int },

	[MAP_AMBIENT] = { mtl_offsetof(map_ka), parse_map },
	[MAP_DIFFUSE] = { mtl_offsetof(map_kd), parse_map },
	[MAP_SPECULAR] = { mtl_offsetof(map_ks), parse_map },
	[MAP_EXPONENT] = { mtl_offsetof(map_ns), parse_map },
	[MAP_DISSOLVE] = { mtl_offsetof(map_d), parse_map },
	[MAP_BUMP] = { mtl_offsetof(map_bump), parse_map }
};

static struct token classify(char const *kw)
{
	static struct keyword const keywords[] = {
        	{ "\n", NEWLINE },
        	{ "#", COMMENT },
        	{ "newmtl", NEW_MATERIAL },
        	{ "Ka", FIELD },
        	{ "Kd", FIELD },
        	{ "Ks", FIELD },
        	{ "Ns", FIELD },
        	{ "d", FIELD },
		{ "Tr", FIELD },
        	{ "map_Ka", FIELD },
        	{ "map_Kd", FIELD },
        	{ "map_Ks", FIELD },
        	{ "map_d", FIELD },
		{ "map_Tr", FIELD },
		{ "map_bump", FIELD },
		{ "bump", FIELD }
	};

	static struct keyword const fields[] = {
        	{ "Ka", AMBIENT },
        	{ "Kd", DIFFUSE },
        	{ "Ks", SPECULAR },
        	{ "Ns", EXPONENT },
        	{ "d", DISSOLVE },
		{ "Tr", DISSOLVE },
        	{ "map_Ka", MAP_AMBIENT },
        	{ "map_Kd", MAP_DIFFUSE },
        	{ "map_Ks", MAP_SPECULAR },
        	{ "map_d", MAP_DISSOLVE },
		{ "map_Tr", MAP_DISSOLVE },
		{ "map_bump", MAP_BUMP },
		{ "bump", MAP_BUMP }
	};

	struct token t;
	t.type = wf_parse_keyword(kw, keywords, length_of(keywords), UNKNOWN);
	if (t.type == FIELD) {
		t.field = wf_parse_keyword(kw, fields, length_of(fields), -1);
	}
	return t;
}

static struct token next_keyword(FILE *fp)
{
        char directive[20];
        
        if (wf_next_token(directive, sizeof directive, fp) == 0) {
		struct token t;
		t.type = END_OF_FILE;
                return t;
        } else {
                return classify(directive);
        }
}

struct wf_material *push_material(struct wf_material **list, char const *name)
{
	static const float white[3] = { 1.f, 1.f, 1.f };

	struct wf_material *mtl;
	struct wf_map *map;
	size_t i;

	if (!name || !list) { return NULL; }

	mtl = list_prepend(*list, sizeof *mtl + strlen(name) + 1);
	if (!mtl) { return NULL; }

	*list = mtl;
	mtl->name = strcpy((char*)(mtl + 1), name);
	mtl->illum = 0;
	for (i = 0; i < length_of(maps); i++) {
		map = getmap(mtl, i);
		map->filename = NULL;
	}
	memcpy(mtl->ka, white, sizeof white);
	memcpy(mtl->kd, white, sizeof white);
	memcpy(mtl->ks, white, sizeof white);
        mtl->ns = 0.f;
	mtl->d = 0.f;

	return mtl;
}

static char const *push_filename(char **list, char const *filename)
{
	char *p;

	p = list_prepend(*list, strlen(filename) + 1);
	if (!p) { return NULL; }
	*list = p;
	return strcpy(p, filename);
}

static int parse_color(void *field, struct mtl_buffer *buffer, FILE *fp)
{
	double vec[3];
	size_t i;
	float (*p)[3];

	(void)buffer;

	if (wf_parse_vector(vec, 3, fp)) { return -1; }
	for (i = 0, p = field; i < length_of(vec); i++) {
		if (vec[i] < 0.0 || vec[i] > 1.0) {
			return -1;
		}
		(*p)[i] = vec[i];
	}
	return 0;
}

static int parse_scalar(void *field, struct mtl_buffer *buffer, FILE *fp)
{
	double val;

	(void)buffer;

	if (wf_parse_vector(&val, 1, fp)) { return -1; }
	*((float *)field) = val;

	return 0;
}

static int parse_int(void *field, struct mtl_buffer *buffer, FILE *fp)
{
	char token[sizeof(int) * CHAR_BIT + 1], *end;
	int *p;

	(void)buffer;

        if (wf_next_token(token, sizeof token, fp) == 0) { return -1; }

	p = field;
	*p = strtol(token, &end, 10);
	if (end == token || *end != '\0') { return -1; }

	return 0;
}

static int parse_map(void *field, struct mtl_buffer *buffer, FILE *fp)
{
	char filename[1024];
	struct wf_map *map;

	/* Only read filename, no parameters yet */
	if (wf_next_argument(filename, sizeof filename, fp) == 0) { return -1; }
	map = field;
	map->filename = push_filename(&buffer->str_list, filename);
	if (!map->filename) { return -1; }

	return 0;
}

static struct wf_map *getmap(struct wf_material *m, size_t i)
{
	return (struct wf_map *)((char *)m + maps[i]);
}

static int parse_mtllib(struct mtl_buffer *buffer, FILE *fp)
{
	struct token keyword;
	struct wf_material *m;
	char mtlname[500];
	void *p;

	m = NULL;

	while (1) {
		keyword = next_keyword(fp);
		switch (keyword.type) {
		case END_OF_FILE: return 0;

		case NEWLINE: continue;

		case UNKNOWN:
		case COMMENT:
			wf_skip_line(fp);
			break;

		case NEW_MATERIAL:
			if (wf_parse_mtlname(mtlname, sizeof mtlname, fp)) {
				return -1;
			}
			m = push_material(&buffer->mtl_list, mtlname);
			if (!m) { return -1; }
			break;

		case FIELD:
			/* A field - use lookup table */
			if (!m) { return -1; }
			p = (char *)m + field_parsers[keyword.field].offset;
			if (field_parsers[keyword.field].parse(p, buffer, fp)) {
				return -1;
			}
			break;

		default:
			assert(0 && "Unhandled material case!");
		}

		if (wf_expect_eol(fp)) { return -1; }
	}
}

static struct wf_mtllib *alloc_block(struct mtl_buffer *buffer)
{
	struct wf_mtllib *mtllib;
	struct wf_material *head, *tail, *mtl, *p;
	struct wf_map *map;
	size_t total, i, nmtl, off_str, len;
	char *str, *filename;

	head = list_head(buffer->mtl_list);
	tail = list_tail(buffer->mtl_list);
	nmtl = list_length(head);

	/* Calculate total size of block */
	total = sizeof *mtllib;
	total += sizeof mtllib->m[0] * nmtl;

	off_str = total;

	for (mtl = head; mtl; mtl = list_next(mtl)) {
		/* Allocate space for material name */
		total += strlen(mtl->name) + 1;

		/* Allocate space for map filenames */
		for (i = 0; i < length_of(maps); i++) {
			map = getmap(mtl, i);
			if (map->filename) {
				total += strlen(map->filename) + 1;
			}
		}
	}

	/* Allocate, and initialize values and pointers */
	mtllib = malloc(total);
	if (!mtllib) { return NULL; }

	str = ((char *)mtllib) + off_str;
	mtllib->n = nmtl;

	for (p = mtllib->m, mtl = tail; mtl; p++, mtl = list_prev(mtl)) {
		memcpy(p, mtl, sizeof *mtl);
		len = strlen(mtl->name);
		p->name = memcpy(str, mtl->name, len + 1);
		str += len + 1;
		for (i = 0; i < length_of(maps); i++) {
			map = getmap(mtl, i);
			if (map->filename) {
				len = strlen(map->filename);
				filename = memcpy(str, map->filename, len);
				str += len + 1;
			} else {
				filename = NULL;
			}
			getmap(p, i)->filename = filename;
		}
	}
	return mtllib;
}

struct wf_mtllib const *wf_parse_mtllib(char const *filename)
{
	FILE *fp;
	struct wf_mtllib const *result;

	fp = fopen(filename, "r");
	if (fp) {
		result = wf_fparse_mtllib(fp);
		fclose(fp);
	} else {
		result = NULL;
	}
	return result;
}

struct wf_mtllib const *wf_fparse_mtllib(FILE *fp)
{
	struct wf_mtllib *result;
	struct mtl_buffer buffer = { NULL, NULL };

	assert(fp != NULL);

	result = NULL;
	if (parse_mtllib(&buffer, fp)) { goto clean; }
	result = alloc_block(&buffer);

clean:	list_free(buffer.mtl_list);
	list_free(buffer.str_list);

	return result;
}

struct wf_material const *wf_get_material(struct wf_mtllib const *mtllib,
                                          char const *name)
{
	size_t i;
	assert(mtllib != NULL);
	for (i = 0; i < mtllib->n; i++) {
		if (streq(mtllib->m[i].name, name)) { return mtllib->m + i; }
	}
	return NULL;
}

void wf_free_mtllib(struct wf_mtllib const *mtllib)
{
	free((void *)mtllib);
}

