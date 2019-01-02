lx - list expressions
---------------------

lx aims to bring list processing to C. It implements an immutable tree
data-structure built from nested lists, which is very similar to S-expressions
found in LISP and Scheme.

- Immutable lists, booleans, integers, and floating point numbers.
- 16, 32, or 64 bit reference/integer size for small and large data structures.
- Only proper lists are supported, i.e. no circular or improper lists.
- Lists have a very compact representation: in the best case a list of integers
  of length N will only need an extra *char[N]* of storage compared to an
  equivalent array. And there's no padding. In the worst case the list of will
  need twice as much memory as an array and an extra *char[2N]* for
  bookkeeping.
- List contents are stored in tagged unions - the type tag is stored along with
  the data so that integers, booleans, and floats can be stored directly in the
  list and do not have to be boxed.


Tagged union representation
---------------------------

S-expressions can be used to build arbitrary tree structures and fundamentally
consist of ordered pairs and atomic symbols. Pairs contain two terms, each of
which can be either an atomic symbol or another pair. Atomic symbols are
textually represented as a sequence of characters but the only essential
property is that one symbol can be distinguished from another.

A straight-forward implementation of a pair is a structure of two pointers to
symbols or pairs, and in order to know which one, run-time type information
needs to be stored somewhere. There are many ways to associate dynamic type
information with objects. C conveniently allows you to place a header field in
every object which contains a virtual table pointer or a type tag. For
instance, a toy LISP interpreter might implement S-expressions with

    enum sexp_type { pair_type, atom_type };
    enum symbol { NIL, A, B, C, ..., ZERO, ONE, ... };

    struct sexp { enum sexp_type type; };
    struct atom { struct sexp header; enum symbol value; };
    struct pair { struct sexp header, *value[2]; };

    struct atom *as_atom(struct sexp *v) {
        return v->type == atom_type ? (struct atom *)v : NULL;
    }

    struct pair *as_pair(struct sexp *v) {
        return v->type == pair_type ? (struct pair *)v : NULL;
    }

One bit is enough for the header but the pair structure most likely padded to
the size of three pointers. Practical implementations would have more fine
grained atoms, such as strings and machine friendly numbers, as well as data
structures other than pairs.

The convenience of treating every S-expression as a pointer comes at the cost
of pointer indirection, which might result in cache-misses, and garbage and
bloat from small boxed values. A well-known option used in interpreters for
dynamically typed languages like Scheme is to store small values, such as
`enum symbol` (or integers generally), in the pointer itself. Addresses might
have a minimum alignment which leaves some bits in the pointer unused. A small
type tag can be stored there to tell an integer and a pointer apart (a tagged
pointer). A similar alternative is to store the type tag next to the pointer as
a tagged union so that all the bits can be used for the integer (a fat pointer).

If the number of types is small enough (an escape tag can indicate that the
pointer points to a large object with a extensible header) then a few bits is
enough to store the tag. In particular, if two (or more) tags can be stored in
the space of a pointer then we can replace the header with field tags and the
size of the pair structure remains the same, but small values are no longer
boxed:

    union data { enum symbol atom; struct pair *pair; };

    struct sexp { unsigned char tag; union data value; };
    struct pair { unsigned char tag[2]; union data value[2]; };

    enum sexp_type tag_to_type(unsigned char tag) {
        return tag;
    }

    struct sexp get(struct sexp v, int i) {
        assert(tag_to_type(v.tag) == pair_type);
        return (struct sexp) {
            .tag = v.value.pair->tag[i],
            .value = v.value.pair->value[i]
        };
    }

    struct sexp car(struct sexp v) {
        return get(v, 0);
    }

    struct sexp cdr(struct sexp v) {
        return get(v, 1);
    }

It's no longer necessary to down-cast pointers since you can branch on the tag
in `struct sexp` but the structure is larger than a pointer. Hopefully there
will be more pointers from pairs to other pairs than `struct sexp` variables.

There is likely some padding left in the pair structure unless `sizeof(void *)`
equals two. In fact, a structure with as many tags and pointers as there are
bytes in a pointer would have no padding at all. Let's define that structure as
`struct span`:

    #define N sizeof(void *)
    struct span { unsigned char tag[N]; union data data[N]; };

