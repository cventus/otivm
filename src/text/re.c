#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <string.h>

#include "text/re.h"

enum groupopt { CAPTURE = 1 };
enum quantpol { GREEDY = 0, LAZY };

struct slice
{
	char const *p;
	size_t len;
};

struct quant
{
	size_t min, max;
	int more;
	enum quantpol policy;
};

/* Functions that match a single character */
typedef int matchfn(struct slice re, struct slice text);
static matchfn
	match_any,
	match_literal,
	match_digit, match_not_digit,
	match_space, match_not_space,
	match_word, match_not_word,
	match_exact,
	match_class,
	match_complement;

/* Match next (quantified) expression */
static int match_exp(struct slice, struct slice, int, struct recap [], int);

/* Match any of the branches in the regular expression */
static int match_alt(struct slice, struct slice, int, struct recap [], int);

static struct slice mkslicen(char const *p, size_t len)
{
	return (struct slice){ p, len };
}

static struct slice mkslice(char const *p)
{
	return mkslicen(p, strlen(p));
}

/* Return suffix of `s` where the first `n` characters have been removed. */
static struct slice advance(struct slice s, size_t n)
{
	return mkslicen(s.p + n, s.len - n);
}

/* Return prefix of `s` where the last `n` characters have been removed. */
static struct slice shrink(struct slice s, size_t n)
{
	return mkslicen(s.p, n > s.len ? 0 : s.len - n);
}

/* Return prefix of str that is at most n characters long. */
static struct slice prefix(struct slice s, size_t n)
{
	return mkslicen(s.p, n > s.len ? s.len : n);
}

/* Parse string of one of the forms "{a}", "{a,}" and "{a,b}" where `a` and `b`
   are non-negative integers. */
static int parse_range(char const *re, struct quant *q)
{
	char const *end, *p;
	unsigned long min, max;

	assert(re[0] == '{');

	p = re + 1;
	min = strtoul(p, (char **)&end, 10);
	if (end == p) { return -1; }
	
	q->min = min;

	if (end[0] == '}') {
		/* Constant number of repetitions */
		q->max = min;
		q->more = 0;
		return (end - re) + 1;
	}
	if (end[0] != ',') {
		/* Syntax error */
		return -1;
	}
	if (end[1] == '}') {
		/* Open ended */
		q->max = 0;
		q->more = 1;
		return (end - re) + 2;
	}

	/* Max repetitions */
	p = end + 1;
	max = strtoul(p, (char **)&end, 10);

	if (max < min || p == end || *end != '}') {
		/* Invalid range */
		return -1;
	} else {
		q->max = max;
		q->more = 0;
		return (end - re) + 1;
	}
}

/* Store {min,max} in range[0] and range[1], return length or
   negative in case of error. */
static int parse_quantifier(struct slice const s, struct quant *q)
{
	int n;

	/* End of regex, or escaped character */
	if (s.len == 0 || s.p[0] == '\\') {
		q->min = q->max = 1;
		q->more = 0;
		q->policy = GREEDY;
		return 0;
	}

	switch (s.p[0]) {
	case '*': /* zero or more */
		q->min = 0;
		q->max = 0;
		q->more = 1;
		n = 1;
		break;

	case '+': /* one or more */
		q->min = 1;
		q->max = 0;
		q->more = 1;
		n = 1;
		break;

	case '?': /* zero or one */
		q->min = 0;
		q->max = 1;
		q->more = 0;
		n = 1;
		break;

	case '{': /* custom range */
		n = parse_range(s.p, q);
		assert(n < 0 || ((size_t)n <= s.len && "Parsed too far"));
		if (n < 0) { return -1; }
		break;

	default: /* general case, only one */
		q->min = 1;
		q->max = 1;
		q->more = 0;
		q->policy = GREEDY;
		return 0;
	}

	if ((int)s.len > n && s.p[n] == '?') {
		q->policy = LAZY;
		return n + 1;
	} else {
		q->policy = GREEDY;
		return n;
	}
}

static struct slice get_groupre(char const *p, int len, unsigned flags)
{
	if (flags & CAPTURE) {
		/* Skip prefix "(" and suffix ")" */
		return mkslicen(p + 1, len - 2);
	} else {
		/* Skip prefix "(?:" and suffix ")" */
		return mkslicen(p + 3, len - 4);
	}
}

