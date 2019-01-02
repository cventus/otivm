#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "ok/ok.h"
#include "lx32x4.h"

enum {
	ALPHA,
	BETA,
	GAMMA
};

union lxcell state[] = {
   span(
	tag(int, adjacent), int_data(ALPHA),
	tag(int, adjacent), int_data(BETA),
	tag(int, nil),      int_data(GAMMA),
	tag(int, nil),      int_data(42)
)
};

struct lx_list list, list_cdr, list_cddr, list_cdddr;

void before_each_test(void)
{
	list = mklist(state, 0);
	list_cdr = mklist(state, 1);
	list_cddr = mklist(state, 2);
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