A span is a short and compact array of *tagged unions* the size of a native
machine word (or at least that of a pointer). An array of spans can be seen as
two interleaved arrays, the tag and data arrays, or as an arbitrarily long
array of tagged unions. Given that a `struct span` is pointer aligned, a
*tagged pointer* is enough to store the address of a span and the offset of any
span element (combined tag and data) in the lower bits of the pointer:

    /* assuming sizeof(union data) == sizeof(void *) == a power of two */
    #define OFFSET_MASK (uintptr_t)(sizeof(union data) - 1)
    struct span_ref { uintptr_t p; };

    struct span_ref span_ref(struct span *s, int offset) {
        /* assumes flat address space, compiler/architecture dependent */
        return (struct span_ref) { (uintptr_t)s + (offset & OFFSET_MASK) };
    }

    uintptr_t span_ref_offset(struct span_ref sref) {
        return sref.p & OFFSET_MASK;
    }

    struct span *span_ref_deref(struct span_ref sref) {
        return (struct span *)(sref.p & ~OFFSET_MASK);
    }

    struct span_ref span_ref_next(struct span_ref sref) {
        if (span_ref_offset(sref) + 1 < N)
            return (struct span_ref) { sref.p + 1 };
        else
            return span_ref(span_ref_deref(sref) + 1, 0);
    }

    unsigned char *span_ref_tag(struct span_ref sref) {
        /* p might be interpretable as a pointer to the tag already */
        return span_ref_deref(sref)->tag + span_ref_offset(sref);
    }

    union data *span_ref_data(struct span_ref sref) {
        return span_ref_deref(sref)->data + span_ref_offset(sref);
    }

The memory allocator would allocate pairs from an array of spans and a pair is
naturally represented as two consequtive tagged elements in a span:

    struct pair { struct span_ref sref; };
    union data { enum symbol atom; struct pair pair; };

    struct sexp { unsigned char tag; union data value; };

    struct sexp extract(struct span_ref sref) {
        return (struct sexp) {
            .tag = *span_ref_tag(sref),
            .value = *span_ref_data(sref)
        };
    }

    struct sexp car(struct sexp v) {
        assert(tag_to_type(v.tag) == pair_type);
        return extract(v.value.pair.sref);
    }

    struct sexp cdr(struct sexp v) {
        assert(tag_to_type(v.tag) == pair_type);
        return extract(span_ref_next(v.value.pair.sref));
    }

Compared to the previous example there's a lot of added complexity for a few
eliminated padding bytes. However, in an array of spans we can store not just
pairs, but structurs with any number of fields and a `struct sref` tagged
pointer can point into the middle of a span to any of its element.

Lists are built from ordered pairs where the second term is the rest of the
list (either another list pair or NIL). Lists can be used as records or tuples
and as building blocks for more complex data structures. Lists are fundamental
to many programming languages, LISP in particular, where it's very common that
the second element (CDR) of a pair will refer to another pair.

CDR-coding was deviced to save memory by eliminating the CDR field when
possible. A list structural tag is added to each pair, a single bit at a
minimum, which specifies that the pair is either a normal pair with a CDR field
or that the CDR is a pair which begins where you would normally find the CDR
field -- two pairs partially overlap. In this way lists can be made of compact
sections followed by a link to another compact section, which might be a common
tail of many lists. Special list construction functions and/or a compacting
garbage collector would arrange lists in this way. If the CDR field is mutable
it really complicates things but CDR-coding is still possible.

In a `struct span` we can put the CDR-code inside of the tag and leave the rest
of the bits for type tags:

    enum cdr_code { cdr_normal, cdr_adjacent };

    enum sexp_type tag_to_type(unsigned char tag) {
        return tag >> 1;
    }

    enum sexp_type tag_to_cdr_code(unsigned char tag) {
        return tag & 1;
    }

    struct sexp cdr(struct sexp v) {
        assert(tag_to_type(v.tag) == pair_type);
        switch(tag_to_cdr_code(v.tag)) {
        case cdr_normal:
            /* as before, dereference the CDR field */
            return extract(span_ref_next(v.value.pair.sref));
        case cdr_adjacent:
            /* no need to dereference (in extract) a pointer, we know the type
               and can derive its location directly from "v" */
            return (struct sexp) {
                .tag = pair_type,
                .value.list.sref = span_ref_next(v.value.pair.sref)
            };
        }
    }

lx uses a memory layout similar to `struct span` to implement a compact
tagged-union scheme where an 8-bit tag field is associated with 16, 32, or 64
bit wide data fields.


Auto-relative pointers
-----------------------

References in lx are auto-relative pointers, i.e. the reference contains the
offset you need to add to its address to reach the target. Auto-relative
pointers have various benefits:

 - Like array indexes they can be smaller than the native pointer type, but you
   don't need a pointer to the start of the array to use them.
 - Auto-relative offsets remain valid after the memory region is copied or
   moved by e.g. a call to realloc(3).
 - Auto-relative pointers are integers which gives flexibility (and
   portability) in where to store tags for tagged pointers.
 - No longer encumbered by patents. 

The main drawback is that standard C makes pointer arithmetic undefined for
pointers from different arrays. This mandates that auto-relative pointers can
only point to locations in the same storage object, or the compiler might find
opportunities to exploint undefined behavior.