static int parse_builtin(struct slice const re, matchfn **match)
{
	if (re.len < 2) { return -1; }

	if (strchr("{}*?+()[]^$|.\\", re.p[1])) {
		if (match) { *match = match_literal; }
	} else {
		switch (re.p[1]) {
		case 'd': if (match) { *match = match_digit; } break;
		case 'D': if (match) { *match = match_not_digit; } break;
		case 's': if (match) { *match = match_space; } break;
		case 'S': if (match) { *match = match_not_space; } break;
		case 'w': if (match) { *match = match_word; } break;
		case 'W': if (match) { *match = match_not_word; } break;
		default: return -1;
		}
	}
	return 2;
}

static int parse_class(struct slice const s, matchfn **match)
{
	size_t i, cmpl;

	if (s.len < 2) { return -1; }
	cmpl = s.p[1] == '^';
	for (i = 1 + cmpl; i < s.len; i++) {
		switch (s.p[i]) {
		case '\\': i++; break;
		case ']':
			if (match) {
				*match = cmpl ? match_complement : match_class;
			}
			return i + 1;
		default: break;
		}
	}
	return -1;
}

/* return length of group regular expression including parentheses */
static int parse_group(struct slice const re, unsigned *opt, unsigned *ncap)
{
	int i, j;

	/* Parse group option (capturing or not) */
	if (re.len > 3 && strncmp(re.p, "(?:", 3) == 0) {
		if (opt) { *opt = 0; }
		i = 3;
	} else {
		if (opt) { *opt = CAPTURE; }
		if (ncap) { (*ncap)++; }
		i = 1;
	}
	while (i < (int)re.len) {
		switch (re.p[i]) {
		case '\\': j = parse_builtin(advance(re, i), NULL); break;
		case '[': j = parse_class(advance(re, i), NULL); break;
		case '(': j = parse_group(advance(re, i), NULL, ncap); break;
		case ')': return i + 1;
		default: j = 1; break;
		}
		if (j < 0) { return -1; }
		i += j;
	}
	return -1;
}

/* Return character span length until the first '|' character or end of string,
   or negative if there are unmatched parentheses. Increment `ncap` with the
   number of capturing groups in the branch. */
static int parse_branch(struct slice re, unsigned *ncap)
{
	int i, j;

	assert(re.len < INT_MAX);

	for (i = 0; i < (int)re.len; ) {
		switch (re.p[i]) {
		case '|': return i;
		case '\\': j = parse_builtin(advance(re, i), NULL); break;
		case '[': j = parse_class(advance(re, i), NULL); break;
		case '(': j = parse_group(advance(re, i), NULL, ncap); break;
		default: j = 1; break;
		}
		if (j < 0) { return -1; }
		i += j;
	}
	return i;
}

static void increase_length(int *len, int n, struct quant const *q)
{
	if (len && *len >= 0) {
		if (n < 0 || q->min != q->max || q->more) {
			/* Variable length expression */
			*len = -1;
		} else {
			/* Fixed length expression */
			*len += n * q->min;
		}
	}
}

static int analyze(struct slice re, int *len, unsigned *ncap);

static int analyze_rec(struct slice re, int *len)
{
	size_t i;
	int relen, qlen, n;
	struct quant q;
	struct slice gre;
	unsigned flags;

	if (len) { *len = 0; }

	for (i = 0; i < re.len; ) {
		if (strchr("{}*?+)]", re.p[i])) {
			/* Unmatched quantifier, parenthesis, or character
			   class */
			return -1;
		} else if (re.p[i] == '(') {
			relen = parse_group(advance(re, i), &flags, NULL);
			if (relen < 0) { return -1; }

			qlen = parse_quantifier(advance(re, i + relen), &q);
			if (qlen < 0) { return -1; }

			gre = get_groupre(re.p + i, relen, flags);
			if (analyze(gre, &n, NULL) != 0) { return -1; }

			increase_length(len, n, &q);
			i += relen + qlen;
		} else {
			if (re.p[i] == '\\') {
				relen = parse_builtin(advance(re, i), NULL);
			} else if (re.p[i] == '[') {
				/* custom character class */
				relen = parse_class(advance(re, i), NULL);
			} else {
				/* wildcard or any character */
				relen = 1;
			}
			if (relen < 0) { return -1; }

			qlen = parse_quantifier(advance(re, i + relen), &q);
			if (qlen < 0) { return -1; }

			increase_length(len, 1, &q);
			i += relen + qlen;
		}
	}
	return 0;
}

/* Analyze and validate regular expression */
static int analyze(struct slice re, int *len, unsigned *ncap)
{
	size_t off;
	int m, n, first;
	struct slice s;

	for (first = 1, m = 0, off = 0; off <= re.len; off += m + 1) {
		s = advance(re, off);
		m = parse_branch(s, ncap);
		if (m < 0) { return -1; }
		if (analyze_rec(prefix(s, m), &n) != 0) { return -1; }

		/* Compare lengths of consequtive branches and see if their
		   lengths match. */
		if (len) {
			if (first) {
				*len = n;
				first = 0;
			} else if (*len >= 0 && *len != n) {
				*len = -1;
			}
		}
	}
	return 0;
}

