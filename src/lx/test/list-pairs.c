#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "ok/ok.h"

#include "lx32x4.h"

enum { ALPHA, BETA, GAMMA };

/* Two spans, one conses in each and one (ALPHA) that is in both */
union lxcell state[] = {
   span(
	tag(int, nil),  int_data(GAMMA),
	tag(int, nil),  int_data(0xDEAD),
	tag(int, nil),  int_data(0xDEAD),
	tag(int, link), int_data(ALPHA)
), span(
	cdr_tag,        ref_data(0, 0, 1),
	tag(int, link), int_data(BETA),
	cdr_tag,        ref_data(2, -1, 0),
	tag(int, nil),  int_data(0xDEAD)
)
};

struct lx_list list, list_cdr, list_cddr, list_cdddr;

void before_each_test(void)
{
	list = mklist(state, 3); /* ALPHA */
	list_cdr = mklist(state + SPAN_LENGTH, 1); /* BETA */
	list_cddr = mklist(state, 0); /* GAMMA */
	list_cdddr = empty_list;
}

int test_car(void)
{
	return assert_eq(lx_car(list), lx_int(ALPHA));
}

int test_cdr(void)
{
	return assert_list_eq(lx_cdr(list), list_cdr);
}

int test_cadr(void)
{
	return assert_eq(lx_car(list_cdr), lx_int(BETA));
}

int test_cddr(void)
{
	return assert_list_eq(lx_cdr(list_cdr), list_cddr);
}

int test_caddr(void)
{
	return assert_eq(lx_car(list_cddr), lx_int(GAMMA));
}

int test_cdddr(void)
{
	return assert_list_eq(lx_cdr(list_cddr), list_cdddr);
}

int test_list_length(void)
{
	return assert_int_eq(lx_length(list), 3);
}

int test_list_cdr_length(void)
{
	return assert_int_eq(lx_length(list_cdr), 2);
}

int test_list_cddr_length(void)
{
	return assert_int_eq(lx_length(list_cddr), 1);
}

int test_list_cdddr_length(void)
{
	return assert_int_eq(lx_length(list_cdddr), 0);
}
