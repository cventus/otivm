/* rematch - a small regular-expression library by Christoffer Ventus,
   based on ``A Regular-Expression Matcher'' by Brian Kernighan and Rob Pike
   (http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html)

   This implementation is a recursive, depth-first regular expression matcher
   that (despite Kernighan's suggestion) interprets the regular expression on
   the fly and doesn't allocate any data-structures on the heap -- everything
   (e.g back-tracking state) is all kept on the call-stack. Inteded for simple
   regular expressions (e.g. parsing line-oriented config/data).

   It supports grouping and captures, alternations, classes, greedy and lazy
   quantifiers, and common short-hand notation.

   The stack space complexity is typically linear to the length of the regular
   expression, except when there are groups with (greedy) quantifiers. For this
   reason it's best to only use these functions for parsers with constant
   regexes, and not for matching run-time defined regexes, and avoid unbounded
   quantifiers for groups, i.e. only `{m,n}` and `?`, but not `*`, `{m,}` etc.

   Examples:

       rematch("[0-9]+?\\da{1,2}c\\D", "123aacc");
        -> 1

       rematch("foo|bar", "123 foo xyz");
        -> 1

       // `recap` can be used to get captures, and groups can be non-capturing
       // by using the "?:" prefix right after the opening parenthesis
       struct recap captures[3];
       recap("\s(?:(foo)|(bar))\s", "123 bar xyz", captures);
        -> 1,
        -> captures = {
             [0] = { 3, 5 },   // The range of the match - " bar "
             [1] = { -1, 0 },  // The first group (foo)
             [2] = { 4, 3 },   // The second group (bar) - "bar"
           }

       // Validate expressions with `recount`
       assert(recount("'([^']*)'") == 2);
*/
#ifndef RE_H_INCLUDED

struct recap
{
	/* Position in text where match starts (or -1 if group didn't match) */
	int offset;

	/* Length of capture in chars */
	size_t length;
};

/* Return non-zero if `text` matches with `re` */
int rematch(const char *re, const char *text);
int rematchn(const char *re, const char *text, size_t textn);

/* Match and capture groups. The array `cap` should be of length greater or
   equal to the number of groups in `re` (i.e. number of unescaped opening
   parentheses) which can be checked with `recount()`. */
int recap(const char *re, const char *text, struct recap cap[]);
int recapn(const char *re, const char *text, size_t textn, struct recap cap[]);

/* Validate the regular expression syntactically, and return the number of
   capture groups it contains, or -1 on failure */
int recount(const char *re);

#endif