static int match_any(struct slice re, struct slice text)
{
	(void)re, (void)text;
	return 1;
}

static int match_literal(struct slice re, struct slice text)
{
	assert(re.p[0] == '\\');
	assert(re.len == 2);
	return text.p[0] == re.p[1];
}

static int match_digit(struct slice re, struct slice text)
{
	(void)re;
	return isdigit(text.p[0]);
}

static int match_not_digit(struct slice re, struct slice text)
{
	return !match_digit(re, text);
}

static int match_space(struct slice re, struct slice text)
{
	(void)re;
	return isspace(text.p[0]);
}

static int match_not_space(struct slice re, struct slice text)
{
	return !match_space(re, text);
}

static int match_word(struct slice re, struct slice text)
{
	/* [A-Za-z0-9_] */
	(void)re;
	return isalnum(text.p[0]) || text.p[0] == '_';
}

static int match_not_word(struct slice re, struct slice text)
{
	return !match_word(re, text);
}

static int match_exact(struct slice re, struct slice text)
{
	return text.p[0] == re.p[0];
}

/* Store the character that possibly starts a range in `ch`, and return the
   number of characters consumed. */
static int classch(struct slice cls, size_t i, char const **ch)
{
	assert(i < cls.len);

	/* Get start of range */
	if (cls.p[i] == '\\') {
		/* A class cannot end with a back-slash (as it would escape the
		   closing square bracket). */
		assert(i + 1 < cls.len);

		*ch = cls.p + i + 1;
		return 2;
	} else {
		*ch = cls.p + i;
		return 1;
	}
}

/* Check whether text.p[0] is in the character class cls, which is a string
   that consists of characters or ranges of characters, e.g. "0-9". This
   version does not support the union of built-in character classes as some
   regular expression matchers do. */
static int oneof(struct slice cls, struct slice text)
{
	size_t i;
	char const *p, *q;

	for (i = 0; i < cls.len; ) {
		i += classch(cls, i, &p);
		if (*p == text.p[0]) {
			return 1;
		} else if (i + 1 < cls.len && cls.p[i] == '-') {
			i++;
			i += classch(cls, i, &q);
			if (*p < text.p[0] && text.p[0] <= *q) {
				return 1;
			}
		}
	}
	return 0;
}

static int match_class(struct slice cls, struct slice text)
{
	/* Strip out `[` and `]` */
	return oneof(shrink(advance(cls, 1), 1), text);
}

static int match_complement(struct slice cls, struct slice text)
{
	/* Strip out `[`, `^` and `]` */
	return !oneof(shrink(advance(cls, 2), 1), text);
}

static struct recap *nextcap(struct recap *cap, unsigned flags, unsigned ncap)
{
	return cap && flags & CAPTURE ? cap + ncap : cap;
}

static void setcap(struct recap *cap, unsigned flags, int offset, size_t length)
{
	if (cap && flags & CAPTURE) {
		cap->offset = offset;
		cap->length = length;
	}
}

static int greedy_exp(
	struct quant const *q,
	struct slice re,
	matchfn *match,
	struct slice rest_re,
	struct slice text,
	size_t offset,
	struct recap cap[],
	int anchor)
{
	size_t i;
	int n;

	for (i = 0; i < text.len && (i < q->max || q->more); i++) {
		if (!match(re, advance(text, i))) { break; }
	}
	if (i < q->min) { return -1; }
	while (1) {
		/* Note: alternation is handled before this function is called
		   because we're matching a single character */
		n = match_exp(rest_re, advance(text, i), offset + i, cap,
		              anchor);
		if (n >= 0) { return n + (int)i; }
		else if (i == q->min) { return -1; }
		else { i--; }
	}
}

static int lazy_exp(
	struct quant const *q,
	struct slice re,
	matchfn *match,
	struct slice rest_re,
	struct slice text,
	size_t offset,
	struct recap cap[],
	int anchor)
{
	size_t i;
	int n;

	/* minimum */
	for (i = 0; i < text.len && i < q->min; i++) {
		if (!match(re, advance(text, i))) { return -1; }
	}
	/* match rest */
	do {
		if (!q->more && i == q->max) { return -1; }
		/* Note: alternation is handled before this function is called
		   because we're matching a single character */
		n = match_exp(rest_re, advance(text, i), offset + 1, cap,
		              anchor);
		if (n >= 0) { return n + (int)i; }
	} while (++i < text.len && match(re, advance(text, i)));
	return -1;
}

