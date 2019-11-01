#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "ok/ok.h"
#include "lx32x4.h"

enum { ALPHA, BETA, GAMMA };

struct lx_list list, list_cdr, list_cddr, list_cdddr;

#include STATE_DEFINITION

void before_each_test(void)
{
	list = mklist(state + alpha_cell, alpha_offset);
	list_cdr = mklist(state + beta_cell, beta_offset);
	list_cddr = mklist(state + gamma_cell, gamma_offset);
	list_cdddr = lx_empty_list();
}

int test_car(void)
{
	return assert_eq(lx_car(list), lx_valuei(ALPHA));
}

int test_cdr(void)
{
	return assert_list_eq(lx_cdr(list), list_cdr);
}

int test_cadr(void)
{
	return assert_eq(lx_car(list_cdr), lx_valuei(BETA));
}

int test_cddr(void)
{
	return assert_list_eq(lx_cdr(list_cdr), list_cddr);
}

int test_caddr(void)
{
	return assert_eq(lx_car(list_cddr), lx_valuei(GAMMA));
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
