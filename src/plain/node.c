#include <stddef.h>
#include <setjmp.h>
#include <stdbool.h>

typedef float plain_f;
typedef long plain_i;

struct plain_alloc
{
	void *free;
	jmp_buf oom;
};

void *plain_alloc(struct plain_alloc *alloc, size_t size);

enum plain_type
{
	plain_int_type,
	plain_float_type,
	plain_symbol_type,
	plain_string_type,
	plain_list_type,

	plain_type_count
};

struct plain_symbol
{
	char const *name;
};

struct plain_list
{
	struct plain_value value;
	struct plain_list const *next;
};

struct plain_iarray
{
	size_t size;
	plain_i items[];
};

struct plain_farray
{
	size_t size;
	plain_float_type items[];
};

struct plain_element {
	struct plain_element (*type)(
		struct plain_alloc *a,
		struct plain_value param);
	struct plain_value param;
};

struct plain_value
{
	enum plain_type type;
	union {
		struct plain_list const *list;
		struct plain_symbol const *symbol;
		struct plain_string const *string;
		struct plain_element const *element;
		plain_i i;
		plain_float_type f; 
	};
};

bool plain_value_eq(struct plain_value a, struct plain_value b)
{
	struct plain_list const *p, *q;

	if (a.type != b.type) return false;
	switch (a.type) {
	case plain_int_type: return a.i == b.i;
	case plain_float_type: return a.f == b.f;
	case plain_symbol_type: return a.symbol == b.symbol;
	case plain_string_type:
		return a.string == b.string || strcmp(a.string, b.string) == 0;
	case plain_list_type:
		p = a.list;
		q = b.list;
		if (p == q) { return true; }
		if (!p || !q) { return false; }
		while (*p && *q) {
			if (!plain_value_eq(p->value, q->value)) return false;
			p = p->next;
			q = q->next;
		}
		return p == q;
	}
}

struct plain_value plain_int(plain_i i)
{
	return (struct plain_value) { .type = plain_int_type, .i = i };
}

struct plain_value plain_mkfloat(plain_float_type f)
{
	return (struct plain_value) { .type = plain_float_type, .f = f };
}

struct plain_value plain_cons(
	struct plain_alloc *a,
	struct plain_value val,
	struct plain_list const *next)
{
	struct plain_list *l;

	l = plain_alloc(a, sizeof *l);
	l->val = val;
	l->next = next;
	return (struct plain_value) { .type = plain_list, .list = l };
}

struct plain_value plain_mklist(struct plain_alloc *a, ...)
{
	va_list ap;

	struct plain_list *p;

	va_start(ap, a);

	va_end(ap);
}

struct plain_element *plain_element(
	struct plain_alloc *a,
	struct plain_element (*type)(struct plain_alloc *, struct plain_value),
	struct plain_value param)
{
	struct plain_element *elem;

	elem = plain_alloc(a, sizeof *elem);
	elem->type = type;
	elem->param = param;
	return elem;
}

/* Render to stream example */
struct plain_element plain_header_tag(
	struct plain_alloc *a,
	struct plain_value param)
{
	abort();
}

struct plain_element plain_header(
	struct plain_alloc *a,
	struct plain_value param)
{
	check_plain_string(param);
	return plain_element(a, plain_header_tag, param);
}

enum {
	plain_header_element,
	plain_panel_element,
	plain_text_element
};

struct plain
{
	struct plain_alloc *alloc;
	struct plain_element *root;
};

struct plain make_plain(void);
void free_plain(struct plain *);

void plain_diff(
	struct plain_alloc *a,
	struct plain_element const *element)
{
}

void plain_render(struct plain *plain, struct plain_element *element)
{
	if (element == plain->root) { return; }

}