/* Array of *_exp functions, indexed by `quantpol` */
static int (*expfn[])(struct quant const *, struct slice, matchfn *,
	struct slice, struct slice, size_t, struct recap [], int) =
{
	greedy_exp,
	lazy_exp
};

static int group_done(
	size_t const matches,
	struct quant const *q,
	struct slice re,
	struct slice const text,
	size_t const offset,
	struct recap cap[],
	int anchor)
{
	if (matches < q->min) {
		/* Unable to match enough times */
		return -1;
	} else {
		return match_exp(re, text, offset, cap, anchor);
	}
}

static int greedy_group_rec(
	size_t matches,
	unsigned flags,
	struct quant const *q,
	struct slice re,
	struct slice text,
	size_t offset,
	struct recap cap[],
	struct slice r_re,
	struct recap r_cap[],
	int anchor)
{
	size_t i;
	int m, n;

	if (q->more || matches < q->max) {
		/* Iteratively use a shorter and shorter sub-string of text */
		for (i = 0; i <= text.len ; i++) {
			/* Match group expression for the current match */
			n = match_alt(re, shrink(text, i), offset,
			              nextcap(cap, flags, 1), 0);

			/* No match, */
			if (n < 0) { break; }

			/* Zero-length match - trying to match expression again
			   would yield zero as well, cut it short here. */
			if (n == 0) {
				m = group_done(matches + 1, q, r_re, text,
				               offset, r_cap, anchor);
				if (m < 0) { return -1; }
			} else {
				/* Try to match the rest of the repetitions
				   (but they cannot capture anymore). This
				   recursive call eventually calls
				   `group_done()`, which matches the rest of
				   the regular expression. */
				m = greedy_group_rec(matches + 1, flags, q,
				                     re, advance(text, n),
				                     offset + n, NULL,
				                     r_re, r_cap, anchor);
				if (m < 0) { continue; }
			}
			if (matches == 0) {
				setcap(cap, flags, offset, n);
			}
			return n + m;
		}
	}
	return group_done(matches, q, r_re, text, offset, r_cap, anchor);
}

static int greedy_group(
	unsigned flags,
	struct quant const *q,
	struct slice re,
	struct slice text,
	size_t off,
	int anchor,
	struct recap cap[],
	struct slice r_re,
	struct recap r_cap[])
{
	/* OPTIMIZE: `analyze()` could be called here to check whether the
	   group regular expression has a fixed capture length, in which case
	   it is possible to backtrack by jumping back with a fixed length
	   offset, like `greedy_exp()`, rather than storing the backtrack
	   information on the stack as in the following recursive function. */
	return greedy_group_rec(0, flags, q, re, text, off, cap, r_re, r_cap,
	                        anchor);
}

static int lazy_group(
	unsigned flags,
	struct quant const *q,
	struct slice re,
	struct slice text,
	size_t offset,
	int anchor,
	struct recap cap[],
	struct slice r_re,
	struct recap r_cap[])
{
	size_t i, j;
	int n, m;

	for (n = i = j = 0; j <= text.len; i++, j += n) {
		m = group_done(i, q, r_re, advance(text, j), offset + j,
		               r_cap, anchor);
		if (m >= 0) {
			return j + m;
		}
		if (!q->more && i == q->max) { break; }
		n = match_alt(re, advance(text, j), offset + j,
		              i == 0 ? nextcap(cap, flags, 1) : NULL, 0);
		if (n < 0) { break; }
		if (i == 0) { setcap(cap, flags, offset, n); }
	}
	return -1;
}

/* Array of *_group functions, indexed by `quantpol` */
static int (*groupfn[])(unsigned, struct quant const *, struct slice,
	struct slice, size_t, int, struct recap [], struct slice,
	struct recap []) =
{
	greedy_group,
	lazy_group
};

