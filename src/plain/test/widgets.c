#include "ok/ok.h"
#include "widget/widgets.h"

int test_create_and_interact_with_a_button(void)
{
	struct wgt_tree *t;
	struct wgt_widget *w;

	enum { BUTTON };

	t = wgt_make_tree();

	w = wgt_make_button(t, BUTTON, "btn");


	return 0;
}