void frender(
	FILE *fp,
	struct plain_cache *cache,
	struct plain_element *element,
	int depth)
{
	int primitive_type;
	struct element *cached;

	cached = cache_get(element);
	do {
		primitive_type = to_primitive(element);
		if (primitive_type < 0) {
			element = element->type(a, element->props);
			continue;
		}
		break;
	} while (1);

	switch (primitive_type) {
	case plain_header_element:
		fprintf(fp, "<header>\n");
		children = element->param.list
		for (p = children; p; p = plain_next(p)) {
			element = to_element(a, p);
			frender(fp, cache, element, depth + 1);
		}
		fprintf(fp, "</header>\n");
		break;

	case plain_panel_element:
		fprintf(fp, "<panel>\n");
		children = element->param.list
		for (p = children; p; p = plain_next(p)) {
			element = to_element(a, p);
			frender(fp, cache, element, depth + 1);
		}
		fprintf(fp, "</panel>\n");
		break;

	case plain_text_element:
		fprintf(fp, "%s\n", element->param.string);
		break;
	}
}

bool plain_value_equal(struct plain_value a, struct plain_value b);

typedef struct plain_value val;
typedef struct plain_list const list;

val head(list *l);
val tail(list *l);

/*
 
 <table class="nice" colums="3">
   <tr>
    <td>one</td><td>two</td>
   <tr>
   <tr>
    <td><button>click</button></td>
    <td><button class="btn" group="my-group">press me</button></td>
    <td>
      <ul>
       <li>Item one</li>
       <li>Item two</li>
       <li>Item three</li>
      </ul>
    </td>
   <tr>
 <table>

 - Properties always as second parameter
 (table ((class nice)
         (columns 3))
   (tr ()
     (td ()
       (text () "one"))
     (td ()
       (text () "two")))
   (tr ()
     (td ()
       (button ()
         (text () "click")))
     (td ()
       (button ((class btn)
                (group "my-group"))
         (text () "press" "me")))
     (td ()
       (ul ()
         (li ()
	   (text () "Item" "one"))
         (li ()
	   (text () "Item" "two"))
         (li ()
	   (text () "Item" "three"))))))

 - Parentheses wraps tag
 - optionally, list or symbol in car
 - (tag . a-list)
 ((table (class nice)
         (columns 3))
   (tr (td "one")
       (td "two"))
   (tr (td (button "click"))
       (td ((button (class btn) (group "my-group"))
            "press me"))
       (td (ul (li (text "Item" "one"))
               (li (text "Item" "two"))
               (li (text "Item" "three"))))))

 - Parentheses wraps tag
 - optionally, list or symbol in car
 - (tag . p-list)
 ((table :class nice :columns 3)
   (tr (td (text "one"))
       (td (text "two")))
   (tr (td ((button :class btn :group "my-group")
            (text "press" "me")))
       (td (ul (li (text "Item" "one"))
               (li (text "Item" "two"))
               (li (text "Item" "three"))))))

 - Parentheses always mandatory
 ((table :class nice :columns 3)
   ((tr) ((td) ((text) "one"))
       ((td) ((text) "two")))
   ((tr) ((td) ((button :class btn :group "my-group")
                (text "press" "me")))
       ((td) ((ul) ((li) ((text) "Item" "one"))
               ((li) ((text) "Item" "two"))
               ((li) ((text) "Item" "three"))))))


 - (tag . p-list . children)
 (table :class nice
        :columns 3
        :children
   (tr (td (text "one"))
       (td (text "two")))
   (tr (td (button :class btn
                   :group "my-group")
            (text "press" "me")))
       (td (ul (li (text "Item" "one"))
               (li (text "Item" "two"))
               (li (text "Item" "three"))))))



 ((button
    (class nice very-cool)
    (click "group five"))
   (text "something here"))
 
 (button
   ((class nice very-cool)
    (click "group five"))
   (text "something here"))

*/

bool plain_element_equal(list *a, list *b)
{
	if (a == b) { return true; }

	/* type tags must equal */
	if (!plain_value_equal(head(a), head(b))) { return false; }

	/* properties must equal */
	return plain_value_equal(head(tail(a)), head(tail(b)));
}

void *update(component_t component, void *state, void *cache)
{
	void *element, *props, *new_children, *;

	element = component.render(state);

	if (wrong_type(element)) { return NULL; }
	if (cache && plain_element_equal(element, cache)) {
		
	}

	new_children = children_of(element);
	old_children = children_of(cache);

	for (child in new_children) {
		
	}
}