/* Match the next regular-expression in the current branch */
static int match_exp(struct slice re, struct slice text,
                     int offset, struct recap cap[], int anchor)
{
	int relen, qlen, n;
	matchfn *match;
	struct quant q;
	struct slice next, rest;

	if (re.len == 0) { /* end of regex or branch */
		return anchor && text.len ? -1 : 0;
	}

	if (re.p[0] == '(') { /* group */
		unsigned flags, ncap;

		ncap = 0;
		relen = parse_group(re, &flags, cap ? &ncap : NULL);
		if (relen < 0) { return -1; }

		qlen = parse_quantifier(advance(re, relen), &q);
		if (qlen < 0) { return -1; }

		next = get_groupre(re.p, relen, flags);
		rest = advance(re, relen + qlen);
	
		n = groupfn[q.policy](flags, &q, next, text, offset, anchor,
		                      cap, rest, nextcap(cap, flags, ncap));
		if (n > 0 && anchor && rest.len == 0) {
			return (size_t)n == text.len ? n : -1;
		} else {
			return n;
		}
	}

	/* single character */

	if (re.p[0] == '\\') {
		/* built-in character class */
		relen = parse_builtin(re, &match);
	} else if (re.p[0] == '[') {
		/* custom character class */
		relen = parse_class(re, &match);
	} else {
		match = (re.p[0] == '.') ? match_any : match_exact;
		relen = 1;
	}
	if (relen < 0) { return -1; }

	qlen = parse_quantifier(advance(re, relen), &q);
	if (qlen < 0) { return -1; }

	next = mkslicen(re.p, relen);
	rest = advance(re, relen + qlen);

	return expfn[q.policy](&q, next, match, rest, text, offset, cap,
	                       anchor);
}

static void clear_captures(unsigned index, unsigned n, struct recap cap[])
{
	size_t i;

	if (cap) {
		for (i = 0; i < n; i++) {
			cap[index + i].offset = -1;
			cap[index + i].length = 0;
		}
	}
}

/* Match the first possible branch */
static int match_alt(struct slice re, struct slice text,
                     int offset, struct recap cap[], int anchor)
{
	size_t off;
	int n, m, l;
	struct slice s;
	unsigned i, j, ncap, mcap;

	off = 0;
	l = n = -1;
	mcap = 0;
	j = i = 0;

	do {
		ncap = 0;

		/* Skip past previous branches */
		s = advance(re, off);

		/* Find length of current branch */
		m = parse_branch(s, &ncap);
		if (m < 0) { return -1; }
		off += m + 1;
	
		/* Clear out capture groups within branch */
		clear_captures(i, ncap, cap);
		n = match_exp(prefix(s, m), text, offset,
		              nextcap(cap, CAPTURE, i), anchor);

		if (n < 0) {
			/* Clear out capture groups in branch that didn't
			   match */
			clear_captures(i, ncap, cap);
		} else if (l < n) {
			/* Clear out captures from the previously longest
			   branch */
			clear_captures(j, mcap, cap);
			l = n;
			j = i;
			mcap = ncap;
		}
		i += ncap;
	} while (off <= re.len);

	return l;
}

static size_t has_end_anchor(char const *str)
{
	size_t i, n;

	n = strlen(str);
	if (str[n - 1] != '$') {
		return 0;
	}
	/* Is it escaped? */
	for (i = 1; i < n; i++) {
		if (str[n - i - 1] != '\\') { break; }
	}
	return i & 1;
}

int recapn(char const *re, char const *text, size_t textn, struct recap cap[])
{
	size_t i;
	int len, anchor;
	struct slice restr, textstr;
	struct recap *subcap;

	if (!re || !text) { return 0; }

	anchor = has_end_anchor(re);
	restr = shrink(mkslice(re), anchor);
	textstr = mkslicen(text, textn);
	subcap = nextcap(cap, CAPTURE, 1);
	setcap(cap, CAPTURE, -1, 0);

	if (re[0] == '^') {
		/* Only match at start of string */
		len = match_alt(advance(restr, 1), textstr, 0, subcap, anchor);
		if (len < 0) {
			setcap(cap, CAPTURE, -1, 0);
			return 0;
		} else {
			setcap(cap, CAPTURE, 0, len);
			return 1;
		}
	} else {
		/* Attempt to match at every location */
		i = 0;
		do {
			len = match_alt(restr, advance(textstr, i), i, subcap,
			                anchor);
			if (len >= 0) {
				setcap(cap, CAPTURE, i, len);
				return 1;
			}
		} while (++i < textstr.len);
		return 0;
	}
}

int rematchn(const char *re, const char *text, size_t textn)
{
	return recapn(re, text, textn, NULL);
}

int recap(char const *re, char const *text, struct recap cap[])
{
	return text ? recapn(re, text, strlen(text), cap) : -1;
}

int rematch(const char *re, const char *text)
{
	return recap(re, text, NULL);
}

/* Validate regular expression and return number of capturing groups in the
   regex */
int recount(const char *re)
{
	unsigned ncap = 0;
	/* Anchors don't have to bve removed from the regex since they cannot
	   invalidate a string or add additional capture groups, but they would
	   add to the length of the pattern. */
	return analyze(mkslice(re), NULL, &ncap) < 0 ? -1 : (int)(ncap + 1);
}

